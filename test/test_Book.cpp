#include "catch.hpp"
#include "market/book.h"
#include "utilities/comparison.h"

#include <array>

using namespace std;

typedef std::map<double, long, FloatComparator<>> TMAP;

SCENARIO("book handles market depth data", "[Market][Book]") {

    GIVEN("an ask book with 2 levels") {
        market::AskBook<2> book;

        array<double, 2> prices{100.0, 200.0};
        array<long, 2> volumes{500, 1000};

        REQUIRE(book.depth() == 2);

        WHEN("the market condition is updated") {
            book.ApplyChanges(prices, volumes);

            THEN("the prices and volumes are consistent") {
                REQUIRE(book.price(0) == prices[0]);
                REQUIRE(book.volume(prices[0]) == volumes[0]);
            }

            THEN("the prices may be accessed using reverse indices") {
                REQUIRE(book.price(-1) == prices[1]);
                REQUIRE(book.price(-2) == prices[0]);

                REQUIRE_THROWS(book.price(-3));
            }
        }

        WHEN("the market condition is updated twice") {
            book.ApplyChanges(prices, volumes);

            book.StashState();
            book.ApplyChanges(prices, volumes);

            THEN("the last prices and volumes are consistent") {
                REQUIRE(book.last_price(0) == prices[0]);
                REQUIRE(book.last_volume(prices[0]) == volumes[0]);
            }

            THEN("the last top price/volume can be accessed using price(-1)") {
                REQUIRE(book.last_price(-1) == prices[1]);
            }
        }
    }

    GIVEN("an bid book with 2 levels") {
        market::BidBook<2> book;

        array<double, 2> prices{100.0, 200.0};
        array<long, 2> volumes{500, 1000};

        REQUIRE(book.depth() == 2);

        WHEN("the market condition is updated") {
            book.ApplyChanges(prices, volumes);

            THEN("the prices and volumes are consistent") {
                REQUIRE(book.price(0) == prices[1]);
                REQUIRE(book.volume(prices[1]) == volumes[1]);
            }

            THEN("the prices may be accessed using reverse indices") {
                REQUIRE(book.price(-1) == prices[0]);
                REQUIRE(book.price(-2) == prices[1]);

                REQUIRE_THROWS(book.price(-3));
            }
        }

        WHEN("the market condition is updated twice") {
            book.ApplyChanges(prices, volumes);

            book.StashState();
            book.ApplyChanges(prices, volumes);

            THEN("the last prices and volumes are consistent") {
                REQUIRE(book.last_price(0) == prices[1]);
                REQUIRE(book.last_volume(prices[1]) == volumes[1]);
            }

            THEN("the last top price/volume can be accessed using price(-1)") {
                REQUIRE(book.last_price(-1) == prices[0]);
            }
        }
    }

    GIVEN("a book with 1 level") {
        market::AskBook<1> book;

        WHEN("the book is updated with negative prices") {
            array<double, 1> prices{-1.0};
            array<long, 1> volumes{100};

            THEN("an exception is raised") {
                REQUIRE_THROWS(book.ApplyChanges(prices, volumes));
            }
        }

        WHEN("the book is updated with negative volumes") {
            array<double, 1> prices{100.0};
            array<long, 1> volumes{-100};

            THEN("an exception is raised") {
                REQUIRE_THROWS(book.ApplyChanges(prices, volumes));
            }
        }
    }

    GIVEN("an empty book with 1 level") {
        market::AskBook<1> book;

        array<double, 1> prices1{123.4560};
        array<long, 1> volumes1{500};
        array<double, 1> prices2{987.6540};
        array<long, 1> volumes2{1000};

        REQUIRE(book.depth() == 1);

        WHEN("accessing price/volume information before updating") {
            THEN("an exception is thrown since the price is undefined") {
                REQUIRE_THROWS(book.price(0));
                REQUIRE_THROWS(book.price_level(100.0));
            }

            THEN("the volume returns zero since there are no orders at that price") {
                REQUIRE(book.volume(prices1[0]) == 0);
            }

            THEN("an exception is thrown since the last price is undefined") {
                REQUIRE_THROWS(book.last_price(0));
                REQUIRE_THROWS(book.last_price_level(100.0));
            }

            THEN("the volume returns zero since there were no orders at that price") {
                REQUIRE(book.last_volume(prices1[0]) == 0);
            }
        }

        WHEN("the market condition is updated once") {
            book.ApplyChanges(prices1, volumes1);

            THEN("the new prices and volumes are consistent") {
                REQUIRE(book.price(0) == prices1[0]);
                REQUIRE(book.volume(prices1[0]) == volumes1[0]);

                REQUIRE(book.price_level(prices1[0]) == 0);
            }

            THEN("the last prices and volumes are still undefined/zero") {
                REQUIRE_THROWS(book.last_price(0));
                REQUIRE_THROWS(book.last_price_level(prices1[0]));

                REQUIRE(book.last_volume(prices1[0]) == 0);
            }
        }

        WHEN("the market condition is updated twice") {
            book.ApplyChanges(prices1, volumes1);

            book.StashState();
            book.ApplyChanges(prices2, volumes2);

            THEN("the prices and volumes are consistent") {
                REQUIRE(book.price(0) == prices2[0]);
                REQUIRE(book.volume(prices2[0]) == volumes2[0]);

                REQUIRE(book.price_level(prices2[0]) == 0);
                REQUIRE(book.price_level(prices1[0]) == -1);
            }

            THEN("the last prices and volumes are consistent") {
                REQUIRE(book.last_price(0) == prices1[0]);
                REQUIRE(book.last_volume(prices1[0]) == volumes1[0]);

                REQUIRE(book.last_price_level(prices1[0]) == 0);
                REQUIRE(book.last_price_level(prices2[0]) == -1);
            }
        }
    }

    GIVEN("a non-empty book with 1 level") {
        market::AskBook<1> book;

        array<double, 1> prices{100.0};
        array<long, 1> volumes{500};
        book.ApplyChanges(prices, volumes);

        WHEN("the book is Reset") {
            book.Reset();

            THEN("an exception is raised since the prices and volumes are now undefined") {
                REQUIRE_THROWS(book.price(0));
                REQUIRE_THROWS(book.last_price(0));

                REQUIRE(book.volume(prices[0]) == 0);
                REQUIRE(book.last_volume(prices[0]) == 0);
            }
        }
    }
}

