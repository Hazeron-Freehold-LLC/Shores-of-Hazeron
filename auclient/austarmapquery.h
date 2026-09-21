//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austarmapquery.h,v 1.6 2022/02/19 18:01:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStarMapQuery_H
#define AuStarMapQuery_H

#include "auclientlib.h"
#include "audef.h"
#include "audialog.h"

class QCheckBox;
class QComboBox;
class QRadioButton;
class QSpinBox;

class AUCLIENT_DLL AuStarMapQuery : public AuDialog
{
	Q_OBJECT;
private:
	int									m_CurRadio;

	// Widgets
	QCheckBox*							m_3xSectors;
	QRadioButton*						m_All;
	QCheckBox*							m_ClearMap;
	QComboBox*							m_Galaxies;
	QRadioButton*						m_Galaxy;
	QSpinBox*							m_Range;
	QRadioButton*						m_Sector;

public:
	AuStarMapQuery(QWidget* parent=0);
	virtual ~AuStarMapQuery();

	bool ClearMap() const;	// true to clear local map data before getting fresh query
	GALAXY Galaxy() const;
	bool Include3xSectors() const;
	int Query() const;	// 0=sector, 1=galaxy, 2=universe
	int Range() const;

public slots:
	void AllRadio();
	void EnableControls();
	void GalaxyRadio();
	void PostQuery();
	virtual void setVisible(bool visible);
	void SectorRadio();

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuStarMapQuery_H
