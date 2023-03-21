#include <QApplication>

#include "brainsoup.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QGuiApplication::setApplicationDisplayName(brainSoup::tr("BrainSOUP"));

	brainSoup soup;
	soup.show();
	return app.exec();
}
