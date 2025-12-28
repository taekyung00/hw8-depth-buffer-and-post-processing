/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Spring
 * \par CS230
 * \copyright DigiPen Institute of Technology
 */
#pragma once
#include "../../../Engine/GameState.h"
#include "../../../Engine/Engine.h"
#include "../../../Engine/Texture.h"

class NormalSandwich : public CS230::GameState
{
public:
    NormalSandwich()
    {
    }

    void          Load() override;
    void          Update(double dt) override;
    void          Unload() override;
    void          Draw() override;
    void          DrawImGui() override { };

    gsl::czstring GetName() const override
    {
        return "NormalSandwich";
    }

private:
    std::shared_ptr<CS230::Texture> texture;
	static constexpr int			default_width  = 600;
	static constexpr int			default_height = 800;
};