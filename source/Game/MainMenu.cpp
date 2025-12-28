/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  MainMenu.cpp
Project:    CS230 Engine
Author:     Taekyung Ho
Created:    May 6, 2025
*/
#include "MainMenu.h"
#include "CS200/IRenderer2D.h"
#include "CS200/NDC.h"
#include "CS200/RenderingAPI.h"
#include "Demo/DemoDepthPost.h"
#include "Engine/Engine.h"
#include "Engine/GameStateManager.h"
#include "Engine/Input.h"
#include "Engine/TextManager.h"
#include "Engine/TextureManager.h"
#include "Engine/Window.h"

#include "OpenGL/Environment.h"

#include "States.h"
#include <imgui.h>

// (0.0 = 0%, 1.0 = 100%)
namespace
{
	// --- Menu UI Ratio ---
	const double TITLE_X_RATIO			   = 0.15;
	const double TITLE_Y_RATIO_FROM_BOTTOM = 0.8;
	const double TITLE_SCALE_VAL		   = 1.5;

	const double MENU_CENTER_X_RATIO	 = 0.5;
	const double MENU_WIDTH_RATIO		 = 0.4;
	const double MENU_ITEM_HEIGHT_RATIO	 = 0.05;
	const double MENU_ITEM_SPACING_RATIO = 0.03;
	const double MENU_START_Y_RATIO		 = 0.4;
}

MainMenu::MainMenu() : current_option(Option::DemoDepthPost)
{
}

void MainMenu::DrawImGui()
{
	
}

void MainMenu::select_option()
{
	switch (current_option)
	{
		case MainMenu::Option::DemoDepthPost:
			Engine::GetGameStateManager().PopState();
			Engine::GetGameStateManager().PushState<DemoDepthPost>();
			break;

		case MainMenu::Option::Exit: Engine::GetGameStateManager().PopState(); break;
		case MainMenu::Option::COUNT: break;
	}
}

void MainMenu::Load()
{
	CS200::RenderingAPI::SetClearColor(0x000000FF);
	if (!OpenGL::IsWebGL)
	{
		Engine::GetWindow().ForceResize(default_window_size.x, default_window_size.y);
		Engine::GetWindow().SetWindowPosition(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}
	Option temp_option = current_option;
	while (temp_option != Option::COUNT)
	{
		colors[temp_option] = non_seleted_color;
		temp_option			= static_cast<Option>(static_cast<int>(temp_option) + 1);
	}
	update_colors();

	const auto window_size = default_window_size;

	title_pos.x = window_size.x * TITLE_X_RATIO;
	title_pos.y = window_size.y * TITLE_Y_RATIO_FROM_BOTTOM;
	title_scale = Math::vec2{ TITLE_SCALE_VAL, TITLE_SCALE_VAL };

	const double text_width		  = window_size.x * MENU_WIDTH_RATIO;
	const double text_x			  = (window_size.x * MENU_CENTER_X_RATIO) - (text_width / 2.0);
	const double text_height	  = window_size.y * MENU_ITEM_HEIGHT_RATIO;
	const double start_y_from_top = window_size.y * MENU_START_Y_RATIO;

	const double item_bottom_y_from_top	   = start_y_from_top + text_height;
	const double item_bottom_y_from_bottom = window_size.y - item_bottom_y_from_top;

	menu_start_pos_bl	   = Math::vec2{ text_x, item_bottom_y_from_bottom };
	menu_item_size		   = Math::vec2{ text_width, text_height };
	menu_item_total_height = text_height + (window_size.y * MENU_ITEM_SPACING_RATIO);
}

void MainMenu::Update([[maybe_unused]] double dt)
{
	CS230::Input& input		  = Engine::GetInput();
	// Math::vec2	  mouse_pos	  = input.GetMousePos();
	// auto		  window_size = Engine::GetWindow().GetSize();
	
	update_colors();
	if (input.KeyJustReleased(CS230::Input::Keys::Up))
	{
		int current_index = static_cast<int>(current_option);
		int total_options = static_cast<int>(Option::COUNT);
		current_index	  = (current_index - 1 + total_options) % total_options;
		current_option	  = static_cast<Option>(current_index);
	}
	else if (input.KeyJustReleased(CS230::Input::Keys::Down))
	{
		int current_index = static_cast<int>(current_option);
		int total_options = static_cast<int>(Option::COUNT);
		current_index	  = (current_index + 1) % total_options;
		current_option	  = static_cast<Option>(current_index);
	}
	else if (input.KeyJustReleased(CS230::Input::Keys::Enter))
	{
		select_option();
	}


	// bool mouse_is_hovering = false;
	// int	 total_options	   = static_cast<int>(Option::COUNT);
	// for (int i = 0; i < total_options; ++i)
	// {
	// 	double item_bottom_y = menu_start_pos_bl.y - (i * menu_item_total_height);

	// 	Math::vec2 rect_pos{ menu_start_pos_bl.x, item_bottom_y };
	// 	Math::vec2 rect_size = menu_item_size;

	// 	if (IsPointInRect(mouse_pos, rect_pos, rect_size))
	// 	{
	// 		current_option	  = static_cast<Option>(i);
	// 		mouse_is_hovering = true;
	// 		break;
	// 	}
	// }

	// if (input.MouseJustPressed(0) && mouse_is_hovering)
	// {
	// 	select_option();
	// }

}

void MainMenu::Unload()
{
}

void MainMenu::Draw()
{
	CS200::RenderingAPI::Clear();
	auto renderer_2d = Engine::GetTextureManager().GetRenderer2D();
	renderer_2d->BeginScene(CS200::build_ndc_matrix(Engine::GetWindow().GetSize()));

	auto& text_manager = Engine::GetTextManager();

	text_manager.DrawText("Engine Porting", title_pos, Fonts::Outlined, title_scale, title_color);

	double current_item_y = 0;
	int	   i			  = 0;

	// Option: demo depth post
	i			   = static_cast<int>(Option::DemoDepthPost);
	current_item_y = menu_start_pos_bl.y - (i * menu_item_total_height);
	text_manager.DrawText("Demo Depth Post", Math::vec2{ menu_start_pos_bl.x, current_item_y }, Fonts::Outlined, { 1.0, 1.0 }, colors[Option::DemoDepthPost]);

	// Option: exit
	i			   = static_cast<int>(Option::Exit);
	current_item_y = menu_start_pos_bl.y - (i * menu_item_total_height);
	text_manager.DrawText("Exit", Math::vec2{ menu_start_pos_bl.x, current_item_y }, Fonts::Outlined, { 1.0, 1.0 }, colors[Option::Exit]);

	renderer_2d->EndScene();
}

gsl::czstring MainMenu::GetName() const
{
	return "MainMenu";
}

void MainMenu::update_colors()
{
	for (auto& color : colors)
	{
		if (color.first == current_option)
		{
			color.second = seleted_color;		
		}
		else
		{
			color.second = non_seleted_color;
		}
	}
}
