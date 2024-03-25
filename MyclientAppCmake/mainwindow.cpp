#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDialog>
#include <QBoxLayout>

#include "Common.hpp"
#include "json.hpp"
#include "qlabel.h"

MainWindow::MainWindow(boost::asio::io_service & is_, tcp::resolver & resolver_, tcp::resolver::query & query_, QWidget *parent)
    : s(is_), resolver(resolver_), query(query_), QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SendBuyRequest(const std::string & aId, const std::string & aRequestType, const std::pair<int, int> & dol_to_rub_)
{
    nlohmann::json req;
    req["UserId"] = aId;
    req["ReqType"] = aRequestType;
    req["Buy"] = dol_to_rub_;
    std::string request = req.dump();
    boost::asio::write(s, boost::asio::buffer(request, request.size()));
}
void MainWindow::SendSellRequest(const std::string & aId, const std::string & aRequestType, const std::pair<int, int> & dol_to_rub_)
{
    nlohmann::json req;
    req["UserId"] = aId;
    req["ReqType"] = aRequestType;
    req["Sell"] = dol_to_rub_;
    std::string request = req.dump();
    boost::asio::write(s, boost::asio::buffer(request, request.size()));
}

void MainWindow::SendGetBalance(const std::string & aId, const std::string & aRequestType)
{
    nlohmann::json req;
    req["UserId"] = aId;
    req["ReqType"] = aRequestType;
    std::string request = req.dump();
    boost::asio::write(s, boost::asio::buffer(request, request.size()));
}
// Отправка сообщения на сервер по шаблону.
void MainWindow::SendMessage(const std::string & aId, const std::string & aRequestType, const std::string & aMessage)
{
    nlohmann::json req;
    req["UserId"] = aId;
    req["ReqType"] = aRequestType;
    req["Message"] = aMessage;

    std::string request = req.dump();
    boost::asio::write(s, boost::asio::buffer(request, request.size()));
}

// Возвращает строку с ответом сервера на последний запрос.
std::string MainWindow::ReadMessage()
{
    boost::asio::streambuf b;
    boost::asio::read_until(s, b, "\0");
    std::istream is(&b);
    std::string line(std::istreambuf_iterator<char>(is), {});
    return line;
}

// "Создаём" пользователя, получаем его ID.
void MainWindow::ProcessRegistration()
{
    QFont font("Times", 15, QFont::Bold);
    ui->textBrowser->setFont(font);
    ui->textBrowser->append("Session starts at " + QTime::currentTime().toString("hh:mm:ss"));
    QDialog dialog(this);
    QFormLayout form(&dialog);
    QLabel * label = new QLabel("Hello! Enter your name:");

    form.addRow(label);
    QLineEdit * lineEdit = new QLineEdit();
    form.addRow(lineEdit);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    dialog.exec();

    name = lineEdit->text().toStdString();
    QString sr = QString::fromStdString(name);
    ui->textBrowser->append(sr);
    SendMessage("0", Requests::Registration, name);
    m_My_id = ReadMessage();
    QString qstr = QString::fromStdString(m_My_id);
    ui->textBrowser->append("Your ID " + qstr);
}

void MainWindow::on_btn_connect_clicked()
{
    try
    {
        iterator = resolver.resolve(query);
        s.connect(*iterator);
        ProcessRegistration();
    }
    catch (std::exception & e)
    {
        QMessageBox::critical(this, "Exception: ", e.what());
        return;
    }
}

void MainWindow::on_btn_type_clicked()
{
   ui->textBrowser->append(ui->lineEdit->text());
}


void MainWindow::on_btn_hello_clicked()
{
    SendMessage(m_My_id, Requests::Hello, "");
    std::string st = ReadMessage();
    QString qstr = QString::fromStdString(st);
    ui->textBrowser->append(qstr);
}

void MainWindow::on_btn_buy_clicked()
{
    QFont font("Times", 15, QFont::Bold);
    ui->textBrowser->setFont(font);
    QDialog dialog(this);
    QFormLayout form(&dialog);
    QLabel * label = new QLabel("Enter dollars/rubles");
    form.addRow(label);
    std::list<QLineEdit *> fieldsList;

    QLineEdit * lineEdit = new QLineEdit();
    lineEdit->setValidator(new QIntValidator(0, 5000, this));

    for(int i = 0; i < 2; ++i)
    {
        QLineEdit * lineEdit = new QLineEdit(&dialog);
        QString label = QString("<span style=\" color:#2f4f4f;\"> %1</span>").arg(i%2 ? "Rub" : "Dollar");
        form.addRow(label, lineEdit);
        fieldsList.emplace_back(lineEdit);
    }

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    dialog.exec();

    std::pair<int, int> doll_to_rub(fieldsList.front()->text().toInt(), fieldsList.back()->text().toInt());
    SendBuyRequest(m_My_id, Requests::Buy, doll_to_rub);
    std::string info_from_server = ReadMessage();
    QString qstr = QString::fromStdString(info_from_server);
    ui->textBrowser->append("Inforamtion from server " + qstr);
}

void MainWindow::on_btn_sell_clicked()
{
    QFont font("Times", 15, QFont::Bold);
    ui->textBrowser->setFont(font);
    QDialog dialog(this);
    QFormLayout form(&dialog);
    QLabel * label = new QLabel("Enter dollars/rubles");
    form.addRow(label);
    std::list<QLineEdit *> fieldsList;

    QLineEdit * lineEdit = new QLineEdit();
    lineEdit->setValidator(new QIntValidator(0, 5000, this));

    for(int i = 0; i < 2; ++i)
    {
        QLineEdit * lineEdit = new QLineEdit(&dialog);
        QString label = QString("<span style=\" color:#2f4f4f;\"> %1</span>").arg(i%2 ? "Rub" : "Dollar");
        form.addRow(label, lineEdit);
        fieldsList.emplace_back(lineEdit);
    }

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    dialog.exec();

    std::pair<int, int> doll_to_rub(fieldsList.front()->text().toInt(), fieldsList.back()->text().toInt());
    SendSellRequest(m_My_id, Requests::Sell, doll_to_rub);
    std::string info_from_server = ReadMessage();
    QString qstr = QString::fromStdString(info_from_server);
    ui->textBrowser->append("Inforamtion from server " + qstr);
}


void MainWindow::on_btn_balance_clicked()
{
    SendGetBalance(m_My_id, Requests::Balance);
    std::string info_from_server = ReadMessage();
    QString qstr = QString::fromStdString(info_from_server);
    ui->textBrowser->append("Inforamtion from server " + qstr);
}


void MainWindow::on_btn_exit_clicked()
{
    boost::system::error_code ec;
    s.close(ec);
    if (ec)
    {
        QString qstr = QString::fromStdString(ec.to_string());
        QMessageBox::critical(this, "Exception: ", qstr);
        return;
    }
    ui->textBrowser->append("Session was closed at " +  QTime::currentTime().toString("hh:mm:ss"));

}

