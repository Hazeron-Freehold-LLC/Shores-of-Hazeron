//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldstoragetank.h,v 1.6 2022/02/19 17:52:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBldStorageTank_H
#define AuBldStorageTank_H

#include "auclientlib.h"
#include "auid.h"
#include <QWidget>

class QComboBox;
class QLabel;
class QTimer;

class AUCLIENT_DLL AuBldStorageTank : public QWidget
{
	Q_OBJECT;
private:
	AuId								m_CapitolOpId;			// Id of building when operated from a capitol building.
	QTimer*								m_Timer;

	// Widgets
	QComboBox*							m_StoreCombo;

public:
	AuBldStorageTank(QWidget* parent=0);
	virtual ~AuBldStorageTank();

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

protected slots:
	void CommoditySelectionChanged(int index);
	void UpdateContents();
};

#endif // AuBldStorageTank_H
