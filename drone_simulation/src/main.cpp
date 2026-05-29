#define _USE_MATH_DEFINES // NOLINT (disable CppPreprocessorUndefineMacro)
#include <cmath>
#include <fstream>
#include <iostream>
// To create better diagnostics messages, each JSON value needs a pointer to its parent value such that a global context (i.e., a path from
// the root value to the value that led to the exception) can be created. They can, however, be enabled by defining the preprocessor symbol
// JSON_DIAGNOSTICS to 1 before including json.hpp.
#define JSON_DIAGNOSTICS 1 // NOLINT (consider using a 'constexpr' constant )
#include "json.hpp"
using json = nlohmann::json;

#define HW_ENABLE_LOG 0 // NOLINT (consider using a 'constexpr' constant )
#define HW_ENABLE_DEBUG 0 // NOLINT (consider using a 'constexpr' constant )
#define HW_DEBUG_JSON_CONTENT 1 // NOLINT (consider using a 'constexpr' constant )

#if HW_ENABLE_LOG
#define LOG(msg) std::cout << "[LOG] " << msg << '\n'
#else
#define LOG(msg)
#endif

#if HW_ENABLE_DEBUG
#define DEBUG(msg) std::cout << "[DEBUG] " << msg << '\n'
#else
#define DEBUG(msg)
#endif

#define USE_OWN_CALCS_DEFAULT 1

constexpr float M_GI = 9.81;
constexpr uint8_t AMMO_NAME_SIZE = 32;

// ============================================================
// Стани дрона (enum)
// ============================================================
enum DroneState {
    DS_STOPPED = 0,
    DS_ACCELERATING = 1,
    DS_DECELERATING = 2,
    DS_TURNING = 3,
    DS_MOVING = 4,
};

// ============================================================
// 2. Структури
// ============================================================
struct Settings {
    // Налаштування
    const char *droneConfigFile;
    const char *ammoFile;
    const char *targetsFile;
    const char *outFile;
    const uint16_t maxSteps;
};

constexpr Settings settings = {
    .droneConfigFile="config.json",      // droneConfigFile
    .ammoFile="ammo.json",        // ammoFile
    .targetsFile="targets.json",     // targetsFile
    .outFile="simulation.json",  // outFile
    .maxSteps=10000               // maxSteps
};

// ============================================================
// 2.1 Coord
// ============================================================
// Координата у 2D (або 3D для дрона). Зберігає x, y (та опціонально z). Ця структура має перевантажені оператори (див. розділ 3).
// > z використовується тільки для одного підрахунку
struct Coord {
    float x;
    float y;

    // =========================================================
    // Структура Coord повинна підтримувати арифметичні операції
    // =========================================================

    // Додавання координат
    Coord operator+(const Coord &other) const
    {
        Coord result{};
        result.x = x + other.x;
        result.y = y + other.y;
        return result;
    }

    // Віднімання координат
    Coord operator-(const Coord &other) const
    {
        Coord result{};
        result.x = x - other.x;
        result.y = y - other.y;
        return result;
    }

    // Множення на скаляр
    Coord operator*(const float s) const
    {
        Coord result{};
        result.x = x * s;
        result.y = y * s;
        return result;
    }

    // Віднімання скаляр
    Coord operator-(const float s) const
    {
        Coord result{};
        result.x = x - s;
        result.y = y - s;
        return result;
    }

    // Ділення скаляр
    Coord operator/(const float s) const
    {
        Coord result{};
        result.x = x / s;
        result.y = y / s;
        return result;
    }
};

// ============================================================
// 2.2 AmmoParams - Параметри одного типу боєприпасу.
// ============================================================
struct AmmoParams {
    char name[AMMO_NAME_SIZE];
    float mass;  // маса (кг)
    float drag;  // коефіцієнт опору
    float lift;  // коефіцієнт підйому
};

// ============================================================
// 2.3 DroneConfig - Вхідні параметри дрона та симуляції (те, що раніше було набором окремих змінних).
// ============================================================
struct DroneConfig {
    Coord startPos;                 // початкова позиція (x, y)
    float altitude;                 // висота
    float initialDir;               // початковий напрямок (рад)
    float attackSpeed;              // швидкість атаки (м/с)
    float accelPath;                // шлях розгону (м)
    char ammoName[AMMO_NAME_SIZE];  // обрані боєприпаси
    float arrayTimeStep;            // крок часу масиву цілей
    float simTimeStep;              // крок симуляції ??
    float hitRadius;                // радіус влучення
    float angularSpeed;             // кутова швидкість (рад/с)
    float turnThreshold;            // поріг повороту (рад)

