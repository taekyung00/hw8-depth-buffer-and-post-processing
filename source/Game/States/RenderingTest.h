/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#pragma once
#include <memory>
#include <SDL.h>

#include "./Engine/GameState.h"
#include "./Engine/Texture.h"
#include "Engine/FPS.h"

#include "Demo/Demo7/Triangle.h"
#include "Demo/Demo7/Stars.h"
#include "Demo/Demo7/Moon.h"
#include "Demo/Demo7/Road.h"
#include "Demo/DemoSceneShowcase/Samurai.h"



class RenderingTest : public CS230::GameState
{
public:
	RenderingTest() = default;
	void Load() override;
	void Update(double dt) override;
	void Unload() override;
	void Draw() override;
	void DrawImGui() override;

	gsl::czstring GetName() const override
	{
		return "Rendering Test";
	}

private:
	std::shared_ptr<CS230::Texture> testTexture = nullptr;
	Math::fvec2 scale = {1.f,1.f};
	float rotate = 0.f;
	Math::fvec2 translate = {0.f,0.f};
	Math::ivec2 window_size;


	util::FPS FPSTracker;
	Uint32	  LastTicks = 0;
};