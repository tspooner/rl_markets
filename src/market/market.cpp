#include "market/market.h"
#include "utilities/time.h"

#include <iostream>
#include <algorithm>
#include <exception>

using namespace market;


Market::Market(std::string symbol, Currency c,
               long mo, long mc,
               std::map<double, double> pts):
    date_(0),
    time_(0),

    symbol_(symbol),
    currency_(c),

    mo_(mo),
    mc_(mc),

    pts_(pts)
{
    std::transform(symbol_.begin(), symbol_.end(), symbol_.begin(), ::toupper);

    // Populate tts_ list:
    tts_[0] = pts.begin()->second;

    long acc_ticks = 0;
    for (auto it = next(pts_.begin()); it != pts_.end(); it++) {
        auto pit = prev(it);
        acc_ticks += (it->first - pit->first) / pit->second;

        tts_[acc_ticks] = it->second;
    }
}

Market* Market::make_market(std::string symbol, std::string venue)
{
    std::transform(venue.begin(), venue.end(), venue.begin(), ::toupper);

    if (venue == "AS") return new AmsterdamStockExchange(symbol);
    else if (venue == "BR") return new BrusselsStockExchange(symbol);
    else if (venue == "CO") return new CopenhagenStockExchange(symbol);
    else if (venue == "DE") return new DeutscheBorseXetra(symbol);
    else if (venue == "HE") return new HelsinkiStockExchange(symbol);
    else if (venue == "I") return new IrishStockExchange(symbol);
    else if (venue == "L") return new LondonStockExchange(symbol);
    else if (venue == "MC") return new MadridStockExchange(symbol);
    else if (venue == "MI") return new MilanStockExchange(symbol);
    else if (venue == "OL") return new OsloStockExchange(symbol);
    else if (venue == "PA") return new ParisStockExchange(symbol);
    else if (venue == "S" or venue == "VX") return new SwissExchange(symbol);
    else if (venue == "ST") return new StockholmStockExchange(symbol);
    else if (venue == "VI") return new ViennaStockExchange(symbol);
    else
        throw std::invalid_argument("[Market] Unknown exchange venue \"" + venue + "\".");
}

int Market::date() { return date_; }
void Market::set_date(int new_date) { date_ = new_date; }

long Market::time() { return time_; }
void Market::set_time(long new_time) { time_ = new_time; }

bool Market::IsOpen()
{
    return (time_ > add_minutes(30, mo_)) and (time_ < add_minutes(-30, mc_));
}

std::string Market::symbol() { return symbol_; }
Currency Market::currency() { return currency_; }

long Market::open_time() { return mo_; }
long Market::close_time() { return mc_; }

int Market::ToTicks(double price)
{
    int ticks = 0;

    double ub;
    auto it = pts_.begin();

    if (price < it->first)
        throw std::invalid_argument("[Market] Invalid price " + std::to_string(price) + " for tick conversion.");

    while (price+tick_size(it->first)/2.0 > it->first and it != pts_.end()) {
        double np = next(it)->first;

        if (it == prev(pts_.end()) or price < np)
            ub = price+tick_size(price)/2.0;
        else
            ub = np;

        ticks += (ub - it->first) / it->second;

        it++;
    }

    return ticks;
}

double Market::ToPrice(int ticks)
{
    double price = 0;

    double ub;
    auto it = tts_.begin();

    if (ticks < it->first)
        throw std::invalid_argument("[Market] Invalid number of ticks " + std::to_string(ticks) + " for price conversion.");

    while (ticks > it->first and it != tts_.end()) {
        double nt = next(it)->first;

        if (it == prev(tts_.end()) or ticks < nt)
            ub = ticks;
        else
            ub = nt;

        price += (ub - it->first) * it->second;

        it++;
    }

    return price;
}

double Market::tick_size(double price)
{
    auto it = prev(pts_.upper_bound(price));

    if (it == prev(pts_.begin()))
        throw std::invalid_argument("[Market] Invalid price " + std::to_string(price) + " for tick conversion.");

    return it->second;
}


// Intermediate:
Euronext::Euronext(std::string symbol, Currency c, long mo, long mc):
    Market(symbol, c, mo, mc, {
            {100., 0.05},
            {50., 0.01},
            {10., 0.005},
            {0., 0.001}
           })
{}

NasdaqNordic::NasdaqNordic(std::string symbol, Currency c, long mo, long mc):
    Market(symbol, c, mo, mc, {
            {100000., 100.},
            {80000., 80.},
            {50000., 50.},
            {40000., 40.},
            {20000., 20.},
            {10000., 10.},
            {5000., 5.},
            {1000., 1.},
            {500., 0.5},
            {100., 0.1},
            {50., 0.05},
            {10., 0.01},
            {5., 0.005},
            {2., 0.002},
            {1., 0.001},
            {0.5, 0.0005},
            {0., 0.0001}
           })
{}


