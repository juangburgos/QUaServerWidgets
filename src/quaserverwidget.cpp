#include "quaserverwidget.h"
#include "ui_quaserverwidget.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>

QUaServerWidget::QUaServerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaServerWidget)
{
    ui->setupUi(this);
	m_readOnly = false;
	m_allowStart = true;
#ifndef UA_ENABLE_ENCRYPTION
	ui->labelPrivateKey->setVisible(false);
	ui->framePrivateKey->setVisible(false);
#endif
	this->clear();
}

QUaServerWidget::~QUaServerWidget()
{
    delete ui;
}

void QUaServerWidget::bindServer(QUaServer* server)
{
	// disable old connections
	while (m_connections.count() > 0)
	{
		QObject::disconnect(m_connections.takeFirst());
	}
	// check if valid
	if (!server)
	{
		this->clear();
		this->setEnabled(false);
		return;
	}
	// bind common
	m_connections <<
	QObject::connect(server, &QObject::destroyed, this,
	[this]() {
		this->bindServer(nullptr);
	});
	// set enabled
	this->setEnabled(true);

	// running
	this->setReadOnly(server->isRunning());
	ui->lineEditStatus->setText(server->isRunning() ? tr("Running") : tr("Stopped"));
	ui->pushButtonStart->setText(server->isRunning() ? tr("Stop") : tr("Start"));
	ui->pushButtonStart->setToolTip(server->isRunning() ? tr("Stop Server") : tr("Start Server"));
	m_connections <<
	QObject::connect(server, &QUaServer::isRunningChanged, this,
	[server, this](const bool &running) {
		Q_CHECK_PTR(server);
		this->setReadOnly(running);
		ui->lineEditStatus->setText(running ? tr("Running") : tr("Stopped"));
		ui->pushButtonStart->setText(running ? tr("Stop") : tr("Start"));
		ui->pushButtonStart->setToolTip(running ? tr("Stop Server") : tr("Start Server"));
	});

	// port
	ui->spinBoxPort->setValue(server->port());
	m_connections <<
	QObject::connect(server, &QUaServer::portChanged, this,
	[server, this](const quint16& port) {
		Q_CHECK_PTR(server);
		ui->spinBoxPort->setValue(port);
	});

	// certificate
	m_byteLastCertificate = server->certificate();
	ui->lineEditCertificate->setText(m_byteLastCertificate.isEmpty() ? "" : tr("<ByteArray>"));
	m_connections <<
	QObject::connect(server, &QUaServer::certificateChanged, this,
	[server, this](const QByteArray &byteCertificate) {
		Q_CHECK_PTR(server);
		if (m_byteLastCertificate == byteCertificate)
		{
			return;
		}
		m_byteLastCertificate = byteCertificate;
		ui->lineEditCertificate->setText(m_byteLastCertificate.isEmpty() ? "" : tr("<ByteArray>"));
	});
#ifdef UA_ENABLE_ENCRYPTION
	// private key
	m_byteLastPrivatekey = server->privateKey();
	ui->lineEditPrivateKey->setText(m_byteLastPrivatekey.isEmpty() ? "" : tr("<ByteArray>"));
	m_connections <<
	QObject::connect(server, &QUaServer::privateKeyChanged, this,
	[server, this](const QByteArray &bytePrivateKey) {
		Q_CHECK_PTR(server);
		if (m_byteLastPrivatekey == bytePrivateKey)
		{
			return;
		}
		m_byteLastPrivatekey = bytePrivateKey;
		ui->lineEditPrivateKey->setText(m_byteLastPrivatekey.isEmpty() ? "" : tr("<ByteArray>"));
	});
#endif

	// max secure channels
	ui->spinBoxMaxSecureChannels->setValue(server->maxSecureChannels());
	m_connections <<
	QObject::connect(server, &QUaServer::maxSecureChannelsChanged, this,
	[server, this](const quint16 &maxSecureChannels) {
		Q_CHECK_PTR(server);
		ui->spinBoxMaxSecureChannels->setValue(maxSecureChannels);
	});

	// max sessions
	ui->spinBoxMaxSessions->setValue(server->maxSessions());
	m_connections <<
	QObject::connect(server, &QUaServer::maxSessionsChanged, this,
	[server, this](const quint16 &maxSessions) {
		Q_CHECK_PTR(server);
		ui->spinBoxMaxSessions->setValue(maxSessions);
	});

	// application name
	ui->lineEditApplicationName->setText(server->applicationName());
	m_connections <<
	QObject::connect(server, &QUaServer::applicationNameChanged, this,
	[server, this](const QString &strApplicationName) {
		Q_CHECK_PTR(server);
		ui->lineEditApplicationName->setText(strApplicationName);
	});

	// application URI
	ui->lineEditApplicationURI->setText(server->applicationUri());
	m_connections <<
	QObject::connect(server, &QUaServer::applicationUriChanged, this,
	[server, this](const QString &strApplicationUri) {
		Q_CHECK_PTR(server);
		ui->lineEditApplicationURI->setText(strApplicationUri);
	});

	// product name
	ui->lineEditProductName->setText(server->productName());
	m_connections <<
	QObject::connect(server, &QUaServer::productNameChanged, this,
	[server, this](const QString &strProductName) {
		Q_CHECK_PTR(server);
		ui->lineEditProductName->setText(strProductName);
	});

	// product URI
	ui->lineEditProductURI->setText(server->productUri());
	m_connections <<
	QObject::connect(server, &QUaServer::productUriChanged, this,
	[server, this](const QString &strApplicationUri) {
		Q_CHECK_PTR(server);
		ui->lineEditProductURI->setText(strApplicationUri);
	});

	// manufacturer name
	ui->lineEditManufacturerName->setText(server->manufacturerName());
	m_connections <<
	QObject::connect(server, &QUaServer::manufacturerNameChanged, this,
	[server, this](const QString &strManufacturerName) {
		Q_CHECK_PTR(server);
		ui->lineEditManufacturerName->setText(strManufacturerName);
	});

	// software version
	ui->lineEditSoftwareVersion->setText(server->softwareVersion());
	m_connections <<
	QObject::connect(server, &QUaServer::softwareVersionChanged, this,
	[server, this](const QString &strSoftwareVersion) {
		Q_CHECK_PTR(server);
		ui->lineEditSoftwareVersion->setText(strSoftwareVersion);
	});

	// build number
	ui->lineEditBuildNumber->setText(server->buildNumber());
	m_connections <<
	QObject::connect(server, &QUaServer::buildNumberChanged, this,
	[server, this](const QString & strBuildNumber) {
		Q_CHECK_PTR(server);
		ui->lineEditBuildNumber->setText(strBuildNumber);
	});

	// on start/stop
	m_connections <<
	QObject::connect(ui->pushButtonStart, &QPushButton::clicked, server,
	[server, this]() {
		Q_CHECK_PTR(server);
		server->setIsRunning(!server->isRunning());
		// reload parameters from server to ui that might have been edited but not applied
		this->bindServer(server);
	});
	// on cancel
	m_connections <<
	QObject::connect(ui->pushButtonCancel, &QPushButton::clicked, server,
	[server, this]() {
		Q_CHECK_PTR(server);
		this->bindServer(server);
	});
	// on apply
	m_connections <<
	QObject::connect(ui->pushButtonApply, &QPushButton::clicked, server,
	[server, this]() {
		server->setPort(ui->spinBoxPort->value());
		m_byteLastCertificate = QUaServerWidget::readFileData(this->certificateFile());
		server->setCertificate(m_byteLastCertificate);
#ifdef UA_ENABLE_ENCRYPTION
		m_byteLastPrivatekey = QUaServerWidget::readFileData(this->privateKeyFile());
		server->setPrivateKey(m_byteLastPrivatekey);
#endif
		server->setMaxSecureChannels(ui->spinBoxMaxSecureChannels->value());
		server->setMaxSessions      (ui->spinBoxMaxSessions->value());
		server->setApplicationName  (ui->lineEditApplicationName->text());
		server->setApplicationUri   (ui->lineEditApplicationURI->text());
		server->setProductName      (ui->lineEditProductName->text());
		server->setProductUri       (ui->lineEditProductURI->text());
		server->setManufacturerName (ui->lineEditManufacturerName->text());
		server->setSoftwareVersion  (ui->lineEditSoftwareVersion->text());
		server->setBuildNumber      (ui->lineEditBuildNumber->text());
	});
}

