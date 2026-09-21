//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovchooseplace.cpp,v 1.3 2022/02/19 18:10:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augovchooseplace.h"
//
#include "aucity.h"
#include "auglobal.h"
#include "auoffice.h"
#include "dgempire.h"
#include <QGridLayout>
#include <QTreeWidget>
#include <QPushButton>

AuGovChoosePlace::AuGovChoosePlace(const DgEmpire& empire, const AuOffice& office, QWidget* parent) : AuDialog(parent)
{
	m_Place = 0;
	setAuDlgTitle(tr("Choose Place"));
	setAuDlgIcon(QIcon(":data/CommGovernment.png"));

	m_DomainTree = new QTreeWidget(this);
	m_DomainTree->setColumnCount(1);
	QStringList citizensHdr;
	citizensHdr << tr("Empire Domain");
	m_DomainTree->setHeaderLabels(citizensHdr);
	connect(m_DomainTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), SLOT(SelectionChanged()));
	connect(m_DomainTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(Accept()));

	m_OkButton = new QPushButton(tr("Ok"), this);
	m_OkButton->setAutoDefault(true);
	m_OkButton->setDefault(true);
	m_OkButton->setEnabled(false);
	m_OkButton->setToolTip(tr("<html>Accepts the chosen place.</html>"));
	connect(m_OkButton, SIGNAL(clicked()), SLOT(Accept()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(m_DomainTree, row,0, 1,2);
	row++;
	lay->addWidget(m_OkButton, row,1);
	lay->setColumnStretch(0, 1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	FillTree(m_DomainTree, empire, office);
}

AuGovChoosePlace::~AuGovChoosePlace()
{
	if(m_Place)
		delete m_Place;
}

void AuGovChoosePlace::setVisible(bool visible)
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

void AuGovChoosePlace::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GovChoosePlace/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GovChoosePlace/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/GovChoosePlace/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/GovChoosePlace/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuGovChoosePlace::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GovChoosePlace/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GovChoosePlace/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/GovChoosePlace/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/GovChoosePlace/SizeY",sz.height());
}

void AuGovChoosePlace::SelectionChanged()
{
	m_OkButton->setEnabled(m_DomainTree->currentItem());
}

void AuGovChoosePlace::Accept()
{
	const QTreeWidgetItem* item = m_DomainTree->currentItem();
	if(!item)
		return;

	AuId placeId = AuId(item->data(0, Qt::UserRole).toUInt());
	QString placeName = item->text(0);
	int placeType = item->data(0, Qt::UserRole+1).toInt();
	switch(placeType)
	{
	case 1:
		m_Place = new AuOfficeDomainUniverse();
		break;
	case 2:
		m_Place = new AuOfficeDomainGalaxy(placeId, placeName);
		break;
	case 3:
		m_Place = new AuOfficeDomainSector(placeId, placeName);
		break;
	case 4:
		m_Place = new AuOfficeDomainSolarSystem(placeId, placeName);
		break;
	case 5:
		m_Place = new AuOfficeDomainWorld(placeId, placeName);
		break;
	case 6:
		m_Place = new AuOfficeDomainCity(placeId, placeName);
		break;
	}

	accept();
}

void AuGovChoosePlace::FillTree(QTreeWidget* tree, const DgEmpire& empire, const AuOffice& office)
{
	// Add items for the government.
	AuId universeId(AuId::Universe());
	QSet<AuId> alreadyListed;

	// Mark places already in their domain as already listed so they won't be shown.
	//for(AuOfficeDomain::const_iterator di = office.Domain().cbegin(); di != office.Domain().cend(); di++)
		//alreadyListed.insert((*di)->AtomId());

	QTreeWidgetItem* universe = new QTreeWidgetItem(tree);
	universe->setExpanded(true);
	universe->setIcon(0, QIcon(":data/RangeUniverse.png"));
	universe->setText(0, tr("Universe"));
	universe->setData(0, Qt::UserRole, universeId.GetID());
	universe->setData(0, Qt::UserRole+1, AuOfficeDomainUniverse::PlaceType());
	alreadyListed.insert(universeId);

	QFont noGovernorFont = universe->font(0);
	QBrush noGovernorBrush(QColor(255,255,32));

	QFont domainFont = universe->font(0);
	domainFont.setWeight(QFont::Bold);
	QBrush domainBrush(QColor(32,255,255));

	if(office.IsPlaceInDomain(universeId))
	{
		universe->setFont(0, domainFont);
		universe->setForeground(0, domainBrush);
	}
	else if(!empire.PlaceHasGovernor(universeId))
	{
		universe->setFont(0, noGovernorFont);
		universe->setForeground(0, noGovernorBrush);
	}

	AddSubordinates(universe, empire, office, alreadyListed, domainFont, domainBrush, noGovernorFont, noGovernorBrush);
}

// Recursive
void AuGovChoosePlace::AddSubordinates(QTreeWidgetItem* domain, const DgEmpire& empire, const AuOffice& office,
		QSet<AuId>& alreadyListed, const QFont& domainFont, const QBrush& domainBrush, const QFont& noGovernorFont,
		const QBrush& noGovernorBrush)
{
	// Get the list of names and ids of all places that are subordinate to the domain.
	AuId domainId = AuId(domain->data(0, Qt::UserRole).toUInt());
	int domainType = domain->data(0, Qt::UserRole+1).toInt();
	QIcon icon;
	switch(domainType)
	{
	case 1:	// Universe
		icon = QIcon(":data/RangeGalaxy.png");
		break;
	case 2:	// Galaxy
		icon = QIcon(":data/RangeSector.png");
		break;
	case 3:	// Sector
		icon = QIcon(":data/RangeSystem.png");
		break;
	case 4:	// Solar System
		icon = QIcon(":data/RangeHail.png");
		break;
	case 5:	// World
		icon = QIcon(":data/City.png");
		break;
	case 6:	// City
		return;	// Cities do not have subordinate places.
	}
	QMap<QString, AuId> subs;
	QSet<AuId> inmap;
	for(AuCityHash::const_iterator ci = empire.m_Cities->cbegin(); ci != empire.m_Cities->cend(); ci++)
	{
		AuId placeId;
		AuId parentId;
		QString placeName;
		switch(domainType)
		{
		case 1:	// Universe
			placeId = (*ci).second.GalaxyId();
			placeName = (*ci).second.GalaxyName();
			parentId = AuId::Universe();
			break;
		case 2:	// Galaxy
			placeId = (*ci).second.SectorId();
			placeName = (*ci).second.SectorName();
			parentId = (*ci).second.GalaxyId();
			break;
		case 3:	// Sector
			placeId = (*ci).second.SystemId();
			placeName = (*ci).second.SystemName();
			parentId = (*ci).second.SectorId();
			break;
		case 4:	// Solar System
			placeId = (*ci).second.PlanetId();
			placeName = (*ci).second.PlanetName();
			parentId = (*ci).second.SystemId();
			break;
		case 5:	// World
			placeId = (*ci).second.CityId();
			placeName = (*ci).second.CityName();
			parentId = (*ci).second.PlanetId();
			break;
		case 6:	// City
			continue;
		}
		if(parentId != domainId)
			continue;
		if(inmap.contains(placeId))
			continue;

		subs.insert(placeName, placeId);
	}

	// Create a tree entry under domain for each subordinate place.
	for(QMap<QString, AuId>::const_iterator si = subs.cbegin(); si != subs.cend(); si++)
	{
		if(alreadyListed.contains(*si))
			continue;

		QTreeWidgetItem* sub = new QTreeWidgetItem(domain);
		sub->setExpanded(true);
		sub->setIcon(0, icon);
		sub->setText(0, si.key());
		sub->setData(0, Qt::UserRole, (*si).GetID());
		sub->setData(0, Qt::UserRole+1, domainType+1);
		alreadyListed.insert(*si);

		if(office.IsPlaceInDomain(*si))
		{
			sub->setFont(0, domainFont);
			sub->setForeground(0, domainBrush);
		}
		else if(!empire.PlaceHasGovernor(*si))
		{
			sub->setFont(0, noGovernorFont);
			sub->setForeground(0, noGovernorBrush);
		}

		// Add the citizen's subordinates, if any.
		AddSubordinates(sub, empire, office, alreadyListed, domainFont, domainBrush, noGovernorFont, noGovernorBrush);
	}
}
