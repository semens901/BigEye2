#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication.h"
#include "ServerNetwork.h"
#include<QPushButton>
#include<QMessageBox>

class Server : public QMainWindow
{
    Q_OBJECT

public:
    Server(QWidget *parent = nullptr);
    ~Server();

public slots:
    void get_info(QJsonObject js_pkg);
    void get_mac_address();

private:
    ServerNetwork server_network;
    const int TABLE_COLUMNS = 6;
    QString selected_mac;
    QMap<QPushButton*, QString> buttons_sc;

signals:
    void send_address(QString msg);

private:
    Ui::QtWidgetsApplicationClass ui;
    
};
