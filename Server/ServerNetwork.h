#pragma once
#include <QTcpServer>
#include <QTcpSocket>
#include <Qvector>
#include <Qstring>
#include <QJsonObject>
#include <QJsonArray>
#include<QFile>
#include <QMessageBox>
class ServerNetwork : public QTcpServer
{
	Q_OBJECT
public:
	ServerNetwork();
	void send_to_client(QJsonObject& js_pkg, QTcpSocket* socket);
	void save_screenshot(QByteArray& data);

public slots:
	void incomingConnection(qintptr socket_descriptor);
	void readyRead();
	void deleteLater();
	void get_screenshot(QString msg);

private:
	QTcpSocket* socket;
	QMap <QTcpSocket*, QString> clients; //socket, domain
	QTcpSocket* selected_user;
	QByteArray data;
	quint64 next_block_size;
	QJsonObject js_inpkg;

signals:
	void signal(QJsonObject js_pkg);

};

