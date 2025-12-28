/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#include "RenderingTest.h"
#include "Engine/Camera.h"
#include "Engine/Engine.h"
#include "Engine/GameStateManager.h"
#include "Engine/Input.h"
#include "Engine/ShowCollision.h"
#include "Engine/TextureManager.h"
#include "Engine/Window.h"

#include "Engine/Particle.h"
#include <imgui.h>
#include <numbers>

#include "./CS200/IRenderer2D.h"
#include "./CS200/NDC.h"
#include "CS200/RenderingAPI.h"

#include "Game/CS230_Final/Components/Grid.h"
#include "Game/MainMenu.h"
#include "Game/Particles.h"

#include "Demo/DemoSceneShowcase/Samurai.h"

#include <OpenGL/GL.h>

void RenderingTest::Load()
{
	Engine::GetTextureManager().SwitchRenderer(CS230::TextureManager::RendererType::Batch);
#ifdef DEVELOPER_VERSION
	AddGSComponent(new CS230::ShowCollision());
	AddGSComponent(new Grid());
#endif
	AddGSComponent(new CS230::GameObjectManager);
	testTexture = Engine::GetTextureManager().Load("Assets/images/DemoDepthPost/background_0.png");

	AddGSComponent(new CS230::ParticleManager<Particles::Tears>());
	AddGSComponent(new CS230::ParticleManager<Particles::Shining>());
	AddGSComponent(new CS230::ParticleManager<Particles::Flame>());


	// Initialize FPS tracking
	LastTicks = SDL_GetTicks();

	CS200::RenderingAPI::SetClearColor(CS200::BLACK);
	Engine::GetWindow().ForceResize(1920, 1080);
	Engine::GetWindow().SetWindowPosition(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void RenderingTest::Update([[maybe_unused]] double dt)
{
	UpdateGSComponents(dt);


	// Update FPS tracker
	const Uint32 currentTicks = SDL_GetTicks();
	const Uint32 deltaTicks	  = currentTicks - LastTicks;
	const double deltaSeconds = deltaTicks / 1000.0;
	LastTicks				  = currentTicks;
	FPSTracker.Update(deltaSeconds);


	GetGSComponent<CS230::GameObjectManager>()->UpdateAll(dt);
	if (Engine::GetInput().KeyJustReleased(CS230::Input::Keys::Escape)) // very bottom!!
	{
		Engine::GetGameStateManager().PopState();
		Engine::GetGameStateManager().PushState<MainMenu>();
		return;
	}
}

void RenderingTest::Draw()
{
	if (Engine::GetWindow().GetSize() != window_size)
	{
		window_size = Engine::GetWindow().GetSize();
	}
	CS200::RenderingAPI::Clear();
	auto renderer_2d = Engine::GetTextureManager().GetRenderer2D();
	GL::Viewport(0, 0, window_size.x, window_size.y);

	renderer_2d->BeginScene(CS200::build_ndc_matrix(window_size) );
	GetGSComponent<CS230::GameObjectManager>()->DrawAll(Math::TransformationMatrix());


	testTexture->Draw(
		Math::TranslationMatrix(Math::to_vec2(translate)) * Math::RotationMatrix(static_cast<double>(rotate / 180 * std::numbers::pi_v<float>)) * Math::ScaleMatrix(Math::to_vec2(scale)));
	renderer_2d->EndScene();
}

void RenderingTest::DrawImGui()
{
	if (Engine::GetWindow().GetSize() != window_size)
	{
		window_size = Engine::GetWindow().GetSize();
	}

	if (ImGui::Begin("Texture Controls"))
	{
		ImGui::SliderFloat("Scale X", &(scale.x), -20.f, 20.0f, "%.1f px/s");
		ImGui::SliderFloat("Scale Y", &(scale.y), -20.f, 20.0f, "%.1f px/s");
		ImGui::SliderFloat("Rotate", &rotate, 0.f, 360.0f, "%.1f px/s");
		ImGui::SliderFloat("Translate X", &(translate.x), -(static_cast<float>(window_size.x) - 100.f), static_cast<float>(window_size.x) - 100.f, "%.1f px/s");
		ImGui::SliderFloat("Translate Y", &(translate.y), -(static_cast<float>(window_size.y) - 100.f), static_cast<float>(window_size.y) - 100.f, "%.1f px/s");
	}
	ImGui::End();

	/*if (ImGui::Begin("Particle Controls"))
	{
		if (ImGui::Button("Shine"))
		{
			const auto shining_particle = Engine::GetGameStateManager().GetGSComponent<CS230::ParticleManager<Particles::Shining>>();
			if (!shining_particle)
			{
				throw std::logic_error("no shining particle");
			}
			shining_particle->Emit(10, Math::to_vec2(translate), { 0, 0 }, { 20, 20 }, std::numbers::pi * 2);
		}
		ImGui::SameLine();

		if (ImGui::Button("Tears"))
		{
			const auto tears_particle = Engine::GetGameStateManager().GetGSComponent<CS230::ParticleManager<Particles::Tears>>();
			if (tears_particle)
			{
				tears_particle->Emit(10, Math::to_vec2(translate), { 0, 0 }, { -20, -20 }, std::numbers::pi * 2);
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Flame"))
		{
			const auto tears_particle = Engine::GetGameStateManager().GetGSComponent<CS230::ParticleManager<Particles::Flame>>();
			if (tears_particle)
			{
				tears_particle->Emit(1, Math::to_vec2(translate), { 0, 0 }, { 0, 20 }, std::numbers::pi * 2, 0xF36301FF);
			}
		}
	}
	ImGui::End();*/

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

void RenderingTest::Unload()
{
	GetGSComponent<CS230::GameObjectManager>()->Unload();
	ClearGSComponents();
}