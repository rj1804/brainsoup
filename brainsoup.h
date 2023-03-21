#ifndef _BRAINSOUP_H
#define _BRAINSOUP_H

#include <QMainWindow>
#include <QSerialPort>
#include <QPushButton>

class QLabel;
class QMenu;
class QActionGroup;
class QSerialPort;

class brainSoup:public QMainWindow {
Q_OBJECT public:
	brainSoup();

protected:

private slots:
	void about();
	void check();
	void check2();
	void erase();
	void start();
	void update();
	void open();
	void selectPort();
	void portConnect();
	void portDisconnect();
	void updateSerialPorts();
	void handleError(QSerialPort::SerialPortError serialPortError);

private:
	int wait_for_OK(int timeout);
	void createActions();
	void createMenus();
	void updateActions();

	QMenu *serialPortMenu;
	QActionGroup *serialPorts;

	QString selectedPort;
	QSerialPort serialPort;
	
	QByteArray file_data;
	QByteArray wave_data;
	QByteArray prog_data;

	QPushButton *load_button;
	QPushButton *prog_button;
	
	QLabel *info_label;
	
	QAction *actConnect;
	QAction *actDisconnect;
};

#endif
