/* 0.0

ПРИНЦИП РАБОТЫ

Для обработки входящих запросов Клиента используется структура struct ClientsLog, в которой содержится вся информация о «кошельке» и действиях продавца/пользователя.
Данные всех Клиентов хранятся в хэш-таблице std::unordered_map<std::string_view, ClientsLog> ClientsBalanceStruct; прим. Неудачное название, но для отладки приемлемо.
В основной программе контейнер носит название m_OperationsLog.
Операции «биржи» происходят через std::unordered_map<int, std::vector<std::pair<std::string_view, int>>> StockData, где ключ =  выставляемая цена за доллар в рублях, пара = ID(имя),
количество долларов на покупку.
Чтобы итерироваться по наибольшей цене в рублях при сделке, используется буфер индексов(ключей) StockData,
который отсортирован в убывающем порядке и не содержит дубликатов.  Если по выставленной цене в рублях есть несколько предложений в долларах,
то во внутреннем вектор хранятся данные по мере заполнения. При расчетах берутся значения, которые пришли раньше.

ДОКАЗАТЕЛЬСТВО КОРРЕКТНОСТИ
Поскольку данные приходят пакетами, то нужно предусмотреть проверку при их потере во время передачи. Поэтому значения долларов и рублей «обернуты» в std::optional,
и в основном алгоритме происходит проверка валидности данных. Далее, если значение в рублях на продажу больше первой цены в контейнере индексов (он отсортирован по убыванию),
то сделка невозможна, в баланс продавца записываются данные по доллару и рублю.

В другом случае доллары и рубли записываются в ClientsBalanceStruct_ по индексу продавца.
Расчет сделок и запись результатов производится в одном цикле по индексам цены в рублях. Если баланс долларов у продавца положительный (  > 0 )
и цена в рублях покупателей больше (или равна), чем у продавца, то:
1)	В случае, если по одной цене только одно предложение, то производится сделка между покупателями и продавцами. Данные записываются ClientsBalanceStruct по индексу покупатели и продавца.
2)	В случае, если по одной цене несколько предложений, то начинается обход вектора с данными о предложении в долларах и проходит сделка по порядку (с наиболее ранних) предложений.
    Данные записываются ClientsBalanceStruct по индексу покупатели и продавца.

В конце происходит подсчет прибыли продавца и запись по индексу в ClientsBalanceStruct.

ВРЕМЕННАЯ СЛОЖНОСТЬ

Асимптотика выполнения алгоритма зависит количества предложений в рублях и долларах.
В худшем случае придется обрабатывать значения по цене в рублях и внутреннем цикле, если предложений на одну цену больше одного,
т.е временная сложность равна O((n – k) * j ), где n = количество предложений в рублях, k = дубликаты среди n,
j = количество предложений в долларах на одну цену, если больше одного.

ПРОСТРАНСТВЕННАЯ СЛОЖНОСТЬ

В памяти приходится хранить данные о пользователях, долларах и цены в рублях.
Асимптотика равна О(n + k + j), где n = пользователи, k = информация о долларах, j = цены в рублях
*/



#include <cassert>
#include <iostream>
#include <map>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>

struct ClientsLog
{
    int income_doll = 0;
    int income_rub = 0;
    int spent_doll = 0;
    int spent_rub = 0;
    int balance_doll = 0;
    int balance_rub = 0;
    int active_doll_balance = 0;
    int active_rub_balance = 0;
};

