#include "market/order.h"

#include <math.h>
#include <string>
#include <iostream>
#include <stdexcept>


using namespace std;
using namespace market;

Order::Order(double price, long size, long q_head):
    q_head(q_head),
    q_tail(0),

    id(Order::id_counter_),

    price(price),
    size(size),
    initial_queue(q_head)
{
    if (price <= 0)
        throw runtime_error("Order price must be non-zero and positive.");
    if (size <= 0)
        throw runtime_error("Order size must be non-zero and positive.");
    if (q_head < 0)
        throw runtime_error("Order queue must be positive.");

    Order::id_counter_++;
}

long Order::id_counter_ = 0;

long Order::remaining()
{
    return max(size - total_executed, 0L);
}

long Order::getTotalExecutedVolume()
{
    return total_executed;
}

long Order::getTotalTransactedVolume()
{
    return transactions;
}

bool Order::isExecuted()
{
    return total_executed >= size;
}

long Order::doTransaction(long volume)
{
    if (volume < 0)
        throw runtime_error("Transaction volume must be positive.");

    transactions += volume;

    // Update transactions ahead
    long remaining_volume = volume - q_head;
    if (remaining_volume > 0) {
        // Transaction volume reached our order
        q_head = 0;

        if (remaining() <= remaining_volume) {
            // Fully executed our order
            total_executed = size;
            remaining_volume -= size;
        } else {
            // Partially executed our order
            total_executed += remaining_volume;
            remaining_volume = 0;
        }
    } else {
        // Only executed ahead of our order
        q_head -= volume;
    }

    return max(remaining_volume, 0L);
}

void Order::doCancellation(long volume)
{
    if (volume < 0)
        throw runtime_error("Cancellation volume must be positive.");

    if (q_tail == 0) {
        q_head -= volume;
    } else {
        // Uniform scheme
        double total = q_head + q_tail;

        q_head -= ceil(volume * q_head / total);
        q_tail -= floor(volume * q_tail / total);
    }

    // Correct for over-cancellations ahead and behind
    if (q_head < 0) {
        q_tail += q_head;
        q_head = 0;
    }

    if (q_tail < 0)
        q_tail = 0;
}

void Order::addVolumeBehind(long volume)
{
    q_tail += volume;
}

void Order::clearQueues()
{
    q_head = 0;
    q_tail = 0;
}

long Order::getQueueAhead()
{
    return q_head;
}

long Order::getQueueBehind()
{
    return q_tail;
}

float Order::getQueueProgress()
{
    return q_head / max(1.0f, (float) initial_queue);
}

std::string Order::ToString()
{
    return std::string("Order(price=" + to_string(price) +
                       ", size=" + to_string(size) +
                       ", rem=" + to_string(remaining()) +
                       ", q_head=" + to_string(q_head) + ")");
}
