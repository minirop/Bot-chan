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
#include "QIrc.h"
#include <QtDebug>

QIrc::QIrc()
{
	conf = new QSettings( "config.ini", QSettings::IniFormat, this );
	connected = false;
	
	pseudo_bloques = getValue( "bot/ignore" ).split( ',' );
	
	loadScripts();
	
	socket = new QTcpSocket( this );
	connect( socket, SIGNAL(readyRead()), this, SLOT(readData()) );
	connect( socket, SIGNAL(connected()), this, SLOT(connecte()) );
	connect( socket, SIGNAL(disconnected()), this, SLOT(deconnecte()) );
	connect( socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)) );
	
	socket->connectToHost( getValue( "bot/server" ), getValue( "bot/port" ).toInt() );
}

void QIrc::addValue( QString name, QString value )
{
	conf->setValue( name, value );
	conf->sync();
}

QString QIrc::getValue( QString name, QString defaut )
{
	return conf->value( name, defaut ).toString();
}

QStringList QIrc::getValues( QString name )
{
	conf->beginGroup( name );
    QStringList keys = conf->allKeys();
	conf->endGroup();
	
	return keys;
}

void QIrc::scriptError( const QScriptValue & exception )
{
	qDebug() << "Script error : " + exception.toString();
}

void QIrc::print( QString message )
{
	qDebug() << message;
}

void QIrc::loadScripts()
{
	QDir pluginsDir(qApp->applicationDirPath());
	
	QDir dir( "scripts" );
	QFileInfoList entries = dir.entryInfoList( QStringList() << "*.js" );
	for (int i = 0; i < entries.size(); ++i)
	{
		QFile file( "scripts/" + entries.at(i).fileName() );
		if(file.open(QIODevice::ReadOnly))
		{
			QString script_content = file.readAll();
			file.close();
			
			QScriptEngine* engine = new QScriptEngine( this );
			connect(engine, SIGNAL(signalHandlerException(const QScriptValue &)), this, SLOT(scriptError(const QScriptValue &)));
			
			engine->globalObject().setProperty( "irc", engine->newQObject( this ) );
			engine->evaluate( script_content );
			
			qDebug() << "loading : " + entries.at(i).fileName();
			
			QScriptValue hook = engine->evaluate( "func_hook" );
			if( hook.isValid() )
			{
				QScriptValue hooks = hook.call();
				if( hooks.isString() )
				{
					foreach( QString s, hooks.toString().split( ',' ) )
					{
						commandes[ s ].first = engine;
						commandes[ s ].second = engine->evaluate( "func_exec" );
						qDebug() << "hook command : " + s;
					}
				}
			}
			
			QScriptValue hook_ev = engine->evaluate( "func_hook_event" );
			if( hook_ev.isValid() )
			{
				QScriptValue hooks_event = hook_ev.call();
				if( hooks_event.isString() )
				{
					foreach( QString s, hooks_event.toString().split( ',' ) )
					{
						hook_events[ s ] += qMakePair( engine, engine->evaluate( "func_event" ) );
						qDebug() << "hook event : " + s;
					}
				}
			}
		}
		else
		{
			qDebug() << "ERROR : couldn't open : scripts/" + entries.at(i).fileName();
		}
	}
}

void QIrc::sendRaw( QString s )
{
	socket->write( s.toAscii() + "\r\n" );
}

void QIrc::connecte()
{
	sendRaw( "NICK " + getValue( "bot/pseudo" ) + "\r\nUSER " + getValue( "bot/ident" ) + " " + getValue( "bot/ident" ) + " " + socket->peerName() + " :" + getValue( "bot/real_name" ) );
}

void QIrc::deconnecte()
{
}

void QIrc::displayError( QAbstractSocket::SocketError erreur )
{
	switch( erreur ) // On affiche un message différent selon l'erreur qu'on nous indique
	{
		case QAbstractSocket::HostNotFoundError:
			qDebug() << "ERREUR : le serveur n'a pas pu être trouvé. Vérifiez l'IP et le port.";
			break;
		case QAbstractSocket::ConnectionRefusedError:
			qDebug() << "ERREUR : le serveur a refusé la connexion. Vérifiez si le programme \"serveur\" a bien été lancé. Vérifiez aussi l'IP et le port.";
			break;
		case QAbstractSocket::RemoteHostClosedError:
			qDebug() << "ERREUR : le serveur a coupé la connexion.";
			break;
		default:
			qDebug() << "ERREUR : " + socket->errorString() + "";
	}
}

void QIrc::readData()
{
	messageRecu += socket->readAll();
	bool not_finished = false;
	if(messageRecu.right(2) != "\r\n")
		not_finished = true;
	
	QStringList liste = messageRecu.split( "\r\n", QString::SkipEmptyParts );
	if( not_finished )
	{
		messageRecu = liste.back();
		liste.removeLast();
	}
	else
		messageRecu = QString();
	
	foreach( QString s, liste )
	{
		parseCommand( s );
	}
}