    float fTime;     // час польоту снаряда
    float hDist;     // дистанція польоту снаряда
    float stepTurn;  // кут повороту за час симуляції

    float accel;     // прискорення
    float curSpeed;  // поточна швидкість
};

// ============================================================
// 2.4 SimStep - Один крок симуляції для виведення.
// ============================================================
struct SimStep {
    Coord pos;              // позиція дрона
    float direction;        // напрямок (рад)
    int state;              // стан автомата (0-4)
    int targetIdx;          // індекс поточної цілі
    Coord dropPoint;        // точка скиду (куди летить дрон)
    Coord aimPoint;         // куди впаде бомба (якщо скинути зараз)
    Coord predictedTarget;  // прогнозована позиція цілі

    // float time; // час
    // int num; // крок
};

// ============================================================
// Визначення масиву балістичних даних
// ============================================================
struct Ballistics {
    Coord fireCoords;
    float fireDist;
    float tgtDist;
};

// ============================================================
// Відкриття вхідного файлу та перетворення у JSON об'єкт
// ============================================================
inline bool readInputJson(const char *filename, json &js)
{
    std::ifstream fa(filename);
    if (!fa.is_open()) {
        std::cerr << "Failed to open " << filename << " file" << '\n';
        return false;
    }

    json ja;
    try {
        fa >> js;
    }
    catch (json::exception &e) {
        std::cerr << "Failed to parse" << filename << ":" << '\n';
        std::cerr << e.what() << '\n';
        return false;
    }

#if HW_ENABLE_DEBUG && HW_DEBUG_JSON_CONTENT
    DEBUG(filename << " content:");
    std::cout << ja.dump(2) << '\n';
#endif

    fa.close();

    return true;
}

// ============================================================
// Визначення відстані між двома координатами
// ============================================================
inline float calculateDistance(const Coord targetCoords, const Coord currentCoords)
{
    return std::sqrt(std::pow(targetCoords.x - currentCoords.x, 2.0f) + std::pow(targetCoords.y - currentCoords.y, 2.0f));
}

// ============================================================
// Нормалізація кута
// ============================================================
float normalizeAngle(float angle)
{
    while (angle > M_PI) {
        angle -= 2.0f * M_PI;
    }

    while (angle < -M_PI) {
        angle += 2.0f * M_PI;
    }

    return angle;
}

// ============================================================
// Балістична задача - часу польоту (метод Кардано)
// ============================================================
bool calculateDroneBombFallTime(DroneConfig &drone, const AmmoParams &ammo)
{
    const float d = ammo.drag;
    const float m = ammo.mass;
    const float l = ammo.lift;

    // a = d·g·m − 2d²·l·V₀
    // b = −3g·m² + 3d·l·m·V₀
    // c = 6m²·Z₀
    float a{d * M_GI * m};
    float b{-3.0f * M_GI * m * m};
    const float c{6.0f * m * m * drone.altitude};
    if (l != 0.0f) {
        // спрощення формули при l=0
        a -= 2.0f * d * d * l * drone.attackSpeed;
        b += 3.0f * d * l * m * drone.attackSpeed;
    }

    // p = − b² / (3a²)
    const float p = -1.0f * (b * b / (3.0f * a * a));
    if (p >= 0) {
        return false;
    }

    // q = 2b³ / (27a³) + c / a
    const float q = (2.0f * b * b * b) / (27.0f * a * a * a) + c / a;

    // φ = arccos( 3q / (2p) · √(−3/p) )
    const float argArc = 3.0f * q / (2.0f * p) * std::sqrt(-3.0f / p);
    if (argArc < -1.0f || argArc > 1.0f) {
        return false;
    }
    const float phi = std::acos(argArc);

    // t = 2√(−p/3) · cos( (φ + 4π) / 3 ) − b / (3a)
    drone.fTime = 2.0f * std::sqrt(-p / 3.0f) * std::cos((phi + static_cast<float>(4.0f * M_PI)) / 3.0f) - b / (3.0f * a);
    if (drone.fTime <= 0.0f) {
        return false;
    }

    return true;
}

