//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumissionorderui.h,v 1.6 2022/02/19 17:58:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMissionOrderUi_H
#define AuMissionOrderUi_H

#include "auclientlib.h"
#include <audialog.h>
#include <aumission.h>

class AuMissionOrderWidget;
class AuSensorBlipList;
class QPushButton;

class AUCLIENT_DLL AuMissionOrderUi : public AuDialog
{
	Q_OBJECT;
private:
	AuMissionOrderWidget*		m_MO;
	QPushButton*				m_Ok;

public:
	AuMissionOrderUi(const AuId& authorization, const QString& title, const AuId& solarSystem, const AuSensorBlipList& blips,
			const AuPoint& galaxyLoc, const AuPoint& sectorLoc, const AuPoint& systemLoc, const AuMissionOrder* order=0,
			QWidget* parent=0);
	virtual ~AuMissionOrderUi();

	// Returns a mission order corresponding to current settings.
	// Mission order is allocated on the heap using new.
	// Caller must delete the pointer that is returned.
	AuMissionOrder* Order() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
	virtual QSize sizeHint() const;

protected slots:
	void AcceptOrder();
	void EnableControls();
};

#endif // AuMissionOrderUi_H
