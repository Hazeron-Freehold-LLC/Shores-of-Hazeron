//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentenvironment.cpp,v 1.9 2022/02/19 18:03:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auagentenvironment.h"
//
#include "auatomicstructure.h"
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "dabd.h"
#include "daperson.h"
#include "dasolarsystem.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

//NOTE: This entire page is only seen by programmers and above. Regular agents cannot do these things.
AuAgentEnvironment::AuAgentEnvironment(QWidget* parent) : QWidget(parent)
{
	const DgData& gd = g_Au.LockDataForRead();
	bool isProgrammer = gd.ClientPlayerIsProgrammer();
	g_Au.UnlockData();
	AuView* view = auApp->Frame()->View();
	bool hasTarget = !view->TargetId().IsNull();

	// Creatures
	QLabel* dnaLabel = new QLabel("Creatures");

	QPushButton* pbchgcitizen = 0;
	QPushButton* pbchgindigenous = 0;
	QPushButton* pbclearindigenous = 0;
	if(isProgrammer)
	{
		pbchgcitizen = new QPushButton("Set Resident DNA of Building to Mine...");
		connect(pbchgcitizen, SIGNAL(clicked()), SLOT(EnvChgCitizenDNA()));

		pbchgindigenous = new QPushButton("Set Indigenous DNA of Zone to Mine...");
		connect(pbchgindigenous, SIGNAL(clicked()), SLOT(EnvChgIndigenousDNA()));

		pbclearindigenous = new QPushButton("Clear Indigenous DNA of Zone...");
		connect(pbclearindigenous, SIGNAL(clicked()), SLOT(EnvClearIndigenousDNA()));
	}

	// Plants
	QLabel* plantsLabel = 0;
	QPushButton* pbchggroundcover = 0;
	QPushButton* pbchgshrubbery = 0;
	QPushButton* pbchgtrees = 0;
	if(isProgrammer)
	{
		plantsLabel = new QLabel("Plants");

		pbchggroundcover = new QPushButton("Regenerate Ground Cover");
		connect(pbchggroundcover, SIGNAL(clicked()), SLOT(EnvChgGroundCover()));

		pbchgshrubbery = new QPushButton("Regenerate Shrubbery");
		connect(pbchgshrubbery, SIGNAL(clicked()), SLOT(EnvChgShrubbery()));

		pbchgtrees = new QPushButton("Regenerate Trees");
		connect(pbchgtrees, SIGNAL(clicked()), SLOT(EnvChgTrees()));
	}

	QLabel* atomsLabel = new QLabel("Atoms");

	QPushButton* cityReportLast = new QPushButton("Request Last City Report...");
	connect(cityReportLast, SIGNAL(clicked()), SLOT(RequestCityLastReport()));
	if(isProgrammer)
		cityReportLast->setToolTip(tr("<html>Request last city report of current city, or a city id.</html>"));
	else
		cityReportLast->setToolTip(tr("<html>Request last city report of current city.</html>"));

	QPushButton* cityReportHistory = new QPushButton("Request City Report History...");
	connect(cityReportHistory, SIGNAL(clicked()), SLOT(RequestCityHistoryReport()));
	if(isProgrammer)
		cityReportHistory->setToolTip(tr("<html>Request city report history of current city, or a city id.</html>"));
	else
		cityReportHistory->setToolTip(tr("<html>Request city report history of current city.</html>"));


	QPushButton* showSceneAtoms = 0;
	QPushButton* showTargetAtoms = 0;
	if(isProgrammer)
	{
		showSceneAtoms = new QPushButton("Show Scene Atomic Structure");
		connect(showSceneAtoms, SIGNAL(clicked()), SLOT(ShowSceneAtomicStructure()));

		showTargetAtoms = new QPushButton("Show Target Atomic Structure");
		showTargetAtoms->setEnabled(hasTarget);
		connect(showTargetAtoms, SIGNAL(clicked()), SLOT(ShowTargetAtomicStructure()));
	}

	// Layout
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(dnaLabel, row,0, 1,3);
	row++;

	if(pbchgcitizen)
	{
		lay->addWidget(pbchgcitizen, row,1, 1,2);
		row++;
		lay->addWidget(pbchgindigenous, row,1, 1,2);
		row++;
		lay->addWidget(pbclearindigenous, row,1, 1,2);
		row++;
	}

	if(plantsLabel)
	{
		lay->addWidget(plantsLabel, row,0, 1,3);
		row++;

		lay->addWidget(pbchggroundcover, row,1, 1,2);
		row++;
		lay->addWidget(pbchgshrubbery, row,1, 1,2);
		row++;
		lay->addWidget(pbchgtrees, row,1, 1,2);
		row++;
	}

	lay->addWidget(atomsLabel, row,0, 1,3);
	row++;

	lay->addWidget(cityReportLast, row,1, 1,2);
	row++;

	lay->addWidget(cityReportHistory, row,1, 1,2);
	row++;

	if(showSceneAtoms && showTargetAtoms)
	{
		lay->addWidget(showSceneAtoms, row,1, 1,2);
		row++;

		lay->addWidget(showTargetAtoms, row,1, 1,2);
		row++;
	}

	lay->setRowStretch(row,1);
	row++;

	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuAgentEnvironment::~AuAgentEnvironment()
{
}

void AuAgentEnvironment::EnvChgGroundCover()
{
	EnvChgPlantDNA(0);
}

void AuAgentEnvironment::EnvChgShrubbery()
{
	EnvChgPlantDNA(1);
}

void AuAgentEnvironment::EnvChgTrees()
{
	EnvChgPlantDNA(2);
}

// 'plantType' is 0=ground cover, 1=shrubbery, 2=trees
void AuAgentEnvironment::EnvChgPlantDNA(quint8 plantType)
{
	QString plantNames[3] =
	{
		"ground cover plants",
		"shrubbery",
		"trees",
	};

	QString zoneNames[3] =
	{
		"polar",
		"temperate",
		"tropical",
	};

	qint8 floraZone = CurrentFloraZone();
	if(floraZone < 0)
	{
		QMessageBox::warning(this, g_Au.ApplicationName(),
				tr("You must be on the terrain of a planet to regenerate %1.").arg(plantNames[plantType]));
		return;
	}

	int button = QMessageBox::question(this, g_Au.ApplicationName(),
		tr("The DNA of the %1 in the %2 zone of this world will be regenerated. "
		"Players in this area, including you, must restart <i>%3</i> to see the change.<br><br>"
		"<b>Are you sure you want to do this?<b>").arg(plantNames[plantType]).arg(zoneNames[floraZone]).arg(g_Au.ApplicationName()),
		QMessageBox::Ok, QMessageBox::Cancel);
	if(button == QMessageBox::Ok)
		auApp->AgentRegenPlantDNA(plantType, floraZone);
}

void AuAgentEnvironment::EnvChgCitizenDNA()
{
	QString cityName;
	AuView* view = auApp->Frame()->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person && person->IsAtBd())
	{
		DaWorld* planet = person->FindParentPlanetInEnv();
		DaBd* bd = planet->FindBd(person->BdId());
		if(bd)
			cityName = bd->Name();
	}
	view->UnlockSceneAtom();

	if(cityName.isEmpty())
	{
		QMessageBox::warning(this, g_Au.ApplicationName(), tr("You must be at a building to change resident DNA."));
		return;
	}

	int button = QMessageBox::question(this, g_Au.ApplicationName(),
		tr("The resident DNA at %1 will change to match your DNA.<br><br>"
		"<b>Are you sure you want to do this?<b>").arg(cityName),
		QMessageBox::Ok, QMessageBox::Cancel);
	if(button == QMessageBox::Ok)
		auApp->AgentSetCitizenDNA();
}

