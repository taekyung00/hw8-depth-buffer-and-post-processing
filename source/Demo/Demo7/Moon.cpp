#include "Moon.h"
#include <numbers>
#include "CS200/Shape.h"
#include "Engine/Window.h"

Moon::Moon() : GameObject(Math::vec2(Engine::GetWindow().GetSize())-Math::vec2{radius * 2,radius * 2})
{
	std::vector<CS230::Shape::Shape::Vertex> vertices;
	constexpr int							 numSegments = 64;
	// Center vertex with texture center
	vertices.emplace_back(0.0f, 0.0f);
	// Outer vertices
	for (int i = 0; i <= numSegments; ++i)
	{
		const float angle = static_cast<float>(i) * 2.0f * std::numbers::pi_v<float> / static_cast<float>(numSegments);
		const float x	  = std::cos(angle) * 0.5f;
		const float y	  = std::sin(angle) * 0.5f;
		vertices.emplace_back(x, y);
	}
	AddGOComponent(new CS230::Shape(CS230::Shape::PrimitivePattern::TriangleFan, vertices));
	SetScale({ radius * 2, radius * 2 });
}

void Moon::Draw(Math::TransformationMatrix camera_matrix, [[maybe_unused]]unsigned int color, [[maybe_unused]]float depth)
{
	CS230::Shape* shape = GetGOComponent<CS230::Shape>();
	if (shape)
	{
		shape->Draw(camera_matrix * GetMatrix(), CS200::YELLOW, 0.8f);
	}
}