void CalcIncome2(std::unordered_map<int, std::vector<std::pair<std::string_view, int>>> & StockData_,
                 const std::set<int, std::greater<int>> & indecies_, std::optional<int> doll_, std::optional<int> rub_,
                 const std::string_view & seller_,
                 std::unordered_map<std::string_view, ClientsLog> & ClientsBalanceStruct_)
{
    if(!doll_.has_value() || !rub_.has_value())
    {
        std::cout << "Error! Data lost, no doll or rub value found!" << std::endl;
        return;
    }

    if(*indecies_.cbegin() < rub_.value())
    {
        ClientsBalanceStruct_[seller_].balance_doll = doll_.value();
        ClientsBalanceStruct_[seller_].active_doll_balance = doll_.value();
        ClientsBalanceStruct_[seller_].active_rub_balance = rub_.value();
        return;
    }

    int seller_income = 0;
    ClientsBalanceStruct_[seller_].balance_doll = doll_.value();
    ClientsBalanceStruct_[seller_].balance_rub = rub_.value();

    for(const auto price_rub : indecies_)
    {
        if(price_rub >= ClientsBalanceStruct_[seller_].balance_rub && ClientsBalanceStruct_[seller_].balance_doll > 0)
        {
            auto & id_doll_vec = StockData_[price_rub];
            if(id_doll_vec.size() == 1)
            {
                if(id_doll_vec.back().second < ClientsBalanceStruct_[seller_].balance_doll)
                {
                    seller_income += price_rub * id_doll_vec.back().second;
                    ClientsBalanceStruct_[id_doll_vec.back().first].income_doll += id_doll_vec.back().second;
                    ClientsBalanceStruct_[id_doll_vec.back().first].spent_rub -= price_rub * id_doll_vec.back().second;
                    ClientsBalanceStruct_[id_doll_vec.back().first].active_doll_balance = 0; // продадутся все доллары
                    ClientsBalanceStruct_[id_doll_vec.back().first].active_rub_balance = 0; // цена в рублях тоже не актуальна

                    ClientsBalanceStruct_[seller_].balance_doll -= id_doll_vec.back().second;
                }
                else
                {
                    seller_income += price_rub * ClientsBalanceStruct_[seller_].balance_doll;
                    ClientsBalanceStruct_[id_doll_vec.back().first].active_doll_balance = id_doll_vec.back().second - ClientsBalanceStruct_[seller_].balance_doll;
                    ClientsBalanceStruct_[id_doll_vec.back().first].income_doll += ClientsBalanceStruct_[seller_].balance_doll;
                    ClientsBalanceStruct_[id_doll_vec.back().first].balance_doll += ClientsBalanceStruct_[seller_].balance_doll;
                    ClientsBalanceStruct_[id_doll_vec.back().first].spent_rub -= price_rub * ClientsBalanceStruct_[seller_].balance_doll;
                    ClientsBalanceStruct_[seller_].balance_doll = 0; // поскольку долларов на покупку больше, то будут куплены все оставшиеся
                }
            }
            else
            {
                for(auto & it : id_doll_vec)
                {
                    if(it.second < ClientsBalanceStruct_[seller_].balance_doll)
                    {
                        seller_income += price_rub * it.second;
                        ClientsBalanceStruct_[it.first].income_doll += it.second;
                        ClientsBalanceStruct_[it.first].spent_rub -= price_rub * it.second;
                        ClientsBalanceStruct_[it.first].active_doll_balance = 0; // продадутся все доллары
                        ClientsBalanceStruct_[it.first].active_rub_balance = 0; // цена в рублях тоже не актуальна

                        ClientsBalanceStruct_[seller_].balance_doll -= it.second;
                    }
                    else
                    {
                        seller_income += price_rub * ClientsBalanceStruct_[seller_].balance_doll;
                        ClientsBalanceStruct_[it.first].active_doll_balance = it.second - ClientsBalanceStruct_[seller_].balance_doll;
                        ClientsBalanceStruct_[it.first].income_doll += ClientsBalanceStruct_[seller_].balance_doll;
                        ClientsBalanceStruct_[it.first].balance_doll += ClientsBalanceStruct_[seller_].balance_doll;
                        ClientsBalanceStruct_[it.first].spent_rub -= price_rub * ClientsBalanceStruct_[seller_].balance_doll;

                        ClientsBalanceStruct_[seller_].balance_doll = 0; // поскольку долларов на покупку больше, то будут куплены все оставшиеся
                    }
                }
            }
        }
    }
    ClientsBalanceStruct_[seller_].spent_doll -= doll_.value() - ClientsBalanceStruct_[seller_].balance_doll;
    ClientsBalanceStruct_[seller_].income_rub = seller_income;
    ClientsBalanceStruct_[seller_].balance_rub = ClientsBalanceStruct_[seller_].income_rub;
    ClientsBalanceStruct_[seller_].active_doll_balance = ClientsBalanceStruct_[seller_].balance_doll;
    //if(!is_price_bigger_than_offer)
        ClientsBalanceStruct_[seller_].active_rub_balance = ClientsBalanceStruct_[seller_].income_rub;
}

