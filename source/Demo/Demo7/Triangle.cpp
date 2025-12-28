#include "Triangle.h"

#include "CS200/Shape.h"

Triangle::Triangle() : GameObject({ 0.0, 0.0 })
{
	std::vector<CS230::Shape::Shape::Vertex> vertices;
	vertices.emplace_back(0.0f, -0.5f, 0.0f, 0.0f);
	vertices.emplace_back(0.5f, 0.5f, 0.0f, 0.0f);
	vertices.emplace_back(-0.5f, 0.5f, 0.0f, 0.0f);
	AddGOComponent(new CS230::Shape(CS230::Shape::PrimitivePattern::Triangles, vertices));
    SetScale({ 32.0, 32.0 });
}

void Triangle::Update([[maybe_unused]]double dt)
{
    
}

void Triangle::Draw(Math::TransformationMatrix camera_matrix, [[maybe_unused]]unsigned int color, [[maybe_unused]]float depth)
{
	CS230::Shape* shape = GetGOComponent<CS230::Shape>();
	if (shape)
	{
		shape->Draw(camera_matrix * GetMatrix(), CS200::ORANGE, 0.8f);
	}
}