// ============================================================
// Балістична задача - горизонтальна дистанція (степеневий ряд до t⁵)
// ============================================================
bool calculateDroneBombFlightDistance(DroneConfig &drone, const AmmoParams &ammo)
{
    const float d = ammo.drag;
    const float m = ammo.mass;
    const float l = ammo.lift;

    const float t = drone.fTime;

    // h = V₀t
    //   − t²d·V₀/(2m)
    //   + t³(6d·g·l·m − 6d²(l²-1)·V₀)/(36m²)
    //   + t⁴ (−6d²g·l·(1+l²+l⁴)m + 3d³l²(1+l²)V₀ + 6d³l⁴(1+l²)V₀) / (36(1+l²)²m³)
    //   + t⁵(3d³g·l³m − 3d⁴l²(1+l²)V₀) / (36(1+l²)m⁴)
    const float t2{t * t},  // для спрощення запису рівняння
        m2{m * m}, d2{d * d}, l2{l * l};
    drone.hDist = drone.attackSpeed * t - t2 * d * drone.attackSpeed / (2.0f * m) +
                  t2 * t * (6.0f * d * M_GI * l * m - 6.0f * d2 * (l2 - 1.0f) * drone.attackSpeed) / (36.0f * m2);
    if (l != 0.0f) {
        // спрощення формули при l=0
        const float l2p1{l2 + 1.0f};
        drone.hDist +=
            t2 * t2 *
                (-6.0f * d2 * M_GI * l * (l2p1 + l2 * l2) * m + 3.0f * d2 * d * l2 * l2p1 * drone.attackSpeed +
                 6.0f * d2 * d * l2 * l2 * l2p1 * drone.attackSpeed) /
                (36.0f * l2p1 * l2p1 * m2 * m) +
            t2 * t2 * t * (3.0f * d2 * d * M_GI * l2 * l * m - 3.0f * d2 * d2 * l2 * l2p1 * drone.attackSpeed) / (36.0f * l2p1 * m2 * m2);
    }

    if (drone.hDist <= 0.0f) {
        return false;
    }

    return true;
}

// ============================================================
// Інтерполяція між кроками — лінійна
// ============================================================
void interpolateTarget(const float time, const DroneConfig &drone, const Coord *targetCoords, const uint8_t tgtCount, Coord &result)
{
    const uint8_t positionIndex = static_cast<uint8_t>(std::floor(time / drone.arrayTimeStep)) % tgtCount;
    const uint8_t nextPositionIndex = (positionIndex + 1) % tgtCount;
    const float frac = (time - static_cast<float>(positionIndex) * drone.arrayTimeStep) / drone.arrayTimeStep;

    result = targetCoords[positionIndex] + (targetCoords[nextPositionIndex] - targetCoords[positionIndex]) * frac;
}

// ============================================================
// Балістична задача - точка скиду
// ============================================================
void calculateBallistics(const Coord &targetCoords, const Coord &droneCoords, const DroneConfig &drone, Ballistics &result)
{
    result.tgtDist = calculateDistance(targetCoords, droneCoords);
    if (std::fabs(result.tgtDist) < 1e-6f) {
        result.tgtDist = 1e-6f;
    }

    const float dxT = targetCoords.x - droneCoords.x;
    const float dyT = targetCoords.y - droneCoords.y;

    result.fireCoords.x = targetCoords.x - dxT / result.tgtDist * drone.hDist;
    result.fireCoords.y = targetCoords.y - dyT / result.tgtDist * drone.hDist;
    result.fireDist = calculateDistance(result.fireCoords, droneCoords);
}

