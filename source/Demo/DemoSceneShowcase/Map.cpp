/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#include "Map.h"
#include <numbers>

#include "Engine/GameStateManager.h"
#include "Engine/TextureManager.h"

#include "Game/Particles.h"

Map::Map(Math::ivec2 tileSize, Math::ivec2 tileAmount, Samurai* samurai_ptr)
	: tile_size(tileSize), tile_amount(tileAmount), samurai(samurai_ptr)
{
	for (int i = 0; i < tile_amount.x; ++i)
	{
		for (int j = 0; j < tile_amount.y; ++j)
		{
			grid_info[{ i, j }].first = false;
		}
	}
}

void Map::Update([[maybe_unused]] double dt)
{
	const Math::vec2 samurai_position = samurai->GetPosition() /*+ static_cast<Math::vec2>(samurai->GetGOComponent<CS230::Sprite>()->GetHotSpot(1))*/;
	Math::vec2		 grid_pos{};
	for (int i = 0; i < tile_amount.x; ++i)
	{
		for (int j = 0; j < tile_amount.y; ++j)
		{
			if ((samurai_position.x >= i * tile_size.x && samurai_position.x <= (i + 1) * tile_size.x) && (samurai_position.y >= j * tile_size.y && samurai_position.y <= (j + 1) * tile_size.y))
			{
				grid_pos				  = Math::vec2{ static_cast<double>(i * tile_size.x), static_cast<double>(j * tile_size.y) } + Math::vec2{ tile_size.x / 2.0, tile_size.y / 2.0 };
				grid_info[{ i, j }].first = true;
				grid_info[{ i, j }].second += dt;
			}
			else
			{
				grid_info[{ i, j }].first = false;
				grid_info[{ i, j }].second += dt;
			}
		}
	}
}

void Map::Draw(float depth)
{
	CS200::RGBA			tile_color;
	CS200::IRenderer2D* renderer_ptr = Engine::GetTextureManager().GetRenderer2D();
	Math::vec2			grid_pos{};
	const auto			tears_particle = Engine::GetGameStateManager().GetGSComponent<CS230::ParticleManager<Particles::Tears>>();
	const auto			shine_particle = Engine::GetGameStateManager().GetGSComponent<CS230::ParticleManager<Particles::Shining>>();

	for (int i = 0; i < tile_amount.x; ++i)
	{
		for (int j = 0; j < tile_amount.y; ++j)
		{
			grid_pos = Math::vec2{ static_cast<double>(i * tile_size.x), static_cast<double>(j * tile_size.y) } + Math::vec2{ tile_size.x / 2.0, tile_size.y / 2.0 };
			if (grid_info[{ i, j }].first)
			{
				tile_color = CS200::PURPLE;
				if (tears_particle && grid_info[{ i, j }].second >= PARTICLE_TIME)
				{
					tears_particle->Emit(5, grid_pos + Math::to_vec2(tile_size) * 0.5, { 0, 0 }, { -100, -100 }, std::numbers::pi);
					grid_info[{ i, j }].second = 0.0;
				}
			}
			else
			{
				tile_color = CS200::BLACK;
				if (shine_particle && grid_info[{ i, j }].second >= PARTICLE_TIME)
				{
					shine_particle->Emit(5, grid_pos + Math::to_vec2(tile_size) * 0.5, { 0, 0 }, { -100, -100 }, std::numbers::pi);
					grid_info[{ i, j }].second = 0.0;
				}
			}
			renderer_ptr->DrawRectangle(Math::TranslationMatrix(grid_pos) * Math::ScaleMatrix(tile_size), tile_color, CS200::WHITE, 2.0, depth);
		}
	}
}
