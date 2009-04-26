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
#ifndef __QIRC_H__
#define __QIRC_H__

#include <QtCore>
#include <QtNetwork>
#include <QtScript>

class QIrc : public QObject
{
Q_OBJECT

public:
	QIrc();

public slots:
	// print "message" in the console
	void print( QString message );
	// send "message" to "dest"
	void send( QString dest, QString message );
	// send "message" to "dest" as a notice
	void notice( QString dest, QString message );
	// perform the "message" as an action
	void action( QString dest, QString message );
	// send a raw command
	void sendRaw( QString  s );
	// add an entry in the settings file
	void addValue( QString name, QString value );
	// get the value of an entry in the settings file
	QString getValue( QString name, QString defaut = QString() );
	// return all the value of a group in the settings file
	QStringList getValues( QString name );
	// update the ini file
	void sync();
	// return "true" if the user "name" has been identified as an admin
	bool isAdmin( QString pseudo ) { return admins.contains( pseudo ); };
	// disconnect the bot
	void deconnection( QString message = QString() );
	// send the file "filename" to "dest"
	void xdcc_sendFile( QString dest, QString filename );

private slots:
	void scriptError( const QScriptValue & exception );
	// main socket
	void readData();
	void connecte();
	void deconnecte();
	void displayError( QAbstractSocket::SocketError erreur );
	// for incoming dcc
	void dcc_displayError( QAbstractSocket::SocketError erreur );
	void dcc_transfertFinished( int temps, float vitesse );
	// for the socket from outcoming dcc
	void xdcc_readData();
	void xdcc_connecte();
	void xdcc_deconnecte();
	void xdcc_displayError( QAbstractSocket::SocketError erreur );
	void xdcc_onConnexion();

private:
	void parseCommand( QString s );
	void dispatchMessage( QStringList sender_data, QString destination, QString command );
	void loadScripts();
	void unloadScripts();
	
	QTcpSocket *socket;
	QSettings* conf;
	QString messageRecu;
	bool connected;
	
	QHash< QString, QPair< QScriptEngine *, QScriptValue > > commandes;
	QHash< QString, QVector< QPair< QScriptEngine *, QScriptValue > > > hook_events;
	QList< QScriptEngine * > script_engines;
	// username that you don't want to parse the command (logically, server's bots : NickServ and so)
	QStringList pseudo_bloques;
	// logged admin
	QVector< QString > admins;
	// dcc files sockets (outcome)
	QHash< QTcpSocket *, QString > fichiers_xdcc;
	// the xdcc server
	QTcpServer * xdcc_server;
};

#endif
