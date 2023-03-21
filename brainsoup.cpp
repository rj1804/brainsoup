#include <stdio.h>

#include <QtWidgets>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>

#include "brainsoup.h"

brainSoup::brainSoup()
	: serialPort(Q_NULLPTR)
{
	createActions();
	serialPorts = new QActionGroup(this);
	setFixedSize(300, 200);
}

void brainSoup::about()
{
	QMessageBox::about(this, tr("About"), tr("<p><p>update the <b>B* Brains</b> Eurorack module</p>" "</p>by me :)</p>"));
}

void brainSoup::updateSerialPorts()
{
	const auto infos = QSerialPortInfo::availablePorts();

	serialPortMenu->clear();

	QActionGroup *sp = new QActionGroup(this);

	actConnect = serialPortMenu->addAction(tr("Connect"), this, &brainSoup::portConnect);
	actConnect->setEnabled(!selectedPort.isEmpty() && !serialPort.isOpen());
	
	actDisconnect = serialPortMenu->addAction(tr("Disconnect"), this, &brainSoup::portDisconnect);
	actDisconnect->setEnabled(serialPort.isOpen());

	serialPortMenu->addSeparator();

	QAction *first = Q_NULLPTR;
	for (const QSerialPortInfo & info:infos) {
#if 1
		printf("port: %12s  ", info.systemLocation().toLatin1().constData());
		printf("VID:PID %04X:%04X  ", info.vendorIdentifier(), info.productIdentifier());
		printf("busy: %s  ", info.isBusy()? "Y": "N");
		printf("[manuf/desc] %s / %s\n", info.manufacturer().toLatin1().constData(), info.description().toLatin1().constData() );
#endif
		if (info.manufacturer() != "Behringer") {
			continue;
		}
		QAction *portAct = serialPortMenu->addAction(info.systemLocation(), this, &brainSoup::selectPort);
		portAct->setCheckable(true);
		portAct->setEnabled(!info.isBusy());
		sp->addAction(portAct);
		if (!first) {
			first = portAct;
		}
	}

	QAction *act = Q_NULLPTR;

	if (!selectedPort.isEmpty()) {
		for (QAction * a:serialPortMenu->actions()) {
qDebug() << "a " << a->text().toLatin1().constData() << "b: " << selectedPort.toLatin1().constData();
			if (a->text() == selectedPort) {
				act = a;
				break;
			}
		}
	}

	if (!act) {
		act = first;
	}
	selectedPort = QString();

	if (act) {
		act->setChecked(true);
		selectedPort = QString(act->text());
qDebug() << "selected port " << selectedPort.toLatin1().constData();
		actConnect->setEnabled(!selectedPort.isEmpty() && !serialPort.isOpen());

		portConnect();
	}
}

void brainSoup::handleError(QSerialPort::SerialPortError serialPortError)
{
//	qDebug() << "serial error";
	switch (serialPortError) {
	case QSerialPort::NoError: 								break;
	case QSerialPort::DeviceNotFoundError: 	qDebug() << "DeviceNotFoundError"; 		break;
	case QSerialPort::PermissionError:	qDebug() << "PermissionError";			break;
	case QSerialPort::NotOpenError:		qDebug() << "NotOpenError";			break;
	case QSerialPort::ParityError:		qDebug() << "ParityError";			break;
	case QSerialPort::FramingError:		qDebug() << "FramingError";			break;
	case QSerialPort::BreakConditionError:	qDebug() << "BreakConditionError";		break;
	case QSerialPort::WriteError:		qDebug() << "WriteError";			break;
	case QSerialPort::ReadError:		qDebug() << "SerialReadError";			break;
	case QSerialPort::ResourceError:	qDebug() << "ResourceError";portDisconnect();	break;
	case QSerialPort::UnsupportedOperationError:qDebug() << "UnsupportedOperationError";	break;
	case QSerialPort::TimeoutError:		qDebug() << "TimeoutError";			break;
	case QSerialPort::UnknownError:	
	default:				qDebug() << "UnknownError";			break;
	}
}

void brainSoup::selectPort()
{
	QAction *act = (QAction *) sender();
	selectedPort = act->text();
printf("selectPort %s\n", act->text().toLatin1().constData());
}

