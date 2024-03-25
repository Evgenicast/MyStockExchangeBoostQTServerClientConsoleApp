#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <boost/asio.hpp>
#include <memory>
#include <QTime>
#include <QtWidgets>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using boost::asio::ip::tcp;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(boost::asio::io_service & is_, tcp::resolver & resolver_, tcp::resolver::query & query_, QWidget *parent = nullptr);
    ~MainWindow();
    //std::list<QLineEdit *> CreateDialogBoxDoll_To_Rub(); to do!
private slots:
    void on_btn_connect_clicked();
    void on_btn_type_clicked();
    void on_btn_hello_clicked();
    void on_btn_buy_clicked();
    void on_btn_sell_clicked();
    void on_btn_balance_clicked();
    void on_btn_exit_clicked();

private:
    Ui::MainWindow *ui;
    std::string m_My_id;
    std::string name;
    //std::list<QLineEdit *> m_doll_to_rub;
    QTime time;

    int i = 0;
    tcp::socket s;
    tcp::resolver & resolver;
    tcp::resolver::query query;
    tcp::resolver::iterator iterator;

    void ProcessRegistration();
    void SendBuyRequest(const std::string & aId, const std::string & aRequestType, const std::pair<int, int> & dol_to_rub_);
    void SendSellRequest(const std::string & aId, const std::string & aRequestType, const std::pair<int, int> & dol_to_rub_);
    void SendGetBalance(const std::string & aId, const std::string & aRequestType);
    void SendMessage(const std::string & aId, const std::string & aRequestType, const std::string & aMessage);
    std::string ReadMessage();

};
#endif // MAINWINDOW_H
