//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auintro.cpp,v 1.28 2022/04/12 17:06:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auintro.h"
//
#include "auapp.h"
#include "aulogincomm.h"
#include "AuMsgDialog.h"
#include "aupicturewidget.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedLayout>
#include <QToolButton>

AuIntro::AuIntro(QWidget* parent) : AuIntroBase(parent)
{
	SetSplashImage(QImage(":data/HazCommSplash.png"));

	m_HasInfo = false;
	m_msgDialog = 0;
	m_WaitTime.start();

	m_Pages = new QStackedLayout();

	// Login page
	m_LoginPage = new AuIntroLogin(this);
	connect(m_LoginPage, SIGNAL(LoginOk()), SLOT(LoggedIn()));

	// Avatar selection page
	m_ChooseAvatarPage = new AuIntroChooseAvatarNoRender(this);
	connect(m_ChooseAvatarPage, SIGNAL(AvatarLogin(AuId)), SLOT(ExistingAvatarLogin(AuId)));

	m_Pages->addWidget(m_LoginPage);
	m_Pages->addWidget(m_ChooseAvatarPage);

	QVBoxLayout* lay = new QVBoxLayout();
	lay->addLayout(m_Pages);
	lay->setMargin(0);
	lay->setSpacing(2);
	setLayout(lay);

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), SLOT(TimerUpdate()));
	timer->start(66);
}

AuIntro::~AuIntro()
{
}

void AuIntro::LoggedIn()
{
	ShowWaitInfoMsg();
	emit LoginOk();
}

void AuIntro::SetAccountInfo()
{
	if(m_HasInfo)
		return;
	m_HasInfo = true;

	if(m_msgDialog)
		m_msgDialog->hide();

	// Make the avatar selection page current.
	m_ChooseAvatarPage->SetAccountInfo();
	m_Pages->setCurrentIndex(1);
}

void AuIntro::ShowWaitInfoMsg()
{
	if(!m_msgDialog)
	{
		m_msgDialog = new AuMsgDialog(this);
		m_msgDialog->SetMessage(0, tr("Getting Game Data"));
		m_msgDialog->show();
	}
}

void AuIntro::paintEvent(QPaintEvent* pe)
{
	if(m_msgDialog)
	{
		int iWaitTime = m_WaitTime.isValid() ? m_WaitTime.elapsed() : -1;
		if(iWaitTime < 0 || iWaitTime > 200)
		{
			m_WaitTime.restart();
			m_msgDialog->StepNext();
		}
	}
	AuIntroBase::paintEvent(pe);
}

void AuIntro::TimerUpdate()
{
	if(!isVisible())
	{
		// Insure the little message dialog does not remain visible.
		if(m_msgDialog)
			m_msgDialog->hide();
		return;
	}
	update();
}

