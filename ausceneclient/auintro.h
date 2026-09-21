//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auintro.h,v 1.19 2022/04/15 20:38:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuIntro_H
#define AuIntro_H

#include "aucrypt.h"
#include "auelapsedtimer.h"
#include "auintrobase.h"

class AuIntroLaunch;
class AuIntroLogin;
class AuMsgDialog;
class QCheckBox;
class QKeyEvent;
class QLineEdit;
class QPushButton;
class QStackedLayout;
class QToolButton;

//#define INTRO_SOLO_TOO					// When defined, Hazeron Starship can be played using this client.

class AuIntro : public AuIntroBase
{
	Q_OBJECT;
	int									m_ChooseAvatarIdx;
	int									m_CreateAvatarIdx;
	int									m_CreateAvatar4Idx;
	bool								m_HasInfo;
	int									m_LaunchIdx;
	int									m_LoginIdx;
	AuMsgDialog*						m_msgDialog;
#if defined(INTRO_SOLO_TOO)
	bool								m_Solo;
	int									m_SoloAvatarIdx;
#endif
	AuElapsedTimer						m_WaitTime;

	// Widgets
	AuIntroChooseAvatar*				m_ChooseAvatarPage;
	AuIntroCreateAvatar*				m_CreateAvatarPage;
	AuIntroCreateAvatar4*				m_CreateAvatar4Page;
	AuIntroLaunch*						m_LaunchPage;
	AuIntroLogin*						m_LoginPage;
	QStackedLayout*						m_Pages;
#if defined(INTRO_SOLO_TOO)
	AuIntroSoloAvatar*					m_SoloAvatarPage;
#endif

public:
	AuIntro(QWidget* parent=0);
	virtual ~AuIntro();

public:
	virtual bool HasAccountInfo() const
	{
		return m_HasInfo;
	}

	virtual void paintEvent(QPaintEvent* pe);

	virtual void SetAccountInfo();

	virtual void ShowWaitInfoMsg();

public slots:
	virtual void EmpireDataReceived();
	virtual void EnterNewAvatar();

protected slots:
	void ExistingAvatarLogin(AuId avatarId);
	void LaunchLogin();
#if defined(INTRO_SOLO_TOO)
	void LaunchSolo();
#endif
	void LoggedIn();
	void LoginBack();
	void NewAvatarBack();
	void NewAvatarDNA3(int index);
	void NewAvatarDNA4(int index);
	void NewAvatarLogin(int index);
	void TimerUpdate();

signals:
	void AvatarLogin(AuId);
#if defined(INTRO_SOLO_TOO)
	void AvatarSolo(AuId);
#endif
	void ExitGame();
	void HelpAbout();
	void LaunchDesigner();
	void LoginOk();
	void ToggleFullScreen();
	void ToggleSettingsUI();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroLaunch : public QWidget
{
	Q_OBJECT;
	AuCrypt								m_Affirm;

	// Widgets
	QToolButton*						m_DesignerButton;
	QToolButton*						m_FullScreenButton;
	QToolButton*						m_HazeronButton;
	QToolButton*						m_LoginButton;
	QToolButton*						m_Mature17Button;
	QCheckBox*							m_Mature17Check;
	QLabel*								m_Mature17Icon;
	QLabel*								m_Mature17Label;
	QToolButton*						m_SettingsButton;
	QToolButton*						m_SoloButton;

public:
	AuIntroLaunch(AuIntro* parent=0);
	virtual ~AuIntroLaunch();

protected slots:
	void Mature17Clicked();
	void Mature17Show();

signals:
	void LaunchDesigner();
	void LaunchLogin();
	void LaunchSolitaire();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroLogin : public QWidget
{
	Q_OBJECT;
	QByteArray							m_PassCrypt;			// Encrypted password.

	// Widgets
	QLineEdit*							m_AccountName;
	QPushButton*						m_Login;
	QLineEdit*							m_Password;
	QCheckBox*							m_SavePassword;

public:
	AuIntroLogin(AuIntro* parent=0);
	virtual ~AuIntroLogin();

protected:
	virtual void keyPressEvent(QKeyEvent* event);

protected slots:
	void Login();
	void PasswordEdited(const QString& password);

signals:
	void Back();
	void LoginOk();
};

#endif // AuIntro_H
