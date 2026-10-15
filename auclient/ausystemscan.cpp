//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausystemscan.cpp,v 1.52 2022/05/04 21:23:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausystemscan.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "ausystemwidget.h"
#include "autoolbutton.h"
#include "auview.h"
#include "daasteroid.h"
#include "daatomlist.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "davehicle.h"
#include "daworldgasgiant.h"
#include "daworldring.h"
#include "daworldringsection.h"
#include "dgdata.h"
#include <QHeaderView>
#include <QLabel>
#include <QSpinBox>
#include <QTextBrowser>
#include <QTimer>
#include <QTreeWidget>
#include <QSplitter>
#include <QVBoxLayout>

void AuLocatorSystemScanDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleLocatorSystemScanUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSystemScanDialog::AuSystemScanDialog(const AuId& sectorId, const QString& sectorName, const AuId& systemId,
		const QString& systemName, GALAXY galaxy, bool isCoreBlackHole, HABITABILITY genHab, quint32 genSeed,
		const AuPoint& systemGalaxyLoc, float starLight, int scanEx, QWidget* parent) :
	AuDialog(parent)
{
	SetWikiTopic("System Survey");
	if(scanEx == 2)
	{
		setAuDlgTitle(tr("Solar System Survey Report"));
		setAuDlgIcon(QIcon(":data/SystemSurvey.png"));
	}
	else if(scanEx == 1)
	{
		setAuDlgTitle(tr("Solar System Exploration Report"));
		setAuDlgIcon(QIcon(":data/SystemWorlds.png"));
	}
	else
	{
		setAuDlgTitle(tr("Solar System Observatory Report"));
		setAuDlgIcon(QIcon(":data/bObservatory.png"));
	}

	DaAtomList asteroids;

	AuSystemScan* scan = new AuSystemScan(false, this);
	scan->SetSystem(sectorId, sectorName, systemId, systemName, galaxy, isCoreBlackHole, genHab, genSeed, systemGalaxyLoc,
			starLight, scanEx, asteroids);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(scan);
	setAuDlgLayout(lay);
}

AuSystemScanDialog::~AuSystemScanDialog()
{
}

void AuSystemScanDialog::setVisible(bool visible)
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
		deleteLater();
	}
	else
		AuDialog::setVisible(visible);
}