void QUaServerWidget::clear()
{
	// disable old connections
	while (m_connections.count() > 0)
	{
		QObject::disconnect(m_connections.takeFirst());
	}
	// widgets
	ui->lineEditStatus->clear();
	ui->spinBoxPort->setValue(0);
	ui->lineEditCertificate->clear();
#ifdef UA_ENABLE_ENCRYPTION
	ui->lineEditPrivateKey->clear();
#endif
	// internals
	ui->spinBoxMaxSecureChannels->setValue(0);
	ui->spinBoxMaxSessions->setValue(0);
	ui->lineEditApplicationName->clear();
	ui->lineEditApplicationURI->clear();
	ui->lineEditProductName->clear();
	ui->lineEditProductURI->clear();
	ui->lineEditManufacturerName->clear();
	ui->lineEditSoftwareVersion->clear();
	ui->lineEditBuildNumber->clear();
	// as if no server is bound
	this->setReadOnly(true);
	this->setEnabled(false);
}

QString QUaServerWidget::certificateFile() const
{
	return ui->lineEditCertificate->text();
}

void QUaServerWidget::setCertificateFile(const QString &strFileName)
{
	Q_ASSERT_X(this->isEnabled(), "QUaServerWidget::setCertificateFile", "Server should be bound before calling this method.");
	ui->lineEditCertificate->setText(strFileName);
	ui->lineEditCertificate->setToolTip(strFileName);
}

