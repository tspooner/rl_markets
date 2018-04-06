#include "data/basic.h"
#include "utilities/time.h"

#include <iostream>

using namespace std;
using namespace data::basic;

MarketDepth::MarketDepth():
    data::MarketDepth(),
    csv_()
{}

MarketDepth::MarketDepth(string file_path):
    MarketDepth()
{
    LoadCSV(file_path);
}

void MarketDepth::LoadCSV(string path)
{
    Reset();

    csv_.openFile(path);
    csv_.skip(1);        // Ignore header

    LoadNext();
}

bool MarketDepth::_LoadRow()
{
    while (row_.size() != 22) {
        if (not csv_.hasData())
            return false;

        csv_.next(row_);
    }

    if (row_.size() == 22)
        return true;
    else
        return false;
}

bool MarketDepth::_ParseRow()
{
    record_next.date = stoi(row_.at(DATE));
    record_next.time = string_to_time(row_.at(TIME));

    for (int i = 0; i < 5; i++) {
        double nap = stof(row_.at(AP1 + i)),
               nbp = stof(row_.at(BP1 + i));

        if (nap <= 0.0 or nbp <= 0.0) {
            row_.clear();

            return false;
        }

        record_next.ask_prices[i] = nap;
        record_next.ask_volumes[i] = stol(row_.at(AV1 + i));

        record_next.bid_prices[i] = nbp;
        record_next.bid_volumes[i] = stol(row_.at(BV1 + i));
    }

    row_.clear();

    return true;
}

bool MarketDepth::_LoadNext()
{
    if (not csv_.isOpen())
        return false;

    while (true) {
        if (not _LoadRow())
            break;

        if (_ParseRow())
            return true;
    }

    return false;
}

long MarketDepth::_TimeLookAhead()
{
    if (not _LoadRow())
        return -1;
    else
        return string_to_time(row_.at(TIME));
}

void MarketDepth::Reset()
{
    Streamer::Reset();

    csv_.closeFile();
    row_.clear();
}

void MarketDepth::SkipN(long n)
{
    csv_.skip(n);
}

// ------------------------------------------------------------------

TimeAndSales::TimeAndSales():
    data::TimeAndSales(),

    csv_()
{}

TimeAndSales::TimeAndSales(string file_path):
    data::TimeAndSales()
{
    LoadCSV(file_path);
}

void TimeAndSales::LoadCSV(string path)
{
    Reset();

    csv_.openFile(path);
    csv_.skip(1);

    LoadNext();
}

bool TimeAndSales::_LoadRow()
{
    while (row_.size() != 4) {
        if (not csv_.hasData())
            return false;

        csv_.next(row_);
    }

    if (row_.size() == 4)
        return true;
    else
        return false;
}

bool TimeAndSales::_ParseRow()
{
    record_next.date = stoi(row_.at(DATE));
    record_next.time = string_to_time(row_.at(TIME));

    double price = stof(row_.at(PRICE));
    long size = stol(row_.at(SIZE));

    if (price > 0.0 and size > 0)
        record_next.transactions[price] += size;

    row_.clear();

    return true;
}

bool TimeAndSales::_LoadNext()
{
    if (not csv_.isOpen() or not _LoadRow())
        return false;

    // Parse the row_
    long la = 0;
    long target = _TimeLookAhead();
    do {
        if (la == -1 or (not _ParseRow()))
            return false;

        la = _TimeLookAhead();

    } while (la <= target);

    return true;
}

long TimeAndSales::_TimeLookAhead()
{
    if (not _LoadRow())
        return -1;
    else
        return string_to_time(row_.at(TIME));
}

void TimeAndSales::Reset()
{
    Streamer::Reset();

    csv_.closeFile();
    row_.clear();
}

void TimeAndSales::SkipN(long n)
{
    csv_.skip(n);
}
