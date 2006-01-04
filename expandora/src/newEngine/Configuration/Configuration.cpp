#include "Configuration.h"
#include <iostream>
#include <qvariant.h>
#include <qapplication.h>

/**
 * this method is called when a component of this type should be
 * created from a library. MY_EXPORT is defined in Component.h
 * and handles platform specific issues
 */
extern "C" MY_EXPORT Component * createComponent() {
	return new Configuration;
}

Configuration::Configuration() : currentComponent( this ) {
	put( *( new QString( "configuration" ) ), this );
}

void Configuration::start() {
	QFile xmlFile( property( "fileName" ).toString() );
	QXmlInputSource source( &xmlFile );

	QXmlSimpleReader reader;
	reader.setContentHandler( this );
	reader.parse( source );
}

Component * Configuration::get
	( QString & id ) {
	return ( *components.find( id ) ).second;
}


void Configuration::put( QString & id, Component * component ) {
	pair<QString , Component *> val;
	val.first = id;
	val.second = component;
	components.insert( val );
}



bool Configuration::startElement( const QString& , const QString& ,
                                  const QString& qName,
                                  const QXmlAttributes& attributes ) {
	if ( qName == "component" )
		newComponent( attributes );
	else if ( qName == "option" )
		addOption( attributes );
	else if ( qName == "connection" )
		connectComponents( attributes );
	else if ( qName == "configuration" )
		return true;
	else
		return false;
	return true;
}


bool Configuration::endElement ( const QString &, const QString &,
                                 const QString & qName ) {
	if ( qName == "component" )
		currentComponent->start();
	return true;
}

void Configuration::newComponent( const QXmlAttributes & atts ) {
	QString id = atts.value( "id" );
	QString file = atts.value( "file" );
	QLibrary * lib;

	if ( libs.find( file ) == libs.end() ) {
		lib = new QLibrary( file );
		libs.insert( make_pair( file, lib ) );
	} else
		lib = ( *libs.find( file ) ).second;

	componentCreator creator = ( componentCreator ) lib->resolve( "createComponent" );
	if ( creator == 0 ) {
		if ( lib->isLoaded() )
			cout << "library loaded but creator not found: " << lib->fileName().toStdString() << "\n";
		else
			cout << "library can't be loaded: " << lib->fileName().toStdString() << "\n";
		exit(-1) ;
	}
	currentComponent = creator();
	put( id, currentComponent );
}

void Configuration::addOption( const QXmlAttributes & atts ) {
	QString name = atts.value( "name" );
	QString value = atts.value( "value" );
	currentComponent->setProperty( name.toAscii(), value );
}

void Configuration::connectComponents( const QXmlAttributes & atts ) {
	QString temp = atts.value( "from" );
	Component * from = get(temp);
	temp = atts.value( "signal" );
	QString sig = signal(temp);
	temp = atts.value( "to" );
	Component * to = get(temp);
	temp = atts.value( "slot" );
	QString sl = slot(temp);

	Qt::ConnectionType requiredSlot = to->requiredConnectionType(sl.toLatin1());
	Qt::ConnectionType requiredSignal = from->requiredConnectionType(sig.toLatin1());
	
	Qt::ConnectionType resultType;
	if (requiredSlot == requiredSignal || requiredSignal == Qt::AutoCompatConnection)
		resultType = requiredSlot;
	else if (requiredSlot == Qt::AutoCompatConnection)
		resultType = requiredSignal;
	//else throw ConnectionImpossible(from, sig, to, slot);

	to->connect(from, sig.toLatin1(), sl.toLatin1(), resultType);
}


/**
 * an ugyl hack; let's hope qt itself will contain function replacements 
 * for the SIGNAL and SLOT macros in the future
 * @param name name of the signal
 * @return mangled name of the signal
 */
QString & Configuration::signal( QString & name ) {
	return name.prepend( "2" );
}

QString & Configuration::slot( QString & name ) {
	return name.prepend( "1" );
}
