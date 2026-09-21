//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auaccountmanagerdlg.h,v 1.7 2022/02/19 18:40:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAccountManagerDlg_H
#define AuAccountManagerDlg_H

#include <QDialog>
#include <QMap>
#include <QPushButton>

class AuPushButtonValue;
class QComboBox;
class QLineEdit;
class QPushButton;
class QSqlDatabase;

class AuAccountManagerDlg : public QDialog
{
	Q_OBJECT;

	// Widgets
	QComboBox*							m_Accounts;
	QLineEdit*							m_Active;
	QLineEdit*							m_Days;
	QLineEdit*							m_Email;
	QLineEdit*							m_Id;
	QLineEdit*							m_Logged;
	QLineEdit*							m_UnitId[6];
	QLineEdit*							m_UnitName[6];
	QLineEdit*							m_UnitSex[6];
	AuPushButtonValue*					m_UnitAcct[6];

public:
	AuAccountManagerDlg(QWidget* parent=0);
	virtual ~AuAccountManagerDlg();

public slots:
	void AccountChanged(int acctIndex);
	void FindAvatarId();
	void FindAvatarName();
	void FindEmail();
	virtual void setVisible(bool visible);

protected:
	void ListAccounts();
	void ListAccounts(QSqlDatabase& dbLogin);
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void ChangeUnitAccount(int unitIndex);
};

#endif // AuAccountManagerDlg_H
