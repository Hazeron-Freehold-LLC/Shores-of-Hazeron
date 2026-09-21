//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austarmapplaces.cpp,v 1.17 2022/02/19 18:17:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austarmapplaces.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daatom.h"
#include "dgdata.h"
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>
#include <QVariant>

AuStarMapPlaces::AuStarMapPlaces(QWidget* parent) : QTreeView(parent)
{
	m_PreviewType = DaAtom::ATOM_Invalid;	// AtomType of preview atom.
	setEditTriggers(QTreeView::NoEditTriggers);
	connect(header(), SIGNAL(sectionResized(int, int, int)), SLOT(ColumnResized(int, int, int)));
}

AuStarMapPlaces::~AuStarMapPlaces()
{
}

void AuStarMapPlaces::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QTreeView::selectionChanged(selected, deselected);

	emit SelectionChanged(selected, deselected);
}

void AuStarMapPlaces::mouseReleaseEvent(QMouseEvent* event)
{
	QTreeView::mouseReleaseEvent(event);

	if(event->button() == Qt::RightButton)
	{
		QModelIndex itemIdx = indexAt(event->pos());
		if(itemIdx.isValid())
		{
			// Get data off the selected item.
			QModelIndex col0 = itemIdx.sibling(itemIdx.row(), 0);
			QModelIndex col1 = itemIdx.sibling(itemIdx.row(), 1);
			m_PreviewName = model()->data(col0).toString();						// Place name
			m_Preview = AuId(model()->data(col0, Qt::UserRole).toUInt());		// Atom Id
			quint8 genHab = model()->data(col0, Qt::UserRole+1).toUInt();
			quint32 genSeed = model()->data(col0, Qt::UserRole+2).toUInt();
			quint8 galaxy = model()->data(col0, Qt::UserRole+3).toUInt();
			bool isCoreBlackHole = model()->data(col0, Qt::UserRole+4).toBool();
			quint32 cityEmpire = model()->data(col0, Qt::UserRole+5).toUInt();
			bool isExplored = model()->data(col0, Qt::UserRole+7).toUInt();
			bool isScanned = model()->data(col0, Qt::UserRole+8).toUInt();
			m_PreviewType = model()->data(col1, Qt::UserRole).toUInt();			// Atom type.
			QString gloc = model()->data(col1).toString();
			m_PreviewLoc = AuPoint(gloc);										// Galaxy location in parsecs.

			QMenu menu(this);

			QAction* lookAt = menu.addAction(QIcon(":data/MsgSpot.png"), tr("Look At %1").arg(m_PreviewName));

			QAction* querySector = 0;
			QAction* reportCityLast = 0;
			QAction* reportCityHistory = 0;
			QAction* reportExplored = 0;
			QAction* reportObserved = 0;
			QAction* reportSurveyed = 0;
			QAction* domainAdd = 0;
			QAction* selectSector = 0;
			if(m_PreviewType == DaAtom::ATOM_Avatar)
			{
				selectSector = menu.addAction(QIcon(":data/RangeSector.png"), tr("Select Sector Containing %1").arg(m_PreviewName));
			}
			else if(m_PreviewType == DaAtom::ATOM_Bd)
			{
				AuView* pView = auApp->Frame()->View();
				quint32 sceneEmpire = pView->SceneAtomEmpire();
				reportCityLast = menu.addAction(QIcon(":data/MailCityStatus.png"),
						tr("%1 Recent Report...").arg(m_PreviewName));
				if(cityEmpire != sceneEmpire)
					reportCityLast->setEnabled(false);
				reportCityHistory = menu.addAction(QIcon(":data/MailCityStatus.png"),
						tr("%1 History Report...").arg(m_PreviewName));
				if(cityEmpire != sceneEmpire)
					reportCityHistory->setEnabled(false);
			}
			else if(m_PreviewType == DaAtom::ATOM_Sector)
			{
				querySector = menu.addAction(QIcon(":data/dp_NavConsole.png"), tr("Requery %1").arg(m_PreviewName));
			}
			else if(m_PreviewType == DaAtom::ATOM_SolarSystem)
			{
				if(isScanned)
					reportSurveyed = menu.addAction(QIcon(":data/SystemSurvey.png"),
							tr("%1 Survey Report...").arg(m_PreviewName));
				else if(isExplored)
					reportExplored = menu.addAction(QIcon(":data/SystemWorlds.png"),
							tr("%1 Exploration Report...").arg(m_PreviewName));
				else
					reportObserved = menu.addAction(QIcon(":data/bObservatory.png"),
							tr("%1 Observatory Report...").arg(m_PreviewName));

				if(!auApp->IsSingleUserDesignerMode())
					domainAdd = menu.addAction(QIcon(":data/AvatarDomain.png"),
							tr("Add %1 to Land...").arg(m_PreviewName));
			}

			QAction* goDirect = 0;
			QAction* goDirectToCoord = 0;
			QAction* goViaWormholes = 0;
			QAction* doScanSystem = 0;
			QAction* doScanTactical = 0;
			QAction* landAtCity = 0;
			QAction* orbitCity = 0;
			if(!auApp->IsSingleUserDesignerMode())
			{
				menu.addSeparator();

				// Travel orders
				if(m_PreviewType == DaAtom::ATOM_Sector)
				{
					AuMissionOrderGoToCoordinate order(AuMissionOrderGoToCoordinate::Galaxy, m_PreviewLoc);
					goDirectToCoord = menu.addAction(QIcon(order.ListNameIcon()), order.ListName());
				}
				else if(m_PreviewType == DaAtom::ATOM_SolarSystem)
				{
					AuMissionOrderGoToSystemDirect order1(m_Preview, m_PreviewName, m_PreviewLoc);
					goDirect = menu.addAction(QIcon(order1.ListNameIcon()), order1.ListName());

					AuMissionOrderGoToSystemViaWormholes order2(m_Preview, m_PreviewName);
					goViaWormholes = menu.addAction(QIcon(order2.ListNameIcon()), order2.ListName());

					AuMissionOrderScanSurveySystem order3(false);
					doScanSystem = menu.addAction(QIcon(order3.ListNameIcon()), order3.ListName());
					if(isScanned)
						doScanSystem->setEnabled(false);

					AuMissionOrderScanTactical order4(false);
					doScanTactical = menu.addAction(QIcon(order4.ListNameIcon()), order4.ListName());
				}
				else if(m_PreviewType == DaAtom::ATOM_Bd)
				{
					AuMissionOrderGoToOrbitCity orbitOrder(m_Preview, m_PreviewName);
					orbitCity = menu.addAction(QIcon(orbitOrder.ListNameIcon()), orbitOrder.ListName());

					AuMissionOrderGoToCityAnywhere landOrder(m_Preview, m_PreviewName);
					landAtCity = menu.addAction(QIcon(landOrder.ListNameIcon()), landOrder.ListName());
				}
			}

			QAction* action = menu.exec(QCursor::pos());
			if(!action)
				return;

			if(action == lookAt)
			{
				// Center view on preview atom, which is the closest atom to the cursor.
				emit PlaceDoubleClicked(m_Preview);
			}
			else if(action == querySector)
			{
				emit QuerySector(m_Preview);
			}
			else if(action == selectSector)
			{
				AuView* pView = auApp->Frame()->View();
				emit PlaceClicked(pView->SceneSector());
			}
			else if(action == reportSurveyed)
			{
				emit ReportSystemResources(m_Preview, m_PreviewName, galaxy, isCoreBlackHole, genHab, genSeed, m_PreviewLoc, -1.f,
						2);
			}
			else if(action == reportExplored)
			{
				emit ReportSystemResources(m_Preview, m_PreviewName, galaxy, isCoreBlackHole, genHab, genSeed, m_PreviewLoc, -1.f,
						1);
			}
			else if(action == reportObserved)
			{
				emit ReportSystemResources(m_Preview, m_PreviewName, galaxy, isCoreBlackHole, genHab, genSeed, m_PreviewLoc, -1.f,
						0);
			}
			else if(action == domainAdd)
			{
				QString sysName(m_PreviewName);
				int nl = sysName.indexOf('\n');
				if(nl >= 0)
					sysName = sysName.left(nl);
				emit DomainAdd(m_Preview, sysName);
			}
			else if(action == reportCityLast)
			{
				emit ReportCityLast(m_Preview);
			}
			else if(action == reportCityHistory)
			{
				emit ReportCityHistory(m_Preview);
			}
			else if(action == goDirect)
			{
				emit GoDirectToSystem(m_Preview, m_PreviewName, m_PreviewLoc);
			}
			else if(action == goDirectToCoord)
			{
				emit GoDirectToCoord(m_PreviewLoc);
			}
			else if(action == goViaWormholes)
			{
				emit GoViaWormholes(m_Preview, m_PreviewName);
			}
			else if(action == doScanSystem)
			{
				emit ScanSystem();
			}
			else if(action == doScanTactical)
			{
				emit ScanTactical();
			}
			else if(action == orbitCity)
			{
				emit OrbitCity(m_Preview, m_PreviewName);
			}
			else if(action == landAtCity)
			{
				emit LandAtCity(m_Preview, m_PreviewName);
			}
		}
	}
}

void AuStarMapPlaces::RestoreSettings()
{
	int col0 = g_Au.Settings().readNumEntry("Widget/Locator/StarMap/ColumnWidth1", 210);
	int col1 = g_Au.Settings().readNumEntry("Widget/Locator/StarMap/ColumnWidth2", 110);
	header()->blockSignals(true);
	header()->resizeSection(0, col0);
	header()->resizeSection(1, col1);
	header()->blockSignals(false);
}

void AuStarMapPlaces::ColumnResized(int columnIdx, int oldWidth, int nuWidth)
{
	Q_UNUSED(oldWidth);
	//qDebug() << "AuStarMapPlaces::ColumnResized" << columnIdx << oldWidth << nuWidth;
	if(columnIdx > 1)
		return;	// Don't store the stretchy column.
	g_Au.Settings().writeEntry(QString("/Widget/Locator/StarMap/ColumnWidth%1").arg(columnIdx+1), nuWidth);
}
