//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulauncherdialog.cpp,v 1.112 2022/04/11 20:06:22 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aulauncherdialog.h"
//
#include "audice.h"
#include "aulaunchersettings.h"
#include "auutil.h"
#include <QGuiApplication>
#include <QCryptographicHash>
#include <QDate>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPainter>
#include <QProcess>
#include <QTimer>
#include <QToolButton>
#include <QWheelEvent>

#define d_BackdropFiles				20
#define d_MyVersion					1.8

AuLauncherNetworkReply::AuLauncherNetworkReply(const QUrl& url, QNetworkReply* reply)
{
	m_Url = url;
	m_NetReply = reply;
	if(m_NetReply)
		connect(m_NetReply, SIGNAL(downloadProgress(qint64, qint64)), SLOT(BaseDownloadProgress(qint64, qint64)));
}

void AuLauncherNetworkReply::BaseDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	emit DownloadProgress(m_Url, bytesReceived, bytesTotal);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuLauncherDialog::AuLauncherDialog(QWidget* parent) : QWidget(parent),
	m_Settings("Hazeron Freehold", "Shores of Hazeron", parent)
{
	m_CommState = 0;
	m_ErrorMessage = 0;
	m_Timer = 0;

	setWindowTitle(tr("Shores of Hazeron Launcher"));
	setWindowIcon(QIcon(":data/Launcher.png"));

	bool canLaunchSoH = CanLaunchSoH();
	bool canLaunchCommander = CanLaunchCommander();
	bool canLaunchMail = CanLaunchMail();
	//bool canLaunchStoryEditor = CanLaunchStoryEditor();
	if(!canLaunchSoH && !canLaunchCommander && !canLaunchMail /*&& !canLaunchStoryEditor*/)
	{
		// None of the programs can be launched. This looks like an initial install.
		// Load a specific backdrop.
		m_Backdrop.load(":data/Backdrop4.png");
		m_BackdropTime.start();

		m_Init = new QToolButton(this);
		m_Init->setIcon(QIcon(":data/Wormhole.png"));
		m_Init->setIconSize(QSize(256,245));
		m_Init->setToolTip(tr("<html><b>Shores of Hazeron Initial Update</b><br>"
					"Downloads and configures the files needed to play <i>Shores of Hazeron</i>.<br><br>"
					"About 300MB of files will be stored on your computer. "
					"The download folder can be configured using the <b>Settings</b> button before the download begins."
#if defined(Q_OS_MACOS)
					"<br><b>Use the Settings button to configure your OpenGL version now.</b>"
#endif
					"</html>"));
		m_Init->setAutoRaise(true);
		m_Init->setFocusPolicy(Qt::NoFocus);
		connect(m_Init, SIGNAL(clicked()), SLOT(InitInstall()));
	}
	else
	{
		// Something is already here. Set up for a normal run with automatic check for updates.
		// Load a random backdrop.
		LoadRandomBackdrop();

		m_Init = 0;
	}

	m_ErrorMessage = new QLabel(tr("<html><b><big><big>Communication Error</big></big></b><br>"
				"Information about the latest updates cannot be retrieved from the <i>Shores of Hazeron</i> web site.<p>"
				"<i>Troubleshooting Tips</i><br>"
				"- Check your computer's connection to the Internet.<br>"
				"- Try opening the <u>Hazeron.com</u> web site with your browser, "
				"to ensure the server is accessible.<br>"
				"- Try running this program again in a few minutes; we could be updating the files right now."
				"</html>"), this);
	QBrush brush(QColor(160, 179, 249));
	QPalette palette;
	palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
	palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
	palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush);
	m_ErrorMessage->setPalette(palette);
	m_ErrorMessage->setWordWrap(true);
	m_ErrorMessage->hide();

	m_UpdateMessage = new QLabel("", this);
	m_UpdateMessage->setWordWrap(true);
	m_UpdateMessage->hide();

	m_LaunchSoH = new QToolButton(this);
	m_LaunchSoH->setIcon(QIcon(":data/Hazeron.png"));
	m_LaunchSoH->setIconSize(QSize(256,144));
	m_LaunchSoH->setToolTip(tr("<html><b>Shores of Hazeron</b><br>"
				"Launches <i>Shores of Hazeron</i>, the MMO sandbox universe.<br>"
				"</html>"));
	m_LaunchSoH->setFocusPolicy(Qt::NoFocus);
	connect(m_LaunchSoH, SIGNAL(clicked()), SLOT(LaunchSoH()));

	m_LaunchCommander = new QToolButton(this);
	m_LaunchCommander->setIcon(QIcon(":data/Commander.png"));
	m_LaunchCommander->setIconSize(QSize(128,72));
	m_LaunchCommander->setToolTip(tr("<html><b>Hazeron Commander</b><br>"
				"Launches <i>Hazeron Commander</i> for limited interaction with <i>Shores of Hazeron</i>. "
				"No OpenGL view of the scene is provided. "
				"Most actions that do not require the OpenGL scene can be performed."
				"</html>"));
	m_LaunchCommander->setFocusPolicy(Qt::NoFocus);
	connect(m_LaunchCommander, SIGNAL(clicked()), SLOT(LaunchCommander()));

	m_LaunchMail = new QToolButton(this);
	m_LaunchMail->setIcon(QIcon(":data/Mail.png"));
	m_LaunchMail->setIconSize(QSize(128,72));
	m_LaunchMail->setToolTip(tr("<html><b>Hazeron Mail</b><br>"
				"Launches <i>Hazeron Mail</i> for sending and receiving mail in <i>Shores of Hazeron</i> "
				"without launching the full game program."
				"</html>"));
	m_LaunchMail->setFocusPolicy(Qt::NoFocus);
	connect(m_LaunchMail, SIGNAL(clicked()), SLOT(LaunchMail()));

	/*
	m_LaunchStoryEditor = new QToolButton(this);
	m_LaunchStoryEditor->setIcon(QIcon(":data/StoryEditor.png"));
	m_LaunchStoryEditor->setIconSize(QSize(128,72));
	m_LaunchStoryEditor->setToolTip(tr("<html><b>Hazeron Story Editor</b><br>"
				"Launches <i>Hazeron Story Editor</i>, a story authoring tool for the game of <i>Shores of Hazeron</i>. "
				"</html>"));
	m_LaunchStoryEditor->setFocusPolicy(Qt::NoFocus);
	connect(m_LaunchStoryEditor, SIGNAL(clicked()), SLOT(LaunchStoryEditor()));
	*/

	m_Files = new QListWidget(this);
	m_Files->setFocusPolicy(Qt::NoFocus);
	m_Files->hide();

	m_WebSite = new QToolButton(this);
	m_WebSite->setAutoRaise(true);
	m_WebSite->setFocusPolicy(Qt::NoFocus);
	m_WebSite->setIcon(QIcon(":data/Launcher.png"));
	m_WebSite->setIconSize(QSize(32,32));
	m_WebSite->setToolTip(tr("<html><b>Hazeron.com</b><br>"
				"Opens your web browser at the <i>Shores of Hazeron</i> web site.<p>"
				"<i>Shores of Hazeron Launcher</i> Usage Tips:<br>"
				"- Escape or Double-Click - Toggles button visibility on and off.<br>"
				"- Up Arrow - Selects a picture at random. "
				"Pictures are loaded from this program <u>and from your picture folder</u>. "
				"Your picture folder can be configured using the <b>Settings</b> button.<br>"
				"- Down Arrow - Toggles between window and full screen modes.<br>"
				"- Left Arrow - Select the previously viewed picture.<br>"
				"- Right Arrow - Select the next viewed picture.<br>"
				"- Wheel - Select the previous/next viewed picture."
				"</html>"));
	connect(m_WebSite, SIGNAL(clicked()), SLOT(WebSite()));

	m_SettingsButton = new QToolButton(this);
	m_SettingsButton->setAutoRaise(true);
	m_SettingsButton->setFocusPolicy(Qt::NoFocus);
	m_SettingsButton->setIcon(QIcon(":data/Settings.png"));
	m_SettingsButton->setIconSize(QSize(32,32));
	m_SettingsButton->setToolTip(tr("<html><b>Settings...</b><br>"
				"Configures <i>Shores of Hazeron</i> launcher settings."
				"</html>"));
	connect(m_SettingsButton, SIGNAL(clicked()), SLOT(Settings()));

	QVBoxLayout* vlay = new QVBoxLayout;
	vlay->addWidget(m_WebSite);
	vlay->addWidget(m_SettingsButton);

	QHBoxLayout* hlay = new QHBoxLayout;
	hlay->addLayout(vlay);
	hlay->addStretch();

	QGridLayout* blay = new QGridLayout;
	blay->addWidget(m_LaunchSoH, 1,1, 2,1);
	blay->addWidget(m_LaunchCommander, 1,2);
	blay->addWidget(m_LaunchMail, 2,2);
	//lay->addWidget(m_LaunchStoryEditor, row,2);
	blay->setRowStretch(0, 1);
	blay->setRowStretch(3, 1);
	blay->setColumnStretch(0, 1);
	blay->setColumnStretch(3, 1);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addLayout(hlay, row,0, 1,6);
	row++;
	lay->setRowStretch(row, 1);
	row++;
	lay->addWidget(m_ErrorMessage, row,1);
	row++;
	lay->addWidget(m_UpdateMessage, row,1);
	row++;
	if(m_Init)
		lay->addWidget(m_Init, row++,1);
	lay->setRowStretch(row, 1);
	row++;
	lay->addWidget(m_Files, row,1, 1,5);
	row++;
	lay->addLayout(blay, row,0, 1,6);
	row++;
	lay->setRowMinimumHeight(row, 20);
	lay->setColumnMinimumWidth(0, 20);
	lay->setColumnStretch(4, 2);
	lay->setColumnStretch(5, 1);
	lay->setSpacing(8);
	lay->setMargin(12);
	setLayout(lay);

	// Determine which architecture we will download.
