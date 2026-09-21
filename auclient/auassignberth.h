//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auassignberth.h,v 1.6 2022/02/19 17:49:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAssignBerth_H
#define AuAssignBerth_H

#include "auclientlib.h"
#include <auroster.h>
#include <audialog.h>
#include <QIcon>

class QListWidgetItem;

class AUCLIENT_DLL AuAssignBerth : public AuDialog
{
	Q_OBJECT;
public:
	AuAssignBerth(bool captainOk, bool officerOk, bool crewOk,
			bool passengerOk, bool troopOk, bool anyOk, QWidget* parent=0);
	~AuAssignBerth();

	AuRosterEntry::TYPE TypeAssigned() const
	{
		// Returns Captain if the 'Any Berth' option was selected.
		return m_TypeAssigned;
	}

protected slots:
	void AssignBerth();
	void TypeDoubleClicked(QListWidgetItem* item);
	void EnableControls();

private:
	AuRosterEntry::TYPE		m_TypeAssigned;	// Set on ok.
};

#endif // AuAssignBerth_H
