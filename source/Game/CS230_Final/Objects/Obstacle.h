/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Spring
 * \par CS230
 * \copyright DigiPen Institute of Technology
 */
#pragma once
#include "../../../Engine/GameObject.h"
#include "../../Score.h"
#include "../Components/Random.h"
#include "../Components/Ziggle.h"

class Player;
class Obstacle : public CS230::GameObject {
public:
	Obstacle(Player*, Math::vec2 pos);
	int			DrawPriority() const override{ return 32; }

	int UpdatePriority() const override
	{
		return CS230::GameObject::UPDATEPRIORITY + 3;
	}
	GameObjectTypes		Type() override { return GameObjectTypes::Obstacle; }
	std::string			TypeName() override { return "Obstacle"; }

	bool				CanCollideWith(GameObjectTypes other_object_type) override;
	void				ResolveCollision(GameObject* other_object) override;
private:
	Player* player;
};