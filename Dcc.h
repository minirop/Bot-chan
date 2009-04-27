/*
  * BotChan : Scriptable IRC Bot
  * Copyright (C) 2008 Alexander Roper
  * 
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License version 3
  * as published by the Free Software Foundation.
  * 
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */
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
