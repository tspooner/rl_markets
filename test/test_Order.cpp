#include "catch.hpp"
#include "market/order.h"

using namespace std;

SCENARIO("an order has a fixed price and size", "[Market][Order]") {

    GIVEN("an order at price 1.0 and size 100") {
        market::Order o(1.0, 100, 0);

        THEN("the price must be consistent") {
            REQUIRE(o.price == 1.0);
        }

        THEN("the size must be consistent") {
            REQUIRE(o.size == 100);
        }
    }
}

SCENARIO("an order has variable size and queue length", "[Market][Order]") {

    GIVEN("any order with queue volume ahead and behind") {
        market::Order o(1.0, 100, 50);
        o.addVolumeBehind(50);

        THEN("the object must be consistent with the construction arguments") {
            REQUIRE(o.size == 100);
            REQUIRE(o.getTotalExecutedVolume() == 0);
            REQUIRE(o.isExecuted() == false);
            REQUIRE(o.getQueueAhead() == 50);
            REQUIRE(o.getQueueBehind() == 50);
        }

        THEN("the queues can be cleared") {
            o.clearQueues();

            REQUIRE(o.getQueueAhead() == 0);
            REQUIRE(o.getQueueBehind() == 0);
        }
    }

    GIVEN("a non-zero positive integer size: v = 100 > 0") {
        THEN("the book can be constructed") {
            REQUIRE_NOTHROW(market::Order(1.0, 100, 0));
        }
    }

    GIVEN("a zero queue: 0") {
        THEN("the book can be constructed") {
            REQUIRE_NOTHROW(market::Order(1.0, 100, 0));
        }
    }

    GIVEN("an erroneous (negative) size: -100") {
        THEN("an exception is raised since the order size must be positive") {
            REQUIRE_THROWS(market::Order(1.0, -100, 0));
        }
    }

    GIVEN("an erroneous (zero) size: 0") {
        THEN("an exception is raised since the order size must be non-zero") {
            REQUIRE_THROWS(market::Order(1.0, 0, 0));
        }
    }

    GIVEN("a negative integer queue: -100") {
        THEN("an exception is raised since the queue must be positive") {
            REQUIRE_THROWS(market::Order(1.0, 100, -100));
        }
    }
}

SCENARIO("order handles cancellations", "[Market][Order]") {

    GIVEN("any order") {
        market::Order o(1.0, 1, 1);

        WHEN("a negative cancellation volume: v = -50 < 0") {
            THEN("an exception is thrown since cancellations must be positive") {
                REQUIRE_THROWS(o.doCancellation(-50));
            }
        }

        WHEN("a zero cancellation volume: v = 0") {
            THEN("nothing should change") {
                REQUIRE(o.getQueueAhead() == 1);
            }
        }
    }

    GIVEN("an order with: 0:100:0") {
        market::Order o(1.0, 100, 0);

        WHEN("50 units are cancelled") {
            o.doCancellation(50);

            THEN("the queue ahead remains unchanged") {
                REQUIRE(o.getQueueAhead() == 0);
            }
        }
    }

    GIVEN("an order with: 100:100:0") {
        market::Order o(1.0, 100, 100);

        WHEN("50 units are cancelled") {
            o.doCancellation(50);

            THEN("the new queue ahead should be 50") {
                REQUIRE(o.getQueueAhead() == 50);
            }
        }

        WHEN("100 units are cancelled") {
            o.doCancellation(100);

            THEN("the new queue ahead should be 0") {
                REQUIRE(o.getQueueAhead() == 0);
            }
        }

        WHEN("150 units are cancelled") {
            o.doCancellation(150);

            THEN("the new queue ahead should be 0") {
                REQUIRE(o.getQueueAhead() == 0);
            }
        }
    }

    GIVEN("an order with: 0:100:100") {
        market::Order o(1.0, 100, 0);
        o.addVolumeBehind(100);

        WHEN("50 units are cancelled") {
            o.doCancellation(50);

            THEN("the new queue behind should be 50") {
                REQUIRE(o.getQueueBehind() == 50);
            }
        }

        WHEN("100 units are cancelled") {
            o.doCancellation(100);

            THEN("the new queue behind should be 0") {
                REQUIRE(o.getQueueBehind() == 0);
            }
        }

        WHEN("150 units are cancelled") {
            o.doCancellation(150);

            THEN("the new queue behind should be 0") {
                REQUIRE(o.getQueueBehind() == 0);
            }
        }
    }

    GIVEN("an order with: 100:100:100") {
        market::Order o(1.0, 100, 100);
        o.addVolumeBehind(100);

        WHEN("50 units are cancelled") {
            o.doCancellation(50);

            THEN("the new queues should both be 75") {
                REQUIRE(o.getQueueAhead() == 75);
                REQUIRE(o.getQueueBehind() == 75);
            }
        }

        WHEN("100 units are cancelled") {
            o.doCancellation(100);

            THEN("the new queues should both be 50") {
                REQUIRE(o.getQueueAhead() == 50);
                REQUIRE(o.getQueueBehind() == 50);
            }
        }

        WHEN("1000 units are cancelled") {
            o.doCancellation(1000);

            THEN("the new queues should both be 0") {
                REQUIRE(o.getQueueAhead() == 0);
                REQUIRE(o.getQueueBehind() == 0);
            }
        }
    }

    GIVEN("an order with: 100:100:500") {
        market::Order o(1.0, 100, 100);
        o.addVolumeBehind(500);

        WHEN("50 units are cancelled") {
            o.doCancellation(50);

            THEN("the new order should be: 91:100:459") {
                REQUIRE(o.getQueueAhead() == 91);
                REQUIRE(o.getQueueBehind() == 459);
            }
        }

        WHEN("100 units are cancelled") {
            o.doCancellation(100);

            THEN("the new order should be: 83:100:417") {
                REQUIRE(o.getQueueAhead() == 83);
                REQUIRE(o.getQueueBehind() == 417);
            }
        }

        WHEN("600 units are cancelled") {
            o.doCancellation(600);

            THEN("the new order should be: 0:100:0") {
                REQUIRE(o.getQueueAhead() == 0);
                REQUIRE(o.getQueueBehind() == 0);
            }
        }

        WHEN("1000 units are cancelled") {
            o.doCancellation(1000);

            THEN("the new queues should both be 0") {
                REQUIRE(o.getQueueAhead() == 0);
                REQUIRE(o.getQueueBehind() == 0);
            }
        }
    }

    GIVEN("an order with: 100:100:5000") {
        market::Order o(1.0, 100, 100);
        o.addVolumeBehind(5000);

        WHEN("50 units are cancelled") {
            o.doCancellation(50);

            THEN("the new order should be: 99:100:4951") {
                REQUIRE(o.getQueueAhead() == 99);
                REQUIRE(o.getQueueBehind() == 4951);
            }
        }

        WHEN("100 units are cancelled") {
            o.doCancellation(100);

            THEN("the new order should be: 98:100:4902") {
                REQUIRE(o.getQueueAhead() == 98);
                REQUIRE(o.getQueueBehind() == 4902);
            }
        }

        WHEN("10000 units are cancelled") {
            o.doCancellation(10000);

            THEN("the new queues should both be 0") {
                REQUIRE(o.getQueueAhead() == 0);
                REQUIRE(o.getQueueBehind() == 0);
            }
        }
    }
}

