/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#pragma once
#include <SDL.h>

#include "Engine/GameState.h"
#include "Engine/GameObjectManager.h"
#include "Engine/Particle.h"
#include "Engine/Vec2.h"
#include "Engine/FPS.h"

#include "Game/Particles.h"

#include "Demo/DemoSceneShowcase/Samurai.h"

class SceneState : public CS230::GameState
{
public:
	SceneState() = default;
	void Load() override;
	void Update(double dt) override;
	void Unload() override;
	void Draw() override;
	void DrawImGui() override;

	gsl::czstring GetName() const override
	{
		return "Demo Scene state";
	}

private:
	Samurai* samurai = nullptr;
	static constexpr Math::ivec2 TILESIZE = { 96, 96 };
	static constexpr Math::ivec2 TILEAMOUNT  = { 8, 8 };

	util::FPS FPSTracker;
	Uint32	  LastTicks = 0;
	bool	  VSyncEnabled = true;

	size_t current_draw_call = 0;
	size_t current_draw_texture_call = 0;
};