void AuSystemScanDialog::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SystemScanDialog/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SystemScanDialog/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SystemScanDialog/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SystemScanDialog/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuSystemScanDialog::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SystemScanDialog/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SystemScanDialog/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SystemScanDialog/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SystemScanDialog/SizeY",sz.height());
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSystemScan::AuSystemScan(bool showCurrentSystem, QWidget* parent) : QWidget(parent), m_Ether("AuSystemScan")
{
	int minQ = g_Au.Settings().readNumEntry("Widget/SystemScan/MinQToShow", 200);
	m_ScanEx = 0;
	m_ShowCurrentSystem = showCurrentSystem;
	m_System = 0;
	m_Timer = 0;

	m_Title = new QLabel(this);

	m_SystemPic = new AuSystemWidget(this);
	m_SystemPic->SetMinQToShow(minQ);
	m_SystemPic->setToolTip(tr("<html>"
				"<i>Tips:</i>"
				"<br>"
				"%1 Left click to select. "
				"<br>"
				"%1 Left button drag to pan. "
				"<br>"
				"%1 Right click for menu. "
				"<br>"
				"%1 Right button drag to rotate. "
				"<br>"
				"%1 Wheel to magnify. "
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_SystemPic, SIGNAL(Attack(AuId, QString)), auApp, SLOT(Attack(AuId, QString)));
	connect(m_SystemPic, SIGNAL(Follow(AuId, QString)), auApp, SLOT(Follow(AuId, QString)));
	connect(m_SystemPic, SIGNAL(BodyClicked(AuId)), SLOT(BodyClicked(AuId)));
	connect(m_SystemPic, SIGNAL(GoToCoord(AuPoint)), auApp, SLOT(GoToCoord(AuPoint)));
	connect(m_SystemPic, SIGNAL(GoLand(AuId, QString)), auApp, SLOT(GoLand(AuId, QString)));
	connect(m_SystemPic, SIGNAL(GoOrbit(AuId, QString)), auApp, SLOT(GoOrbit(AuId, QString)));

	m_LandAdd = new AuIconButton(QIcon(":data/AvatarDomain.png"), this);
	m_LandAdd->setToolTip(tr("<html><p><b>Add to Land...</b><br>"
				"Adds this solar system to your land."
				"</p>"
				"<p>"
				"An avatar's land can contain up to %1 solar systems. "
				"Buildings on your land are immune to abandonment decay for 30 days after this avatar was last on line. "
				"</p>"
				"</html>").arg(AuDomain::LimitPerAvatar()));
	connect(m_LandAdd, SIGNAL(clicked()), SLOT(LandAdd()));

	m_MinQLabel = new QLabel(tr("Quality of Resources to Show"), this);
	m_MinQLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_MinQToShow = new QSpinBox(this);
	m_MinQToShow->setRange(1,255);
	m_MinQToShow->setValue(minQ);
	m_MinQToShow->setToolTip(tr("<html>Sets the lowest quality of resources shown when a sun or world is selected. "
				"No resources are shown when the system is not surveyed.</html>"));
	connect(m_MinQToShow, SIGNAL(valueChanged(int)), m_SystemPic, SLOT(SetMinQToShow(int)));

	m_Bodies = new QTreeWidget;
	m_Bodies->setColumnCount(2);
	QStringList hdr;
	hdr << tr("Name");
	hdr << tr("Orbit Zone");
	m_Bodies->setHeaderLabels(hdr);
	m_Bodies->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_Bodies->setSelectionMode(QAbstractItemView::SingleSelection);
	connect(m_Bodies, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
			SLOT(BodySelected(QTreeWidgetItem*, QTreeWidgetItem*)));

	m_ViewReset = new AuIconButton(QIcon(":data/RangeSystem.png"), this);
	m_ViewReset->setToolTip(tr("<html><p><b>Reset View</b><br>"
				"Resets the view to top down. "
				"</p>"
				"</html>"));
	connect(m_ViewReset, SIGNAL(clicked()), m_SystemPic, SLOT(ViewReset()));

	m_AtomItem = new QTreeWidgetItem(m_Bodies);

	m_BodyInfo = new QTextBrowser;
	m_BodyInfo->setReadOnly(true);

	QHBoxLayout* qlay = new QHBoxLayout;
	qlay->addWidget(m_ViewReset);
	qlay->addStretch();
	qlay->addWidget(m_MinQLabel, 1);
	qlay->addWidget(m_MinQToShow);

	// Assemble the widgets into splitters.
	QVBoxLayout* blay = new QVBoxLayout;
	blay->addLayout(qlay);
	blay->addWidget(m_SystemPic);
	blay->setMargin(0);
	blay->setSpacing(2);

	QWidget* bwid = new QWidget;
	bwid->setLayout(blay);

	m_ScanSplitter = new QSplitter;
	m_ScanSplitter->setOrientation(Qt::Vertical);
	m_ScanSplitter->addWidget(bwid);
	m_ScanSplitter->addWidget(m_Bodies);
	m_ScanSplitter->setCollapsible(0, false);
	m_ScanSplitter->setCollapsible(1, false);
	m_ScanSplitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_ScanSplitter, SIGNAL(splitterMoved(int, int)), SLOT(ScanSplitterMoved(int, int)));

	m_BodySplitter = new QSplitter;
	m_BodySplitter->setOrientation(Qt::Horizontal);
	m_BodySplitter->addWidget(m_ScanSplitter);
	m_BodySplitter->addWidget(m_BodyInfo);
	m_BodySplitter->setCollapsible(0, false);
	m_BodySplitter->setCollapsible(1, false);
	m_BodySplitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_BodySplitter, SIGNAL(splitterMoved(int, int)), SLOT(BodySplitterMoved(int, int)));

	QHBoxLayout* tlay = new QHBoxLayout;
	tlay->addWidget(m_Title,1);
	tlay->addWidget(m_LandAdd);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(tlay);
	lay->addWidget(m_BodySplitter);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	QByteArray splitterstate;
	g_Au.Settings().readEntry("Widget/SystemScan/BodySplitter", splitterstate);
	m_BodySplitter->restoreState(splitterstate);

	g_Au.Settings().readEntry("Widget/SystemScan/ScanSplitter", splitterstate);
	m_ScanSplitter->restoreState(splitterstate);
}

