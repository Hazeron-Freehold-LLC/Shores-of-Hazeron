//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auflowerpotsettings.cpp,v 1.3 2022/02/19 18:10:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auflowerpotsettings.h"
//
#include "auglobal.h"
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>

AuFlowerPotSettings::AuFlowerPotSettings(int size, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Flower Pot Settings"));
	setAuDlgIcon(QIcon(":data/c_FlowerPotLarge.png"));

	QLabel* lbSize = new QLabel(tr("Size of Potted Plant"));

	m_SizeSlider = new QSlider(Qt::Horizontal, this);
	m_SizeSlider->setRange(0, 255);
	m_SizeSlider->setSingleStep(1);
	m_SizeSlider->setPageStep(5);
	m_SizeSlider->setValue(size);
	m_SizeSlider->setToolTip(tr("<html>"
			"Adjusts the size of the plant in the pot."
			"</html>"));

	QPushButton* pbok = new QPushButton("Ok");
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("Accepts the changes to the flower pot."));
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbSize, row,0);
	row++;
	lay->addWidget(m_SizeSlider, row,0);
	row++;
	lay->addWidget(pbok, row,0);
	row++;
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuFlowerPotSettings::~AuFlowerPotSettings()
{
}

void AuFlowerPotSettings::setVisible(bool visible)
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

void AuFlowerPotSettings::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/FlowerPotSettings/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/FlowerPotSettings/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/FlowerPotSettings/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/FlowerPotSettings/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuFlowerPotSettings::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/FlowerPotSettings/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/FlowerPotSettings/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/FlowerPotSettings/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/FlowerPotSettings/SizeY",sz.height());
}

int AuFlowerPotSettings::PlantSize() const
{
	return m_SizeSlider->value();
}