#if defined(Q_OS_MACOS)
	QFile file(qApp->applicationDirPath() + "/../Resources/data.bin");
#else
	QFile file(qApp->applicationDirPath() + "/data.bin");
#endif
	if(file.open(QIODevice::ReadOnly))
	{
		QTextStream ts(&file);
		ts >> m_Arch;
		file.close();
#if defined(Q_OS_MACOS)
		m_Arch.replace("-gl41-", "-");	// Remove this thing if it's in there already.
#endif
		//qDebug() << "Architecture" << m_Arch;
	}
	HideLauncherButtons();
	if(!m_Init)
		CheckForLauncherUpdates();
}

AuLauncherDialog::~AuLauncherDialog()
{
}

QSize AuLauncherDialog::sizeHint() const
{
	return QSize(890, 500);
}

void AuLauncherDialog::paintEvent(QPaintEvent* pe)
{
	QPainter p(this);
	p.drawImage(rect(), m_Backdrop);

	// Title
	QRect titleRect(rect());
	titleRect.setTop(titleRect.top() + 40);	// top margin
	QImage imgTitle(":data/TitleText.png");
	QPoint imgLoc((titleRect.left() + titleRect.right())/2 - imgTitle.width()/2, titleRect.top());
	p.drawImage(imgLoc, imgTitle);

	// Copyright
#if defined(Q_OS_WIN)
	QString Csymbol = QString(QChar(0xa9));
#else
#if defined(UNICODE)
	QString Csymbol = QString("©");
#else
	QString Csymbol = QString::fromLocal8Bit("©");
#endif
#endif
	QString copy;
	if(QString(AuUtil::CopyrightYear()) == QString("2022"))
		copy = QString("Copyright %1 2022 Hazeron Freehold LLC\nAll rights reserved.")
			.arg(Csymbol);
	else
		copy = QString("Copyright %1 2022-%2 Hazeron Freehold LLC\nAll rights reserved.")
			.arg(Csymbol)
			.arg(AuUtil::CopyrightYear());

	QString typeface;
#if defined(Q_OS_WIN)
	typeface = QString("Times New Roman");
#elif defined(Q_OS_MACOS)
	typeface = QString("Baskerville");
#else
	typeface = QString("Liberation Serif");
#endif
	p.setFont(QFont(typeface, 8, QFont::Light, true));
	QRect copyRect(rect());
	copyRect.setBottom(copyRect.bottom()-2);	// bottom margin
	QRect copyShadow(copyRect);
	copyShadow.moveTopLeft(QPoint(copyShadow.left()+1,copyShadow.top()+1));
	p.setPen(QPen(QColor(0,0,0,128), 0.f));
	p.drawText(copyShadow, Qt::AlignBottom|Qt::AlignHCenter, copy);
	p.setPen(QPen(QColor(160, 179, 249), 0.f));
	p.drawText(copyRect, Qt::AlignBottom|Qt::AlignHCenter, copy);

	QWidget::paintEvent(pe);
}

