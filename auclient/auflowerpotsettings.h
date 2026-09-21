//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auflowerpotsettings.h,v 1.6 2022/02/19 17:55:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuFlowerPotSettings_H
#define AuFlowerPotSettings_H

#include "auclientlib.h"
#include <audialog.h>

class QSlider;

class AUCLIENT_DLL AuFlowerPotSettings : public AuDialog
{
	Q_OBJECT;

private:
	// Widgets
	QSlider*							m_SizeSlider;

public:
	AuFlowerPotSettings(int size, QWidget* parent=0);
	virtual ~AuFlowerPotSettings();

	int PlantSize() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuFlowerPotSettings_H
