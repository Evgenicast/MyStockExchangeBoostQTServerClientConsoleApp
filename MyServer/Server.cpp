#include "Server.h"
#include "json.hpp"
#include <iostream>


// Core
Core & GetCore()
{
    static Core core;
    return core;
}

std::string Core::RegisterNewUser(const std::string &aUserName)
{
    size_t newUserId = mUsers.size();
    mUsers[newUserId] = aUserName;

    return std::to_string(newUserId);
}

std::size_t Core::GetUserId(const std::string &aUserId)
{
    const auto userIt = mUsers.find(std::stoi(aUserId));
    if (userIt == mUsers.cend())
    {
        return 1;
    }
    else
    {
        return userIt->first;
    }
}

void Core::ProcessSellingOrBuying(const int & userID, std::optional<std::pair<int, int>> & doll_to_rub_, const TradeStatus & reqtype)
{
    if(!doll_to_rub_.has_value())
    {
        std::cout << "Error! Data lost, no doll or rub value found!" << std::endl;
        return;
    }

    switch(reqtype)
    {
        case TradeStatus::BUY:
        {
            m_IndeciesOfStockData.emplace(doll_to_rub_->second);
            m_OperationsLog[userID].active_doll_balance = doll_to_rub_->first;
            m_OperationsLog[userID].active_rub_balance = doll_to_rub_->second;
            m_StockData[doll_to_rub_->second].emplace_back(std::make_pair(userID, doll_to_rub_->first));
            break;
        }
        case TradeStatus::SELL:
        {
            if(*m_IndeciesOfStockData.cbegin() < doll_to_rub_->second)
            {
                m_OperationsLog[userID].balance_doll = doll_to_rub_->first;
                m_OperationsLog[userID].active_doll_balance = doll_to_rub_->first;
                m_OperationsLog[userID].active_rub_balance = doll_to_rub_->second;
                return;
            }

            int seller_income = 0;
            m_OperationsLog[userID].balance_doll = doll_to_rub_->first;
            m_OperationsLog[userID].balance_rub = doll_to_rub_->second;

            for(const auto price_rub : m_IndeciesOfStockData)
            {
                if(price_rub >= m_OperationsLog[userID].balance_rub && m_OperationsLog[userID].balance_doll > 0)
                {
                    auto & id_to_doll_vec = m_StockData[price_rub];
                    if(id_to_doll_vec.size() == 1)
                    {
                        if(id_to_doll_vec.back().second < m_OperationsLog[userID].balance_doll)
                        {
                            seller_income += price_rub * id_to_doll_vec.back().second;
                            m_OperationsLog[id_to_doll_vec.back().first].income_doll += id_to_doll_vec.back().second;
                            m_OperationsLog[id_to_doll_vec.back().first].spent_rub -= price_rub * id_to_doll_vec.back().second;
                            m_OperationsLog[id_to_doll_vec.back().first].active_doll_balance = 0; // продадутся все доллары
                            m_OperationsLog[id_to_doll_vec.back().first].active_rub_balance = 0; // цена в рублях тоже не актуальна

                            m_OperationsLog[userID].balance_doll -= id_to_doll_vec.back().second;
                        }
                        else
                        {
                            seller_income += price_rub * m_OperationsLog[userID].balance_doll;
                            m_OperationsLog[id_to_doll_vec.back().first].active_doll_balance = id_to_doll_vec.back().second - m_OperationsLog[userID].balance_doll;
                            m_OperationsLog[id_to_doll_vec.back().first].income_doll += m_OperationsLog[userID].balance_doll;
                            m_OperationsLog[id_to_doll_vec.back().first].balance_doll += m_OperationsLog[userID].balance_doll;
                            m_OperationsLog[id_to_doll_vec.back().first].spent_rub -= price_rub * m_OperationsLog[userID].balance_doll;

                            m_OperationsLog[userID].balance_doll = 0;
                        }
                    }
                    else
                    {
                        for(auto & it : id_to_doll_vec)
                        {
                            if(it.second < m_OperationsLog[userID].balance_doll)
                            {
                                seller_income += price_rub * it.second;
                                m_OperationsLog[it.first].income_doll += it.second;
                                m_OperationsLog[it.first].spent_rub -= price_rub * it.second;
                                m_OperationsLog[it.first].active_doll_balance = 0; // продадутся все доллары
                                m_OperationsLog[it.first].active_rub_balance = 0; // цена в рублях тоже не актуальна

                                m_OperationsLog[userID].balance_doll -= it.second;
                            }
                            else
                            {
                                seller_income += price_rub * m_OperationsLog[userID].balance_doll;
                                m_OperationsLog[it.first].active_doll_balance = it.second - m_OperationsLog[userID].balance_doll;
                                m_OperationsLog[it.first].income_doll += m_OperationsLog[userID].balance_doll;
                                m_OperationsLog[it.first].balance_doll += m_OperationsLog[userID].balance_doll;
                                m_OperationsLog[it.first].spent_rub -= price_rub * m_OperationsLog[userID].balance_doll;

                                m_OperationsLog[userID].balance_doll = 0; // поскольку долларов на покупку больше, то будут куплены все оставшиеся
                            }
                        }
                    }
                }
            }
            m_OperationsLog[userID].spent_doll -= doll_to_rub_->first - m_OperationsLog[userID].balance_doll;
            m_OperationsLog[userID].income_rub = seller_income;
            m_OperationsLog[userID].balance_rub =  m_OperationsLog[userID].income_rub;
            m_OperationsLog[userID].active_doll_balance = m_OperationsLog[userID].balance_doll;
            m_OperationsLog[userID].active_rub_balance = m_OperationsLog[userID].income_rub;
            break;
        }
    }
}

