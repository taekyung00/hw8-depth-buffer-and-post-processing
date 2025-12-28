/**
 * \file
 * \author Rudy Castan
 * \author Taekyung Ho
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

#include "FPS.hpp"
#include "FarmSceneRenderer.hpp"
#include "ImmediateRenderer2D.hpp"
#include "OffscreenFramebuffer.hpp"
#include "Path.hpp"
#include "PostProcessingPipeline.hpp"
#include "Random.hpp"
#include "Shader.hpp"
#include "GL.h"
#include <GL/glew.h>
#include <SDL.h>
#include <algorithm>
#include <array>
#include <imgui.h>
#include <iostream>
#include <memory>
#include <stb_image.h>
#include <string>
#include <vector>
#include <random>

// Simple vector types
struct dvec2
{
    double x, y;
};

struct ivec2
{
    int x, y;
};

// rendering variables
std::unique_ptr<IRenderer2D> gRenderer;

// background scene variables
static constexpr ivec2 BACKGROUND_FRAME_SIZE{ 1920, 1080 };
static constexpr int    NUM_BACKGROUND_LAYERS = 8;
struct BackgroundLayer
{
    OpenGL::Handle texture;
    float depth;
};
std::array<BackgroundLayer, NUM_BACKGROUND_LAYERS> gBackgroundLayers;
void BackgroundRender();
void BackgroundSetup();

// duck variables
static constexpr ivec2 DUCK_FRAME_SIZE{ 256, 256 };
struct Duck
{
    dvec2  position;
    float r, g, b, a; // tint color
    float depth;
};
std::vector<Duck> gDucks;
OpenGL::Handle    gDuckTexture = 0;
void DuckRender();
void DuckSetup();
Duck CreateRandomDuck();

util::FPS gFPSTracker;
Uint32    gLastTicks = 0;

extern int gWidth;
extern int gHeight;


FarmSceneRenderer      gFarmScene;
float                  gZoom = 1.0f;
OffscreenFramebuffer   gOffscreenBuffer;
PostProcessingPipeline gPostProcessing;


OpenGL::CompiledShader gScreenShader;
OpenGL::Handle         gScreenVAO;
OpenGL::Handle         gScreenVBO;
GLsizei                gScreenVertexCount = 0;


float gAnimationTime  = 0.0f;
bool  gPauseAnimation = false;


bool  gEnablePostFX              = true;
bool  gUseMSAA                   = true;
int   gMSAASamples               = 4;
float gBoxBlurSize               = 2.0f;
float gBoxBlurSeparation         = 1.0f;
float gGammaValue                = 2.2f;
float gChromaticAberrationMouseX = 0.5f;
float gChromaticAberrationMouseY = 0.5f;
int   gPixelSize                 = 5;

// Forward declarations


void setupScreenTriangle()
{
    struct ScreenVertex
    {
        float x, y;
        float u, v;
    };

    const ScreenVertex vertices[] = {
        { -1.0f, -1.0f, 0.0f, 0.0f },
        {  3.0f, -1.0f, 2.0f, 0.0f },
        { -1.0f,  3.0f, 0.0f, 2.0f },
    };

    gScreenVertexCount = static_cast<GLsizei>(std::ssize(vertices));

    GL::GenBuffers(1, &gScreenVBO);
    GL::BindBuffer(GL_ARRAY_BUFFER, gScreenVBO);
    GL::BufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    GL::BindBuffer(GL_ARRAY_BUFFER, 0);

    GL::GenVertexArrays(1, &gScreenVAO);
    GL::BindVertexArray(gScreenVAO);
    GL::BindBuffer(GL_ARRAY_BUFFER, gScreenVBO);


    GL::EnableVertexAttribArray(0);
    GL::VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ScreenVertex), nullptr);
    GL::VertexAttribDivisor(0, 0);


    GL::EnableVertexAttribArray(1);
    GL::VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ScreenVertex), reinterpret_cast<void*>(2 * sizeof(float)));
    GL::VertexAttribDivisor(1, 0);

    GL::BindVertexArray(0);
    GL::BindBuffer(GL_ARRAY_BUFFER, 0);
}

void demo_setup()
{
    // Initialize renderer
    gRenderer = std::make_unique<ImmediateRenderer2D>();
    gRenderer->Init();

    BackgroundSetup();
    DuckSetup();
    GL::Enable(GL_BLEND);
    GL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GL::Enable(GL_DEPTH_TEST);
    GL::DepthFunc(GL_LESS); // set depth function to less
    GL::DepthMask(GL_TRUE); // enable depth write

    constexpr int ADAPTIVE_VSYNC = -1;
    constexpr int VSYNC          = 1;
    if (const auto result = SDL_GL_SetSwapInterval(ADAPTIVE_VSYNC); result != 0)
    {
        SDL_GL_SetSwapInterval(VSYNC);
    }
    

    // Initialize FPS tracking
    gLastTicks = SDL_GetTicks();

    // farm and post-processing initialization
    gFarmScene.Initialize();

    const auto use_msaa = gUseMSAA ? OffscreenFramebuffer::MSAA::True : OffscreenFramebuffer::MSAA::False;
    gOffscreenBuffer.Initialize(gWidth, gHeight, use_msaa, gMSAASamples);


    const std::filesystem::path screen_vert = assets::locate_asset("Assets/shaders/simple.vert");
    const std::filesystem::path screen_frag = assets::locate_asset("Assets/shaders/simple-texture.frag");
    gScreenShader                           = OpenGL::CreateShader(screen_vert, screen_frag);

    setupScreenTriangle();


    gPostProcessing.Initialize(gWidth, gHeight);

    {
        const std::filesystem::path box_blur_vert   = assets::locate_asset("Assets/shaders/simple.vert");
        const std::filesystem::path box_blur_frag   = assets::locate_asset("Assets/shaders/box-blur.frag");
        auto                        box_blur_shader = OpenGL::CreateShader(box_blur_vert, box_blur_frag);

        gPostProcessing.AddEffect(PostProcessingEffect(
            "Box Blur", PostProcessingEffect::Enable::False, box_blur_shader,
            [](const OpenGL::CompiledShader& shader)
            {
                glUniform1i(shader.UniformLocations.at("uBlurSize"), static_cast<int>(gBoxBlurSize));
                glUniform1f(shader.UniformLocations.at("uSeparation"), gBoxBlurSeparation);
            }));
    }

    {
       const std::filesystem::path gamma_vert   = assets::locate_asset("Assets/shaders/simple.vert");
       const std::filesystem::path gamma_frag   = assets::locate_asset("Assets/shaders/gamma-correct.frag");
       auto                        gamma_shader = OpenGL::CreateShader(gamma_vert, gamma_frag);

       gPostProcessing.AddEffect(PostProcessingEffect(
           "Gamma Correction", PostProcessingEffect::Enable::False, gamma_shader, [](const OpenGL::CompiledShader& shader) { glUniform1f(shader.UniformLocations.at("uGamma"), gGammaValue); }));
    }

    {
       const std::filesystem::path chroma_vert   = assets::locate_asset("Assets/shaders/simple.vert");
       const std::filesystem::path chroma_frag   = assets::locate_asset("Assets/shaders/chromatic-aberration.frag");
       auto                        chroma_shader = OpenGL::CreateShader(chroma_vert, chroma_frag);

       gPostProcessing.AddEffect(PostProcessingEffect(
           "Chromatic Aberration", PostProcessingEffect::Enable::False, chroma_shader,
           [](const OpenGL::CompiledShader& shader) { glUniform2f(shader.UniformLocations.at("uMouseFocusPoint"), gChromaticAberrationMouseX, gChromaticAberrationMouseY); }));
    }
    {
       const std::filesystem::path pixel_vert   = assets::locate_asset("Assets/shaders/simple.vert");
       const std::filesystem::path pixel_frag   = assets::locate_asset("Assets/shaders/pixelize.frag");
       auto                        pixel_shader = OpenGL::CreateShader(pixel_vert, pixel_frag);

       gPostProcessing.AddEffect(PostProcessingEffect(
           "Pixelization", PostProcessingEffect::Enable::False, pixel_shader, [](const OpenGL::CompiledShader& shader) { glUniform1i(shader.UniformLocations.at("pixelSize"), gPixelSize); })); 
        //    must be odd
    }

    if (gUseMSAA)
    {
        GL::Enable(GL_MULTISAMPLE);
    }
}

void demo_shutdown()
{
    gFarmScene.Shutdown();
    gOffscreenBuffer.Shutdown();
    gPostProcessing.Shutdown();

    if (gScreenVAO != 0)
    {
        GL::DeleteVertexArrays(1, &gScreenVAO);
        gScreenVAO = 0;
    }
    if (gScreenVBO != 0)
    {
        GL::DeleteBuffers(1, &gScreenVBO);
        gScreenVBO = 0;
    }
    if (gScreenShader.Shader != 0)
    {
        GL::DeleteProgram(gScreenShader.Shader);
        gScreenShader.Shader = 0;
    }
}

void demo_draw()
{
    // Update FPS tracker
	const Uint32 currentTicks = SDL_GetTicks();
	const Uint32 deltaTicks	  = currentTicks - gLastTicks;
	const double deltaSeconds = deltaTicks / 1000.0;
	gLastTicks				  = currentTicks;
	gFPSTracker.Update(deltaSeconds);

    static int last_width  = 0;
    static int last_height = 0;
    if (gWidth != last_width || gHeight != last_height)
    {
        gOffscreenBuffer.Resize(gWidth, gHeight);
        gPostProcessing.Resize(gWidth, gHeight);
        last_width  = gWidth;
        last_height = gHeight;
    }


    if (!gPauseAnimation)
    {
        gAnimationTime += 0.016f;
    }


    gOffscreenBuffer.BindForRendering();
    GL::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    GL::Clear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    GL::Viewport(0, 0, gWidth, gHeight);

    gFarmScene.Render(gWidth, gHeight, gAnimationTime, gZoom);
    BackgroundRender();
    DuckRender();


    GLuint scene_texture = gOffscreenBuffer.GetTexture();


    GLuint final_texture = scene_texture;
    if (gEnablePostFX)
    {
        // GL::Disable(GL_DEPTH_TEST); // disable depth test for post-processing
        final_texture = gPostProcessing.Apply(scene_texture);
        
    }


    GL::BindFramebuffer(GL_FRAMEBUFFER, 0);
    GL::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    GL::Clear(GL_COLOR_BUFFER_BIT);
    GL::Viewport(0, 0, gWidth, gHeight);

    GL::UseProgram(gScreenShader.Shader);

    GL::ActiveTexture(GL_TEXTURE0);
    GL::BindTexture(GL_TEXTURE_2D, final_texture);
    GL::Uniform1i(gScreenShader.UniformLocations.at("uColorTexture"), 0);

    GL::BindVertexArray(gScreenVAO);
    GL::DrawArrays(GL_TRIANGLES, 0, gScreenVertexCount);
    GL::BindVertexArray(0);
    GL::BindTexture(GL_TEXTURE_2D, 0);
    GL::UseProgram(0);
}

void demo_imgui()
{
    ImGui::Begin("Farm Scene");
    ImGui::SeparatorText("Depth Settings");

	if (ImGui::Button("Sort as Painters Algorithm"))
	{
		std::sort(gBackgroundLayers.begin(), gBackgroundLayers.end(), [](const BackgroundLayer& left, const BackgroundLayer& right) {
			return left.depth > right.depth; 
		});
	}

	if (ImGui::Button("Sort as Front to Back"))
	{
		std::sort(
			gBackgroundLayers.begin(), gBackgroundLayers.end(),
			[](const BackgroundLayer& left, const BackgroundLayer& right)
			{
				return left.depth < right.depth; // then smaller depth drawn first, and frag of larger depth gonna be skipped over by depth test, and hopefully save effort of fragment shader
			});
	}

	if (ImGui::Button("Sort Randomly"))
	{
		std::random_device rd;
		std::mt19937	   g(rd());
		std::shuffle(gBackgroundLayers.begin(), gBackgroundLayers.end(), g);
	}


    float percent = 100.0f / gZoom;
    ImGui::SliderFloat("Zoom", &percent, 100.0f, 400.0f);
    gZoom = 100.0f / percent;

    ImGui::SeparatorText("Animation");
    ImGui::Checkbox("Pause Animation", &gPauseAnimation);

    ImGui::SeparatorText("MSAA Settings");
    bool msaa_changed = ImGui::Checkbox("Enable MSAA", &gUseMSAA);

    ImGui::BeginDisabled(!gUseMSAA);


    static GLint max_msaa_samples = 0;
    if (max_msaa_samples == 0)
    {
        glGetIntegerv(GL_MAX_SAMPLES, &max_msaa_samples);
    }


    static std::vector<int> valid_samples;
    if (valid_samples.empty())
    {
        for (int samples = 2; samples <= max_msaa_samples; samples *= 2)
        {
            valid_samples.push_back(samples);
        }
    }


    int current_index = 0;
    for (size_t i = 0; i < valid_samples.size(); ++i)
    {
        if (valid_samples[i] == gMSAASamples)
        {
            current_index = static_cast<int>(i);
            break;
        }
    }


    std::string preview = std::to_string(gMSAASamples) + "x";
    if (ImGui::BeginCombo("MSAA Samples", preview.c_str()))
    {
        for (size_t i = 0; i < valid_samples.size(); ++i)
        {
            bool        is_selected = (current_index == static_cast<int>(i));
            std::string label       = std::to_string(valid_samples[i]) + "x";

            if (ImGui::Selectable(label.c_str(), is_selected))
            {
                gMSAASamples = valid_samples[i];
                msaa_changed = true;
            }

            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::EndDisabled();

    if (msaa_changed)
    {
        const auto use_msaa = gUseMSAA ? OffscreenFramebuffer::MSAA::True : OffscreenFramebuffer::MSAA::False;
        gOffscreenBuffer.SetMSAA(use_msaa, gMSAASamples);

        gMSAASamples = gOffscreenBuffer.GetMSAASamples();

        if (gUseMSAA)
        {
            glEnable(GL_MULTISAMPLE);
        }
        else
        {
            glDisable(GL_MULTISAMPLE);
        }
    }

    ImGui::SeparatorText("Post-Processing Effects");
    ImGui::Checkbox("Enable Post-FX", &gEnablePostFX);

    ImGui::BeginDisabled(!gEnablePostFX);


    if (auto* box_blur = gPostProcessing.GetEffect("Box Blur"))
    {
        ImGui::Checkbox(box_blur->Name.c_str(), reinterpret_cast<bool*>(&box_blur->Enabled));

        ImGui::BeginDisabled(box_blur->Enabled == PostProcessingEffect::Enable::False);
        ImGui::Indent();
        ImGui::SliderFloat("Blur Size", &gBoxBlurSize, 0.0f, 10.0f);
        ImGui::SliderFloat("Blur Separation", &gBoxBlurSeparation, 1.0f, 5.0f);
        ImGui::Unindent();
        ImGui::EndDisabled();
    }


    if (auto* gamma = gPostProcessing.GetEffect("Gamma Correction"))
    {
        ImGui::Checkbox(gamma->Name.c_str(), reinterpret_cast<bool*>(&gamma->Enabled));

        ImGui::BeginDisabled(gamma->Enabled == PostProcessingEffect::Enable::False);
        ImGui::Indent();
        ImGui::SliderFloat("Gamma", &gGammaValue, 0.5f, 4.0f);
        ImGui::Unindent();
        ImGui::EndDisabled();
    }


    if (auto* chroma = gPostProcessing.GetEffect("Chromatic Aberration"))
    {
        ImGui::Checkbox(chroma->Name.c_str(), reinterpret_cast<bool*>(&chroma->Enabled));

        ImGui::BeginDisabled(chroma->Enabled == PostProcessingEffect::Enable::False);
        ImGui::Indent();
        ImGui::SliderFloat("Focus X", &gChromaticAberrationMouseX, 0.0f, 1.0f);
        ImGui::SliderFloat("Focus Y", &gChromaticAberrationMouseY, 0.0f, 1.0f);
        ImGui::Unindent();
        ImGui::EndDisabled();
    }

    if (auto* pixelization = gPostProcessing.GetEffect("Pixelization"))
    {
        ImGui::Checkbox(pixelization->Name.c_str(), reinterpret_cast<bool*>(&pixelization->Enabled));

        ImGui::BeginDisabled(pixelization->Enabled == PostProcessingEffect::Enable::False);
        ImGui::SliderInt("Pixel Size", &gPixelSize, 1, 800);
        if ((gPixelSize & 1) == 0)
        {
            gPixelSize += 1; // make sure it's odd
        }
        // ImGui::Indent();
        // ImGui::Unindent();
        ImGui::EndDisabled();
    }

    ImGui::EndDisabled();

    ImGui::SeparatorText("Render Textures");


    static int         selected_effect_index = -1;
    static const char* effect_names[]        = { "Box Blur", "Gamma Correction", "Chromatic Aberration", "Pixelization" };
    static const char* current_effect_name   = "None";

    if (selected_effect_index >= 0 && selected_effect_index < 4)
    {
        current_effect_name = effect_names[selected_effect_index];
    }

    if (ImGui::BeginCombo("View Effect", current_effect_name))
    {
        if (ImGui::Selectable("None", selected_effect_index == -1))
        {
            selected_effect_index = -1;
        }

        for (int i = 0; i < 4; i++)
        {
            bool is_selected = (selected_effect_index == i);
            if (ImGui::Selectable(effect_names[i], is_selected))
            {
                selected_effect_index = i;
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }


    if (gEnablePostFX && selected_effect_index >= 0 && selected_effect_index < 4)
    {
        if (auto* effect = gPostProcessing.GetEffect(effect_names[selected_effect_index]))
        {
            if (effect->Enabled == PostProcessingEffect::Enable::True && effect->Framebuffer)
            {
                ImGui::Text("Output Texture (%s):", effect->Name.c_str());
                GLuint texture = effect->Framebuffer->GetTexture();

                const float     aspect_ratio   = static_cast<float>(gWidth) / static_cast<float>(gHeight);
                constexpr float display_width  = 400.0f;
                const float     display_height = display_width / aspect_ratio;

                ImGui::Image(static_cast<ImTextureRef>(texture), ImVec2(display_width, display_height), ImVec2(0, 1), ImVec2(1, 0));
            }
            else if (effect->Enabled == PostProcessingEffect::Enable::False)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Effect is disabled");
            }
        }
    }

    ImGui::End();
}


void BackgroundSetup()
{
    
	for (int i = 0; i < NUM_BACKGROUND_LAYERS; ++i)
	{
        GL::GenTextures(1, &gBackgroundLayers[i].texture);
		// Load background texture
		std::ostringstream sout;
		sout << "Assets/images/background_" <<  (i) << ".png";
		const std::filesystem::path image_path = assets::locate_asset(sout.str());

		const bool FLIP = true;
		stbi_set_flip_vertically_on_load(FLIP);
		int		   w = 0, h = 0;
		const int  num_channels		  = 4;
		int		   files_num_channels = 0;
		const auto image_bytes		  = stbi_load(image_path.string().c_str(), &w, &h, &files_num_channels, num_channels);


		GL::BindTexture(GL_TEXTURE_2D, gBackgroundLayers[i].texture);

		// Texture filtering
		GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR
		GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Texture wrapping
		GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
		GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		constexpr int base_mipmap_level = 0;
		constexpr int zero_border		= 0;
		GL::TexImage2D(GL_TEXTURE_2D, base_mipmap_level, GL_RGBA8, w, h, zero_border, GL_RGBA, GL_UNSIGNED_BYTE, image_bytes);
		stbi_image_free(image_bytes);

		GL::BindTexture(GL_TEXTURE_2D, 0);

        gBackgroundLayers[i].depth = static_cast<float>(i) / NUM_BACKGROUND_LAYERS; // Depth from 0.0, 0.125, ..., 0.875
	}
}

void BackgroundRender()
{
    // opaque background layers
    
    

    // Create NDC transform matrix
	std::array<float, 9> to_ndc{
		2.0f / static_cast<float>(gWidth),
		0.0f,
		0.0f, // column 0
		0.0f,
		2.0f / static_cast<float>(gHeight),
		0.0f, // column 1
		0.0f,
		0.0f,
		1.0f // column 2
	};

	gRenderer->BeginScene(to_ndc);

	// Draw each back
	for (int i = 0; i < NUM_BACKGROUND_LAYERS; ++i)
	{
		// Create transform matrix (scale by size and translate to position)
		const float width  = static_cast<float>(gWidth);
		const float height = static_cast<float>(gHeight);
		const float pos_x  = static_cast<float>(0);
		const float pos_y  = static_cast<float>(0);

		std::array<float, 9> transform{
			width, 0.0f,   0.0f, // column 0: scale X
			0.0f,  height, 0.0f, // column 1: scale Y
			pos_x, pos_y,  1.0f	 // column 2: translation
		};

		// Texture coordinates for sprite frame selection (left, bottom, right, top)
		const float left		= 0.0f;
		const float right		= 1.0f;
		const float bottom		= 0.0f;
		const float top			= 1.0f;

		std::array<float, 4> texture_coords{ left, bottom, right, top };

		// Tint color
		std::array<float, 4> tint{ 1, 1, 1, 1 };

		gRenderer->DrawQuad(transform, gBackgroundLayers[i].depth, gBackgroundLayers[i].texture, texture_coords, tint);
	}

	gRenderer->EndScene();
}

void DuckRender()
{

    GL::DepthMask(GL_FALSE); // disable depth write
	// Create NDC transform matrix
	std::array<float, 9> to_ndc{
		2.0f / static_cast<float>(gWidth),
		0.0f,
		0.0f, // column 0
		0.0f,
		2.0f / static_cast<float>(gHeight),
		0.0f, // column 1
		0.0f,
		0.0f,
		1.0f // column 2
	};

	gRenderer->BeginScene(to_ndc);

	// Draw each duck
	for (const auto& duck : gDucks)
	{
		// Create transform matrix (scale by size and translate to position)
		const float width  = static_cast<float>(DUCK_FRAME_SIZE.x);
		const float height = static_cast<float>(DUCK_FRAME_SIZE.y);
		const float pos_x  = static_cast<float>(duck.position.x);
		const float pos_y  = static_cast<float>(duck.position.y);

		std::array<float, 9> transform{
			width, 0.0f,   0.0f, // column 0: scale X
			0.0f,  height, 0.0f, // column 1: scale Y
			pos_x, pos_y,  1.0f	 // column 2: translation
		};

		// Texture coordinates for sprite frame selection (left, bottom, right, top)
		const float left		= 0.0f;
		const float right		= 1.0f;
		const float bottom		= 0.0f;
		const float top			= 1.0f;

		std::array<float, 4> texture_coords{ left, bottom, right, top };

		// Tint color
		std::array<float, 4> tint{ duck.r, duck.g, duck.b, duck.a };

		gRenderer->DrawQuad(transform, duck.depth, gDuckTexture, texture_coords, tint);
	}

	gRenderer->EndScene();
    GL::DepthMask(GL_TRUE); // enable depth write
}

void DuckSetup()
{
    // Load duck texture
    const std::filesystem::path image_path = assets::locate_asset("Assets/images/duck.png");

    const bool FLIP = true;
    stbi_set_flip_vertically_on_load(FLIP);
    int        w = 0, h = 0;
    const int  num_channels       = 4;
    int        files_num_channels = 0;
    const auto image_bytes        = stbi_load(image_path.string().c_str(), &w, &h, &files_num_channels, num_channels);

    GL::GenTextures(1, &gDuckTexture);
    GL::BindTexture(GL_TEXTURE_2D, gDuckTexture);

    // Texture filtering
    GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR
    GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Texture wrapping
    GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
    GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    constexpr int base_mipmap_level = 0;
    constexpr int zero_border       = 0;
    GL::TexImage2D(GL_TEXTURE_2D, base_mipmap_level, GL_RGBA8, w, h, zero_border, GL_RGBA, GL_UNSIGNED_BYTE, image_bytes);
    stbi_image_free(image_bytes);

    GL::BindTexture(GL_TEXTURE_2D, 0);

    // Create random ducks
    constexpr int NUM_DUCKS = 10;
    gDucks.reserve(NUM_DUCKS);
    for (int i = 0; i < NUM_DUCKS; ++i)
    {
        gDucks.push_back(CreateRandomDuck());
    }

    
    // GL::Enable(GL_DEPTH_TEST);

    // Initialize VSync (adaptive vsync with fallback to regular vsync)
    // https://wiki.libsdl.org/SDL_GL_SetSwapInterval
    
}

Duck CreateRandomDuck()
{
    Duck duck;
    duck.position = { static_cast<double>(util::random(-gWidth/2 + 100, gWidth/2 - 100)), static_cast<double>(util::random(-gHeight/2 + 100, gHeight/2 - 100)) };
    duck.r        = util::random(0.f, 1.f);
    duck.g        = util::random(0.f, 1.f);
    duck.b        = util::random(0.f, 1.f);
    duck.a        = util::random(0.5f, 0.8f);
    duck.depth    = util::random(-0.9f, -0.1f);
    return duck;
}