void AuAgentEnvironment::EnvChgIndigenousDNA()
{
	qint8 floraZone = CurrentFloraZone();
	if(floraZone < 0)
	{
		QMessageBox::warning(this, g_Au.ApplicationName(), tr("You must be on the terrain of a planet to change indigenous DNA."));
		return;
	}

	int button = QMessageBox::question(this, g_Au.ApplicationName(),
		tr("The indigenous DNA in your current zone of this world will change to match your DNA.<br><br>"
		"<b>Are you sure you want to do this?<b>"),
		QMessageBox::Ok, QMessageBox::Cancel);
	if(button == QMessageBox::Ok)
		auApp->AgentSetIndigenousDNA(floraZone);
}

void AuAgentEnvironment::EnvClearIndigenousDNA()
{
	qint8 floraZone = CurrentFloraZone();
	if(floraZone < 0)
	{
		QMessageBox::warning(this, g_Au.ApplicationName(), tr("You must be on the terrain of a planet to clear indigenous DNA."));
		return;
	}

	int button = QMessageBox::question(this, g_Au.ApplicationName(),
		tr("The indigenous DNA in your current zone of this world will be cleared, like it was never set. "
			"<b>DO NOT DO THIS IN A ZONE WITH A CITY!</b><br><br>"
		"<b>Are you sure you want to do this?<b>"),
		QMessageBox::Ok, QMessageBox::Cancel);
	if(button == QMessageBox::Ok)
		auApp->AgentClearIndigenousDNA(floraZone);
}

