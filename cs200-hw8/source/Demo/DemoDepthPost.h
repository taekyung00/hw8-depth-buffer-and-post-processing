/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#pragma once
#include <SDL.h>

#include "Engine/FPS.h"
#include "Engine/GameObjectManager.h"
#include "Engine/GameState.h"
#include "Engine/Particle.h"
#include "Engine/Vec2.h"

#include "CS200/OffscreenFramebuffer.h"
#include "CS200/PostProcessingPipeline.h"
#include "OpenGL/VertexArray.h"

class DemoDepthPost : public CS230::GameState
{
public:
	DemoDepthPost() = default;
	void Load() override;
	void Update(double dt) override;
	void Unload() override;
	void Draw() override;
	void DrawImGui() override;

	gsl::czstring GetName() const override
	{
		return "Demo Depth & Post-Processing";
	}

private:
	struct BackGroundLayer
	{
		std::shared_ptr<CS230::Texture> texture;
		float							depth;
	};

	static constexpr size_t					NUM_LAYERS = 8;
	std::array<BackGroundLayer, NUM_LAYERS> background_layers{};
	static constexpr Math::ivec2			default_window_size{ 1920, 1080 };
	inline static double ratio = 1.0;

	struct Duck
	{
		Math::vec2	position;
		CS200::RGBA color;
		float		depth;
	};

	std::shared_ptr<CS230::Texture> duck_texture;

	static constexpr size_t		NUM_DUCKS = 10;
	std::array<Duck, NUM_DUCKS> ducks{};

	util::FPS FPSTracker;
	Uint32	  LastTicks = 0;

	// msaa, post-processing
	bool				 useMSAA = true;
	OffscreenFramebuffer offscreenBuffer{};
	int					 MSAASamples = 4;

	OpenGL::CompiledShader	  screenShader{};
	OpenGL::BufferHandle	  screenVBO{};
	OpenGL::VertexArrayHandle screenVAO{};
	GLsizei					  screenVertexCount = 0;

	void setupScreenTriangle();

	PostProcessingPipeline postProcessing{};

	bool enablePostFX = true;


	float boxBlurSize				= 2.0f;
	float boxBlurSeparation			= 1.0f;
	float gammaValue				= 2.2f;
	float chromaticAberrationMouseX = 0.5f;
	float chromaticAberrationMouseY = 0.5f;
	int	  pixelSize					= 5;
};