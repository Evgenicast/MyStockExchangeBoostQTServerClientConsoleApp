#ifndef CLIENSERVERECN_COMMON_HPP
#define CLIENSERVERECN_COMMON_HPP

#include <string>
#include <iostream>

static short port = 5555;

namespace Requests
{
    static std::string Registration = "Reg";
    static std::string Hello = "Hel";
    static std::string Sell = "Sell";
    static std::string Buy = "Buy";
    static std::string Balance = "Bala";
}

enum class TradeStatus
{
    BUY,
    SELL
};
#endif //CLIENSERVERECN_COMMON_HPP
