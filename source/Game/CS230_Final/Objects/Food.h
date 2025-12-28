/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Spring
 * \par CS230
 * \copyright DigiPen Institute of Technology
 */
#pragma once

#include "../../../Engine/GameObject.h"
#include "../Components/Bounce.h"
#include "../Components/Random.h"

class Passenger;

class Food : public CS230::GameObject {
public:
	Food(Passenger*, Math::vec2 pos);
	virtual int		DrawPriority() const override { return 33; }

	int UpdatePriority() const override
	{
		return CS230::GameObject::UPDATEPRIORITY + 2;
	}
	GameObjectTypes Type() override { return GameObjectTypes::Food; }
	std::string		TypeName() override { return "Food"; }
	void			Update(double dt) override;
private:
	Passenger* owner;
};
