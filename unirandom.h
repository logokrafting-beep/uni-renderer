#pragma once
#include <random>
#include <limits>

#include "unicolor.h"

std::mt19937& randomEngine() {
	static thread_local std::mt19937 gen(std::random_device{}());
	return gen;
}

int randomNum() {
	std::uniform_int_distribution<int> dist;
	return dist(randomEngine());
}

int randomInt(int min, int max) {
	std::uniform_int_distribution<int> dist(min, max);
	return dist(randomEngine());
}

float randomFloat(float min, float max) {
	std::uniform_real_distribution<float> dist(min, max);
	return dist(randomEngine());
}

double randomDouble(double min, double max) {
	std::uniform_real_distribution<double> dist(min, max);
	return dist(randomEngine());
}

Color randomColor() {
	return { randomInt(0, 255), randomInt(0, 255), randomInt(0, 255), randomInt(0, 255) };
}