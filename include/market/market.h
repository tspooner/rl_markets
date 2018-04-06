#ifndef MARKET_MARKET_H
#define MARKET_MARKET_H

#include <map>
#include <string>

#include "market/currency.h"


namespace market {

// Base class:
class Market
{
    protected:
        int date_;
        long time_;

        std::string symbol_;
        const Currency currency_;

        const long mo_, mc_;
        const std::map<double, double> pts_;

        std::map<int, double> tts_;

        Market(std::string symbol, Currency c,
               long mo, long mc,
               std::map<double, double> pts);

    public:
        // Accepts arguments of the form "SYMBOL.VENUE":
        static Market* make_market(std::string symbol, std::string venue);

        int date();
        void set_date(int new_date);

        long time();
        void set_time(long new_time);

        bool IsOpen();

        std::string symbol();
        Currency currency();

        long open_time();
        long close_time();

        double tick_size(double price);

        int ToTicks(double price);
        double ToPrice(int ticks);
};


// Intermediate subclasses:
class Euronext: public Market {
    protected: Euronext(std::string symbol, Currency c, long mo, long mc);
};

class NasdaqNordic: public Market {
    protected: NasdaqNordic(std::string symbol, Currency c, long mo, long mc);
};


// Concrete subclasses:
class AmsterdamStockExchange: public Euronext {
    public: AmsterdamStockExchange(std::string symbol);
};

class BrusselsStockExchange: public Euronext {
    public: BrusselsStockExchange(std::string symbol);
};

class CopenhagenStockExchange: public NasdaqNordic {
    public: CopenhagenStockExchange(std::string symbol);
};

class DeutscheBorseXetra: public Market {
    public: DeutscheBorseXetra(std::string symbol);
};

class HelsinkiStockExchange: public NasdaqNordic {
    public: HelsinkiStockExchange(std::string symbol);
};

class IrishStockExchange: public Market {
    public: IrishStockExchange(std::string symbol);
};

class LondonStockExchange: public Market
{
    public: LondonStockExchange(std::string symbol);

    static std::map<double, double> pts_(std::string symbol);
};

class MadridStockExchange: public Market {
    public: MadridStockExchange(std::string symbol);
};

class MilanStockExchange: public Market {
    public: MilanStockExchange(std::string symbol);
};

class OsloStockExchange: public Market {
    public: OsloStockExchange(std::string symbol);
};

class ParisStockExchange: public Euronext {
    public: ParisStockExchange(std::string symbol);
};

class StockholmStockExchange: public NasdaqNordic {
    public: StockholmStockExchange(std::string symbol);
};

class SwissExchange: public Market {
    public: SwissExchange(std::string symbol);
};

class ViennaStockExchange: public Market {
    public: ViennaStockExchange(std::string symbol);
};

}

#endif
