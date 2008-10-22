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
	hasIdentified = false;
	hasJoinChans = false;
	
	pseudo_bloques = getValue( "bot/ignore" ).split( ',' );
	
	loadScripts();
	
	socket = new QTcpSocket( this );
	connect( socket, SIGNAL(readyRead()), this, SLOT(readData()) );
	connect( socket, SIGNAL(connected()), this, SLOT(connecte()) );
	connect( socket, SIGNAL(disconnected()), this, SLOT(deconnecte()) );
	connect( socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)) );
	
	socket->connectToHost( getValue( "bot/server" ), getValue( "bot/port" ).toInt() );
}

void QIrc::sync()
{
	conf->sync();
}

void QIrc::addValue( QString name, QString value )
{
	conf->setValue( name, value );
	sync();
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
	qDebug() << tr( "Script error : %1" ).arg( exception.toString() );
}

void QIrc::print( QString message )
{
	qDebug() << message;
}

void QIrc::unloadScripts()
{
	commandes.clear();
	hook_events.clear();
	while (!script_engines.isEmpty())
		delete script_engines.takeFirst();
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
			script_engines.append( engine );
			connect(engine, SIGNAL(signalHandlerException(const QScriptValue &)), this, SLOT(scriptError(const QScriptValue &)));
			
			engine->globalObject().setProperty( "irc", engine->newQObject( this ) );
			engine->evaluate( script_content );
			
			qDebug() << tr( "loading : %1" ).arg( entries.at(i).fileName() );
			
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
						qDebug() << tr( "hook command : %1" ).arg( s );
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
						qDebug() << tr( "hook event : %1" ).arg( s );
					}
				}
			}
		}
		else
		{
			qDebug() << tr( "ERROR : couldn't open : scripts/%1" ).arg( entries.at(i).fileName() );
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
	qDebug() << "disconnected";
}

void QIrc::displayError( QAbstractSocket::SocketError erreur )
{
	switch( erreur ) // On affiche un message différent selon l'erreur qu'on nous indique
	{
		case QAbstractSocket::HostNotFoundError:
			qDebug() << tr( "ERROR : host not found." );
			break;
		case QAbstractSocket::ConnectionRefusedError:
			qDebug() << tr( "ERROR : connection refused." );
			break;
		case QAbstractSocket::RemoteHostClosedError:
			qDebug() << tr( "ERROR : remote host closed the connection." );
			break;
		default:
			qDebug() << tr( "ERROR : %1" ).arg( socket->errorString() );
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
				else if( argu[1] == "NOTICE" )
				{
					if( argu[0].mid( 1 ).split( '!' ).at(0) == "NickServ" && !hasJoinChans && hasIdentified )
					{
						joinChans();
					}
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
			else // no "!" no the user has no mask, messages from the host
			{
				bool isInt;
				int code_msg = argu[1].toInt( &isInt );
				if( isInt )
				{
					switch( code_msg )
					{
					case 401:
						// no such nick/channel
						break;
					case 403:
						// no such channel
						break;
					case 404:
						// cannot send text to channel
						break;
					case 405:
						// no many channels joined
						break;
					case 407:
						// duplicate entries
						break;
					case 421:
						// unknown command
						break;
					case 431:
						// no nick given
						break;
					case 432:
						// erroneus nickname (invalid character)
						break;
					case 433:
						// nick already in use
						break;
					case 436:
						// nickname collision
						break;
					case 442:
						// you are not on that channel
						break;
					case 451:
						// you have not registered
						break;
					case 461:
						// no enough parameters
						break;
					case 464:
						// password incorrect
						break;
					case 471:
						// cannot join this channel (full)
						break;
					case 473:
						// cannot join this channel (invite only)
						break;
					case 474:
						// cannot join this channel (you've been ban)
						break;
					case 475:
						// cannot join this channel (need password)
						break;
					case 481:
						// you are not an IRC operator
						break;
					case 482:
						// you are not a channel operator
						break;
					default:
						// useless numbers
						;
					}
				}
				// we don't care about the messages from the server if it's not a "number" message.
			}
		}
		else if( s.section( ' ', 0, 0 ) == "PING" )
		{
			s[1] = 'O';
			sendRaw( s );
			if( !connected )
			{ // if not already connected (it's the first "ping"), send the password if necessary and join the chans
				if( !getValue( "bot/password" ).isEmpty() )
				{
					hasIdentified = true;
					sendRaw( "PRIVMSG NickServ :IDENTIFY " + getValue( "bot/password" ) );
				}
				else
					joinChans();
				connected = true;
			}
		}
		else // used to know which message hasn't been hook
		{
			// this shouldn't happened
			qDebug() << tr( "unparsed message : %1" ).arg( s );
		}
	}
}

void QIrc::joinChans()
{
	sendRaw( "MODE " + getValue( "bot/pseudo" ) + " +B" );
	QStringList canaux = getValue( "bot/chans" ).split( ' ' );
	foreach( QString c, canaux )
	{
		sendRaw( "JOIN #" + c );
	}
	hasJoinChans = true;
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
			notice( sender_data[0], QString("VERSION BotChan: IRC Bot Qt%1").arg(QT_VERSION_STR) );
		}
		else if( cmd == "admin" )
		{
			if( m[0] == getValue( "bot/admin" ) )
			{
				admins += sender_data[1];
				notice( sender_data[0], tr( "you are now identified" ) );
			}
		}
		else
		{
			if( cmd[0] != '!' && commandes.contains( cmd ) )
			{
				QString destt;
				if( m.size() )
				{
					destt = m[0];
					m.removeFirst();
				}
				
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
			else if(isAdmin( sender_data[1] ))
			{
				if( cmd == "quit" )
				{
					deconnection( destination + " " + m.join( " " ) );
				}
				else if( cmd == "reload" )
				{
					unloadScripts();
					loadScripts();
				}
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

void QIrc::deconnection( QString message )
{
	QString msg_quit;
	if( message.length() )
	{
		msg_quit = message;
	}
	else
	{
		msg_quit = getValue( "bot/quit_msg" );
	}
	
	sendRaw( "QUIT " + msg_quit );
	socket->disconnectFromHost();
}
