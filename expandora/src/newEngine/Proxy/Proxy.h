#ifndef PROXY
#define PROXY


#include <qobject.h>
#include <QTcpSocket>
#include "Component.h"
#include "ConnectionAccepter.h"

class Proxy : public Component {


		Q_PROPERTY( int localPort READ getLocalPort WRITE setLocalPort )
		Q_PROPERTY( QString remoteHost READ getRemoteHost WRITE setRemoteHost )
		Q_PROPERTY( int remotePort READ getRemotePort WRITE setRemotePort )

	signals:
		void analyzeUserStream( char *, int );
		void analyzeMudStream( char *, int );


	public slots:
		void processUserStream();
		void processMudStream();

	public:
		Proxy() {}
		~Proxy() {}
		
		void run();
		void acceptConnection( ConnectionAccepter * source, int socket );

		int getLocalPort() const {return localPort;}
		void setLocalPort(int & i) {localPort = i;}
		QString getRemoteHost() const {return remoteHost;}
		void setRemoteHost(QString & i) {remoteHost = i;}
		int getRemotePort() const {return remotePort;}
		void setRemotePort(int & i) {remotePort = i;}
		
	private:
		Q_OBJECT

		QString remoteHost;
		int remotePort;
		int localPort;
		QTcpSocket mudSocket;
		QTcpSocket userSocket;
		char buffer[ 8192 ];

	};
#endif

#ifdef DMALLOC
#include <dmalloc.h>
#endif