AuSystemScan::~AuSystemScan()
{
}

void AuSystemScan::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	if(m_ShowCurrentSystem)
	{
		// Create a timer to keep this page updated.
		if(!m_Timer)
		{
			m_Timer = new QTimer(this);
			connect(m_Timer, SIGNAL(timeout()), SLOT(TimerUpdate()));
		}
		if(!m_Timer->isActive())
			m_Timer->start(1000);
		TimerUpdate();	// Don't wait for the first timer update.
	}
}

void AuSystemScan::TimerUpdate()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_Timer && m_Timer->isActive())
			m_Timer->stop();
		return;
	}
	DaPerson* person;
	AuView* pView = auApp->Frame()->View();
	pView->LockSceneAtom(&person);
	if(person)
	{
		AuSensorBlip::TYPE blipType = AuSensorBlip::Invalid;
		POLITICAL_STANCE blipStance = POL_Neutral;

		const DaVessel* ves = person->FindParentVesselInEnv();
		if(ves)
		{
			blipStance = PoliticalStance(person->EmpireId(), ves->EmpireId());
			if(ves->ShieldsWorkingAndUp())
			{
				if(ves->IsSpaceStation())
					blipType = AuSensorBlip::StationShielded;
				else if(ves->HasFTLDrive())
					blipType = AuSensorBlip::StarshipShielded;
				else
					blipType = AuSensorBlip::SpaceshipShielded;
			}
			else
			{
				if(ves->IsSpaceStation())
					blipType = AuSensorBlip::Station;
				else if(ves->HasFTLDrive())
					blipType = AuSensorBlip::Starship;
				else
					blipType = AuSensorBlip::Spaceship;
			}
		}
		else
		{
			const DaVehicle* veh = dynamic_cast<const DaVehicle*>(person->Parent());
			if(veh)
			{
				blipStance = PoliticalStance(person->EmpireId(), veh->EmpireId());
				switch(veh->ParkingSpotType())
				{
					//DHH SPOT_SpecificVehicle generates a warning here, until design changes are backported.
				case SPOT_Invalid:
				case SPOT_GroundVehicle:
				case SPOT_Count:
					blipType = AuSensorBlip::GroundVehicle;
					break;
				case SPOT_Rocket:
				case SPOT_SpaceVehicle:
					blipType = AuSensorBlip::Aircraft;
					break;
				case SPOT_WaterVehicle:
					blipType = AuSensorBlip::Boat;
					break;
				}
			}
			else
			{
				blipStance = POL_Empire;
				blipType = AuSensorBlip::Avatar;
			}
		}

		SetSystem(person->FindParentSolarSystemInEnv(), person->EmpireId(), blipType, blipStance, person->EnvLocation());
	}
	pView->UnlockSceneAtom();
}

void AuSystemScan::setVisible(bool visible)
{
	if(visible)
	{
		QWidget::setVisible(visible);
		RestoreGeometry();
	}
	else
	{
		SaveGeometry();
		QWidget::setVisible(visible);
	}
}

void AuSystemScan::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SystemScan/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SystemScan/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SystemScan/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SystemScan/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuSystemScan::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SystemScan/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SystemScan/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SystemScan/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SystemScan/SizeY",sz.height());

	g_Au.Settings().writeEntry("Widget/SystemScan/MinQToShow", m_MinQToShow->value());
}