// ============================================================
// Розрахунок орієнтовного часу прильоту дрона до точки скиду (totalTime)
// також вираховує напрямок до цілі,
// враховує "timeToStop" та час розгону залежно від поточної швидкості дрона
// ============================================================
void calculateDirAndTimeToFire(
    const Ballistics &ballistics, const SimStep &step, const DroneConfig &drone, float &resultDir, float &resultTime)
{
    // Якщо ціль занадто близько
    bool isTurnAdded{false};

    if (ballistics.tgtDist < drone.hDist) {
        // враховуємо час відльоту і розвороту
        resultTime = (2 * drone.hDist - ballistics.tgtDist) / drone.attackSpeed;
        resultTime += static_cast<float>(M_PI) / drone.angularSpeed;
        isTurnAdded = true;
    }
    else {
        resultTime = ballistics.fireDist / drone.attackSpeed;
    }

    // час польоту боєприпасу
    resultTime += drone.fTime;

    const float dx = ballistics.fireCoords.x - step.pos.x;
    const float dy = ballistics.fireCoords.y - step.pos.y;
    resultDir = std::atan2(dy, dx);
    const float deltaAngle = normalizeAngle(resultDir - step.direction);
    // Якщо кут між поточним напрямком і новим напрямком > turnThreshold:
    if (std::fabs(deltaAngle) > drone.turnThreshold) {
        // 5. Дрон гальмує (шлях гальмування = accelerationPath)
        if (drone.curSpeed > 0.0f) {
            resultTime += drone.curSpeed / drone.accel;
        }

        // 6. Повертається на місці. Час повороту = |deltaAngle| / angularSpeed
        if (!isTurnAdded) {
            resultTime += std::fabs(deltaAngle) / drone.angularSpeed;
        }

        // 7. Розганяється у новому напрямку
        resultTime += drone.attackSpeed / drone.accel;
    }
    else if (drone.curSpeed < drone.attackSpeed) {
        // час розгону
        resultTime += (drone.attackSpeed - drone.curSpeed) / drone.accel;
    }
}

// ============================================================
// Хелпери для парсингу структур - Simplify your life with macros
// ============================================================
// This function is usually called by the get() function of the basic_json class (either explicitly or via the conversion operators).
// This function is chosen for default-constructible value types.
// This function is chosen for value types which are not default-constructible.

void from_json(const json &j, DroneConfig &drone)
{
    j["drone"]["position"]["x"].get_to(drone.startPos.x);
    j["drone"]["position"]["y"].get_to(drone.startPos.y);
    j["drone"]["altitude"].get_to(drone.altitude);
    j["drone"]["initialDirection"].get_to(drone.initialDir);
    j["drone"]["attackSpeed"].get_to(drone.attackSpeed);
    j["drone"]["accelerationPath"].get_to(drone.accelPath);
    j["drone"]["angularSpeed"].get_to(drone.angularSpeed);
    j["drone"]["turnThreshold"].get_to(drone.turnThreshold);
    j["simulation"]["timeStep"].get_to(drone.simTimeStep);
    j["simulation"]["hitRadius"].get_to(drone.hitRadius);
    j.at("targetArrayTimeStep").get_to(drone.arrayTimeStep);
    std::strncpy(drone.ammoName, j.at("ammo").get<std::string>().c_str(), AMMO_NAME_SIZE - 1);
}

void from_json(const json &j, AmmoParams &ammo)
{
    std::strncpy(ammo.name, j.at("name").get<std::string>().c_str(), AMMO_NAME_SIZE - 1);
    j.at("mass").get_to(ammo.mass);
    j.at("drag").get_to(ammo.drag);
    j.at("lift").get_to(ammo.lift);
}

void to_json(json &j, const Coord &c)
{
    j = json{{"x", c.x}, {"y", c.y}};
}

void to_json(json &j, const SimStep &step)
{
    j["position"] = json::object();
    j["position"] = step.pos;
    j["direction"] = step.direction;
    j["state"] = step.state;
    j["targetIndex"] = step.targetIdx;
    j["dropPoint"] = step.dropPoint;
    j["aimPoint"] = step.aimPoint;
    j["predictedTarget"] = step.predictedTarget;
}

// ============================================================
// Хелпери для чистки кучи
//
// > чи доречно і безпечно використовувати подібні конструкції?
// ============================================================
inline void heapDelete(DroneConfig *&item, const bool isArr)
{
    if (isArr) {
        delete[] item;
    }
    else {
        delete item;
    }
    item = nullptr;
}

inline void heapDelete(AmmoParams *&item, const bool isArr)
{
    if (isArr) {
        delete[] item;
    }
    else {
        delete item;
    }
    item = nullptr;
}

inline void heapDelete(Coord *&item, const bool isArr)
{
    if (isArr) {
        delete[] item;
    }
    else {
        delete item;
    }
    item = nullptr;
}