// returns 0=polar, 1=temperate, 2=tropical, negative is an error, most likely error is no parent terrain patch
int AuAgentEnvironment::CurrentFloraZone() const
{
	int retVal = -1;
	AuView* view = auApp->Frame()->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person && person->IsInAtmosphere())
	{
		const DaWorldLL* ter = dynamic_cast<const DaWorldLL*>(person->FindParentPlanetInEnv());
		if(ter)
		{
			if(fabs(person->GlobeLL().Latitude()) > ter->PolarLatitude())
				retVal = 0;		// polar
			else if(fabs(person->GlobeLL().Latitude()) <= ter->TropicalLatitude())
				retVal = 2;		// tropical
			else
				retVal = 1;		// temperate
		}
	}
	view->UnlockSceneAtom();
	return retVal;
}

void AuAgentEnvironment::ShowSceneAtomicStructure()
{
	AuView* view = auApp->Frame()->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	AuAtomicStructure* dlg;
	if(person)
		dlg = new AuAtomicStructure(*person->FindParentSolarSystem(), this);
	else
		dlg = 0;
	view->UnlockSceneAtom();
	if(dlg)
		dlg->show();
}

void AuAgentEnvironment::ShowTargetAtomicStructure()
{
	AuView* view = auApp->Frame()->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	const DaAtom* target;
	if(person)
		target = person->Ether()->FindAtom(view->TargetId());
	else
		target = 0;
	if(!target)
	{
		view->UnlockSceneAtom();
		return;
	}

	// The dlg constructor collects all the atom information. It is not needed to exec the dialog so we can unlock the scene.
	AuAtomicStructure* dlg = new AuAtomicStructure(*target, this);
	view->UnlockSceneAtom();
	dlg->show();
}

void AuAgentEnvironment::RequestCityHistoryReport()
{
	const DgData& gd = g_Au.LockDataForRead();
	bool isProgrammer = gd.ClientPlayerIsProgrammer();
	g_Au.UnlockData();

	AuId atomId;
	if(isProgrammer)
	{
		QString atomIdS = QInputDialog::getText(this, g_Au.ApplicationName(),
				"<html>Enter city id to report. "
				"An invalid id will result in no response.<p>"
				"Enter <i>Current City</i> to report your current city. "
				"<b>Agent powers must be enabled if your empire does not own the city</b>.</html>", QLineEdit::Normal,
				"Current City");
		if(atomIdS.isEmpty())
			return;

		if(atomIdS != "Current City")
		{
			bool ok;
			quint32 id = atomIdS.toUInt(&ok);
			if(ok)
				atomId = AuId(id);
			else
				atomId = AuId(atomIdS);
		}
	}
	//qDebug() << "AuAgentEnvironment::RequestCityHistoryReport" << atomId;

	auApp->ReportCityHistory(atomId);
}

void AuAgentEnvironment::RequestCityLastReport()
{
	const DgData& gd = g_Au.LockDataForRead();
	bool isProgrammer = gd.ClientPlayerIsProgrammer();
	g_Au.UnlockData();

	AuId atomId;
	if(isProgrammer)
	{
		QString atomIdS = QInputDialog::getText(this, g_Au.ApplicationName(),
				"<html>Enter city id to report. "
				"An invalid id will result in no response.<p>"
				"Enter <i>Current City</i> to report your current city. "
				"<b>Agent powers must be enabled if your empire does not own the city</b>.</html>", QLineEdit::Normal,
				"Current City");
		if(atomIdS.isEmpty())
			return;

		if(atomIdS != "Current City")
		{
			bool ok;
			quint32 id = atomIdS.toUInt(&ok);
			if(ok)
				atomId = AuId(id);
			else
				atomId = AuId(atomIdS);
		}
	}
	//qDebug() << "AuAgentEnvironment::RequestCityLastReport" << atomId;
	auApp->ReportCityLast(atomId);
}

