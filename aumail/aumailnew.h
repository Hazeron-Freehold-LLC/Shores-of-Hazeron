//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumailnew.h,v 1.9 2022/02/19 18:36:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMailNew_H
#define AuMailNew_H

#include <aucontact.h>
#include <audialog.h>
#include <auid.h>
#include "aumaillib.h"

class QLabel;
class QLineEdit;
class QTextEdit;
class QToolButton;

class AUMAIL_DLL AuMailNew : public AuDialog
{
	Q_OBJECT;
private:
	AuId				m_idPerson;		// id of the person sending the message
	QList<AuContact>	m_Contacts;	// id(s) of the person(s) to receive the message

	// Widgets
	QTextEdit*			m_Message;
	QToolButton*		m_Send;
	QLineEdit*			m_Subject;
	QLabel*				m_ToLabel;

public:
	AuMailNew(QWidget* parent=0, AuId idPerson=0, QList<AuContact>* qlContacts=0, QString contactName="", QString subject="",
			QString body="");

protected slots:
	void MessageChanged();
	void SendClicked();
	void ToClicked();
};

#endif // AuMailNew_H
