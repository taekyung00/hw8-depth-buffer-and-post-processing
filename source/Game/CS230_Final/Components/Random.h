/**
 * \file
 * \author Taekyung Ho
 * \date 2025 Spring
 * \par CS230
 * \copyright DigiPen Institute of Technology
 */
#pragma once
#include <random>
#include <vector>

#include "../../../Engine/Engine.h"
#include "../../../Engine/Component.h"

class Random : public CS230::Component {
public:
	Random() :gen(rd()){}

	int PickRandomIndex(size_t amount, bool same_probability, const std::vector<double>& probability_array = {});

private:
	std::random_device rd;
	std::mt19937 gen;
};

