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
#ifndef __XDCC_H__
#define __XDCC_H__

#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QTcpServer>
#include <QFileInfo>
#include <QTime>

class QIrc;

class XDCC : public QObject
{
Q_OBJECT

public:
	XDCC( QIrc * bot, QString dest, QString filename );

signals:
	void onError( QAbstractSocket::SocketError erreur );
	void onFinished( int temps, float vitesse );

private slots:
	void onConnexion();
	void connecte();
	void deconnecte();
	void displayError( QAbstractSocket::SocketError erreur );
	void onDeletion( QObject * obj );
	void onTimerTimeout();

private:
	QFileInfo fichier;
	unsigned int size;
	QTime beginTime;
	QTcpServer * server;
	QTcpSocket * socket;
	QIrc * parent_irc;
};

#endif
