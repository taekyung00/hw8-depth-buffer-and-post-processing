/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Spring
 * \par CS230
 * \copyright DigiPen Institute of Technology
 */
#include "BadSandwich.h"
#include "Project.h"
#include "../../MainMenu.h"
#include "../../../Engine/TextureManager.h"
#include "../../../Engine/GameStateManager.h"
#include "../../../Engine/Input.h"
#include "../../../Engine/Window.h"

#include "CS200/NDC.h"
void BadSandwich::Load() {
    texture = Engine::GetTextureManager().Load("Assets/images/CS230_Final/BadSandwich.png");
#if defined(__EMSCRIPTEN__)

	AddGSComponent(new CS230::Camera(Math::rect{
		Math::vec2{								  0,								   0 },
		Math::vec2{ static_cast<double>(default_width), static_cast<double>(default_height) }
	  }));
	GetGSComponent<CS230::Camera>()->SetPosition(Math::vec2{ static_cast<double>(default_width) / 2, static_cast<double>(default_height) / 2 });
	GetGSComponent<CS230::Camera>()->SetFirstPersonView() = true;
	GetGSComponent<CS230::Camera>()->SetAnchoring()		  = true;
	GetGSComponent<CS230::Camera>()->SetScale({ 0.75, 0.75 });

#endif
}

void BadSandwich::Update([[maybe_unused]] double dt) {
    if (Engine::GetInput().KeyJustReleased(CS230::Input::Keys::Escape))
    {
        Engine::GetGameStateManager().PopState();
        Engine::GetGameStateManager().PushState<MainMenu>();
    }

    if (Engine::GetInput().KeyJustReleased(CS230::Input::Keys::R))
    {
        Engine::GetGameStateManager().PopState();
        Engine::GetGameStateManager().PushState<Project>();
    }
}

void BadSandwich::Unload() {

}

void BadSandwich::Draw() {
    Engine::GetWindow().Clear(UINT_MAX);
	auto renderer_2d = Engine::GetTextureManager().GetRenderer2D();
#if defined(__EMSCRIPTEN__)
	renderer_2d->BeginScene(CS200::build_ndc_matrix(Engine::GetWindow().GetSize(), true) * GetGSComponent<CS230::Camera>()->GetMatrix());
#else
	renderer_2d->BeginScene(CS200::build_ndc_matrix(Engine::GetWindow().GetSize()));
#endif
	texture->Draw(Math::TransformationMatrix());

	renderer_2d->EndScene();
}