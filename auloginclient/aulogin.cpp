//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulogin.cpp,v 1.31 2022/04/08 14:52:37 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aulogin.h"
//
#include "auglobal.h"
#include "aulogincheck.h"
#include "aulogincomm.h"
#include <QCheckBox>
#include <QCryptographicHash>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

AuLogin::AuLogin(bool testCredentialsOnly, QWidget* parent) : AuDialog(parent)
{
	m_TestOnly = testCredentialsOnly;
	setAuDlgIcon(QIcon(":/data/Login.png"));
	setAuDlgTitle(g_Au.ApplicationName() + " Login");

	m_AcctName = g_Au.Settings().readEntry("/Account/Name","");
	int savePassword = g_Au.Settings().readNumEntry("/Account/SavePass", 0);
	if(savePassword)
		g_Au.Settings().readEntry("/Account/Password", m_AcctPassCrypt);

	QLabel* lb1 = new QLabel("Account Name");
	QLineEdit* le1 = new QLineEdit(m_AcctName);
	le1->setObjectName("m_AcctName");
	QString toolTip;
	toolTip = QObject::tr("Enter the account name that was e-mailed to you when you "
			"licensed %1.\nThe account name is case sensitive.\nThis "
			"is not your avatar's 'screen name'.").arg(g_Au.ApplicationName());
	le1->setToolTip(toolTip);

	QLabel* lb2 = new QLabel("Password");
	m_PasswordEdit = new QLineEdit;
	if(!m_AcctPassCrypt.isEmpty())
		m_PasswordEdit->setText("*");
	m_PasswordEdit->setEchoMode(QLineEdit::Password);
	toolTip = QObject::tr("Enter the password that was last e-mailed to you, when "
			"you\ninitially licensed %1 or last requested a password "
			"change.\nThe password is case sensitive.").arg(g_Au.ApplicationName());
	m_PasswordEdit->setToolTip(toolTip);
	connect(m_PasswordEdit, SIGNAL(textEdited(const QString&)), SLOT(PasswordEdited(const QString&)));

	QCheckBox* cb = new QCheckBox("Save Password");
	cb->setObjectName("savepass");
	cb->setToolTip(tr("Enable this check box to save your login password\n"
			"between sessions."));
	if(savePassword)
		cb->setCheckState(Qt::Checked);

	QPushButton* pb1 = new QPushButton("Login", this);
	pb1->setObjectName("login");
	pb1->setAutoDefault(true);
	pb1->setDefault(true);
	connect(pb1, SIGNAL(clicked()), SLOT(Login()));
	toolTip = QObject::tr("Login to %1 using the account name and password "
			"entered.").arg(g_Au.ApplicationName());
	pb1->setToolTip(toolTip);

	QPushButton* pb2 = new QPushButton("Quit");
	pb2->setObjectName("quit");
	connect(pb2, SIGNAL(clicked()), SLOT(Quit()));
	toolTip = QObject::tr("Close the %1 program.").arg(g_Au.ApplicationName());
	pb2->setToolTip(toolTip);

	QGridLayout* lay = new QGridLayout();
	//if(accountName.isEmpty())
	{
		// First time login so request account name first.
		lay->addWidget(lb1, 0, 0);
		lay->addWidget(le1, 0, 1);
		lay->addWidget(lb2, 1, 0);
		lay->addWidget(m_PasswordEdit, 1, 1);
		lay->addWidget(cb, 2, 0, 1,2);
	}
	/*
	else
	{
		// We already have the name so present the password box first.
		// This makes it easier for different people to alternate logging in
		// by taking advantage of the tab select behavior when moving to the
		// second edit control. Otherwise you end up pushing a lot of buttons.
		lay->addWidget(lb2, 0, 0);
		lay->addWidget(m_PasswordEdit, 0, 1);
		lay->addWidget(lb1, 1, 0);
		lay->addWidget(le1, 1, 1);
	}
	*/
	lay->addWidget(pb1, 3, 1);
	lay->addWidget(pb2, 3, 0);
    lay->setMargin(4);
    lay->setSpacing(4);
	setAuDlgLayout(lay);
}

