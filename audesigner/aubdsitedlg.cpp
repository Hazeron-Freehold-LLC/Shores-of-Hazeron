//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdsitedlg.cpp,v 1.3 2022/02/19 18:25:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubdsitedlg.h"
//
#include "aubddesign.h"
#include "auclientglobal.h"
#include "auscparts.h"
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

AuBdSiteDlg::AuBdSiteDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Building Site"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));
	m_Draw = 0;

	QGroupBox* drawGroup = new QGroupBox(tr("Site Properties"), this);
	{
		m_Pave = new QComboBox(this);
		m_Pave->setToolTip(tr("<html>"
					"Choose what texture to apply over the natural ground texture at the site. "
					"<br>"
					"<i>Field</i> sites create fields needed by farms, logging camps, orchards, parks and zoos. "
					"One field is created for the first %L1m%3 of field space. "
					"A field is added for every %L2m%3 of field space after that. "
					"</html>").arg(AuBdDesign::AreaFirstField()).arg(AuBdDesign::AreaAddField()).arg(g_Au.SquaredSymbol()));

		m_Grass = new QComboBox(this);
		m_Grass->setToolTip(tr("<html>"
					"Choose what to do with grass at the site. "
					"</html>"));

		m_Shrubs = new QComboBox(this);
		m_Shrubs->setToolTip(tr("<html>"
					"Choose what to do with naturally occurring shrubs at the site. "
					"</html>"));

		m_Trees = new QComboBox(this);
		m_Trees->setToolTip(tr("<html>"
					"Choose what to do with naturally occurring trees at the site. "
					"</html>"));

		m_Rocks = new QComboBox(this);
		m_Rocks->setToolTip(tr("<html>"
					"Choose what to do with rocks at the site. "
					"</html>"));

		QLabel* lbfig = new QLabel(this);
		lbfig->setPixmap(QPixmap(":data/ScFigSiteMesh.png"));
		lbfig->setAlignment(Qt::AlignCenter);

		m_Grade = new QComboBox(this);
		m_Grade->setToolTip(tr("<html>"
					"Choose how the terrain should be graded at the site. "
					"</html>"));

		m_Blend = new QComboBox(this);
		m_Blend->setToolTip(tr("<html>"
					"Choose how site grading should be blended with native terrain. "
					"<br>"
					"Red glow at each vertex determines the amount of influence the site has on the native terrain. "
					"Where red glow is 0, site grading has full effect. "
					"Where red glow is 255, site grading has no effect. "
					"In between, the terrain and the site result are blended. "
					"<br>"
					"%1 <b>Flat Blend</b> produces a flat slope transition. "
					"<br>"
					"%1 <b>Roll Blend</b> produces a rolling hill transition. "
					"</html>"));

		QPushButton* pbdraw = new QPushButton(tr("Draw Outline..."), this);
		pbdraw->setToolTip(tr("<html>"
					"Draws a site mesh outline using the configured settings. "
					"</html>"));
		connect(pbdraw, SIGNAL(clicked()), SLOT(DoDraw()));

		QVBoxLayout* glay = new QVBoxLayout;
		glay->addWidget(lbfig);
		glay->addWidget(m_Grade);
		glay->addWidget(m_Blend);
		glay->addWidget(m_Pave);
		glay->addWidget(m_Rocks);
		glay->addWidget(m_Grass);
		glay->addWidget(m_Shrubs);
		glay->addWidget(m_Trees);
		glay->addWidget(pbdraw);
		glay->setMargin(12);
		glay->setSpacing(2);
		drawGroup->setLayout(glay);
	}

	QPushButton* pblotres = new QPushButton(tr("Residential Lot"), this);
	pblotres->setToolTip(tr("<html>"
				"Creates site meshes for a residential home lot. "
				"<br>"
				"The building site will be level, cobblestone, no grass. "
				"<br>"
				"The yard will be level, blended to terrain at edges, natural ground, lawn. "
				"</html>"));
	connect(pblotres, SIGNAL(clicked()), SLOT(DoLotResidential()));

	QPushButton* pblotag = new QPushButton(tr("Agricultural Lot"), this);
	pblotag->setToolTip(tr("<html>"
				"Creates site meshes for an agricultural home lot. "
				"<br>"
				"The building site will be level, cobblestone, no grass. "
				"<br>"
				"The yard will be level, blended to 50% of terrain at edges, natural ground, lawn. "
				"<br>"
				"Fields start where yard ends, are level 50% terrain, blended to terrain at the edges, field, no grass. "
				"</html>"));
	connect(pblotag, SIGNAL(clicked()), SLOT(DoLotAgricultural()));

	QPushButton* pblotcom = new QPushButton(tr("Commercial Lot"), this);
	pblotcom->setToolTip(tr("<html>"
				"Creates site meshes for a commercial building lot. "
				"<br>"
				"The building site will be level, concrete, no grass. "
				"</html>"));
	connect(pblotcom, SIGNAL(clicked()), SLOT(DoLotCommercial()));

	QPushButton* pblotind = new QPushButton(tr("Industrial Lot"), this);
	pblotind->setToolTip(tr("<html>"
				"Creates site meshes for an industrial building lot. "
				"<br>"
				"The building site will be level, concrete, no grass. "
				"</html>"));
	connect(pblotind, SIGNAL(clicked()), SLOT(DoLotIndustrial()));

	QPushButton* pblotmine = new QPushButton(tr("Mine or Well Lot"), this);
	pblotmine->setToolTip(tr("<html>"
				"Creates site meshes for a mine or well lot. "
				"<br>"
				"The building site will be level, cobblestone, no grass. "
				"<br>"
				"The yard will be level, blended to terrain at edges, rock, no grass. "
				"</html>"));
	connect(pblotmine, SIGNAL(clicked()), SLOT(DoLotMine()));

	QPushButton* pblotshore = new QPushButton(tr("Shore Lot"), this);
	pblotshore->setToolTip(tr("<html>"
				"Creates site meshes for a building lot with a harbor. "
				"<br>"
				"The building site will be level, concrete, no grass. "
				"<br>"
				"The harbor will be cut to 10m depth, natural ground, no grass. "
				"The harbor will have a margin that is cut, blended to terrain, natural ground, no grass. "
				"</html>"));
	connect(pblotshore, SIGNAL(clicked()), SLOT(DoLotShore()));

	QGridLayout* blay = new QGridLayout;
	int row=0;
	blay->addWidget(pblotag, row,0);
	blay->addWidget(pblotres, row,1);
	row++;
	blay->addWidget(pblotcom, row,0);
	blay->addWidget(pblotind, row,1);
	row++;
	blay->addWidget(pblotmine, row,0);
	blay->addWidget(pblotshore, row,1);
	row++;
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(drawGroup);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuBdSiteDlg::~AuBdSiteDlg()
{
}