void brainSoup::portConnect()
{
printf("portOpen %s\n", selectedPort.toLatin1().constData());
	if (!selectedPort.isEmpty() && !serialPort.isOpen()) {
		serialPort.setPortName(selectedPort);
		serialPort.setBaudRate(QSerialPort::Baud115200);

		if (!serialPort.open(QIODevice::ReadWrite)) {
printf("failed to open port\n");
			return;
		}
		connect(&serialPort, &QSerialPort::errorOccurred, this, &brainSoup::handleError);

		info_label->setText("Brains connected");
		actConnect->setEnabled(false);
		actDisconnect->setEnabled(true);
printf("connected\n");
	}
}

void brainSoup::portDisconnect()
{
	if (serialPort.isOpen()) {
		disconnect(&serialPort, &QSerialPort::errorOccurred, this, &brainSoup::handleError);
		serialPort.close();

		info_label->setText("Brains disconnected");
		actConnect->setEnabled(true);
		actDisconnect->setEnabled(false);
printf("disconnected\n");
	}
}

static uint8_t XOR_bytes(uint8_t *data, int len)
{
	uint8_t chk;
	int i;

	chk = *data;
	for (i = 1; i < len; i++) {
		chk = chk ^ data[i];
	}
	return chk;
}

int brainSoup::wait_for_OK(int timeout)
{
	while(1) {
		if(!serialPort.waitForReadyRead(timeout)) {
			return 1;
		}
		QByteArray d = serialPort.readAll();
		int len = d.length();
		const char *ptr = d.constData();
		if(len) {
//printf("len %2d:  ", len);
			int done = 0;
			while (len > 0) {
//printf("0x%02X[%c]  ", *ptr, *ptr);
				if(*ptr == 0x79) {
					done = 1;
				} 
				if(*ptr == 0x1F) {
//printf("\n");
					return 1;
				} 
				ptr++;
				len--;
			}
//printf("\n");
			if(done) {
				return 0;
			}
		}
	}	
	return 0;
}

void brainSoup::check()
{
printf("check\n");
	const unsigned char data[] = { 0xaa, 0x55 };
	serialPort.write((const char *)data, 2);
	if( wait_for_OK(1000) )  {
printf("ERROR\n");
		return;
	}
	if( wait_for_OK(1000) )  {
printf("ERROR\n");
		return;
	}
}

void brainSoup::check2()
{
printf("check2\n");
	const unsigned char data1[] = { 0x7F };
	serialPort.write((const char *)data1, 1);
	if( wait_for_OK(1000) )  {
printf("ERROR\n");
		return;
	}

	const unsigned char data[] = { 0x02, 0xFD };
	serialPort.write((const char *)data, 2);
	if( wait_for_OK(1000) )  {
printf("ERROR\n");
		return;
	}
	if( wait_for_OK(1000) )  {
printf("ERROR\n");
		return;
	}
}

void brainSoup::erase()
{
printf("erase\n");
	const unsigned char data1[] = { 0x44, 0xBB };
	serialPort.write((const char *)data1, 2);
	if( wait_for_OK(1000) )  {
printf("erase ERROR\n");
		return;
	}

	const unsigned char data2[] = { 0xFF, 0xFF, 0x00 };
	serialPort.write((const char *)data2, 3);
	if( wait_for_OK(10000) )  {
printf("erase 2 ERROR\n");
		return;
	}
printf("erase done\n");
}

void brainSoup::start()
{
printf("start\n");
	const unsigned char data[] = { 0x21, 0xde };
	serialPort.write((const char *)data, 2);

	if( wait_for_OK(1000) )  {
printf("ERROR\n");
		return;
	}
}

