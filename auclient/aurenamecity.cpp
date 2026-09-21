//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurenamecity.cpp,v 1.6 2022/02/19 18:16:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aurenamecity.h"
//
#include "aucity.h"
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "dabd.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "daworld.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuRenameCity::AuRenameCity(CITYTYPE type, QWidget* parent) : AuDialog(parent)
{
	QIcon icon(AuCity::CityIcon(type));
	switch(type)
	{
	case CITY_City_Obsolete:
		setAuDlgTitle("Change City Name");
		break;
	case CITY_Capitol:
		setAuDlgTitle("Change City Name");
		break;
	case CITY_MilitaryBase:
		setAuDlgTitle("Change Military Base Name");
		break;
	}
	setAuDlgIcon(icon);

	// Get the current name of the city.
	QString cityName;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		DaWorld* planet = person->FindParentPlanetInEnv();
		if(planet)
		{
			if(!person->BdId().IsNull())
			{
				const DaBd* bd = planet->FindBd(person->BdId());
				if(bd)
					cityName = bd->CityName();
			}
		}
	}
	pView->UnlockSceneAtom();

	// Create city name controls.
	QLabel* lbtextcity = new QLabel(this);
	lbtextcity->setWordWrap(true);

	m_CityNameEdit = new QLineEdit(this);
	m_CityNameEdit->setFocus();
	connect(m_CityNameEdit, SIGNAL(textEdited(const QString&)), SLOT(CityNameChanged()));

	QPushButton* pbrandomcity = new QPushButton(this);
	pbrandomcity->setText(tr("Suggest a Name"));
	connect(pbrandomcity, SIGNAL(clicked(bool)), SLOT(RandomCityName()));

	switch(type)
	{
	case CITY_City_Obsolete:
		lbtextcity->setText(tr("Enter a new name for the old city of <i>%1</i>.").arg(cityName));
		m_CityNameEdit->setToolTip(tr("<html><b>City Name</b><br>"
					"Enter a new name for the old city of <i>%1</i>.</html>").arg(cityName));
		pbrandomcity->setToolTip(tr("Suggests a name for the old city."));
		m_Ok = new QPushButton(tr("Rename Old City"));
		m_Ok->setToolTip(tr("Changes the name of the old city to the name entered."));
		break;
	case CITY_Capitol:
		lbtextcity->setText(tr("Enter a new name for the modern city of <i>%1</i>.").arg(cityName));
		m_CityNameEdit->setToolTip(tr("<html><b>City Name</b><br>"
					"Enter a new name for the modern city of <i>%1</i>.</html>").arg(cityName));
		pbrandomcity->setToolTip(tr("Suggests a name for the modern city."));
		m_Ok = new QPushButton(tr("Rename Modern City"));
		m_Ok->setToolTip(tr("Changes the name of the modern city to the name entered."));
		break;
	case CITY_MilitaryBase:
		lbtextcity->setText(tr("Enter a new name for the military base of <i>%1</i>.").arg(cityName));
		m_CityNameEdit->setToolTip(tr("<html><b>Military Base Name</b><br>"
					"Enter a new name for the military base of <i>%1</i>.</html>").arg(cityName));
		pbrandomcity->setToolTip(tr("Suggests a name for the military base."));
		m_Ok = new QPushButton(tr("Rename Military Base"));
		m_Ok->setToolTip(tr("Changes the name of the military base to the name entered."));
		break;
	}

	QLabel* lbimg = 0;
	QPixmap img = QPixmap(AuCity::CityIcon(type)).scaled(80,80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	if(!img.isNull())
	{
		lbimg = new QLabel(this);
		lbimg->setPixmap(img);
	}

	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	connect(m_Ok, SIGNAL(clicked()), SLOT(OkPressed()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbtextcity, row,0, 1,2);
	row++;
	lay->addWidget(m_CityNameEdit, row,0, 1,1);
	lay->addWidget(pbrandomcity, row,1, 1,1);
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

AuRenameCity::~AuRenameCity()
{
}

// returns enabled state of the ok button
void AuRenameCity::EnableControls()
{
	bool okenabled = IsCityNameOk();
	m_Ok->setEnabled(okenabled);
}

bool AuRenameCity::IsCityNameOk() const
{
	Q_ASSERT(m_CityNameEdit);

	QString name = m_CityNameEdit->text().simplified();
	if(!IsNameValid(name))
		return false;						// Name must not be invalid.
	return true;
}

void AuRenameCity::CityNameChanged()
{
	EnableControls();
}

void AuRenameCity::RandomCityName()
{
	// Generate random city names until we get one that's ok.
	do
	{
		QString name = g_Au.Dice().RandomCityName();
		m_CityNameEdit->setText(name);
	}
	while(!IsCityNameOk());

	EnableControls();
}

void AuRenameCity::OkPressed()
{
	// Make a final check for valid name(s).
	if(!IsCityNameOk())
		return;

	m_CityName = m_CityNameEdit->text().simplified();
	auApp->CityRename(m_CityName);
	accept();
}

