/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Spring
 * \par CS230
 * \copyright DigiPen Institute of Technology
 */
#pragma once
#include "../../../Engine/Engine.h"
#include "../../../Engine/Texture.h"
#include "../../../Engine/GameState.h"

class GoodSandwich : public CS230::GameState {
public:
    GoodSandwich(){}

    void Load() override;
    void Update(double dt) override;
    void Unload() override;
    void Draw() override;
    void DrawImGui() override { };

    gsl::czstring GetName() const override
    {
        return "Good Sandwich";
    }

private:
    std::shared_ptr<CS230::Texture> texture;
	static constexpr int			default_width  = 600;
	static constexpr int			default_height = 800;
};