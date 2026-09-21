//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdpreviewinit.cpp,v 1.4 2022/02/19 18:25:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubdpreviewinit.h"
//
#include "auglobal.h"
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>

AuBdPreviewInit::AuBdPreviewInit(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Building Preview Setup"));
	setAuDlgIcon(QIcon(":data/MsgSolicitSales.png"));
	quint32 genSeed = g_Au.Settings().readNumEntry("Widget/BuildingPreviewInit/GenSeed",32);
	m_PreviewType = 3;

	/*
	QLabel* lbsun = new QLabel(tr("Sun"), this);

	QComboBox* spectype = new QComboBox(this);
	spectype->addItem(tr("O"), DaStar::TYPE_O);
	spectype->addItem(tr("B"), DaStar::TYPE_B);
	spectype->addItem(tr("A"), DaStar::TYPE_A);
	spectype->addItem(tr("F"), DaStar::TYPE_F);
	spectype->addItem(tr("G"), DaStar::TYPE_G);
	spectype->addItem(tr("K"), DaStar::TYPE_K);
	spectype->addItem(tr("M"), DaStar::TYPE_M);

	QComboBox* specdec = new QComboBox(this);
	specdec->addItem(tr("0"), 0);
	specdec->addItem(tr("1"), 1);
	specdec->addItem(tr("2"), 2);
	specdec->addItem(tr("3"), 3);
	specdec->addItem(tr("4"), 4);
	specdec->addItem(tr("5"), 5);
	specdec->addItem(tr("6"), 6);
	specdec->addItem(tr("7"), 7);
	specdec->addItem(tr("8"), 8);
	specdec->addItem(tr("9"), 9);

	QComboBox* specsize = new QComboBox(this);
	specsize->addItem(tr("Ia"), DaStar::SIZE_Ia);
	specsize->addItem(tr("Ib"), DaStar::SIZE_Ib);
	specsize->addItem(tr("II"), DaStar::SIZE_II);
	specsize->addItem(tr("III"), DaStar::SIZE_III);
	specsize->addItem(tr("IV"), DaStar::SIZE_IV);
	specsize->addItem(tr("V"), DaStar::SIZE_V);
	specsize->addItem(tr("VI"), DaStar::SIZE_VI);
	specsize->addItem(tr("D"), DaStar::SIZE_D);

	QHBoxLayout* slay = new QHBoxLayout;
	slay->addWidget(lbsun);
	slay->addWidget(spectype);
	slay->addWidget(specdec);
	slay->addWidget(specsize);
	slay->addStretch();
	*/

	QLabel* lbgen = new QLabel(tr("World Number"), this);

	legen = new QSpinBox(this);
	legen->setToolTip(tr("<html>"
				"Choose the number of the world to generate for the preview. "
				"Worlds 1 through 9999 can be used. "
				"</html>"));
	legen->setRange(1,9999);
	legen->setValue(genSeed);

	QPushButton* pbriver = new QPushButton(tr("River"), this);
	pbriver->setToolTip(tr("<html>"
				"Selects a world number that produces a river land area on a habitable world. "
				"</html>"));
	connect(pbriver, SIGNAL(clicked()), SLOT(SetRiver()));

	QPushButton* pbcoast = new QPushButton(tr("Coast"), this);
	pbcoast->setToolTip(tr("<html>"
				"Selects a world number that produces a coastal land area on a habitable world. "
				"</html>"));
	connect(pbcoast, SIGNAL(clicked()), SLOT(SetCoast()));

	QPushButton* pbhills = new QPushButton(tr("Hills"), this);
	pbhills->setToolTip(tr("<html>"
				"Selects a world number that produces a hilly land area on a habitable world. "
				"</html>"));
	connect(pbhills, SIGNAL(clicked()), SLOT(SetHills()));

	QPushButton* pbplains = new QPushButton(tr("Plains"), this);
	pbplains->setToolTip(tr("<html>"
				"Selects a world number that produces a flat land area on a habitable world. "
				"</html>"));
	connect(pbplains, SIGNAL(clicked()), SLOT(SetPlains()));

	QPushButton* pbrandom = new QPushButton(tr("Random"), this);
	pbrandom->setToolTip(tr("<html>"
				"Selects a world number at random. "
				"</html>"));
	connect(pbrandom, SIGNAL(clicked()), SLOT(SetRandom()));

	QPushButton* pbfrigid = new QPushButton(tr("Frigid World"), this);
	pbfrigid->setToolTip(tr("<html>"
				"Location will be on a frigid world. "
				"The sun is a G2 V main sequence star, like the sun of Earth. "
				"</html>"));
	connect(pbfrigid, SIGNAL(clicked()), SLOT(OkFrigid()));

	QPushButton* pbinferno = new QPushButton(tr("Inferno World"), this);
	pbinferno->setToolTip(tr("<html>"
				"Location will be on an inferno world. "
				"The sun is a G2 V main sequence star, like the sun of Earth. "
				"</html>"));
	connect(pbinferno, SIGNAL(clicked()), SLOT(OkInferno()));

	QPushButton* pbrw = new QPushButton(tr("Ringworld"), this);
	pbrw->setToolTip(tr("<html>"
				"Location will be on a ringworld. "
				"The sun is a G2 V main sequence star, like the sun of Earth. "
				"</html>"));
	connect(pbrw, SIGNAL(clicked()), SLOT(OkRingworld()));

	QPushButton* pbgg = new QPushButton(tr("Gas Giant"), this);
	pbgg->setToolTip(tr("<html>"
				"Location will be at the core of a gas giant. "
				"The sun is a G2 V main sequence star, like the sun of Earth. "
				"</html>"));
	connect(pbgg, SIGNAL(clicked()), SLOT(OkGasGiant()));

	QPushButton* pbmoon = new QPushButton(tr("Moon"), this);
	pbmoon->setToolTip(tr("<html>"
				"Location will be on a moon of a habitable world. "
				"The sun is a G2 V main sequence star, like the sun of Earth. "
				"</html>"));
	connect(pbmoon, SIGNAL(clicked()), SLOT(OkMoon()));

	QPushButton* pbworld = new QPushButton(tr("Habitable World Land"), this);
	pbworld->setToolTip(tr("<html>"
				"Location will be on land on a habitable world. "
				"It could be a moon of a gas giant. "
				"The sun is a G2 V main sequence star, like the sun of Earth. "
				"</html>"));
	pbworld->setDefault(true);
	pbworld->setAutoDefault(true);
	connect(pbworld, SIGNAL(clicked()), SLOT(OkWorld()));

	QPushButton* pbsea = new QPushButton(tr("Habitable World Sea"), this);
	pbsea->setToolTip(tr("<html>"
				"Location will be under the sea on a habitable world. "
				"It could be a moon of a gas giant. "
				"The sun is a G2 V main sequence star, like the sun of Earth. "
				"</html>"));
	connect(pbsea, SIGNAL(clicked()), SLOT(OkSea()));

	QLabel* lbenv = new QLabel(tr("<html>"
				"Choose an environment to preview the building design. "
				"<br>"
				"%1 Use <i>Preview Settings</i> to adjust lighting. "
				"<br>"
				"%1 Use <i>Construct Building</i> to place one or more buildings on the terrain. "
				"</html>").arg(g_Au.MiddleDotSymbol()));
	lbenv->setWordWrap(true);

	QHBoxLayout* glay = new QHBoxLayout;
	glay->addWidget(lbgen);
	glay->addWidget(legen);
	glay->addWidget(pbcoast);
	glay->addWidget(pbriver);
	glay->addWidget(pbhills);
	glay->addWidget(pbplains);
	glay->addWidget(pbrandom);

	QGridLayout* blay = new QGridLayout;
	int row=0;
	blay->addWidget(lbenv, row,0, 1,6);
	row++;
	blay->addWidget(pbgg, row,0, 1,2);
	blay->addWidget(pbfrigid, row,2, 1,2);
	blay->addWidget(pbinferno, row,4, 1,2);
	row++;
	blay->addWidget(pbrw, row,0, 1,3);
	blay->addWidget(pbmoon, row,3, 1,3);
	row++;
	blay->addWidget(pbsea, row,0, 1,3);
	blay->addWidget(pbworld, row,3, 1,3);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(glay);
	lay->addStretch();
	lay->addLayout(blay);
	setAuDlgLayout(lay);
}

