#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include <stdexcept>
#include <sstream>
#include <cstdlib>

#include "unicolor.h"
#include "univec.h"
#include "unilog.h"

// здесь генерация, сохранение, загрузка и настройка пикселя.

// Режим рендера определяет, храним ли мы альфа-канал.
// На сохранение в BMP это не влияет (BMP всегда пишется как 24-бит BGR),
// но пригодится позже для логики смешивания цветов (blending) при отрисовке.
enum class RenderMode {
	RGB,
	RGBA,
	GRAYSCALE
};

class Image {
public:
	Image() = default;

	// Создаёт "холст" width x height, изначально залитый черным (0,0,0,0).
	Image(int width, int height, RenderMode mode = RenderMode::RGBA)
		: m_width(width), m_height(height), m_mode(mode),
		m_pixels(static_cast<size_t>(width)* static_cast<size_t>(height), Color{ 0, 0, 0, 0 })
	{
		Log("[IMAGE]", " : Initing");
	}

	inline int width() const { return m_width; }
	inline int height() const { return m_height; }
	inline RenderMode mode() const { return m_mode; }

	// Установить цвет пикселя. Координаты вне холста молча игнорируются -
	// это обычная практика в растеризации: отсекать (clip) то, что не попадает в кадр.
	inline void set(Vec2 pos, Color color) {
		if (!inBounds(pos)) return;

		size_t idx = index(pos);

		// Если альфа = 255 — просто заменяем
		if (color.a == 255) {
			m_pixels[idx] = (m_mode == RenderMode::GRAYSCALE) ? toGrayscale(color) : color;
		}
		// Если альфа = 0 — ничего не делаем
		else if (color.a == 0) {
			return;
		}
		// Иначе — смешиваем с существующим пикселем
		else {
			Color blended = blendColorsInt(color, m_pixels[idx]);
			m_pixels[idx] = (m_mode == RenderMode::GRAYSCALE) ? toGrayscale(blended) : blended;
		}
	}

	inline void set(int x, int y, Color color) {

		size_t idx = index(Vec2{ x, y });

		// Если альфа = 255 — просто заменяем
		if (color.a == 255) {
			m_pixels[idx] = (m_mode == RenderMode::GRAYSCALE) ? toGrayscale(color) : color;
		}
		// Если альфа = 0 — ничего не делаем
		else if (color.a == 0) {
			return;
		}
		// Иначе — смешиваем с существующим пикселем
		else {
			Color blended = blendColors(color, m_pixels[idx]);
			m_pixels[idx] = (m_mode == RenderMode::GRAYSCALE) ? toGrayscale(blended) : blended;
		}
	}

	// Прочитать цвет пикселя. Возвращает std::nullopt, если координаты вне холста.
	inline std::optional<Color> getPixel(Vec2 pos) const {
		if (!inBounds(pos)) return std::nullopt;
		return m_pixels[index(pos)];
	}

	// Сохранение в формате BMP.
	// Для RGB/RGBA пишем обычный 24-бит BGR (альфа BMP всё равно не хранит по-человечески).
	// Для GRAYSCALE пишем настоящий 8-битный индексированный BMP с палитрой оттенков серого -
	// это честный "чёрно-белый" формат, а не просто RGB со слипшимися каналами
	void imageSave(const std::string& filename) const {
		Log("[IMAGE]", " : Saving");
		if (m_mode == RenderMode::GRAYSCALE) {
			saveGrayscaleBMP(filename);
		}
		else {
			saveColorBMP(filename);
		}
		Log("[IMAGE]", " : Save done");
	}

private:
	// Заголовки BMP должны быть упакованы без выравнивания (padding),
	// так как формат жёстко фиксирует раскладку байт в файле.
#pragma pack(push, 1)
	struct BMPFileHeader {
		uint16_t fileType = 0x4D42; // "BM" в little-endian
		uint32_t fileSize = 0;
		uint16_t reserved1 = 0;
		uint16_t reserved2 = 0;
		uint32_t offsetData = 0;
	};

	struct BMPInfoHeader {
		uint32_t size = 0;
		int32_t width = 0;
		int32_t height = 0;
		uint16_t planes = 1;
		uint16_t bitCount = 24;
		uint32_t compression = 0;
		uint32_t sizeImage = 0;
		int32_t xPixelsPerMeter = 2835; // ~72 DPI, значение не критично
		int32_t yPixelsPerMeter = 2835;
		uint32_t colorsUsed = 0;
		uint32_t colorsImportant = 0;
	};

	struct FazeHeader {
		char     magic[4] = { 'F', 'A', 'Z', 'E' };  // сигнатура
		uint16_t version = 1;
		uint8_t  mode = 0;   // 0=RGB, 1=RGBA, 2=GRAY
		uint8_t  channels = 3;   // 3 или 4 или 1
		int32_t  width = 0;
		int32_t  height = 0;
	};
#pragma pack(pop)

