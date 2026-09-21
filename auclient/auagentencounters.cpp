//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentencounters.cpp,v 1.17 2022/02/19 18:03:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auagentencounters.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "aucommpacket.h"
#include "auglobal.h"
#include "daperson.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

AuAgentEncounters::AuAgentEncounters(QWidget* parent) : QWidget(parent)
{
	const DgData& gd = g_Au.LockDataForRead();
	bool isProgrammer = gd.ClientPlayerIsProgrammer();
	g_Au.UnlockData();

	QLabel* animalLabel = new QLabel("Creatures");

	QLabel* pirateLabel = 0;
	QPushButton* pbpirate = 0;

	QPushButton* pbanimalenc = 0;
	QPushButton* pbaerialenc = 0;
	QPushButton* pbindigenous = 0;
	QPushButton* pbindmovem = 0;
	QPushButton* pbmonsterenc = 0;
	QPushButton* pbasteroid = 0;
	if(isProgrammer)
	{
		pbanimalenc = new QPushButton("Animal Encounter");
		connect(pbanimalenc, SIGNAL(clicked()), SLOT(AnimalEncounter()));
		pbanimalenc->setToolTip(QString("<html><b>Animal Encounter</b><br>"
					"Triggers an animal encounter in your current terrain. "
					"Terrain must be habitable. Animal population limits are ignored.</html>"));

		pbaerialenc = new QPushButton("Aerial Animal Encounter");
		connect(pbaerialenc, SIGNAL(clicked()), SLOT(AerialEncounter()));
		pbaerialenc->setToolTip(QString("<html><b>Aerial Animal Encounter</b><br>"
					"Triggers an animal encounter in your current terrain with an aerial creature. "
					"Terrain must be habitable. Animal population limits are ignored.</html>"));

		if(!auApp->IsSingleUserDesignerMode())
		{
			pbmonsterenc = new QPushButton("Monster Encounter");
			connect(pbmonsterenc, SIGNAL(clicked()), SLOT(MonsterEncounter()));
			pbmonsterenc->setToolTip(QString("<html><b>Monster Encounter</b><br>"
						"Triggers a monster encounter in your current spacecraft."
						"</html>"));

			pbindigenous = new QPushButton("Indigenous Encounter");
			connect(pbindigenous, SIGNAL(clicked()), SLOT(IndigenousEncounter()));
			pbindigenous->setToolTip(QString("<html><b>Indigenous Encounter</b><br>"
						"Triggers an indigenous encounter in your current terrain. "
						"Terrain must be habitable. "
						"Existing village may be too far away.</html>"));

			pbindmovem = new QPushButton("Indigenous Encounter Force Move Village");
			connect(pbindmovem, SIGNAL(clicked()), SLOT(IndigenousEncounterForceMove()));
			pbindmovem->setToolTip(QString("<html><b>Indigenous Encounter Force Move Village</b><br>"
						"Triggers an indigenous encounter in your current terrain. "
						"Terrain must be habitable. "
						"Existing village will be moved if it is too far away.</html>"));

			pirateLabel = new QLabel("Pirates");

			pbpirate = new QPushButton("Pirate");
			connect(pbpirate, SIGNAL(clicked()), SLOT(PirateShip()));
			pbpirate->setToolTip(QString("<html>Triggers a pirate encounter. "
						"Result is reported as a thought message."
						"</html>"));
		}

		pbasteroid = new QPushButton("Asteroid...");
		connect(pbasteroid, SIGNAL(clicked()), SLOT(Asteroid()));
		pbasteroid->setToolTip(QString("<html>Triggers a world killer asteroid targetted at your current world. "
					"</html>"));
	}

	QPushButton* pbkilldom = new QPushButton("Kill Domestic Animals...");
	pbkilldom->setToolTip("<html>Kills all domestic animals in your scene.</html>");
	connect(pbkilldom, SIGNAL(clicked()), SLOT(KillDomesticAnimals()));

	QPushButton* pbkillwild = new QPushButton("Kill Wild Animals...");
	pbkillwild->setToolTip("<html>Kills all wild animals in your scene.</html>");
	connect(pbkillwild, SIGNAL(clicked()), SLOT(KillWildAnimals()));

	QGridLayout* lay = new QGridLayout;
	int row=0;

	if(pirateLabel && pbpirate)
	{
		lay->addWidget(pirateLabel, row,0, 1,3);
		row++;

		lay->addWidget(pbpirate, row,1, 1,2);
		row++;
	}

	lay->addWidget(animalLabel, row,0, 1,3);
	row++;

	if(pbanimalenc || pbaerialenc || pbmonsterenc || pbindigenous || pbindmovem)
	{
		if(pbanimalenc)
		{
			lay->addWidget(pbanimalenc, row,1, 1,2);
			row++;
		}
		if(pbaerialenc)
		{
			lay->addWidget(pbaerialenc, row,1, 1,2);
			row++;
		}
		if(pbmonsterenc)
		{
			lay->addWidget(pbmonsterenc, row,1, 1,2);
			row++;
		}
		if(pbindigenous)
		{
			lay->addWidget(pbindigenous, row,1, 1,2);
			row++;
		}
		if(pbindmovem)
		{
			lay->addWidget(pbindmovem, row,1, 1,2);
			row++;
		}
	}

	lay->addWidget(pbkilldom, row,1, 1,2);
	row++;
	lay->addWidget(pbkillwild, row,1, 1,2);
	row++;

	if(pbasteroid)
	{
		lay->addWidget(pbasteroid, row,1, 1,2);
		row++;
	}

	lay->setRowStretch(row,1);
	row++;

	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuAgentEncounters::~AuAgentEncounters()
{
}

void AuAgentEncounters::KillDomesticAnimals()
{
	int button = QMessageBox::question(this, g_Au.ApplicationName(),
		tr("<html>All domestic animals in your scene will be killed.<br><br>"
			"<b>Are you sure you want to do this?<b></html>"),
		QMessageBox::Ok, QMessageBox::Cancel);
	if(button == QMessageBox::Ok)
		auApp->AgentKillDomesticAnimals();
}

void AuAgentEncounters::KillWildAnimals()
{
	int button = QMessageBox::question(this, g_Au.ApplicationName(),
		tr("<html>All wild animals in your scene will be killed.<br><br>"
			"<b>Are you sure you want to do this?<b></html>"),
		QMessageBox::Ok, QMessageBox::Cancel);
	if(button == QMessageBox::Ok)
		auApp->AgentKillWildAnimals();
}

void AuAgentEncounters::AerialEncounter()
{
	auApp->AgentAerialEncounter();
}

void AuAgentEncounters::AnimalEncounter()
{
	auApp->AgentAnimalEncounter();
}

void AuAgentEncounters::MonsterEncounter()
{
	auApp->AgentMonsterEncounter();
}

void AuAgentEncounters::IndigenousEncounter()
{
	auApp->AgentIndigenousEncounter();
}

void AuAgentEncounters::IndigenousEncounterForceMove()
{
	int button = QMessageBox::question(this, g_Au.ApplicationName(),
		tr("<html>An indigenous encounter will be forced.<p>"
			"<b>Do you want your reputation reset also?<b></html>"),
		QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);

	if(button == QMessageBox::Yes)
		auApp->AgentIndigenousEncounterForceMoveResetReputation();
	else if(button == QMessageBox::No)
		auApp->AgentIndigenousEncounterForceMove();
	else
		return;
}

void AuAgentEncounters::PirateShip()
{
	auApp->AgentEncounterPirateShip();
}

void AuAgentEncounters::Asteroid()
{
	bool ok;
	quint32 minutesFromNow = (quint32)QInputDialog::getInt(this, tr("Asteroid..."),
			tr("Enter the number of minutes before the asteroid impacts your current world. "), 10, 1, 60*24*7, 1, &ok);
	if(!ok)
		return;

	auApp->AgentEncounterAsteroid(minutesFromNow);
}