void brainSoup::update()
{
	// change the text
	prog_button->setText("Updating...");
	QCoreApplication::processEvents();

printf("update\n");

	int      wave_len  = wave_data.length();
	uint8_t *wave_ptr  = (uint8_t*)wave_data.constData();
	uint32_t wave_addr = 0x90040000;

	int      prog_len  = prog_data.length();
	uint8_t *prog_ptr  = (uint8_t*)prog_data.constData();
	uint32_t prog_addr = 0x24000000;

	int	total_len = wave_len + prog_len;

	const unsigned char get_version[] = { 0xAA, 0x55 };
	const unsigned char get_7F[] = { 0x7F };
	const unsigned char poke[] = { 0x02, 0xFD };
	const unsigned char erase[] = { 0x44, 0xBB };
	const unsigned char confirm[] = { 0xFF, 0xFF, 0x00 };
	const unsigned char reboot[] = { 0x21, 0xDE };

printf("wave %d  prog %d\n", wave_len, prog_len );
	if(!wave_len || !prog_len) {
		goto ErrorExit;
	}

printf("\nget version");
	prog_button->setText("open skull...");
	QCoreApplication::processEvents();

	serialPort.write((const char *)get_version, 2);
	if( wait_for_OK(1000) )  {
		goto ErrorExit;
	}
	if( wait_for_OK(1000) )  {
		goto ErrorExit;
	}

printf("\nsend 0x7F");
	prog_button->setText("insert scalpel...");
	QCoreApplication::processEvents();

	serialPort.write((const char *)get_7F, 1);
	if( wait_for_OK(1000) )  {
		goto ErrorExit;
	}

printf("\nno idea");
	prog_button->setText("poke around...");
	QCoreApplication::processEvents();
	serialPort.write((const char *)poke, 2);
	if( wait_for_OK(1000) )  {
		goto ErrorExit;
	}
	if( wait_for_OK(1000) )  {
		goto ErrorExit;
	}

printf("\nchip erase");
	prog_button->setText("remove old brain...");
	QCoreApplication::processEvents();
	serialPort.write((const char *)erase, 2);
	if( wait_for_OK(1000) )  {
		goto ErrorExit;
	}

printf("\nerase confirm");
	serialPort.write((const char *)confirm, 3);
	if( wait_for_OK(10000) )  {
		goto ErrorExit;
	}

	prog_button->setText("insert new brain...");
	QCoreApplication::processEvents();

	while(wave_len > 0) {
printf("\nwave  ");
		const unsigned char cmd[] = { 0x31, 0xCE };
		unsigned char addr[5] = {};
		unsigned char size[1] = {};
		unsigned char chk [1] = {};

		serialPort.write((const char *)cmd, 2);
		if( wait_for_OK(1000) )  {
			goto ErrorExit;
		}

		addr[0] = (wave_addr >> 24) & 0xFF;
		addr[1] = (wave_addr >> 16) & 0xFF;
		addr[2] = (wave_addr >>  8) & 0xFF;
		addr[3] = (wave_addr >>  0) & 0xFF;
		addr[4] = XOR_bytes(addr, 4);

printf("addr %02X%02X%02X%02X  %02X  ", addr[0], addr[1], addr[2], addr[3], addr[4] );
		serialPort.write((const char *)addr, 5);
		if( wait_for_OK(1000) )  {
			goto ErrorExit;
		}


		int len = qMin(32, wave_len);
		size[0] = len - 1;
		chk[0]  = XOR_bytes(wave_ptr, len);
		chk[0] ^= size[0];
printf("len %02X", size[0]);
		serialPort.write((const char *)size, 1);
		serialPort.write((const char *)wave_ptr, len);
		serialPort.write((const char *)chk, 1);

		if( wait_for_OK(1000) )  {
			goto ErrorExit;
		}
		wave_len  -= len,
		wave_ptr  += len;
		wave_addr += len;

		int percent = (total_len - (wave_len + prog_len)) * 100 / total_len;
		QString text = QString("insert new brain...%1%").arg(percent);
		prog_button->setText(text);
		QCoreApplication::processEvents();
	}

	while(prog_len > 0) {
printf("\nprog  ");
		const unsigned char cmd[] = { 0x31, 0xCE };
		unsigned char addr[5] = {};
		unsigned char size[1] = {};
		unsigned char chk [1] = {};

		serialPort.write((const char *)cmd, 2);
		if( wait_for_OK(1000) )  {
			goto ErrorExit;
		}

		addr[0] = (prog_addr >> 24) & 0xFF;
		addr[1] = (prog_addr >> 16) & 0xFF;
		addr[2] = (prog_addr >>  8) & 0xFF;
		addr[3] = (prog_addr >>  0) & 0xFF;
		addr[4] = XOR_bytes(addr, 4);

printf("addr %02X%02X%02X%02X  %02X  ", addr[0], addr[1], addr[2], addr[3], addr[4] );
		QCoreApplication::processEvents();

		serialPort.write((const char *)addr, 5);
		if( wait_for_OK(1000) )  {
			goto ErrorExit;
		}

		int len = qMin(32, prog_len);
		size[0] = len - 1;
		chk[0]  = XOR_bytes(prog_ptr, len);
		chk[0] ^= size[0];
printf("len %02X", size[0]);
		serialPort.write((const char *)size, 1);
		serialPort.write((const char *)prog_ptr, len);
		serialPort.write((const char *)chk, 1);

		if( wait_for_OK(1000) )  {
			goto ErrorExit;
		}
		prog_len  -= len,
		prog_ptr  += len;
		prog_addr += len;

		int percent = (total_len - (wave_len + prog_len)) * 100 / total_len;
		QString text = QString("insert new brain...%1%").arg(percent);
		prog_button->setText(text);
		QCoreApplication::processEvents();
	}

printf("\nreboot");
	prog_button->setText("Zzzzap with high voltage...");
	QCoreApplication::processEvents();
	
	serialPort.write((const char *)reboot, 2);

	wait_for_OK(1000);

	QThread::sleep(1); 

	portDisconnect();
printf("\nupdate done!\n");
	prog_button->setText("Brain replaced!");
	QCoreApplication::processEvents();

	return;
ErrorExit:
	prog_button->setText("Brainsurgery failed!");
	printf("\nERROR\n");
}

