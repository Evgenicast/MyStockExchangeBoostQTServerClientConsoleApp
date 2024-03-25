#include "mainwindow.h"
#include <boost/asio.hpp>
#include <QApplication>
#include "Common.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::resolver resolver(io_service);
    tcp::resolver::query query(tcp::v4(), "127.0.0.1", std::to_string(port));
    MainWindow w(io_service, resolver, query);
    w.show();

    return a.exec();
}
