/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#include "SceneState.h"
#include <imgui.h>
#include "Engine/Collision.h"
#include "Engine/Input.h"
#include "Engine/TextureManager.h"
#include "Engine/Window.h"
#include "Engine/Timer.h"

#include "CS200/IRenderer2D.h"
#include "CS200/NDC.h"
#include "CS200/RenderingAPI.h"
#include "CS200/ImGuiHelper.h"

#include "OpenGL/Environment.h"

#include "Game/CS230_Final/Components/Grid.h"
#include "Game/MainMenu.h"
#include "Game/Particles.h"

#include "Demo/DemoSceneShowcase/Map.h"
#include "Demo/DemoSceneShowcase/Samurai.h"

void SceneState::Load()
{
	Engine::GetTextureManager().SwitchRenderer(CS230::TextureManager::RendererType::Batch);

#ifdef DEVELOPER_VERSION
	AddGSComponent(new CS230::ShowCollision());
	//AddGSComponent(new Grid());
#endif

	if (!OpenGL::IsWebGL)
	{
		Engine::GetWindow().ForceResize(TILESIZE.x * TILEAMOUNT.x, TILESIZE.y * TILEAMOUNT.y);
		Engine::GetWindow().SetWindowPosition(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}
	AddGSComponent(new CS230::GameObjectManager);
	AddGSComponent(new CS230::ParticleManager<Particles::Shining>());
	AddGSComponent(new CS230::ParticleManager<Particles::Tears>());

	samurai = new Samurai();
	GetGSComponent<CS230::GameObjectManager>()->Add(samurai);
	AddGSComponent(new Map(TILESIZE, TILEAMOUNT, samurai));

	// Initialize FPS tracking
	LastTicks = SDL_GetTicks();

	CS200::RenderingAPI::SetClearColor(0xFFFFFFFF);
}

void SceneState::Update([[maybe_unused]] double dt)
{
	UpdateGSComponents(dt);
	GetGSComponent<CS230::GameObjectManager>()->UpdateAll(dt);

	// Update FPS tracker
	const Uint32 currentTicks = SDL_GetTicks();
	const Uint32 deltaTicks	  = currentTicks - LastTicks;
	const double deltaSeconds = deltaTicks / 1000.0;
	LastTicks				  = currentTicks;
	FPSTracker.Update(deltaSeconds);

	if (Engine::GetInput().KeyJustReleased(CS230::Input::Keys::Escape))
	{
		Engine::GetGameStateManager().PopState();
		Engine::GetGameStateManager().PushState<MainMenu>();
	}
}

void SceneState::Unload()
{
	GetGSComponent<CS230::GameObjectManager>()->Unload();
	ClearGSComponents();
}

void SceneState::Draw()
{
	CS200::RenderingAPI::Clear();
	CS200::IRenderer2D* renderer_2d = Engine::GetTextureManager().GetRenderer2D();
	renderer_2d->BeginScene(CS200::build_ndc_matrix(Engine::GetWindow().GetSize()));
	GetGSComponent<Map>()->Draw(0.8f);
#ifdef DEVELOPER_VERSION
	//GetGSComponent<Grid>()->Draw(Grid::DotColor::black,0.2f);
#endif
	GetGSComponent<CS230::GameObjectManager>()->DrawAll(Math::TransformationMatrix());

	renderer_2d->EndScene();
	current_draw_call = renderer_2d->GetDrawCallCounter();
	current_draw_texture_call = renderer_2d->GetDrawTextureCounter();
}

void SceneState::DrawImGui()
{
	
	ImGui::Begin("Demo Settings");

	// Display FPS at the top
	ImGui::Text("FPS: %d", static_cast<int>(FPSTracker));
	ImGui::Separator();

	ImGui::Text("Draw Call Counter: %d", static_cast<int>(current_draw_call));
	ImGui::Text("Draw Texture Call Counter: %d", static_cast<int>(current_draw_texture_call));
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

	// Renderer selection
	auto&	   texture_manager		 = Engine::GetTextureManager();
	const auto current_renderer_type = texture_manager.GetCurrentRendererType();
	// const auto renderer_2d = texture_manager.GetRenderer2D();
	ImGui::Text("Renderer:");

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

	ImGui::End();

	
}
