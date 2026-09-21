//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auintro.cpp,v 1.50 2022/04/12 18:37:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auintro.h"
//
#include "auapp.h"
#include "aucrypt.h"
#include "auglobal.h"
#include "aulogincomm.h"
#include "AuMsgDialog.h"
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
	SetSplashImage(QImage(":data/Splash.png"));
	//SetSplashMovie(QUrl("ShieldHits.mp4"));

	m_HasInfo = false;
	m_msgDialog = 0;
#if defined(INTRO_SOLO_TOO)
	m_Solo = false;
#endif
	m_WaitTime.start();

	m_Pages = new QStackedLayout();

	// Launch page
	m_LaunchPage = new AuIntroLaunch(this);
	connect(m_LaunchPage, SIGNAL(LaunchDesigner()), SIGNAL(LaunchDesigner()));
	connect(m_LaunchPage, SIGNAL(LaunchLogin()), SLOT(LaunchLogin()));
#if defined(INTRO_SOLO_TOO)
	connect(m_LaunchPage, SIGNAL(LaunchSolitaire()), SLOT(LaunchSolo()));
#endif

	// Login page
	m_LoginPage = new AuIntroLogin(this);
	connect(m_LoginPage, SIGNAL(LoginOk()), SLOT(LoggedIn()));
	connect(m_LoginPage, SIGNAL(Back()), SLOT(LoginBack()));

	// Avatar selection page
	m_ChooseAvatarPage = new AuIntroChooseAvatar(this);
	connect(m_ChooseAvatarPage, SIGNAL(NewAvatarDNA3(int)), SLOT(NewAvatarDNA3(int)));
	connect(m_ChooseAvatarPage, SIGNAL(NewAvatarDNA4(int)), SLOT(NewAvatarDNA4(int)));
	connect(m_ChooseAvatarPage, SIGNAL(AvatarLogin(AuId)), SLOT(ExistingAvatarLogin(AuId)));

#if defined(INTRO_SOLO_TOO)
	// Solo avatar selection page
	m_SoloAvatarPage = new AuIntroSoloAvatar(this);
	connect(m_SoloAvatarPage, SIGNAL(NewAvatarDNA3(int)), SLOT(NewAvatarDNA3(int)));
	connect(m_SoloAvatarPage, SIGNAL(NewAvatarDNA4(int)), SLOT(NewAvatarDNA4(int)));
	connect(m_SoloAvatarPage, SIGNAL(AvatarLogin(AuId)), SLOT(ExistingAvatarLogin(AuId)));
#endif

	// Avatar creation page
	m_CreateAvatarPage = new AuIntroCreateAvatar(this);
	connect(m_CreateAvatarPage, SIGNAL(Back()), SLOT(NewAvatarBack()));
	connect(m_CreateAvatarPage, SIGNAL(NewAvatarLogin(int)), SLOT(NewAvatarLogin(int)));

	// Avatar creation page
	m_CreateAvatar4Page = new AuIntroCreateAvatar4(this);
	connect(m_CreateAvatar4Page, SIGNAL(Back()), SLOT(NewAvatarBack()));
	connect(m_CreateAvatar4Page, SIGNAL(NewAvatarLogin(int)), SLOT(NewAvatarLogin(int)));

	int idx = 0;
	m_Pages->addWidget(m_LaunchPage);
	m_LaunchIdx = idx++;
	m_Pages->addWidget(m_LoginPage);
	m_LoginIdx = idx++;
	m_Pages->addWidget(m_ChooseAvatarPage);
	m_ChooseAvatarIdx = idx++;
#if defined(INTRO_SOLO_TOO)
	m_Pages->addWidget(m_SoloAvatarPage);
	m_SoloAvatarIdx = idx++;
