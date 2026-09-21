//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausitepropertiesdlg.cpp,v 1.4 2022/02/19 18:26:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausitepropertiesdlg.h"
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

AuSitePropertiesDlg::AuSitePropertiesDlg(quint8 grade, quint8 grass, quint8 pavement, quint8 rocks, quint8 shrubs, quint8 terBlend,
		quint8 trees, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Site Properties"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	m_Pave = new QComboBox(this);
	m_Pave->setToolTip(tr("<html>"
				"Choose what texture to apply over the natural ground texture at the site. "
				"<br>"
				"<i>Field</i> sites create fields needed by farms, logging camps, orchards, parks and zoos. "
				"One field is created for the first %L1m%3 of field space. "
				"A field is added for every %L2m%3 of field space after that. "
				"</html>").arg(AuBdDesign::AreaFirstField()).arg(AuBdDesign::AreaAddField()).arg(g_Au.SquaredSymbol()));
	ListPavement(pavement);

	m_Grass = new QComboBox(this);
	m_Grass->setToolTip(tr("<html>"
				"Choose what to do with grass at the site. "
				"</html>"));
	ListGrass(grass);

	m_Shrubs = new QComboBox(this);
	m_Shrubs->setToolTip(tr("<html>"
				"Choose what to do with naturally occurring shrubs at the site. "
				"</html>"));
	ListShrubs(shrubs);

	m_Trees = new QComboBox(this);
	m_Trees->setToolTip(tr("<html>"
				"Choose what to do with naturally occurring trees at the site. "
				"</html>"));
	ListTrees(trees);

	m_Rocks = new QComboBox(this);
	m_Rocks->setToolTip(tr("<html>"
				"Choose what to do with rocks at the site. "
				"</html>"));
	ListRocks(rocks);

	QLabel* lbfig = new QLabel(this);
	lbfig->setPixmap(QPixmap(":data/ScFigSiteMesh.png"));
	lbfig->setAlignment(Qt::AlignCenter);

	m_Grade = new QComboBox(this);
	m_Grade->setToolTip(tr("<html>"
				"Choose how the terrain should be graded at the site. "
				"</html>"));
	ListGrades(grade);

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
	ListBlends(terBlend);

	QVBoxLayout* glay = new QVBoxLayout;
	glay->addWidget(lbfig);
	glay->addWidget(m_Grade);
	glay->addWidget(m_Blend);
	glay->addWidget(m_Pave);
	glay->addWidget(m_Rocks);
	glay->addWidget(m_Grass);
	glay->addWidget(m_Shrubs);
	glay->addWidget(m_Trees);

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setDefault(true);
	pbok->setAutoDefault(true);
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(glay);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuSitePropertiesDlg::~AuSitePropertiesDlg()
{
}

void AuSitePropertiesDlg::setVisible(bool visible)
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

void AuSitePropertiesDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SitePropertiesDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SitePropertiesDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SitePropertiesDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SitePropertiesDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuSitePropertiesDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SitePropertiesDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SitePropertiesDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SitePropertiesDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SitePropertiesDlg/SizeY",sz.height());
}

int AuSitePropertiesDlg::Pavement() const
{
	if(m_Pave->currentIndex() < 0)
		return 255;
	return m_Pave->itemData(m_Pave->currentIndex()).toInt();
}

void AuSitePropertiesDlg::ListPavement(quint8 pavement)
{
	m_Pave->addItem(QIcon(":data/ScHide.png"), tr("Natural Ground"), WORLDTEX_ConstructionZone);
	m_Pave->addItem(QIcon(":data/DirtSmooth.png"), tr("Dirt"), WORLDTEX_DirtSmooth);
	m_Pave->addItem(QIcon(":data/CobblestoneLight.png"), tr("Cobblestone"), WORLDTEX_CobblestoneLight);
	m_Pave->addItem(QIcon(":data/ConcreteHeavy.png"), tr("Concrete"), WORLDTEX_ConcreteHeavy);
	m_Pave->addItem(QIcon(":data/Tarmac.png"), tr("Asphalt"), WORLDTEX_Tarmac);
	m_Pave->addItem(QIcon(":data/DirtCrackedSmooth.png"), tr("Field"), WORLDTEX_DirtCrackedSmooth);
	m_Pave->addItem(QIcon(":data/RockTan.png"), tr("Rock"), WORLDTEX_RockTan);
	int idx = m_Pave->findData(pavement);
	m_Pave->setCurrentIndex(idx);
}

