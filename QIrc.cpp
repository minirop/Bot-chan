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
#include "Dcc.h"
#include "XDCC.h"

#include <QtDebug>

QString int_to_ip(unsigned int i)
{
	int a, b, c, d;
	a = i >> 24;
	i -= (a << 24);
	b = i >> 16;
	i -= (b << 16);
	c = i >> 8;
	i -= (c << 8);
	d = i;
	return QString("%1.%2.%3.%4").arg(a).arg(b).arg(c).arg(d);
}

QIrc::QIrc()
{
	conf = new QSettings( "config.ini", QSettings::IniFormat, this );
	connected = false;
	
	pseudo_bloques = getValue( "bot/ignore" ).split( ',' );
	admins = getValue( "admin/hosts" ).remove( ' ' ).split( ',' ).toVector();
	
	loadScripts();
	
#ifndef QT_NO_OPENSSL
	socket = new QSslSocket( this );
	socket->setPeerVerifyMode( QSslSocket::VerifyNone );
	connect( socket, SIGNAL( encrypted() ), this, SLOT( socketEncrypted() ) );
	connect( socket, SIGNAL( sslErrors ( const QList<QSslError> & ) ), this, SLOT( onSslErrors( const QList<QSslError> & ) ) );
#else
	socket = new QTcpSocket( this );
#endif
	connect( socket, SIGNAL( readyRead() ), this, SLOT( readData() ) );
	connect( socket, SIGNAL( connected() ), this, SLOT( connecte() ) );
	connect( socket, SIGNAL( disconnected() ), this, SLOT(deconnecte() ) );
	connect( socket, SIGNAL( error( QAbstractSocket::SocketError ) ), this, SLOT( displayError( QAbstractSocket::SocketError ) ) );
	
#ifndef QT_NO_OPENSSL
	if( getValue( "bot/ssl" ).toInt() )
		socket->connectToHostEncrypted( getValue( "bot/server" ), getValue( "bot/port_ssl" ).toUShort() );
	else
#endif
		socket->connectToHost( getValue( "bot/server" ), getValue( "bot/port" ).toUShort() );
}

#ifndef QT_NO_OPENSSL
void QIrc::onSslErrors( const QList<QSslError> & erreurs )
{
	foreach(QSslError e, erreurs)
	{
		switch(e.error())
		{
			case QSslError::UnableToGetIssuerCertificate:
				print( tr( "SSL : Unable to get issuer certificate" ) );
				break;
			case QSslError::UnableToDecryptCertificateSignature:
				print( tr( "SSL : Unable to decrypt certificate signature" ) );
				break;
			case QSslError::UnableToDecodeIssuerPublicKey:
				print( tr( "SSL : Unable to decode issuer public key" ) );
				break;
			case QSslError::CertificateSignatureFailed:
				print( tr( "SSL : Certificate signature failed" ) );
				break;
			case QSslError::CertificateNotYetValid:
				print( tr( "SSL : Certificate not yet valid" ) );
				break;
			case QSslError::CertificateExpired:
				print( tr( "SSL : Certificate expired" ) );
				break;
			case QSslError::InvalidNotBeforeField:
				print( tr( "SSL : Invalid 'not before' field" ) );
				break;
			case QSslError::InvalidNotAfterField:
				print( tr( "SSL : Invalid 'not after' field" ) );
				break;
			case QSslError::SelfSignedCertificate:
				print( tr( "SSL : Self signed certificate" ) );
				break;
			case QSslError::SelfSignedCertificateInChain:
				print( tr( "SSL : Self signed certificate in chain" ) );
				break;
			case QSslError::UnableToGetLocalIssuerCertificate:
				print( tr( "SSL : Unable to get local issuer certificate" ) );
				break;
			case QSslError::UnableToVerifyFirstCertificate:
				print( tr( "SSL : Unable to verify first certificate" ) );
				break;
			case QSslError::CertificateRevoked:
				print( tr( "SSL : Certificate revoked" ) );
				break;
			case QSslError::InvalidCaCertificate:
				print( tr( "SSL : Invalide Ca certificate" ) );
				break;
			case QSslError::PathLengthExceeded:
				print( tr( "SSL : Path length exceeded" ) );
				break;
			case QSslError::InvalidPurpose:
				print( tr( "SSL : Invalid purpose" ) );
				break;
			case QSslError::CertificateUntrusted:
				print( tr( "SSL : Certificate untrusted" ) );
				break;
			case QSslError::CertificateRejected:
				print( tr( "SSL : Certificate rejected" ) );
				break;
			case QSslError::SubjectIssuerMismatch:
				print( tr( "SSL : Subject issuer mismatch" ) );
				break;
			case QSslError::AuthorityIssuerSerialNumberMismatch:
				print( tr( "SSL : Autority issuer serial number mismatch" ) );
				break;
			case QSslError::NoPeerCertificate:
				print( tr( "SSL : No peer certificate" ) );
				break;
			case QSslError::HostNameMismatch:
				print( tr( "SSL : Hostname mismatch" ) );
				break;
			case QSslError::UnspecifiedError:
				print( tr( "SSL : Unspecified error" ) );
				break;
			case QSslError::NoSslSupport:
				print( tr( "SSL : No SSL Support" ) );
				break;
			case QSslError::NoError:
			default:
				print( tr( "No SSL error ?! WTF ?! : %s" ).arg( e.errorString() ) );
		}
	}
}
#endif

