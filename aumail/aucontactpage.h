//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucontactpage.h,v 1.13 2022/02/19 18:36:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuContactPage_H
#define AuContactPage_H

#include "aumaillib.h"
#include "auid.h"
#include <QWidget>

class QListWidgetItem;

class AUMAIL_DLL AuContactPage : public QWidget
{
	Q_OBJECT;
	
private:
	AuId								m_idPerson;
	bool								m_isBuddyList;	// false = blocked list

public:
	AuContactPage(const AuId& idPerson, bool isBuddyList, const QString& objectName, QWidget* parent=0);

	void ShowContactList();

signals:
	void UpdateSquelch(AuId idContact, bool bDelete);
	void UpdateSquelches(AuIdList idContacts, bool bDelete);

public slots:
	virtual void setVisible(bool visible);

protected slots:
	void Add();
	void Delete();
};

#endif // AuContactPage_H
