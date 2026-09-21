//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentencounters.h,v 1.9 2022/02/19 17:49:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAgentEncounters_H
#define AuAgentEncounters_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"
#include <QWidget>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;

class AUCLIENT_DLL AuAgentEncounters : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	QGroupBox*	m_gbplay;

public:
	AuAgentEncounters(QWidget* parent=0);
	virtual ~AuAgentEncounters();

protected slots:
	void AerialEncounter();
	void AnimalEncounter();
	void Asteroid();
	void IndigenousEncounter();
	void IndigenousEncounterForceMove();
	void KillDomesticAnimals();
	void KillWildAnimals();
	void MonsterEncounter();
	void PirateShip();
};

#endif // AuAgentEncounters_H
