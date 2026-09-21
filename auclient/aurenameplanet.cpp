//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurenameplanet.cpp,v 1.5 2022/02/19 18:16:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aurenameplanet.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "daworld.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

AuRenamePlanet::AuRenamePlanet(QWidget* parent) : AuDialog(parent)
{
	QIcon icon(":/data/RangeHail.png");
	setAuDlgIcon(icon);
	setAuDlgTitle("Change World Name");

	// Get the current name of the planet.
	QString planetName;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		DaWorld* planet = person->FindParentPlanetInEnv();
		if(planet)
			planetName = planet->Name();
	}
	pView->UnlockSceneAtom();

	// Create planet name controls.
	QLabel* lbtextplanet = new QLabel(this);
	lbtextplanet->setText(tr("Enter a new name for the world. The current name is <i>%1</i>.").arg(planetName));
	lbtextplanet->setWordWrap(true);

	m_PlanetNameEdit = new QLineEdit(this);
	m_PlanetNameEdit->setFocus();
	m_PlanetNameEdit->setToolTip(tr("<html><b>World Name</b><br>"
				"Enter a new name for the world. The current name is <i>%1</i>.</html>").arg(planetName));
	connect(m_PlanetNameEdit, SIGNAL(textEdited(const QString&)), SLOT(PlanetNameChanged()));

	QPushButton* pbrandomplanet = new QPushButton(this);
	pbrandomplanet->setText(tr("Suggest a Name"));
	pbrandomplanet->setToolTip(tr("Suggests a name for the world."));
	connect(pbrandomplanet, SIGNAL(clicked(bool)), SLOT(RandomPlanetName()));

	// Select a picture based on what is getting named.
	QPixmap img = QPixmap(":data/Backdrop.png").scaled(100,100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QLabel* lbimg = 0;
	if(!img.isNull())
	{
		lbimg = new QLabel(this);
		lbimg->setPixmap(img);
	}

	m_Ok = new QPushButton(tr("Rename Current World"));
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	connect(m_Ok, SIGNAL(clicked()), SLOT(OkPressed()));
	m_Ok->setToolTip(tr("Changes the name of the current world to the name entered."));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbtextplanet, row,0, 1,2);
	row++;
	lay->addWidget(m_PlanetNameEdit, row,0, 1,1);
	lay->addWidget(pbrandomplanet, row,1, 1,1);
	row++;
	lay->addWidget(m_Ok, row,0, 1,2);
	row++;
	lay->setRowStretch(row, 1);
	row++;
	if(lbimg)
	{
		QVBoxLayout* vlay = new QVBoxLayout;
		vlay->addWidget(lbimg);
		vlay->addStretch(1);
		lay->addLayout(vlay, 0,2, row,1);
	}
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	EnableControls();
}

AuRenamePlanet::~AuRenamePlanet()
{
}

// returns enabled state of the ok button
void AuRenamePlanet::EnableControls()
{
	bool okenabled = IsPlanetNameOk();
	m_Ok->setEnabled(okenabled);
}

bool AuRenamePlanet::IsPlanetNameOk() const
{
	Q_ASSERT(m_PlanetNameEdit);

	QString name = m_PlanetNameEdit->text().simplified();
	if(!IsNameValid(name))
		return false;						// Name must not be invalid.
	return true;
}

void AuRenamePlanet::PlanetNameChanged()
{
	EnableControls();
}

void AuRenamePlanet::RandomPlanetName()
{
	// Generate random planet names until we get one that's ok.
	do
	{
		QString name = g_Au.Dice().RandomWorldName();
		m_PlanetNameEdit->setText(name);
	}
	while(!IsPlanetNameOk());
	EnableControls();
}

void AuRenamePlanet::OkPressed()
{
	// Make a final check for valid name(s).
	if(!IsPlanetNameOk())
		return;
	m_PlanetName = m_PlanetNameEdit->text().simplified();
	auApp->PlanetRename(m_PlanetName);
	accept();
}