QString AuLauncherDialog::FileDir() const
{
	QString sohPath = m_Settings.value("Settings/BaseDir").toString();
	if(sohPath.isEmpty())
		sohPath = QDir::home().absoluteFilePath("Shores of Hazeron");
	return sohPath + "/Programs";
}

bool AuLauncherDialog::CanLaunchStoryEditor() const
{
#if defined(Q_OS_WIN)
	QString launch = FileDir() + "/Hazeron Story Editor.exe";
	return QFile::exists(launch);
#elif defined(Q_OS_MACOS)
	QString launch = FileDir() + "/StoryEditor.app/Contents/MacOS/StoryEditor";
	return QFile::exists(launch);
#else
	QString launch = FileDir() + "/StoryEditor";
	QString launch2 = FileDir() + "/AuStoryEditor";
	return QFile::exists(launch) && QFile::exists(launch2);
#endif
}

void AuLauncherDialog::LaunchStoryEditor()
{
#if defined(Q_OS_WIN)
	QString launch = FileDir() + "/Hazeron Story Editor.exe";
#elif defined(Q_OS_MACOS)
	QString launch = FileDir() + "/StoryEditor.app/Contents/MacOS/StoryEditor";
#else
	QString launch = FileDir() + "/StoryEditor";
#endif
	//qDebug() << "Launching" << launch;
	QStringList args;
	QProcess::startDetached(launch, args, FileDir());
	close();
}

bool AuLauncherDialog::CanLaunchCommander() const
{
	if(m_ErrorMessage && m_ErrorMessage->isVisible())
		return false;

#if defined(Q_OS_WIN)
	QString launch = FileDir() + "/Hazeron Commander.exe";
	return QFile::exists(launch);
#elif defined(Q_OS_MACOS)
	QString launch = FileDir() + "/SoHCommander.app/Contents/MacOS/SoHCommander";
	return QFile::exists(launch);
#else
	QString launch = FileDir() + "/HazeronCommander";
	QString launch2 = FileDir() + "/AuChatClient";
	return QFile::exists(launch) && QFile::exists(launch2);
#endif
}

void AuLauncherDialog::LaunchCommander()
{
	if(m_ErrorMessage && m_ErrorMessage->isVisible())
		return;

#if defined(Q_OS_WIN)
	QString launch = FileDir() + "/Hazeron Commander.exe";
#elif defined(Q_OS_MACOS)
	QString launch = FileDir() + "/SoHCommander.app/Contents/MacOS/SoHCommander";
#else
	QString launch = FileDir() + "/HazeronCommander";
#endif

#if defined(Q_OS_MACOS)
	// Enable execute permission on the file.
	QFileDevice::Permissions perms = QFile::permissions(launch);
	perms |= QFileDevice::ExeOwner|QFileDevice::ExeUser;
	QFile::setPermissions(launch, perms);
#endif

	//qDebug() << "Launching" << launch;
	QStringList args;
	QProcess::startDetached(launch, args, FileDir());
	close();
}

bool AuLauncherDialog::CanLaunchMail() const
{
	if(m_ErrorMessage && m_ErrorMessage->isVisible())
		return false;

#if defined(Q_OS_WIN)
	QString launch = FileDir() + "/Hazeron Mail.exe";
	return QFile::exists(launch);
#elif defined(Q_OS_MACOS)
	QString launch = FileDir() + "/SoHMail.app/Contents/MacOS/SoHMail";
	return QFile::exists(launch);
#else
	QString launch = FileDir() + "/HazeronMail";
	QString launch2 = FileDir() + "/AuMailClient";
	return QFile::exists(launch) && QFile::exists(launch2);
#endif
}

void AuLauncherDialog::LaunchMail()
{
	if(m_ErrorMessage && m_ErrorMessage->isVisible())
		return;

#if defined(Q_OS_WIN)
	QString launch = FileDir() + "/Hazeron Mail.exe";
#elif defined(Q_OS_MACOS)
	QString launch = FileDir() + "/SoHMail.app/Contents/MacOS/SoHMail";
#else
	QString launch = FileDir() + "/HazeronMail";
#endif

#if defined(Q_OS_MACOS)
	// Enable execute permission on the file.
	QFileDevice::Permissions perms = QFile::permissions(launch);
	perms |= QFileDevice::ExeOwner|QFileDevice::ExeUser;
	QFile::setPermissions(launch, perms);
#endif

	//qDebug() << "Launching" << launch;
	QStringList args;
	QProcess::startDetached(launch, args, FileDir());
	close();
}

