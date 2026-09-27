#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include "uniimage.h"
#include "univec.h"
#include "unicolor.h"
#include "unimath.h"

void drawPixel(Vec2 pos, Image& frame, Color color) {
	frame.set(pos, color);
}

void drawLine(Vec2 startPos, Vec2 endPos, Image& frame, Color color) {
	bool steep = std::abs(startPos.x - endPos.x) < std::abs(startPos.y - endPos.y);

	if (steep) {
		std::swap(startPos.x, startPos.y);
		std::swap(endPos.x, endPos.y);
	}
	if (startPos.x > endPos.x) {
		std::swap(startPos.x, endPos.x);
		std::swap(startPos.y, endPos.y);
	}
	int y = startPos.y;
	int ierror = 0;
	for (int x = startPos.x; x <= endPos.x; x++) {
		if (steep) {
			frame.set(Vec2{y, x}, color);
		}
		else {
			frame.set(Vec2{ x, y }, color);
		}
		ierror += 2 * std::abs(endPos.y - startPos.y);
		y += (endPos.y > startPos.y ? 1 : -1) * (ierror > endPos.x - startPos.x);
		ierror -= 2 * (endPos.x - startPos.x) * (ierror > endPos.x - startPos.x);
	}
}

void drawFillTriangle(Vec2 pointA, Vec2 pointB, Vec2 pointC, Image& frame, Color color) {
	// Сортируем точки по Y (от меньшего к большему)
	if (pointA.y > pointB.y) std::swap(pointA, pointB);
	if (pointA.y > pointC.y) std::swap(pointA, pointC);
	if (pointB.y > pointC.y) std::swap(pointB, pointC);

	// Функция для интерполяции X по Y
	auto interpolateX = [](const Vec2& p1, const Vec2& p2, float y) -> float {
		if (p1.y == p2.y) return p1.x;
		return p1.x + (p2.x - p1.x) * (y - p1.y) / (p2.y - p1.y);
	};

	// Отрисовка нижней/верхней части треугольника
	auto drawScanline = [&](float y, float xStart, float xEnd) {
		int x1 = (int)std::ceil(std::min(xStart, xEnd));
		int x2 = (int)std::floor(std::max(xStart, xEnd));
		int yInt = (int)y;

		if (yInt < 0 || yInt >= frame.width()) return;

		for (int x = x1; x <= x2; x++) {
			if (x >= 0 && x < frame.width()) {
				frame.set(Vec2{ x, yInt }, color);
			}
		}
	};

	// Если треугольник плоский
	if (pointA.y == pointC.y) return;

	// Точка на ребре AC на высоте pointB.y
	float xB_on_AC = interpolateX(pointA, pointC, pointB.y);

	// Определяем левую и правую границы
	float xLeft, xRight;
	if (pointB.x < xB_on_AC) {
		xLeft = pointB.x;
		xRight = xB_on_AC;
	}
	else {
		xLeft = xB_on_AC;
		xRight = pointB.x;
	}

	// Рисуем нижнюю часть (от A до B)
	int yStart = (int)std::ceil(pointA.y);
	int yEnd = (int)std::floor(pointB.y);

	for (int y = yStart; y <= yEnd; y++) {
		float x1 = interpolateX(pointA, pointB, (float)y);
		float x2 = interpolateX(pointA, pointC, (float)y);
		drawScanline((float)y, x1, x2);
	}

	// Рисуем верхнюю часть (от B до C)
	yStart = (int)std::ceil(pointB.y);
	yEnd = (int)std::floor(pointC.y);

	for (int y = yStart; y <= yEnd; y++) {
		float x1 = interpolateX(pointB, pointC, (float)y);
		float x2 = interpolateX(pointA, pointC, (float)y);
		drawScanline((float)y, x1, x2);
	}
}

void drawFillRectangle(Vec2 pos, int width, int height, Image& frame, Color color) {
	drawFillTriangle(pos, { pos.x + width, pos.y }, { pos.x, pos.y + height }, frame, color);
	drawFillTriangle({ pos + Vec2{width, height} }, { pos.x, pos.y + height }, { pos.x + width, pos.y }, frame, color);
}

void drawFillCircle(Vec2 pos, int r, Image& frame, Color color) {
	for (int y = -r; y <= r; ++y) {
		for (int x = -r; x <= r; ++x) {
			if (x * x + y * y <= r * r) {
				frame.set(Vec2{ pos.x + x, pos.y + y }, color);
			}
		}
	}
}

void drawFillRectangleRounded(fVec2 pos, int width, int height, float rn, Image& frame, Color color) {
	const float startX = pos.x - width;
	const float endX = pos.x + width;

	const float startY = pos.y - height;
	const float endY = pos.y + height;

	const float thk = 0.5;

	for (int y = (int)startY; y <= (int)endY; y++) {
		for (int x = (int)startX; x <= (int)endX; x++) {
			float nx = (float)(x - pos.x) / width;
			float ny = (float)(y - pos.y) / height;

			float val = pow(abs(nx), rn) + pow(abs(ny), rn);

			if (val >= 0.4f - thk && val <= 0.4f + thk) {
				frame.set(Vec2{ x, y }, color);
			}
		}
	}
}

void drawPreview(int count, Image& frame) {
	int centerX = frame.width() / 2;
	int centerY = frame.height() / 2;
	int radius = std::min(frame.width(), frame.height()) / 3;
	int circleRadius = radius / 2;

	Color colors[] = {
		colors::red, 
		colors::orange, 
		colors::yellow, 
		colors::green, 
		colors::blue, 
		colors::navy, 
		colors::purple, 
		colors::pink,
		colors::white
	};

	for (int i = 0; i < count; i++) {
		float angle = (2.0f * 3.14159f * i) / count;
		int x = centerX + (int)(radius * cos(angle));
		int y = centerY + (int)(radius * sin(angle));

		Color color = colors[i % 9];
		drawFillCircle(Vec2{ x, y }, circleRadius, frame, setAlfa(color, 0.9f));
	}
}