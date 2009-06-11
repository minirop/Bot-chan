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
#include "Dcc.h"

Dcc::Dcc( QString host, unsigned int port, QString filename, unsigned int size )
{
	fichier = new QFile( filename );
	socket = new QTcpSocket( this );
	connect( socket, SIGNAL( readyRead() ), this, SLOT( readData() ) );
	connect( socket, SIGNAL( connected() ), this, SLOT( connecte() ) );
	connect( socket, SIGNAL( disconnected() ), this, SLOT( deconnecte() ) );
	connect( socket, SIGNAL( error( QAbstractSocket::SocketError ) ), this, SIGNAL( onError( QAbstractSocket::SocketError ) ) );
	socket->connectToHost(host, port, QIODevice::ReadOnly);
	fileSize = size;
	receivedBytes = 0;
}

void Dcc::connecte()
{
	fichier->open( QIODevice::WriteOnly );
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
