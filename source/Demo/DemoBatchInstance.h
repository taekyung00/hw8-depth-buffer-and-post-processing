/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#pragma once
#include <GL/glew.h>
#include <SDL.h>
#include <array>
#include <imgui.h>
#include <memory>
#include <stb_image.h>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/FPS.h"
#include "Engine/GameState.h"
#include "Engine/Texture.h"
#include "Engine/Vec2.h"
#include "Engine/Window.h"

#include "OpenGL/Buffer.h"

#include "CS200/IRenderer2D.h"
#include "CS200/ImmediateRenderer2D.h"
#include "CS200/NDC.h"
#include "CS200/RenderingAPI.h"
#include "OpenGL/GL.h"

class DemoBatchInstance : public CS230::GameState
{
public:
	void Load() override;
	void Unload() override;
	void Update(double dt) override;
	void Draw() override;
	void DrawImGui() override;

	gsl::czstring GetName() const override
	{
		return "DemoBatchInstance";
	};

private:

	// Robot sprite sheet constants
	static constexpr Math::ivec2 ROBOT_FRAME_SIZE{ 63, 127 };
	static constexpr int		 ROBOT_NUM_FRAMES = 5;
	static constexpr size_t		 ROBOT_VARIATIONS = 64;

	// Robot instance data
	struct Robot
	{
		Math::vec2 position;
		int		   frame;
		float	   r, g, b; // tint color
		int		   variation;
	};

	std::vector<Robot>											  Robots;
	// std::unique_ptr<CS200::IRenderer2D>			 Renderer;
	// OpenGL::Handle				 gRobotTexture = 0;
	// std::array<OpenGL::Handle, ROBOT_VARIATIONS> RobotTextures{};
	std::array<std::shared_ptr<CS230::Texture>, ROBOT_VARIATIONS> robotTextures{};
	util::FPS													  FPSTracker;
	Uint32														  LastTicks		  = 0;
	bool														  VSyncEnabled	  = true;
	const char*													  OpenGLRenderer  = nullptr;
	int															  MaxTextureUnits = 0;
	size_t current_draw_call = 0;
	size_t current_draw_texture_call = 0;


	// Helper function to create a random robot
	Robot CreateRandomRobot();
};