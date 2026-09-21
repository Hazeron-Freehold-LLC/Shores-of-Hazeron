//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulogin.h,v 1.11 2022/02/19 18:34:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuLogin_H
#define AuLogin_H

#include "audialog.h"
#include "auloginclient.h"

class QLineEdit;

class AULOGINCLIENT_DLL AuLogin : public AuDialog
{
	Q_OBJECT;
private:
	QString								m_AcctName;				// needed to look up contacts
	QByteArray							m_AcctPassCrypt;		// needed to look up contacts. Is a SHA-1 hash of the password.
	bool								m_TestOnly;				// true if only testing credentials, not really 'logging in'

	// Widgets
	QLineEdit*							m_PasswordEdit;

public:
	AuLogin(bool testCredentialsOnly, QWidget* parent=0);

	inline const QString& GetAcctName() const
	{
		return m_AcctName;
	}

	inline const QByteArray& GetAcctPassCrypt() const
	{
		return m_AcctPassCrypt;
	}

public slots:
	void Login();
	void PasswordEdited(const QString& password);
	void Quit();

signals:
	void LoginOk();
};

#endif // AuLogin_H
