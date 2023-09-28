#pragma once
#include <QTcpServer>
#include <QTcpSocket>
#include <Qvector>
#include <Qstring>
#include <QJsonObject>
#include <QJsonArray>
#include<QCoreApplication>
#include <QTimer>
#include <QTime>
#include<QDir>
#include <qhostinfo.h>
#include <QtNetwork/QNetworkInterface>
#pragma comment(lib,"user32")
#pragma comment(lib, "powrprof.lib")
#pragma comment(lib, "gdi32.lib")
#include<windows.h>
#include<winuser.h>
#include <iostream>
#include<QSettings>

class ClientServer : public QTcpServer
{
	Q_OBJECT
public:
	ClientServer();
	void send_to_server(const QJsonObject& js_pkg);
	void send_to_server(QString name);
	QJsonObject get_user_info();
	QString make_screenshot();
	bool connect_to_server();
	bool is_sleep();
	QJsonObject get_all_info();

private slots:
	void readyRead();
	void updateTime();
	void deleteLater();
	void onBytesWritten(qint64);
private:
	QTcpSocket* socket;
	QByteArray data;
	quint64 next_block_size;
	QJsonObject js_inpkg;
	QString ip_address = "127.0.0.1";// Local address
	const unsigned int PORT = 2000;
	QTimer* timer;
	bool is_screenshot = false;

};

