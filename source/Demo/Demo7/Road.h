/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#pragma once
#include "Engine/GameObject.h"
#include "Engine/Matrix.h"

class Road : public CS230::GameObject
{
public:
	Road();

	GameObjectTypes Type() override
	{
		return GameObjectTypes::Road;
	}

	std::string TypeName() override
	{
		return "Road";
	}

	bool CanCollideWith([[maybe_unused]] GameObjectTypes other_object_type) override
	{
		return true;
	};

	void ResolveCollision([[maybe_unused]] GameObject* other_object) override {};
	void Draw(Math::TransformationMatrix camera_matrix, unsigned int color, float depth) override;

private:
};