void AuSystemScan::SetSystem(const AuId& sectorId, const QString& sectorName, const AuId& systemId, const QString& systemName,
		GALAXY galaxy, bool isCoreBlackHole, HABITABILITY genHab, quint32 genSeed, const AuPoint& systemGalaxyLoc, float starLight,
		int scanEx, const DaAtomList& asteroids)
{
	// Show planets in current system even if not "explored".
	if(!scanEx && m_ShowCurrentSystem)
		scanEx = 1;

	QString sysname;
	int nl = systemName.indexOf("\n");
	if(nl >= 0)
		sysname = systemName.left(nl);
	else
		sysname = systemName;

	AuIdList aids;
	asteroids.ListIds(aids);
	if(m_System)
	{
		if(m_System->Is(systemId) && m_ScanEx == scanEx && m_SectorName == sectorName && m_System->Name() == sysname &&
				m_AsteroidIds == aids)
			return;
		m_Ether.DeleteEther();
	}
	m_AsteroidIds = aids;

	QString catname = DaSolarSystem::CatalogName(systemGalaxyLoc);
	// Determine sector coordinates.
	AuPoint sectorLoc(systemGalaxyLoc);
	sectorLoc.Round(10.);
	AuPoint systemLocSector = systemGalaxyLoc - sectorLoc;
	AuMatrix4x4 systemT;
	systemT.Translate(systemLocSector);

	m_SectorName = sectorName;
	m_ScanEx = scanEx;
	m_System = new DaSolarSystem;
	m_System->SetId(systemId);		// Needed for the ether.
	m_Ether.AddAtom(m_System);
	m_System->PrepareForResourceScan(catname, sysname, galaxy, genHab, genSeed, isCoreBlackHole, systemT, starLight);
	m_SystemPic->SetSystem(m_System, m_ScanEx);
	if(scanEx)
		m_Worms = ReportWormholes(sectorId, systemId);
	if(m_System->IsEmptySystem())
		m_Title->setText(tr("<html><b>%1</b> <small>Deep Space</small></html>").arg(m_System->Name()));
	else if(scanEx == 2)
		m_Title->setText(tr("<html><b>%1</b> <small>Observed, Explored, Surveyed</small></html>").arg(m_System->Name()));
	else if(scanEx == 1)
		m_Title->setText(tr("<html><b>%1</b> <small>Observed, Explored, Not Surveyed</small></html>").arg(m_System->Name()));
	else
		m_Title->setText(tr("<html><b>%1</b> <small>Observed, Not Explored, Not Surveyed</small></html>").arg(m_System->Name()));
	m_BodyItems.clear();
	QModelIndex ai = m_Bodies->model()->index(0,0);
	Q_ASSERT(ai.internalPointer() == m_AtomItem);
	m_Bodies->model()->removeRows(0, m_AtomItem->childCount(), ai);
	ListAtomRecursive(m_AtomItem, m_System);

	// Add asteroids
	m_AsteroidDesc.clear();
	for(DaAtomList::const_iterator ai = asteroids.cbegin(); ai != asteroids.cend(); ai++)
	{
		m_AsteroidDesc.insert((*ai)->Id(), (*ai)->SystemScanReport(scanEx));
		QTreeWidgetItem* subItem = new QTreeWidgetItem(m_AtomItem);
		ListAtomRecursive(subItem, *ai);
	}

	m_Bodies->resizeColumnToContents(0);
	m_BodyInfo->clear();
	m_MinQLabel->setVisible(m_ScanEx == 2);
	m_MinQToShow->setVisible(m_ScanEx == 2);
	m_LandAdd->setEnabled(!m_System->IsEmptySystem());
}

