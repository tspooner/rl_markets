#ifndef DATA_STREAMER_H
#define DATA_STREAMER_H

#include <map>
#include <string>
#include <vector>

#include "data/records.h"
#include "utilities/csv.h"

using namespace std;

namespace data
{

template<typename R>
class Streamer
{
    protected:
        R record_1;
        R record_2;
        R record_3;

        R& record_last;
        R& record_curr;
        R& record_next;

        Streamer();

        void _RotateRefs();

        virtual bool _LoadNext() = 0;
        virtual long _TimeLookAhead() = 0;

    public:
        bool LoadNext();

        bool LoadUntil(const data::Record& rec);
        bool LoadUntil(int date, long time);

        bool SkipUntil(const data::Record& rec);
        bool SkipUntil(int date, long time = 0);

        const R& Record();

        virtual void Reset();
        virtual void SkipN(long n = 1L) = 0;

        bool HasTimeChanged();
        bool WillTimeChange();

        int NextDate();
        long NextTime();
};

class MarketDepth: public Streamer<MarketDepthRecord> {};
class TimeAndSales: public Streamer<TimeAndSalesRecord> {};

}

#endif
