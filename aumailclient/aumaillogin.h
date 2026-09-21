//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumaillogin.h,v 1.2 2022/02/19 18:37:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMailLogin_H
#define AuMailLogin_H

#include "audialog.h"
#include "auloginclient.h"

class QLineEdit;

class AuMailLogin : public AuDialog
{
	Q_OBJECT;
private:
	QString								m_AcctName;				// needed to look up contacts
	QByteArray							m_AcctPassCrypt;		// needed to look up contacts. Is a SHA-1 hash of the password.

private:
	// Widgets
	QLineEdit*							m_PasswordEdit;

public:
	AuMailLogin(QWidget* parent=0);

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
	void Settings();

signals:
	void LoginOk();
};

#endif // AuMailLogin_H
