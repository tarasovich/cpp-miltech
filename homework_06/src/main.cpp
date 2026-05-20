#include <ballistics.hpp>
#include <fstream>
#include <iostream>

int main(const int argc, char **argv)
{
    // The executable expects exactly one telemetry log path.
    if (argc != 2) {
        std::cerr << "usage: ballistics <input_path>" << '\n';
        return 1;
    }

    std::ifstream inputFile{argv[1]};
    if (!inputFile) {
        std::cerr << "error: failed to open input file: " << argv[1] << '\n';
        return 0;
    }

    BallisticsInput inputData{};
    if (!(inputFile >> inputData.droneX >> inputData.droneY >> inputData.altitude >> inputData.targetX >> inputData.targetY >>
          inputData.attackSpeed >> inputData.accelerationPath >> inputData.ammoName)) {
        std::cerr << "error: failed to read input data" << '\n';
        return 0;
    }

    BallisticsResult result{};
    try {
        result = calculateBallistics(inputData);
    }
    catch (const std::invalid_argument &e) {
        std::cerr << "error: invalid input data - " << e.what() << '\n';

        return 1;
    }
    catch (const std::logic_error &e) {
        std::cerr << "error: calculation failed - " << e.what() << '\n';
        return 1;
    }

    std::cout << "Ballistic result: " << result.fireX << " " << result.fireY << '\n';

    return 0;
}