void AuSystemScan::SetSystem(const DaSolarSystem* system, quint32 empireId, AuSensorBlip::TYPE viewerBlip,
		POLITICAL_STANCE viewerStance, const AuPoint& viewerLoc)
{
	if(!system)
		return;	// System is not valid. Keep current system scan data.

	const DaSector* sector = system->FindParentSector();
	if(!sector)
		return;	// wut?

	const DaGalaxy* pg = sector->FindParentGalaxy();
	if(!pg)
		return;	// wut?

	int scanEx;
	if(system->IsScannedBy(empireId))
		scanEx = 2;
	else if(system->IsExploredBy(empireId))
		scanEx = 1;
	else
		scanEx = 0;

	DaAtomList asteroids;
	system->FindAsteroidsRecursive(asteroids);

	m_SystemPic->SetViewerLoc(viewerBlip, viewerStance, viewerLoc, asteroids);

	AuPoint systemGalaxyLoc;
	system->GetLocationRelativeToParent(systemGalaxyLoc, pg);
	SetSystem(sector->Id(), sector->SectorName(), system->Id(), system->Name(), pg->Galaxy(), system->IsCoreBlackHole(),
			system->GenHab(), system->GenSeed(), systemGalaxyLoc, system->StarLight(), scanEx, asteroids);
}

void AuSystemScan::ListAtomRecursive(QTreeWidgetItem* atomItem, const DaAtom* atom)
{
	m_BodyItems.insert(atom->Id(), atomItem);

	// Make a tree node for the atom.
	QString orbitZone;
	QColor orbitColor;
	const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(atom);
	if(system)
	{
		if(system->IsEmptySystem())
		{
			atomItem->setIcon(0, QIcon(":data/RangeSector.png"));
			if(m_SectorName.isEmpty())
				atomItem->setText(0, tr("Deep Space"));
			else
				atomItem->setText(0, m_SectorName);
		}
		else
		{
			atomItem->setIcon(0, QIcon(":data/RangeSystem.png"));
			atomItem->setText(0, atom->Name());
		}
	}
	else
	{
		const DaStar* star = dynamic_cast<const DaStar*>(atom);
		if(star)
		{
			switch(star->StarType())
			{
			case DaStar::TYPE_O:
				atomItem->setIcon(0, QIcon(":data/c_TypeOPreons.png"));
				break;
			case DaStar::TYPE_B:
				atomItem->setIcon(0, QIcon(":data/c_TypeBPreons.png"));
				break;
			case DaStar::TYPE_A:
				atomItem->setIcon(0, QIcon(":data/c_TypeAPreons.png"));
				break;
			case DaStar::TYPE_F:
				atomItem->setIcon(0, QIcon(":data/c_TypeFPreons.png"));
				break;
			case DaStar::TYPE_G:
				atomItem->setIcon(0, QIcon(":data/c_TypeGPreons.png"));
				break;
			case DaStar::TYPE_K:
				atomItem->setIcon(0, QIcon(":data/c_TypeKPreons.png"));
				break;
			case DaStar::TYPE_M:
				atomItem->setIcon(0, QIcon(":data/c_TypeMPreons.png"));
				break;
			case DaStar::TYPE_StellarBlackHole:
			case DaStar::TYPE_SupermassiveBlackHole:
				atomItem->setIcon(0, QIcon(":data/StarBlackHole.png"));
				break;
			case DaStar::TYPE_NeutronStar:
				atomItem->setIcon(0, QIcon(":data/StarNeutron.png"));
				break;
			}
			if(star->IsPrimary())
			{
				orbitZone = tr("Primary Star");
				orbitColor = QColor(255,255,255);
			}
			else
			{
				orbitZone = tr("Companion Star");
				orbitColor = QColor(96,96,192);
			}
		}
		else
		{
			const DaWorld* world = dynamic_cast<const DaWorld*>(atom);
			if(world && m_ScanEx)
			{
				atomItem->setIcon(0, QIcon(BodyTypeIcon(world->BodyType())));
				orbitZone = OrbitZoneName(world->OrbitZone());
				QColor penorbit[5] =
				{
					QColor(255,0,0),		// Inferno
					QColor(192,192,0),		// Inner
					QColor(0,255,0),		// Habitable
					QColor(0,192,192),		// Outer
					QColor(128,128,128)		// Frigid
				};
				orbitColor = penorbit[world->OrbitZone()];
			}
			else
			{
				const DaAsteroid* roid = dynamic_cast<const DaAsteroid*>(atom);
				if(roid && m_ScanEx)
				{
					atomItem->setIcon(0, QIcon(BodyTypeIcon(BODY_Planetoid)));
					orbitColor = QColor(255,128,64);
					orbitZone = QObject::tr("Eccentric");
				}
				else
					return;
			}
		}

		atomItem->setText(0, atom->Name());
	}
	atomItem->setData(0, Qt::UserRole, atom->Id().GetID());
	atomItem->setText(1, orbitZone);
	atomItem->setBackground(1, QColor(0,0,0));
	atomItem->setForeground(1, orbitColor);

	// Make tree nodes for its subatoms.
	if(atom->HasSubatoms())
	{
		DaAtomList subs;
		atom->ListNormalSubatoms(subs);

		// Make a map of sub stars and planets, sorted by distance from the parent atom.
		QMultiMap<double, const DaAtom*> starplanet;
		if(m_ScanEx)
		{
			// Show planets and stars.
			for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
			{
				double dist = (*ai)->Location().Length();
				const DaWorld* world = dynamic_cast<const DaWorld*>(*ai);
				if(world)
					dist += world->OrbitIndex() * 100;
				starplanet.insert(dist, *ai);
			}
		}
		else
		{
			// Show only stars.
			for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
			{
				double dist = (*ai)->Location().Length();
				const DaStar* star = dynamic_cast<const DaStar*>(*ai);
				if(!star)
					continue;
				starplanet.insert(dist, *ai);
			}
		}

		for(QMultiMap<double, const DaAtom*>::const_iterator ai = starplanet.begin(); ai != starplanet.end(); ai++)
		{
			QTreeWidgetItem* subItem = new QTreeWidgetItem(atomItem);
			ListAtomRecursive(subItem, *ai);
		}

		m_Bodies->expandItem(atomItem);
	}
}