inline void heapDelete(Coord **&items, const uint8_t itemsCount)
{
    for (int i = 0; i < itemsCount; i++) {
        delete[] items[i];
    }

    delete[] items;
    items = nullptr;
}

inline void heapDelete(Ballistics *&item, const bool isArr)
{
    if (isArr) {
        delete[] item;
    }
    else {
        delete item;
    }
    item = nullptr;
}

inline void heapDelete(SimStep *&item, const bool isArr)
{
    if (isArr) {
        delete[] item;
    }
    else {
        delete item;
    }
    item = nullptr;
}

inline void heapDelete(DroneConfig *&drone, AmmoParams *&ammo, Coord **&targets, const uint8_t tgtCount, SimStep *&steps)
{
    heapDelete(drone, false);
    heapDelete(ammo, true);
    heapDelete(targets, tgtCount);
    heapDelete(steps, true);
}

inline void heapDelete(DroneConfig *&drone, AmmoParams *&ammo, Coord **&targets, const uint8_t tgtCount)
{
    heapDelete(drone, false);
    heapDelete(ammo, true);
    heapDelete(targets, tgtCount);
}

inline void heapDelete(DroneConfig *&drone, AmmoParams *&ammo)
{
    heapDelete(drone, false);
    heapDelete(ammo, true);
}