void TestCalcIncome2()
{
    std::string buyer1 = {"Leo"};
    std::string buyer2 = {"John"};
    std::string buyer3 = {"Mike"};
    std::string buyer4 = {"Tim"};
    std::string buyer5 = {"Tom"};
    std::string buyer6 = {"Ben"};
    std::string buyer7 = {"Kate"};

    std::unordered_map<int, std::vector<std::pair<std::string_view, int>>> StockData;
    std::unordered_map<std::string_view, ClientsLog> ClientsBalanceStruct;
    int doll = 50; int rub = 61; std::string seller = "Frank";
    StockData[62].emplace_back(buyer1, 10);
    StockData[63].emplace_back(buyer2, 20);
    std::set<int, std::greater<int>> indeciesOfStockDataUnMap = {62, 63};

    CalcIncome2(StockData, indeciesOfStockDataUnMap, doll, rub, seller, ClientsBalanceStruct);
    // Контрльный пример из ТЗ.
    //- Пользователь 3: **-30** USD, **1880** RUB...осталась активной на **20$**
    assert(ClientsBalanceStruct[seller].income_rub   == 1880);
    assert(ClientsBalanceStruct[seller].balance_doll == 20);
    assert(ClientsBalanceStruct[seller].spent_doll   == -30);
    assert(ClientsBalanceStruct[seller].balance_rub  == 1880);
    assert(ClientsBalanceStruct[seller].active_doll_balance  == 20);
    assert(ClientsBalanceStruct[seller].active_rub_balance   == 1880);
    assert(ClientsBalanceStruct[seller].income_doll  == 0);
    assert(ClientsBalanceStruct[seller].spent_rub    == 0);
    //- Пользователь 1: **10** USD, **-620** RUB.
    assert(ClientsBalanceStruct[buyer1].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer1].balance_doll == 0);
    assert(ClientsBalanceStruct[buyer1].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer1].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer1].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[buyer1].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer1].income_doll  == 10);
    assert(ClientsBalanceStruct[buyer1].spent_rub    == -620);
    // - Пользователь 2: **20** USD, **-1260** RUB.
    assert(ClientsBalanceStruct[buyer1].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer1].balance_doll == 0);
    assert(ClientsBalanceStruct[buyer1].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer1].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer1].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[buyer1].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer2].income_doll  == 20);
    assert(ClientsBalanceStruct[buyer2].spent_rub    == -1260);

    // Собственные тесты.
    StockData.clear();
    ClientsBalanceStruct.clear();
    StockData[62].emplace_back(buyer1, 10);
    StockData[63].emplace_back(buyer2, 20);
    StockData[62].emplace_back(buyer3, 12);

    CalcIncome2(StockData, indeciesOfStockDataUnMap, doll, rub, seller, ClientsBalanceStruct);

    assert(ClientsBalanceStruct[seller].income_rub   == 2624);
    assert(ClientsBalanceStruct[seller].balance_doll == 8);
    assert(ClientsBalanceStruct[seller].spent_doll   == -42);
    assert(ClientsBalanceStruct[seller].balance_rub  == 2624);
    assert(ClientsBalanceStruct[seller].active_doll_balance  == 8);
    assert(ClientsBalanceStruct[seller].active_rub_balance   == 2624);
    assert(ClientsBalanceStruct[seller].income_doll  == 0);
    assert(ClientsBalanceStruct[seller].spent_rub    == 0);

    assert(ClientsBalanceStruct[buyer1].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer1].balance_doll == 0);
    assert(ClientsBalanceStruct[buyer1].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer1].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer1].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[buyer1].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer1].income_doll  == 10);
    assert(ClientsBalanceStruct[buyer1].spent_rub    == -620);

    assert(ClientsBalanceStruct[buyer2].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer2].balance_doll == 0);
    assert(ClientsBalanceStruct[buyer2].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer2].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer2].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[buyer2].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer2].income_doll  == 20);
    assert(ClientsBalanceStruct[buyer2].spent_rub    == -1260);

    assert(ClientsBalanceStruct[buyer3].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer3].balance_doll == 0);
    assert(ClientsBalanceStruct[buyer3].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer3].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer3].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[buyer3].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer3].income_doll  == 12);
    assert(ClientsBalanceStruct[buyer3].spent_rub    == -744);

    StockData.clear();
    ClientsBalanceStruct.clear();
    StockData[21].emplace_back(buyer1, 10);
    StockData[45].emplace_back(buyer2, 30);
    StockData[16].emplace_back(buyer3, 15);
    StockData[35].emplace_back(buyer4, 100);
    doll = 50; rub = 22;
    indeciesOfStockDataUnMap = {21, 45, 16, 35};

    CalcIncome2(StockData, indeciesOfStockDataUnMap, doll, rub, seller, ClientsBalanceStruct);

    assert(ClientsBalanceStruct[seller].income_rub   == 2050);
    assert(ClientsBalanceStruct[seller].balance_doll == 0);
    assert(ClientsBalanceStruct[seller].spent_doll   == -50);
    assert(ClientsBalanceStruct[seller].balance_rub  == 2050);
    assert(ClientsBalanceStruct[seller].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[seller].active_rub_balance   == 2050);
    assert(ClientsBalanceStruct[seller].income_doll  == 0);
    assert(ClientsBalanceStruct[seller].spent_rub    == 0);

    assert(ClientsBalanceStruct[buyer2].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer2].balance_doll == 0);
    assert(ClientsBalanceStruct[buyer2].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer2].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer2].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[buyer2].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer2].income_doll  == 30);
    assert(ClientsBalanceStruct[buyer2].spent_rub    == -1350);

    assert(ClientsBalanceStruct[buyer4].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer4].balance_doll == 20);
    assert(ClientsBalanceStruct[buyer4].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer4].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer4].active_doll_balance  == 80);
    assert(ClientsBalanceStruct[buyer4].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer4].income_doll  == 20);
    assert(ClientsBalanceStruct[buyer4].spent_rub    == -700);

    ClientsBalanceStruct.clear();
    doll = 35; rub = 15;

    CalcIncome2(StockData, indeciesOfStockDataUnMap, doll, rub, seller, ClientsBalanceStruct);

    assert(ClientsBalanceStruct[seller].income_rub   == 1525);
    assert(ClientsBalanceStruct[seller].balance_doll == 0);
    assert(ClientsBalanceStruct[seller].spent_doll   == -35);
    assert(ClientsBalanceStruct[seller].balance_rub  == 1525);
    assert(ClientsBalanceStruct[seller].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[seller].active_rub_balance   == 1525);
    assert(ClientsBalanceStruct[seller].income_doll  == 0);
    assert(ClientsBalanceStruct[seller].spent_rub    == 0);

    assert(ClientsBalanceStruct[buyer2].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer2].balance_doll == 0);
    assert(ClientsBalanceStruct[buyer2].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer2].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer2].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[buyer2].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer2].income_doll  == 30);
    assert(ClientsBalanceStruct[buyer2].spent_rub    == -1350);

    assert(ClientsBalanceStruct[buyer4].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer4].balance_doll == 5);
    assert(ClientsBalanceStruct[buyer4].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer4].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer4].active_doll_balance  == 95);
    assert(ClientsBalanceStruct[buyer4].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer4].income_doll  == 5);
    assert(ClientsBalanceStruct[buyer4].spent_rub    == -175);

    ClientsBalanceStruct.clear();