QString AuSystemScan::ReportWormholes(const AuId& sectorId, const AuId& systemId)
{
	QString rpt;
	rpt += QObject::tr("<hr><b>Wormholes</b><br>");
	rpt += "<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(3, 18, 48);\">";
	rpt += "<tbody>";
	rpt += QObject::tr("<tr><td colspan=\"2\"><b>Polarity</b></td>"
			"<td><b>To System</b></td>"
			"<td><b>To Sector</b></td>"
			"<td><b>Explorer</b></td></tr>");

	if(m_ShowCurrentSystem)
	{
		DaPerson* person;
		AuView* pView = auApp->Frame()->View();
		pView->LockSceneAtom(&person);
		if(person)
		{
			const DaSolarSystem* system = person->FindParentSolarSystemInEnv();
			if(system)
				rpt += ReportSystemWormholes(system);
		}
		pView->UnlockSceneAtom();
	}
	else
	{
		// See if we have the system in our local data for better wormhole information.
		bool hasSystemLocally = false;
		DaPerson* person;
		AuView* pView = auApp->Frame()->View();
		pView->LockSceneAtom(&person);
		if(person)
		{
			const DaGalaxy* galaxy = person->FindParentGalaxy();
			if(galaxy)
			{
				const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(galaxy->FindIdRecursive(systemId));
				if(system)
				{
					rpt += ReportSystemWormholes(system);
					hasSystemLocally = true;
				}
			}
		}
		pView->UnlockSceneAtom();

		if(!hasSystemLocally)
		{
			const AuStarMapData& starMap = g_AuClient.LockStarMapForRead();
			unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI = starMap.m_Sectors.find(sectorId);
			if(sectI != starMap.m_Sectors.end())
			{
				unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator sysI = (*sectI).second.m_Systems.find(systemId);
				if(sysI != (*sectI).second.m_Systems.end())
				{
					for(QList<AuStarMapWormhole>::const_iterator wormI = (*sysI).second.m_Wormholes.cbegin();
							wormI != (*sysI).second.m_Wormholes.cend(); wormI++)
					{
						QString polarity;
						QString polarityIcon;
						switch((*wormI).Polarity())
						{
						case AuWormhole::Invalid:
						case AuWormhole::POLARITY_Count:
						default:
							continue;
						case AuWormhole::Positive:
							polarity = tr("Positive");
							polarityIcon = QString(":data/WormholePositive.png");
							break;
						case AuWormhole::Negative:
							polarity = tr("Negative");
							polarityIcon = QString(":data/WormholeNegative.png");
							break;
						case AuWormhole::Intergalactic:
						case AuWormhole::IntergalacticOneWay:
							polarityIcon = QString(":data/WormholeNeutral.png");
							polarity = tr("Neutral");
							break;
						}

						QString toSystem;
						QString toSector;
						if((*wormI).Polarity() == AuWormhole::Intergalactic)
						{
							toSystem = tr("A'A'A'a");
							GALAXY destGal = DaSolarSystem::WormholeDestGalaxy((GALAXY)(*sectI).second.m_Galaxy,
									(*wormI).VecToDest());
							toSector = QString("(0, 0, 0)<br>%2").arg(::GalaxyName(destGal));
						}
						else if((*wormI).Polarity() == AuWormhole::IntergalacticOneWay)
						{
							toSystem = tr("Unknown");
							GALAXY destGal = DaSolarSystem::WormholeDestGalaxy((GALAXY)(*sectI).second.m_Galaxy,
									(*wormI).VecToDest());
							AuPoint sLoc = (*sectI).second.m_Loc/10.0;
							toSector = QString("(%1, %2, %3)<br>%4")
								.arg(sLoc.x).arg(sLoc.y).arg(sLoc.z)
								.arg(::GalaxyName(destGal));
						}
						else
						{
							AuPoint sectDest = (*sectI).second.m_Loc + (*sysI).second.m_Loc + (*wormI).VecToDest();
							starMap.WormholeDest((*sectI).second.Galaxy(), sectDest, toSector, toSystem);
						}

						QString explorer;
						if((*wormI).IsExplored())
							explorer = tr("<span style=\"color: rgb(255, 85, 42);\">Insufficient Data</span>");
						else
							explorer = tr("<span style=\"color: rgb(42, 85, 255);\">Unexplored</span>");

						rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\">"
								"<td></td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
							.arg(polarityIcon)
							.arg(polarity)
							.arg(toSystem)
							.arg(toSector)
							.arg(explorer);
					}
				}
			}
			g_AuClient.UnlockStarMap();
		}
	}

	rpt += "</tbody></table>";
	return rpt;
}

