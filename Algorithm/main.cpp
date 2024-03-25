#include <fstream>
#include "Tests.cpp"
#include <unordered_map>
#include <vector>
#include <set>

enum class TradeStatus
{
    BUY,
    SELL
};

using namespace std;

int main()
{
    //TestCalcIncome();
    //TestCalcIncome1();
    TestCalcIncome2();

    cout << "end of tests" << endl;
    return 0;
}
