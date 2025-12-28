/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#include "ConsoleTest.h"
#include "Engine/Engine.h"
#include "Engine/Input.h"
#include "Engine/GameStateManager.h"
#include "Engine/Window.h"
#include "Engine/TextureManager.h"


#include "CS200/IRenderer2D.h"
#include "CS200/NDC.h"
		
#include "Game/MainMenu.h"

#include <imgui.h>



ConsoleTest::ConsoleTest()
{
}

void ConsoleTest::Load()
{
	
}

void ConsoleTest::Update([[maybe_unused]] double dt)
{
	if (Engine::GetInput().KeyJustReleased(CS230::Input::Keys::Escape))
	{
		Engine::GetGameStateManager().PopState();
		Engine::GetGameStateManager().PushState<MainMenu>();
	}


}

void ConsoleTest::Draw()
{
	Engine::GetWindow().Clear(0x1a1a1aff);
	auto renderer_2d = Engine::GetTextureManager().GetRenderer2D();

	renderer_2d->BeginScene(CS200::build_ndc_matrix(Engine::GetWindow().GetSize()));


	renderer_2d->EndScene();
}

void ConsoleTest::DrawImGui()
{
	ImGui::Begin("Tests");

	ImGui::End();
}

void ConsoleTest::Unload()
{
}
