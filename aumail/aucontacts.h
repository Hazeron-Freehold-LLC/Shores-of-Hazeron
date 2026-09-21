//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucontacts.h,v 1.12 2022/02/19 18:36:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUCONTACTS_H
#define AUCONTACTS_H

#include <audialog.h>
#include <auid.h>
#include "aumaillib.h"

class AUMAIL_DLL AuContacts : public AuDialog
{
	Q_OBJECT;
public:
	AuContacts(const AuId& idPerson, QWidget* parent=0);

protected:
	void CreatePage(const AuId& idPerson, const QString& tabName, const QString& objectName, bool isBuddyPage, const QIcon& icon);

signals:
	void UpdateSquelch(AuId idContact, bool bDelete);
	void UpdateSquelches(AuIdList idContacts, bool bDelete);
};

#endif // AUCONTACTS_H