SCENARIO("book handles transaction data (no agent orders)", "[Market][Book]") {

    GIVEN("a book with a single level of: 100 units @ 100.0") {
        market::AskBook<1> book;

        array<double, 1> prices{100.0};
        array<long, 1> volumes{100};
        book.ApplyChanges(prices, volumes);

        WHEN("the book is updated using a array of transactions at available prices") {
            book.ApplyTransactions({{100.0, 100}}, 0.0);

            THEN("total transaction volume is updated") {
                REQUIRE(book.observed_value() == 10000.0);
                REQUIRE(book.observed_volume() == 100L);
            }
        }
    }
}

SCENARIO("handles agent order placement", "[Market][Book]") {

    GIVEN("any book") {
        market::AskBook<1> book;

        WHEN("no orders exists") {
            THEN("order methods should return false (or equivalent)") {
                REQUIRE(book.HasOpenOrder(100.0) == false);
                REQUIRE(book.order_size(100.0) == -1);
                REQUIRE(book.queue_ahead(100.0) == -1);
                REQUIRE(book.queue_behind(100.0) == -1);
            }
        }
    }

    GIVEN("a bid book with a single level: 100 units @ 100.0") {
        market::BidBook<1> book;

        array<double, 1> prices{100.0};
        array<long, 1> volumes{100};

        book.ApplyChanges(prices, volumes);

        WHEN("an order of 50 units is placed at level 1") {
            book.PlaceOrderAtLevel(0, 50);

            THEN("the order exists in the book and is consistent") {
                REQUIRE(book.order_count() == 1);
                REQUIRE(book.HasOpenOrder(100.0));
                REQUIRE(book.order_size(100.0) == 50);
                REQUIRE(book.order_remaining_volume(100.0) == 50);
            }

            THEN("there are 100 units ahead and 0 behind") {
                REQUIRE(book.queue_ahead(100.0) == 100);
                REQUIRE(book.queue_behind(100.0) == 0);
            }
        }

        WHEN("an order is placed at a specified level not in the book") {
            THEN("an exception is raised") {
                REQUIRE_THROWS(book.PlaceOrderAtLevel(100, 50));
            }
        }

        WHEN("an order of 50 units is placed at 100.0 (level 1)") {
            book.PlaceOrder(100.0, 50);

            THEN("the order exists in the book and is consistent") {
                REQUIRE(book.order_count() == 1);
                REQUIRE(book.HasOpenOrder(100.0));
                REQUIRE(book.order_size(100.0) == 50);
                REQUIRE(book.order_remaining_volume(100.0) == 50);
            }

            THEN("there are 100 units ahead and 0 behind") {
                REQUIRE(book.queue_ahead(100.0) == 100);
                REQUIRE(book.queue_behind(100.0) == 0);
            }
        }

        WHEN("an order of 50 units is placed at 110.0 (not in any level)") {
            book.PlaceOrder(110.0, 50);

            THEN("the order exists in the book and is consistent") {
                REQUIRE(book.order_count() == 1);
                REQUIRE(book.HasOpenOrder(110.0));
                REQUIRE(book.order_size(110.0) == 50);
                REQUIRE(book.order_remaining_volume(110.0) == 50);
            }

            THEN("there are 0 units ahead and 0 behind") {
                REQUIRE(book.queue_ahead(110.0) == 0);
                REQUIRE(book.queue_behind(110.0) == 0);
            }
        }
    }

    GIVEN("an ask book with two levels: 100 units @ 100.0; 200 units @ 110.0") {
        market::AskBook<2> book;

        array<double, 2> prices{100.0, 110.0};
        array<long, 2> volumes{100, 200};

        book.ApplyChanges(prices, volumes);

        WHEN("an order of 50 units is placed at level 1 (100.0)") {
            book.PlaceOrderAtLevel(0, 50);

            THEN("the order exists in the book and is consistent") {
                REQUIRE(book.order_count() == 1);
                REQUIRE(book.HasOpenOrder(100.0));
                REQUIRE(book.order_size(100.0) == 50);
                REQUIRE(book.order_remaining_volume(100.0) == 50);
            }

            THEN("there are 100 units ahead and 0 behind") {
                REQUIRE(book.queue_ahead(100.0) == 100);
                REQUIRE(book.queue_behind(100.0) == 0);
            }
        }

        WHEN("an order of 100 units is placed at level 2 (110.0)") {
            book.PlaceOrderAtLevel(1, 100);

            THEN("the order exists in the book and is consistent") {
                REQUIRE(book.order_count() == 1);
                REQUIRE(book.HasOpenOrder(110.0));
                REQUIRE(book.order_size(110.0) == 100);
                REQUIRE(book.order_remaining_volume(110.0) == 100);
            }

            THEN("there are 200 units ahead and 0 behind") {
                REQUIRE(book.queue_ahead(110.0) == 200);
                REQUIRE(book.queue_behind(110.0) == 0);
            }
        }

        WHEN("an order is placed at a specified level not in the book") {
            THEN("an exception is raised") {
                REQUIRE_THROWS(book.PlaceOrderAtLevel(100, 50));
            }
        }

        WHEN("an order of 50 units is placed at 100.0 (level 1)") {
            book.PlaceOrder(100.0, 50);

            THEN("the order exists in the book and is consistent") {
                REQUIRE(book.order_count() == 1);
                REQUIRE(book.HasOpenOrder(100.0));
                REQUIRE(book.order_size(100.0) == 50);
                REQUIRE(book.order_remaining_volume(100.0) == 50);
            }

            THEN("there are 100 units ahead and 0 behind") {
                REQUIRE(book.queue_ahead(100.0) == 100);
                REQUIRE(book.queue_behind(100.0) == 0);
            }
        }

        WHEN("an order of 50 units is at 90.0 (not in any level)") {
            book.PlaceOrder(90.0, 50);

            THEN("the order exists in the book and is consistent") {
                REQUIRE(book.order_count() == 1);
                REQUIRE(book.HasOpenOrder(90.0));
                REQUIRE(book.order_size(90.0) == 50);
                REQUIRE(book.order_remaining_volume(90.0) == 50);
            }

            THEN("there are 0 units ahead and 0 behind") {
                REQUIRE(book.queue_ahead(90.0) == 0);
                REQUIRE(book.queue_behind(90.0) == 0);
            }
        }

        WHEN("orders of 50 units are placed at 90.0 and 100.0") {
            book.PlaceOrder(90.0, 50);
            book.PlaceOrder(100.0, 50);

            THEN("there are two orders in the book") {
                REQUIRE(book.order_count() == 2);
            }

            THEN("the orders are consistent") {
                REQUIRE(book.HasOpenOrder(90.0));
                REQUIRE(book.order_size(90.0) == 50);
                REQUIRE(book.order_remaining_volume(90.0) == 50);

                REQUIRE(book.HasOpenOrder(100.0));
                REQUIRE(book.order_size(100.0) == 50);
                REQUIRE(book.order_remaining_volume(100.0) == 50);
            }
        }
    }
}