void AuSystemScan::BodySplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/SystemScan/BodySplitter", m_BodySplitter->saveState());
}

void AuSystemScan::ScanSplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/SystemScan/ScanSplitter", m_ScanSplitter->saveState());
}

void AuSystemScan::BodySelected(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
	AuId atomId;
	if(current)
		atomId = AuId(current->data(0, Qt::UserRole).toUInt());

	m_SystemPic->SetSelected(atomId);

	if(atomId.IsNull())
		m_BodyInfo->clear();
	else if(!m_System->IsEmptySystem())
	{
		if(m_AsteroidDesc.contains(atomId))
		{
			m_BodyInfo->setHtml(m_AsteroidDesc[atomId]);
		}
		else
		{
			// Locate the atom in our generated system.
			DaAtomList atoms;
			atoms.append(m_System);
			m_System->ListNormalSubatomsRecursive(atoms);
			for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
				if((*ai)->Is(atomId))
				{
					const DaAtom* atom = *ai;
					// Append the wormholes information to the solar system report.
					if(dynamic_cast<const DaSolarSystem*>(atom))
						m_BodyInfo->setHtml(atom->SystemScanReport(m_ScanEx) + m_Worms);
					else
						m_BodyInfo->setHtml(atom->SystemScanReport(m_ScanEx));
					break;
				}
		}
	}
}