void AuLogin::Login()
{
	QLineEdit* le1 = findChild<QLineEdit*>("m_AcctName");
	if(le1)
		m_AcctName = le1->text();
	if(m_AcctName.isEmpty())
		return;	// Blank name is not allowed.

	if(m_AcctPassCrypt.isEmpty())
		return;	// Blank password is not allowed.

	bool savePass = false;
	QCheckBox* cb = findChild<QCheckBox*>("savepass");
	if(cb && cb->checkState() == Qt::Checked)
		savePass = true;

	QPushButton* pb1 = findChild<QPushButton*>("login");
	QPushButton* pb2 = findChild<QPushButton*>("quit");

	// Save last attempted name.
	g_Au.Settings().writeEntry("/Account/Name", m_AcctName);
	g_Au.Settings().writeEntry("/Account/SavePass", savePass?1:0);
	if(savePass)
		g_Au.Settings().writeEntry("/Account/Password", m_AcctPassCrypt);
	else
	{
		QByteArray blankPass;
		g_Au.Settings().writeEntry("/Account/Password", blankPass);
	}

	// Disable controls while waiting to login.
	le1->setEnabled(false);
	m_PasswordEdit->setEnabled(false);
	cb->setEnabled(false);
	pb1->setEnabled(false);
	pb2->setEnabled(false);

	setCursor(QCursor(Qt::WaitCursor));

	int result;
	if(m_TestOnly)
	{
		AuLoginCheck logincomm;
		result = logincomm.LoginTestCredentials(m_AcctName, m_AcctPassCrypt);
	}
	else
	{
		AuLoginComm logincomm;
		result = logincomm.Login(m_AcctName, m_AcctPassCrypt);
	}

	// Enable controls now that we have the login result.
	le1->setEnabled(true);
	m_PasswordEdit->setEnabled(true);
	cb->setEnabled(true);
	pb1->setEnabled(true);
	pb2->setEnabled(true);

	switch(result)
	{
	case 0:	// Login ok
		{
			accept();
			emit LoginOk();
		}
		break;

	case 1:	// Error.
		{
			QString msg = tr("An error occurred while attempting to log in.\n\n"
				"Tips:\n- Check your network connection.\n- Reinstall the "
				"software.\n- Download the latest version from %1\nand try again.")
				.arg(g_Au.WebAddress());
			QMessageBox::critical(0, g_Au.ApplicationName(), msg);
			Quit();
		}
		break;

	case 2:	// Login failure probably due to bad password or disabled login
			// account.
		{
			QString msg = tr("Login attempt failed. Please try again.\n\n"
					"Tips:\n- Double check your login account name and "
					"password. They are case\nsensitive.\n- Your login "
					"account name is usually not the same as your\n"
					"avatar's screen name.");
			QMessageBox::warning(0, g_Au.ApplicationName(), msg);
		}
		break;

	case 3:	// Incorrect software version.
		{
			QString msg = tr("This software version is incompatible with the "
					"server.\n\nTips:\n- Download the latest version from %1"
					"\nand try again.").arg(g_Au.WebAddress());
			QMessageBox::critical(0, g_Au.ApplicationName(), msg);
			Quit();
		}
		break;
	}
}

void AuLogin::Quit()
{
	qApp->exit();	//DHH Qt5.3.0 qApp->closeAllWindows();
	reject();
}

void AuLogin::PasswordEdited(const QString& password)
{
	// Put password into a byte array.
	QByteArray passBytes;
	passBytes.append(password.toUtf8());

	// Create a hash from the password.
	QCryptographicHash passHash(QCryptographicHash::Sha1);
	passHash.addData(passBytes);
	m_AcctPassCrypt = passHash.result();
}