SCENARIO("handles agent order cancellation", "[Market][Book]") {

    GIVEN("any book with no orders") {
        market::AskBook<1> book;

        WHEN("any order cancellation is attempted") {
            REQUIRE(book.order_count() == 0);

            THEN("nothing happens") {
                REQUIRE(book.order_count() == 0);
            }
        }
    }

    GIVEN("a bid book with a single level (100 units @ 100.0) and an order of 50 units at 100.0") {
        market::BidBook<1> book;

        array<double, 1> prices{100.0};
        array<long, 1> volumes{100};
        book.ApplyChanges(prices, volumes);

        book.PlaceOrderAtLevel(0, 50);

        WHEN("the order at 100.0 is cancelled") {
            book.CancelOrder(100.0);

            THEN("the order no longer exists in the book") {
                REQUIRE(book.order_count() == 0);
                REQUIRE(not book.HasOpenOrder(100.0));
            }
        }

        WHEN("an order cancellation is attempted at another price 90.0") {
            book.CancelOrder(90.0);

            THEN("nothing changes") {
                REQUIRE(book.order_count() == 1);
                REQUIRE(not book.HasOpenOrder(90.0));
                REQUIRE(book.HasOpenOrder(100.0));
            }
        }
    }
}

SCENARIO("handles agent market orders", "[Market][Book]") {
    GIVEN("an ask book with three levels (100 units @ 100.0; 200 @ 110.0; 150 @ 120.0)") {
        market::AskBook<3> book;

        array<double, 3> prices{100.0, 110.0, 120.0};
        array<long, 3> volumes{100, 200, 150};
        book.ApplyChanges(prices, volumes);

        WHEN("the agent's MO arrives that has more volume than in the book") {

            auto out = book.WalkTheBook(90.0, 500);

            THEN("we should abort") {
                REQUIRE(get<0>(out) == 0L);
                REQUIRE(get<1>(out) == 0.0f);
            }
        }

        WHEN("the agent's MO arrives") {

            auto out = book.WalkTheBook(90.0, 400);

            THEN("") {
                REQUIRE(get<0>(out) == 400);
                REQUIRE(get<1>(out) == -(100*10.0 + 200*20.0 + 100*30.0));
            }
        }

        WHEN("a negative MO arrives") {

            auto out = book.WalkTheBook(90.0, -400);

            THEN("we should treat it as positive") {
                REQUIRE(get<0>(out) == 400);
                REQUIRE(get<1>(out) == -(100*10.0 + 200*20.0 + 100*30.0));
            }
        }
    }
}