#ifdef UA_ENABLE_ENCRYPTION
QString QUaServerWidget::privateKeyFile() const
{
	return ui->lineEditPrivateKey->text();
}

void QUaServerWidget::setPrivateKeyFile(const QString &strFileName)
{
	Q_ASSERT_X(this->isEnabled(), "QUaServerWidget::setPrivateKeyFile", "Server should be bound before calling this method.");
	ui->lineEditPrivateKey->setText(strFileName);
	ui->lineEditPrivateKey->setToolTip(strFileName);
}
#endif

bool QUaServerWidget::readOnly() const
{
	return m_readOnly;
}

void QUaServerWidget::setReadOnly(const bool& readOnly)
{
	if (readOnly == m_readOnly)
	{
		return;
	}
	m_readOnly = readOnly;
	ui->spinBoxPort               ->setReadOnly(readOnly);
	ui->pushButtonLoadCertificate ->setVisible(!readOnly);
	ui->pushButtonClearCertificate->setVisible(!readOnly);
#ifdef UA_ENABLE_ENCRYPTION
	ui->pushButtonLoadPrivateKey  ->setVisible(!readOnly);
	ui->pushButtonClearPrivateKey ->setVisible(!readOnly);
#endif
	ui->spinBoxMaxSecureChannels->setReadOnly(readOnly);
	ui->spinBoxMaxSessions      ->setReadOnly(readOnly);
	ui->lineEditApplicationName ->setReadOnly(readOnly);
	ui->lineEditApplicationURI  ->setReadOnly(readOnly);
	ui->lineEditProductName     ->setReadOnly(readOnly);
	ui->lineEditProductURI      ->setReadOnly(readOnly);
	ui->lineEditManufacturerName->setReadOnly(readOnly);
	ui->lineEditSoftwareVersion ->setReadOnly(readOnly);
	ui->lineEditBuildNumber     ->setReadOnly(readOnly);
	ui->pushButtonCancel        ->setVisible(!readOnly);
	ui->pushButtonApply         ->setVisible(!readOnly);
	ui->line->setVisible(!readOnly);
	ui->line_2->setVisible(!readOnly);
	emit this->readOnlyChanged();
}

