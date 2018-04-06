#ifndef MARKET_ORDER_H
#define MARKET_ORDER_H

#include <memory>
#include <string>

namespace market {

struct Order
{
    private:
        static long id_counter_;

        int transactions = 0;
        long total_executed = 0;

        long q_head;
        long q_tail;

    public:
        const long id;

        const double price;
        const long size;

        const long initial_queue;

        Order(double price, long size, long q_head);

        long remaining();
        long getLastExecutedVolume();
        long getTotalExecutedVolume();
        long getTotalTransactedVolume();

        bool isExecuted();

        long doTransaction(long volume);
        void doCancellation(long volume);
        void addVolumeBehind(long volume);

        void clearQueues();
        long getQueueAhead();
        long getQueueBehind();

        float getQueueProgress();

        std::string ToString();
};

typedef std::unique_ptr<Order> OrderPtr;

}

#endif
