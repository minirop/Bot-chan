#ifndef __DCC_H__
#define __DCC_H__

#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QFile>
#include <QTime>

class Dcc : public QObject
{
Q_OBJECT

public:
	Dcc( QString host, unsigned int port, QString filename, unsigned int size );

signals:
	void onError( QAbstractSocket::SocketError erreur );
	void onFinished( int temps, float vitesse );

private slots:
	void readData();
	void connecte();
	void deconnecte();
	void displayError( QAbstractSocket::SocketError erreur );

private:
	QFile * fichier;
	QTcpSocket * socket;
	unsigned int fileSize;
	unsigned int receivedBytes;
	QTime beginTime;
};

#endif
