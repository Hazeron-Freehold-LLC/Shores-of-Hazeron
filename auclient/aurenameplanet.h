//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurenameplanet.h,v 1.6 2022/02/19 17:59:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuRenamePlanet_H
#define AuRenamePlanet_H

#include "auclientlib.h"
#include <audialog.h>

class QLineEdit;
class QPushButton;

class AUCLIENT_DLL AuRenamePlanet : public AuDialog
{
	Q_OBJECT;
private:
	QString		m_PlanetName;		// Final name on accept()

	// Widgets
	QLineEdit*		m_PlanetNameEdit;
	QPushButton*	m_Ok;

public:
	AuRenamePlanet(QWidget* parent=0);
	virtual ~AuRenamePlanet();

	// Only valid on accept()
	const QString& PlanetName() const
	{
		return m_PlanetName;
	}

protected slots:
	void EnableControls();
	void OkPressed();
	void PlanetNameChanged();
	void RandomPlanetName();

protected:
	bool IsPlanetNameOk() const;													// Checks the text in the edit control.
};

#endif // AuRenamePlanet_H
