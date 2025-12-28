/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#include "CS200/BatchRenderer2D.h"
#include "Engine/GameStateManager.h"
#include "Engine/Input.h"
#include "Engine/Path.h"
#include "Engine/Random.h"
#include "Engine/TextureManager.h"
#include "Game/MainMenu.h"

#include <imgui.h>
#include "DemoBatchInstance.h"

// Request high-performance GPU on systems with multiple GPUs (laptops with integrated + discrete)
// https://docs.nvidia.com/gameworks/content/technologies/desktop/optimus.htm
// https://gpuopen.com/learn/amdpowerxpressrequesthighperformance/
// https://gcc.gnu.org/wiki/Visibility

#if defined _WIN32 || defined __CYGWIN__
// Windows: Use __declspec(dllexport)
#	define GPU_EXPORT __declspec(dllexport)
#else
// Linux/Mac: Use visibility attribute
#	define GPU_EXPORT __attribute__((visibility("default")))
#endif

extern "C"
{
// NVIDIA Optimus: Request high-performance GPU
GPU_EXPORT unsigned long NvOptimusEnablement = 0x00000001;

// AMD PowerXpress: Request high-performance GPU
GPU_EXPORT int AmdPowerXpressRequestHighPerformance = 0x00000001;
}

void DemoBatchInstance::Load()
{
	// Cache OpenGL renderer info
	OpenGLRenderer = reinterpret_cast<const char*>(GL::GetString(GL_RENDERER));
	GL::GetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MaxTextureUnits);

	// // Initialize renderer
	// Renderer = std::make_unique<CS200::ImmediateRenderer2D>();
	// Renderer->Init();
	
	auto& texture_manager = Engine::GetTextureManager();
	texture_manager.SwitchRenderer(CS230::TextureManager::RendererType::Immediate);

	for (size_t i = 0; i < ROBOT_VARIATIONS; ++i)
	{
		// Load robot texture
		std::ostringstream sout;
		sout << "Assets/images/DemoBatchInstance/variations/robot_var_" << std::setfill('0') << std::setw(2) << (i + 1) << ".png";

		robotTextures[i] = texture_manager.Load(sout.str());
	}
	// Create random robots
	constexpr int NUM_ROBOTS = 20;
	Robots.reserve(NUM_ROBOTS);
	for (int i = 0; i < NUM_ROBOTS; ++i)
	{
		Robots.push_back(CreateRandomRobot());
	}

	// Initialize FPS tracking
	LastTicks = SDL_GetTicks();

	CS200::RenderingAPI::SetClearColor(0x578FE5FF);
}

void DemoBatchInstance::Unload()
{
}

void DemoBatchInstance::Update([[maybe_unused]] double dt)
{
	auto						texture_manager		  = Engine::GetTextureManager();
	[[maybe_unused]] const auto current_renderer_type = texture_manager.GetCurrentRendererType();
	// Update FPS tracker
	const Uint32				currentTicks		  = SDL_GetTicks();
	const Uint32				deltaTicks			  = currentTicks - LastTicks;
	const double				deltaSeconds		  = deltaTicks / 1000.0;
	LastTicks										  = currentTicks;
	FPSTracker.Update(deltaSeconds);

	if (Engine::GetInput().KeyJustReleased(CS230::Input::Keys::Escape))
	{
		Engine::GetGameStateManager().PopState();
		Engine::GetGameStateManager().PushState<MainMenu>();
	}
}

void DemoBatchInstance::Draw()
{
	CS200::RenderingAPI::Clear();
	// auto& renderer_2d = Engine::GetRenderer2D();
	CS200::IRenderer2D* renderer_2d = Engine::GetTextureManager().GetRenderer2D();
	renderer_2d->BeginScene(CS200::build_ndc_matrix(Engine::GetWindow().GetSize()));
	// Draw each robot
	for (const auto& robot : Robots)
	{
		// Tint color
		std::array<float, 4> tint{ robot.r, robot.g, robot.b, 1.0f };

		robotTextures[static_cast<size_t>(robot.variation)]->Draw(Math::TranslationMatrix(robot.position), { robot.frame * ROBOT_FRAME_SIZE.x, 0 }, ROBOT_FRAME_SIZE, CS200::pack_color(tint));
	}

	renderer_2d->EndScene();
	current_draw_call = renderer_2d->GetDrawCallCounter();
	current_draw_texture_call = renderer_2d->GetDrawTextureCounter();
}

