#ifndef SERVER_H
#define SERVER_H
#include <cstdlib>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <map>
#include <set>
#include <optional>
#include "Common.hpp"

//0.0
using boost::asio::ip::tcp;

class Core
{
private:

    struct ClientLog
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

    // <UserId, UserName>
    std::map<size_t, std::string> mUsers;

    //new
    std::unordered_map<int, ClientLog> m_OperationsLog;
    std::unordered_map<int, std::vector<std::pair<int, int>>> m_StockData;
    std::set<int, std::greater<int>> m_IndeciesOfStockData;
public:
    // "Регистрирует" нового пользователя и возвращает его ID.
    std::string RegisterNewUser(const std::string & aUserName);
    // Запрос имени клиента по ID
    std::string GetUserName(const std::string & aUserId);

    //добавленные мной методы
    std::size_t GetUserId(const std::string & aUserId);
    void ProcessSellingOrBuying(const int & userID, std::optional<std::pair<int, int>> & doll_to_rub_, const TradeStatus & reqtype);
    const ClientLog & GetBalanceData(const int & userID);
};

class session
{
public:
    session(boost::asio::io_service & io_service)
    : socket_(io_service) {}

    tcp::socket & socket()
    {
        return socket_;
    }

    void start();

    // Обработка полученного сообщения.
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write(const boost::system::error_code & error);

private:
    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class server
{
public:
    server(boost::asio::io_service & io_service);

    void handle_accept(session * new_session, const boost::system::error_code & error);

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};

#endif // SERVER_H