void QIrc::sync()
{
	conf->sync();
}

void QIrc::addValue( QString name, QString value )
{
	conf->setValue( name, value );
	sync();
}

void QIrc::join( QString chan )
{
	sendRaw( "JOIN #" + chan );
}

void QIrc::leave( QString chan, QString reason )
{
	sendRaw( "PART #" + chan + ": " + reason );
}

#ifndef QT_NO_OPENSSL
void QIrc::socketEncrypted()
{
	print( tr( "Encryption successful" ) );
}
#endif

QString QIrc::getValue( QString name, QString defaut )
{
	QVariant tmp = conf->value( name, defaut );
	if( tmp.type() == QVariant::StringList )
		return tmp.toStringList().join( "," );
	else
		return tmp.toString();
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
	print( tr( "Script error : %1" ).arg( exception.toString() ) );
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
			
			print( tr( "loading : %1" ).arg( entries.at(i).fileName() ) );
			
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
						print( tr( "hook command : %1" ).arg( s ) );
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
						print( tr( "hook event : %1" ).arg( s ) );
					}
				}
			}
		}
		else
		{
			print( tr( "ERROR : couldn't open : scripts/%1" ).arg( entries.at( i ).fileName() ) );
		}
	}
}

void QIrc::sendRaw( QString s )
{
	socket->write( s.toUtf8() + "\r\n" );
}

void QIrc::connecte()
{
	print( tr( "Connection established" ) );
	sendRaw( "NICK " + getValue( "bot/pseudo" ) + "\r\nUSER " + getValue( "bot/ident" ) + " " + getValue( "bot/ident" ) + " " + socket->peerName() + " :" + getValue( "bot/real_name" ) );
	
	if( !getValue( "bot/password" ).isEmpty() )
	{
		sendRaw( "PRIVMSG NickServ :IDENTIFY " + getValue( "bot/password" ) );
		print( tr( "password identification" ) );
	}
	
	sendRaw( "MODE " + getValue( "bot/pseudo" ) + " +B" );
}

void QIrc::deconnecte()
{
	print( tr( "disconnected" ) );
}

