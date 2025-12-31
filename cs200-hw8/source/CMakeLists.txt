# author Rudy Castan
# date 2025 Fall
# CS200 Computer Graphics I
# copyright DigiPen Institute of Technology

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(SOURCE_CODE
    Engine/Engine.h Engine/Engine.cpp
    Engine/Error.h
    Engine/Font.h Engine/Font.cpp
    Engine/Fonts.h
    Engine/FPS.h
    Engine/GameState.h
    Engine/GameStateManager.h Engine/GameStateManager.cpp
    Engine/Input.h Engine/Input.cpp
    Engine/Logger.h Engine/Logger.cpp
    Engine/Matrix.h Engine/Matrix.cpp
    Engine/Path.h Engine/Path.cpp
    Engine/Random.h Engine/Random.cpp
    Engine/Rect.h
    Engine/Texture.h Engine/Texture.cpp
    Engine/TextureManager.h Engine/TextureManager.cpp
    Engine/TextManager.h Engine/TextManager.cpp
    Engine/Timer.h
    Engine/Vec2.h Engine/Vec2.cpp
    Engine/Window.h Engine/Window.cpp
    Engine/Animation.cpp Engine/Animation.h
    Engine/Camera.cpp Engine/Camera.h
    Engine/Collision.cpp Engine/Collision.h
    Engine/Component.h
    Engine/ComponentManager.h
    Engine/GameObject.cpp Engine/GameObject.h
    Engine/GameObjectManager.cpp Engine/GameObjectManager.h
    Engine/Particle.cpp Engine/Particle.h
    Engine/ShowCollision.cpp Engine/ShowCollision.h
    Engine/Sprite.cpp Engine/Sprite.h

    OpenGL/Buffer.h OpenGL/Buffer.cpp
    OpenGL/Environment.h
    OpenGL/Framebuffer.h OpenGL/Framebuffer.cpp
    OpenGL/GL.cpp OpenGL/GL.h
    OpenGL/GLConstants.h
    OpenGL/GLTypes.h
    OpenGL/Handle.h
    OpenGL/Shader.cpp OpenGL/Shader.h
    OpenGL/Texture.h OpenGL/Texture.cpp
    OpenGL/VertexArray.cpp OpenGL/VertexArray.h

    CS200/Image.h CS200/Image.cpp
    CS200/ImGuiHelper.h CS200/ImGuiHelper.cpp
    CS200/ImmediateRenderer2D.h CS200/ImmediateRenderer2D.cpp
    CS200/BatchRenderer2D.h CS200/BatchRenderer2D.cpp
    CS200/InstancedRenderer2D.h CS200/InstancedRenderer2D.cpp
    CS200/IRenderer2D.h
    CS200/NDC.h
    CS200/Renderer2DUtils.h CS200/Renderer2DUtils.cpp
    CS200/RenderingAPI.h CS200/RenderingAPI.cpp
    CS200/RGBA.h
    CS200/Shape.h CS200/Shape.cpp
    CS200/PostProcessingPipeline.h CS200/PostProcessingPipeline.cpp
    CS200/OffscreenFramebuffer.h CS200/OffscreenFramebuffer.cpp

    Demo/DemoDepthPost.h Demo/DemoDepthPost.cpp

    Game/Background.h Game/Background.cpp
    Game/GameObjectTypes.h
    Game/MainMenu.h Game/MainMenu.cpp
    Game/Particles.h
    Game/Score.h Game/Score.cpp
    Game/Splash.h Game/Splash.cpp
    Game/States.h

    main.cpp
)

add_executable(engine_porting ${SOURCE_CODE})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_CODE})

target_link_libraries(engine_porting PRIVATE project_options dependencies)
target_include_directories(engine_porting PRIVATE .)

# Check the IS_DEVELOPER_VERSION cache variable
# This is set by the cmake configure preset
if (IS_DEVELOPER_VERSION)
    target_compile_definitions(engine_porting PRIVATE DEVELOPER_VERSION)
endif()

if(EMSCRIPTEN)

    # https://emscripten.org/docs/tools_reference/settings_reference.html
    # ASSERTIONS=1                  - we want asserts to work
    # WASM=1                        - we want web assembly generated rather than just javascript
    # DISABLE_EXCEPTION_CATCHING=0  - we are using exceptions so let's not disable them
    # ALLOW_MEMORY_GROWTH=1         - we don't know exactly how much heap space we'll need so allow it to grow
    # EXIT_RUNTIME=1                - have exiting actually stop the program
    # SINGLE_FILE=1                 - generate everything into one html file
    # --embed-file                  - https://emscripten.org/docs/tools_reference/emcc.html#emcc-embed-file
    # --use-preload-cache           - help with faster reloads : https://emscripten.org/docs/compiling/Deploying-Pages.html#providing-a-quick-second-time-load
    # -lembind                      - to call c++ from javascript https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html
    # --shell-file                  - to customize the webpage https://emscripten.org/docs/compiling/Deploying-Pages.html#build-files-and-custom-shell
    target_link_options(engine_porting PRIVATE 
    -sASSERTIONS=$<$<BOOL:${IS_DEVELOPER_VERSION}>:1>$<$<NOT:$<BOOL:${IS_DEVELOPER_VERSION}>>:0>
    -sWASM=1 
    -sDISABLE_EXCEPTION_CATCHING=0 
    -sALLOW_MEMORY_GROWTH=1 
    -sEXIT_RUNTIME=1 
    -sSINGLE_FILE=1 
    --embed-file ${CMAKE_SOURCE_DIR}/Assets@/Assets
    --use-preload-cache
    -lembind
    --shell-file ${CMAKE_SOURCE_DIR}/app_resources/web/index_shell.html
    )

    set_target_properties(engine_porting PROPERTIES SUFFIX ".html")
elseif(WIN32)

    target_link_options(engine_porting PRIVATE 
        $<IF:$<BOOL:${IS_DEVELOPER_VERSION}>,/SUBSYSTEM:CONSOLE,/SUBSYSTEM:WINDOWS>
        /ENTRY:mainCRTStartup)

    set(ICON_RC ${CMAKE_CURRENT_BINARY_DIR}/app.rc)
    if(NOT EXISTS ${ICON_RC})
        set(ICON_FILE ${CMAKE_SOURCE_DIR}/app_resources/windows/icon.ico)
        file(WRITE ${ICON_RC} "IDR_MAINFRAME ICON \"${ICON_FILE}\"\nIDI_ICON1 ICON DISCARDABLE \"${ICON_FILE}\"")
    endif()

    target_sources(engine_porting PRIVATE ${ICON_RC})

endif()
