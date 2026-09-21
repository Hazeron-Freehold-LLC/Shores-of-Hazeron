//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austarmapquery.cpp,v 1.3 2022/02/19 18:17:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austarmapquery.h"
//
#include "auglobal.h"
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

AuStarMapQuery::AuStarMapQuery(QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/RangeSector.png"));
	setAuDlgTitle(tr("Star Map Query"));

	int lastGalaxy = g_Au.Settings().readNumEntry("Widget/StarMapQuery/LastGalaxy", ShoresOfHazeron);
	m_CurRadio = g_Au.Settings().readNumEntry("Widget/StarMapQuery/LastQuery", 0);
	int lastRange = g_Au.Settings().readNumEntry("Widget/StarMapQuery/LastRange", 1);
	bool lastClear = g_Au.Settings().readBoolEntry("Widget/StarMapQuery/LastClear", false);
	bool last3x = g_Au.Settings().readBoolEntry("Widget/StarMapQuery/Last3x", true);

	QLabel* lbinfo = new QLabel(tr("<html>"
				"Choose which portion of the star map to query. "
				"Information known to your empire is retrieved from the master star map. "
				"</html>"), this);
	lbinfo->setWordWrap(true);

	m_Sector = new QRadioButton(tr("Sector"), this);
	m_Sector->setChecked(m_CurRadio == 0);
	m_Sector->setToolTip(tr("<html>"
				"Requests star map information within a specified range, centered on your current sector. "
				"</html>"));
	connect(m_Sector, SIGNAL(clicked()), SLOT(SectorRadio()));

	m_Galaxy = new QRadioButton(tr("Galaxy"), this);
	m_Galaxy->setChecked(m_CurRadio == 1);
	m_Galaxy->setToolTip(tr("<html>"
				"Requests star map information for a specific galaxy. "
				"This could take a long time, in a galaxy where your empire explored many sectors. "
				"</html>"));
	connect(m_Galaxy, SIGNAL(clicked()), SLOT(GalaxyRadio()));

	m_All = new QRadioButton(tr("Universe"), this);
	m_All->setChecked(m_CurRadio == 2);
	m_All->setToolTip(tr("<html>"
				"Requests all known star map information. "
				"This could take a very long time, in an empire that explored many sectors of the universe. "
				"</html>"));
	connect(m_All, SIGNAL(clicked()), SLOT(AllRadio()));

	m_Range = new QSpinBox(this);
	m_Range->setRange(0, 1000);
	m_Range->setValue(lastRange);
	m_Range->setToolTip(tr("<html>"
				"Query range beyond your current sector, measured in sectors. "
				"</html>"));

	m_Galaxies = new QComboBox(this);
	for(int g=0; g<GALAXY_Count; g++)
		m_Galaxies->addItem(GalaxyName((GALAXY)g), g);
	int g = m_Galaxies->findData(lastGalaxy);
	m_Galaxies->setCurrentIndex(g);
	m_Galaxies->setToolTip(tr("<html>"
				"Information about the selected galaxy is queried. "
				"</html>"));

	m_ClearMap = new QCheckBox(tr("Clear Star Map"), this);
	m_ClearMap->setChecked(lastClear);
	m_ClearMap->setToolTip(tr("<html>"
				"Star map is cleared before requesting new information. "
				"The resulting star map will contain only the newly queried information. "
				"</html>"));

	m_3xSectors = new QCheckBox(tr("Include Unexplored Nearby Sectors"), this);
	m_3xSectors->setChecked(last3x);
	m_3xSectors->setToolTip(tr("<html>"
				"The resulting star map will contain any unexplored sectors in the 3x3x3 box of sectors around your current sector. "
				"</html>"));

	QPushButton* pbok = new QPushButton(tr("Query Star Map"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>Posts a query to the master star map for up-to-date information.</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(PostQuery()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbinfo, row,0, 1,2);
	row++;
	lay->addWidget(m_Sector, row,0, 1,2);
	row++;
	lay->addWidget(m_Range, row,1, 1,1);
	row++;
	lay->addWidget(m_Galaxy, row,0, 1,2);
	row++;
	lay->addWidget(m_Galaxies, row,1, 1,1);
	row++;
	lay->addWidget(m_All, row,0, 1,2);
	row++;
	lay->addWidget(m_ClearMap, row,0, 1,2);
	row++;
	lay->addWidget(m_3xSectors, row,0, 1,2);
	row++;
	lay->addLayout(blay, row,0, 1,2);
	lay->setColumnMinimumWidth(0, 20);
	setAuDlgLayout(lay);
}

AuStarMapQuery::~AuStarMapQuery()
{
}

void AuStarMapQuery::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
		EnableControls();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuStarMapQuery::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/StarMapQuery/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/StarMapQuery/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/StarMapQuery/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/StarMapQuery/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuStarMapQuery::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/StarMapQuery/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/StarMapQuery/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/StarMapQuery/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/StarMapQuery/SizeY",sz.height());
}

void AuStarMapQuery::PostQuery()
{
	g_Au.Settings().writeEntry("Widget/StarMapQuery/LastGalaxy", Galaxy());

	g_Au.Settings().writeEntry("Widget/StarMapQuery/LastQuery", Query());

	g_Au.Settings().writeEntry("Widget/StarMapQuery/LastRange", Range());

	g_Au.Settings().writeEntry("Widget/StarMapQuery/LastClear", ClearMap());

	g_Au.Settings().writeEntry("Widget/StarMapQuery/Last3x", Include3xSectors());

	accept();
}

void AuStarMapQuery::EnableControls()
{
	m_Range->setEnabled(Query() == 0);
	m_Galaxies->setEnabled(Query() == 1);
}

void AuStarMapQuery::AllRadio()
{
	m_CurRadio = 2;
	EnableControls();
}

void AuStarMapQuery::GalaxyRadio()
{
	m_CurRadio = 1;
	EnableControls();
}

void AuStarMapQuery::SectorRadio()
{
	m_CurRadio = 0;
	EnableControls();
}

int AuStarMapQuery::Query() const
{
	return m_CurRadio;
}

GALAXY AuStarMapQuery::Galaxy() const
{
	return (GALAXY)m_Galaxies->itemData(m_Galaxies->currentIndex()).toUInt();
}

int AuStarMapQuery::Range() const
{
	return m_Range->value();
}

bool AuStarMapQuery::ClearMap() const
{
	return m_ClearMap->isChecked();
}

bool AuStarMapQuery::Include3xSectors() const
{
	return m_3xSectors->isChecked();
}