// Concrete:
AmsterdamStockExchange::AmsterdamStockExchange(std::string symbol):
    Euronext(symbol, EUR, add_hours(9, 0), add_hours(17, add_minutes(40, 0)))
{}

BrusselsStockExchange::BrusselsStockExchange(std::string symbol):
    Euronext(symbol, EUR, add_hours(9, 0), add_hours(17, add_minutes(40, 0)))
{}

CopenhagenStockExchange::CopenhagenStockExchange(std::string symbol):
    NasdaqNordic(symbol, SEK, add_hours(9, 0), add_hours(17, 0))
{}

DeutscheBorseXetra::DeutscheBorseXetra(std::string symbol):
    Market(symbol, EUR, add_hours(9, 0), add_hours(17, add_minutes(30, 0)), {
            {100., 0.05}, {50., 0.01}, {10., 0.005}, {0., 0.001}
           })
{}

HelsinkiStockExchange::HelsinkiStockExchange(std::string symbol):
    NasdaqNordic(symbol, EUR, add_hours(10, 0), add_hours(16, add_minutes(30, 0)))
{}

IrishStockExchange::IrishStockExchange(std::string symbol):
    Market(symbol, EUR, add_hours(8, 0), add_hours(16, add_minutes(16, 40)), {
            {100., 0.05},
            {50., 0.01},
            {10., 0.005},
            {0., 0.001}
           })
{}

LondonStockExchange::LondonStockExchange(std::string symbol):
    Market(symbol, GBp,
           add_hours(8, 0), add_hours(16, add_minutes(30, 0)),
           LondonStockExchange::pts_(symbol))
{}

std::map<double, double> LondonStockExchange::pts_(std::string symbol)
{
    if (symbol == "AAL" or symbol == "BATS" or symbol == "GSK" or
        symbol == "VOD" or symbol == "HSBA")
        return {
            {10000., 5},
            {5000., 1},
            {1000., 0.5},
            {500., 0.1},
            {100., 0.05},
            {50., 0.01},
            {10., 0.005},
            {5., 0.001},
            {1., 0.0005},
            {0., 0.0001}
        };
    else if (symbol == "BAES" or symbol == "UU" or symbol == "LGEN" or
             symbol == "LSE" or symbol == "NXT")
        return {
            {10000., 10},
            {5000., 5},
            {1000., 1},
            {500., 0.5},
            {100., 0.1},
            {50., 0.05},
            {10., 0.01},
            {5., 0.005},
            {1., 0.001},
            {0.5, 0.0005},
            {0., 0.0001}
        };
    else
        throw std::invalid_argument("[LondonStockExchange] Unknown symbol \"" + symbol + "\".");
}

MadridStockExchange::MadridStockExchange(std::string symbol):
    Market(symbol, EUR, add_hours(9, 0), add_hours(17, add_minutes(30, 0)), {
            {100., 0.05}, {50., 0.01}, {10., 0.005}, {0., 0.001}
           })
{}

MilanStockExchange::MilanStockExchange(std::string symbol):
    Market(symbol, EUR, add_hours(9, 0), add_hours(17, add_minutes(25, 0)), {
            {50., 0.01},
            {5., 0.005},
            {2., 0.0025},
            {1., 0.001},
            {0.25, 0.0005},
            {0., 0.0001}
           })
{}

OsloStockExchange::OsloStockExchange(std::string symbol):
    Market(symbol, NOK, add_hours(9, 0), add_hours(16, add_minutes(30, 0)), {
            {100000., 100.},
            {80000., 80.},
            {50000., 50.},
            {40000., 40.},
            {20000., 20.},
            {10000., 10.},
            {5000., 5.},
            {1000., 1.},
            {500., 0.5},
            {100., 0.1},
            {50., 0.05},
            {10., 0.01},
            {5., 0.005},
            {2., 0.002},
            {1., 0.001},
            {0.5, 0.0005},
            {0., 0.0001}
           })
{}

ParisStockExchange::ParisStockExchange(std::string symbol):
    Euronext(symbol, EUR, add_hours(9, 0), add_hours(17, add_minutes(30, 0)))
{}

StockholmStockExchange::StockholmStockExchange(std::string symbol):
    NasdaqNordic(symbol, SEK, add_hours(9, 0), add_hours(17, add_minutes(30, 0)))
{}

SwissExchange::SwissExchange(std::string symbol):
    Market(symbol, CHF, add_hours(9, 0), add_hours(17, add_minutes(30, 0)), {
            {10000., 10.},
            {5000., 5.},
            {1000., 1.},
            {500., 0.5},
            {100., 0.1},
            {50., 0.05},
            {10., 0.01},
            {5., 0.005},
            {1., 0.001},
            {0.5, 0.0005},
            {0., 0.0001}
           })
{}

ViennaStockExchange::ViennaStockExchange(std::string symbol):
    Market(symbol, EUR, add_hours(9, 0), add_hours(17, add_minutes(30, 0)), {
            {100., 0.5}, {50., 0.01}, {10., 0.005}, {0., 0.001}
           })
{}