AuBdPreviewInit::~AuBdPreviewInit()
{
}

void AuBdPreviewInit::setVisible(bool visible)
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

void AuBdPreviewInit::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/BuildingPreviewInit/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/BuildingPreviewInit/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/BuildingPreviewInit/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/BuildingPreviewInit/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuBdPreviewInit::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/BuildingPreviewInit/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/BuildingPreviewInit/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/BuildingPreviewInit/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/BuildingPreviewInit/SizeY",sz.height());
	g_Au.Settings().writeEntry("Widget/BuildingPreviewInit/GenSeed",GenSeed());
}

quint32 AuBdPreviewInit::GenSeed() const
{
	quint32 genSeed = legen->value();
	return genSeed;
}

void AuBdPreviewInit::OkInferno()
{
	m_PreviewType = 5;
	accept();
}

void AuBdPreviewInit::OkFrigid()
{
	m_PreviewType = 6;
	accept();
}

void AuBdPreviewInit::OkGasGiant()
{
	m_PreviewType = 7;
	accept();
}

void AuBdPreviewInit::OkRingworld()
{
	m_PreviewType = 8;
	accept();
}

void AuBdPreviewInit::OkMoon()
{
	m_PreviewType = 4;
	accept();
}

void AuBdPreviewInit::OkSea()
{
	m_PreviewType = 9;
	accept();
}

void AuBdPreviewInit::OkWorld()
{
	m_PreviewType = 3;
	accept();
}

void AuBdPreviewInit::SetHills()
{
	legen->setValue(3);
}

void AuBdPreviewInit::SetPlains()
{
	legen->setValue(8);
}

void AuBdPreviewInit::SetRandom()
{
	legen->setValue(g_Au.Dice().Roll(1,9999));
}

void AuBdPreviewInit::SetCoast()
{
	legen->setValue(5);
}

void AuBdPreviewInit::SetRiver()
{
	legen->setValue(10);
}

