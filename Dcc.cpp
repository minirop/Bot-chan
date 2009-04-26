#include "Dcc.h"

Dcc::Dcc( QString host, unsigned int port, QString filename, unsigned int size )
{
	fichier = new QFile(filename);
	socket = new QTcpSocket(this);
	connect( socket, SIGNAL(readyRead()), this, SLOT(readData()) );
	connect( socket, SIGNAL(connected()), this, SLOT(connecte()) );
	connect( socket, SIGNAL(disconnected()), this, SLOT(deconnecte()) );
	connect( socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)) );
	socket->connectToHost(host, port, QIODevice::ReadOnly);
	fileSize = size;
	receivedBytes = 0;
}

void Dcc::connecte()
{
	fichier->open(QIODevice::WriteOnly);
	beginTime = QTime::currentTime();
}

void Dcc::readData()
{
	QByteArray v = socket->readAll();
	receivedBytes += v.size();
	fichier->write( v );
	fichier->flush();
	if(receivedBytes == fileSize)
	{
		int t = beginTime.msecsTo(QTime::currentTime());
		socket->close();
		emit onFinished(t, (float)fileSize/t);
	}
}

void Dcc::deconnecte()
{
	socket->deleteLater();
	fichier->close();
}

void Dcc::displayError( QAbstractSocket::SocketError erreur )
{
	emit onError( erreur );
}
