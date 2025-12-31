#pragma once

#include "OpenGL/Buffer.h"
#include "OpenGL/Shader.h"
#include "OpenGL/Texture.h"
#include "OpenGL/VertexArray.h"

#include "Engine/Component.h"
#include "Engine/Matrix.h"

#include <GL/glew.h>
#include <array>
#include <memory>
#include <span>

namespace CS230
{
	class Shape : public Component
	{
	public:
		enum class PrimitivePattern : GLenum
		{
			Points		  = GL_POINTS,
			Lines		  = GL_LINES,
			LineStrip	  = GL_LINE_STRIP,
			LineLoop	  = GL_LINE_LOOP,
			Triangles	  = GL_TRIANGLES,
			TriangleStrip = GL_TRIANGLE_STRIP,
			TriangleFan	  = GL_TRIANGLE_FAN
		};

		struct Vertex
		{
			float						 x = 0, y = 0; //layout1
			float						 s = 0, t = 0; //layout2
			// std::array<unsigned char, 4> tint{ 255, 255, 255, 255 }; //layout3

			constexpr Vertex() noexcept = default;

			constexpr Vertex(float x_, float y_, float s_, float t_) noexcept : x(x_), y(y_), s(s_), t(t_)
			{
			}

			constexpr Vertex(float x_, float y_) noexcept : Vertex(x_, y_, 0.0f, 0.0f)
			{
			}


		};

	public:
		Shape(PrimitivePattern pattern, std::span<const Vertex> vertices);
		Shape(PrimitivePattern pattern, OpenGL::TextureHandle texture, std::span<const Vertex> vertices);

		~Shape();
		Shape(const Shape&)			   = delete;
		Shape& operator=(const Shape&) = delete;
		Shape(Shape&& other) noexcept;
		Shape& operator=(Shape&& other) noexcept;

		void Draw(Math::TransformationMatrix model_matrix, unsigned int color = 0xFFFFFFFF, float depth = 0.5f) const noexcept;


	private:
		OpenGL::BufferHandle	  vertexBuffer		= 0;
		OpenGL::VertexArrayHandle vertexArrayObject = 0;
		OpenGL::TextureHandle	  textureHandle		= 0;
		OpenGL::CompiledShader	  shapeShader{};
		PrimitivePattern		  primitivePattern = PrimitivePattern::Triangles;
		GLsizei					  vertexCount	   = 0;

	private:
		void setup(PrimitivePattern pattern, OpenGL::TextureHandle texture, std::span<const Vertex> vertices);
	};
}