	// Стандартная формула перцептивной яркости (luma, ITU-R BT.601).
	// Человеческий глаз воспринимает зелёный ярче красного, а красный ярче синего,
	// поэтому просто усреднять r,g,b нельзя - серый получится "неправильным" на вид.
	static Color toGrayscale(Color c) {
		int gray = static_cast<int>(0.299 * c.r + 0.587 * c.g + 0.114 * c.b + 0.5);
		if (gray < 0) gray = 0;
		if (gray > 255) gray = 255;
		return Color{ gray, gray, gray, c.a };
	}

	void saveColorBMP(const std::string& filename) const {
		// BMP хранит строки снизу вверх, и каждая строка должна быть выровнена на 4 байта.
		const int rowStride = (m_width * 3 + 3) & ~3; // округление вверх до кратного 4
		const int paddingBytes = rowStride - m_width * 3;

		BMPFileHeader fileHeader{};
		BMPInfoHeader infoHeader{};

		infoHeader.size = sizeof(BMPInfoHeader);
		infoHeader.width = m_width;
		infoHeader.height = m_height;
		infoHeader.bitCount = 24;
		infoHeader.sizeImage = static_cast<uint32_t>(rowStride) * static_cast<uint32_t>(m_height);

		fileHeader.offsetData = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
		fileHeader.fileSize = fileHeader.offsetData + infoHeader.sizeImage;

		std::ofstream file(filename, std::ios::binary);
		if (!file) {
			throw std::runtime_error("Can't open file: " + filename);
			Log("[IMAGE]", " : File can't open");
		}

		file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
		file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

		const uint8_t zeroPad[3] = { 0, 0, 0 };

		for (int y = m_height - 1; y >= 0; --y) { // снизу вверх
			for (int x = 0; x < m_width; ++x) {
				const Color& c = m_pixels[static_cast<size_t>(y) * m_width + x];
				const uint8_t bgr[3] = {
					static_cast<uint8_t>(c.b),
					static_cast<uint8_t>(c.g),
					static_cast<uint8_t>(c.r)
				};
				file.write(reinterpret_cast<const char*>(bgr), 3);
			}
			if (paddingBytes > 0) {
				file.write(reinterpret_cast<const char*>(zeroPad), paddingBytes);
			}
		}
	}

	void saveGrayscaleBMP(const std::string& filename) const {
		// 8 бит на пиксель = индекс в палитре (0..255), а не сам цвет напрямую.
		// Палитра из 256 записей BGRA, где запись i - это (i, i, i, 0) - ровно оттенок серого i.
		// Поэтому индекс пикселя можно взять прямо равным его яркости.
		const int rowStride = (m_width + 3) & ~3; // округление вверх до кратного 4
		const int paddingBytes = rowStride - m_width;
		const uint32_t paletteSize = 256 * 4; // 256 записей по 4 байта (B, G, R, reserved)

		BMPFileHeader fileHeader{};
		BMPInfoHeader infoHeader{};

		infoHeader.size = sizeof(BMPInfoHeader);
		infoHeader.width = m_width;
		infoHeader.height = m_height;
		infoHeader.bitCount = 8;
		infoHeader.colorsUsed = 256;
		infoHeader.colorsImportant = 256;
		infoHeader.sizeImage = static_cast<uint32_t>(rowStride) * static_cast<uint32_t>(m_height);

		fileHeader.offsetData = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + paletteSize;
		fileHeader.fileSize = fileHeader.offsetData + infoHeader.sizeImage;

		std::ofstream file(filename, std::ios::binary);
		if (!file) {
			throw std::runtime_error("Can't open file to write: " + filename);
			Log("[IMAGE]", " : File can't open to write");
		}

		file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
		file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

		// Палитра: 256 оттенков серого от чёрного (0,0,0) до белого (255,255,255).
		for (int i = 0; i < 256; ++i) {
			const uint8_t entry[4] = {
				static_cast<uint8_t>(i), // B
				static_cast<uint8_t>(i), // G
				static_cast<uint8_t>(i), // R
				0                        // reserved
			};
			file.write(reinterpret_cast<const char*>(entry), 4);
		}

		const uint8_t zeroPad[3] = { 0, 0, 0 };

		for (int y = m_height - 1; y >= 0; --y) { // снизу вверх
			for (int x = 0; x < m_width; ++x) {
				const Color& c = m_pixels[static_cast<size_t>(y) * m_width + x];
				// Пиксель мог быть сохранён ещё не в grayscale-режиме (например, режим
				// сменили уже после отрисовки), поэтому яркость на всякий случай
				// пересчитываем прямо здесь, а не полагаемся, что c.r==c.g==c.b.
				const uint8_t gray = static_cast<uint8_t>(toGrayscale(c).r);
				file.write(reinterpret_cast<const char*>(&gray), 1);
			}
			if (paddingBytes > 0) {
				file.write(reinterpret_cast<const char*>(zeroPad), paddingBytes);
			}
		}
	}

	bool inBounds(Vec2 pos) const {
		return pos.x >= 0 && pos.x < m_width && pos.y >= 0 && pos.y < m_height;
	}

	size_t index(Vec2 pos) const {
		return static_cast<size_t>(pos.y) * m_width + pos.x;
	}

	int m_width = 0;
	int m_height = 0;
	RenderMode m_mode = RenderMode::RGBA;
	std::vector<Color> m_pixels;
};