#include "ballistics.hpp"

#include <gtest/gtest.h>

TEST(Ballistics, ValidateInput)
{
    std::unordered_map<std::string, BallisticsInput> testCases{
        {"Unknown ammo \"VPK-3000\"", {100.0f, 100.0f, 100.0f, 200.0f, 200.0f, 10, 10, "VPK-3000"}},
        {"Attack speed out of range", {100.0F, 100.0F, 100.0F, 200.0F, 200.0F, -1, 10, "VOG-17"}},
        {"Attack speed out of range", {100.0F, 100.0F, 100.0F, 200.0F, 200.0F, 61, 10, "VOG-17"}},
        {"Drone too low", {100.0F, 100.0F, 5.0F, 200.0F, 200.0F, 10, 10, "VOG-17"}},
        {"Acceleration path out of range", {100.0F, 100.0F, 100.0F, 200.0F, 200.0F, 10, -1, "VOG-17"}},
    };

    // ReSharper disable once CppUseElementsView
    for (const auto& [expectedError, input] : testCases) {
        try {
            calculateBallistics(input);
            FAIL() << "Expected std::invalid_argument";
        }
        catch (const std::invalid_argument& e) {
            EXPECT_EQ(e.what(), expectedError);  // тут краще б contains
        }
        catch (...) {
            FAIL() << "Expected std::invalid_argument";
        }
    }

}

TEST(Ballistics, ComputesKnownDropPoint)
{
    constexpr BallisticsInput input{
        .droneX = 100.0,
        .droneY = 100.0,
        .altitude = 100.0,
        .targetX = 200.0,
        .targetY = 200.0,
        .attackSpeed = 10.0,
        .accelerationPath = 10.0,
        .ammoName = "VOG-17",
    };

    BallisticsResult solution{};
    solution = calculateBallistics(input);

    EXPECT_NEAR(solution.fireX, 173.759, 0.01);

    EXPECT_NEAR(solution.fireY, 173.759, 0.01);
}
