#include <QCoreApplication>
#include "QIrc.h"

int main(int argc,char** argv)
{
	QCoreApplication app( argc,argv );
	
	QIrc irc;
	
	return app.exec();
}
