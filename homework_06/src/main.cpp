#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cmath>
#include <cstdint>

constexpr float M_GI = 9.81; // прискорення вільного падіння

// трохи занесло, не стосується ДЗ
constexpr uint8_t DEBUG_TO_OUTPUT_FILE = 1;
void errorOutput(std::ofstream& file, const char* msg, const uint8_t debugEnabled = DEBUG_TO_OUTPUT_FILE) {
    if (debugEnabled) {
        file << "error: " << msg << std::endl;
    } else {
        file << std::endl;
    }

    std::cerr << msg << std::endl;
}

int main() {
    std::ifstream inputFile("input.txt");
    if (!inputFile) {
        std::cerr << "Failed to open file" << std::endl;
        return 1;
    }

    std::ofstream outputFile("output.txt");
    if (!outputFile) {
        std::cerr << "Failed to create/open output file" << std::endl;
        return 1;
    }

    // Додав порядкове зчитування виключно для зручності тестування
    // ...потім пожалкував
    char line[64];
    int lineNum = 0,
        successCount = 0;
    while (inputFile.getline(line, 64)) {
        ++lineNum;

        // використовую CLion на Win11 з toolchain на WSL
        // тому довелось хендлити переноси строк
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\r') {
            line[len - 1] = '\0';
        }

        if (line[0] == '\0') {
            outputFile << std::endl; // для збереження "структури"
            continue;
        }

        std::cout << std::endl << "Input: " << line << std::endl;

        float xd{}, yd{}, zd{};
        float targetX{}, targetY{};
        float attackSpeed{};
        float accelerationPath{};
        char ammoName[15]{};

        std::istringstream lineStream(line);
        lineStream >> xd >> yd >> zd >> targetX >> targetY >> attackSpeed >> accelerationPath >> ammoName;

        //
        // START: Валідація даних
        //
        if (attackSpeed < 0 || attackSpeed > 60) { // навряд скоро будуть такі дрони
            errorOutput(outputFile, "Attack speed out of range");
            continue; // return 1;
        }

        if (zd < 10) { // з міркувань безпеки самого дрону та точності розрахунків
            errorOutput(outputFile, "Drone too low");
            continue;
        }

        if (accelerationPath < 0) {
            errorOutput(outputFile, "Acceleration path out of range");
            continue;
        }

        float ammo_m, ammo_d, ammo_l;
        if (strcmp(ammoName, "VOG-17") == 0) {
            ammo_m = 0.35f;
            ammo_d = 0.07f;
            ammo_l = 0.0f;
        } else if (strcmp(ammoName, "M67") == 0) {
            ammo_m = 0.6f;
            ammo_d = 0.1f;
            ammo_l = 0.0f;
        } else if (strcmp(ammoName, "RKG-3") == 0) {
            ammo_m = 1.2f;
            ammo_d = 0.1f;
            ammo_l = 0.0f;
        } else if (strcmp(ammoName, "GLIDING-VOG") == 0) {
            ammo_m = 0.45f;
            ammo_d = 0.1f;
            ammo_l = 1.0f;
        } else if (strcmp(ammoName, "GLIDING-RKG") == 0) {
            ammo_m = 1.4f;
            ammo_d = 0.1f;
            ammo_l = 1.0f;
        } else {
            errorOutput(outputFile, "Unknown ammo");
            continue;
        }
        //
        // END: Валідація даних

        std::cout << "xd=" << xd << ", yd=" << yd << ", zd=" << zd << std::endl
            << "targetX=" << targetX << ", targetY=" << targetY << std::endl
            << "attackSpeed=" << attackSpeed << ", accelerationPath=" << accelerationPath << std::endl
            << "ammoName=" << ammoName << std::endl
            << "ammo_m=" << ammo_m << ",ammo_d=" << ammo_d << ",ammo_l=" << ammo_l << std::endl
        ;

        //
        // START: 5.2. Розв'язок кубічного рівняння (метод Кардано)
        //

        // a = d·g·m − 2d²·l·V₀
        // b = −3g·m² + 3d·l·m·V₀
        // c = 6m²·Z₀
        float m2 = ammo_m * ammo_m; // для спрощення запису
        float coef_a{ammo_d * M_GI * ammo_m};
        float coef_b{-3.0f * M_GI * m2};
        float coef_c{6.0f * m2 * zd};
        if (ammo_l != 0.0f) { // спрощення формули при l=0
            coef_a -= 2.0f * ammo_d * ammo_d * ammo_l * attackSpeed;
            coef_b += 3.0f * ammo_d * ammo_l * ammo_m * attackSpeed;
        }
        std::cout << "coef_a=" << coef_a << ", coef_b=" << coef_b << ", coef_c=" << coef_c << std::endl;

        // p = − b² / (3a²)
        float coef_p = -1.0f * (coef_b * coef_b / (3.0f * coef_a * coef_a));
        std::cout << "coef_p=" << coef_p;
        if (coef_p >= 0) {
            errorOutput(outputFile, "p >= 0 is not supported");
            continue;
        }

        // q = 2b³ / (27a³) + c / a
        float coef_q = (2.0f * coef_b * coef_b * coef_b) / (27.0f * coef_a * coef_a * coef_a) + coef_c / coef_a;
        std::cout << ", coef_q=" << coef_q << std::endl;

        // φ = arccos( 3q / (2p) · √(−3/p) )
        float arg_arc = 3.0f * coef_q / (2.0f * coef_p) * std::sqrt(-3.0f / coef_p);
        std::cout << "arg_arc=" << arg_arc;
        if (arg_arc < -1.0f || arg_arc > 1.0f) {
            errorOutput(outputFile, "Simplified model does not work at high altitudes");
            continue;
        }

        float phi = std::acos(arg_arc);
        std::cout << ", phi=" << phi << std::endl;

        // t = 2√(−p/3) · cos( (φ + 4π) / 3 ) − b / (3a)
        // десь вичитав що в ДЗ1 для обчислення достатньо float, а M_PI - double, тому каст у float,
        float fallTime = 2.0f * std::sqrt(-coef_p / 3.0f) * std::cos((phi + static_cast<float>(4.0f * M_PI)) / 3.0f) - coef_b / (3.0f * coef_a);
        std::cout << "fallTime=" << fallTime << std::endl;
        if (fallTime <= 0.0f) {
            errorOutput(outputFile, "fallTime <= 0.0f");
            continue;
        }
        //
        // END: 5.2

        //
        // START: 5.3. Горизонтальна дистанція польоту
        //

        // h = V₀t
        //   − t²d·V₀/(2m)
        //   + t³(6d·g·l·m − 6d²(l²-1)·V₀)/(36m²)
        //   + t⁴ (−6d²g·l·(1+l²+l⁴)m + 3d³l²(1+l²)V₀ + 6d³l⁴(1+l²)V₀) / (36(1+l²)²m³)
        //   + t⁵(3d³g·l³m − 3d⁴l²(1+l²)V₀) / (36(1+l²)m⁴)
        float t2 = fallTime * fallTime, // для спрощення запису рівняння
            d2 = ammo_d * ammo_d,
            l2 = ammo_l * ammo_l;
        float flightDistance = attackSpeed * fallTime
            - t2 * ammo_d * attackSpeed / (2.0f * ammo_m)
            + t2 * fallTime * (6.0f * ammo_d * M_GI * ammo_l * ammo_m - 6.0f * d2 * (l2 - 1.0f) * attackSpeed) / (36.0f * m2);
        if (ammo_l != 0.0f) { // спрощення формули при l=0
            float l2p1 = l2 + 1.0f;
            flightDistance += t2 * t2 * (-6.0f * d2 * M_GI * ammo_l * (l2p1 + l2 * l2) * ammo_m + 3.0f * d2 * ammo_d * l2 * l2p1 * attackSpeed + 6.0f * d2 * ammo_d * l2 * l2 * l2p1 * attackSpeed) / (36.0f * l2p1 * l2p1 * m2 * ammo_m)
                + t2 * t2 * fallTime * (3.0f * d2 * ammo_d * M_GI * l2 * ammo_l * ammo_m - 3.0f * d2 * d2 * l2 * l2p1 * attackSpeed) / (36.0f * l2p1 * m2 * m2);
        }
        std::cout << "flightDistance=" << flightDistance << std::endl;
        if (flightDistance <= 0.0f) {
            errorOutput(outputFile, "flightDistance <= 0.0f");
            continue;
        }
        //
        // END: 5.3

        //
        // START: 5.4. Визначення точки скиду
        //

        // Крок 1. Відстань від дрона до цілі:
        // D = √((targetX − xd)² + (targetY − yd)²)
        float targetDistance{std::sqrt(std::pow(targetX - xd, 2.0f) + std::pow(targetY - yd, 2.0f))};
        std::cout << "targetDistance=" << targetDistance << std::endl;

        // Крок 2. Перевірка необхідності маневру:
        // Якщо h + accelerationPath > D, дрону потрібно відлетіти далі від цілі перед атакою.
        // > 3. Там є ділення на відстань від дрона до цілі. Якщо вона нуль - знайдіть інший можливий розв'язок)
        uint8_t isManeuverNeeded{1};
        float accelerateDistance = flightDistance + accelerationPath;
        if (targetDistance == 0.0f) {
            // відлітаємо по осі X на потрібну дистанцію
            // тут можна одразу порахувати fireX, fireY, щоб не дублювати код запису intermediate point зробив так
            // звісно можна оптимізувати
            xd -= accelerateDistance;
        } else if (accelerateDistance > targetDistance) {
            // xd' = targetX − (targetX − xd) · (h + accelerationPath) / D
            // yd' = targetY − (targetY − yd) · (h + accelerationPath) / D
            float ratio = accelerateDistance / targetDistance;
            xd = targetX - (targetX - xd) * ratio;
            yd = targetY - (targetY - yd) * ratio;
        } else {
            isManeuverNeeded = 0;
        }

        if (isManeuverNeeded) {
            std::cout << "ipX=" << xd << ", ipY=" << yd << std::endl;

            targetDistance = std::sqrt(std::pow(targetX - xd, 2.0f) + std::pow(targetY - yd, 2.0f));
            std::cout << "newTargetDistance=" << targetDistance << std::endl;

            outputFile << xd << " " << yd  << " ";
        }

        // Крок 3. Координати точки скиду:
        // ratio = (D − h) / D
        // fireX = xd + (targetX − xd) · ratio
        // fireY = yd + (targetY − yd) · ratio
        float ratio = (targetDistance - flightDistance) / targetDistance;
        float fireX{xd + (targetX - xd) * ratio};
        float fireY{yd + (targetY - yd) * ratio};
        std::cout << "ratio=" << ratio << ",fireX=" << fireX << ", fireY=" << fireY << std::endl;
        //
        // END: 5.4

        outputFile << fireX << " " << fireY << std::endl;
        ++successCount;
    }

    inputFile.close();
    outputFile.close();

    if (lineNum <= 0) {
        std::cerr << "Empty \"input.txt\"" << std::endl;

        return 1;
    }

    if (successCount == 0) {
        std::cerr << "No valid input data" << std::endl;

        return 1;
    }

    return 0;
}

