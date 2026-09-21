//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augotogalaxyloc.h,v 1.2 2022/02/19 17:56:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGoToGalaxyLoc_H
#define AuGoToGalaxyLoc_H

#include "audef.h"
#include "audialog.h"
#include "aupoint.h"

class QComboBox;
class QLineEdit;

class AuGoToGalaxyLoc : public AuDialog
{
	Q_OBJECT;
	QComboBox*							m_Galaxies;
	QLineEdit*							m_GalaxyCoord;

public:
	AuGoToGalaxyLoc(QWidget* parent=0);
	virtual ~AuGoToGalaxyLoc();

	AuPoint Coordinate() const;

	GALAXY Galaxy() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuGoToGalaxyLoc_H
