/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */
#include "Demo/DemoDepthPost.h"
#include "DemoDepthPost.h"
#include "Engine/TextureManager.h"
#include <algorithm>
#include <random>

#include "Engine/Camera.h"
#include "Engine/Collision.h"
#include "Engine/Input.h"
#include "Engine/Path.h"
#include "Engine/Random.h"
#include "Engine/TextureManager.h"
#include "Engine/Timer.h"
#include "Engine/Window.h"
#include <imgui.h>

#include "CS200/IRenderer2D.h"
#include "CS200/ImGuiHelper.h"
#include "CS200/NDC.h"
#include "CS200/RenderingAPI.h"

#include "OpenGL/Buffer.h"
#include "OpenGL/Environment.h"
#include "OpenGL/GL.h"

#include "Game/MainMenu.h"

void DemoDepthPost::setupScreenTriangle()
{
	struct ScreenVertex
	{
		float x, y;
		float u, v;
	};

	const ScreenVertex vertices[] = {
		{ -1.0f, -1.0f, 0.0f, 0.0f },
		{  3.0f, -1.0f, 2.0f, 0.0f },
		{ -1.0f,	 3.0f, 0.0f, 2.0f },
	};

	screenVertexCount = static_cast<GLsizei>(std::ssize(vertices));

	screenVBO = OpenGL::CreateBuffer(OpenGL::BufferType::Vertices, std::as_bytes(std::span{ vertices }));

	const auto layout = {
		OpenGL::VertexBuffer{ screenVBO,
							  {
								  OpenGL::Attribute::Float2, // Location 0: Position (x, y)
								  OpenGL::Attribute::Float2	 // Location 1: TexCoord (u, v)
							  } }
	};

	screenVAO = OpenGL::CreateVertexArrayObject(layout);
}