SCENARIO("handles agent order transactions", "[Market][Book]") {

    // Orders at the best price:
    GIVEN("a bid book with a single level (100 units @ 100.0) and an order of 50 units at 100.0") {
        market::BidBook<1> book;

        array<double, 1> prices{100.0};
        array<long, 1> volumes{100};
        book.ApplyChanges(prices, volumes);

        book.PlaceOrderAtLevel(0, 50);

        WHEN("a market order of 50 units at 100.0 arrives") {
            TMAP transactions{{100.0, 50}};
            auto out = book.ApplyTransactions(transactions, 110.0);

            REQUIRE(get<0>(out) == 0L);
            REQUIRE(get<1>(out) == 0.0);

            THEN("the queue ahead should decrease to 50 units") {
                REQUIRE(book.queue_ahead(100.0) == 50);
            }
        }

        WHEN("a market order of 100 units at 100.0 arrives") {
            TMAP transactions{{100.0, 100}};
            auto out = book.ApplyTransactions(transactions, 110.0);

            REQUIRE(get<0>(out) == 0L);
            REQUIRE(get<1>(out) == 0.0);

            THEN("the order should be at the front of the queue") {
                REQUIRE(book.queue_ahead(100.0) == 0);
            }
        }

        WHEN("a market order of 125 units at 100.0 arrives") {
            TMAP transactions{{100.0, 125}};
            auto out = book.ApplyTransactions(transactions, 110.0);

            REQUIRE(get<0>(out) == 25L);
            REQUIRE(get<1>(out) == 250.0);

            THEN("the order should be partially executed, leaving 25 units") {
                REQUIRE(book.queue_ahead(100.0) == 0);
                REQUIRE(book.order_size(100.0) == 50);
                REQUIRE(book.order_remaining_volume(100.0) == 25);
            }
        }

        WHEN("a market order of 125 units at 100.0 arrives, followed by a further 10 units") {
            TMAP transactions{{100.0, 125}};
            auto out = book.ApplyTransactions(transactions, 110.0);

            REQUIRE(get<0>(out) == 25L);
            REQUIRE(get<1>(out) == 250.0);

            transactions[100.0] = 10;

            out = book.ApplyTransactions(transactions, 105.0);

            REQUIRE(get<0>(out) == 10L);
            REQUIRE(get<1>(out) == 50.0);

            THEN("the order should be partially executed, leaving 15 units") {
                REQUIRE(book.queue_ahead(100.0) == 0);
                REQUIRE(book.order_size(100.0) == 50);
                REQUIRE(book.order_remaining_volume(100.0) == 15);
            }
        }

        WHEN("a market order of 150 units at 100.0 arrives") {
            TMAP transactions{{100.0, 150}};
            auto out = book.ApplyTransactions(transactions, 110.0);

            REQUIRE(get<0>(out) == 50L);
            REQUIRE(get<1>(out) == 500.0);

            THEN("the order should be fully executed") {
                REQUIRE(not book.HasOpenOrder(100.0));
            }
        }

        WHEN("a market order of 1000 units at 100.0 arrives") {
            TMAP transactions{{100.0, 1000}};
            auto out = book.ApplyTransactions(transactions, 110.0);

            REQUIRE(get<0>(out) == 50L);
            REQUIRE(get<1>(out) == 500.0);

            THEN("the order should be fully executed") {
                REQUIRE(book.HasOpenOrder(100.0) == false);
            }
        }
    }

    // Orders at defined levels (not best price):
    GIVEN("an ask book with three levels (100 units @ 100.0; 200 @ 110.0; 150 @ 120.0) and an order of 100 units at 110.0") {
        market::AskBook<3> book;

        array<double, 3> prices{100.0, 110.0, 120.0};
        array<long, 3> volumes{100, 200, 150};
        book.ApplyChanges(prices, volumes);

        book.PlaceOrder(110.0, 100);

        WHEN("a market order of 100 units at 110.0") {
            TMAP transactions{{110.0, 100}};
            auto out = book.ApplyTransactions(transactions, 100.0);

            REQUIRE(get<0>(out) == 0L);
            REQUIRE(get<1>(out) == 0.0);

            THEN("the order should be 100 units behind the front of the queue") {
                REQUIRE(book.order_remaining_volume(110.0) == 100);
                REQUIRE(book.queue_ahead(110.0) == 100);
            }
        }

        WHEN("a market order of 300 units at 110.0") {
            TMAP transactions{{110.0, 300}};
            auto out = book.ApplyTransactions(transactions, 100.0);

            REQUIRE(get<0>(out) == -100L);
            REQUIRE(get<1>(out) == 1000.0);

            THEN("the order should be fully executed") {
                REQUIRE(book.HasOpenOrder(110.0) == false);
            }
        }

        WHEN("a market orders of 200 units at 110.0 and 50 units at 120.0 arrive") {
            TMAP transactions{{110.0, 200}, {120.0, 50}};
            auto out = book.ApplyTransactions(transactions, 100.0);

            REQUIRE(get<0>(out) == -50L);
            REQUIRE(get<1>(out) == 500.0);

            THEN("the order should be partially executed") {
                REQUIRE(book.queue_ahead(110.0) == 0);
                REQUIRE(book.order_remaining_volume(110.0) == 50);
            }
        }

        WHEN("a market orders of 200 units at 110.0 and 100 units at 120.0 arrive") {
            TMAP transactions{{110.0, 200}, {120.0, 100}};
            auto out = book.ApplyTransactions(transactions, 100.0);

            REQUIRE(get<0>(out) == -100L);
            REQUIRE(get<1>(out) == 1000.0);

            THEN("the order should be fully executed") {
                REQUIRE(book.HasOpenOrder(110.0) == false);
            }
        }
    }

    // Orders at better price in bid book:
    GIVEN("a bid book with a single level (100 units @ 100.0) and an order of 50 units at 110.0") {
        market::BidBook<1> book;

        array<double, 1> prices{100.0};
        array<long, 1> volumes{100};
        book.ApplyChanges(prices, volumes);

        book.PlaceOrder(110.0, 50);

        WHEN("a market order of 10 units at 100.0 arrives") {
            TMAP transactions{{100.0, 10}};
            auto out = book.ApplyTransactions(transactions, 111.0);

            REQUIRE(get<0>(out) == 10L);
            REQUIRE(get<1>(out) == 10.0);

            THEN("the order should be partially executed") {
                REQUIRE(book.order_remaining_volume(110.0) == 40);
            }
        }

        WHEN("a market order of 100 units at 100.0 arrives") {
            TMAP transactions{{100.0, 100}};
            auto out = book.ApplyTransactions(transactions, 111.0);

            REQUIRE(get<0>(out) == 50L);
            REQUIRE(get<1>(out) == 50.0);

            THEN("the order should be fully executed") {
                REQUIRE(book.HasOpenOrder(110.0) == false);
            }
        }
    }

    // Orders at better price in ask book:
    GIVEN("an ask book with a single level (100 units @ 100.0) and an order of 50 units at 90.0") {
        market::AskBook<1> book;

        array<double, 1> prices{100.0};
        array<long, 1> volumes{100};
        book.ApplyChanges(prices, volumes);

        book.PlaceOrder(90.0, 50);

        WHEN("a market order of 10 units at 100.0 arrives") {
            TMAP transactions{{100.0, 10}};
            auto out = book.ApplyTransactions(transactions, 85.0);

            REQUIRE(get<0>(out) == -10L);
            REQUIRE(get<1>(out) == 50.0);

            THEN("the order should be partially executed") {
                REQUIRE(book.order_remaining_volume(90.0) == 40);
            }
        }

        WHEN("a market order of 100 units at 100.0 arrives") {
            TMAP transactions{{100.0, 100}};
            auto out = book.ApplyTransactions(transactions, 85.0);

            REQUIRE(get<0>(out) == -50L);
            REQUIRE(get<1>(out) == 250.0);

            THEN("the order should be fully executed") {
                REQUIRE(book.HasOpenOrder(90.0) == false);
            }
        }
    }

    // Order at undefined price level:
    GIVEN("a bid book with two levels (100 units @ 100.0; 200 units @ 110.0) and an order of 50 units at 105.0") {
        market::AskBook<2> book;

        array<double, 2> prices{100.0, 110.0};
        array<long, 2> volumes{100, 200};
        book.ApplyChanges(prices, volumes);

        book.PlaceOrder(105.0, 50);

        WHEN("an aggregate market order of 150 units arrives, with 50 units above 105.0") {
            TMAP transactions{{100.0, 100}, {110.0, 50}};
            auto out = book.ApplyTransactions(transactions, 100.0);

            REQUIRE(get<0>(out) == -50L);
            REQUIRE(get<1>(out) == 250.0);

            THEN("the order should be fully executed") {
                REQUIRE(book.HasOpenOrder(105.0) == false);
            }
        }
    }

    // Multiple orders at undefined price levels:
    GIVEN("a bid book with two levels (100 units @ 100.0; 200 units @ 110.0) and an order of 50 units at 105.0 and 50 units at 107.0") {
        market::AskBook<2> book;

        array<double, 2> prices{100.0, 110.0};
        array<long, 2> volumes{100, 200};
        book.ApplyChanges(prices, volumes);

        book.PlaceOrder(105.0, 50);
        book.PlaceOrder(107.0, 50);

        WHEN("an aggregate market order of 200 units arrives, with 100 units above 110.0") {
            TMAP transactions{{100.0, 100}, {110.0, 100}};
            auto out = book.ApplyTransactions(transactions, 100.0);

            REQUIRE(get<0>(out) == -100L);
            REQUIRE(get<1>(out) == (5.0*50 + 7.0*50));

            THEN("both orders should be fully executed") {
                REQUIRE(book.HasOpenOrder(105.0) == false);
                REQUIRE(book.HasOpenOrder(107.0) == false);
            }
        }
    }
}

