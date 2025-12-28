/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Spring
 * \par CS230
 * \copyright DigiPen Institute of Technology
 */
#include "Project.h"
#include "../../MainMenu.h"
#include "BadSandwich.h"
#include "Engine/Input.h"
#include "Engine/TextureManager.h"
#include "Engine/Window.h"
#include "GoodSandwich.h"
#include "NormalSandwich.h"
#include "OpenGL/Environment.h"
#include <SDL.h>
#include <imgui.h>

#include "CS200/IRenderer2D.h"
#include "CS200/NDC.h"
#include "CS200/RenderingAPI.h"


Project::Project() : player_ptr(nullptr)
// camera(nullptr)
{
}

void Project::Load()
{
	Engine::GetLogger().LogDebug("CS230 Final start loading");
	Engine::GetTextureManager().SwitchRenderer(CS230::TextureManager::RendererType::Batch);
	// srand(static_cast<unsigned int>(time(NULL)));
	// Engine::GetWindow().SetSize({ default_width, default_height });
	/*camera = new CS230::Camera({ Math::vec2{ 0,0 }, static_cast<Math::vec2>(Engine::GetWindow().GetSize()) });
	camera->SetLimit({ {0,0},Engine::GetWindow().GetSize() });
	AddGSComponent(camera);*/
	// GetGSComponent<CS230::Camera>()->SetPosition({ 0.0, 0.0 });
#if defined(__EMSCRIPTEN__)

	AddGSComponent(new CS230::Camera(Math::rect{
		Math::vec2{								  0,								   0 },
		Math::vec2{ static_cast<double>(default_width), static_cast<double>(default_height) }
	  }));
	GetGSComponent<CS230::Camera>()->SetPosition(Math::vec2{ static_cast<double>(default_width) / 2, static_cast<double>(default_height) / 2 });
	GetGSComponent<CS230::Camera>()->SetFirstPersonView() = true;
	GetGSComponent<CS230::Camera>()->SetAnchoring()		  = true;
	GetGSComponent<CS230::Camera>()->SetScale({ 0.75, 0.75 });

#else
	Engine::GetWindow().ForceResize(default_width, default_height);
	Engine::GetWindow().SetWindowPosition(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

#endif


#ifdef DEVELOPER_VERSION
	AddGSComponent(new CS230::ShowCollision());
	AddGSComponent(new Grid());
#endif
	AddGSComponent(new Random());
	AddGSComponent(new CS230::GameObjectManager);
	AddGSComponent(new CS230::ParticleManager<Particles::Tears>());
	AddGSComponent(new CS230::ParticleManager<Particles::Shining>());

	player_ptr = new Player();
	GetGSComponent<CS230::GameObjectManager>()->Add(player_ptr);
	bus_ptr = new Bus(player_ptr);
	GetGSComponent<CS230::GameObjectManager>()->Add(bus_ptr);
	GetGSComponent<CS230::GameObjectManager>()->Add(new Passenger(BusLine::line1, player_ptr, 0));
	GetGSComponent<CS230::GameObjectManager>()->Add(new Passenger(BusLine::line2, player_ptr, 0));
	GetGSComponent<CS230::GameObjectManager>()->Add(new Passenger(BusLine::line3, player_ptr, 0));
	GetGSComponent<CS230::GameObjectManager>()->Add(new Passenger(BusLine::line4, player_ptr, 0));
	GetGSComponent<CS230::GameObjectManager>()->Add(new Passenger(BusLine::line5, player_ptr, 0));
	GetGSComponent<CS230::GameObjectManager>()->Add(new Passenger(BusLine::line1, player_ptr, 1));
	GetGSComponent<CS230::GameObjectManager>()->Add(new Passenger(BusLine::line2, player_ptr, 1));
	GetGSComponent<CS230::GameObjectManager>()->Add(new Passenger(BusLine::line3, player_ptr, 1));
	GetGSComponent<CS230::GameObjectManager>()->Add(new Passenger(BusLine::line4, player_ptr, 1));
	GetGSComponent<CS230::GameObjectManager>()->Add(new Passenger(BusLine::line5, player_ptr, 1));
	GetGSComponent<CS230::GameObjectManager>()->Add(
		new Obstacle(player_ptr, { (BusStartPosition + SeatWidthHeight + PlayerWidthHeight * (static_cast<double>(rand()) / RAND_MAX) * 2), SeatWidthHeight * 2 }));
	GetGSComponent<CS230::GameObjectManager>()->Add(
		new Obstacle(player_ptr, { (BusStartPosition + SeatWidthHeight + PlayerWidthHeight * (static_cast<double>(rand()) / RAND_MAX) * 2), SeatWidthHeight * 3 }));
	GetGSComponent<CS230::GameObjectManager>()->Add(
		new Obstacle(player_ptr, { (BusStartPosition + SeatWidthHeight + PlayerWidthHeight * (static_cast<double>(rand()) / RAND_MAX) * 2), SeatWidthHeight * 5 }));

	CS200::RenderingAPI::SetClearColor(0x000000FF);

	Engine::GetLogger().LogDebug("CS230 Final finish loading");
}

void Project::Update([[maybe_unused]] double dt)
{
	UpdateGSComponents(dt);

	GetGSComponent<CS230::GameObjectManager>()->UpdateAll(dt);

	// Engine::GetGameStateManager().GetGSComponent<CS230::GameObjectManager>()->SortForDraw();
	Engine::GetGameStateManager().GetGSComponent<CS230::GameObjectManager>()->SortForUpdate();
	if (bus_ptr->IsEnd())
	{
		Bus::SandwichLevel level = bus_ptr->sandwich_level;
		switch (level)
		{
			case Bus::SandwichLevel::GoodSandwich:
				Engine::GetGameStateManager().PopState();
				Engine::GetGameStateManager().PushState<GoodSandwich>();
				break;
			case Bus::SandwichLevel::NormalSandwich:
				Engine::GetGameStateManager().PopState();
				Engine::GetGameStateManager().PushState<NormalSandwich>();
				break;
			case Bus::SandwichLevel::BadSandwich:
				Engine::GetGameStateManager().PopState();
				Engine::GetGameStateManager().PushState<BadSandwich>();
				break;
			default: break;
		}
	}
	if (Engine::GetInput().KeyJustReleased(CS230::Input::Keys::Escape))
	{
		Engine::GetGameStateManager().PopState();
		Engine::GetGameStateManager().PushState<MainMenu>();
	}
}

void Project::Unload()
{
	GetGSComponent<CS230::GameObjectManager>()->Unload();
	ClearGSComponents();
	player_ptr = nullptr;
	bus_ptr	   = nullptr;
	// camera = nullptr;
}

void Project::Draw()
{
	CS200::RenderingAPI::Clear();
	auto renderer_2d = Engine::GetTextureManager().GetRenderer2D();
#if defined(__EMSCRIPTEN__)
	renderer_2d->BeginScene(CS200::build_ndc_matrix(Engine::GetWindow().GetSize(),true) * GetGSComponent<CS230::Camera>()->GetMatrix());
#else
	renderer_2d->BeginScene(CS200::build_ndc_matrix(Engine::GetWindow().GetSize()));
#endif
	// Math::TransformationMatrix camera_matrix = camera->GetMatrix();
#ifdef DEVELOPER_VERSION
	GetGSComponent<Grid>()->Draw(Grid::DotColor::black, 0.2f);
#endif
	GetGSComponent<CS230::GameObjectManager>()->DrawAll(Math::TransformationMatrix());

	renderer_2d->EndScene();
}

void Project::DrawImGui()
{
	ImGui::Begin("Renderer Settings");
	ImGui::Separator();

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

	ImGui::End();
}

gsl::czstring Project::GetName() const
{
	return "Final Project";
}