#endif
	m_Pages->addWidget(m_CreateAvatarPage);
	m_CreateAvatarIdx = idx++;
	m_Pages->addWidget(m_CreateAvatar4Page);
	m_CreateAvatar4Idx = idx++;

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
	m_Pages->setCurrentIndex(m_ChooseAvatarIdx);
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
	if(m_Pages->currentIndex() == m_ChooseAvatarIdx)
		m_ChooseAvatarPage->UpdateGL();
	else if(m_Pages->currentIndex() == m_CreateAvatarIdx)
		m_CreateAvatarPage->UpdateGL();
	else if(m_Pages->currentIndex() == m_CreateAvatar4Idx)
		m_CreateAvatar4Page->UpdateGL();
#if defined(INTRO_SOLO_TOO)
	else if(m_Pages->currentIndex() == m_SoloAvatarIdx)
		m_SoloAvatarPage->UpdateGL();
#endif
}

void AuIntro::LoginBack()
{
	m_Pages->setCurrentIndex(m_LaunchIdx);
}

void AuIntro::NewAvatarBack()
{
#if defined(INTRO_SOLO_TOO)
	if(m_Solo)
		m_Pages->setCurrentIndex(m_SoloAvatarIdx);
	else
#endif
		m_Pages->setCurrentIndex(m_ChooseAvatarIdx);
}

void AuIntro::NewAvatarDNA3(int index)
{
	m_Pages->setCurrentIndex(m_CreateAvatarIdx);
	m_CreateAvatarPage->SetAvatarIndex(index);
}

void AuIntro::NewAvatarDNA4(int index)
{
	m_Pages->setCurrentIndex(m_CreateAvatar4Idx);
	m_CreateAvatar4Page->SetAvatarIndex(index);
}

void AuIntro::NewAvatarLogin(int index)
{
	Q_UNUSED(index);
#if defined(INTRO_SOLO_TOO)
	if(m_Solo)
	{
		// Create the new solitaire mode universe.
		EnterNewAvatar();
		const DgData& gd = g_Au.LockDataForRead();
        AuId idUnit = gd.ClientUnitId();    // Id of unit the player will be controlling.
        g_Au.UnlockData();
		emit AvatarSolo(idUnit);
	}
	else
#endif
		emit AvatarLogin(AuId(0));
}

void AuIntro::ExistingAvatarLogin(AuId avatarId)
{
	if(m_msgDialog)
	{
		m_msgDialog->SetMessage(1, tr("Entering Hazeron"));
		m_msgDialog->show();
	}
#if defined(INTRO_SOLO_TOO)
	if(m_Solo)
		emit AvatarSolo(avatarId);
	else
#endif
		emit AvatarLogin(avatarId);
}

void AuIntro::EnterNewAvatar()
{
	if(m_msgDialog)
	{
		m_msgDialog->SetMessage(1, tr("Entering Hazeron"));
		m_msgDialog->show();
	}
	if(m_Pages->currentIndex() == m_CreateAvatarIdx)
		m_CreateAvatarPage->EnterNewAvatar();
	else
		m_CreateAvatar4Page->EnterNewAvatar();
}

void AuIntro::EmpireDataReceived()
{
	emit m_CreateAvatarPage->EmpireDataReceived();
	emit m_CreateAvatar4Page->EmpireDataReceived();
}

void AuIntro::LaunchLogin()
{
#if defined(INTRO_SOLO_TOO)
	m_Solo = false;
#endif
	m_CreateAvatarPage->SetSolo(false);
	m_CreateAvatar4Page->SetSolo(false);
	m_Pages->setCurrentIndex(m_LoginIdx);
}