// ============================================================
// Програма
// ============================================================
int main(int argc, char *argv[])
{
    // > В ДЗ №2 в мене були проблеми з математикою, слідуючи рекомендаціям з документу (мабуть, я там не зовсім все зрозумів)
    // > якось дрон криво літав, тому додав макрос USE_OWN_CALCS де сильно спростив обчислення, але це позитивно вплинуло на результат
    //
    // > В ДЗ №3 є критерій "Коректність симуляції (результат збігається з ДЗ 2)"
    // > тому написав невеликий тест (знаходиться в папці test/test.cpp)
    // > 1. бере вхідні дані з /test/fixtures/ (там є кілька наборів)
    // > 2. конвертує txt <> json згідно з форматом
    // > 3. копією їх та виконувані файли обох ДЗ у папку test/runtime,
    // > 4. запускає обидва
    // > 5. конвертує результат із ДЗ №3 в TXT формат відповідно ДЗ №3
    // > 6. порівнює вміст файлів

    // > після написання тесту знайшов помилку в ДЗ №2, там на кіл-сть кроків у результуючому файлі правильна,
    // > але даних виводиться на 1 крок більше
    // > у вихідному файлі dz2.cpp в цій папці це виправлено
    //
    // > ще знайшов помилку - як я її не побачив не знаю (був не той індекс для Y)
    // >  float predCoords[CS_ARRAY_SIZE]{
    // >    targetXInTime[i][curPosIdx] + targetVx * totalTime,
    // >    targetYInTime[i][predPosIdx] + targetVy * totalTime
    // > };
    // > та в мене ці координати все одно якісь нереальні виходили
    // > у вихідному файлі dz2.cpp в цій папці виправив

    // > вхідний аргумент додано, щоб була можливість порівняти результати з USE_OWN_CALCS та без
    bool ownCalcs = USE_OWN_CALCS_DEFAULT;
    if (argc > 1) {
        ownCalcs = std::strcmp(argv[1], "1") == 0;
    }

    // ============================================================
    // 6.1 config.json (замість input.txt) - Параметри дрона та симуляції.
    // ============================================================
    json jc{};
    if (!readInputJson(settings.droneConfigFile, jc)) {
        return 1;
    }
    auto drone = new DroneConfig;
    try {
        jc.get_to<DroneConfig>(*drone);
        jc.clear();
    }
    catch (json::exception &e) {
        std::cerr << "Failed to parse drone config:" << '\n';
        std::cerr << e.what() << '\n';
        heapDelete(drone, false);
        return 1;
    }
    LOG("Drone start post: " << drone->startPos.x << " " << drone->startPos.y);

    // ============================================================
    // 6.2 ammo.json (замість хардкоджених масивів) - Масив боєприпасів.
    // ============================================================
    json ja{};
    readInputJson(settings.ammoFile, ja);
    const int ammoCount{static_cast<int>(ja.size())};
    if (ammoCount < 1) {
        std::cerr << "No ammos, can do nothing" << '\n';
        heapDelete(drone, false);
        return 1;
    }
    auto *ammo = new AmmoParams[ammoCount];
    int currentAmmo{-1};
    try {
        for (int i = 0; i < ammoCount; i++) {
            ammo[i] = ja[i].get<AmmoParams>();
            if (strcmp(ammo[i].name, drone->ammoName) == 0) {
                currentAmmo = i;
            }
        }
        ja.clear();
    }
    catch (json::exception &e) {
        std::cerr << "Failed to parse ammos:" << '\n';
        std::cerr << e.what() << '\n';
        heapDelete(drone, ammo);
        return 1;
    }

    LOG("Ammos count: " << ammoCount);

    if (currentAmmo < 0) {
        std::cerr << "Invalid ammo name \"" << drone->ammoName << "\"" << '\n';
    }

    // ============================================================
    // 6.3 targets.json (замість targets.txt)
    // Масив цілей. targetCount та timeSteps — динамічні (визначаються з JSON, не хардкоджені)
    // ============================================================
    json jt{};
    if (!readInputJson(settings.targetsFile, jt)) {
        heapDelete(drone, ammo);
        return 1;
    }

    uint8_t tgtCount{};
    uint8_t timeSteps{};
    Coord **targets = nullptr;
    try {
        tgtCount = jt["targetCount"];
        timeSteps = jt["timeSteps"];
        targets = new Coord *[tgtCount];
        for (int i = 0; i < tgtCount; i++) {
            targets[i] = new Coord[timeSteps];
            for (int j = 0; j < timeSteps; j++) {
                targets[i][j].x = jt["targets"][i]["positions"][j]["x"];
                targets[i][j].y = jt["targets"][i]["positions"][j]["y"];
            }
        }
        LOG("Targets count: " << static_cast<unsigned int>(tgtCount));
        LOG("Targets steps: " << static_cast<unsigned int>(timeSteps));
        ja.clear();
    }
    catch (json::exception &e) {
        std::cerr << "Failed to parse targets:" << '\n';
        std::cerr << e.what() << '\n';
        if (targets) {
            heapDelete(targets, tgtCount);
        }
        heapDelete(drone, ammo);
        return 1;
    }

    LOG("Steps count: " << settings.maxSteps);

    // Часу польоту (метод Кардано)
    if (!calculateDroneBombFallTime(*drone, ammo[currentAmmo])) {
        std::cerr << "No real solution for input data" << '\n';
        heapDelete(drone, ammo, targets, tgtCount);
        return 1;
    }
    LOG("Bomb fall time: " << drone->fTime);

    // Горизонтальної дистанції (степеневий ряд до t⁵)
    if (!calculateDroneBombFlightDistance(*drone, ammo[currentAmmo])) {
        std::cerr << "No real solution for input data" << '\n';
        heapDelete(drone, ammo, targets, tgtCount);
        return 1;
    }
    LOG("Bomb flight distance: " << drone->hDist);

    // Завершуємо ініціалізацію дрона
    drone->accel = drone->attackSpeed * drone->attackSpeed / (2.0f * drone->accelPath);
    drone->stepTurn = drone->angularSpeed * drone->simTimeStep;
    drone->curSpeed = 0.0f;

    // ============================================================
    // Симуляція
    // ============================================================
    auto *steps = new SimStep[settings.maxSteps];
    float currentTime{0.0f};
    uint16_t currentStep = 0;

    steps[currentStep].pos = drone->startPos;
    steps[currentStep].direction = drone->initialDir;
    steps[currentStep].state = DS_STOPPED;
    steps[currentStep].targetIdx = -1;

    while (currentStep < settings.maxSteps) {
        const uint16_t nextStep = currentStep + 1;

        float bestTotalTime{-1.0f};
        float bestDir{0.0f};
        for (int i = 0; i < tgtCount; ++i) {
            // 8. Інтерполювати позиції всіх 5 цілей
            auto *targetCoords = new Coord;
            interpolateTarget(currentTime, *drone, targets[i], timeSteps, *targetCoords);

            auto *predCoords = new Coord;
            auto *ballistics = new Ballistics;
            float totalTime{}, desiredDir{};
            if (!ownCalcs) {
                // ============================================================
                // 6.3. Lead targeting (коригування на випередження)
                // ============================================================
                // Балістика розраховується не до поточної позиції цілі, а до прогнозованої позиції в момент
                // прильоту дрона до точки скиду.

                // 1. Розрахувати орієнтовний час прильоту дрона до точки скиду (totalTime) для поточної позиції цілі
                calculateBallistics(*targetCoords, steps[currentStep].pos, *drone, *ballistics);
                calculateDirAndTimeToFire(*ballistics, steps[currentStep], *drone, desiredDir, totalTime);

                // 2. Обчислити швидкість цілі (targetVx, targetVy) через кінцеві різниці
                const uint8_t curPosIdx = static_cast<uint8_t>(std::floor(currentTime / drone->arrayTimeStep)) % timeSteps;
                const uint8_t predPosIdx = static_cast<uint8_t>(std::floor((currentTime + totalTime) / drone->arrayTimeStep)) % timeSteps;
                const Coord targetV = (targets[i][predPosIdx] - targets[i][curPosIdx]) / drone->simTimeStep;

                // Прогнозована позиція цілі на момент t + totalTime:
                // > виходить дічь якась, не розумію
                *predCoords = targets[i][curPosIdx] + targetV * totalTime;

                // 3. Інтерполювати прогнозовану позицію цілі на момент currentTime + totalTime
                // > не розумію що тут потрібно зробити, ?

                interpolateTarget(currentTime + totalTime, *drone, targets[i], timeSteps, *predCoords);

                // 4. Перерахувати балістику до прогнозованої позиції
                calculateBallistics(*predCoords, steps[currentStep].pos, *drone, *ballistics);
                calculateDirAndTimeToFire(*ballistics, steps[currentStep], *drone, desiredDir, totalTime);
            }
            else {
                // ============================================================
                // 6.3. Lead targeting (коригування на випередження)
                // ============================================================

                // > до кінця не розумію чому при такій калькуляції дрон швидше вражає ціль

                // 9. Для кожної цілі: розрахувати балістику з lead targeting, точку скиду, орієнтовний час
                interpolateTarget(currentTime + drone->fTime, *drone, targets[i], timeSteps, *predCoords);
                calculateBallistics(*predCoords, steps[currentStep].pos, *drone, *ballistics);
                calculateDirAndTimeToFire(*ballistics, steps[currentStep], *drone, desiredDir, totalTime);
            }

            // 10.Обрати ціль з мінімальним загальним часом (з врахуванням timeToStop при зміні цілі)
            // > timeToStop вже врахований у totalTime
            if (bestTotalTime < 0.0f || totalTime < bestTotalTime) {
                steps[currentStep].targetIdx = i;
                bestDir = desiredDir;
                steps[currentStep].dropPoint = ballistics->fireCoords;
                steps[currentStep].predictedTarget = *predCoords;

                bestTotalTime = totalTime;
            }

            heapDelete(predCoords, false);
            heapDelete(targetCoords, false);
            heapDelete(ballistics, false);
        }

        // такого бути не повинно
        if (steps[currentStep].targetIdx < 0) {
            std::cerr << "No valid target found\n";
            heapDelete(drone, ammo, targets, tgtCount);
            return 1;
        }

        // Симуляція завершується, коли дрон досягне точки скиду (hitRadius) і скине боєприпас.
        //
        // > тут трохи не зрозумів саму задачу, якщо скидати в межах hitRadius,
        // > то буде менш точне попадання, але скид на декілька кроків раніше
        // > виходячи з цього взяв щось середнє - hitRadius / 2
        //
        const float hitRadius = drone->hitRadius / 2;

        // точка влучання бомби
        steps[currentStep].aimPoint.x = steps[currentStep].pos.x + std::cos(steps[currentStep].direction) * drone->hDist;
        steps[currentStep].aimPoint.y = steps[currentStep].pos.y + std::sin(steps[currentStep].direction) * drone->hDist;

        // позиція цілі на момент прильоту бомби
        interpolateTarget(
            currentTime + drone->fTime, *drone, targets[steps[currentStep].targetIdx], timeSteps, steps[currentStep].predictedTarget);

        // перевірка точності влучання
        const float dx = steps[currentStep].predictedTarget.x - steps[currentStep].aimPoint.x;
        const float dy = steps[currentStep].predictedTarget.y - steps[currentStep].aimPoint.y;
        if (dx * dx + dy * dy <= (hitRadius * hitRadius) && steps[currentStep].state == DS_MOVING) {
            std::cout << "DROPPED on step " << currentStep + 1 << '\n';
            break;  // скид боєприпасу
        }

        // {
        //     json js = steps[currentStep];
        //     LOG("Step #" << currentStep << ":");
        //     LOG(js.dump());
        // }

        if (nextStep >= settings.maxSteps) {
            ++currentStep;

            break;
        }

        // Наступний крок
        steps[nextStep].state = steps[currentStep].state;
        steps[nextStep].direction = steps[currentStep].direction;

        // 11. Перевірити кут повороту. Якщо > turnThreshold — змінити стан на DECELERATING/TURNING
        float deltaAngle = normalizeAngle(bestDir - steps[nextStep].direction);
        if (std::fabs(deltaAngle) > drone->turnThreshold) {
            steps[nextStep].state = drone->curSpeed > 0.0f ? DS_DECELERATING : DS_TURNING;
        }
        else {
            // Якщо кут ≤ turnThreshold — дрон змінює напрямок без зупинки.
            steps[nextStep].direction = bestDir;

            // якщо поворот завершено
            if (steps[nextStep].state == DS_TURNING) {
                // розганяється у новому напрямку
                steps[nextStep].state = DS_ACCELERATING;
            }
        }

        // 12.Оновити координати, швидкість та стан дрона відповідно до поточної фази
        bool isUpdateDroneCoords = false;
        switch (steps[nextStep].state) {
            case DS_STOPPED:
                steps[nextStep].state = DS_ACCELERATING;  // не стоїмо на місці
                break;
            case DS_ACCELERATING:
                // розгін
                drone->curSpeed += drone->accel * drone->simTimeStep;

                if (drone->curSpeed >= drone->attackSpeed) {
                    // досягли attackSpeed
                    drone->curSpeed = drone->attackSpeed;
                    steps[nextStep].state = DS_MOVING;  // рух зі сталою шв.
                }

                isUpdateDroneCoords = true;  // оновити координати дрону
                break;
            case DS_DECELERATING:
                // гальмування
                drone->curSpeed -= drone->accel * drone->simTimeStep;

                if (drone->curSpeed <= 0.0f) {
                    // зупинились
                    drone->curSpeed = 0.0f;
                    steps[nextStep].state = DS_STOPPED;
                }

                isUpdateDroneCoords = true;  // оновити координати дрону
                break;
            case DS_TURNING: {
                // поворот на залишок
                float turn = std::max(-drone->stepTurn, std::min(drone->stepTurn, deltaAngle));
                steps[nextStep].direction = normalizeAngle(steps[nextStep].direction + turn);

                // якщо поворот завершено
                if (std::fabs(normalizeAngle(bestDir - steps[nextStep].direction)) <= 0.0f) {
                    steps[nextStep].direction = bestDir;
                    steps[nextStep].state = DS_ACCELERATING;  // починаємо розгон
                }
                break;
            }
            case DS_MOVING:
                isUpdateDroneCoords = true;  // оновити координати дрону
                break;
            default:
                break;
        }

        // Оновлення позиції дрона
        if (isUpdateDroneCoords) {
            steps[nextStep].pos.x = steps[currentStep].pos.x + std::cos(steps[nextStep].direction) * drone->curSpeed * drone->simTimeStep;
            steps[nextStep].pos.y = steps[currentStep].pos.y + std::sin(steps[nextStep].direction) * drone->curSpeed * drone->simTimeStep;
        }
        else {
            steps[nextStep].pos = steps[currentStep].pos;
        }

        currentStep++;
        currentTime += drone->simTimeStep;
    }

    // ============================================================
    // 6.4 simulation.json (вихідний файл) - Результат симуляції.
    // ============================================================
    auto js = json::object();
    js["totalSteps"] = currentStep;
    js["steps"] = json::array();
    for (int i = 0; i < currentStep; i++) {
        js["steps"][i] = steps[i];
    }
    {
        std::ofstream file(settings.outFile);
        file << js.dump(2) << '\n';
        file.close();
    }
    js.clear();

    heapDelete(drone, ammo, targets, tgtCount, steps);

    return 0;
}
