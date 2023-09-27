#include "ServerNetwork.h"
ServerNetwork::ServerNetwork() : next_block_size(0), selected_user(NULL), file("screenshot.bmp")
{
	if (this->listen(QHostAddress::Any, 2000))
	{
		qDebug() << "Start server";
	}
	else
	{
		qDebug() << "Error, server don't start";
	}
}

void ServerNetwork::send_to_client(QJsonObject& js_pkg, QTcpSocket* socket)
{
	QDataStream out(&data, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_6_5);
	out << quint64(0) << js_pkg;
	out.device()->seek(0);
	out << quint64(data.size() - sizeof(quint64));
	if (socket != NULL)
		socket->write(data);
}

void ServerNetwork::incomingConnection(qintptr socket_descriptor)
{
	socket = new QTcpSocket;
	socket->setSocketDescriptor(socket_descriptor);
	connect(socket, &QTcpSocket::readyRead, this, &ServerNetwork::readyRead);
	connect(socket, &QTcpSocket::disconnected, this, &ServerNetwork::deleteLater);
	clients[socket] = "";

	qDebug() << "Client connected " << socketDescriptor();
}

void signal(QJsonObject js_pkg)
{

}

void ServerNetwork::readyRead() {
	socket = (QTcpSocket*)sender();
	QDataStream in(socket);
	in.setVersion(QDataStream::Qt_6_5);
	qint64 data_size;
	QByteArray base_data;
	QString type;
	qint64 len = 0;
	if (in.status() == QDataStream::Ok) {
		while (true)
		{
			if (next_block_size == 0)
			{
				qDebug() << "nextBlockSize = 0";
				if (socket->bytesAvailable() < 2)
				{
					qDebug() << "Data < 2, break";
					break;
				}
				in >> next_block_size;
				qDebug() << "nextBlockSize = " << next_block_size;
			}
			if (socket->bytesAvailable() < next_block_size) {
				qDebug() << "Data not full, break";
				break;
			}
			in >> type;
			next_block_size = 0;
			if (type == "FILE")
			{
				if (is_file)
				{
					file.open(QIODeviceBase::Append);
					QByteArray bytes;
					in >> bytes;
					qint64 len = file.write(bytes);
					accepted_size += len;
					qDebug() << len;
					if (accepted_size == file_size)
					{
						qDebug() << "FILE CLOSED";
						is_file = false;
						data_size = 0;
						accepted_size = 0;
						break;
					}
				}
				if (!is_file)
				{
					is_file = true;
					in >> data_size;
					file.open(QIODeviceBase::WriteOnly);
					file.close();
				}
			}
			else if (type == "JSON") {
				in >> data_size; 
				in >> js_inpkg;
				if (js_inpkg["TYPE"].toString() == "USERINFO") {
					if (clients[(QTcpSocket*)sender()] == "") 
						clients[(QTcpSocket*)sender()] = js_inpkg["MAC"].toString();
					emit signal(js_inpkg);
				}
			}
		}
	}
}


void ServerNetwork::save_screenshot(QByteArray& data)
{
	QFile file("screenshot.bmp");
	file.open(QIODevice::WriteOnly);
	file.write(data);
	file.close();
}

void ServerNetwork::get_screenshot(QString msg)
{

	QMap <QTcpSocket*, QString>::iterator clients_end = clients.end();
	for (auto var : clients.keys())
	{
		if (msg == clients.value(var))
		{
			selected_user = var;
		}
	}
	QJsonObject js_pkg =
	{
		{"TYPE", "GETSCREENSHOT"}
	};
	send_to_client(js_pkg, selected_user);
}

void ServerNetwork::deleteLater()
{
	auto being_deleted = clients.find((QTcpSocket*)sender());
	QJsonObject js_pkg =
	{
		{"TYPE", "DELETE"},
		{"DOMAIN", being_deleted.value()}
	};
	emit signal(js_pkg);
	clients.erase(being_deleted);
}

