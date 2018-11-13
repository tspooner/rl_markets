#include "../build/ext/spdlog/src/spdlog/tests/catch.hpp"
#include "utilities/accumulators.h"

using namespace std;

SCENARIO("a sequence of values 1:1:5", "[Accumulator][RollingMedian]") {

    GIVEN("a rolling mean with window size 3") {
        RollingMean<float> rm(3);

        for (int i = 1; i <= 3; i++)
            rm.push((float) i);

        REQUIRE(rm.mean() == Approx(2.0f));
        REQUIRE(rm.var() == Approx(1.0f));

        for (int i = 4; i <= 5; i++) {
            rm.push(i);
            REQUIRE(rm.mean() == Approx(i-1.0f));
            REQUIRE(rm.var() == Approx(1.0f));
        }
    }

    GIVEN("a rolling median") {
        RollingMedian<float> rm(5);

        for (int i = 1; i <= 5; i++)
            rm.push((float) i);

        THEN("the median should be 3") {
            REQUIRE(rm.median() == Approx(3.0f));
        }
    }
}

SCENARIO("a sequence of values 10.5, 11.5, 11.5, 11.5, 12.5", "[Accumulator][RollingMedian]") {

    GIVEN("a rolling mean with window size 3") {
        RollingMean<float> rm(3);

        rm.push(10.5);
        rm.push(11.5);
        rm.push(11.5);

        REQUIRE(rm.mean() == Approx(11.1667f));
        REQUIRE(rm.var() == Approx(1.0f / 3));

        rm.push(11.5);
        REQUIRE(rm.mean() == Approx(11.5f));
        REQUIRE(rm.var() == Approx(0.0f).scale(1.0f));

        rm.push(12.5);
        REQUIRE(rm.mean() == Approx(11.8333f));
        REQUIRE(rm.var() == Approx(1.0f / 3));
    }

    GIVEN("a rolling median") {
        RollingMedian<float> rm(5);

        rm.push(10.5);
        rm.push(11.5);
        rm.push(11.5);
        rm.push(11.5);
        rm.push(12.5);

        THEN("the median should be 3") {
            REQUIRE(rm.median() == Approx(11.5f));
        }
    }
}
