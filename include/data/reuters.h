// #ifndef DATA_REUTERS_H
// #define DATA_REUTERS_H

// #include "data/streamer.h"
// #include "utilities/csv.h"

// #include <map>
// #include <string>
// #include <vector>
// #include <utility>

// namespace data {
// namespace reuters {

// class MarketDepth: public data::MarketDepth
// {
    // enum Columns
    // {
        // RIC = 0,
        // DATE = 1,
        // TIME = 2,
        // GMT_OFFSET = 3,
        // TYPE = 4,
        // FID_ID = 5,
        // MSG_TYPE = 6,
        // FID_NAME = 7,
        // FID_VAL = 8,
        // FID_ENUM = 9,
        // PE_CODE = 10,
        // TEMPLATE_ID = 11,
        // RTL = 12,
        // SEQUENCE_ID = 13,
        // RIC_SRC = 14,
        // N_FIDS = 15
    // };

    // private:
        // CSV csv;

        // void parseFids(int n_fids);

    // public:
        // MarketDepth();
        // MarketDepth(string file_path);

        // void load(string path);

        // bool next();

        // long peekTime();
        // string peekDate();
// };

// class TimeAndSales: public data::TimeAndSales
// {
    // enum Columns
    // {
        // RIC = 0,
        // DATE = 1,
        // TIME = 2,
        // GMT_OFFSET = 3,
        // TYPE = 4,
        // FID_ID = 5,
        // MSG_TYPE = 6,
        // FID_NAME = 7,
        // FID_VAL = 8,
        // FID_ENUM = 9,
        // PE_CODE = 10,
        // TEMPLATE_ID = 11,
        // RTL = 12,
        // SEQUENCE_ID = 13,
        // RIC_SRC = 14,
        // N_FIDS = 15
    // };

    // private:
        // CSV csv;

        // void parseFids(int n_fids);

    // public:
        // TimeAndSales();
        // TimeAndSales(string file_path);

        // void load(string path);

        // bool next();
        // bool next(string to_date, long to_time);
        // bool skip(string to_date, long to_time);

        // long peekTime();
        // string peekDate();
// };

// }
// }

// #endif