SCENARIO("order handles transactions", "[Market][Order]") {

    GIVEN("any order") {
        market::Order o(1.0, 1, 0);

        WHEN("a negative transaction volume: v = -50 < 0") {
            THEN("an exception is thrown since transactions must be positive") {
                REQUIRE_THROWS(o.doTransaction(-50));
                REQUIRE(o.getTotalTransactedVolume() == 0);
            }
        }

        WHEN("a zero transaction volume: v = 0") {
            THEN("nothing should change") {
                REQUIRE(o.getTotalExecutedVolume() == 0);
                REQUIRE(o.getTotalTransactedVolume() == 0);
            }
        }
    }

    GIVEN("an order with: 0:100:0") {
        market::Order o(1.0, 100, 0);

        WHEN("50 units are transacted") {
            o.doTransaction(50);

            THEN("the order should be partially executed: 0:50:0") {
                REQUIRE(o.isExecuted() == false);
                REQUIRE(o.getTotalExecutedVolume() == 50);
                REQUIRE(o.getTotalTransactedVolume() == 50);
            }
        }

        WHEN("100 units are transacted") {
            o.doTransaction(100);

            THEN("the order should be fully executed") {
                REQUIRE(o.isExecuted());
                REQUIRE(o.getTotalExecutedVolume() == 100);
                REQUIRE(o.getTotalTransactedVolume() == 100);
            }
        }
    }

    GIVEN("an order with: 100:100:0") {
        market::Order o(1.0, 100, 100);

        WHEN("50 units are transacted") {
            o.doTransaction(50);

            THEN("the order should now be: 50:100:0") {
                REQUIRE(o.isExecuted() == false);
                REQUIRE(o.getTotalExecutedVolume() == 0);
                REQUIRE(o.getTotalTransactedVolume() == 50);
            }
        }

        WHEN("100 units are transacted") {
            o.doTransaction(100);

            THEN("the order should now be: 0:100:0") {
                REQUIRE(o.isExecuted() == false);
                REQUIRE(o.getTotalExecutedVolume() == 0);
                REQUIRE(o.getTotalTransactedVolume() == 100);
            }
        }

        WHEN("150 units are transacted") {
            o.doTransaction(150);

            THEN("the order should now be: 0:50:0") {
                REQUIRE(o.isExecuted() == false);
                REQUIRE(o.getTotalExecutedVolume() == 50);
                REQUIRE(o.getTotalTransactedVolume() == 150);
            }
        }

        WHEN("200 units are transacted") {
            o.doTransaction(200);

            THEN("the order should now be: 50:100:0") {
                REQUIRE(o.isExecuted() == true);
                REQUIRE(o.getTotalExecutedVolume() == 100);
                REQUIRE(o.getTotalTransactedVolume() == 200);
            }
        }
    }
}