bool QUaServerWidget::allowStart() const
{
	return m_allowStart;
}

void QUaServerWidget::setAllowStart(const bool& allowStart)
{
	if (allowStart == m_allowStart)
	{
		return;
	}
	m_allowStart = allowStart;
	ui->pushButtonStart->setVisible(m_allowStart);
	emit this->allowStartChanged();
}

QByteArray QUaServerWidget::readFileData(const QString& strFileName)
{
	QByteArray byteData;
	if (strFileName.isEmpty())
	{
		// empty is also valid
		return byteData;
	}
	// setup error dialog just in case
	QMessageBox msgBox;
	msgBox.setWindowTitle(tr("Error"));
	msgBox.setIcon(QMessageBox::Critical);
	// create files
	QFile fileConfig(strFileName);
	// exists
	if (!fileConfig.exists())
	{
		msgBox.setText(tr("File %1 does not exist. No data will be loaded.").arg(strFileName));
		msgBox.exec();
		return byteData;
	}
	else if (fileConfig.open(QIODevice::ReadOnly))
	{
		// load config
		byteData = fileConfig.readAll();
	}
	else
	{
		msgBox.setText(tr("File %1 could not be opened. No data will be loaded.").arg(strFileName));
		msgBox.exec();
	}
	// return
	return byteData;
}


void QUaServerWidget::on_pushButtonClearCertificate_clicked()
{
	ui->lineEditCertificate->clear();
}

void QUaServerWidget::on_pushButtonLoadCertificate_clicked()
{
	QString strLastPath = "";
	QString strLastFile = this->certificateFile();
	if (!strLastFile.isEmpty())
	{
		QDir dirLast =  QFileInfo(strLastFile).absoluteDir();
		if (dirLast.exists())
		{
			strLastPath = dirLast.path();
		}
	}
	// read from file
	QString strFileName = QFileDialog::getOpenFileName(this, tr("Open Certificate File (DER format)"),
		strLastPath.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) : strLastPath,
        tr("DER (*.der)")
#if defined(Q_OS_LINUX) && QT_VERSION_MAJOR == 5 && QT_VERSION_MINOR == 12
        , nullptr, QFileDialog::DontUseNativeDialog
#endif
        );
	// set in lineedit
	this->setCertificateFile(strFileName);
}

#ifdef UA_ENABLE_ENCRYPTION
void QUaServerWidget::on_pushButtonClearPrivateKey_clicked()
{
	ui->lineEditPrivateKey->clear();
}

void QUaServerWidget::on_pushButtonLoadPrivateKey_clicked()
{
	QString strLastPath = "";
	QString strLastFile = this->privateKeyFile();
	if (!strLastFile.isEmpty())
	{
		QDir dirLast = QFileInfo(strLastFile).absoluteDir();
		if (dirLast.exists())
		{
			strLastPath = dirLast.path();
		}
	}
	// read from file
	QString strFileName = QFileDialog::getOpenFileName(this, tr("Open Private Key File (DER format)"),
		strLastPath.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) : strLastPath,
        tr("DER (*.der)")
#if defined(Q_OS_LINUX) && QT_VERSION_MAJOR == 5 && QT_VERSION_MINOR == 12
        , nullptr, QFileDialog::DontUseNativeDialog
#endif
        );
	// set in lineedit
	this->setPrivateKeyFile(strFileName);
}
#endif