void AuIntro::ExistingAvatarLogin(AuId avatarId)
{
	if(m_msgDialog)
	{
		m_msgDialog->SetMessage(1, tr("Entering Hazeron"));
		m_msgDialog->show();
	}
	emit AvatarLogin(avatarId);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroLogin::AuIntroLogin(AuIntro* parent) : QWidget(parent)
{
	QString accountName = g_Au.Settings().readEntry("/Account/Name","");
	int savePassword = g_Au.Settings().readNumEntry("/Account/SavePass", 0);
	QString password;
	if(savePassword)
		g_Au.Settings().readEntry("/Account/Password", m_PassCrypt);

	m_AccountName = new QLineEdit(accountName, this);
	m_AccountName->setToolTip(QObject::tr("<html><b>Account Name</b><br>"
				"Enter the name of the login account you created on the <i>%1</i> web site.</html>").arg(g_Au.ApplicationName()));

	m_Password = new QLineEdit(this);
	m_Password->setEchoMode(QLineEdit::Password);
	m_Password->setToolTip(QObject::tr("<html><b>Account Password</b><br>"
				"Enter the password for the login account you created on the <i>%1</i> web site.</html>").arg(g_Au.ApplicationName()));
	if(!m_PassCrypt.isEmpty())
		m_Password->setText("*");	// Put something into the password box to indicate it was restored.
	connect(m_Password, SIGNAL(textEdited(const QString&)), SLOT(PasswordEdited(const QString&)));

	m_SavePassword = new QCheckBox(tr("Save Password"), this);
	m_SavePassword->setToolTip(QObject::tr("<html><b>Save Password</b><br>"
				"Chooses to remember or forget your password when the <b>Login</b> button is pressed.<br>"
				"%1 Check this control to store your password for fast login next time.<br>"
				"%1 Uncheck this control to erase your password from local storage."
				"</html>").arg(g_Au.MiddleDotSymbol()));
	if(savePassword)
		m_SavePassword->setChecked(true);

	m_Login = new QPushButton(tr("Login"), this);
	m_Login->setAutoDefault(true);
	m_Login->setDefault(true);
	m_Login->setToolTip(QObject::tr("<html><b>Login</b><br>"
				"Requests access to the <i>%1</i> game by sending the name and password to the server.</html>").arg(g_Au.ApplicationName()));
	connect(m_Login, SIGNAL(clicked()), SLOT(Login()));

	m_HazeronButton = new QToolButton(this);
	m_HazeronButton->setAutoRaise(true);
	m_HazeronButton->setIcon(QIcon(":data/SoHLogo32.png"));
	m_HazeronButton->setToolTip(QObject::tr("<html><b>About <i>%1</i>...</b><br>"
				"Shows information about the <i>%1</i> program."
				"</html>").arg(g_Au.ApplicationName()));
	connect(m_HazeronButton, SIGNAL(clicked()), parent, SIGNAL(HelpAbout()));

	m_FullScreenButton = new QToolButton(this);
	m_FullScreenButton->setAutoRaise(true);
	m_FullScreenButton->setIcon(QIcon(":data/ToggleFull.png"));
	m_FullScreenButton->setToolTip(QObject::tr("<html><b>Toggle Full Screen Mode</b> (Alt+Enter)<br>"
				"Switches between <i>full screen</i> and <i>window</i> modes."
				"</html>"));
	connect(m_FullScreenButton, SIGNAL(clicked()), parent, SIGNAL(ToggleFullScreen()));

	m_SettingsButton = new QToolButton(this);
	m_SettingsButton->setAutoRaise(true);
	m_SettingsButton->setIcon(QIcon(":data/Settings.png"));
	m_SettingsButton->setToolTip(QObject::tr("<html><b>Settings</b><br>"
				"Shows the <i>Settings</i> window to change settings that affect the <i>%1</i> program."
				"</html>").arg(g_Au.ApplicationName()));
	connect(m_SettingsButton, SIGNAL(clicked()), parent, SIGNAL(ToggleSettingsUI()));

	QToolButton* exitButton = new QToolButton(this);
	exitButton->setAutoRaise(true);
	exitButton->setIcon(QIcon(":data/ExitGame.png"));
	exitButton->setToolTip(QObject::tr("<b>Exit Game</b> (Alt+F4)<br>"
				"Exits the <i>%1</i> game.").arg(g_Au.ApplicationName()));
	connect(exitButton, SIGNAL(clicked()), parent, SIGNAL(ExitGame()));

	QHBoxLayout* play = new QHBoxLayout;
	play->addWidget(m_Password);
	play->addWidget(m_SavePassword);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(m_Login);
	blay->addWidget(m_HazeronButton);
	blay->addWidget(m_FullScreenButton);
	blay->addWidget(m_SettingsButton);
	blay->addWidget(exitButton);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* glay = new QVBoxLayout;
	glay->addWidget(m_AccountName);
	glay->addLayout(play);
	glay->addLayout(blay);
	QGroupBox* group = new QGroupBox(tr("Login"));
	group->setLayout(glay);

	// Build the final layout.
	QGridLayout* lay = new QGridLayout;
	int row=1;
	lay->addWidget(group, row,1);
	row++;
	lay->setRowStretch(0, 1);
	lay->setRowStretch(row, 1);
	lay->setColumnStretch(0, 1);
	lay->setColumnStretch(2, 1);
	setLayout(lay);
}

AuIntroLogin::~AuIntroLogin()
{
}

void AuIntroLogin::PasswordEdited(const QString& password)
{
	// Put password into a byte array.
	QByteArray passBytes;
	passBytes.append(password.toUtf8());

	// Create a SHA-1 hash from the password.
	QCryptographicHash passHash(QCryptographicHash::Sha1);
	passHash.addData(passBytes);
	m_PassCrypt = passHash.result();
}

void AuIntroLogin::Login()
{
	QString acctName = m_AccountName->text();
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("Intro: Login '%1' passcrypt %2").arg(acctName).arg(m_PassCrypt.isEmpty()));
	if(acctName.isEmpty())
		return;	// Blank name is not allowed.

	if(m_PassCrypt.isEmpty())
		return;

	bool savePass = false;
	if(m_SavePassword && m_SavePassword->checkState() == Qt::Checked)
		savePass = true;

	// Save last attempted name.
	g_Au.Settings().writeEntry("/Account/Name", acctName);
	g_Au.Settings().writeEntry("/Account/SavePass", savePass?1:0);
	if(savePass)
		g_Au.Settings().writeEntry("/Account/Password", m_PassCrypt);
	else
	{
		QByteArray emptyArray;
		g_Au.Settings().writeEntry("/Account/Password", emptyArray);
	}

	// Disable controls while waiting to login.
	m_AccountName->setEnabled(false);
	m_Password->setEnabled(false);
	m_SavePassword->setEnabled(false);
	m_Login->setEnabled(false);

	AuLoginComm logincomm;
	int result = logincomm.Login(acctName, m_PassCrypt);

	// Enable controls now that we have the login result.
	m_AccountName->setEnabled(true);
	m_Password->setEnabled(true);
	m_SavePassword->setEnabled(true);
	switch(result)
	{
	case 0:	// Login ok
		{
			// We store the name and password locally for use in db lookups (contacts)
			DgData& gd = g_Au.LockDataForWrite();
			gd.SetAcctName(acctName);
			gd.SetAcctPassCrypt(m_PassCrypt);
			g_Au.UnlockData();
			emit LoginOk();
		}
		break;

	case 1:	// Error.
		// Servers down?
		{
			m_Login->setEnabled(true);
			QString msg = tr("<html><b>Login Error</b><p>"
					"An error occurred while attempting to log in.<p>"
					"Tips:<br>"
					"%1 Check your network connection.<br>"
					"%1 Check server status on our web site.<br>"
					"%1 Run the <i>Shores of Hazeron Launcher</i> to update your files.<br>"
					"%1 If that doesn't work, "
					"download and install the latest <i>Shores of Hazeron Launcher</i> and try again."
					"</html>")
				.arg(g_Au.MiddleDotSymbol());
			QMessageBox mb(QMessageBox::NoIcon, g_Au.ApplicationName(), msg, QMessageBox::Cancel, this);
			mb.setIconPixmap(QPixmap(":data/SoHLogo32.png"));
			QAbstractButton* openStatus = mb.addButton(tr("Open Server Status Web Page"),
					QMessageBox::ActionRole);
			QAbstractButton* openDownloads = mb.addButton(tr("Open Downloads Web Page"),
					QMessageBox::ActionRole);
			mb.exec();
			if(mb.clickedButton() == openStatus)
			{
				QUrl url("https://www.hazeron.com/status.php");
				QDesktopServices::openUrl(url);
			}
			else if(mb.clickedButton() == openDownloads)
			{
				QUrl url("https://www.hazeron.com/downloads.php");
				QDesktopServices::openUrl(url);
			}
		}
		break;

	case 2:	// Login failure probably due to bad password or disabled login account.
		{
			m_Login->setEnabled(true);
			QString msg = tr("<html><b>Login Failure</b><p>"
					"Login attempt failed. Please try again.<p>"
					"Tips:<br>"
					"%1 Double check your login account name and password.<br>"
					"%1 They are case sensitive; check <b>Caps Lock</b>.<br>"
					"%1 Your login account name is usually not the same as your avatar's screen name."
					"</html>")
				.arg(g_Au.MiddleDotSymbol());
			QMessageBox mb(QMessageBox::NoIcon, g_Au.ApplicationName(), msg, QMessageBox::Cancel, this);
			mb.setIconPixmap(QPixmap(":data/SoHLogo32.png"));
			mb.exec();
		}
		break;

	case 3:	// Incorrect software version.
		{
			QString msg = tr("<html><b>Version Mismatch</b><p>"
					"This program's version is incompatible with the server.<p>"
					"Tips:<br>"
					"%1 Run the <i>Shores of Hazeron Launcher</i> to update your files.<br>"
					"%1 If that doesn't work, "
					"download and install the latest <i>Shores of Hazeron Launcher</i> and try again."
					"</html>")
				.arg(g_Au.MiddleDotSymbol());
			QMessageBox mb(QMessageBox::NoIcon, g_Au.ApplicationName(), msg, QMessageBox::Cancel, this);
			mb.setIconPixmap(QPixmap(":data/SoHLogo32.png"));
			QAbstractButton* openDownloads = mb.addButton(tr("Open Downloads Web Page"),
					QMessageBox::ActionRole);
			mb.exec();
			if(mb.clickedButton() == openDownloads)
			{
				// Thanks Qt for making this one so easy.
				QUrl url("https://www.hazeron.com/downloads.php");
				QDesktopServices::openUrl(url);
			}
		}
		break;

	case 6:	// Account locked
		{
			m_Login->setEnabled(true);
			QString msg = tr("<html><b>Account Locked</b><p>"
					"The account is locked. "
					"Contact Hazeron Freehold to unlock the account. "
					"</html>");
			QMessageBox mb(QMessageBox::NoIcon, g_Au.ApplicationName(), msg, QMessageBox::Cancel, this);
			mb.setIconPixmap(QPixmap(":data/SoHLogo32.png"));
			QAbstractButton* openLink = mb.addButton(tr("Open Hazeron Freehold Web Page"), QMessageBox::ActionRole);
			mb.exec();
			if(mb.clickedButton() == openLink)
			{
				// Thanks Qt for making this one so easy.
				QUrl url("https://Hazeron.com/");
				QDesktopServices::openUrl(url);
			}
		}
		break;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroChooseAvatarNoRender::AuIntroChooseAvatarNoRender(AuIntro* parent) : QWidget(parent)
{
	m_AvatarSampleRendered = 0;

	QVBoxLayout* alay = new QVBoxLayout;
	for(int av=0; av<6; av++)
	{
		m_Avatar[av] = new AuIntroAvatarWidget(false, av, this);
		alay->addWidget(m_Avatar[av]);
		connect(m_Avatar[av], SIGNAL(Selected(int)), SLOT(AvatarSelected(int)));
		connect(m_Avatar[av], SIGNAL(EnterHazeron(AuId)), SIGNAL(AvatarLogin(AuId)));
		connect(m_Avatar[av], SIGNAL(Deleted(AuId)), SLOT(AvatarDeleted(AuId)));
	}

	QToolButton* exitButton = new QToolButton(this);
	exitButton->setAutoRaise(true);
	exitButton->setIcon(QIcon(":data/ExitGame.png"));
	exitButton->setIconSize(QSize(32,32));
	exitButton->setToolTip(QObject::tr("<b>Exit Game</b> (Alt+F4)<br>"
				"Exits the <i>%1</i> game.").arg(g_Au.ApplicationName()));
	connect(exitButton, SIGNAL(clicked()), parent, SIGNAL(ExitGame()));

	alay->addStretch(1);
	alay->setSpacing(2);
	alay->setMargin(2);

	QVBoxLayout* xlay = new QVBoxLayout;
	xlay->addStretch(1);
	xlay->addWidget(exitButton);
	xlay->setSpacing(2);
	xlay->setMargin(2);

	m_NewAvatarImage = new AuPictureWidget(true, this);
	m_NewAvatarImage->SetImage(QImage(":data/ButtonJoinGS.png"));

	QHBoxLayout* glay = new QHBoxLayout;
	glay->addLayout(alay, 1);
	glay->addWidget(m_NewAvatarImage, 1);
	glay->addLayout(xlay);

	QGroupBox* group = new QGroupBox(tr("Avatar"));
	group->setLayout(glay);

	// Build the final layout.
	QGridLayout* lay = new QGridLayout;
	int row=1;
	lay->addWidget(group, row,1, 2,1);
	row++;
	lay->setRowStretch(0, 1);
	lay->setRowStretch(row, 3);
	lay->setColumnStretch(0, 1);
	lay->setColumnStretch(1, 3);
	setLayout(lay);
}

AuIntroChooseAvatarNoRender::~AuIntroChooseAvatarNoRender()
{
}

void AuIntroChooseAvatarNoRender::SetAccountInfo()
{
	const DgData& gd = g_Au.LockDataForRead();
	for(int unitIndex=0; unitIndex<6; unitIndex++)
	{
		AuId unitId = gd.ClientUnitId(unitIndex);
		QString unitName = gd.ClientUnitName(unitId);
		const AuDNABase* dna = gd.ClientUnitDNA(unitId);

		if(unitId.IsNull() || unitName.isEmpty() || !dna)
			m_Avatar[unitIndex]->Clear();
		else
			m_Avatar[unitIndex]->SetAvatar(unitId, unitName, *dna, gd.ClientUnitChromatid(unitId), gd.ClientUnitHand(unitId));
		m_Avatar[unitIndex]->setVisible(true);
	}
	g_Au.UnlockData();

	// Default to last used account, otherwise use first account
	bool didSel = false;				// Set true if we actually call AvatarSelected.
	AuId idLastPlayer(g_Au.Settings().readEntry("/Account/LastPlayerId"));
	if(!idLastPlayer.IsNull())
	{
		for(int iu=0; iu<6; iu++)
		{
			if(m_Avatar[iu]->Id() == idLastPlayer)
			{
				AvatarSelected(iu);
				didSel = true;
				break;
			}
		}
	}
	if(!didSel)
		AvatarSelected(0);				// Select the first avatar slot.
}

void AuIntroChooseAvatarNoRender::AvatarSelected(int index)
{
	// Get Player info from unit id if necessary
	if(!m_Avatar[index]->Id().IsNull())
	{
		// Set DNA and invalidate GL model of body.
		m_NewAvatarImage->show();

		// Load unit values from local settings, based on their string id.
		QString unitStringId = m_Avatar[index]->Id().StringId();

		// Load last known location.
		QString section = QString("/Whereabouts/%1").arg(unitStringId);
		QString prevLoc = g_Au.Settings().readEntry(section, "");
		for(int av=0; av<6; av++)
			m_Avatar[av]->SetWhereabouts(prevLoc);
	}

	// Update unit selection buttons.
	for(int unitButtonIndex=0; unitButtonIndex<6; unitButtonIndex++)
		m_Avatar[unitButtonIndex]->SetCurrent(index == unitButtonIndex);
}

void AuIntroChooseAvatarNoRender::AvatarDeleted(AuId avatarId)
{
	Q_UNUSED(avatarId);
	m_NewAvatarImage->show();
}