//    StockData[21].emplace_back(buyer1, 10); // для понимания, чем заполнен.
//    StockData[45].emplace_back(buyer2, 30);
//    StockData[16].emplace_back(buyer3, 15);
//    StockData[35].emplace_back(buyer4, 100);
    StockData[21].emplace_back(buyer5, 70);
    StockData[45].emplace_back(buyer6, 28);
    StockData[45].emplace_back(buyer7, 45);
//     doll = 35; rub = 15; для понимания (покупкаZ)

    CalcIncome2(StockData, indeciesOfStockDataUnMap, doll, rub, seller, ClientsBalanceStruct);

    assert(ClientsBalanceStruct[seller].income_rub   == 1575);
    assert(ClientsBalanceStruct[seller].balance_doll == 0);
    assert(ClientsBalanceStruct[seller].spent_doll   == -35);
    assert(ClientsBalanceStruct[seller].balance_rub  == 1575);
    assert(ClientsBalanceStruct[seller].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[seller].active_rub_balance   == 1575);
    assert(ClientsBalanceStruct[seller].income_doll  == 0);
    assert(ClientsBalanceStruct[seller].spent_rub    == 0);

    assert(ClientsBalanceStruct[buyer6].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer6].balance_doll == 5);
    assert(ClientsBalanceStruct[buyer6].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer6].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer6].active_doll_balance  == 23);
    assert(ClientsBalanceStruct[buyer6].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer6].income_doll  == 5);
    assert(ClientsBalanceStruct[buyer6].spent_rub    == -225);

    assert(ClientsBalanceStruct[buyer2].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer2].balance_doll == 0);
    assert(ClientsBalanceStruct[buyer2].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer2].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer2].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[buyer2].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer2].income_doll  == 30);
    assert(ClientsBalanceStruct[buyer2].spent_rub    == -1350);

    assert(ClientsBalanceStruct[buyer7].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer7].balance_doll == 0);
    assert(ClientsBalanceStruct[buyer7].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer7].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer7].active_doll_balance  == 45);
    assert(ClientsBalanceStruct[buyer7].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer7].income_doll  == 0);
    assert(ClientsBalanceStruct[buyer7].spent_rub    == 0);


    ClientsBalanceStruct.clear();
    doll = 35; rub = 50; // цена продажи в рублях больше покупки. Сделки нет. На балансе и активной заявке должна остаться заявленная сумма.

    CalcIncome2(StockData, indeciesOfStockDataUnMap, doll, rub, seller, ClientsBalanceStruct);

    assert(ClientsBalanceStruct[seller].income_rub   == 0);
    assert(ClientsBalanceStruct[seller].balance_doll == 35);
    assert(ClientsBalanceStruct[seller].active_doll_balance == 35);
    assert(ClientsBalanceStruct[seller].active_rub_balance == 50);
}