#if defined(INTRO_SOLO_TOO)
void AuIntro::LaunchSolo()
{
	m_Solo = true;
	g_Au.SetMode(AuGlobal::SceneSolo);	// So atoms will know we're solo when loading the ether from a file.
	m_CreateAvatarPage->SetSolo(true);
	m_CreateAvatar4Page->SetSolo(true);
	m_SoloAvatarPage->SetAccountInfo();
	m_Pages->setCurrentIndex(m_SoloAvatarIdx);
}
#endif

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroLaunch::AuIntroLaunch(AuIntro* parent) : QWidget(parent)
{
	QString affirmText = tr("<html>"
			"<p>"
			"<i>%1</i> presents a comprehensive simulation of life, including mating. "
			"<u>The theme is not pornography</u> but this game can create scenes with strong sexual content. "
			"</p>"
			"<p>"
			"<i>%1</i> is intended for a mature audience. "
			"Communications between players are not censored. "
			"Nudity and adult situations are graphically depicted. "
			"</p>"
			"<p>"
			"<b>You must be 17 years of age or older to run this software.</b> "
			"</p>"
			"</html>").arg(g_Au.ApplicationName());
	QDataStream ds(&m_Affirm, QIODevice::WriteOnly);
	ds.setVersion(QDataStream::Qt_5_7);
	ds << affirmText;
	m_Affirm.Crypt(0x5592);

	// The user must affirm that they are 17 years or older.
	bool mustAffirm17 = true;
	QString affirm17 = g_Au.Settings().readEntry("/Launch/Affirm17", "");
	if(affirm17 != "")
	{
		// An affirm date has been stored.
		// Make them reaffirm if the affirm date looks bogus or if the affirmation text changes.
		QDateTime when = QDateTime::fromString(affirm17);
		QDateTime now = QDateTime::currentDateTimeUtc();
		//qDebug() << "whenAffirmed" << when << "now" << now << "days" << when.daysTo(now);
		if(!when.isNull() && when < now)
		{
			// We have a valid looking affirmation date.
			// Check to see if the affirmation text has changed.
			QByteArray affirmedWhat;
			g_Au.Settings().readEntry("/Launch/Affirm", affirmedWhat);
			if(affirmedWhat == m_Affirm)
				mustAffirm17 = false;		// Affirmation text did not change.
		}
	}

	m_Mature17Icon = new QLabel(this);
	m_Mature17Icon->setPixmap(QPixmap(":data/Mature17Plus.png"));
	m_Mature17Icon->setVisible(mustAffirm17);

	m_Mature17Label = new QLabel(affirmText, this);
	m_Mature17Label->setWordWrap(true);
	m_Mature17Label->setAlignment(Qt::AlignCenter);
	m_Mature17Label->setVisible(mustAffirm17);

	m_Mature17Check = new QCheckBox(tr("I am at least 17 years of age."), this);
	m_Mature17Check->setVisible(mustAffirm17);
	connect(m_Mature17Check, SIGNAL(clicked()), SLOT(Mature17Clicked()));

	m_LoginButton = new QToolButton(this);
	m_LoginButton->setIcon(QIcon(QPixmap(":data/HazeronStart.png").scaled(256,144,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
	m_LoginButton->setIconSize(QSize(256,144));
	m_LoginButton->setToolTip(tr("<html><b>Play <i>%1</i>...</b><br>"
				"Plays <i>%1</i>, the MMO sandbox universe."
				"</html>").arg(g_Au.ApplicationName()));
	m_LoginButton->setFocusPolicy(Qt::NoFocus);
	m_LoginButton->setVisible(!mustAffirm17);
	connect(m_LoginButton, SIGNAL(clicked()), SIGNAL(LaunchLogin()));

	m_DesignerButton = new QToolButton(this);
	m_DesignerButton->setIcon(QIcon(":data/DesignerStart.png"));
	m_DesignerButton->setIconSize(QSize(128,72));
	m_DesignerButton->setToolTip(tr("<html><b>Design Studio...</b><br>"
				"Starts the <i>%1</i> design studio in solitaire architect mode. "
				"</html>").arg(g_Au.ApplicationName()));
	m_DesignerButton->setFocusPolicy(Qt::NoFocus);
	m_DesignerButton->setVisible(!mustAffirm17);
	connect(m_DesignerButton, SIGNAL(clicked()), SIGNAL(LaunchDesigner()));

#if defined(INTRO_SOLO_TOO)
	m_SoloButton = new QToolButton(this);
	m_SoloButton->setIcon(QIcon(":data/HSSSmallCapsule.png"));
	m_SoloButton->setIconSize(QSize(231,87));
	m_SoloButton->setToolTip(QObject::tr("<html><b>Play <i>Hazeron Starship</i>...</b><br>"
				"Plays <i>Hazeron Starship</i>, the sandbox universe. "
				"</html>"));
	m_SoloButton->setFocusPolicy(Qt::NoFocus);
	m_SoloButton->setVisible(!mustAffirm17);
	connect(m_SoloButton, SIGNAL(clicked()), SIGNAL(LaunchSolitaire()));
#endif

	m_HazeronButton = new QToolButton(this);
	m_HazeronButton->setAutoRaise(true);
	m_HazeronButton->setIcon(QIcon(QPixmap(":data/SoHLogo256.png").scaled(64,64,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
	m_HazeronButton->setIconSize(QSize(64,64));
	m_HazeronButton->setToolTip(QObject::tr("<html><b>About <i>%1</i>...</b><br>"
				"Shows information about the <i>%1</i> program."
				"</html>").arg(g_Au.ApplicationName()));
	connect(m_HazeronButton, SIGNAL(clicked()), parent, SIGNAL(HelpAbout()));

	m_Mature17Button = new QToolButton(this);
	m_Mature17Button->setAutoRaise(true);
	m_Mature17Button->setIcon(QIcon(QPixmap(":data/Mature17Plus.png").scaled(32,32,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
	m_Mature17Button->setIconSize(QSize(32,32));
	m_Mature17Button->setToolTip(QObject::tr("<html>"
				"<p><b>Show Mature 17+ Warning</b><br>"
				"The mature 17+ warning message must be acknowleged before playing <i>%1</i>. "
				"The message is reshown whenever the warning message changes. "
				"</p>"
				"</html>").arg(g_Au.ApplicationName()));
	m_Mature17Button->setVisible(!mustAffirm17);
	connect(m_Mature17Button, SIGNAL(clicked()), SLOT(Mature17Show()));

	m_FullScreenButton = new QToolButton(this);
	m_FullScreenButton->setAutoRaise(true);
	m_FullScreenButton->setIcon(QIcon(QPixmap(":data/ToggleFull.png").scaled(16,16,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
	m_FullScreenButton->setIconSize(QSize(16,16));
	m_FullScreenButton->setToolTip(QObject::tr("<html><b>Toggle Full Screen Mode</b><br>"
				"Switches between <i>full screen</i> and <i>window</i> modes."
				"</html>"));
	connect(m_FullScreenButton, SIGNAL(clicked()), parent, SIGNAL(ToggleFullScreen()));

	m_SettingsButton = new QToolButton(this);
	m_SettingsButton->setAutoRaise(true);
	m_SettingsButton->setIcon(QIcon(QPixmap(":data/Settings.png").scaled(16,16,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
	m_SettingsButton->setIconSize(QSize(16,16));
	m_SettingsButton->setToolTip(QObject::tr("<html><b>Settings...</b><br>"
				"Shows the <i>Settings</i> window to change settings that affect the <i>%1</i> program."
				"</html>").arg(g_Au.ApplicationName()));
	connect(m_SettingsButton, SIGNAL(clicked()), parent, SIGNAL(ToggleSettingsUI()));

	QToolButton* exitButton = new QToolButton(this);
	exitButton->setAutoRaise(true);
	exitButton->setIcon(QIcon(QPixmap(":data/ExitGame.png").scaled(12,12,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
	exitButton->setIconSize(QSize(12,12));
	exitButton->setToolTip(QObject::tr("<b>Exit</b><br>"
				"Exits the <i>%1</i> program.").arg(g_Au.ApplicationName()));
	connect(exitButton, SIGNAL(clicked()), parent, SIGNAL(ExitGame()));

	QVBoxLayout* glay = new QVBoxLayout;
	{
		QHBoxLayout* gl1 = new QHBoxLayout;
		gl1->addStretch();
		gl1->addWidget(m_LoginButton);
		gl1->addStretch();
		glay->addLayout(gl1);

#if defined(INTRO_SOLO_TOO)
		QHBoxLayout* gl2 = new QHBoxLayout;
		gl2->addStretch();
		gl2->addWidget(m_SoloButton);
		gl2->addStretch();
		glay->addLayout(gl2);
#endif

		QHBoxLayout* gl3 = new QHBoxLayout;
		gl3->addStretch();
		gl3->addWidget(m_DesignerButton);
		gl3->addStretch();
		glay->addLayout(gl3);
		glay->addStretch();
	}

	QGridLayout* mlay = new QGridLayout;
	mlay->addWidget(m_Mature17Icon, 0,1);
	mlay->addWidget(m_Mature17Label, 1,1);
	mlay->addWidget(m_Mature17Check, 2,1);
	mlay->setColumnStretch(0, 10);
	mlay->setColumnStretch(2, 10);

	QHBoxLayout* slay = new QHBoxLayout;
	slay->addStretch();
	slay->addWidget(m_Mature17Button);
	slay->addStretch();

	QHBoxLayout* alay = new QHBoxLayout;
	alay->addStretch();
	alay->addWidget(m_FullScreenButton);
	alay->addWidget(m_HazeronButton);
	alay->addWidget(m_SettingsButton);
	alay->addStretch();

	QHBoxLayout* xlay = new QHBoxLayout;
	xlay->addStretch();
	xlay->addWidget(exitButton);
	xlay->addStretch();

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addStretch(30);
	lay->addLayout(mlay);
	lay->addLayout(glay);
	lay->addStretch(10);
	lay->addLayout(slay);
	lay->addLayout(alay);
	lay->addLayout(xlay);
	setLayout(lay);
}

AuIntroLaunch::~AuIntroLaunch()
{
}

void AuIntroLaunch::Mature17Show()
{
	m_Mature17Check->show();
	m_Mature17Check->setChecked(false);
	m_Mature17Icon->show();
	m_Mature17Label->show();
	m_DesignerButton->hide();
	m_LoginButton->hide();
	m_Mature17Button->hide();
#if defined(INTRO_SOLO_TOO)
	m_SoloButton->hide();
#endif
}

void AuIntroLaunch::Mature17Clicked()
{
	// Store the current date and time.
	AuTime now(AuTime::Now());
	g_Au.Settings().writeEntry("/Launch/Affirm17", now.UTC().toString());

	// Store the text that was affirmed.
	g_Au.Settings().writeEntry("/Launch/Affirm", m_Affirm);

	m_Mature17Check->hide();
	m_Mature17Icon->hide();
	m_Mature17Label->hide();
	m_DesignerButton->show();
	m_LoginButton->show();
	m_Mature17Button->show();
#if defined(INTRO_SOLO_TOO)
	m_SoloButton->show();
#endif
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

	QToolButton* backButton = new QToolButton(this);
	backButton->setAutoRaise(true);
	backButton->setIcon(QIcon(QPixmap(":data/hazLeftArrow.png").scaled(12,12,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
	backButton->setIconSize(QSize(12,12));
	backButton->setToolTip(QObject::tr("<b>Back</b><br>"
				"Returns to the <i>%1</i> entry screen.").arg(g_Au.ApplicationName()));
	connect(backButton, SIGNAL(clicked()), SIGNAL(Back()));

	QHBoxLayout* play = new QHBoxLayout;
	play->addWidget(m_Password);
	play->addWidget(m_SavePassword);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(m_Login);
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
	lay->addWidget(backButton, row,2);
	row++;
	lay->addWidget(group, row,1, 1,2);
	row++;
	lay->setRowStretch(0, 15);
	lay->setRowStretch(row, 15);
	lay->setColumnStretch(0, 1);
	lay->setColumnStretch(3, 1);
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
			//qDebug() << "AuIntroLogin::Login ok setting global mode to SceneClient";
			g_Au.SetMode(AuGlobal::SceneClient);	// Switch client mode to multi player.
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
			mb.setIconPixmap(QPixmap(auApp->Icon32()));
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
			mb.setIconPixmap(QPixmap(auApp->Icon32()));
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
			mb.setIconPixmap(QPixmap(auApp->Icon32()));
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
			mb.setIconPixmap(QPixmap(auApp->Icon32()));
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

void AuIntroLogin::keyPressEvent(QKeyEvent* e)
{
	QWidget::keyPressEvent(e);
	if(e->key() == Qt::Key_Escape)
		emit Back();
}