bool AuLauncherDialog::CanLaunchSoH() const
{
	if(m_ErrorMessage && m_ErrorMessage->isVisible())
		return false;

#if defined(Q_OS_WIN)
	QString launch = FileDir() + "/Shores of Hazeron.exe";
	return QFile::exists(launch);
#elif defined(Q_OS_MACOS)
	QString launch = FileDir() + "/SoH.app/Contents/MacOS/SoH";
	return QFile::exists(launch);
#else
	QString launch = FileDir() + "/ShoresOfHazeron";
	QString launch2 = FileDir() + "/AuSceneClient";
	return QFile::exists(launch) && QFile::exists(launch2);
#endif
}

void AuLauncherDialog::LaunchSoH()
{
	if(m_ErrorMessage && m_ErrorMessage->isVisible())
		return;

#if defined(Q_OS_WIN)
	QString launch = FileDir() + "/Shores of Hazeron.exe";
#elif defined(Q_OS_MACOS)
	QString launch = FileDir() + "/SoH.app/Contents/MacOS/SoH";
#else
	QString launch = FileDir() + "/ShoresOfHazeron";
#endif

#if defined(Q_OS_MACOS)
	// Enable execute permission on the file.
	QFileDevice::Permissions perms = QFile::permissions(launch);
	perms |= QFileDevice::ExeOwner|QFileDevice::ExeUser;
	QFile::setPermissions(launch, perms);
#endif

	//qDebug() << "Launching" << launch;
	QStringList args;
	QProcess::startDetached(launch, args, FileDir());
	close();
}

void AuLauncherDialog::CheckForLauncherUpdates()
{
	m_CommState = 4;											// Wait for launcher version.

	// Request the file that contains the current launcher version.
	m_Net = new QNetworkAccessManager(this);
	connect(m_Net, SIGNAL(finished(QNetworkReply*)), SLOT(NetReplyFinished(QNetworkReply*)));
	connect(m_Net, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
			SLOT(NetSSLErrors(QNetworkReply*, const QList<QSslError>&)));
	m_Net->get(QNetworkRequest(QUrl(QString("http://Hazeron.com/software/LauncherVersion.%1").arg(Arch()))));
}

void AuLauncherDialog::CheckForProgramUpdates()
{
	//qDebug() << "AuLauncherDialog::CheckForProgramUpdates";
	QString arch(Arch());
	if(!arch.isEmpty())
	{
		// Request the file of md5 checksums for all of our files.
		m_CommState = 1;											// Wait for checksums.
		m_Net = new QNetworkAccessManager(this);
		connect(m_Net, SIGNAL(finished(QNetworkReply*)), SLOT(NetReplyFinished(QNetworkReply*)));
		m_Net->get(QNetworkRequest(QUrl(QString("http://Hazeron.com/software/%1/md5sums").arg(arch))));
	}
	else
	{
		// This is fatal.
		QMessageBox::critical(this, tr("Launcher Error"),
				tr("<html>The <i>Shores of Hazeron Launcher</i> installation is broken. "
					"Try reinstalling from the original distribution files."
					"</html>"));
		close();
	}
}

