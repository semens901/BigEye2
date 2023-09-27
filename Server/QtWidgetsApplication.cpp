#include "QtWidgetsApplication.h"

Server::Server(QWidget *parent)
    : QMainWindow(parent)
{
    connect(&server_network, &ServerNetwork::signal, this, &Server::get_info);
    //connect(this, &Server::get_pic, &server_network, &ServerNetwork::get_screenshot);
    connect(this, &Server::send_address, &server_network, &ServerNetwork::get_screenshot);
    ui.setupUi(this);
}
#define TO_STRING_AUX( x )  #x
#define TO_STRING( x )      TO_STRING_AUX( x )
void Server::get_info(QJsonObject js_pkg)
{
    int rows = ui.tableWidget->rowCount();
    QList<QTableWidgetItem*> items = ui.tableWidget->findItems(js_pkg["USER"].toString(), Qt::MatchExactly);
    if (js_pkg["TYPE"].toString() == "DELETE")
    {
        QList<QTableWidgetItem*> items = ui.tableWidget->findItems(js_pkg["DOMAIN"].toString(), Qt::MatchExactly);
        if (items.isEmpty())
            return;
        auto it = items.begin();
        int col = (*it)->column();
        int row = (*it)->row();
        ui.tableWidget->removeRow(row);
        return;
    }
    if (!items.isEmpty())
    {
        auto it = items.begin();
        int col = (*it)->column();
        int row = (*it)->row();
        ui.tableWidget->removeRow(row);
        rows = row;
    }
    QTableWidgetItem* mac = new QTableWidgetItem(js_pkg["MAC"].toString());
    mac->setFlags(mac->flags() &= ~Qt::ItemIsEditable);
    QTableWidgetItem* localhost_name = new QTableWidgetItem(js_pkg["LOCALHOSTNAME"].toString());
    localhost_name->setFlags(localhost_name->flags() &= ~Qt::ItemIsEditable);
    QTableWidgetItem* IP = new QTableWidgetItem(js_pkg["IP"].toString());
    IP->setFlags(IP->flags() &= ~Qt::ItemIsEditable);
    QTableWidgetItem* user = new QTableWidgetItem(js_pkg["USER"].toString());
    user->setFlags(user->flags() &= ~Qt::ItemIsEditable);
    QTableWidgetItem* time = new QTableWidgetItem(js_pkg["TIME"].toString());
    time->setFlags(time->flags() &= ~Qt::ItemIsEditable);
    ui.tableWidget->insertRow(rows);
    ui.tableWidget->setItem(rows, 0, mac);
    ui.tableWidget->setItem(rows, 1, localhost_name);
    ui.tableWidget->setItem(rows, 2, IP);
    ui.tableWidget->setItem(rows, 3, user);
    ui.tableWidget->setItem(rows, 4, time);
    QPushButton* button = new QPushButton(QString(TO_STRING("Screenshot")), this);
    buttons_sc[button] = js_pkg["MAC"].toString();
    ui.tableWidget->setCellWidget(rows, 5, button);
    connect(button, &QAbstractButton::clicked, this, &Server::get_mac_address);


}

void Server::get_mac_address()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    QString mac =buttons_sc[btn];
    emit send_address(mac);
}

void send_address(QString msg)
{

}

Server::~Server()
{}
