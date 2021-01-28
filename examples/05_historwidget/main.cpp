#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QRandomGenerator>

#include <QUaServer>

#include <quamultisqlitehistorizer.h>

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
#include <myevent.h>
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;
	QObject::connect(&server, &QUaServer::logMessage,
	[](const QUaLog &log) {
		qDebug() 
			<< "["   << log.timestamp.toLocalTime().toString("dd.MM.yyyy hh:mm:ss.zzz")
			<< "]["  << log.level
			<< "]["  << log.category
			<< "] :" << log.message;
	});

	QUaFolderObject* objsFolder = server.objectsFolder();

	// set historizer (must live at least as long as the server)
	QUaMultiSqliteHistorizer historizer;
	historizer.setTotalSizeLimMb(10);
	server.setHistorizer(historizer);

	// set speed for generating data
	int msecs = 20;

	// add test variables
	QList<QUaBaseDataVariable*> listVars;
	for (int i = 0; i < 10; i++)
	{
		// create int variable
		auto varInt = objsFolder->addBaseDataVariable(QString("Int%1").arg(i), QString("ns=1;s=Int%1").arg(i));
		// NOTE : must enable historizing for each variable
		varInt->setHistorizing(true);
		varInt->setReadHistoryAccess(true);
		varInt->setValue(0);
		varInt->setMaxHistoryDataResponseSize(2000);
		// store
		listVars << varInt;
	}

	// set random value on timeout	
	QTimer timerVars;
	QObject::connect(&timerVars, &QTimer::timeout, &server, 
	[&listVars, &timerVars, msecs]() {
		// stop timer
		timerVars.stop();
		auto currDt = QDateTime::currentDateTimeUtc();
		// update all variables
		for (auto varInt : listVars)
		{
			varInt->setValue(
				QRandomGenerator::global()->generate(),
				QUaStatus::Good,
				currDt, currDt
			);
		}
		// restart timer
		timerVars.start(msecs);
	});
	// update variable every n milisecs
	timerVars.start(msecs);

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	server.registerType<MyEvent>({ 0, "CustomEventType" });
	// create event with server as originator
	auto srvEvt = server.createEvent<MyEvent>();
	srvEvt->setDisplayName("ServerEvent");
	srvEvt->setSourceName("Server");
	// create cusotm object to trigger a custom event
	auto obj = objsFolder->addBaseObject("CustomObject", { 0, "CustomObject" });
	// enable event history on custom object
	obj->setSubscribeToEvents(true);
	// create custom event with custom object as originator
	auto objEvt = obj->createEvent<MyEvent>();
	objEvt->setDisplayName("CustomEvent");
	objEvt->setSourceName("CustomObject");
	// enable event history on server object
	server.setEventHistoryRead(true);
	server.setMaxHistoryEventResponseSize(2500);
	// enable event history on custom object
	obj->setEventHistoryRead(true);
	QTimer timerEvts;
	QObject::connect(&timerEvts, &QTimer::timeout, srvEvt,
	[srvEvt, objEvt, &timerEvts, msecs]() {
		// stop timer
		timerEvts.stop();
		// increase counter
		static quint32 counter = 0;
		counter++;
		auto currDt = QDateTime::currentDateTimeUtc();
		// trigger server event
		srvEvt->setMessage(QObject::tr("An event occured in the server %1").arg(counter));
		srvEvt->setTime(currDt);
		srvEvt->setReceiveTime(currDt);
		srvEvt->trigger();
		// trigger object event
		objEvt->setMessage(QObject::tr("An event occured in the object %1").arg(counter));
		objEvt->setTime(currDt);
		objEvt->setReceiveTime(currDt);
		objEvt->trigger();
		// restart timer
		timerEvts.start(msecs);
	});	
	// trigger event every second
	timerEvts.start(msecs);
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

	// start server
	server.start();

	return a.exec();
}
