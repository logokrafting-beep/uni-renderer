#include "unigraphics.h"
#include "unirandom.h"

constexpr int WIDTH = 512;
constexpr int HEIGHT = 512;

// width:height - image size
// RGB - first pixel
// R+(different)/G+(different)/B+(different) - only if it is and if different lower than 50%
// ; - next pixel/next data
// - if different more than 50% write new pixel fully
// | - new line

void faze(const Image& frame, const std::string& fileName) {
    std::ofstream file(fileName);
    if (file.fail()) {
        Log("[FAZE]", " : Can't open file");
        return;
    }

    const int W = frame.width();
    const int H = frame.height();

    file << W << ":" << H << ";";

    int prevR = 0, prevG = 0, prevB = 0;

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {

            auto opt = frame.getPixel(Vec2{ x, y });
            if (!opt.has_value()) continue;
            const Color c = *opt;

            const int r = static_cast<int>(c.r);
            const int g = static_cast<int>(c.g);
            const int b = static_cast<int>(c.b);

            // Первый пиксель — полностью, без дельт
            if (x == 0 && y == 0) {
                file << r << "/" << g << "/" << b << ";";
                prevR = r; prevG = g; prevB = b;
                continue;
            }

            const int dR = r - prevR;
            const int dG = g - prevG;
            const int dB = b - prevB;

            // Ничего не изменилось — пустая запись между ';'
            if (dR == 0 && dG == 0 && dB == 0) {
                file << ";";
                continue;
            }

            std::ostringstream oss;

            auto writeChannel = [&](char name, int delta, int value) {
                if (delta == 0) return;                 // канал не изменился
                if (delta > 127 || delta < -127) {
                    oss << name << value << "/";        // изменение > 50% — новое значение
                }
                else {
                    oss << name
                        << (delta > 0 ? "+" : "")       // минус уже в числе
                        << delta << "/";
                }
            };

            writeChannel('R', dR, r);
            writeChannel('G', dG, g);
            writeChannel('B', dB, b);

            std::string s = oss.str();
            if (!s.empty()) s.pop_back();               // убрать хвостовой '/'
            file << s << ";";

            prevR = r; prevG = g; prevB = b;
        }
        file << "|";                                     // конец строки
    }

    Log("[FAZE]", " : Save done");
}

Image fazeLoad(const std::string& fileName) {
    Log("[FAZE]", " : Loading");

    std::ifstream file(fileName);
    if (!file) {
        throw std::runtime_error("Can't open file: " + fileName);
    }

    // ---- 1. Читаем заголовок "W:H;" ----
    int W = 0, H = 0;
    char ch = 0;

    if (!(file >> W)) throw std::runtime_error("Faze: bad width");
    if (!(file >> ch) || ch != ':') throw std::runtime_error("Faze: expected ':'");
    if (!(file >> H)) throw std::runtime_error("Faze: bad height");
    if (!(file >> ch) || ch != ';') throw std::runtime_error("Faze: expected ';' after header");

    if (W <= 0 || H <= 0) throw std::runtime_error("Faze: bad dimensions");

    Image img(W, H, RenderMode::RGB);

    int prevR = 0, prevG = 0, prevB = 0;

    // ---- 2. Читаем поток пикселей ----
    // Идём посимвольно, собирая "токен" между ';'.
    // '|' — конец строки, переводит x в 0, y++.
    // Пустой токен — пиксель не изменился.
    int x = 0, y = 0;
    bool firstPixel = true;
    std::string token;

    auto flushPixel = [&]() {
        // token уже накоплен. Разбираем его.
        // Если пусто — пиксель как предыдущий.
        if (!token.empty()) {
            // Токен вида "R+3/G-5/B+10" или "R255/G128/B0"
            size_t pos = 0;
            while (pos < token.size()) {
                // Ищем следующий '/' или конец
                size_t slash = token.find('/', pos);
                std::string part = token.substr(
                    pos,
                    slash == std::string::npos ? std::string::npos : slash - pos
                );

                if (part.size() >= 2) {
                    char chName = part[0];   // 'R' / 'G' / 'B'
                    std::string num = part.substr(1);

                    int value = 0;
                    if (num[0] == '+' || num[0] == '-') {
                        // Дельта: прибавляем к предыдущему
                        value = std::stoi(num);
                        int* target = nullptr;
                        if (chName == 'R') target = &prevR;
                        else if (chName == 'G') target = &prevG;
                        else if (chName == 'B') target = &prevB;

                        if (target) {
                            int v = *target + value;
                            if (v < 0) v = 0;
                            if (v > 255) v = 255;
                            *target = v;
                        }
                    }
                    else {
                        // Абсолютное значение канала
                        value = std::stoi(num);
                        if (value < 0) value = 0;
                        if (value > 255) value = 255;
                        if (chName == 'R') prevR = value;
                        else if (chName == 'G') prevG = value;
                        else if (chName == 'B') prevB = value;
                    }
                }

                if (slash == std::string::npos) break;
                pos = slash + 1;
            }
        }

        // Пишем пиксель
        if (x < W && y < H) {
            Color c{
                static_cast<uint8_t>(prevR),
                static_cast<uint8_t>(prevG),
                static_cast<uint8_t>(prevB),
                255
            };
            img.set(x, y, c);
        }
        ++x;
    };

    char c;
    while (file.get(c)) {
        if (c == ';') {
            flushPixel();
            token.clear();
        }
        else if (c == '|') {
            // Конец строки
            x = 0;
            ++y;
        }
        else if (c == '\n' || c == '\r' || c == ' ' || c == '\t') {
            // Игнорируем whitespace (если кто-то форматировал файл руками)
            continue;
        }
        else {
            token.push_back(c);
        }
    }

    // Если файл не заканчивается ';', но токен остался — дописываем
    if (!token.empty()) {
        flushPixel();
    }

    Log("[FAZE]", " : Load done");
    return img;
}

int main() {
	Image frame(WIDTH, HEIGHT, RenderMode::RGBA);

    //frame = fazeLoad("frame.fz");

    ///*
    drawFillTriangle({WIDTH / 2, 0}, {0, HEIGHT}, {WIDTH, HEIGHT}, frame, setAlfa(BLUE, 0.4f));
    drawFillRectangle({ 50, 50 }, WIDTH - 100, HEIGHT - 100, frame, setAlfa(BLUE, 0.75f));

    drawFillCircle({ WIDTH / 4, HEIGHT / 4 }, 50, frame, RED);
    drawFillCircle({ 348, HEIGHT / 4 }, 50, frame, GREEN);
    drawFillCircle({ WIDTH / 4, 348 }, 50, frame, PURPLE);
    drawFillCircle({ 348, 348 }, 50, frame, WHITE);

    drawFillRectangle({ WIDTH / 4, HEIGHT / 4 }, WIDTH / 2, HEIGHT / 2, frame, setAlfa(BLUE, 0.8f));
    drawFillCircle({ WIDTH / 2, HEIGHT / 2 }, 50, frame, setAlfa(RED, 0.7f));
    //*/
    
	frame.imageSave("frame.bmp");

    //faze(frame, "frame.fz");
	
	return 0;
}