void brainSoup::open()
{
	// 0x001B0A74	0x09004000 to             size 932632, xor 0x0D
	// 0x00294590	0x24000000 to 0x24040040  size 262208, xor 0x5D
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Update File"), QDir::currentPath(), tr("(*.bin *.exe)"));
qDebug() << fileName;
	QFile file(fileName);
	if(file.open(QIODevice::ReadOnly)) {

		file.seek(0x001B0A74);
		wave_data = file.read(932632);

		file.seek(0x00294590);
		prog_data = file.read(262208);

		file.close();

		uint8_t *wave_ptr = (uint8_t*)wave_data.constData();
		int wave_len = wave_data.length();
		uint8_t wave_xor = XOR_bytes(  wave_ptr, wave_len);

		uint8_t *prog_ptr = (uint8_t*)prog_data.constData();
		int prog_len = prog_data.length();
		uint8_t prog_xor = XOR_bytes(  prog_ptr, prog_len);
		
printf("%d %02X %d %02X\n", wave_len, wave_xor, prog_len, prog_xor );

		if(wave_xor != 0x0D || prog_xor != 0x5D) {
			prog_button->setText("Bad file!");
			wave_data.clear();
			prog_data.clear();
			return;
		}

		QFileInfo fileInfo(fileName);
		load_button->setText(fileInfo.fileName());
		prog_button->setText("Update");
		prog_button->setEnabled(true);
	}
}

void brainSoup::createActions()
{
	info_label = new QLabel("Brains disconnected", this);
	info_label->setGeometry(QRect(QPoint(10, 160), QSize(200, 50)));

	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

	QAction *exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);

	exitAct->setShortcut(tr("Ctrl+Q"));

	serialPortMenu = menuBar()->addMenu(tr("&Serial Port"));
	updateSerialPorts();
/*
	QMenu *progMenu = menuBar()->addMenu(tr("&Program"));
	progMenu->addAction(tr("&Check"), this, &brainSoup::check);
	progMenu->addAction(tr("Check2"), this, &brainSoup::check2);
	progMenu->addAction(tr("&Erase"), this, &brainSoup::erase);
	progMenu->addAction(tr("&Write"), this, &brainSoup::write);
	progMenu->addAction(tr("&Read"), this, &brainSoup::read);
	progMenu->addAction(tr("&Start"), this, &brainSoup::start);
*/
	QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

	helpMenu->addAction(tr("&About"), this, &brainSoup::about);

	load_button = new QPushButton("load .exe file", this);
	load_button->setGeometry(QRect(QPoint(10, 40), QSize(280, 50)));
	connect(load_button, &QPushButton::released, this, &brainSoup::open);

	prog_button = new QPushButton("no file loaded", this);
	prog_button->setGeometry(QRect(QPoint(10, 100), QSize(280, 50)));
	connect(prog_button, &QPushButton::released, this, &brainSoup::update);
	prog_button->setEnabled(false);
}

void brainSoup::updateActions()
{
}