void QIrc::displayError( QAbstractSocket::SocketError erreur )
{
	switch( erreur ) // On affiche un message différent selon l'erreur qu'on nous indique
	{
		case QAbstractSocket::HostNotFoundError:
			print( tr( "ERROR : host not found." ) );
			break;
		case QAbstractSocket::ConnectionRefusedError:
			print( tr( "ERROR : connection refused." ) );
			break;
		case QAbstractSocket::RemoteHostClosedError:
			print( tr( "ERROR : remote host closed the connection." ) );
			break;
		default:
			print( tr( "ERROR : %1" ).arg( socket->errorString() ) );
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
		parseCommand( QString::fromUtf8( qPrintable( s ) ) );
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
				if( argu[0][0] == '~' || argu[0][0] == ':' )
					argu[0] = argu[0].mid( 1 );
				QStringList tmp_sender_data = argu[0].split( '!' );
				QStringList sender_data = QStringList() << tmp_sender_data[0] << tmp_sender_data[1].split( '@' );
				QString destination = argu[2];
				
				// check the hooked events before everything else
				if( !pseudo_bloques.contains( sender_data[2] ) )
				{
					QString ev = argu[1];
					if( argu.size() > 3 )
					{
						if( argu[3][0] == '~' || argu[3][0] == ':' )
							argu[3] = argu[3].mid(1);
					}
					if( argu[2][0] == ':' )
						argu[2] = argu[2].mid(1);
					QString destt = argu[2];
					QStringList argu2 = argu;
					argu2.removeFirst();
					argu2.removeFirst();
					argu2.removeFirst();
					/*
					if( ev == "PART" || ev == "QUIT" )
					{
						// if an admin leave, unregister him.
						// TODO : don't delete him if it's still connected on an other chan where the bot is.
						if( admins.contains( sender_data[1] ) )
						{
							admins.remove( admins.indexOf( sender_data[1] ) );
						}
					}
					*/
					if( sender_data[0] == getValue( "bot/pseudo" ) )
						ev = "YOU_" + ev; // add "YOU_" when it's a event of the bot
					if( hook_events.contains( ev ) )
					{
						for(int i = 0;i < hook_events[ ev ].size();i++)
						{
							QScriptValue sender_data_array = hook_events[ ev ][i].first->newArray( sender_data.size() );
							for (int k = 0; k < sender_data.size(); ++k)
								sender_data_array.setProperty( k, QScriptValue( hook_events[ ev ][i].first, sender_data.at(k) ) );
							
							QScriptValue argu_m = hook_events[ ev ][i].first->newArray( argu2.size() );
							for (int k = 0; k < argu2.size(); ++k)
								argu_m.setProperty( k, QScriptValue( hook_events[ ev ][i].first, argu2.at(k) ) );
							
							hook_events[ ev ][i].second.call( QScriptValue(), QScriptValueList()	<< QScriptValue( hook_events[ ev ][i].first, ev )
																									<< sender_data_array
																									<< QScriptValue( hook_events[ ev ][i].first, destt )
																									<< argu_m
																									);
						}
					}
				}
				
				if( argu[1] == "PRIVMSG" ) // a new message arrived
				{
					argu.removeFirst();
					argu.removeFirst();
					argu.removeFirst();
					
					dispatchMessage( sender_data, destination, argu.join( " " ) );
				}
				else if( argu[1] == "NOTICE" )
				{
					if( argu[0].left( 8 ) == "HostServ" )
					{
						QStringList canaux = getValue( "bot/chans" ).split( ',' );
						foreach( QString c, canaux )
						{
							join( c );
						}
					}
				}
				else
				{
					qDebug() << "DEBUG : " << argu;
				}
			}
			else // no "!" no the user has no mask, messages from the host
			{
				bool isInt;
				int code_msg = argu[1].toInt( &isInt );
				if( isInt )
				{
					if( argu.back().indexOf( '@' ) > -1 )
					{
						QString addr = argu.back().split( '@' )[1];
						QHostInfo::lookupHost( addr, this, SLOT( lookedUp( QHostInfo ) ) );
					}
					switch( code_msg )
					{
					case 401:
						print( tr("no such nick/channel") );
						break;
					case 403:
						print( tr("no such channel") );
						break;
					case 404:
						print( tr("cannot send text to channel") );
						break;
					case 405:
						print( tr("too many channels joined") );
						break;
					case 407:
						print( tr("duplicate entries") );
						break;
					case 421:
						print( tr("unknown command") );
						break;
					case 431:
						print( tr("no nick given") );
						break;
					case 432:
						print( tr("erroneus nickname (invalid character)") );
						break;
					case 433:
						print( tr("nick already in use") );
						break;
					case 436:
						print( tr("nickname collision") );
						break;
					case 442:
						print( tr("you are not on that channel") );
						break;
					case 451:
						print( tr("you have not registered") );
						break;
					case 461:
						print( tr("no enough parameters") );
						break;
					case 464:
						print( tr("password incorrect") );
						break;
					case 471:
						print( tr("cannot join this channel (full)") );
						break;
					case 473:
						print( tr("cannot join this channel (invite only)") );
						break;
					case 474:
						print( tr("cannot join this channel (you've been ban)") );
						break;
					case 475:
						print( tr("cannot join this channel (need password)") );
						break;
					case 481:
						print( tr("you are not an IRC operator") );
						break;
					case 482:
						print( tr("you are not a channel operator") );
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
		}
		else // used to know which message hasn't been hooked
		{
			// this shouldn't happened
			print( tr( "unparsed message : %1" ).arg( s ) );
		}
	}
}

void QIrc::lookedUp(const QHostInfo &host)
{
	if ( host.error() != QHostInfo::NoError )
	{
		print( tr( "Lookup failed: %1" ).arg( host.errorString() ) );
		return;
	}
	
	QList< QHostAddress > adresses = host.addresses();
	if( adresses.size() == 1 )
	{
		QString ip_f = adresses[0].toString();
		print( tr( "Found address: %1" ).arg( ip_f ) );
		QStringList ip_list = ip_f.split( "." );
		ip_addr = (ip_list[0].toUInt() << 24) | (ip_list[1].toUInt() << 16) | (ip_list[2].toUInt() << 8) | ip_list[3].toUInt();
	}
	else
	{
		print( tr( "%1 addresses found : too many" ).arg( adresses.size() ) );
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
	
//	if( cmd[0] == ':' )
//		cmd = cmd.mid(1);
	
	if( destination == getValue( "bot/pseudo" ) )
	{
		if( cmd == "VERSION" )
		{
			notice( sender_data[0], QString("VERSION BotChan: IRC Bot Qt%1").arg(QT_VERSION_STR) );
		}
		else if( cmd == "DCC" )
		{
			if( m[0] == "SEND" )
			{
				Dcc * file_dcc = new Dcc( int_to_ip(m[2].toUInt() ), m[3].toUShort(), getValue( "dcc/directory" )+"/"+m[1], m[4].remove( '' ).toUInt() );
				connect( file_dcc, SIGNAL( onError( QAbstractSocket::SocketError ) ), this, SLOT( dcc_displayError( QAbstractSocket::SocketError ) ) );
				connect( file_dcc, SIGNAL( onFinished( int, float ) ), this, SLOT( dcc_transfertFinished( int, float ) ) );
				print( tr( "beginning the DL of %1" ).arg(m[1]) );
			}
		}
		else if( cmd == "admin" )
		{
			if( m[0] == getValue( "admin/password" ) )
			{
				if( !admins.contains( sender_data[2] ) )
				{
					admins += sender_data[2];
					notice( sender_data[0], tr( "you are now identified" ) );
					print( tr( "%1 successfully identified himself as an admin" ).arg(sender_data[0]) );
				}
				else
					notice( sender_data[0], tr( "you are already identified" ) );
			}
			else
			{
				notice( sender_data[0], tr( "wrong password" ) );
				print( tr("%1 try the password : %2").arg(sender_data[0]).arg(m[0]) );
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
			else if( isAdmin( sender_data[2] ) )
			{
				if( cmd == "quit" )
				{
					deconnection( destination + " " + m.join( " " ) );
				}
				else if( cmd == "reload" )
				{
					unloadScripts();
					loadScripts();
					notice( sender_data[0], tr("reloaded") );
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
	if( !message.length() )
	{
		message = getValue( "bot/quit_msg" );
	}
	
	sendRaw( "QUIT :" + message );
	socket->disconnectFromHost();
}

void QIrc::dcc_transfertFinished( int temps, float vitesse )
{
	sender()->deleteLater();
	print( tr( "transfert completed in %1sec. (%2ko/s)" ).arg((float)temps/1000).arg(vitesse) );
}

void QIrc::dcc_displayError( QAbstractSocket::SocketError erreur )
{
	switch( erreur )
	{
		case QAbstractSocket::HostNotFoundError:
			print( tr( "ERROR DCC : host not found." ) );
			break;
		case QAbstractSocket::ConnectionRefusedError:
			print( tr( "ERROR DCC : connection refused." ) );
			break;
		case QAbstractSocket::RemoteHostClosedError:
			print( tr( "ERROR DCC : remote host closed the connection." ) );
			break;
		default:
			print( tr( "ERROR DCC : %1" ).arg( socket->errorString() ) );
	}
}

void QIrc::xdcc_sendFile( QString dest, QString filename )
{
	print( tr( "xdcc send file : %1" ).arg( filename ) );
	XDCC * xdcc = new XDCC( this, dest, filename );
	connect( xdcc, SIGNAL( onError( QAbstractSocket::SocketError ) ), this, SLOT( xdcc_displayError( QAbstractSocket::SocketError ) ) );
}

void QIrc::xdcc_displayError( QAbstractSocket::SocketError erreur )
{
	switch( erreur ) // On affiche un message différent selon l'erreur qu'on nous indique
	{
		case QAbstractSocket::HostNotFoundError:
			print( tr( "ERROR XDCC : host not found." ) );
			break;
		case QAbstractSocket::ConnectionRefusedError:
			print( tr( "ERROR XDCC : connection refused." ) );
			break;
		case QAbstractSocket::RemoteHostClosedError:
			print( tr( "ERROR XDCC : remote host closed the connection." ) );
			break;
		default:
			print( tr( "ERROR XDCC : %1" ).arg( socket->errorString() ) );
	}
}