// depricated. Ошибся, не обратил внимание, что может быть одинаковая цена. Работает только, есть разная цена покупки.
void CalcIncome1(const std::map<int, std::pair<std::string_view, int>> & StockData_,
                std::optional<int> doll_, std::optional<int> rub_, const std::string_view & seller_,
                std::map<std::string_view, ClientsLog> & ClientsBalanceStruct_)
{
    if(!doll_.has_value() || !rub_.has_value())
    {
        std::cout << "Error! Data lost, no doll or rub value found!" << std::endl;
        return;
    }
    int seller_income = 0;
    auto rev_it = StockData_.rbegin();
    ClientsBalanceStruct_[seller_].balance_doll = doll_.value();
    ClientsBalanceStruct_[seller_].balance_rub = rub_.value();
    bool is_price_bigger_than_offer = false;
    while(rev_it != StockData_.rend() && ClientsBalanceStruct_[seller_].balance_doll > 0)
    {
        if(rev_it->first >= ClientsBalanceStruct_[seller_].balance_rub)
        {
            if(rev_it->second.second < ClientsBalanceStruct_[seller_].balance_doll)
            {
                seller_income += rev_it->second.second * rev_it->first;
                ClientsBalanceStruct_[rev_it->second.first].income_doll += rev_it->second.second;
                ClientsBalanceStruct_[rev_it->second.first].spent_rub -= rev_it->first * rev_it->second.second;
                ClientsBalanceStruct_[rev_it->second.first].active_doll_balance = 0; // продадутся все доллары
                ClientsBalanceStruct_[rev_it->second.first].active_rub_balance = 0; // цена в рублях тоже не актуальна

                ClientsBalanceStruct_[seller_].balance_doll -= rev_it->second.second;
            }
            else if(rev_it->second.second > ClientsBalanceStruct_[seller_].balance_doll)
            {
                seller_income += rev_it->first * ClientsBalanceStruct_[seller_].balance_doll;
                ClientsBalanceStruct_[rev_it->second.first].active_doll_balance = rev_it->second.second - ClientsBalanceStruct_[seller_].balance_doll;
                ClientsBalanceStruct_[rev_it->second.first].income_doll += ClientsBalanceStruct_[seller_].balance_doll;
                ClientsBalanceStruct_[rev_it->second.first].balance_doll +=ClientsBalanceStruct_[seller_].balance_doll;
                ClientsBalanceStruct_[rev_it->second.first].spent_rub -= rev_it->first * ClientsBalanceStruct_[seller_].balance_doll;
                ClientsBalanceStruct_[seller_].balance_doll = 0; // поскольку долларов на покупку больше, то будут куплены все оставшиеся
            }
        }
        else
        {
            ClientsBalanceStruct_[seller_].active_rub_balance = rub_.value();
            is_price_bigger_than_offer = true;
        }
        ++rev_it;
    }
    ClientsBalanceStruct_[seller_].spent_doll -= doll_.value() - ClientsBalanceStruct_[seller_].balance_doll;
    ClientsBalanceStruct_[seller_].income_rub = seller_income;
    ClientsBalanceStruct_[seller_].balance_rub = ClientsBalanceStruct_[seller_].income_rub;
    ClientsBalanceStruct_[seller_].active_doll_balance = ClientsBalanceStruct_[seller_].balance_doll;
    if(!is_price_bigger_than_offer)
        ClientsBalanceStruct_[seller_].active_rub_balance = ClientsBalanceStruct_[seller_].income_rub;

}

