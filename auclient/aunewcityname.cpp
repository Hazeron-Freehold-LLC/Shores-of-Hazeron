//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aunewcityname.cpp,v 1.4 2022/02/19 18:16:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aunewcityname.h"
//
#include "aucity.h"
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include <daperson.h>
#include <dasector.h>
#include <dasolarsystem.h>
#include <dgdata.h>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

AuNewCityName::AuNewCityName(CITYTYPE type, QWidget* parent) : AuDialog(parent)
{
	QIcon icon(AuCity::CityIcon(type));
	switch(type)
	{
	case CITY_City_Obsolete:
		setAuDlgTitle("New City Name");
		break;
	case CITY_Capitol:
		setAuDlgTitle("New City Name");
		break;
	case CITY_MilitaryBase:
		setAuDlgTitle("New Military Base Name");
		break;
	}
	setAuDlgIcon(icon);

	// Get a list of all the solar system names in the sector.
	// Determine if the person's current sector and solar system already have names.
	bool hasSectorName = false;
	bool hasSystemName = false;
	QStringList systemNames;					// List of solar system names in the sector.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		const DaSolarSystem* system = person->FindParentSolarSystem();
		if(system)
		{
			hasSystemName = system->HasName();
			const DaSector* sector = system->FindParentSector();
			if(sector)
			{
				hasSectorName = sector->HasName();
				sector->ListSolarSystemNames(systemNames);
			}
		}
	}
	pView->UnlockSceneAtom();

	// List widget showing current solar system names.
	// This is only needed when the solar system is being named.
	QLabel* lbnames = 0;
	QListWidget* lwnames = 0;
	if(!hasSystemName && !systemNames.isEmpty())
	{
		lbnames = new QLabel(this);
		lbnames->setText(tr("Solar System Names In This Sector"));

		lwnames = new QListWidget(this);
		lwnames->setEditTriggers(QAbstractItemView::NoEditTriggers);
		lwnames->setSelectionMode(QAbstractItemView::NoSelection);
		lwnames->setSortingEnabled(true);
		for(QStringList::const_iterator ni = systemNames.constBegin(); ni != systemNames.constEnd(); ni++)
			new QListWidgetItem(icon, *ni, lwnames);
		lwnames->setToolTip(tr("<html>Names of solar systems in the current sector. "
					"Solar system names must be unique within a sector.</html>"));
	}

	// Create city name controls.
	QLabel* lbtextcity = new QLabel(this);
	lbtextcity->setText(tr("Enter a name for the city."));

	m_CityNameEdit = new QLineEdit(this);
	m_CityNameEdit->setFocus();
	connect(m_CityNameEdit, SIGNAL(textEdited(const QString&)), SLOT(CityNameChanged()));

	QPushButton* pbrandomcity = new QPushButton(this);
	pbrandomcity->setText(tr("Suggest a Name"));
	connect(pbrandomcity, SIGNAL(clicked(bool)), SLOT(RandomCityName()));

	// Create solar system name controls when needed.
	QLabel* lbtextsys;
	QPushButton* pbrandomsys;
	if(!hasSystemName)
	{
		// Solar system name needed.
		lbtextsys = new QLabel(this);
		lbtextsys->setText(tr("Enter a name for the solar system."));

		m_SystemNameEdit = new QLineEdit(this);
		m_SystemNameEdit->setToolTip(tr("<html>Solar system name. "
					"Enter a name that is different from the other solar systems in the sector.</html>"));
		connect(m_SystemNameEdit, SIGNAL(textEdited(const QString&)), SLOT(SystemNameChanged()));

		pbrandomsys = new QPushButton(this);
		pbrandomsys->setText(tr("Suggest a Name"));
		pbrandomsys->setToolTip(tr("Suggests a name for the solar system."));
		connect(pbrandomsys, SIGNAL(clicked(bool)), SLOT(RandomSystemName()));
	}
	else
	{
		lbtextsys = 0;
		m_SystemNameEdit = 0;
		pbrandomsys = 0;
	}

	// Create sector name controls when needed.
	QLabel* lbtextsect;
	QPushButton* pbrandomsect;
	if(!hasSectorName)
	{
		// Sector name needed.
		lbtextsect = new QLabel(this);
		lbtextsect->setText(tr("Enter a name for the sector."));

		m_SectorNameEdit = new QLineEdit(this);
		m_SectorNameEdit->setToolTip(tr("<html>Sector name. Enter a name for the sector.</html>"));
		connect(m_SectorNameEdit, SIGNAL(textEdited(const QString&)), SLOT(SectorNameChanged()));

		pbrandomsect = new QPushButton(this);
		pbrandomsect->setText(tr("Suggest a Name"));
		pbrandomsect->setToolTip(tr("Suggests a name for the sector."));
		connect(pbrandomsect, SIGNAL(clicked(bool)), SLOT(RandomSectorName()));
	}
	else
	{
		lbtextsect = 0;
		m_SectorNameEdit = 0;
		pbrandomsect = 0;
	}

	// Select a picture based on what is getting named.
	QPixmap img;
	if(hasSectorName && hasSystemName)
		img = QPixmap(AuCity::CityIcon(type)).scaled(80,80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	else if(hasSectorName)
		img = QPixmap(":data/Galaxy.png").scaled(100,100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	else
		img = QPixmap(":data/WormholeSpace.png").scaled(200,200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QLabel* lbimg = 0;
	if(!img.isNull())
	{
		lbimg = new QLabel(this);
		lbimg->setPixmap(img);
	}

	switch(type)
	{
	case CITY_City_Obsolete:
		m_Ok = new QPushButton(tr("Create Town Square"));
		break;
	case CITY_Capitol:
		m_Ok = new QPushButton(tr("Create City"));
		break;
	case CITY_MilitaryBase:
		m_Ok = new QPushButton(tr("Create Military Base"));
		break;
	}
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	connect(m_Ok, SIGNAL(clicked()), SLOT(OkPressed()));

	switch(type)
	{
	case CITY_City_Obsolete:
		m_CityNameEdit->setToolTip(tr("<html>City name. Enter a name for the new city.</html>"));
		pbrandomcity->setToolTip(tr("Suggests a name for the city."));
		if(hasSectorName && hasSystemName)
			m_Ok->setToolTip(tr("Finish creating the Town Square with the city name entered."));
		else
			m_Ok->setToolTip(tr("Finish creating the Town Square with the names entered."));
		break;
	case CITY_Capitol:
		m_CityNameEdit->setToolTip(tr("<html>City name. Enter a name for the new city.</html>"));
		pbrandomcity->setToolTip(tr("Suggests a name for the city."));
		if(hasSectorName && hasSystemName)
			m_Ok->setToolTip(tr("Finish creating the capitol building with the city name entered."));
		else
			m_Ok->setToolTip(tr("Finish creating the capitol building with the names entered."));
		break;
	case CITY_MilitaryBase:
		m_CityNameEdit->setToolTip(tr("<html>Military base name. Enter a name for the new military base.</html>"));
		pbrandomcity->setToolTip(tr("Suggests a name for the military base."));
		if(hasSectorName && hasSystemName)
			m_Ok->setToolTip(tr("Finish creating the military headquarters the military base name entered."));
		else
			m_Ok->setToolTip(tr("Finish creating the military headquarters the names entered."));
		break;
	}

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbtextcity, row,0, 1,2);
	row++;
	lay->addWidget(m_CityNameEdit, row,0, 1,1);
	lay->addWidget(pbrandomcity, row,1, 1,1);
	row++;
	if(m_SystemNameEdit)
	{
		lay->addWidget(lbtextsys, row,0, 1,2);
		row++;
		lay->addWidget(m_SystemNameEdit, row,0, 1,1);
		lay->addWidget(pbrandomsys, row,1, 1,1);
		row++;
	}
	if(m_SectorNameEdit)
	{
		lay->addWidget(lbtextsect, row,0, 1,2);
		row++;
		lay->addWidget(m_SectorNameEdit, row,0, 1,1);
		lay->addWidget(pbrandomsect, row,1, 1,1);
		row++;
	}
	lay->addWidget(m_Ok, row,0, 1,2);
	row++;
	if(lbnames && lwnames)
	{
		// Show list of reserved names only when there is a list of reserved names.
		lay->addWidget(lbnames, row,0, 1,2);
		row++;
		lay->addWidget(lwnames, row,0, 1,2);
		row++;
	}
	else
	{
		lay->setRowStretch(row, 1);
		row++;
	}
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

AuNewCityName::~AuNewCityName()
{
}

// returns enabled state of the ok button
void AuNewCityName::EnableControls()
{
	bool okenabled = IsCityNameOk();
	if(okenabled && m_SectorNameEdit && !IsSectorNameOk())
		okenabled = false;
	if(okenabled && m_SystemNameEdit && !IsSystemNameOk())
		okenabled = false;
	m_Ok->setEnabled(okenabled);
}

bool AuNewCityName::IsCityNameOk() const
{
	Q_ASSERT(m_CityNameEdit);

	QString name = m_CityNameEdit->text().simplified();
	if(!IsNameValid(name))
		return false;						// Name must not be invalid.
	return true;
}

bool AuNewCityName::IsSectorNameOk() const
{
	Q_ASSERT(m_SectorNameEdit);

	QString name = m_SectorNameEdit->text().simplified();
	if(!IsNameValid(name))
		return false;						// Name must not be invalid.
	return true;
}

bool AuNewCityName::IsSystemNameOk() const
{
	Q_ASSERT(m_SystemNameEdit);

	// Get a list of the current solar system names. This can change constantly so we get the list every time we need to know.
	QStringList systemNames;					// List of solar system names in the sector.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		const DaSolarSystem* system = person->FindParentSolarSystem();
		if(system)
		{
			const DaSector* sector = system->FindParentSector();
			if(sector)
				sector->ListSolarSystemNames(systemNames);
		}
	}
	pView->UnlockSceneAtom();

	QString name = m_SystemNameEdit->text().simplified();
	if(!IsNameValid(name))
		return false;						// Name must not be invalid.
	else if(systemNames.contains(name))
		return false;						// Name must not be in the list of reserved names.
	return true;
}

void AuNewCityName::CityNameChanged()
{
	if(m_SectorNameEdit || m_SystemNameEdit)
	{
		if(IsCityNameOk())
		{
			// Cascade to solar system and sector.
			if(m_SectorNameEdit)
				m_SectorNameEdit->setText(m_CityNameEdit->text());
			if(m_SystemNameEdit)
				m_SystemNameEdit->setText(m_CityNameEdit->text());
		}
	}

	EnableControls();
}

void AuNewCityName::SectorNameChanged()
{
	EnableControls();
}

void AuNewCityName::SystemNameChanged()
{
	if(m_SectorNameEdit)
	{
		if(IsSystemNameOk())
		{
			// Cascade to solar system and sector.
			if(m_SectorNameEdit)
				m_SectorNameEdit->setText(m_SystemNameEdit->text());
		}
	}

	EnableControls();
}

void AuNewCityName::RandomCityName()
{
	// Generate random city names until we get one that's ok.
	do
	{
		QString name = g_Au.Dice().RandomCityName();
		m_CityNameEdit->setText(name);
	}
	while(!IsCityNameOk());

	// Cascade to solar system and sector.
	if(m_SectorNameEdit)
		m_SectorNameEdit->setText(m_CityNameEdit->text());
	if(m_SystemNameEdit)
		m_SystemNameEdit->setText(m_CityNameEdit->text());

	EnableControls();
}

void AuNewCityName::RandomSectorName()
{
	// Generate random city names until we get one that's ok.
	do
	{
		QString name = g_Au.Dice().RandomCityName();
		m_SectorNameEdit->setText(name);
	}
	while(!IsSectorNameOk());

	EnableControls();
}

void AuNewCityName::RandomSystemName()
{
	// Generate random city names until we get one that's ok.
	do
	{
		QString name = g_Au.Dice().RandomCityName();
		m_SystemNameEdit->setText(name);
	}
	while(!IsSystemNameOk());

	// Cascade to sector.
	if(m_SectorNameEdit)
		m_SectorNameEdit->setText(m_SystemNameEdit->text());

	EnableControls();
}

void AuNewCityName::OkPressed()
{
	// Make a final check for valid name(s).
	if(!IsCityNameOk())
		return;
	if(m_SectorNameEdit && !IsSectorNameOk())
		return;
	if(m_SystemNameEdit && !IsSystemNameOk())
		return;

	m_CityName = m_CityNameEdit->text().simplified();
	if(m_SectorNameEdit)
		m_SectorName = m_SectorNameEdit->text().simplified();
	if(m_SystemNameEdit)
		m_SystemName = m_SystemNameEdit->text().simplified();
	accept();
}
