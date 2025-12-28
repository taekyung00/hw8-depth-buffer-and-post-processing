#include "Road.h"
#include "CS200/Shape.h"
#include <numbers>

Road::Road() : GameObject({ 400, 0 })
{
	std::vector<CS230::Shape::Shape::Vertex> vertices;
	const int								 numSegments = 20;
	const float								 width		 = 1000.0f;
	const float								 height		 = 100.0f;

	for (int i = 0; i <= numSegments; ++i)
	{
		const float t	 = static_cast<float>(i) / static_cast<float>(numSegments);
			const float x	 = -width / 2.0f + t * width;
			const float wave = std::sin(t * 4.0f * std::numbers::pi_v<float>) * 10.0f;

		vertices.emplace_back(x, -height / 2.0f + wave);
		vertices.emplace_back(x, height / 2.0f + wave);
	}

    AddGOComponent(new CS230::Shape(CS230::Shape::PrimitivePattern::TriangleStrip, vertices));
    SetRotation(45.0);
}

void Road::Draw(Math::TransformationMatrix camera_matrix,[[maybe_unused]]unsigned int color, [[maybe_unused]]float depth)
{
    CS230::Shape* shape = GetGOComponent<CS230::Shape>();
	if (shape)
	{
		shape->Draw(camera_matrix * GetMatrix(), CS200::PURPLE, 0.8f);
	}
}