void TestCalcIncome1()
{
    std::string buyer1 = {"Leo"};
    std::string buyer2 = {"John"};
    std::string buyer3 = {"Mike"};
    std::string buyer4 = {"Tim"};
    std::string buyer5 = {"Tom"};
    std::string buyer6 = {"Ben"};

    //       rub                   id    doll
    std::map<int, std::pair<std::string_view, int>> StockData;
    StockData[62] = {buyer1, 10};
    StockData[63] = {buyer2, 20};
    std::map<std::string_view, ClientsLog> ClientsBalanceStruct; // [id]({doll, rub});
    int doll = 50; int rub = 61; std::string seller = "Frank";

    // проверяем доход, баланс продавца и покупателей. Пример из ТЗ.
    CalcIncome1(StockData, doll, rub, seller, ClientsBalanceStruct);
    //- Пользователь 3: **-30** USD, **1880** RUB...осталась активной на **20$**
    assert(ClientsBalanceStruct[seller].income_rub   == 1880);
    assert(ClientsBalanceStruct[seller].balance_doll == 20);
    assert(ClientsBalanceStruct[seller].spent_doll   == -30);
    assert(ClientsBalanceStruct[seller].balance_rub  == 1880);
    assert(ClientsBalanceStruct[seller].active_doll_balance  == 20);
    assert(ClientsBalanceStruct[seller].active_rub_balance   == 1880);
    assert(ClientsBalanceStruct[seller].income_doll  == 0);
    assert(ClientsBalanceStruct[seller].spent_rub    == 0);
    //- Пользователь 1: **10** USD, **-620** RUB.
    assert(ClientsBalanceStruct[buyer1].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer1].balance_doll == 0);
    assert(ClientsBalanceStruct[buyer1].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer1].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer1].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[buyer1].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer1].income_doll  == 10);
    assert(ClientsBalanceStruct[buyer1].spent_rub    == -620);
    // - Пользователь 2: **20** USD, **-1260** RUB.
    assert(ClientsBalanceStruct[buyer1].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer1].balance_doll == 0);
    assert(ClientsBalanceStruct[buyer1].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer1].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer1].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[buyer1].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer2].income_doll  == 20);
    assert(ClientsBalanceStruct[buyer2].spent_rub    == -1260);

    //собственные тесты
    StockData.clear();
    ClientsBalanceStruct.clear();
    StockData[21] = {buyer1, 10};
    StockData[45] = {buyer2, 30};
    StockData[16] = {buyer3, 15};
    StockData[35] = {buyer4, 100};

    doll = 50; rub = 22;

    CalcIncome1(StockData, doll, rub, seller, ClientsBalanceStruct);
    assert(ClientsBalanceStruct[seller].income_rub   == 2050);
    assert(ClientsBalanceStruct[seller].balance_doll == 0);
    assert(ClientsBalanceStruct[seller].spent_doll   == -50);
    assert(ClientsBalanceStruct[seller].balance_rub  == 2050);
    assert(ClientsBalanceStruct[seller].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[seller].active_rub_balance   == 2050);
    assert(ClientsBalanceStruct[seller].income_doll  == 0);
    assert(ClientsBalanceStruct[seller].spent_rub    == 0);

    assert(ClientsBalanceStruct[buyer2].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer2].balance_doll == 0);
    assert(ClientsBalanceStruct[buyer2].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer2].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer2].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[buyer2].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer2].income_doll  == 30);
    assert(ClientsBalanceStruct[buyer2].spent_rub    == -1350);

    assert(ClientsBalanceStruct[buyer4].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer4].balance_doll == 20);
    assert(ClientsBalanceStruct[buyer4].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer4].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer4].active_doll_balance  == 80);
    assert(ClientsBalanceStruct[buyer4].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer4].income_doll  == 20);
    assert(ClientsBalanceStruct[buyer4].spent_rub    == -700);

    ClientsBalanceStruct.clear();
    doll = 35; rub = 15;

    CalcIncome1(StockData, doll, rub, seller, ClientsBalanceStruct);
    assert(ClientsBalanceStruct[seller].income_rub   == 1525);
    assert(ClientsBalanceStruct[seller].balance_doll == 0);
    assert(ClientsBalanceStruct[seller].spent_doll   == -35);
    assert(ClientsBalanceStruct[seller].balance_rub  == 1525);
    assert(ClientsBalanceStruct[seller].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[seller].active_rub_balance   == 1525);
    assert(ClientsBalanceStruct[seller].income_doll  == 0);
    assert(ClientsBalanceStruct[seller].spent_rub    == 0);

    assert(ClientsBalanceStruct[buyer2].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer2].balance_doll == 0);
    assert(ClientsBalanceStruct[buyer2].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer2].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer2].active_doll_balance  == 0);
    assert(ClientsBalanceStruct[buyer2].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer2].income_doll  == 30);
    assert(ClientsBalanceStruct[buyer2].spent_rub    == -1350);

    assert(ClientsBalanceStruct[buyer4].income_rub   == 0);
    assert(ClientsBalanceStruct[buyer4].balance_doll == 5);
    assert(ClientsBalanceStruct[buyer4].spent_doll   == 0);
    assert(ClientsBalanceStruct[buyer4].balance_rub  == 0);
    assert(ClientsBalanceStruct[buyer4].active_doll_balance  == 95);
    assert(ClientsBalanceStruct[buyer4].active_rub_balance   == 0);
    assert(ClientsBalanceStruct[buyer4].income_doll  == 5);
    assert(ClientsBalanceStruct[buyer4].spent_rub    == -175);

    ClientsBalanceStruct.clear();
    doll = 35; rub = 50; // цена продажи в рублях больше покупки. Сделки нет. На балансе и активной заявке должна остаться заявленная сумма.

    CalcIncome1(StockData, doll, rub, seller, ClientsBalanceStruct);
    assert(ClientsBalanceStruct[seller].income_rub   == 0);
    assert(ClientsBalanceStruct[seller].balance_doll == 35);
    assert(ClientsBalanceStruct[seller].active_doll_balance == 35);
    assert(ClientsBalanceStruct[seller].active_rub_balance == 50);
}


