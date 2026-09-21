//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auaccountchoosedlg.h,v 1.3 2022/02/19 18:40:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAccountChooseDlg_H
#define AuAccountChooseDlg_H

#include <QDialog>
#include <QMap>

class QComboBox;
class QLineEdit;
class QPushButton;
class QSqlDatabase;

class AuAccountChooseDlg : public QDialog
{
	Q_OBJECT;
	quint32								m_HideAccount;			// Account not shown in the combo box.

	// Widgets
	QComboBox*							m_Accounts;
	QLineEdit*							m_Active;
	QLineEdit*							m_Days;
	QLineEdit*							m_Email;
	QLineEdit*							m_Id;
	QLineEdit*							m_Logged;
	QPushButton*						m_Ok;
	QLineEdit*							m_UnitId[6];
	QLineEdit*							m_UnitName[6];
	QLineEdit*							m_UnitSex[6];

public:
	AuAccountChooseDlg(quint32 hideAccount, QWidget* parent=0);
	virtual ~AuAccountChooseDlg();

	quint32 Account() const;

	int EmptyUnitIndex() const;

public slots:
	void AccountChanged(int acctIndex);
	virtual void setVisible(bool visible);

protected:
	void ListAccounts();
	void ListAccounts(QSqlDatabase& dbLogin, QSqlDatabase& dbUniverse);
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuAccountChooseDlg_H
