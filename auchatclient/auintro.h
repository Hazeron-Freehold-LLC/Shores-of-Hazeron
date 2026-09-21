//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auintro.h,v 1.10 2022/04/15 20:58:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuIntro_H
#define AuIntro_H

#include "auintrobase.h"

class AuIntroChooseAvatarNoRender;
class AuIntroLogin;
class AuMsgDialog;
class AuPictureWidget;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QStackedLayout;
class QToolButton;

class AuIntro : public AuIntroBase
{
	Q_OBJECT;
	bool								m_HasInfo;
	AuMsgDialog*						m_msgDialog;
	AuElapsedTimer						m_WaitTime;

	// Widgets
	AuIntroChooseAvatarNoRender*		m_ChooseAvatarPage;
	AuIntroLogin*						m_LoginPage;
	QStackedLayout*						m_Pages;

public:
	AuIntro(QWidget* parent=0);
	virtual ~AuIntro();

public:
	virtual bool HasAccountInfo() const
	{
		return m_HasInfo;
	}

	virtual void paintEvent(QPaintEvent* pe);

	void SetAccountInfo();

	void ShowWaitInfoMsg();

protected slots:
	void LoggedIn();
	void ExistingAvatarLogin(AuId avatarId);
	void TimerUpdate();

signals:
	void AvatarLogin(AuId);
	void ExitGame();
	void HelpAbout();
	void LoginOk();
	void ToggleFullScreen();
	void ToggleSettingsUI();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroLogin : public QWidget
{
	Q_OBJECT;
	QByteArray							m_PassCrypt;			// Encrypted password.

	// Widgets
	QLineEdit*							m_AccountName;
	QToolButton*						m_FullScreenButton;
	QToolButton*						m_HazeronButton;
	QPushButton*						m_Login;
	QLineEdit*							m_Password;
	QCheckBox*							m_SavePassword;
	QToolButton*						m_SettingsButton;

public:
	AuIntroLogin(AuIntro* parent=0);
	virtual ~AuIntroLogin();

protected slots:
	void Login();
	void PasswordEdited(const QString& password);

signals:
	void LoginOk();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroChooseAvatarNoRender : public QWidget
{
	Q_OBJECT;
	int									m_AvatarSampleRendered;	// Number of frames of sample rendered.

	// Widgets
	AuIntroAvatarWidget*				m_Avatar[6];
	AuPictureWidget*					m_NewAvatarImage;

public:
	AuIntroChooseAvatarNoRender(AuIntro* parent=0);
	virtual ~AuIntroChooseAvatarNoRender();

	void SetAccountInfo();

protected slots:
	void AvatarDeleted(AuId avatarId);
	void AvatarSelected(int index);

signals:
	void AvatarLogin(AuId);
};

#endif // AuIntro_H
