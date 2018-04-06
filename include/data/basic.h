#ifndef DATA_BASIC_H
#define DATA_BASIC_H

#include "data/streamer.h"
#include "utilities/csv.h"

#include <map>
#include <string>
#include <vector>
#include <utility>

namespace data {
namespace basic {

class MarketDepth: public data::MarketDepth
{
    enum Columns
    {
        DATE, TIME,
        AP1, AP2, AP3, AP4, AP5,
        AV1, AV2, AV3, AV4, AV5,
        BP1, BP2, BP3, BP4, BP5,
        BV1, BV2, BV3, BV4, BV5
    };

    private:
        CSV csv_;
        vector<string> row_;

        bool _LoadRow();
        bool _ParseRow();

        bool _LoadNext();

        long _TimeLookAhead();

    public:
        MarketDepth();
        MarketDepth(string file_path);

        void LoadCSV(string path);

        void Reset();
        void SkipN(long n = 1L);
};

class TimeAndSales: public data::TimeAndSales
{
    enum Columns
    {
        DATE, TIME, PRICE, SIZE
    };

    private:
        CSV csv_;
        vector<string> row_;

        bool _LoadRow();
        bool _ParseRow();

        bool _LoadNext();

        long _TimeLookAhead();

    public:
        TimeAndSales();
        TimeAndSales(string file_path);

        void LoadCSV(string path);

        void Reset();
        void SkipN(long n = 1L);
};

}
}

#endif