// depricated. Ипользовался в старой версии, когда были только пары доллар и рубль. Без статистики и активных заявок. В мейне закомментирован.
std::pair<int, int> CalcIncome(const std::map<int, int> & StockData_, int doll_, int rub_, std::map<int, std::pair<int, int>> & Clients_Balance_)
{
    int seller_income = 0;
    int bucks = doll_;
    auto rev_it = StockData_.rbegin();
    while(rev_it->first >= rub_ && rev_it != StockData_.rend() && bucks > 0)
    {
        if(rev_it->second < bucks)
        {
            seller_income += rev_it->second * rev_it->first;
            Clients_Balance_[rev_it->second].first += rev_it->second;
            Clients_Balance_[rev_it->second].second -= rev_it->first * rev_it->second;
            bucks -= rev_it->second;
        }
        else if(rev_it->second > doll_)
        {
            seller_income += rev_it->first * bucks;
            Clients_Balance_[rev_it->second].first += rev_it->second - bucks;
            Clients_Balance_[rev_it->second].second -= rev_it->first * bucks;
            //rev_it->second -= bucks;
            bucks = 0;
        }
        ++rev_it;
    }
    int seller_balance = 0;
    seller_balance -= doll_ -  bucks;
    return std::make_pair(seller_balance, seller_income);
}

