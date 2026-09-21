//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucitieswidget.cpp,v 1.6 2022/02/19 18:10:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucitieswidget.h"
//
#include "aucity.h"
#include "auglobal.h"
#include "dgdata.h"
#include <QHeaderView>
#include <QMouseEvent>

AuCitiesWidget::AuCitiesWidget(QWidget* parent) : QTableWidget(0,8,parent)
{
	horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(5, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(6, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(7, QHeaderView::Interactive);
	QStringList headerLabels;
	headerLabels << tr("City");
	headerLabels << tr("Status");
	headerLabels << tr("Planet");
	headerLabels << tr("Solar System");
	headerLabels << tr("Sector");
	headerLabels << tr("Galaxy");
	headerLabels << tr("Founder");
	headerLabels << tr("Notes");
	setHorizontalHeaderLabels(headerLabels);
	setAlternatingRowColors(true);
	//setGridStyle(Qt::NoPen);
	//setCornerButtonEnabled(false);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setShowGrid(false);
	setSortingEnabled(true);
	setToolTip(tr("Empire cities. Right-click for options."));
	sortByColumn(0, Qt::AscendingOrder);
	verticalHeader()->hide();
	verticalHeader()->setDefaultSectionSize(18);
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
}

AuCitiesWidget::~AuCitiesWidget()
{
}

void AuCitiesWidget::mouseReleaseEvent(QMouseEvent* event)
{
	QTableWidget::mouseReleaseEvent(event);

	if(event->button() != Qt::RightButton)
		return;

	QTableWidgetItem* itemClicked = itemAt(event->pos());
	if(!itemClicked)
		return;
	QTableWidgetItem* itemCol0 = item(itemClicked->row(), 0);
	if(!itemCol0)
		return;

	AuId cityId = AuId(itemCol0->data(Qt::UserRole).toUInt());
	emit CityRightClicked(cityId);
}

void AuCitiesWidget::ListCities(quint32 empireId)
{
	m_EmpireId = empireId;

	DgEmpire empire;
	empire.m_id = empireId;
	const DgData& gd = g_Au.LockDataForRead();
	gd.Empire(empire);
	g_Au.UnlockData();

	// Remove current cities list
	int iCount = rowCount();
	for(int i=0; i<iCount; i++)
		removeRow(0);

	setSortingEnabled(false);
	for(AuCityHash::const_iterator ci = empire.m_Cities->cbegin(); ci != empire.m_Cities->cend(); ci++)
	{
		insertRow(0);

		int iColumn = 0;
		QIcon icon((*ci).second.CityIcon());
		QTableWidgetItem* itemCity = new QTableWidgetItem(icon, (*ci).second.CityName());
		itemCity->setData(Qt::UserRole, (*ci).second.CityId().GetID());
		setItem(0, iColumn++, itemCity);

		QIcon statusIcon;
		switch((*ci).second.AlertCondition())
		{
		case ConditionGreen:
			statusIcon = QIcon(":data/AlertGreen.png");
			break;
		case ConditionYellow:
			statusIcon = QIcon(":data/AlertYellowBright.png");
			break;
		case ConditionRed:
			statusIcon = QIcon(":data/AlertRedBright.png");
			break;
		}
		QString morale = QString("%1").arg((*ci).second.Morale());
		QTableWidgetItem* itemStatus = new QTableWidgetItem(statusIcon, morale);
		itemStatus->setToolTip(tr("<html>"
					"Alert Condition and Morale"
					"</html>"));
		setItem(0, iColumn++, itemStatus);

		QTableWidgetItem* itemPlanet = new QTableWidgetItem((*ci).second.PlanetName());
		itemPlanet->setData(Qt::UserRole, (*ci).second.PlanetId().GetID());
		setItem(0, iColumn++, itemPlanet);

		QTableWidgetItem* itemSystem = new QTableWidgetItem((*ci).second.SystemName());
		itemSystem->setData(Qt::UserRole, (*ci).second.SystemId().GetID());
		setItem(0, iColumn++, itemSystem);

		QTableWidgetItem* itemSector = new QTableWidgetItem((*ci).second.SectorName());
		itemSector->setData(Qt::UserRole, (*ci).second.SectorId().GetID());
		setItem(0, iColumn++, itemSector);

		QTableWidgetItem* itemGalaxy = new QTableWidgetItem((*ci).second.GalaxyName());
		itemGalaxy->setData(Qt::UserRole, (*ci).second.GalaxyId().GetID());
		setItem(0, iColumn++, itemGalaxy);

		// Founder
		QString founder = QString("%1 (%2)").arg((*ci).second.FounderName()).arg((*ci).second.FounderId().StringId());
		QTableWidgetItem* itemFounder = new QTableWidgetItem(founder);
		itemFounder->setData(Qt::UserRole, (*ci).second.FounderId().GetID());
		setItem(0, iColumn++, itemFounder);

		// Notes
		QString cap;
		if((*ci).second.CityType() == CITY_MilitaryBase)
			cap = tr("Military Base");
		else if((*ci).second.CityId() == empire.m_capitol && (*ci).second.IsGalaxyCapital() && (*ci).second.IsSectorCapital())
			cap = tr("Empire, Galaxy and Sector Capital");
		else if((*ci).second.CityId() == empire.m_capitol && (*ci).second.IsGalaxyCapital())
			cap = tr("Empire and Galaxy Capital");
		else if((*ci).second.CityId() == empire.m_capitol && (*ci).second.IsSectorCapital())
			cap = tr("Empire and Sector Capital");
		else if((*ci).second.IsGalaxyCapital() && (*ci).second.IsSectorCapital())
			cap = tr("Galaxy and Sector Capital");
		else if((*ci).second.CityId() == empire.m_capitol)
			cap = tr("Empire Capital");
		else if((*ci).second.IsGalaxyCapital())
			cap = tr("Galaxy Capital");
		else if((*ci).second.IsSectorCapital())
			cap = tr("Sector Capital");

		QTableWidgetItem* itemCapitol = new QTableWidgetItem(cap);
		setItem(0, iColumn++, itemCapitol);
	}
	setSortingEnabled(true);

	// If the table had no rows to begin with, resize the columns to the fresh data.
	if(!iCount)
		resizeColumnsToContents();
}

// Returns 0 = no governance authority, 1 = office authority, 2 = founder authority
int AuCitiesWidget::SelectedCities(AuIdList& cityIds, const AuId& personId) const
{
	int govOrOwn = 2;	// founder
	QList<QTableWidgetItem*> selItems = selectedItems();
	for(QList<QTableWidgetItem*>::const_iterator si = selItems.cbegin(); si != selItems.cend(); si++)
	{
		if((*si)->column() != 0)
			continue;

		AuId cityId = AuId((*si)->data(Qt::UserRole).toUInt());
		cityIds.AddIfUnique(cityId);

		// We initialized govOrOwn to the max so we try to step it down at each city.
		if(govOrOwn)
		{
			const DgData& gd = g_Au.LockDataForRead();
			AuId founderId(gd.EmpireCityFounder(m_EmpireId, cityId));
			if(!founderId.IsNull() && founderId != personId && gd.EmpireHasCitizen(m_EmpireId, founderId))
			{
				// Person is not the founder and founder is an empire citizen. When founder is not a citizen, city can be
				// surrendered.
				// Step down authority level.
				if(gd.IsEmpireCityInDomain(m_EmpireId, cityId, personId))
					govOrOwn = 1;	// person's office has authority
				else
					govOrOwn = 0;	// no governance authority
			}
			g_Au.UnlockData();
		}
	}
	return govOrOwn;
}

// Returns 0 = no governance authority, 1 = office authority, 2 = founder authority
int AuCitiesWidget::SelectedCities(AuIdList& cityIds, QStringList& cityNames, const AuId& personId) const
{
	int govOrOwn = 2;	// founder
	QList<QTableWidgetItem*> selItems = selectedItems();
	for(QList<QTableWidgetItem*>::const_iterator si = selItems.cbegin(); si != selItems.cend(); si++)
	{
		if((*si)->column() != 0)
			continue;

		AuId cityId = AuId((*si)->data(Qt::UserRole).toUInt());
		cityIds.append(cityId);
		QString cityName = (*si)->text();	// Text of column 0 is the city name.
		cityNames.append(cityName);

		// We initialized govOrOwn to the max so we try to step it down at each city.
		if(govOrOwn)
		{
			const DgData& gd = g_Au.LockDataForRead();
			AuId founderId(gd.EmpireCityFounder(m_EmpireId, cityId));
			if(!founderId.IsNull() && founderId != personId && gd.EmpireHasCitizen(m_EmpireId, founderId))
			{
				// Person is not the founder and founder is an empire citizen. When founder is not a citizen, city can be
				// surrendered.
				// Step down authority level.
				if(gd.IsEmpireCityInDomain(m_EmpireId, cityId, personId))
					govOrOwn = 1;	// person's office has authority
				else
					govOrOwn = 0;	// no governance authority
			}
			g_Au.UnlockData();
		}
	}
	return govOrOwn;
}

void AuCitiesWidget::SelectedCitySystems(AuIdList& systemIds, QStringList& systemNames) const
{
	QList<QTableWidgetItem*> selItems = selectedItems();
	for(QList<QTableWidgetItem*>::const_iterator si = selItems.cbegin(); si != selItems.cend(); si++)
	{
		QTableWidgetItem* sysItem = item((*si)->row(), 3);
		if(!sysItem)
			continue;
		AuId sysId = AuId(sysItem->data(Qt::UserRole).toUInt());
		if(!systemIds.AddIfUnique(sysId))
			continue;	// Already in the list
		QString sysName = sysItem->text();
		systemNames.append(sysName);
	}
}