void AuLauncherDialog::NetReplyFinished(QNetworkReply* reply)
{
	Q_ASSERT(m_CommState);
	//qDebug() << "AuLauncherDialog::NetReplyFinished" << reply->bytesAvailable() << "bytes available, isOpen" << reply->isOpen();
	//qDebug() << "AuLauncherDialog::NetReplyFinished" << reply->url().path();
	if(m_CommState == 4)
	{
		// Receive the launcher version.
		int verState = 0;		// 0=ok, 1=out of date
		int priorityState = 0;	// 0=ok, 1=critical, 2=platform, 3=obsolete
		while(reply->canReadLine())
		{
			QString line = reply->readLine().trimmed();
			if(line.left(1) == "#")
				continue;
			QStringList fields = line.split('=');
			if(fields.size() != 2)
				continue;
			if(fields[0].compare("version", Qt::CaseInsensitive) == 0)
			{
				// File contains a version. We expect to be able to compare this like a double.
				if(fields[1].toDouble() > d_MyVersion)
					verState = 1;	// out of date
			}
			else if(fields[0].compare("priority", Qt::CaseInsensitive) == 0)
			{
				// File contains a priority.
				if(fields[1].compare("critical", Qt::CaseInsensitive) == 0)
					priorityState = 1;
				else if(fields[1].compare("platform", Qt::CaseInsensitive) == 0)
					priorityState = 2;
				else if(fields[1].compare("obsolete", Qt::CaseInsensitive) == 0)
					priorityState = 3;
				else
					priorityState = 0;	// normal
			}
		}

		bool okToDownload = true;
		if(verState == 1)
		{
			// Current version is out of date.
			switch(priorityState)
			{
			case 0:	// Normal
			default:
				m_UpdateMessage->setText(tr("<html>"
							"<p>"
							"An update is available for the <i>Shores of Hazeron Launcher</i>. "
							"</p>"
							"<p>"
							"The update is not required. "
							"It may add features, fix minor errors, or change the program's appearance. "
							"</p>"
							"<p>"
							"The launcher cannot update itself. "
							"Download the latest launcher for free from Hazeron.com. "
							"</p>"
							"</html>"));
				break;
			case 1:	// Critical
				okToDownload = false;
				m_UpdateMessage->setText(tr("<html>"
							"<p>"
							"A <u>critical update</u> is available for the <i>Shores of Hazeron Launcher</i>. "
							"</p>"
							"<p>"
							"<b>No files can be updated. "
							"A new launcher is needed.</b> "
							"</p>"
							"<p>"
							"The launcher cannot update itself. "
							"Download the latest launcher for free from Hazeron.com. "
							"</p>"
							"</html>"));
				break;
			case 2:	// Platform
				okToDownload = false;
				m_UpdateMessage->setText(tr("<html>"
							"<p>"
							"A <u>platform upgrade</u> is available for the <i>Shores of Hazeron Launcher</i>. "
							"This version has reached the end of its service life. "
							"</p>"
							"<p>"
							"<b>No files can be updated. "
							"A new launcher is needed.</b> "
							"</p>"
							"<p>"
							"The launcher cannot update itself. "
							"Download the latest launcher for free from Hazeron.com. "
							"</p>"
							"<p>"
							"The current game software can be launched. "
							"It will report when its own version becomes incompatible with the servers. "
							"</p>"
							"</html>"));
				break;
			case 3:	// Obsolete
				okToDownload = false;
				m_UpdateMessage->setText(tr("<html>"
							"<p>"
							"A <u>critical platform upgrade</u> is available for the <i>Shores of Hazeron Launcher</i>. "
							"This version has reached the end of its service life. "
							"</p>"
							"<p>"
							"<b>No files can be updated. "
							"A new launcher is needed.</b> "
							"</p>"
							"<p>"
							"The launcher cannot update itself. "
							"Download the latest launcher for free from Hazeron.com. "
							"</p>"
							"</html>"));
				break;
			}
			m_UpdateMessage->show();
		}

		// Check for updates to the Shores of Hazeron software.
		if(okToDownload)
			CheckForProgramUpdates();
	}
	else if(m_CommState == 1)
	{
		// Receive the md5sums file which also provides a list of the files we care about.
		QMap<QString, QString> filesToCheck;
		while(reply->canReadLine())
		{
			QString line = reply->readLine();

			// First part of line is md5 checksum. Last part of the line is the file name.
			int space = line.indexOf(' ');
			if(space != 32)
			{
				filesToCheck.clear();
				break;
			}
			QString md5sum = line.left(space);
			QString file = line.mid(space+1).trimmed();
			if(file != "./md5sums")
				filesToCheck.insert(file, md5sum);
		}

		// Check local files for proper checksums.
		QMap<QString, int> filesToUpdate;		// Key is file name, value is 0=missing, 1=changed
		for(QMap<QString, QString>::const_iterator fi = filesToCheck.constBegin(); fi != filesToCheck.constEnd(); fi++)
		{
			QString fileName = FileDir() + "/" + fi.key();
			QFile file(fileName);
			if(file.open(QIODevice::ReadOnly))
			{
				QCryptographicHash fileHash(QCryptographicHash::Md5);
				QByteArray fileData = file.readAll();
				fileHash.addData(fileData);
				file.close();

				QByteArray result = fileHash.result();
				QString strResult;
				for(QByteArray::const_iterator bi = result.constBegin(); bi != result.constEnd(); bi++)
				{
					quint8 val = *bi;
					int highnibble = val / 16;
					int lownibble = val - highnibble * 16;

					if(highnibble < 10)
						strResult += '0' + highnibble;
					else
						strResult += 'a' + (highnibble - 10);

					if(lownibble < 10)
						strResult += '0' + lownibble;
					else
						strResult += 'a' + (lownibble - 10);
				}
				if(strResult != *fi)
				{
					//qDebug() << fileName << "md5" << *fi << "localFile" << strResult << "of" << fileData.count();
					filesToUpdate.insert(fi.key(), 1);			// Checksums don't match so this file needs updating.
				}
			}
			else
			{
				// File is missing locally.
				filesToUpdate.insert(fi.key(), 0);
			}
		}

		// List files that need updating.
		if(filesToCheck.isEmpty())
		{
			// Error parsing md5sums file.
			m_Files->hide();
			update();					// Position of copyright text changes.

			// No files are needed. Show the launcher buttons that are valid.
			m_ErrorMessage->show();
			ShowLauncherButtons();
			m_WebSite->show();
			//m_ErrorMessage->setText(QString("filesToCheck empty, arch %1").arg(Arch()));	//DHH

			m_CommState = 3;
		}
		else if(filesToUpdate.isEmpty())
		{
			//m_Files->addItem(tr("All files are up to date."));
			m_Files->hide();
			update();					// Position of copyright text changes.

			// On macOS, the resulting frameworks need symbolic links.
			FixSymbolicLinks();

			// No files are needed. Show the launcher buttons that are valid.
			ShowLauncherButtons();
			m_WebSite->show();

			m_CommState = 0;
		}
		else
		{
			// Files are needed. Hide the launcher buttons until we're done receiving them.
			HideLauncherButtons();
			m_WebSite->show();

			// Show the files list.
			m_Files->show();
			update();					// Position of copyright text changes.

			for(QMap<QString, int>::const_iterator fi = filesToUpdate.constBegin(); fi != filesToUpdate.constEnd(); fi++)
			{
				QString fileName(fi.key());
				int slash = fileName.indexOf('/');
				if(slash >= 0)
					fileName = fileName.mid(slash+1);

				QString fileStatus(fileName);
				if(*fi)
					fileStatus += tr("...changed.");
				else
					fileStatus += tr("...missing.");

				QUrl url(QString("http://Hazeron.com/software/%1/%2").arg(Arch()).arg(fi.key()));
				//qDebug() << url.path();

				QListWidgetItem* item = new QListWidgetItem(fileStatus);
				item->setData(Qt::UserRole, url);
				item->setData(Qt::UserRole+1, fileName);
				m_Files->addItem(item);
			}
			UpdateFiles();
		}
	}
	else if(m_CommState == 2)
	{
		// Receive a file.
		for(int row=0; row<m_Files->count(); row++)
		{
			QListWidgetItem* item = m_Files->item(row);
			QUrl url = item->data(Qt::UserRole).toUrl();
			if(url.path() == reply->url().path())
			{
				// Copy the file to its destination.
				QByteArray ba = reply->readAll();

				QString fileName = item->data(Qt::UserRole+1).toString();
				//qDebug() << " " << fileName << "received" << ba.count() << "bytes";

				// Remove the entry from our list of pending replies.
				m_NetReplies.remove(fileName);

				// Insure the folder structure exists.
#if !defined(Q_OS_WIN)
				bool inSubFolder = (fileName.indexOf('/') > 0) ? true : false;
#endif
				fileName = FileDir() + "/" + fileName;
				QFileInfo fileInfo(fileName);
				QDir dir = fileInfo.absoluteDir();
				if(dir.mkpath(dir.absolutePath()))
				{
					// Save the file.
					QFile flocal(fileName);
					if(flocal.open(QIODevice::WriteOnly))
					{
						flocal.write(ba);
						flocal.close();

#if !defined(Q_OS_WIN)
						// Insure we have execute permissions on all the files in the main folder.
						// Insure we have read/write permissions on all files in subfolders.
						// Note that on the Mac, all of our files are in subfolders. The executables are 'SoH', 'SoHCommander' and
						// 'SoHMail'.
						// So now we set the execute bit when we launch a program.
						if(!inSubFolder ||
								fileName.right(3) == "SoH" ||
								fileName.right(12) == "SoHCommander" ||
								fileName.right(7) == "SoHMail")
							flocal.setPermissions((QFile::Permission)0x7777);
						else
							flocal.setPermissions((QFile::Permission)0x6666);
#endif

						//item->setText(fileName + tr("...done."));
						m_Files->takeItem(row);
						delete item;
					}
					else
					{
						item->setText(fileName + tr("...FAIL! Downloaded OK but failed to overwrite local file."));
					}
				}
				else
				{
					item->setText(fileName + tr("...FAIL! Could not create folder %1.").arg(dir.absolutePath()));
				}
				break;
			}
		}
		if(m_Files->count() == 0)
		{
			m_CommState = 0;

			//m_Files->addItem(tr("All files are up to date."));
			m_Files->hide();
			update();					// Position of copyright text changes.

			// On macOS, the resulting frameworks need symbolic links.
			FixSymbolicLinks();

			// Done checking.
			ShowLauncherButtons();
		}
	}

	//delete reply;
}