SCENARIO("book is robust to double point prices: precision to 4dp", "[Market][Book]") {

    GIVEN("a book with 1 level") {
        market::AskBook<1> book;

        WHEN("the book is updated with price: 1.1111") {
            array<double, 1> prices{1.1111};
            array<long, 1> volumes{100};

            book.ApplyChanges(prices, volumes);

            THEN("it should match the price: 1.1111") {
                REQUIRE(book.price_level(1.1111) == 0);
            }

            THEN("it should match the price: 1.11114") {
                REQUIRE(book.price_level(1.11114) == 0);
            }

            THEN("it should not match the price: 1.11117") {
                REQUIRE(book.price_level(1.11117) == -1);
            }

            THEN("it shouldn't match 1.1110 or 1.1112") {
                REQUIRE(book.price_level(1.1110) == -1);
                REQUIRE(book.price_level(1.1112) == -1);
            }
        }
    }

    GIVEN("a ask book with a single level (100 units @ 1.1111) and an order of 50 units at 1.1111") {
        market::AskBook<1> book;

        array<double, 1> prices{1.1111};
        array<long, 1> volumes{100};
        book.ApplyChanges(prices, volumes);

        book.PlaceOrderAtLevel(0, 50);

        THEN("there is an open order at 1.1110") {
            REQUIRE(book.HasOpenOrder(1.1110) == false);
        }

        THEN("there is an open order at 1.1111") {
            REQUIRE(book.HasOpenOrder(1.1111) == true);
        }

        THEN("there is an open order at 1.11114") {
            REQUIRE(book.HasOpenOrder(1.11114) == true);
        }

        THEN("there is not open order at 1.11117") {
            REQUIRE(book.HasOpenOrder(1.11117) == false);
        }

        THEN("there is an open order at 1.1112") {
            REQUIRE(book.HasOpenOrder(1.1112) == false);
        }

        WHEN("a market order of 150 units at 1.1110 arrives") {
            TMAP transactions{{1.1110, 150}};
            book.ApplyTransactions(transactions, 0.0);

            THEN("the order should not be executed") {
                REQUIRE(book.HasOpenOrder(1.1111) == true);
            }
        }

        WHEN("a market order of 150 units at 1.1111 arrives") {
            TMAP transactions{{1.1111, 150}};
            book.ApplyTransactions(transactions, 0.0);

            THEN("the order should be fully executed") {
                REQUIRE(book.HasOpenOrder(1.1111) == false);
            }
        }

        WHEN("a market order of 150 units at 1.1112 arrives") {
            TMAP transactions{{1.1112, 150}};
            book.ApplyTransactions(transactions, 0.0);

            THEN("the order should be fully executed") {
                REQUIRE(book.HasOpenOrder(1.1111) == false);
            }
        }
    }
}
