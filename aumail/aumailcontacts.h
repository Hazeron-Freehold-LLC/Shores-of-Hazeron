//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumailcontacts.h,v 1.3 2022/02/19 18:36:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMailContacts_H
#define AuMailContacts_H

#include <aucontact.h>
#include <audialog.h>
#include <auid.h>
#include "aumaillib.h"

class QListWidgetItem;

class AUMAIL_DLL AuMailContacts : public AuDialog
{
	Q_OBJECT;
	
public:
	QList<AuContact> m_Contacts;

public:

	AuMailContacts(QWidget* parent=0);

	void ShowContactList();

//public slots:
//	virtual void setVisible(bool visible);

protected slots:
	//void Add();
	void EmitContactDoubleClicked(QListWidgetItem* contact);
	void Recipients();

signals:
	void ContactDoubleClicked(AuContact contact);
};

#endif // AuMailContacts_H
