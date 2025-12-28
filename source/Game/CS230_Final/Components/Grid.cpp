/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Spring
 * \par CS230
 * \copyright DigiPen Institute of Technology
 */
#include "Grid.h"
#include "../../../Engine/Input.h"
#include "../../../Engine/TextureManager.h"
#include "../../../Engine/Window.h"

Grid::Grid()
{
    window_size = Engine::GetWindow().GetSize();
    white_dot   = Engine::GetTextureManager().Load("Assets/images/CS230_Final/WhiteDot.png");
    black_dot   = Engine::GetTextureManager().Load("Assets/images/CS230_Final/BlackDot.png");
    red_dot     = Engine::GetTextureManager().Load("Assets/images/CS230_Final/RedDot.png");
    blue_dot    = Engine::GetTextureManager().Load("Assets/images/CS230_Final/BlueDot.png");
}

void Grid::Update([[maybe_unused]] double dt)
{
    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Tab))
    {
        draw = !draw;
    }
    if(window_size != Engine::GetWindow().GetSize()){
        window_size = Engine::GetWindow().GetSize();
    }
}

void Grid::Draw(DotColor dc,float depth)
{
    if (draw)
    {
        for (int i = 0; i <= window_size.y / 10; ++i)
        {
            for (int j = 0; j <= window_size.x / 10; ++j)
            {
                Math::TransformationMatrix draw_matrix = Math::TranslationMatrix(Math::ivec2{ j * 10, i * 10 }) /** Math::TranslationMatrix(-camera.GetPosition())*/;
                if (i == 0 && j == 0)
                {
                    blue_dot->Draw(draw_matrix * Math::ScaleMatrix(4.0),CS200::WHITE,depth);
                }
                else if (i == 0 || i == window_size.y / 10 || j == 0 || j == window_size.x / 10)
                {
					red_dot->Draw(draw_matrix * Math::ScaleMatrix(2.0), CS200::WHITE, depth);
                }
                else
                {
                    switch (dc)
                    {
                    case DotColor::white: white_dot->Draw(draw_matrix, CS200::WHITE, depth);
                        break;
                    
                    case DotColor::black: black_dot->Draw(draw_matrix, CS200::WHITE, depth);
                        break;
                    }
                    
                }
            }
        }
    }
}
