#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QRandomGenerator>

#include <QUaServer>

#include <quamultisqlitehistorizer.h>

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
	int msecs = 50;
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

	// start server
	server.start();

	return a.exec();
}