const Core::ClientLog &Core::GetBalanceData(const int &userID)
{
    return m_OperationsLog[userID];
}

std::string Core::GetUserName(const std::string & aUserId)
{
    const auto userIt = mUsers.find(std::stoi(aUserId));
    if (userIt == mUsers.cend())
    {
        return "Error! Unknown User";
    }
    else
    {
        return userIt->second;
    }
}

// Session
void session::start()
{
    socket_.async_read_some(boost::asio::buffer(data_, max_length), boost::bind(&session::handle_read, this, boost::asio::placeholders::error,
                                                                                boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code &error, size_t bytes_transferred)
{
    if (!error)
    {
        data_[bytes_transferred] = '\0';

        // Парсим json, который пришёл нам в сообщении.
        auto j = nlohmann::json::parse(data_);
        auto reqType = j["ReqType"];

        std::string reply = "Error! Unknown request type";
        if (reqType == Requests::Registration)
        {
            // Это реквест на регистрацию пользователя.
            // Добавляем нового пользователя и возвращаем его ID.
            reply = GetCore().RegisterNewUser(j["Message"]);
        }
        else if (reqType == Requests::Hello)
        {
            // Это реквест на приветствие.
            // Находим имя пользователя по ID и приветствуем его по имени.
            reply = "Hello, " + GetCore().GetUserName(j["UserId"]) + "ID " + std::to_string(GetCore().GetUserId(j["UserId"])) + "!\n";
        }
        else if (reqType == Requests::Buy)
        {
            int userID = GetCore().GetUserId(j["UserId"]);
            std::optional<std::pair<int, int>> doll_to_rub = j["Buy"];
            GetCore().ProcessSellingOrBuying(userID, doll_to_rub, TradeStatus::BUY);

            reply = "Accepted Buy " + std::to_string(doll_to_rub->first) + " Dollars for " +
                    std::to_string(doll_to_rub->second) + " Rubles" + "\n";

        }
        else if (reqType == Requests::Sell)
        {
            int userID = GetCore().GetUserId(j["UserId"]);
            std::optional<std::pair<int, int>> doll_to_rub = j["Sell"];
            GetCore().ProcessSellingOrBuying(userID, doll_to_rub, TradeStatus::SELL);
            reply = " Income "              + std::to_string(GetCore().GetBalanceData(userID).income_rub) +
                    " Active Doll Balance " + std::to_string(GetCore().GetBalanceData(userID).balance_doll) +
                    " Dollar Spent "        + std::to_string(GetCore().GetBalanceData(userID).spent_doll) + "\n";
        }
        else if (reqType == Requests::Balance)
        {
            int userID = GetCore().GetUserId(j["UserId"]);
            reply = " Active Doll Balance " + std::to_string(GetCore().GetBalanceData(userID).active_doll_balance) + "\n"
                    " Active Rub Balance "  + std::to_string(GetCore().GetBalanceData(userID).active_rub_balance) + "\n"
                    " Balance Rub "         + std::to_string(GetCore().GetBalanceData(userID).balance_rub) + "\n"
                    " Balance Doll "        + std::to_string(GetCore().GetBalanceData(userID).balance_doll) + "\n"
                    " Spent Rub "           + std::to_string(GetCore().GetBalanceData(userID).spent_rub) + "\n"
                    " Spent Dol "           + std::to_string(GetCore().GetBalanceData(userID).spent_doll) + "\n"
                    " Income Rub "          + std::to_string(GetCore().GetBalanceData(userID).income_rub) + "\n"
                    " Income Dol "          + std::to_string(GetCore().GetBalanceData(userID).income_doll) + "\n";
        }

        boost::asio::async_write(socket_, boost::asio::buffer(reply, reply.size()),
                                 boost::bind(&session::handle_write, this,
                                 boost::asio::placeholders::error));
    }
    else
    {
        delete this;
    }
}

void session::handle_write(const boost::system::error_code &error)
{
    if (!error)
    {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                boost::bind(&session::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        delete this;
    }
}

//Server
server::server(boost::asio::io_service &io_service)
    : io_service_(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
{
    std::cout << "Server started! Listen " << port << " port" << std::endl;
    session* new_session = new session(io_service_);
    acceptor_.async_accept(new_session->socket(), boost::bind(&server::handle_accept, this, new_session,
                                                              boost::asio::placeholders::error));
}

void server::handle_accept(session *new_session, const boost::system::error_code &error)
{
    if (!error)
    {
        new_session->start();
        new_session = new session(io_service_);
        acceptor_.async_accept(new_session->socket(), boost::bind(&server::handle_accept, this, new_session,
                                                                  boost::asio::placeholders::error));
    }
    else
    {
        delete new_session;
    }
}
