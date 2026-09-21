//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldshed.h,v 1.6 2022/02/19 17:52:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBldShed_H
#define AuBldShed_H

#include "auclientlib.h"
#include "auid.h"
#include <QWidget>

class QLabel;
class QTimer;

class AUCLIENT_DLL AuBldShed : public QWidget
{
	Q_OBJECT;
private:
	AuId								m_CapitolOpId;			// Id of building when operated from a capitol building.

public:
	AuBldShed(QWidget* parent=0);
	virtual ~AuBldShed();

	// Clear information needed when this building is operated from a capitol building.
	void ClearCapitolOp()
	{
		m_CapitolOpId = AuId(0);
	}

	// Set information needed when this building is operated from a capitol building.
	void SetCapitolOp(const AuId& capitolOpId)
	{
		m_CapitolOpId = capitolOpId;
	}

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void showEvent(QShowEvent* event);
};

#endif // AuBldShed_H