void AuSystemScan::BodyClicked(AuId bodyId)
{
	//qDebug() << "AuSystemScan::BodyClicked" << bodyId;
	QHash<AuId, QTreeWidgetItem*>::const_iterator bodyI = m_BodyItems.find(bodyId);
	if(bodyI != m_BodyItems.cend())
		m_Bodies->setCurrentItem(*bodyI);
	else if(m_Bodies->selectionModel())
		m_Bodies->selectionModel()->clear();
}

QString AuSystemScan::ReportSystemWormholes(const DaSolarSystem* system)
{
	QString rpt;
	const AuWormholeArray& worms = system->LockWormholes();
	for(AuWormholeArray::const_iterator wormI = worms.cbegin(); wormI != worms.cend(); wormI++)
	{
		QString polarity;
		QString polarityIcon;
		switch((*wormI).Polarity())
		{
		case AuWormhole::Invalid:
		case AuWormhole::POLARITY_Count:
		default:
			continue;
		case AuWormhole::Positive:
			polarity = tr("Positive");
			polarityIcon = QString(":data/WormholePositive.png");
			break;
		case AuWormhole::Negative:
			polarity = tr("Negative");
			polarityIcon = QString(":data/WormholeNegative.png");
			break;
		case AuWormhole::Intergalactic:
		case AuWormhole::IntergalacticOneWay:
			polarityIcon = QString(":data/WormholeNeutral.png");
			polarity = tr("Neutral");
			break;
		}

		QString toSystem;
		QString toSector;
		if((*wormI).Polarity() == AuWormhole::Intergalactic)
		{
			toSystem = tr("A'A'A'a");
			GALAXY destGal = DaSolarSystem::WormholeDestGalaxy(system->Galaxy(), (*wormI).SectorVecToDestination());
			toSector = QString("(0, 0, 0)<br>%2").arg(::GalaxyName(destGal));
		}
		else if((*wormI).Polarity() == AuWormhole::IntergalacticOneWay)
		{
			toSystem = tr("Unknown");
			GALAXY destGal = DaSolarSystem::WormholeDestGalaxy(system->Galaxy(), (*wormI).SectorVecToDestination());
			const DaSector* sector = system->FindParentSector();
			toSector = QString("%1<br>%2").arg(sector->Description()).arg(::GalaxyName(destGal));
		}
		else if(!(*wormI).ToSystem().IsNull())
		{
			const DaGalaxy* galaxy = system->FindParentGalaxy();
			if(galaxy)
			{
				const DaAtom* destSys = galaxy->FindIdRecursive((*wormI).ToSystem());
				if(destSys)
				{
					toSystem = destSys->Name();
					const DaSector* destSect = destSys->FindParentSector();
					if(destSect)
						toSector = destSect->SectorName();
				}
			}
		}

		QString explorer;
		if((*wormI).HasName())
			explorer = QString("<span style=\"color: rgb(255, 85, 42);\">%1</span>").arg((*wormI).Name());
		else if((*wormI).IsExplored())
			explorer = tr("<span style=\"color: rgb(255, 85, 42);\">Unknown</span>");
		else
			explorer = tr("<span style=\"color: rgb(42, 85, 255);\">Unexplored</span>");

		rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\">"
				"<td></td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
			.arg(polarityIcon)
			.arg(polarity)
			.arg(toSystem)
			.arg(toSector)
			.arg(explorer);
	}
	system->UnlockWormholes();
	return rpt;
}

void AuSystemScan::LandAdd()
{
	if(!m_System || m_System->IsEmptySystem())
		return;
	auApp->LandAddSystem(m_System->Id(), m_System->Name());
}

QSize AuSystemScan::sizeHint() const
{
	return QSize(800,600);
}

