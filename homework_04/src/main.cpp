#define _USE_MATH_DEFINES
#include <cmath>
#include <fstream>
#include <iostream>

constexpr __uint16_t INPUT_LIMIT = 65535;

struct ImpulseDatum {
    long fl;
    long fr;
    long bl;
    long br;

    ImpulseDatum operator-(const ImpulseDatum &other) const {
        ImpulseDatum result{};
        result.fl = fl - other.fl;
        result.fr = fr - other.fr;
        result.bl = bl - other.bl;
        result.br = br - other.br;
        return result;
    }

};

// Model parameters:
//   ticks_per_revolution = 1024
//   wheel_radius_m       = 0.3
//   wheelbase_m          = 1.0
//
// Input: a text file with 5 whitespace-separated values per line:
//         timestamp_ms fl_ticks fr_ticks bl_ticks br_ticks
// Output: a table on stdout, starting from the second sample:
//         timestamp_ms x y theta
int main(int argc, char** argv) {
    // The program expects exactly one argument: a path to telemetry samples.
    if (argc != 2) {
        std::cerr << "usage: ugv_odometry <input_path>\n";
        return 1;
    }

    // ============================================================
    // 4. Вхідні дані
    // ============================================================

    // Шлях до файлу передається як argv[1]
    const char* filename = argv[1];
    std::ifstream inputFile(filename);
    if (!inputFile) {
        std::cerr << "Cannot open file: " << filename << '\n';
        return 1;
    }

    long prevTime{}; // NOLINT (disable CppTooWideScopeInitStatement)
    auto prevImpulse = ImpulseDatum{};

    // Текстовий файл, кожен рядок мiстить 5 чисел через пробiл
    // > базова перевірка першого рядка
    if (!(inputFile >> prevTime >> prevImpulse.fl >> prevImpulse.fr >> prevImpulse.bl >> prevImpulse.br)) {
        std::cerr << "Invalid input file\n";
        return 1;
    }

    // Параметри робота (жорстко задаються у кодi):
    constexpr int ticks_per_revolution = 1024;
    constexpr double wheel_radius_m = 0.3;
    constexpr double wheelbase_m = 1.0; // NOLINT (disable CppTooWideScope)
    // Одразу рахуємо відстань на один імпульс енкодера
    constexpr double distance_per_tick = 2.0 * M_PI * wheel_radius_m / ticks_per_revolution;

    // Позиція (Початкові значення: x = 0, y = 0, theta = 0.)
    double x{0};
    double y{0};
    double theta{0};

    // ============================================================
    // 5. Алгоритм
    // ============================================================
    __uint16_t curStep{0};
    long curTime{};
    auto curImpulse = ImpulseDatum{};
    // Читаємо далі увесь файл
    while (inputFile >> curTime >> curImpulse.fl >> curImpulse.fr >> curImpulse.bl >> curImpulse.br) {
        ++curStep;
        if (curStep >= INPUT_LIMIT) { // на всякий випадок
            std::cerr << "Input limit: " << INPUT_LIMIT << std::endl;
            return 1;
        }

        // Крок 1. Різниця імпульсів по кожному колесу:
        const auto [fl, fr, bl, br] = curImpulse - prevImpulse;

        // Крок 2. Усереднити борти (переднє i заднє колесо одного боку обертаються синхронно):
        const double d_left = static_cast<double>(fl + bl) / 2.0;
        const double d_right = static_cast<double>(fr + br) / 2.0;

        // Крок 3. Перевести імпульси у метри:
        const double dL = d_left * distance_per_tick;
        const double dR = d_right * distance_per_tick;

        // Крок 4. Скільки пройшов центр робота і на скільки повернувся:
        const double d = (dL + dR) / 2.0;
        const double dtheta = (dR - dL) / wheelbase_m;

        // Крок 5. Оновити позицію через усереднений напрямок на кроці:
        x += d * std::cos(theta + dtheta / 2.0);
        y += d * std::sin(theta + dtheta / 2.0);
        theta += dtheta;

        // ============================================================
        // 6. Вихідні дані
        // ============================================================

        // Текстовий вивід у стандартний вивід (stdout): один рядок на кожен крок.
        // Значення у рядку - через пробіл, у тому ж порядку як у вхідних даних: timestamp_ms x y theta
        std::cout << curTime << ' '
          << x << ' ' << y << ' '
          << theta << std::endl;

        prevImpulse = curImpulse;
    }

    return 0;
}
