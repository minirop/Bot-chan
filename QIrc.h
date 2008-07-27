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
	// return "true" if the user "name" has been identified as an admin
	bool isAdmin( QString pseudo ) { return admins.contains( pseudo ); };

private slots:
	void readData();
	void displayError( QAbstractSocket::SocketError erreur );
	void scriptError( const QScriptValue & exception );
	void connecte();
	void deconnecte();

private:
	void parseCommand( QString s );
	void dispatchMessage( QStringList sender_data, QString destination, QString command );
	void loadScripts();
	
	QTcpSocket *socket;
	QSettings* conf;
	QString messageRecu;
	bool connected;
	
	QHash<QString, QPair< QScriptEngine *, QScriptValue > > commandes;
	QHash<QString, QVector< QPair< QScriptEngine *, QScriptValue > > > hook_events;
	// username that you don't want to parse the command (logically, server's bots)
	QStringList pseudo_bloques;
	// logged admin
	QVector<QString> admins;
};

#endif