void DemoDepthPost::Load()
{
	Engine::GetWindow().ForceResize(default_window_size.x, default_window_size.y);
	Engine::GetWindow().SetWindowPosition(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	auto& texture_manager = Engine::GetTextureManager();
	for (size_t i = 0; i < NUM_LAYERS; ++i)
	{
		background_layers[i].texture = texture_manager.Load("Assets/images/DemoDepthPost/background_" + std::to_string(i) + ".png");
		background_layers[i].depth	 = static_cast<float>(i) / NUM_LAYERS; // Depth from 0.0, 0.125, ..., 0.875
	}
	CS200::RenderingAPI::SetClearColor(CS200::WHITE);

	texture_manager.SwitchRenderer(CS230::TextureManager::RendererType::Batch);

	// Initialize FPS tracking
	LastTicks = SDL_GetTicks();

	// Initialize ducks
	duck_texture = texture_manager.Load("Assets/images/DemoDepthPost/duck.png");
	for (size_t i = 0; i < NUM_DUCKS; ++i)
	{
		ducks[i].position = { static_cast<double>(util::random(100, default_window_size.x - 100)), static_cast<double>(util::random(100, default_window_size.y - 100)) };
		uint8_t r		  = static_cast<uint8_t>(util::random(256));
		uint8_t g		  = static_cast<uint8_t>(util::random(256));
		uint8_t b		  = static_cast<uint8_t>(util::random(256));
		uint8_t a		  = static_cast<uint8_t>(util::random(100, 250)); // get transparency between 100 and 250
		ducks[i].color	  = CS200::pack_color(std::array<uint8_t, 4>{ r, g, b, a });
		ducks[i].depth	  = static_cast<float>(util::random(-0.9, -0.1)); // Depth between -0.9 to -0.1
	}

	// sort ducks by depth back to front(painter's algorithm)
	std::sort(std::begin(ducks), std::end(ducks), [](const Duck& left, const Duck& right) { return left.depth > right.depth; });

	// msaa settings
	const auto use_msaa = useMSAA ? OffscreenFramebuffer::MSAA::True : OffscreenFramebuffer::MSAA::False;
	offscreenBuffer.Initialize(default_window_size.x, default_window_size.y, use_msaa, MSAASamples);

	const std::filesystem::path screen_vert = assets::locate_asset("Assets/shaders/PostProcess/simple.vert");
	const std::filesystem::path screen_frag = assets::locate_asset("Assets/shaders/PostProcess/simple-texture.frag");
	screenShader							= OpenGL::CreateShader(screen_vert, screen_frag);

	setupScreenTriangle();

	postProcessing.Initialize(default_window_size.x, default_window_size.y);

	{
		const std::filesystem::path box_blur_vert	= assets::locate_asset("Assets/shaders/PostProcess/simple.vert");
		const std::filesystem::path box_blur_frag	= assets::locate_asset("Assets/shaders/PostProcess/box-blur.frag");
		auto						box_blur_shader = OpenGL::CreateShader(box_blur_vert, box_blur_frag);

		postProcessing.AddEffect(PostProcessingEffect(
			"Box Blur", PostProcessingEffect::Enable::False, box_blur_shader,
			[&](const OpenGL::CompiledShader& shader)
			{
				GL::Uniform1i(shader.UniformLocations.at("uBlurSize"), static_cast<int>(boxBlurSize));
				GL::Uniform1f(shader.UniformLocations.at("uSeparation"), boxBlurSeparation);
			}));
	}


	{
		const std::filesystem::path chroma_vert	  = assets::locate_asset("Assets/shaders/PostProcess/simple.vert");
		const std::filesystem::path chroma_frag	  = assets::locate_asset("Assets/shaders/PostProcess/chromatic-aberration.frag");
		auto						chroma_shader = OpenGL::CreateShader(chroma_vert, chroma_frag);

		postProcessing.AddEffect(PostProcessingEffect(
			"Chromatic Aberration", PostProcessingEffect::Enable::True, chroma_shader,
			[&](const OpenGL::CompiledShader& shader) { GL::Uniform2f(shader.UniformLocations.at("uMouseFocusPoint"), chromaticAberrationMouseX, chromaticAberrationMouseY); }));
	}
	{
		const std::filesystem::path pixel_vert	 = assets::locate_asset("Assets/shaders/PostProcess/simple.vert");
		const std::filesystem::path pixel_frag	 = assets::locate_asset("Assets/shaders/PostProcess/pixelize.frag");
		auto						pixel_shader = OpenGL::CreateShader(pixel_vert, pixel_frag);

		postProcessing.AddEffect(PostProcessingEffect(
			"Pixelization", PostProcessingEffect::Enable::True, pixel_shader,
			[&](const OpenGL::CompiledShader& shader) { GL::Uniform1i(shader.UniformLocations.at("pixelSize"), pixelSize); })); // must be odd
	}

	{
		const std::filesystem::path gamma_vert	 = assets::locate_asset("Assets/shaders/PostProcess/simple.vert");
		const std::filesystem::path gamma_frag	 = assets::locate_asset("Assets/shaders/PostProcess/gamma-correct.frag");
		auto						gamma_shader = OpenGL::CreateShader(gamma_vert, gamma_frag);

		postProcessing.AddEffect(PostProcessingEffect(
			"Gamma Correction", PostProcessingEffect::Enable::True, gamma_shader, [&](const OpenGL::CompiledShader& shader) { GL::Uniform1f(shader.UniformLocations.at("uGamma"), gammaValue); }));
	}

	GL::Enable(GL_BLEND);
	GL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (useMSAA)
	{
		GL::Enable(GL_MULTISAMPLE);
	}

	AddGSComponent(new CS230::Camera(Math::rect{ Math::to_vec2(default_window_size) * 0.3, Math::to_vec2(default_window_size) * 0.7 }));
	CS230::Camera* camera = GetGSComponent<CS230::Camera>();
	camera->SetLimit(
		{
			{ 0, 0 },
			  default_window_size
	  });
	camera->SetAnchoring() = true;
	camera->SetPosition({ default_window_size.x / 2.0, default_window_size.y / 2.0 });
	camera->SetFirstPersonView() = false;
	camera->SetScale({ scale, scale });
	camera->SetRotation(0.0);
	camera->SetSmoothing() = false;
}

void DemoDepthPost::Update([[maybe_unused]] double dt)
{
	// Update FPS tracker
	const Uint32 currentTicks = SDL_GetTicks();
	const Uint32 deltaTicks	  = currentTicks - LastTicks;
	const double deltaSeconds = deltaTicks / 1000.0;
	LastTicks				  = currentTicks;
	FPSTracker.Update(deltaSeconds);

	if (Engine::GetInput().KeyJustReleased(CS230::Input::Keys::Escape))
	{
		Engine::GetGameStateManager().PopState();
		Engine::GetGameStateManager().PushState<MainMenu>();
	}
}

void DemoDepthPost::Unload()
{
	offscreenBuffer.Shutdown();
	postProcessing.Shutdown();
	if (screenVAO != 0)
	{
		GL::DeleteVertexArrays(1, &screenVAO);
		screenVAO = 0;
	}
	if (screenVBO != 0)
	{
		GL::DeleteBuffers(1, &screenVBO);
		screenVBO = 0;
	}
	if (screenShader.Shader != 0)
	{
		OpenGL::DestroyShader(screenShader);
	}
}

void DemoDepthPost::Draw()
{
	static int		  last_width  = 0;
	static int		  last_height = 0;
	const Math::ivec2 window_size = Engine::GetWindow().GetSize();
	if (window_size.x != last_width || window_size.y != last_height)
	{
		offscreenBuffer.Resize(window_size.x, window_size.y);
		postProcessing.Resize(window_size.x, window_size.y);
		last_width	= window_size.x;
		last_height = window_size.y;
	}
	// 1. Render to Offscreen FBO
	GL::Enable(GL_DEPTH_TEST);
	offscreenBuffer.BindForRendering();
	CS200::IRenderer2D* renderer_2d = Engine::GetTextureManager().GetRenderer2D();
	renderer_2d->BeginScene(CS200::build_ndc_matrix(Engine::GetWindow().GetSize(), true) * GetGSComponent<CS230::Camera>()->GetMatrix());


	CS200::RenderingAPI::Clear(); // Clear Color & Depth

	// opaque background layers
	GL::DepthMask(GL_TRUE); // enable depth write
	GL::DepthFunc(GL_LESS); // set depth function to less

	CS200::RenderingAPI::SetViewport(window_size, { 0, 0 });

	for (const auto& layer : background_layers)
	{
		layer.texture->Draw(Math::TransformationMatrix(), 0xFFFFFFFF, layer.depth);
	}

	// transparent ducks
	GL::DepthMask(GL_FALSE); // disable depth write
	for (const auto& duck : ducks)
	{
		duck_texture->Draw(Math::TranslationMatrix(duck.position), duck.color, duck.depth);
	}
	GL::DepthMask(GL_TRUE); // enable depth write
	renderer_2d->EndScene();

	// 2. Resolve (MSAA -> Texture) & PostProcess
	OpenGL::TextureHandle scene_texture = offscreenBuffer.GetTexture();
	OpenGL::TextureHandle final_texture = scene_texture;

	GL::Disable(GL_DEPTH_TEST);
	if (enablePostFX)
	{
		final_texture = postProcessing.Apply(scene_texture);
	}

	// 3. Render Final Texture to Screen
	GL::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GL::Viewport(0, 0, default_window_size.x, default_window_size.y);


	GL::UseProgram(screenShader.Shader);

	GL::ActiveTexture(GL_TEXTURE0);
	GL::BindTexture(GL_TEXTURE_2D, final_texture);

	if (screenShader.UniformLocations.count("uColorTexture"))
	{
		GL::Uniform1i(screenShader.UniformLocations.at("uColorTexture"), 0);
	}

	GL::BindVertexArray(screenVAO);
	GL::DrawArrays(GL_TRIANGLES, 0, screenVertexCount);

	GL::BindVertexArray(0);
	GL::BindTexture(GL_TEXTURE_2D, 0);
	GL::UseProgram(0);


	GL::Enable(GL_DEPTH_TEST);
	renderer_2d->EndScene();
}

void DemoDepthPost::DrawImGui()
{
	ImGui::Begin("Demo Depth & Post-Processing Controls");
	// Display FPS at the top
	ImGui::Text("FPS: %d", static_cast<int>(FPSTracker));
	ImGui::Separator();

	ImGui::SeparatorText("Depth Settings");

	if (ImGui::Button("Sort as Painters Algorithm"))
	{
		std::sort(std::begin(background_layers), std::end(background_layers), [](const BackGroundLayer& left, const BackGroundLayer& right) { return left.depth > right.depth; });
	}

	if (ImGui::Button("Sort as Front to Back"))
	{
		std::sort(
			std::begin(background_layers), std::end(background_layers),
			[](const BackGroundLayer& left, const BackGroundLayer& right)
			{
				return left.depth < right.depth; // then smaller depth drawn first, and frag of larger depth gonna be skipped over by depth test, and hopefully save effort of fragment shader
			});
	}

	if (ImGui::Button("Sort Randomly"))
	{
		std::random_device rd;
		std::mt19937	   g(rd());
		std::shuffle(std::begin(background_layers), std::end(background_layers), g);
	}
	ImGui::SeparatorText("Camera Scale");
	float previous_scale = static_cast<float>(scale);
	ImGui::SliderFloat("Zoom", &previous_scale, 0.1f, 10.0f);
	if (previous_scale != static_cast<float>(scale))
	{
		CS230::Camera* camera = GetGSComponent<CS230::Camera>();
		camera->SetScale({ static_cast<double>(previous_scale), static_cast<double>(previous_scale) });
		scale = static_cast<double>(previous_scale);
	}
	ImGui::SeparatorText("MSAA Settings");
	bool msaa_changed = ImGui::Checkbox("Enable MSAA", &useMSAA);

	ImGui::BeginDisabled(!useMSAA);


	static GLint max_msaa_samples = 0;
	if (max_msaa_samples == 0)
	{
		GL::GetIntegerv(GL_MAX_SAMPLES, &max_msaa_samples);
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
		if (valid_samples[i] == MSAASamples)
		{
			current_index = static_cast<int>(i);
			break;
		}
	}


	std::string preview = std::to_string(MSAASamples) + "x";
	if (ImGui::BeginCombo("MSAA Samples", preview.c_str()))
	{
		for (size_t i = 0; i < valid_samples.size(); ++i)
		{
			bool		is_selected = (current_index == static_cast<int>(i));
			std::string label		= std::to_string(valid_samples[i]) + "x";

			if (ImGui::Selectable(label.c_str(), is_selected))
			{
				MSAASamples	 = valid_samples[i];
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
		const auto use_msaa = useMSAA ? OffscreenFramebuffer::MSAA::True : OffscreenFramebuffer::MSAA::False;
		offscreenBuffer.SetMSAA(use_msaa, MSAASamples);

		MSAASamples = offscreenBuffer.GetMSAASamples();

		if (useMSAA)
		{
			GL::Enable(GL_MULTISAMPLE);
		}
		else
		{
			GL::Disable(GL_MULTISAMPLE);
		}
	}

	ImGui::SeparatorText("Post-Processing Effects");
	ImGui::Checkbox("Enable Post-FX", &enablePostFX);

	ImGui::BeginDisabled(!enablePostFX);


	if (auto* box_blur = postProcessing.GetEffect("Box Blur"))
	{
		ImGui::Checkbox(box_blur->Name.c_str(), reinterpret_cast<bool*>(&box_blur->Enabled));

		ImGui::BeginDisabled(box_blur->Enabled == PostProcessingEffect::Enable::False);
		ImGui::Indent();
		ImGui::SliderFloat("Blur Size", &boxBlurSize, 0.0f, 10.0f);
		ImGui::SliderFloat("Blur Separation", &boxBlurSeparation, 1.0f, 5.0f);
		ImGui::Unindent();
		ImGui::EndDisabled();
	}


	if (auto* gamma = postProcessing.GetEffect("Gamma Correction"))
	{
		ImGui::Checkbox(gamma->Name.c_str(), reinterpret_cast<bool*>(&gamma->Enabled));

		ImGui::BeginDisabled(gamma->Enabled == PostProcessingEffect::Enable::False);
		ImGui::Indent();
		ImGui::SliderFloat("Gamma", &gammaValue, 0.5f, 4.0f);
		ImGui::Unindent();
		ImGui::EndDisabled();
	}


	if (auto* chroma = postProcessing.GetEffect("Chromatic Aberration"))
	{
		ImGui::Checkbox(chroma->Name.c_str(), reinterpret_cast<bool*>(&chroma->Enabled));

		ImGui::BeginDisabled(chroma->Enabled == PostProcessingEffect::Enable::False);
		ImGui::Indent();
		ImGui::SliderFloat("Focus X", &chromaticAberrationMouseX, 0.0f, 1.0f);
		ImGui::SliderFloat("Focus Y", &chromaticAberrationMouseY, 0.0f, 1.0f);
		ImGui::Unindent();
		ImGui::EndDisabled();
	}

	if (auto* pixelization = postProcessing.GetEffect("Pixelization"))
	{
		ImGui::Checkbox(pixelization->Name.c_str(), reinterpret_cast<bool*>(&pixelization->Enabled));

		ImGui::BeginDisabled(pixelization->Enabled == PostProcessingEffect::Enable::False);
		ImGui::SliderInt("Pixel Size", &pixelSize, 1, 800);
		if ((pixelSize & 1) == 0)
		{
			pixelSize += 1; // make sure it's odd
		}
		// ImGui::Indent();
		// ImGui::Unindent();
		ImGui::EndDisabled();
	}

	ImGui::EndDisabled();

	ImGui::SeparatorText("Render Textures");


	static int		   selected_effect_index = -1;
	static const char* effect_names[]		 = { "Box Blur", "Gamma Correction", "Chromatic Aberration", "Pixelization" };
	static const char* current_effect_name	 = "None";

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


	if (enablePostFX && selected_effect_index >= 0 && selected_effect_index < 4)
	{
		if (auto* effect = postProcessing.GetEffect(effect_names[selected_effect_index]))
		{
			if (effect->Enabled == PostProcessingEffect::Enable::True && effect->Framebuffer)
			{
				ImGui::Text("Output Texture (%s):", effect->Name.c_str());
				OpenGL::TextureHandle texture = effect->Framebuffer->GetTexture();

				const float		aspect_ratio   = static_cast<float>(default_window_size.x) / static_cast<float>(default_window_size.y);
				constexpr float display_width  = 400.0f;
				const float		display_height = display_width / aspect_ratio;

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
