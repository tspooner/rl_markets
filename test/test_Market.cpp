#include "catch.hpp"
#include "market/market.h"

using namespace std;
using namespace market;


SCENARIO("AAL.L", "[Market]") {

    Market* m = Market::make_market("AAL", "L");

    REQUIRE(m->open_time() == 28800000);

    GIVEN("a negative price") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS(m->tick_size(-1.0));
        }
    }

    GIVEN("a price 2750.0") {
        THEN("the tick size should be 0.5") {
            REQUIRE(m->tick_size(2750.0) == 0.5);
        }

        THEN("the tick value should be 52500") {
            REQUIRE(m->ToTicks(2750.0) == 52500);
            REQUIRE(m->ToPrice(52500) == 2750.0);
        }
    }
}

SCENARIO("HSBA.L", "[Market]") {

    Market* m = Market::make_market("HSBA", "L");

    GIVEN("a negative price") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS(m->tick_size(-1.0));
        }
    }

    REQUIRE(m->ToTicks(702.1) == 46021);
    REQUIRE(m->ToTicks(702.5) == 46025);
}

SCENARIO("AIRF.PA", "[Market]") {

    Market* m = Market::make_market("AIRF", "PA");

    GIVEN("a negative price") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS(m->tick_size(-1.0));
        }
    }

    GIVEN("a price 9.5") {
        THEN("the tick size should be 0.001") {
            REQUIRE(m->tick_size(9.5) == 0.001);
        }

        THEN("the tick value should be 9500") {
            REQUIRE(m->ToTicks(9.5) == 9500);
            REQUIRE(m->ToPrice(9500) == 9.5);
        }
    }

    GIVEN("a price 12.6") {
        THEN("the tick size should be 0.005") {
            REQUIRE(m->tick_size(12.6) == 0.005);
        }

        THEN("the tick value should be 10520") {
            REQUIRE(m->ToTicks(12.6) == 10520);
            REQUIRE(m->ToPrice(10520) == 12.6);
        }
    }
}

SCENARIO("CRDI.MI", "[Market]") {

    Market* m = Market::make_market("CRDI", "MI");

    REQUIRE(m->open_time() == 32400000);

    GIVEN("a negative price") {
        THEN("it should throw an exception") {
            REQUIRE_THROWS(m->tick_size(-1.0));
        }
    }

    GIVEN("a price 1.96885 which is inbetween ticks") {
        THEN("the tick size should be 0.001") {
            REQUIRE(m->tick_size(1.96885) == 0.001);
        }

        THEN("a round trip should round the price up to the nearest tick") {
            REQUIRE(m->ToPrice(m->ToTicks(1.96885)) == Approx(1.969));
        }
    }
}
