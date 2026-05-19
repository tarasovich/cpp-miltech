#include "ballistics.hpp"

#include <gtest/gtest.h>

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

    const BallisticsResult solution = calculateBallistics(input);

    EXPECT_NEAR(solution.fireX, 173.759, 0.01);

    EXPECT_NEAR(solution.fireY, 173.759, 0.01);
}