void AuLauncherDialog::UpdateFiles()
{
	m_CommState = 2;					// Prepare to receive program files.
	for(int row=0; row<m_Files->count(); row++)
	{
		QListWidgetItem* item = m_Files->item(row);
		QString fileName = item->data(Qt::UserRole+1).toString();
		item->setText(fileName + tr("...downloading..."));

		QUrl url = item->data(Qt::UserRole).toUrl();
		AuLauncherNetworkReply* reply = new AuLauncherNetworkReply(url, m_Net->get(QNetworkRequest(url)));
		m_NetReplies.insert(fileName, reply);
		connect(reply, SIGNAL(DownloadProgress(QUrl, qint64, qint64)), SLOT(DownloadProgress(QUrl, qint64, qint64)));
	}
}

void AuLauncherDialog::UpdateStatus()
{
	if(m_Init && m_Init->isVisible())
		m_Init->setAutoRaise(!m_Init->autoRaise());

	if(m_BackdropTime.elapsed() > 10 * 1000)
	{
		// Load a new backdrop image.
		LoadRandomBackdrop();

		// Restart the backdrop image timer.
		m_BackdropTime.restart();
	}
}

void AuLauncherDialog::InitInstall()
{
	m_Init->hide();
	CheckForLauncherUpdates();
}

void AuLauncherDialog::LoadRandomBackdrop()
{
	// Load a random backdrop.
	// Check for screen shots in the Shores of Hazeron/Pictures folder.
	QString picPath = m_Settings.value("Settings/PictureDir").toString();
	if(picPath.isEmpty())
	{
		QString sohPath = m_Settings.value("Settings/BaseDir").toString();
		if(sohPath.isEmpty())
			sohPath = QDir::home().absoluteFilePath("Shores of Hazeron");
		picPath = sohPath + "/Pictures";
	}

	QDir screensDir(picPath);
	QStringList screenFilters;
	screenFilters << "*.bmp";
	screenFilters << "*.BMP";
	screenFilters << "*.jpg";
	screenFilters << "*.JPG";
	screenFilters << "*.png";
	screenFilters << "*.PNG";
	QStringList screenFiles = screensDir.entryList(screenFilters, QDir::Files|QDir::Readable, QDir::NoSort);
	QString backdropFile;
	if(screenFiles.isEmpty())
	{
		// No screen shots were found.
		AuDice dice;
		dice.TimeSeed();
		int backdropNum = dice.Roll(1, d_BackdropFiles);
		backdropFile = QString(":data/Backdrop%1.png").arg(backdropNum);
	}
	else
	{
		// Screen shots are available.
		AuDice dice;
		dice.TimeSeed();
		int backdropNum = dice.Roll(1, d_BackdropFiles + screenFiles.count());
		if(backdropNum <= d_BackdropFiles)
		{
			// A built-in backdrop image was selected.
			backdropFile = QString(":data/Backdrop%1.png").arg(backdropNum);
		}
		else
		{
			// A screen shot was selected.
			backdropNum -= (d_BackdropFiles+1);				// Reduce to an index into the screenFiles list.
			backdropFile = picPath + "/" + screenFiles[backdropNum];
		}
	}

	m_BackdropHistory.append(backdropFile);
	while(m_BackdropHistory.count() > 100)
		m_BackdropHistory.takeFirst();

	m_Backdrop = QImage(backdropFile);
	m_BackdropTime.restart();
	update();
}