void AuBdSiteDlg::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
		ListBlends();
		ListGrades();
		ListGrass();
		ListPavement();
		ListRocks();
		ListShrubs();
		ListTrees();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuBdSiteDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/BuildingSiteDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/BuildingSiteDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/BuildingSiteDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/BuildingSiteDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuBdSiteDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/BuildingSiteDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/BuildingSiteDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/BuildingSiteDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/BuildingSiteDlg/SizeY",sz.height());

	// Settings
	g_Au.Settings().writeEntry("Widget/BuildingSiteDlg/Pavement", Pavement());
	g_Au.Settings().writeEntry("Widget/BuildingSiteDlg/Grade", Grade());
	g_Au.Settings().writeEntry("Widget/BuildingSiteDlg/Blend", Blend());
	g_Au.Settings().writeEntry("Widget/BuildingSiteDlg/Grass", Grass());
	g_Au.Settings().writeEntry("Widget/BuildingSiteDlg/Rocks", Rocks());
	g_Au.Settings().writeEntry("Widget/BuildingSiteDlg/Shrubs", Shrubs());
	g_Au.Settings().writeEntry("Widget/BuildingSiteDlg/Trees", Trees());
}

int AuBdSiteDlg::Pavement() const
{
	return m_Pave->itemData(m_Pave->currentIndex()).toInt();
}

void AuBdSiteDlg::ListPavement()
{
	m_Pave->addItem(QIcon(":data/ScHide.png"), tr("Natural Ground"), WORLDTEX_ConstructionZone);
	m_Pave->addItem(QIcon(":data/DirtSmooth.png"), tr("Dirt"), WORLDTEX_DirtSmooth);
	m_Pave->addItem(QIcon(":data/CobblestoneLight.png"), tr("Cobblestone"), WORLDTEX_CobblestoneLight);
	m_Pave->addItem(QIcon(":data/ConcreteHeavy.png"), tr("Concrete"), WORLDTEX_ConcreteHeavy);
	m_Pave->addItem(QIcon(":data/Tarmac.png"), tr("Asphalt"), WORLDTEX_Tarmac);
	m_Pave->addItem(QIcon(":data/DirtCrackedSmooth.png"), tr("Field"), WORLDTEX_DirtCrackedSmooth);
	m_Pave->addItem(QIcon(":data/RockTan.png"), tr("Rock"), WORLDTEX_RockTan);
	int lastVal = g_Au.Settings().readNumEntry("Widget/BuildingSiteDlg/Pavement", WORLDTEX_DirtSmooth);
	int idx = m_Pave->findData(lastVal);
	if(idx >= 0)
		m_Pave->setCurrentIndex(idx);
}

