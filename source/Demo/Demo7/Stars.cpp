#include "Stars.h"
#include "CS200/Shape.h"
#include "Engine/Random.h"
#include "Engine/Window.h"

Stars::Stars() : GameObject({ 0, Engine::GetWindow().GetSize().y * 3.0/4})
{
	std::vector<CS230::Shape::Shape::Vertex> vertices;
	constexpr size_t		   count = 1000;
		vertices.reserve(count);
		for (size_t i = 0; i < count; ++i)
		{
			const float x = static_cast<float>(util::random(-0.5f, 0.5f));
			const float y = static_cast<float>(util::random(-0.5f, 0.5f));
			vertices.emplace_back(x, y); // emplace_back - calling a constructor
		}
	AddGOComponent(new CS230::Shape(CS230::Shape::PrimitivePattern::Points, vertices));
    SetScale({static_cast<double>(Engine::GetWindow().GetSize().x),Engine::GetWindow().GetSize().y * 1.0/4});
}

void Stars::Update([[maybe_unused]]double dt)
{
    
}

void Stars::Draw(Math::TransformationMatrix camera_matrix, [[maybe_unused]]unsigned int color, [[maybe_unused]]float depth)
{
	CS230::Shape* shape = GetGOComponent<CS230::Shape>();
	if (shape)
	{
		shape->Draw(camera_matrix * GetMatrix(), CS200::YELLOW, 0.8f);
	}
}