void QIrc::parseCommand( QString s )
{
	if( s.size() )
	{
		if( s[0] == ':' )
		{
			QStringList argu = s.split( ' ' );
			if( argu[0].indexOf( '!' ) > -1 )
			{
				if( argu[1] == "PRIVMSG" ) // a new message arrived
				{
					QString sender_nick = argu[0].mid( 1 );
					QString destination = argu[2];
					argu.removeFirst();
					argu.removeFirst();
					argu.removeFirst();
					
					dispatchMessage( sender_nick.split( '!' ), destination, argu.join( " " ).mid(1) );
				}
				else // an event has occured (somebody join, leave, ... )
				{
					QStringList sender_data = argu[0].mid( 1 ).split( '!' );
					QString ev = argu[1];
					QString destt = argu[2].mid(1);
					argu.removeFirst();
					argu.removeFirst();
					argu.removeFirst();
					
					if( !pseudo_bloques.contains( sender_data[1] ) )
					{
						if( ev == "PART" || ev == "QUIT" )
						{
							// if an admin leave, unregister him.
							// TODO : don't delete him if it's still connected on an other chan where the bot is.
							if( admins.contains( sender_data[1] ) )
							{
								admins.remove( admins.indexOf( sender_data[1] ) );
							}
						}
						
						if( sender_data[0] == getValue( "bot/pseudo" ) )
							ev = "YOU_" + ev; // add "YOU_" when it's a event of the bot
						if( hook_events.contains( ev ) )
						{
							for(int i = 0;i < hook_events[ ev ].size();i++)
							{
								QScriptValue sender_data_array = hook_events[ ev ][i].first->newArray( sender_data.size() );
								for (int k = 0; k < sender_data.size(); ++k)
									sender_data_array.setProperty( k, QScriptValue( hook_events[ ev ][i].first, sender_data.at(k) ) );
								
								QScriptValue argu_m = hook_events[ ev ][i].first->newArray( argu.size() );
								for (int k = 0; k < argu.size(); ++k)
									argu_m.setProperty( k, QScriptValue( hook_events[ ev ][i].first, argu.at(k) ) );
								
								hook_events[ ev ][i].second.call( QScriptValue(), QScriptValueList()	<< QScriptValue( hook_events[ ev ][i].first, ev )
																										<< sender_data_array
																										<< QScriptValue( hook_events[ ev ][i].first, destt )
																										<< argu_m
																										);
							}
						}
					}
				}
			}
			else // no "!" no the user has no mask
			{
				// this shouldn't happened
				qDebug() << "no \"!\" in the sender's ident : " + s;
			}
		}
		else if( s.section( ' ', 0, 0 ) == "PING" )
		{
			s[1] = 'O';
			sendRaw( s );
			if( !connected )
			{ // if not already connected (it's the first "ping"), send the password if necessary and join de chans
				if( !getValue( "bot/password" ).isEmpty() )
					sendRaw( "PRIVMSG NickServ :IDENTIFY " + getValue( "bot/password" ) );
				connected = true;
				QStringList canaux = getValue( "bot/chans" ).split( ',' );
				foreach( QString c, canaux )
				{
					sendRaw( "JOIN " + c );
				}
			}
		}
		else // used to know which message hasn't been hook
		{
			// this shouldn't happened
			qDebug() << "unparsed message : " + s;
		}
	}
}

void QIrc::send( QString dest, QString message )
{
	sendRaw( "PRIVMSG " + dest + " :" + message );
}

void QIrc::action( QString dest, QString message )
{
	sendRaw( "PRIVMSG " + dest + " :ACTION " + message + "" );
}

void QIrc::notice( QString dest, QString message )
{
	sendRaw( "NOTICE " + dest + " :" + message );
}

void QIrc::dispatchMessage( QStringList sender_data, QString destination, QString command )
{
	QStringList m = command.split( " ", QString::SkipEmptyParts );
	QString cmd = m[0];
	m.removeFirst();
	
	if( destination == getValue( "bot/pseudo" ) )
	{
		if( cmd == "VERSION" )
		{
			notice( sender_data[0], "VERSION IRC Bot Qt4.4.0" );
		}
		else if( cmd == "admin" )
		{
			if( m[0] == getValue( "bot/admin" ) )
			{
				admins += sender_data[1];
				send( sender_data[0], "vous êtes bien identifié" );
			}
		}
		else
		{
			if( cmd[0] != '!' && commandes.contains( cmd ) )
			{
				QString destt = m[0];
				m.removeFirst();
				
				QScriptValue sender_data_array = commandes[ cmd ].first->newArray( sender_data.size() );
				for (int k = 0; k < sender_data.size(); ++k)
					sender_data_array.setProperty( k, QScriptValue( commandes[ cmd ].first, sender_data.at(k) ) );
				
				QScriptValue argu_m = commandes[ cmd ].first->newArray( m.size() );
				for (int i = 0; i < m.size(); ++i)
					argu_m.setProperty( i, QScriptValue( commandes[ cmd ].first, m.at(i) ) );
				
				commandes[ cmd ].second.call( QScriptValue(), QScriptValueList()	<< QScriptValue( commandes[ cmd ].first, cmd )
																					<< sender_data_array
																					<< QScriptValue( commandes[ cmd ].first, destt )
																					<< argu_m
																					);
			}
		}
	}
	else
	{
		if( cmd[0] == '!' && commandes.contains( cmd ) )
		{
			QScriptValue sender_data_array = commandes[ cmd ].first->newArray( sender_data.size() );
			for (int k = 0; k < sender_data.size(); ++k)
				sender_data_array.setProperty( k, QScriptValue( commandes[ cmd ].first, sender_data.at(k) ) );
			
			QScriptValue argu_m = commandes[ cmd ].first->newArray( m.size() );
			for (int i = 0; i < m.size(); ++i)
				argu_m.setProperty( i, QScriptValue( commandes[ cmd ].first, m.at(i) ) );
			
			commandes[ cmd ].second.call( QScriptValue(), QScriptValueList()	<< QScriptValue( commandes[ cmd ].first, cmd )
																				<< sender_data_array
																				<< QScriptValue( commandes[ cmd ].first, destination )
																				<< argu_m
																				);
		}
	}
}