int AuSitePropertiesDlg::Grade() const
{
	if(m_Grade->currentIndex() < 0)
		return 255;
	return m_Grade->itemData(m_Grade->currentIndex()).toInt();
}

int AuSitePropertiesDlg::Blend() const
{
	if(m_Blend->currentIndex() < 0)
		return 255;
	return m_Blend->itemData(m_Blend->currentIndex()).toInt();
}

void AuSitePropertiesDlg::ListGrades(quint8 grade)
{
	m_Grade->addItem(tr("Terrain is Not Changed"), AuScSite::Terrain);
	m_Grade->addItem(tr("Level Terrain to Mesh"), AuScSite::Level);
	m_Grade->addItem(tr("Cut Terrain Down to Mesh"), AuScSite::Cut);
	m_Grade->addItem(tr("Fill Terrain Up to Mesh"), AuScSite::Fill);
	int idx = m_Grade->findData(grade);
	m_Grade->setCurrentIndex(idx);
}

void AuSitePropertiesDlg::ListBlends(quint8 terBlend)
{
	m_Blend->addItem(tr("Flat Blend"), 0);
	m_Blend->addItem(tr("Roll Blend"), 1);
	int idx = m_Blend->findData(terBlend);
	m_Blend->setCurrentIndex(idx);
}

int AuSitePropertiesDlg::Grass() const
{
	if(m_Grass->currentIndex() < 0)
		return 255;
	return m_Grass->itemData(m_Grass->currentIndex()).toInt();
}

int AuSitePropertiesDlg::Rocks() const
{
	if(m_Rocks->currentIndex() < 0)
		return 255;
	return m_Rocks->itemData(m_Rocks->currentIndex()).toInt();
}

int AuSitePropertiesDlg::Shrubs() const
{
	if(m_Shrubs->currentIndex() < 0)
		return 255;
	return m_Shrubs->itemData(m_Shrubs->currentIndex()).toInt();
}

int AuSitePropertiesDlg::Trees() const
{
	if(m_Trees->currentIndex() < 0)
		return 255;
	return m_Trees->itemData(m_Trees->currentIndex()).toInt();
}

void AuSitePropertiesDlg::ListGrass(quint8 grass)
{
	m_Grass->addItem(tr("Remove Grass"), 0);
	m_Grass->addItem(tr("Leave Grass Alone"), 1);
	m_Grass->addItem(tr("Mow Existing Grass"), 2);
	m_Grass->addItem(tr("Grassy Lawn"), 3);
	int idx = m_Grass->findData(grass);
	m_Grass->setCurrentIndex(idx);
}

void AuSitePropertiesDlg::ListRocks(quint8 rocks)
{
	m_Rocks->addItem(tr("Remove Rocks"), 0);
	m_Rocks->addItem(tr("Leave Rocks Alone"), 1);
	int idx = m_Rocks->findData(rocks);
	m_Rocks->setCurrentIndex(idx);
}

void AuSitePropertiesDlg::ListShrubs(quint8 shrubs)
{
	m_Shrubs->addItem(tr("Remove Shrubs"), 0);
	m_Shrubs->addItem(tr("Leave Shrubs Alone"), 1);
	int idx = m_Shrubs->findData(shrubs);
	m_Shrubs->setCurrentIndex(idx);
}

void AuSitePropertiesDlg::ListTrees(quint8 trees)
{
	m_Trees->addItem(tr("Remove Trees"), 0);
	m_Trees->addItem(tr("Leave Trees Alone"), 1);
	int idx = m_Trees->findData(trees);
	m_Trees->setCurrentIndex(idx);
}

