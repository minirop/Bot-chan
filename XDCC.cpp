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
#include "XDCC.h"
#include "QIrc.h"

XDCC::XDCC( QIrc * bot, QString dest, QString filename )
{
	parent_irc = bot;
	server = new QTcpServer( this );
	socket = 0;
	if( !server->listen(QHostAddress::Any, 5990) )
	{
		parent_irc->print( tr( "can't lister on port 5990" ) );
		deleteLater();
		return;
	}
	
	connect( server, SIGNAL( newConnection() ), this, SLOT( onConnexion() ) );
	connect( server, SIGNAL( destroyed( QObject * ) ), this, SLOT( onDeletion( QObject * ) ) );
	fichier = QFileInfo( filename );
	parent_irc->send( dest, QString( "DCC SEND \"%1\" %2 %3 %4" ).arg( fichier.fileName() ).arg( "123123123" ).arg( 5990 ).arg( fichier.size() ) );
	QTimer::singleShot( 10000, this, SLOT( onTimerTimeout() ) );
}

void XDCC::onTimerTimeout()
{
	if( !socket )
	{
		server->close();
		deleteLater();
	}
}

void XDCC::onDeletion( QObject * obj )
{
	parent_irc->print( tr( "the server listening on port %1 has been deleted" ).arg( 5990 ) );
}

void XDCC::onConnexion()
{
	socket = server->nextPendingConnection();
	connect( socket, SIGNAL( connected() ), this, SLOT( connecte() ) );
	connect( socket, SIGNAL( disconnected() ), this, SLOT( deconnecte() ) );
	connect( socket, SIGNAL( error( QAbstractSocket::SocketError ) ), this, SIGNAL( onError( QAbstractSocket::SocketError ) ) );
}

void XDCC::connecte()
{
	QFile f( fichier.canonicalFilePath() );
	f.open( QIODevice::ReadOnly );
	socket->write( f.readAll() );
	parent_irc->print( tr( "transfert beginning" ) );
}

void XDCC::deconnecte()
{
	socket->deleteLater();
	deleteLater();
}

void XDCC::displayError( QAbstractSocket::SocketError erreur )
{
	emit onError( erreur );
}
