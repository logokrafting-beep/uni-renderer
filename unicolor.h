#pragma once

struct Color {
	int r, g, b, a;
};

Color blendColors(Color top, Color bottom) {
    // Если верхний полностью прозрачный — возвращаем нижний
    if (top.a == 0) return bottom;
    // Если верхний полностью непрозрачный — возвращаем верхний
    if (top.a == 255) return top;

    float alpha = top.a / 255.0f;
    float invAlpha = 1.0f - alpha;

    Color result;
    result.r = static_cast<int>(top.r * alpha + bottom.r * invAlpha + 0.5f);
    result.g = static_cast<int>(top.g * alpha + bottom.g * invAlpha + 0.5f);
    result.b = static_cast<int>(top.b * alpha + bottom.b * invAlpha + 0.5f);
    result.a = 255; // Итоговый пиксель всегда непрозрачный
    return result;
}

Color blendColorsInt(Color top, Color bottom) {
    int a = top.a;
    int invA = 255 - a;
    return {
        (top.r * a + bottom.r * invA) / 255,
        (top.g * a + bottom.g * invA) / 255,
        (top.b * a + bottom.b * invA) / 255,
        255
    };
}

Color setAlfa(Color color, float alfa) {
    int newAlpha = (int)(alfa * 255);
    if (newAlpha < 0) newAlpha = 0;
    if (newAlpha > 255) newAlpha = 255;
    return Color{ color.r, color.g, color.b, newAlpha };
}

namespace colors {
    // Базовые цвета
    constexpr Color white = { 255, 255, 255, 255 };
    constexpr Color black = { 0, 0, 0, 255 };
    constexpr Color red = { 255, 0, 0, 255 };
    constexpr Color green = { 0, 255, 0, 255 };
    constexpr Color blue = { 0, 0, 255, 255 };
    constexpr Color yellow = { 255, 255, 0, 255 };
    constexpr Color cyan = { 0, 255, 255, 255 };
    constexpr Color magenta = { 255, 0, 255, 255 };
    constexpr Color orange = { 255, 165, 0, 255 };
    constexpr Color purple = { 128, 0, 128, 255 };
    constexpr Color pink = { 255, 102, 176, 255 };
    constexpr Color gray = { 128, 128, 128, 255 };
    constexpr Color darkGray = { 64, 64, 64, 255 };
    constexpr Color lightGray = { 192, 192, 192, 255 };
    constexpr Color brown = { 165, 42, 42, 255 };
    constexpr Color gold = { 255, 215, 0, 255 };
    constexpr Color silver = { 192, 192, 192, 255 };
    constexpr Color maroon = { 128, 0, 0, 255 };
    constexpr Color olive = { 128, 128, 0, 255 };
    constexpr Color teal = { 0, 128, 128, 255 };
    constexpr Color navy = { 0, 0, 128, 255 };
    constexpr Color coral = { 255, 127, 80, 255 };
    constexpr Color tomato = { 255, 99, 71, 255 };
    constexpr Color salmon = { 250, 128, 114, 255 };
    constexpr Color skyBlue = { 135, 206, 235, 255 };
    constexpr Color lime = { 0, 255, 0, 255 };
    constexpr Color turquoise = { 64, 224, 208, 255 };
    constexpr Color violet = { 238, 130, 238, 255 };
    constexpr Color indigo = { 75, 0, 130, 255 };
    constexpr Color beige = { 245, 245, 220, 255 };
    constexpr Color mint = { 189, 252, 201, 255 };
    constexpr Color lavender = { 230, 230, 250, 255 };
}

// ============= МАКРОСЫ ЦВЕТОВ =============
// Базовые цвета
#define WHITE       Color{255, 255, 255, 255}
#define BLACK       Color{0, 0, 0, 255}
#define RED         Color{255, 0, 0, 255}
#define GREEN       Color{0, 255, 0, 255}
#define BLUE        Color{0, 0, 255, 255}
#define YELLOW      Color{255, 255, 0, 255}
#define CYAN        Color{0, 255, 255, 255}
#define MAGENTA     Color{255, 0, 255, 255}
#define ORANGE      Color{255, 165, 0, 255}
#define PURPLE      Color{128, 0, 128, 255}
#define PINK        Color{255, 192, 203, 255}
#define GRAY        Color{128, 128, 128, 255}
#define DARK_GRAY   Color{64, 64, 64, 255}
#define LIGHT_GRAY  Color{192, 192, 192, 255}
#define BROWN       Color{165, 42, 42, 255}
#define GOLD        Color{255, 215, 0, 255}
#define SILVER      Color{192, 192, 192, 255}