void AuLauncherDialog::ShowLauncherButtons()
{
	if(CanLaunchCommander())
		m_LaunchCommander->show();
	if(CanLaunchMail())
		m_LaunchMail->show();
	if(CanLaunchSoH())
		m_LaunchSoH->show();
	//if(CanLaunchStoryEditor())
		//m_LaunchStoryEditor->show();
}

void AuLauncherDialog::HideLauncherButtons()
{
	m_LaunchCommander->hide();
	m_LaunchMail->hide();
	m_LaunchSoH->hide();
	//m_LaunchStoryEditor->hide();
}

void AuLauncherDialog::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_Timer)
	{
		m_Timer = new QTimer(this);
		connect(m_Timer, SIGNAL(timeout()), SLOT(UpdateStatus()));
	}

	// Activate the page update timer.
	if(!m_Timer->isActive())
	{
		m_Timer->start(500);

		// Reset the backdrop timer so it won't change immediately after we become visible.
		m_BackdropTime.start();
	}
}

void AuLauncherDialog::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be restarted during the next showEvent.
	if(m_Timer && m_Timer->isActive())
		m_Timer->stop();
}

void AuLauncherDialog::mouseDoubleClickEvent(QMouseEvent* e)
{
	QWidget::mouseDoubleClickEvent(e);

	// Ignore when we are showing the init install button.
	if(!m_Init || !m_Init->isVisible())
		ToggleVisibleButtons();
}

void AuLauncherDialog::ToggleVisibleButtons()
{
	if(m_CommState == 0)
	{
		if(m_LaunchSoH->isVisible())
		{
			HideLauncherButtons();
			m_SettingsButton->hide();
			m_WebSite->hide();
		}
		else
		{
			ShowLauncherButtons();
			m_SettingsButton->show();
			m_WebSite->show();
		}
	}
	else if(m_CommState == 2)
	{
		if(m_Files->isVisible())
		{
			m_Files->hide();
			m_SettingsButton->hide();
			m_WebSite->hide();
		}
		else
		{
			m_Files->show();
			m_SettingsButton->show();
			m_WebSite->show();
		}
		update();						// Position of copyright text changes.
	}
	else if(m_CommState == 3)
	{
		if(m_WebSite->isVisible())
		{
			HideLauncherButtons();
			m_ErrorMessage->hide();
			m_SettingsButton->hide();
			m_WebSite->hide();
		}
		else
		{
			ShowLauncherButtons();
			m_ErrorMessage->show();
			//m_ErrorMessage->setText(QString("comm state == 3, arch %1").arg(Arch()));	//DHH
			m_SettingsButton->show();
			m_WebSite->show();
		}
	}
}

void AuLauncherDialog::wheelEvent(QWheelEvent* e)
{
	QWidget::wheelEvent(e);

#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
	if(e->angleDelta().y() > 0)
		LoadNextBackdrop();
	else if(e->angleDelta().y() < 0)
		LoadPreviousBackdrop();
#else
	if(e->delta() > 0)
		LoadNextBackdrop();
	else if(e->delta() < 0)
		LoadPreviousBackdrop();
#endif
}

void AuLauncherDialog::LoadPreviousBackdrop()
{
	if(!m_BackdropHistory.isEmpty())
	{
		// Take the last image and rotate it to the beginning of the history list.
		QString last = m_BackdropHistory.takeLast();
		m_BackdropHistory.prepend(last);

		// Load the current image at the end of the history list.
		m_Backdrop = QImage(m_BackdropHistory.last());
		m_BackdropTime.restart();
		update();
	}
}

void AuLauncherDialog::LoadNextBackdrop()
{
	if(!m_BackdropHistory.isEmpty())
	{
		// Take the first image and rotate it to the end of the history list.
		QString first = m_BackdropHistory.takeFirst();
		m_BackdropHistory.append(first);

		// Load the current image at the end of the history list.
		m_Backdrop = QImage(m_BackdropHistory.last());
		m_BackdropTime.restart();
		update();
	}
}

void AuLauncherDialog::keyPressEvent(QKeyEvent *e)
{
	switch(e->key())
	{
	case Qt::Key_Escape:
		// Ignore when we are showing the init install button.
		if(!m_Init || !m_Init->isVisible())
			ToggleVisibleButtons();
		break;
	case Qt::Key_Down:
		if(isFullScreen())
			showNormal();
		else
			showFullScreen();
		break;
	case Qt::Key_Up:
		LoadRandomBackdrop();
		break;
	case Qt::Key_Left:
		LoadPreviousBackdrop();
		break;
	case Qt::Key_Right:
		LoadNextBackdrop();
		break;
	}

	QWidget::keyPressEvent(e);
}

void AuLauncherDialog::WebSite()
{
	QUrl url("https://www.hazeron.com");
	QDesktopServices::openUrl(url);
}

// On macOS, the resulting frameworks need symbolic links.
void AuLauncherDialog::FixSymbolicLinks()
{
#if defined(Q_OS_MACOS)
	FixSymbolicLink("SoH.app", "QtCore");
	FixSymbolicLink("SoH.app", "QtDBus");
	FixSymbolicLink("SoH.app", "QtGui");
	FixSymbolicLink("SoH.app", "QtNetwork");
	FixSymbolicLink("SoH.app", "QtOpenGL");
	FixSymbolicLink("SoH.app", "QtPrintSupport");
	FixSymbolicLink("SoH.app", "QtSql");
	FixSymbolicLink("SoH.app", "QtSvg");
	FixSymbolicLink("SoH.app", "QtWidgets");
	FixSymbolicLink("SoHMail.app", "QtCore");
	FixSymbolicLink("SoHMail.app", "QtDBus");
	FixSymbolicLink("SoHMail.app", "QtGui");
	FixSymbolicLink("SoHMail.app", "QtNetwork");
	FixSymbolicLink("SoHMail.app", "QtOpenGL");
	FixSymbolicLink("SoHMail.app", "QtPrintSupport");
	FixSymbolicLink("SoHMail.app", "QtSql");
	FixSymbolicLink("SoHMail.app", "QtSvg");
	FixSymbolicLink("SoHMail.app", "QtWidgets");
#endif
}

