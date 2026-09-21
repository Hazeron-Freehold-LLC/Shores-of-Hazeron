//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augotogalaxyloc.cpp,v 1.3 2022/02/19 18:10:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augotogalaxyloc.h"
//
#include "auglobal.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuGoToGalaxyLoc::AuGoToGalaxyLoc(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Choose Galaxy Destination"));
	setAuDlgIcon(QIcon(":data/RangeGalaxy.png"));
	int lastG = g_Au.Settings().readNumEntry("Widget/GoToGalaxyLoc/LastGalaxy",-1);
	double lastX = g_Au.Settings().readNumEntry("Widget/GoToGalaxyLoc/LastGalaxyLocX", 0);
	double lastY = g_Au.Settings().readNumEntry("Widget/GoToGalaxyLoc/LastGalaxyLocY", 0);
	double lastZ = g_Au.Settings().readNumEntry("Widget/GoToGalaxyLoc/LastGalaxyLocZ", 0);

	m_Galaxies = new QComboBox(this);
	for(int g=AndromedaRising; g<GALAXY_Count; g++)
		m_Galaxies->addItem(GalaxyName((GALAXY)g), g);
	int curidx = m_Galaxies->findData(lastG);
	m_Galaxies->setCurrentIndex(curidx);

	QLabel* lb = new QLabel(tr("Enter galaxy coordinate, in pc."), this);

	QString lastGalaxyLoc = QString("%1, %2, %3").arg(lastX, 0,'f',3).arg(lastY, 0,'f',3).arg(lastZ, 0,'f',3);
	m_GalaxyCoord = new QLineEdit(lastGalaxyLoc, this);

	QPushButton* ok = new QPushButton(tr("Ok"), this);
	ok->setDefault(true);
	ok->setAutoDefault(true);
	connect(ok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(ok);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_Galaxies);
	lay->addWidget(lb);
	lay->addWidget(m_GalaxyCoord);
	lay->addLayout(blay);
	setAuDlgLayout(lay);
}

AuGoToGalaxyLoc::~AuGoToGalaxyLoc()
{
}

void AuGoToGalaxyLoc::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuGoToGalaxyLoc::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GoToGalaxyLoc/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GoToGalaxyLoc/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/GoToGalaxyLoc/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/GoToGalaxyLoc/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuGoToGalaxyLoc::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GoToGalaxyLoc/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GoToGalaxyLoc/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/GoToGalaxyLoc/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/GoToGalaxyLoc/SizeY",sz.height());
}

AuPoint AuGoToGalaxyLoc::Coordinate() const
{
	AuPoint coord(m_GalaxyCoord->text().trimmed());
	g_Au.Settings().writeEntry("Widget/GoToGalaxyLoc/LastGalaxyLocX", coord.x);
	g_Au.Settings().writeEntry("Widget/GoToGalaxyLoc/LastGalaxyLocY", coord.y);
	g_Au.Settings().writeEntry("Widget/GoToGalaxyLoc/LastGalaxyLocZ", coord.z);
	return coord;
}

GALAXY AuGoToGalaxyLoc::Galaxy() const
{
	GALAXY galaxy = (GALAXY)m_Galaxies->itemData(m_Galaxies->currentIndex()).toInt();
	g_Au.Settings().writeEntry("Widget/GoToGalaxyLoc/LastGalaxy", galaxy);
	return galaxy;
}
