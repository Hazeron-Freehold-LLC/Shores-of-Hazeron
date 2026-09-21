//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscpreviewinit.cpp,v 1.3 2022/02/19 18:25:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscpreviewinit.h"
//
#include "auglobal.h"
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>

AuScPreviewInit::AuScPreviewInit(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Spacecraft Preview Setup"));
	setAuDlgIcon(QIcon(":data/MsgSolicitSales.png"));
	quint32 genSeed = g_Au.Settings().readNumEntry("Widget/SpacecraftPreviewInit/GenSeed",32);
	m_PreviewType = 1;

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

	QPushButton* pbrandom = new QPushButton(tr("Random"), this);
	pbrandom->setToolTip(tr("<html>"
				"Selects a world number at random. "
				"</html>"));
	connect(pbrandom, SIGNAL(clicked()), SLOT(SetRandom()));

	QPushButton* pbfrigid = new QPushButton(tr("Frigid World"), this);
	pbfrigid->setToolTip(tr("<html>"
				"Location will be in orbit over a frigid world. "
				"The sun is a G2 V main sequence star, like the sun of Earth. "
				"</html>"));
	connect(pbfrigid, SIGNAL(clicked()), SLOT(OkFrigid()));

	QPushButton* pbinferno = new QPushButton(tr("Inferno World"), this);
	pbinferno->setToolTip(tr("<html>"
				"Location will be in orbit over an inferno world. "
				"The sun is a G2 V main sequence star, like the sun of Earth. "
				"</html>"));
	connect(pbinferno, SIGNAL(clicked()), SLOT(OkInferno()));

	QPushButton* pbrw = new QPushButton(tr("Ringworld"), this);
	pbrw->setToolTip(tr("<html>"
				"Location will be in orbit over a ringworld. "
				"The sun is a G2 V main sequence star, like the sun of Earth. "
				"</html>"));
	connect(pbrw, SIGNAL(clicked()), SLOT(OkRingworld()));

	QPushButton* pbgg = new QPushButton(tr("Gas Giant"), this);
	pbgg->setToolTip(tr("<html>"
				"Location will be in orbit over a gas giant. "
				"The sun is a G2 V main sequence star, like the sun of Earth. "
				"</html>"));
	connect(pbgg, SIGNAL(clicked()), SLOT(OkGasGiant()));

	QPushButton* pbmoon = new QPushButton(tr("Moon"), this);
	pbmoon->setToolTip(tr("<html>"
				"Location will be in orbit over a moon of a habitable world. "
				"The sun is a G2 V main sequence star, like the sun of Earth. "
				"</html>"));
	connect(pbmoon, SIGNAL(clicked()), SLOT(OkMoon()));

	QPushButton* pbworld = new QPushButton(tr("Habitable World"), this);
	pbworld->setToolTip(tr("<html>"
				"Location will be in orbit over a habitable world. "
				"It could be a moon of a gas giant. "
				"The sun is a G2 V main sequence star, like the sun of Earth. "
				"</html>"));
	connect(pbworld, SIGNAL(clicked()), SLOT(OkWorld()));

	QPushButton* pbsun = new QPushButton(tr("Sun"), this);
	pbsun->setToolTip(tr("<html>"
				"Location will be in orbit over a sun, at the altitude of its habitable zone. "
				"The sun is a G2 V main sequence star, like the sun of Earth. "
				"</html>"));
	connect(pbsun, SIGNAL(clicked()), SLOT(OkSun()));

	QPushButton* pbspace = new QPushButton(tr("Empty Space"), this);
	pbspace->setToolTip(tr("<html>"
				"Location will be in empty space. "
				"</html>"));
	pbspace->setDefault(true);
	pbspace->setAutoDefault(true);
	connect(pbspace, SIGNAL(clicked()), SLOT(OkSpace()));

	QLabel* lbenv = new QLabel(tr("<html>"
				"Choose an environment to preview the spacecraft design. "
				"<br>"
				"%1 Use <i>Preview Settings</i> to adjust lighting. "
				"</html>").arg(g_Au.MiddleDotSymbol()));
	lbenv->setWordWrap(true);

	QHBoxLayout* glay = new QHBoxLayout;
	glay->addWidget(lbgen);
	glay->addWidget(legen);
	glay->addWidget(pbrandom);

	QGridLayout* blay = new QGridLayout;
	int row=0;
	blay->addWidget(lbenv, row,0, 1,6);
	row++;
	blay->addWidget(pbgg, row,0, 1,2);
	blay->addWidget(pbfrigid, row,2, 1,2);
	blay->addWidget(pbinferno, row,4, 1,2);
	row++;
	blay->addWidget(pbrw, row,0, 1,2);
	blay->addWidget(pbmoon, row,2, 1,2);
	blay->addWidget(pbworld, row,4, 1,2);
	row++;
	blay->addWidget(pbsun, row,0, 1,3);
	blay->addWidget(pbspace, row,3, 1,3);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(glay);
	lay->addStretch();
	lay->addLayout(blay);
	setAuDlgLayout(lay);
}

AuScPreviewInit::~AuScPreviewInit()
{
}

void AuScPreviewInit::setVisible(bool visible)
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

void AuScPreviewInit::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SpacecraftPreviewInit/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SpacecraftPreviewInit/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SpacecraftPreviewInit/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SpacecraftPreviewInit/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScPreviewInit::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SpacecraftPreviewInit/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SpacecraftPreviewInit/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SpacecraftPreviewInit/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SpacecraftPreviewInit/SizeY",sz.height());
	g_Au.Settings().writeEntry("Widget/SpacecraftPreviewInit/GenSeed",GenSeed());
}

quint32 AuScPreviewInit::GenSeed() const
{
	quint32 genSeed = legen->value();
	return genSeed;
}

void AuScPreviewInit::OkInferno()
{
	m_PreviewType = 5;
	accept();
}

void AuScPreviewInit::OkFrigid()
{
	m_PreviewType = 6;
	accept();
}

void AuScPreviewInit::OkGasGiant()
{
	m_PreviewType = 7;
	accept();
}

void AuScPreviewInit::OkRingworld()
{
	m_PreviewType = 8;
	accept();
}

void AuScPreviewInit::OkMoon()
{
	m_PreviewType = 4;
	accept();
}

void AuScPreviewInit::OkWorld()
{
	m_PreviewType = 3;
	accept();
}

void AuScPreviewInit::OkSun()
{
	m_PreviewType = 2;
	accept();
}

void AuScPreviewInit::OkSpace()
{
	m_PreviewType = 1;
	accept();
}

void AuScPreviewInit::SetRandom()
{
	legen->setValue(g_Au.Dice().Roll(1,9999));
}