void AuLauncherDialog::FixSymbolicLink(const QString& app, const QString& qtLib)
{
	QString file = QString("%1/%2/Contents/Frameworks/%3.framework/Versions/5/%3").arg(FileDir()).arg(app).arg(qtLib);
	if(QFile::exists(file))
	{
		QString verLink = QString("%1/%2/Contents/Frameworks/%3.framework/Versions/Current").arg(FileDir()).arg(app).arg(qtLib);
		QString verFile = QString("%1/%2/Contents/Frameworks/%3.framework/Versions/5").arg(FileDir()).arg(app).arg(qtLib);
		QFile::link(verFile, verLink);

		QString libLink = QString("%1/%2/Contents/Frameworks/%3.framework/%3").arg(FileDir()).arg(app).arg(qtLib);
		QString libFile = QString("%1/%2/Contents/Frameworks/%3.framework/Versions/Current/%3").arg(FileDir()).arg(app).arg(qtLib);
		QFile::link(libFile, libLink);

		QString resLink = QString("%1/%2/Contents/Frameworks/%3.framework/Resources").arg(FileDir()).arg(app).arg(qtLib);
		QString resFile = QString("%1/%2/Contents/Frameworks/%3.framework/Versions/Current/Resources")
			.arg(FileDir()).arg(app).arg(qtLib);
		QFile::link(resFile, resLink);
	}
}

void AuLauncherDialog::Settings()
{
	int glVer = m_Settings.value("Settings/OpenGL").toInt();
#if defined(Q_OS_MACOS)
	if(glVer != 41 && glVer != 33)
		glVer = 41;
#else
	glVer = 32;
#endif

	QString sohPath = m_Settings.value("Settings/BaseDir").toString();
	if(sohPath.isEmpty())
		sohPath = QDir::home().absoluteFilePath("Shores of Hazeron");

	QString picPath = m_Settings.value("Settings/PictureDir").toString();
	if(picPath.isEmpty())
		picPath = sohPath + "/Pictures";

	QString cachePath = m_Settings.value("Settings/CacheDir").toString();
	if(cachePath.isEmpty())
		cachePath = sohPath + "/Temporary";

	AuLauncherSettings set(sohPath, picPath, cachePath, glVer, (float)d_MyVersion, Arch(), this);
	if(set.exec() == QDialog::Accepted)
	{
		sohPath = set.SoHFolder();
		picPath = set.PictureFolder();
		cachePath = set.CacheFolder();
		int nuglVer = set.OpenGLVersion();

		m_Settings.setValue("Settings/BaseDir", sohPath);
		m_Settings.setValue("Settings/PictureDir", picPath);
		m_Settings.setValue("Settings/CacheDir", cachePath);
		m_Settings.setValue("Settings/OpenGL", nuglVer);

		// When no initial download button exists, recheck for updates now.
		// Also recheck for updates if the initial download button is not visible, which means the download was already started.
		if(glVer != nuglVer && (!m_Init || !m_Init->isVisible()))
		{
			QMessageBox::warning(this, tr("Launcher Restart Required"),
					tr("<html>The <i>Shores of Hazeron Launcher</i> should be restarted. "
						"Files appropriate to the selected OpenGL version will be updated."
						"</html>"));
		}
	}
}

void AuLauncherDialog::DownloadProgress(QUrl url, qint64 bytesReceived, qint64 bytesTotal)
{
	if(!m_Files)
		return;

	for(int row=0; row<m_Files->count(); row++)
	{
		QListWidgetItem* item = m_Files->item(row);
		if(item->data(Qt::UserRole).toUrl() != url)
			continue;
		QString fileName = item->data(Qt::UserRole+1).toString();
		QMap<QString, AuLauncherNetworkReply*>::const_iterator ri = m_NetReplies.constFind(fileName);
		if(ri == m_NetReplies.constEnd())
			break;

		if(bytesReceived != bytesTotal)
		{
			QString itemText = fileName + tr("...downloading...%L1 of %L2...").arg(bytesReceived).arg(bytesTotal);
			item->setText(itemText);
		}
		else
			item->setText(fileName + tr("...done."));
		break;
	}
}

QString AuLauncherDialog::Arch() const
{
	QString arch(m_Arch);
#if defined(Q_OS_MACOS)
	if(!arch.isEmpty())
	{
		// Change to -gl41 version based on settings.
		int glVer = m_Settings.value("Settings/OpenGL").toInt();
		if(glVer == 41)
			arch.replace("-64", "-gl41-64");
	}
#endif
	return arch;
}

void AuLauncherDialog::NetSSLErrors(QNetworkReply* reply, const QList<QSslError>& errors)
{
	Q_UNUSED(reply);
	QString errMsg = tr("<html>The <i>Shores of Hazeron Launcher</i> received the following SSL error(s). ");
	for(QList<QSslError>::const_iterator ei = errors.cbegin(); ei != errors.cend(); ei++)
		errMsg += "<br> - " + (*ei).errorString();
	errMsg += "</html>";
	QMessageBox::critical(this, tr("Launcher SSL Errors"), errMsg);
}