int AuBdSiteDlg::Grade() const
{
	return m_Grade->itemData(m_Grade->currentIndex()).toInt();
}

int AuBdSiteDlg::Blend() const
{
	return m_Blend->itemData(m_Blend->currentIndex()).toInt();
}

void AuBdSiteDlg::ListGrades()
{
	m_Grade->addItem(tr("Terrain is Not Changed"), AuScSite::Terrain);
	m_Grade->addItem(tr("Level Terrain to Mesh"), AuScSite::Level);
	m_Grade->addItem(tr("Cut Terrain Down to Mesh"), AuScSite::Cut);
	m_Grade->addItem(tr("Fill Terrain Up to Mesh"), AuScSite::Fill);
	int lastVal = g_Au.Settings().readNumEntry("Widget/BuildingSiteDlg/Grade", 1);
	int idx = m_Grade->findData(lastVal);
	if(idx >= 0)
		m_Grade->setCurrentIndex(idx);
}

void AuBdSiteDlg::ListBlends()
{
	m_Blend->addItem(tr("Flat Blend"), 0);
	m_Blend->addItem(tr("Roll Blend"), 1);
	int lastVal = g_Au.Settings().readNumEntry("Widget/BuildingSiteDlg/Blend", 1);
	int idx = m_Blend->findData(lastVal);
	if(idx >= 0)
		m_Blend->setCurrentIndex(idx);
}

int AuBdSiteDlg::Grass() const
{
	return m_Grass->itemData(m_Grass->currentIndex()).toInt();
}

int AuBdSiteDlg::Rocks() const
{
	return m_Rocks->itemData(m_Rocks->currentIndex()).toInt();
}

int AuBdSiteDlg::Shrubs() const
{
	return m_Shrubs->itemData(m_Shrubs->currentIndex()).toInt();
}

int AuBdSiteDlg::Trees() const
{
	return m_Trees->itemData(m_Trees->currentIndex()).toInt();
}

void AuBdSiteDlg::ListGrass()
{
	m_Grass->addItem(tr("Remove Grass"), 0);
	m_Grass->addItem(tr("Leave Grass Alone"), 1);
	m_Grass->addItem(tr("Mow Existing Grass"), 2);
	m_Grass->addItem(tr("Grassy Lawn"), 3);
	int lastVal = g_Au.Settings().readNumEntry("Widget/BuildingSiteDlg/Grass", 1);
	int idx = m_Grass->findData(lastVal);
	if(idx >= 0)
		m_Grass->setCurrentIndex(idx);
}

void AuBdSiteDlg::ListRocks()
{
	m_Rocks->addItem(tr("Remove Rocks"), 0);
	m_Rocks->addItem(tr("Leave Rocks Alone"), 1);
	int lastVal = g_Au.Settings().readNumEntry("Widget/BuildingSiteDlg/Rocks", 1);
	int idx = m_Rocks->findData(lastVal);
	if(idx >= 0)
		m_Rocks->setCurrentIndex(idx);
}

void AuBdSiteDlg::ListShrubs()
{
	m_Shrubs->addItem(tr("Remove Shrubs"), 0);
	m_Shrubs->addItem(tr("Leave Shrubs Alone"), 1);
	int lastVal = g_Au.Settings().readNumEntry("Widget/BuildingSiteDlg/Shrubs", 1);
	int idx = m_Shrubs->findData(lastVal);
	if(idx >= 0)
		m_Shrubs->setCurrentIndex(idx);
}

void AuBdSiteDlg::ListTrees()
{
	m_Trees->addItem(tr("Remove Trees"), 0);
	m_Trees->addItem(tr("Leave Trees Alone"), 1);
	int lastVal = g_Au.Settings().readNumEntry("Widget/BuildingSiteDlg/Trees", 1);
	int idx = m_Trees->findData(lastVal);
	if(idx >= 0)
		m_Trees->setCurrentIndex(idx);
}

void AuBdSiteDlg::DoDraw()
{
	m_Draw = 0;
	accept();
}

void AuBdSiteDlg::DoLotAgricultural()
{
	m_Draw = 1;
	accept();
}

void AuBdSiteDlg::DoLotCommercial()
{
	m_Draw = 2;
	accept();
}

void AuBdSiteDlg::DoLotIndustrial()
{
	m_Draw = 3;
	accept();
}

void AuBdSiteDlg::DoLotResidential()
{
	m_Draw = 4;
	accept();
}

void AuBdSiteDlg::DoLotShore()
{
	m_Draw = 5;
	accept();
}

void AuBdSiteDlg::DoLotMine()
{
	m_Draw = 6;
	accept();
}

