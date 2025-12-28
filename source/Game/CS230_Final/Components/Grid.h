/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Spring
 * \par CS230
 * \copyright DigiPen Institute of Technology
 */
#pragma once
#include "../../../Engine/Camera.h"
#include "../../../Engine/Component.h"
#include "../../../Engine/Engine.h"
#include "../../../Engine/Fonts.h"
#include "../../../Engine/Texture.h"
#include "../../../Engine/Vec2.h"
#include <filesystem>
#include <memory>

class Grid : public CS230::Component
{
public:
	enum class DotColor
	{
		black,
		white
	};
	Grid();
	void Update(double dt) override;
	void Draw(DotColor,float);


private:
	Math::ivec2						window_size;
	bool							draw = true;
	std::shared_ptr<CS230::Texture> white_dot;
	std::shared_ptr<CS230::Texture> black_dot;
	std::shared_ptr<CS230::Texture> red_dot;
	std::shared_ptr<CS230::Texture> blue_dot;
};