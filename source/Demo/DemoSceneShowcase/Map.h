/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#pragma once
#include <map>
#include "Engine/Component.h"
#include "Engine/Vec2.h"
#include "Engine/Timer.h"

#include "Demo/DemoSceneShowcase/Samurai.h"

class Map : public CS230::Component
{
public:
	Map(Math::ivec2 tileSize,Math::ivec2 tileAmount,Samurai* samurai_ptr);
	void Update(double dt) override;
	void Draw(float depth);

private:
	static constexpr double			  PARTICLE_TIME = 2.5;
    const Math::ivec2 tile_size;
    const Math::ivec2 tile_amount;
	std::map<std::pair<int, int>, std::pair<bool, double>> grid_info; //<x,y> , <on_player,time>
    Samurai* samurai;
};