void TestCalcIncome()
{   //       rub doll
    std::map<int, int> StockData = { {62, 10}, {63, 20} };
    std::map<int, std::pair<int, int>> ClientsBalance; // [id]({doll, rub});

    int doll = 50; int rub = 61;

    // проверяем доход и баланс продавца. Пример из ТЗ.
    assert(CalcIncome(StockData, doll, rub, ClientsBalance).second == 1880); //- Пользователь 3: **-30** USD, **1880** RUB.
    ClientsBalance.clear(); // чтобы два раза по ссылке не увеличивалось. Функция вызывается два раза в ассерт.
    assert(CalcIncome(StockData, doll, rub, ClientsBalance).first == -30);

    // проверяем доход и баланс покупателя. Пример из ТЗ.
    std::stack<std::pair<int, int>> res_buffer;
    for(const auto & [key, value] : ClientsBalance)
    {
        res_buffer.push(value);
    }

    assert(res_buffer.top().first == 20); // - Пользователь 2: **20** USD, **-1260** RUB.
    assert(res_buffer.top().second == -1260);
    res_buffer.pop();
    assert(res_buffer.top().first == 10);
    assert(res_buffer.top().second == -620); // - Пользователь 1: **10** USD, **-620** RUB.
    res_buffer.pop();

    //2
    StockData.clear();
    ClientsBalance.clear();
    StockData = { {21, 10}, {45, 30}, {16, 15}, {35, 100} };
    doll = 50; rub = 22;

    assert(CalcIncome(StockData, doll, rub, ClientsBalance).second == 2050);
    ClientsBalance.clear();
    assert(CalcIncome(StockData, doll, rub, ClientsBalance).first == -50);

    for(const auto & [key, value] : ClientsBalance)
    {
        res_buffer.push(value);
    }

    //3
    StockData.clear();
    ClientsBalance.clear();
    StockData = { {21, 10}, {45, 30}, {16, 15}, {35, 100} };
    doll = 35; rub = 15;

    assert(CalcIncome(StockData, doll, rub, ClientsBalance).second == 1525);
    ClientsBalance.clear();
    assert(CalcIncome(StockData, doll, rub, ClientsBalance).first == -35);
}
