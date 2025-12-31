#include "Shape.h"
#include "CS200/RGBA.h"
#include "CS200/Renderer2DUtils.h"
#include "Engine/Path.h"
#include "OpenGL/GL.h"
#include <span>

namespace CS230
{
  Shape::Shape(PrimitivePattern pattern, std::span<const Vertex> vertices)
  {
	setup(pattern, /* WhiteTexture() */ OpenGL::CreateTextureFromMemory({ 1, 1 }, std::array<CS200::RGBA, 1>{ CS200::WHITE }), vertices);
  }

  Shape::Shape(PrimitivePattern pattern, OpenGL::TextureHandle texture, std::span<const Vertex> vertices)
  {
	setup(pattern, texture, vertices);
  }

  Shape::~Shape()
  {
	GL::DeleteBuffers(1, &vertexBuffer), vertexBuffer				 = 0;
	GL::DeleteTextures(1, &textureHandle), textureHandle			 = 0;
	GL::DeleteVertexArrays(1, &vertexArrayObject), vertexArrayObject = 0;
	OpenGL::DestroyShader(shapeShader);
  }

  Shape::Shape(Shape&& other) noexcept
	  : vertexBuffer(other.vertexBuffer), vertexArrayObject(other.vertexArrayObject), textureHandle(other.textureHandle), shapeShader(std::move(other.shapeShader)), primitivePattern(other.primitivePattern),
		vertexCount(other.vertexCount)
  {
	other.vertexBuffer		= 0;
	other.vertexArrayObject = 0;
	other.textureHandle		= 0;
	other.vertexCount		= 0;
	other.shapeShader		= {};
  }

  Shape& Shape::operator=(Shape&& other) noexcept
  {
	std::swap(vertexBuffer, other.vertexBuffer);
	std::swap(vertexArrayObject, other.vertexArrayObject);
	std::swap(textureHandle, other.textureHandle);
	std::swap(primitivePattern, other.primitivePattern);
	std::swap(vertexCount, other.vertexCount);
	std::swap(shapeShader, other.shapeShader);
	return *this;
  }

  void Shape::Draw(Math::TransformationMatrix model_matrix, unsigned int color, float depth) const noexcept
  {
	GL::UseProgram(shapeShader.Shader);

	const auto&				   locations			 = shapeShader.UniformLocations;
	Math::TransformationMatrix anchored_model_matrix = model_matrix * Math::TranslationMatrix(Math::vec2{ 0.5, 0.5 });
	const auto				   model_matrix_opengl	 = CS200::Renderer2DUtils::to_opengl_mat3(anchored_model_matrix);
	GL::UniformMatrix3fv(locations.at("uTransform"), 1, GL_FALSE, model_matrix_opengl.data());
	GL::Uniform4fv(locations.at("uTint"), 1, CS200::unpack_color(color).data());
	GL::Uniform1f(locations.at("uDepth"), depth);

	GL::ActiveTexture(GL_TEXTURE0);
	GL::BindTexture(GL_TEXTURE_2D, textureHandle);
	GL::BindVertexArray(vertexArrayObject);

	GL::DrawArrays(static_cast<GLenum>(primitivePattern), 0, vertexCount);

	GL::BindTexture(GL_TEXTURE_2D, 0);
	GL::BindVertexArray(0);
	GL::UseProgram(0);
  }

  void Shape::setup(PrimitivePattern pattern, OpenGL::TextureHandle texture, std::span<const Vertex> vertices)
  {
	shapeShader		 = OpenGL::CreateShader(assets::locate_asset("Assets/shaders/shape/shape.vert"), assets::locate_asset("Assets/shaders/shape/shape.frag"));
	primitivePattern = pattern;
	textureHandle	 = texture;
	vertexCount		 = static_cast<GLsizei>(vertices.size());

	// Create vertex buffer
	vertexBuffer = OpenGL::CreateBuffer(OpenGL::BufferType::Vertices, std::as_bytes(vertices));

	// no index buffer this time

	// Create vertex array object
	const auto layout = {
	  OpenGL::VertexBuffer{ vertexBuffer, { OpenGL::Attribute::Float2, OpenGL::Attribute::Float2 } }
	};
	vertexArrayObject = OpenGL::CreateVertexArrayObject(layout);
  }
}
