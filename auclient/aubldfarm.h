//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldfarm.h,v 1.7 2022/02/19 17:51:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBldFarm_H
#define AuBldFarm_H

#include "auclientlib.h"
#include "auid.h"
#include "auspecies.h"
#include <QWidget>

class AuDNABase;
class QLabel;
class QPushButton;
class QTimer;

class AUCLIENT_DLL AuBldFarm : public QWidget
{
	Q_OBJECT;
private:
	AuId								m_CapitolOpId;			// Id of building when operated from a capitol building.
	AuSpecies							m_LivestockSpecies;
	QTimer*								m_Timer;

	// Widgets
	QLabel*								m_Info;
	QPushButton*						m_LivestockButton;

public:
	AuBldFarm(QWidget* parent=0);

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
	void SetLivestockSpecies(const AuSpecies& species);
	virtual void showEvent(QShowEvent* event);

protected slots:
	void LivestockButtonClicked();
	void UpdateContents();
};

#endif // AuBldFarm_H