void DemoBatchInstance::DrawImGui()
{
	ImGui::Begin("Demo Settings");

	// Display FPS at the top
	ImGui::Text("FPS: %d", static_cast<int>(FPSTracker));
	ImGui::Separator();

	ImGui::Text("Draw Call Counter: %d", static_cast<int>(current_draw_call));
	ImGui::Text("Draw Texture Call count: %d", static_cast<int>(current_draw_texture_call));
	ImGui::Separator();

	ImGui::Text("Activated Texture count: %d", static_cast<int>(ROBOT_VARIATIONS));
	ImGui::Separator();

	// Display OpenGL renderer info
	if (OpenGLRenderer)
	{
		ImGui::Text("OpenGL Renderer: %s", OpenGLRenderer);
		ImGui::Text("Max Texture Units: %d", MaxTextureUnits);
		ImGui::Separator();
	}

	// Renderer selection
	auto&	   texture_manager		 = Engine::GetTextureManager();
	const auto current_renderer_type = texture_manager.GetCurrentRendererType();
	// const auto renderer_2d = texture_manager.GetRenderer2D();
	ImGui::Text("Renderer:");
	if (ImGui::RadioButton("Immediate", current_renderer_type == CS230::TextureManager::RendererType::Immediate))
	{
		texture_manager.SwitchRenderer(CS230::TextureManager::RendererType::Immediate);
	}
	ImGui::SameLine();

	if (ImGui::RadioButton("Batch", current_renderer_type == CS230::TextureManager::RendererType::Batch))
	{
		texture_manager.SwitchRenderer(CS230::TextureManager::RendererType::Batch);
	}

	ImGui::SameLine();
	if (ImGui::RadioButton("Instanced", current_renderer_type == CS230::TextureManager::RendererType::Instanced))
	{
		texture_manager.SwitchRenderer(CS230::TextureManager::RendererType::Instanced);
	}
	ImGui::Separator();

	// VSync toggle
	if (ImGui::Checkbox("VSync", &VSyncEnabled))
	{
		// https://wiki.libsdl.org/SDL_GL_SetSwapInterval
		constexpr int ADAPTIVE_VSYNC = -1;
		constexpr int VSYNC			 = 1;
		constexpr int NO_VSYNC		 = 0;

		if (VSyncEnabled)
		{
			// Try adaptive vsync first, fall back to regular vsync
			if (const auto result = SDL_GL_SetSwapInterval(ADAPTIVE_VSYNC); result != 0)
			{
				SDL_GL_SetSwapInterval(VSYNC);
			}
		}
		else
		{
			SDL_GL_SetSwapInterval(NO_VSYNC);
		}
	}
	ImGui::Separator();

	// Display current robot count
	ImGui::Text("Current Robot Count: %zu", Robots.size());
	ImGui::Separator();

	// Amounts for adding/removing
	constexpr int	 amounts[]	= { 1, 10, 100, 1000, 10000, 100000 };
	constexpr size_t MAX_ROBOTS = 1000000; // Sanity limit to prevent crashes

	// Add robots buttons
	ImGui::Text("Add Robots:");
	for (int amount : amounts)
	{
		// Disable button if it would exceed max limit
		const bool can_add = (Robots.size() + static_cast<size_t>(amount)) <= MAX_ROBOTS;
		if (!can_add)
		{
			ImGui::BeginDisabled();
		}

		if (ImGui::Button((std::string("+") + std::to_string(amount)).c_str()))
		{
			const size_t new_count = std::min(Robots.size() + static_cast<size_t>(amount), MAX_ROBOTS);
			const size_t to_add	   = new_count - Robots.size();

			if (to_add > 0)
			{
				Robots.reserve(new_count);
				for (size_t i = 0; i < to_add; ++i)
				{
					Robots.push_back(CreateRandomRobot());
				}
			}
		}

		if (!can_add)
		{
			ImGui::EndDisabled();
		}
		ImGui::SameLine();
	}
	ImGui::NewLine();

	// Remove robots buttons
	ImGui::Text("Remove Robots:");
	for (int amount : amounts)
	{
		// Disable button if there are no robots to remove
		const bool can_remove = !Robots.empty();
		if (!can_remove)
		{
			ImGui::BeginDisabled();
		}

		if (ImGui::Button((std::string("-") + std::to_string(amount)).c_str()))
		{
			const size_t to_remove = std::min(static_cast<size_t>(amount), Robots.size());
			if (to_remove > 0)
			{
				Robots.resize(Robots.size() - to_remove);
			}
		}

		if (!can_remove)
		{
			ImGui::EndDisabled();
		}
		ImGui::SameLine();
	}
	ImGui::NewLine();

	ImGui::Separator();

	// Clear all button
	const bool has_robots = !Robots.empty();
	if (!has_robots)
	{
		ImGui::BeginDisabled();
	}

	if (ImGui::Button("Clear All"))
	{
		Robots.clear();
	}

	if (!has_robots)
	{
		ImGui::EndDisabled();
	}

	// Show warning when approaching limit
	if (Robots.size() > static_cast<size_t>(MAX_ROBOTS * 0.8))
	{
		ImGui::Separator();
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Warning: Approaching maximum robot limit!");
	}

	ImGui::End();
}

DemoBatchInstance::Robot DemoBatchInstance::CreateRandomRobot()
{
	const Math::vec2 window_size = Engine::GetWindow().GetSize();

	Robot			 robot;
	robot.position.x = (util::random(0.0, window_size.x));
	robot.position.y = (util::random(0.0, window_size.y));
	robot.frame		 = util::random(ROBOT_NUM_FRAMES);
	if (static_cast<float>(util::random(0.0, 1.0)) < 0.85f)
	{
		robot.r = robot.g = robot.b = 1.0f;
	}
	else
	{
		robot.r = static_cast<float>(util::random(0.5, 1.0));
		robot.g = static_cast<float>(util::random(0.6, 1.0));
		robot.b = static_cast<float>(util::random(0.45, 1.0));
	}
	robot.variation = util::random(static_cast<int>(ROBOT_VARIATIONS));
	return robot;
}
