//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austarmapsearchsystems.cpp,v 1.30 2022/02/19 18:17:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austarmapsearchsystems.h"
//
#include "auapp.h"
#include "aucheckbox.h"
#include "auclientglobal.h"
#include "aufilever.h"
#include "ausystemscan.h"
#include "dasolarsystem.h"
#include "daworldsphere.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressDialog>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSpinBox>

AuStarMapSearchSystems::AuStarMapSearchSystems(QWidget* parent) : AuDialog(parent), m_Ether("AuStarMapSearchSystems")
{
	setAuDlgTitle(tr("Star Map Solar System Search"));
	setAuDlgIcon(QIcon(":data/bObservatory.png"));
	m_LocationSet = false;
	m_ScanSaved = false;

	QByteArray ba;
	g_Au.Settings().readEntry("Widget/StarMapSearchSystems/LastSearch", ba);
	QDataStream in(ba);
	in >> m_Search;

	QGroupBox* rangeGroup = new QGroupBox(tr("Range"));
	{
		m_Range = new QSpinBox(this);
		m_Range->setRange(0, 10000);
		m_Range->setToolTip(tr("<html>"
					"Range of search, in pc. 0 ignores range."
					"</html>"));
		connect(m_Range, SIGNAL(valueChanged(int)), SLOT(RangeChanged(int)));

		QLabel* lbRange = new QLabel(tr("pc"), this);

		m_SearchLoc = new QLineEdit(this);
		m_SearchLoc->setToolTip(tr("<html>"
					"Search location for range test, in pc, relative to the galaxy."
					"</html>"));
		connect(m_SearchLoc, SIGNAL(textChanged(const QString&)), SLOT(SearchLocChanged(const QString&)));

		// Sort galaxy names alphabetically.
		QMap<QString, QVariant> galaxies;
		for(int galaxy=AndromedaRising; galaxy<GALAXY_Count; galaxy++)
			galaxies.insert(GalaxyName((GALAXY)galaxy), QVariant(galaxy));

		m_Galaxy = new QComboBox(this);
		for(QMap<QString, QVariant>::const_iterator gi = galaxies.cbegin(); gi != galaxies.cend(); gi++)
			m_Galaxy->addItem(gi.key(), *gi);
		m_Galaxy->setCurrentIndex(m_Galaxy->findData(QVariant((GALAXY)m_Search.m_Galaxy)));
		m_Galaxy->setToolTip(tr("<html>Galaxy for range test.</html>"));
		connect(m_Galaxy, SIGNAL(currentIndexChanged(int)), SLOT(GalaxyChanged(int)));

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_Range, row,0, 1,2);
		glay->addWidget(lbRange, row,2, 1,1);
		row++;
		glay->addWidget(m_SearchLoc, row,1, 1,2);
		row++;
		glay->addWidget(m_Galaxy, row,1, 1,2);
		row++;
		glay->setColumnMinimumWidth(0, 20);

		rangeGroup->setLayout(glay);
	}

	QGroupBox* nameGroup = new QGroupBox(tr("Name"));
	{
		m_Name = new QLineEdit(this);
		m_Name->setToolTip(tr("<html>"
					"Name to find. Leave blank to ignore names."
					"</html>"));
		connect(m_Name, SIGNAL(textChanged(const QString&)), SLOT(NameChanged(const QString&)));

		m_Case = new QCheckBox(tr("Case Sensitive"), this);
		m_Case->setToolTip(tr("<html>"
					"When checked, comparisons are case sensitive. "
					"</html>"));
		connect(m_Case, SIGNAL(stateChanged(int)), SLOT(CaseChanged(int)));

		m_RegEx = new QCheckBox(tr("Regular Expression Pattern Matching"), this);
		m_RegEx->setToolTip(tr("<html>"
					"When checked, regular expression pattern matching is used, "
					"</html>"));
		connect(m_RegEx, SIGNAL(stateChanged(int)), SLOT(RegExChanged(int)));

		m_SystemName = new QCheckBox(tr("Solar System Names"), this);
		m_SystemName->setToolTip(tr("<html>"
					"When checked, names of solar systems are compared. "
					"<b>Solar system names can only be compared in systems explored by your empire.</b> "
					"</html>"));
		connect(m_SystemName, SIGNAL(stateChanged(int)), SLOT(SystemNameChanged(int)));

		m_SystemCat = new QCheckBox(tr("Solar System Catalog Names"), this);
		m_SystemCat->setToolTip(tr("<html>"
					"When checked, catalog names of solar systems are compared. "
					"Catalog names can be compared in all systems, even if not explored by your empire. "
					"</html>"));
		connect(m_SystemCat, SIGNAL(stateChanged(int)), SLOT(SystemCatChanged(int)));

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_Name, row,0, 1,2);
		row++;
		glay->addWidget(m_Case, row,1, 1,1);
		row++;
		glay->addWidget(m_RegEx, row,1, 1,1);
		row++;
		glay->addWidget(m_SystemName, row,1, 1,1);
		row++;
		glay->addWidget(m_SystemCat, row,1, 1,1);
		row++;
		glay->setColumnMinimumWidth(0, 20);

		nameGroup->setLayout(glay);
	}

	QGroupBox* natGroup = new QGroupBox(tr("Natural Resources"));
	{
		QLabel* lbnatico = new QLabel(this);
		lbnatico->setPixmap(QPixmap(":data/SolarSystemInfo.png").scaled(16,16));
		lbnatico->setAlignment(Qt::AlignRight|Qt::AlignTop);

		QLabel* lbnat = new QLabel(tr("<html>"
					"<small>"
					"Natural resources can only be found in systems scanned by your empire."
					"</small>"
					"</html>"), this);
		lbnat->setWordWrap(true);

		// List all natural resource commodities with check boxes.
		QScrollArea* natScroller = new QScrollArea;
		{
			QWidget* nat = new QWidget;

			QGridLayout* elay = new QGridLayout(nat);
			int erow=0;

			// Alphabetize the list of empires known to the viewer's empire using a map.
			QMap<QString, COMMODITYID> sortedCIDs;	// commodities sorted alphabetically
			for(DbCommodityArray::const_iterator cit = g_Au.m_CommodityArray.cbegin(); cit != g_Au.m_CommodityArray.cend(); cit++)
				if((*cit).IsNaturalResource())
					sortedCIDs.insert((*cit).m_name, (*cit).m_id);

			// Create a check box for every natural resource.
			for(QMap<QString, COMMODITYID>::const_iterator si = sortedCIDs.cbegin(); si != sortedCIDs.cend(); si++)
			{
				QString name(si.key());

				AuCheckBoxValue* cb = new AuCheckBoxValue(name, *si);
				cb->setToolTip(tr("<html>"
							"When checked, %1 must be present in the system, at the minimum quality specified."
							"</html>").arg(name));
				m_NaturalResources.append(cb);
				connect(cb, SIGNAL(StateChanged(int, int)), SLOT(NaturalResourceStateChanged(int, int)));

				QLabel* lbres = new QLabel;
				QPixmap pix(g_Au.CommodityBitmapName(*si));
				lbres->setPixmap(pix.scaled(16,16));

				elay->addWidget(lbres, erow,0);
				elay->addWidget(cb, erow,1);
				erow++;
			}
			nat->setLayout(elay);
			natScroller->setWidget(nat);
		}

		QPushButton* allNaturalResources = new QPushButton(tr("All"), this);
		allNaturalResources->setToolTip(tr("<html>"
					"Checks the check box for all natural resources."
					"</html>"));
		connect(allNaturalResources, SIGNAL(clicked()), SLOT(CheckAllNaturalResources()));

		QPushButton* noNaturalResources = new QPushButton(tr("None"), this);
		noNaturalResources->setToolTip(tr("<html>"
					"Unchecks the check box for all natural resources."
					"</html>"));
		connect(noNaturalResources, SIGNAL(clicked()), SLOT(UncheckAllNaturalResources()));

		QPushButton* invertNaturalResources = new QPushButton(tr("Invert"), this);
		invertNaturalResources->setToolTip(tr("<html>"
					"Inverts the check box for all natural resources. "
					"Check boxes that are checked will be unchecked. "
					"Check boxes that are unchecked will be checked."
					"</html>"));
		connect(invertNaturalResources, SIGNAL(clicked()), SLOT(InvertCheckedNaturalResources()));

		QLabel* lbminq = new QLabel(tr("Minimum Quality"));
		lbminq->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		// Spin box for minimum quality.
		m_MinQuality = new QSpinBox(this);
		m_MinQuality->setRange(1,255);
		m_MinQuality->setToolTip(tr("<html>Minimum quality natural resources to locate.</html>"));
		connect(m_MinQuality, SIGNAL(valueChanged(int)), SLOT(MinQualityChanged(int)));

		m_ResAnd = new QRadioButton(tr("All Natural Resources must be found at the Minimum Quality."), this);
		m_ResAnd->setToolTip(tr("<html>"
					"All checked natural resources must be found in a system, at the minimum quality specified, "
					"to match the search. "
					"</html>"));
		m_ResOr = new QRadioButton(tr("One Natural Resources must be found at the Minimum Quality."), this);
		m_ResOr->setToolTip(tr("<html>"
					"At least one of the checked natural resources must be found in a system, at the minimum quality specified, "
					"to match the search. "
					"</html>"));
		if(m_Search.m_AndOr)
		{
			// OR
			m_ResOr->setChecked(true);
			m_ResAnd->setChecked(false);
		}
		else
		{
			// AND
			m_ResAnd->setChecked(true);
			m_ResOr->setChecked(false);
		}
		connect(m_ResAnd, SIGNAL(clicked()), SLOT(ResAndClicked()));
		connect(m_ResOr, SIGNAL(clicked()), SLOT(ResOrClicked()));

		QHBoxLayout* nblay = new QHBoxLayout;
		nblay->addWidget(allNaturalResources);
		nblay->addWidget(noNaturalResources);
		nblay->addWidget(invertNaturalResources);

		QHBoxLayout* footnotelay = new QHBoxLayout;
		footnotelay->addWidget(lbnatico);
		footnotelay->addWidget(lbnat, 1);

		QGridLayout* nlay = new QGridLayout;
		int row=0;
		nlay->addLayout(nblay, row,0, 1,3);
		row++;
		nlay->addWidget(natScroller, row,1, 1,2);
		row++;
		nlay->addWidget(lbminq, row,1, 1,1);
		nlay->addWidget(m_MinQuality, row,2, 1,1);
		row++;
		nlay->addWidget(m_ResAnd, row,1, 1,2);
		row++;
		nlay->addWidget(m_ResOr, row,1, 1,2);
		row++;
		nlay->addLayout(footnotelay, row,0, 1,3);
		row++;
		nlay->setColumnMinimumWidth(0, 20);

		natGroup->setLayout(nlay);
	}

	QGroupBox* sunGroup = new QGroupBox(tr("Suns"));
	{
		QLabel* lbsunsico = new QLabel(this);
		lbsunsico->setPixmap(QPixmap(":data/SolarSystemInfo.png").scaled(16,16));
		lbsunsico->setAlignment(Qt::AlignRight|Qt::AlignTop);

		QLabel* lbsuns = new QLabel(tr("<html>"
					"<small>"
					"Suns can only be found in systems scanned by your empire."
					"</small>"
					"</html>"), this);
		lbsuns->setWordWrap(true);

		QLabel* lbMinSuns = new QLabel(tr("Minimum Suns Present"), this);

		m_Stars = new QSpinBox(this);
		m_Stars->setRange(0,3);
		m_Stars->setToolTip(tr("<html>Minimum number of suns in the system. "
					"The count includes black holes and neutron stars. "
					"<br>"
					"Set to 0 to ignore sun tests."
					"</html>"));
		connect(m_Stars, SIGNAL(valueChanged(int)), SLOT(StarsChanged(int)));

		QLabel* lbBlackHole = new QLabel(this);
		lbBlackHole->setPixmap(QPixmap(":data/StarBlackHole.png").scaled(16,16));

		m_BlackHole = new QCheckBox(tr("Black Hole"), this);
		m_BlackHole->setToolTip(tr("<html>"
					"When checked, a black hole is wanted in the system. "
					"A solar system is found if any of the checked sun types is found. "
					"</html>"));
		connect(m_BlackHole, SIGNAL(stateChanged(int)), SLOT(BlackHoleChanged(int)));

		QLabel* lbNeutronStar = new QLabel(this);
		lbNeutronStar->setPixmap(QPixmap(":data/StarNeutron.png").scaled(16,16));

		m_NeutronStar = new QCheckBox(tr("Neutron Star"), this);
		m_NeutronStar->setToolTip(tr("<html>"
					"When checked, a neutron star is wanted in the system. "
					"A solar system is found if any of the checked sun types is found. "
					"</html>"));
		connect(m_NeutronStar, SIGNAL(stateChanged(int)), SLOT(NeutronStarChanged(int)));

		QLabel* lbStarTypeO = new QLabel(this);
		lbStarTypeO->setPixmap(QPixmap(":data/c_TypeOPreons.png").scaled(16,16));

		m_StarTypeO = new QCheckBox(tr("Type O Star"), this);
		m_StarTypeO->setToolTip(tr("<html>"
					"When checked, a type O star is wanted in the system. "
					"A solar system is found if any of the checked sun types is found. "
					"</html>"));
		connect(m_StarTypeO, SIGNAL(stateChanged(int)), SLOT(StarTypeOChanged(int)));

		QLabel* lbStarTypeB = new QLabel(this);
		lbStarTypeB->setPixmap(QPixmap(":data/c_TypeBPreons.png").scaled(16,16));

		m_StarTypeB = new QCheckBox(tr("Type B Star"), this);
		m_StarTypeB->setToolTip(tr("<html>"
					"When checked, a type B star is wanted in the system. "
					"A solar system is found if any of the checked sun types is found. "
					"</html>"));
		connect(m_StarTypeB, SIGNAL(stateChanged(int)), SLOT(StarTypeBChanged(int)));

		QLabel* lbStarTypeA = new QLabel(this);
		lbStarTypeA->setPixmap(QPixmap(":data/c_TypeAPreons.png").scaled(16,16));

		m_StarTypeA = new QCheckBox(tr("Type A Star"), this);
		m_StarTypeA->setToolTip(tr("<html>"
					"When checked, a type A star is wanted in the system. "
					"A solar system is found if any of the checked sun types is found. "
					"</html>"));
		connect(m_StarTypeA, SIGNAL(stateChanged(int)), SLOT(StarTypeAChanged(int)));

		QLabel* lbStarTypeF = new QLabel(this);
		lbStarTypeF->setPixmap(QPixmap(":data/c_TypeFPreons.png").scaled(16,16));

		m_StarTypeF = new QCheckBox(tr("Type F Star"), this);
		m_StarTypeF->setToolTip(tr("<html>"
					"When checked, a type F star is wanted in the system. "
					"A solar system is found if any of the checked sun types is found. "
					"</html>"));
		connect(m_StarTypeF, SIGNAL(stateChanged(int)), SLOT(StarTypeFChanged(int)));

		QLabel* lbStarTypeG = new QLabel(this);
		lbStarTypeG->setPixmap(QPixmap(":data/c_TypeGPreons.png").scaled(16,16));

		m_StarTypeG = new QCheckBox(tr("Type G Star"), this);
		m_StarTypeG->setToolTip(tr("<html>"
					"When checked, a type G star is wanted in the system. "
					"A solar system is found if any of the checked sun types is found. "
					"</html>"));
		connect(m_StarTypeG, SIGNAL(stateChanged(int)), SLOT(StarTypeGChanged(int)));

		QLabel* lbStarTypeK = new QLabel(this);
		lbStarTypeK->setPixmap(QPixmap(":data/c_TypeKPreons.png").scaled(16,16));

		m_StarTypeK = new QCheckBox(tr("Type K Star"), this);
		m_StarTypeK->setToolTip(tr("<html>"
					"When checked, a type K star is wanted in the system. "
					"A solar system is found if any of the checked sun types is found. "
					"</html>"));
		connect(m_StarTypeK, SIGNAL(stateChanged(int)), SLOT(StarTypeKChanged(int)));

		QLabel* lbStarTypeM = new QLabel(this);
		lbStarTypeM->setPixmap(QPixmap(":data/c_TypeMPreons.png").scaled(16,16));

		m_StarTypeM = new QCheckBox(tr("Type M Star"), this);
		m_StarTypeM->setToolTip(tr("<html>"
					"When checked, a type M star is wanted in the system. "
					"A solar system is found if any of the checked sun types is found. "
					"</html>"));
		connect(m_StarTypeM, SIGNAL(stateChanged(int)), SLOT(StarTypeMChanged(int)));

		QHBoxLayout* slay = new QHBoxLayout;
		slay->addWidget(lbMinSuns);
		slay->addWidget(m_Stars);
		slay->addStretch();

		QHBoxLayout* footnotelay = new QHBoxLayout;
		footnotelay->addWidget(lbsunsico);
		footnotelay->addWidget(lbsuns, 1);

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addLayout(slay, row,0, 1,3);
		row++;
		glay->addWidget(lbBlackHole, row,1, 1,1);
		glay->addWidget(m_BlackHole, row,2, 1,1);
		row++;
		glay->addWidget(lbNeutronStar, row,1, 1,1);
		glay->addWidget(m_NeutronStar, row,2, 1,1);
		row++;
		glay->addWidget(lbStarTypeO, row,1, 1,1);
		glay->addWidget(m_StarTypeO, row,2, 1,1);
		row++;
		glay->addWidget(lbStarTypeB, row,1, 1,1);
		glay->addWidget(m_StarTypeB, row,2, 1,1);
		row++;
		glay->addWidget(lbStarTypeA, row,1, 1,1);
		glay->addWidget(m_StarTypeA, row,2, 1,1);
		row++;
		glay->addWidget(lbStarTypeF, row,1, 1,1);
		glay->addWidget(m_StarTypeF, row,2, 1,1);
		row++;
		glay->addWidget(lbStarTypeG, row,1, 1,1);
		glay->addWidget(m_StarTypeG, row,2, 1,1);
		row++;
		glay->addWidget(lbStarTypeK, row,1, 1,1);
		glay->addWidget(m_StarTypeK, row,2, 1,1);
		row++;
		glay->addWidget(lbStarTypeM, row,1, 1,1);
		glay->addWidget(m_StarTypeM, row,2, 1,1);
		row++;
		glay->addLayout(footnotelay, row,0, 1,3);
		row++;
		glay->setColumnMinimumWidth(0, 20);
		glay->setColumnMinimumWidth(1, 20);

		sunGroup->setLayout(glay);
	}

	QGroupBox* worldGroup = new QGroupBox(tr("Worlds"));
	{
		QLabel* lbworldsico = new QLabel(this);
		lbworldsico->setPixmap(QPixmap(":data/SolarSystemInfo.png").scaled(16,16));
		lbworldsico->setAlignment(Qt::AlignRight|Qt::AlignTop);

		QLabel* lbworlds = new QLabel(tr("<html>"
					"<small>"
					"Worlds can only be found in systems scanned by your empire."
					"</small>"
					"</html>"), this);
		lbworlds->setWordWrap(true);

		QLabel* lbMinWorlds = new QLabel(tr("Minimum Worlds Present"), this);

		m_Worlds = new QSpinBox(this);
		m_Worlds->setRange(0,100);
		m_Worlds->setToolTip(tr("<html>Minimum number of worlds in the system. "
					"The count includes planets, moons, rings, ringworlds, and gas giants. "
					"When counting ringworlds, the entire ringworld counts as one world. "
					"<br>"
					"Set to 0 to ignore world tests."
					"</html>"));
		connect(m_Worlds, SIGNAL(valueChanged(int)), SLOT(WorldsChanged(int)));

		// Planets and Moons
		QLabel* lbpicon = new QLabel(this);
		lbpicon->setPixmap(QPixmap(":data/WorldPlanet.png").scaled(32,32));

		QLabel* lbmicon = new QLabel(this);
		lbmicon->setPixmap(QPixmap(":data/WorldMoon.png").scaled(16,16));
		lbmicon->setAlignment(Qt::AlignLeft|Qt::AlignTop);

		QLabel* lbp = new QLabel(tr("Planets and Moons"), this);

		m_GlobeInferno = new QComboBox(this);
		m_GlobeInferno->addItem(tr("Ignore Planets and Moons in Inferno Zone"), 0);
		for(int sz=10; sz<=128; sz+=10)
		{
			double dia = ::Radius100km(sz) / FeetPerMeter() * 2;
			m_GlobeInferno->addItem(tr("Minimum %L1m Planet or Moon in Inferno Zone").arg(dia, 0,'f',0), sz);
		}
		m_GlobeInferno->setToolTip(tr("<html>Minimum size planet or moon wanted in the inferno zone. "
					"A solar system is found if a large enough planet or moon exists in any of the checked orbit zones.</html>"));
		connect(m_GlobeInferno, SIGNAL(currentIndexChanged(int)), SLOT(GlobeInfernoChanged(int)));

		m_GlobeInner = new QComboBox(this);
		m_GlobeInner->addItem(tr("Ignore Planets and Moons in Inner Zone"), 0);
		for(int sz=10; sz<=128; sz+=10)
		{
			double dia = ::Radius100km(sz) / FeetPerMeter() * 2;
			m_GlobeInner->addItem(tr("Minimum %L1m Planet or Moon in Inner Zone").arg(dia, 0,'f',0), sz);
		}
		m_GlobeInner->setToolTip(tr("<html>Minimum size planet or moon wanted in the inner zone. "
					"A solar system is found if a large enough planet or moon exists in any of the checked orbit zones.</html>"));
		connect(m_GlobeInner, SIGNAL(currentIndexChanged(int)), SLOT(GlobeInnerChanged(int)));

		m_GlobeHabitable = new QComboBox(this);
		m_GlobeHabitable->addItem(tr("Ignore Planets and Moons in Habitable Zone"), 0);
		for(int sz=10; sz<=128; sz+=10)
		{
			double dia = ::Radius100km(sz) / FeetPerMeter() * 2;
			m_GlobeHabitable->addItem(tr("Minimum %L1m Planet or Moon in Habitable Zone").arg(dia, 0,'f',0), sz);
		}
		m_GlobeHabitable->setToolTip(tr("<html>Minimum size planet or moon wanted in the habitable zone. "
					"A solar system is found if a large enough planet or moon exists in any of the checked orbit zones.</html>"));
		connect(m_GlobeHabitable, SIGNAL(currentIndexChanged(int)), SLOT(GlobeHabitableChanged(int)));

		m_GlobeOuter = new QComboBox(this);
		m_GlobeOuter->addItem(tr("Ignore Planets and Moons in Outer Zone"), 0);
		for(int sz=10; sz<=128; sz+=10)
		{
			double dia = ::Radius100km(sz) / FeetPerMeter() * 2;
			m_GlobeOuter->addItem(tr("Minimum %L1m Planet or Moon in Outer Zone").arg(dia, 0,'f',0), sz);
		}
		m_GlobeOuter->setToolTip(tr("<html>Minimum size planet or moon wanted in the outer zone. "
					"A solar system is found if a large enough planet or moon exists in any of the checked orbit zones.</html>"));
		connect(m_GlobeOuter, SIGNAL(currentIndexChanged(int)), SLOT(GlobeOuterChanged(int)));

		m_GlobeFrigid = new QComboBox(this);
		m_GlobeFrigid->addItem(tr("Ignore Planets and Moons in Frigid Zone"), 0);
		for(int sz=10; sz<=128; sz+=10)
		{
			double dia = ::Radius100km(sz) / FeetPerMeter() * 2;
			m_GlobeFrigid->addItem(tr("Minimum %L1m Planet or Moon in Frigid Zone").arg(dia, 0,'f',0), sz);
		}
		m_GlobeFrigid->setToolTip(tr("<html>Minimum size planet or moon wanted in the frigid zone. "
					"A solar system is found if a large enough planet or moon exists in any of the checked orbit zones.</html>"));
		connect(m_GlobeFrigid, SIGNAL(currentIndexChanged(int)), SLOT(GlobeFrigidChanged(int)));

		// Gas Giants
		QLabel* lbggicon = new QLabel(this);
		lbggicon->setPixmap(QPixmap(":data/WorldGasGiant.png").scaled(32,32));

		QLabel* lbgg = new QLabel(tr("Gas Giants"), this);

		m_GasGiantInferno = new QCheckBox(tr("Gas Giant in Inferno Zone"), this);
		m_GasGiantInferno->setToolTip(tr("<html>"
					"When checked, gas giants are found in the inferno orbit zone. "
					"A solar system is found if a gas giant exists in any of the checked orbit zones. "
					"</html>"));
		connect(m_GasGiantInferno, SIGNAL(stateChanged(int)), SLOT(GasGiantInfernoChanged(int)));

		m_GasGiantInner = new QCheckBox(tr("Gas Giant in Inner Zone"), this);
		m_GasGiantInner->setToolTip(tr("<html>"
					"When checked, gas giants are found in the inner orbit zone. "
					"A solar system is found if a gas giant exists in any of the checked orbit zones. "
					"</html>"));
		connect(m_GasGiantInner, SIGNAL(stateChanged(int)), SLOT(GasGiantInnerChanged(int)));

		m_GasGiantHabitable = new QCheckBox(tr("Gas Giant in Habitable Zone"), this);
		m_GasGiantHabitable->setToolTip(tr("<html>"
					"When checked, gas giants are found in the habitable orbit zone. "
					"A solar system is found if a gas giant exists in any of the checked orbit zones. "
					"</html>"));
		connect(m_GasGiantHabitable, SIGNAL(stateChanged(int)), SLOT(GasGiantHabitableChanged(int)));

		m_GasGiantOuter = new QCheckBox(tr("Gas Giant in Outer Zone"), this);
		m_GasGiantOuter->setToolTip(tr("<html>"
					"When checked, gas giants are found in the outer orbit zone. "
					"A solar system is found if a gas giant exists in any of the checked orbit zones. "
					"</html>"));
		connect(m_GasGiantOuter, SIGNAL(stateChanged(int)), SLOT(GasGiantOuterChanged(int)));

		m_GasGiantFrigid = new QCheckBox(tr("Gas Giant in Frigid Zone"), this);
		m_GasGiantFrigid->setToolTip(tr("<html>"
					"When checked, gas giants are found in the frigid orbit zone. "
					"A solar system is found if a gas giant exists in any of the checked orbit zones. "
					"</html>"));
		connect(m_GasGiantFrigid, SIGNAL(stateChanged(int)), SLOT(GasGiantFrigidChanged(int)));

		// Rings
		QLabel* lbricon = new QLabel(this);
		lbricon->setPixmap(QPixmap(":data/WorldRing.png").scaled(32,32));

		QLabel* lbr = new QLabel(tr("Rings"), this);

		m_RingInferno = new QCheckBox(tr("Ring in Inferno Zone"), this);
		m_RingInferno->setToolTip(tr("<html>"
					"When checked, rings are found in the inferno orbit zone. "
					"A solar system is found if a ring exists in any of the checked orbit zones. "
					"</html>"));
		connect(m_RingInferno, SIGNAL(stateChanged(int)), SLOT(RingInfernoChanged(int)));

		m_RingInner = new QCheckBox(tr("Ring in Inner Zone"), this);
		m_RingInner->setToolTip(tr("<html>"
					"When checked, rings are found in the inner orbit zone. "
					"A solar system is found if a ring exists in any of the checked orbit zones. "
					"</html>"));
		connect(m_RingInner, SIGNAL(stateChanged(int)), SLOT(RingInnerChanged(int)));

		m_RingHabitable = new QCheckBox(tr("Ring in Habitable Zone"), this);
		m_RingHabitable->setToolTip(tr("<html>"
					"When checked, rings are found in the habitable orbit zone. "
					"A solar system is found if a ring exists in any of the checked orbit zones. "
					"</html>"));
		connect(m_RingHabitable, SIGNAL(stateChanged(int)), SLOT(RingHabitableChanged(int)));

		m_RingOuter = new QCheckBox(tr("Ring in Outer Zone"), this);
		m_RingOuter->setToolTip(tr("<html>"
					"When checked, rings are found in the outer orbit zone. "
					"A solar system is found if a ring exists in any of the checked orbit zones. "
					"</html>"));
		connect(m_RingOuter, SIGNAL(stateChanged(int)), SLOT(RingOuterChanged(int)));

		m_RingFrigid = new QCheckBox(tr("Ring in Frigid Zone"), this);
		m_RingFrigid->setToolTip(tr("<html>"
					"When checked, rings are found in the frigid orbit zone. "
					"A solar system is found if a ring exists in any of the checked orbit zones. "
					"</html>"));
		connect(m_RingFrigid, SIGNAL(stateChanged(int)), SLOT(RingFrigidChanged(int)));

		// Ringworlds
		QLabel* lbRwIcon = new QLabel(this);
		lbRwIcon->setPixmap(QPixmap(":data/WorldRingworld.png").scaled(32,16));

		QLabel* lbRwArcs = new QLabel(tr("Minimum Ringworld Arcs Present"), this);

		m_RingworldArcs = new QSpinBox(this);
		m_RingworldArcs->setRange(0,100);
		m_RingworldArcs->setToolTip(tr("<html>Minimum number of ringworld arcs in the system. "
					"Ringworld arcs are always found in the habitable zone. "
					"More than one ringworld can exist in a solar system. "
					"</html>"));
		connect(m_RingworldArcs, SIGNAL(valueChanged(int)), SLOT(RingworldArcsChanged(int)));

		// Layouts
		QHBoxLayout* wlay = new QHBoxLayout;
		wlay->addWidget(lbMinWorlds);
		wlay->addWidget(m_Worlds);
		wlay->addStretch();

		QHBoxLayout* rwlay = new QHBoxLayout;
		rwlay->addWidget(lbRwIcon);
		rwlay->addWidget(lbRwArcs);
		rwlay->addWidget(m_RingworldArcs);
		rwlay->addStretch();

		QHBoxLayout* gglay = new QHBoxLayout;
		gglay->addWidget(lbggicon);
		gglay->addWidget(lbgg);
		gglay->addStretch();

		QHBoxLayout* play = new QHBoxLayout;
		play->addWidget(lbpicon);
		play->addWidget(lbmicon);
		play->addWidget(lbp);
		play->addStretch();

		QHBoxLayout* rlay = new QHBoxLayout;
		rlay->addWidget(lbricon);
		rlay->addWidget(lbr);
		rlay->addStretch();

		QHBoxLayout* footnotelay = new QHBoxLayout;
		footnotelay->addWidget(lbworldsico);
		footnotelay->addWidget(lbworlds, 1);

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addLayout(wlay, row,0, 1,3);
		row++;
		glay->addLayout(play, row,1, 1,2);
		row++;
		glay->addWidget(m_GlobeInferno, row,2, 1,1);
		row++;
		glay->addWidget(m_GlobeInner, row,2, 1,1);
		row++;
		glay->addWidget(m_GlobeHabitable, row,2, 1,1);
		row++;
		glay->addWidget(m_GlobeOuter, row,2, 1,1);
		row++;
		glay->addWidget(m_GlobeFrigid, row,2, 1,1);
		row++;
		glay->addLayout(gglay, row,1, 1,2);
		row++;
		glay->addWidget(m_GasGiantInferno, row,2, 1,1);
		row++;
		glay->addWidget(m_GasGiantInner, row,2, 1,1);
		row++;
		glay->addWidget(m_GasGiantHabitable, row,2, 1,1);
		row++;
		glay->addWidget(m_GasGiantOuter, row,2, 1,1);
		row++;
		glay->addWidget(m_GasGiantFrigid, row,2, 1,1);
		row++;
		glay->addLayout(rlay, row,1, 1,2);
		row++;
		glay->addWidget(m_RingInferno, row,2, 1,1);
		row++;
		glay->addWidget(m_RingInner, row,2, 1,1);
		row++;
		glay->addWidget(m_RingHabitable, row,2, 1,1);
		row++;
		glay->addWidget(m_RingOuter, row,2, 1,1);
		row++;
		glay->addWidget(m_RingFrigid, row,2, 1,1);
		row++;
		glay->addLayout(rwlay, row,1, 1,2);
		row++;
		glay->addLayout(footnotelay, row,0, 1,3);
		row++;
		glay->setColumnMinimumWidth(0, 20);
		glay->setColumnMinimumWidth(1, 20);

		worldGroup->setLayout(glay);
	}

	m_ClearFilter = new QPushButton(QIcon(":data/Sweep.png"), tr("Clear Settings"), this);
	m_ClearFilter->setToolTip(tr("<html>"
				"Clears search settings and previous search result. "
				"<br>"
				"If this window was accessed from the right-click menu on the star map, "
				"<i>Range</i> settings will not change when search settings are cleared. "
				"</html>"));
	connect(m_ClearFilter, SIGNAL(clicked()), SLOT(ClearFilter()));

	m_LoadFilter = new QPushButton(QIcon(":data/dp_Load.png"), tr("Load Settings..."), this);
	m_LoadFilter->setToolTip(tr("<html>"
				"Loads search settings from a file. "
				"<br>"
				"If this window was accessed from the right-click menu on the star map, "
				"<i>Range</i> settings will not change when search settings are loaded. "
				"</html>"));
	connect(m_LoadFilter, SIGNAL(clicked()), SLOT(LoadFilter()));

	m_SaveFilter = new QPushButton(QIcon(":data/dp_Save.png"), tr("Save Settings..."), this);
	m_SaveFilter->setToolTip(tr("<html>"
				"Saves search settings to a file. "
				"</html>"));
	connect(m_SaveFilter, SIGNAL(clicked()), SLOT(SaveFilter()));

	m_SearchButton = new QPushButton(QIcon(":data/Magnifier.png"), tr("Search Now"), this);
	m_SearchButton->setAutoDefault(true);
	m_SearchButton->setDefault(true);
	m_SearchButton->setToolTip(tr("<html>"
				"Conducts a search of the current star map. "
				"Updates the list with solar systems that match. "
				"</html>"));
	connect(m_SearchButton, SIGNAL(clicked()), SLOT(SearchNow()));

	m_SearchResult = new QLabel(this);
	m_SearchResult->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);

	m_Result = new QListWidget(this);
	m_Result->setToolTip(tr("<html>"
				"Solar systems that match the search are listed. "
				"Click a solar system to center the star map on its location. "
				"</html>"));
	connect(m_Result, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(SearchResultClicked(QListWidgetItem*)));

	m_Scan = new AuSystemScan(false, this);

	QVBoxLayout* blay = new QVBoxLayout;
	blay->addWidget(m_SaveFilter);
	blay->addWidget(m_LoadFilter);
	blay->addStretch();
	blay->addWidget(m_ClearFilter);
	blay->addStretch();
	blay->addWidget(m_SearchResult);
	blay->addStretch();
	blay->addWidget(m_SearchButton);

	QVBoxLayout* vlay1 = new QVBoxLayout;
	vlay1->addWidget(rangeGroup);
	vlay1->addWidget(nameGroup);
	vlay1->addWidget(sunGroup);

	QVBoxLayout* vlay2 = new QVBoxLayout;
	vlay2->addWidget(worldGroup);

	QVBoxLayout* vlay3 = new QVBoxLayout;
	vlay3->addWidget(natGroup);

	QHBoxLayout* hlay = new QHBoxLayout;
	hlay->addLayout(vlay1);
	hlay->addLayout(vlay2);
	hlay->addLayout(vlay3);

	QHBoxLayout* hlay2 = new QHBoxLayout;
	hlay2->addWidget(m_Scan, 1);
	hlay2->addWidget(m_Result);
	hlay2->addLayout(blay);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(hlay);
	lay->addLayout(hlay2);
	setAuDlgLayout(lay);

	RestoreScannedSystemsCache();
}

AuStarMapSearchSystems::~AuStarMapSearchSystems()
{
	SaveScannedSystemsCache();
}

void AuStarMapSearchSystems::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
		PostFilter();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuStarMapSearchSystems::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/StarMapSearchSystems/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/StarMapSearchSystems/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/StarMapSearchSystems/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/StarMapSearchSystems/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuStarMapSearchSystems::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/StarMapSearchSystems/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/StarMapSearchSystems/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/StarMapSearchSystems/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/StarMapSearchSystems/SizeY",sz.height());
}

void AuStarMapSearchSystems::SearchNow()
{
	//qDebug() << "AuStarMapSearchSystems::SearchNow"
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out << m_Search;
	g_Au.Settings().writeEntry("Widget/StarMapSearchSystems/LastSearch", ba);

	// Remove current search contents.
	while(m_Result->count())
		delete m_Result->takeItem(0);

	// Search the star map, adding found items as we go.
	int systemsFound = 0;
	const AuStarMapData& starMap = g_AuClient.LockStarMapForRead();

	int totalSystems = 0;
	for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI = starMap.m_Sectors.begin();
			sectI != starMap.m_Sectors.end(); sectI++)
		for(unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator sysI = (*sectI).second.m_Systems.begin();
				sysI != (*sectI).second.m_Systems.end(); sysI++)
			totalSystems++;

	QProgressDialog* prog;
	prog = new QProgressDialog(this);
	prog->setWindowTitle(tr("Searching %1 Sectors").arg(starMap.m_Sectors.size()));
	prog->setWindowModality(Qt::WindowModal);
	prog->setValue(0);
	prog->setRange(0, totalSystems);
	prog->setMinimumDuration(2500);	// ms before appearing
	prog->setLabelText(tr("Searching %L1 solar systems...").arg(totalSystems));

	for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI = starMap.m_Sectors.begin();
			sectI != starMap.m_Sectors.end(); sectI++)
	{
		// Process solar systems in the sector.
		for(unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator sysI = (*sectI).second.m_Systems.begin();
				sysI != (*sectI).second.m_Systems.end(); sysI++)
		{
			prog->setValue(prog->value()+1);
			if(prog->wasCanceled())
				break;

			// We can perform the name and range checks before doing the work of creating the resource scan data.
			AuPoint systemGalaxyLoc = (*sectI).second.m_Loc + (*sysI).second.m_Loc;
			QString catname(DaSolarSystem::CatalogName(systemGalaxyLoc));
			if(!m_Search.PreScanTest((*sysI).second.m_Name, catname, (*sectI).second.Galaxy(), systemGalaxyLoc))
				continue;	// System failed the search filter test that does not require scan data.
			if(m_Search.SystemScanNeeded())
			{
				// More search filter testing is needed, that requires scan data.
				if(!(*sysI).second.IsScanned())
					continue;

				DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(m_Ether.FindAtom((*sysI).first));
				if(system)
				{
					// Verify the system matches our star map system.
					if(!system->IsSystem((*sysI).second.m_Name, (*sectI).second.Galaxy(),
                            (HABITABILITY)(*sysI).second.m_GenHab, (*sysI).second.m_GenSeed, (*sysI).second.IsCoreBlackHole(),
                            systemGalaxyLoc, -1.f))
					{
						qDebug() << "AuStarMapSearchSystems::SearchNow regen system" << *system;
						m_Ether.TrashAtom((*sysI).first);
						system = 0;
					}
				}
				if(!system)
				{
					// Create scan data to continue the search on the system.
					system = new DaSolarSystem;
					system->SetId((*sysI).first);
					m_Ether.AddAtom(system);
					AuMatrix4x4 systemT;
					systemT.Translate((*sysI).second.m_Loc);
					system->PrepareForResourceScan(catname, (*sysI).second.m_Name, (*sectI).second.Galaxy(),
							(HABITABILITY)(*sysI).second.m_GenHab, (*sysI).second.m_GenSeed, (*sysI).second.IsCoreBlackHole(),
							systemT, -1.f);
					m_ScanSaved = false;
				}
				if(!system->StarMapSearch(m_Search))
					continue;	// System failed the star map search.
			}
			QListWidgetItem* item = new QListWidgetItem(QIcon(":data/RangeSystem.png"), (*sysI).second.m_Name, m_Result);
			item->setData(Qt::UserRole, QVariant::fromValue<AuId>((*sysI).first));
			item->setData(Qt::UserRole+1, QVariant::fromValue<AuId>((*sectI).first));
			item->setData(Qt::UserRole+2, (*sysI).second.m_Name);
			item->setData(Qt::UserRole+3, (*sectI).second.m_Galaxy);
			item->setData(Qt::UserRole+4, (*sysI).second.IsCoreBlackHole());
			item->setData(Qt::UserRole+5, (*sysI).second.m_GenHab);
			item->setData(Qt::UserRole+6, (*sysI).second.m_GenSeed);
			item->setData(Qt::UserRole+7, systemGalaxyLoc.x);
			item->setData(Qt::UserRole+8, systemGalaxyLoc.y);
			item->setData(Qt::UserRole+9, systemGalaxyLoc.z);
			item->setData(Qt::UserRole+10, (*sysI).second.IsScanned());
			item->setData(Qt::UserRole+11, (*sectI).second.m_Name);
			systemsFound++;
			if(systemsFound == 1)
				prog->setLabelText(tr("Searching %L1 solar systems; found one match so far...")
						.arg(totalSystems));
			else
				prog->setLabelText(tr("Searching %L1 solar systems; found %L2 matches so far...")
						.arg(totalSystems)
						.arg(systemsFound));
		}
		if(prog->wasCanceled())
			break;
	}
	g_AuClient.UnlockStarMap();

	delete prog;

	QString result;
	if(systemsFound == 0)
		result = tr("No Systems Found");
	else if(systemsFound == 1)
		result = tr("One System Found");
	else
		result = tr("%L1 Systems Found").arg(systemsFound);
	m_SearchResult->setText(result);

	if(!m_ScanSaved)
		SaveScannedSystemsCache();
}

void AuStarMapSearchSystems::SearchResultClicked(QListWidgetItem* item)
{
	AuId systemId = item->data(Qt::UserRole).value<AuId>();
	AuId sectorId = item->data(Qt::UserRole+1).value<AuId>();
	QString systemName = item->data(Qt::UserRole+2).toString();
	GALAXY galaxy = (GALAXY)item->data(Qt::UserRole+3).toUInt();
	bool isCoreBlackHole = item->data(Qt::UserRole+4).toBool();
	HABITABILITY genHab = (HABITABILITY)item->data(Qt::UserRole+5).toUInt();
	quint32 genSeed = item->data(Qt::UserRole+6).toUInt();
	AuPoint systemGalaxyLoc(item->data(Qt::UserRole+7).toDouble(),
			item->data(Qt::UserRole+8).toDouble(),
			item->data(Qt::UserRole+9).toDouble());
	float starLight = 0;
	int scanEx;
	if(item->data(Qt::UserRole+10).toBool())
		scanEx = 2;
	else
		scanEx = 1;
	DaAtomList asteroids;
	QString sectorName = item->data(Qt::UserRole+11).toString();
	m_Scan->SetSystem(sectorId, sectorName, systemId, systemName, galaxy, isCoreBlackHole, genHab, genSeed, systemGalaxyLoc,
			starLight, scanEx, asteroids);
	emit SystemClicked(systemId);
}

void AuStarMapSearchSystems::EnableControls()
{
	bool findName = !m_Search.m_Name.isEmpty();
	m_Case->setEnabled(findName);
	m_RegEx->setEnabled(findName);
	m_SystemCat->setEnabled(findName);
	m_SystemName->setEnabled(findName);

	bool findRange = m_Search.m_Range > 0;
	m_Galaxy->setEnabled(findRange);
	m_SearchLoc->setEnabled(findRange);

	bool findNat = !m_Search.m_NaturalResources.empty();
	m_MinQuality->setEnabled(findNat);
	m_ResAnd->setEnabled(findNat);
	m_ResOr->setEnabled(findNat);

	bool findStars = m_Search.m_Stars;
	m_BlackHole->setEnabled(findStars);
	m_NeutronStar->setEnabled(findStars);
	m_StarTypeO->setEnabled(findStars);
	m_StarTypeB->setEnabled(findStars);
	m_StarTypeA->setEnabled(findStars);
	m_StarTypeF->setEnabled(findStars);
	m_StarTypeG->setEnabled(findStars);
	m_StarTypeK->setEnabled(findStars);
	m_StarTypeM->setEnabled(findStars);

	bool findWorlds = m_Search.m_Worlds;
	m_GasGiantInferno->setEnabled(findWorlds);
	m_GasGiantInner->setEnabled(findWorlds);
	m_GasGiantHabitable->setEnabled(findWorlds);
	m_GasGiantOuter->setEnabled(findWorlds);
	m_GasGiantFrigid->setEnabled(findWorlds);
	m_GlobeInferno->setEnabled(findWorlds);
	m_GlobeInner->setEnabled(findWorlds);
	m_GlobeHabitable->setEnabled(findWorlds);
	m_GlobeOuter->setEnabled(findWorlds);
	m_GlobeFrigid->setEnabled(findWorlds);
	m_RingInferno->setEnabled(findWorlds);
	m_RingInner->setEnabled(findWorlds);
	m_RingHabitable->setEnabled(findWorlds);
	m_RingOuter->setEnabled(findWorlds);
	m_RingFrigid->setEnabled(findWorlds);
	m_RingworldArcs->setEnabled(findWorlds);

	bool searchOk = (findName && (m_SystemCat->isChecked() || m_SystemName->isChecked())) ||
		findRange ||
		findNat ||
		findStars ||
		findWorlds;
	m_SearchButton->setEnabled(searchOk);
}

void AuStarMapSearchSystems::CheckAllNaturalResources()
{
	for(QList<AuCheckBoxValue*>::iterator ci = m_NaturalResources.begin(); ci != m_NaturalResources.end(); ci++)
		if((*ci)->checkState() != Qt::Checked)
			(*ci)->setCheckState(Qt::Checked);
}

void AuStarMapSearchSystems::InvertCheckedNaturalResources()
{
	for(QList<AuCheckBoxValue*>::iterator ci = m_NaturalResources.begin(); ci != m_NaturalResources.end(); ci++)
		if((*ci)->checkState() == Qt::Checked)
			(*ci)->setCheckState(Qt::Unchecked);
		else
			(*ci)->setCheckState(Qt::Checked);
}

void AuStarMapSearchSystems::UncheckAllNaturalResources()
{
	for(QList<AuCheckBoxValue*>::iterator ci = m_NaturalResources.begin(); ci != m_NaturalResources.end(); ci++)
		if((*ci)->checkState() == Qt::Checked)
			(*ci)->setCheckState(Qt::Unchecked);
}

QList<int> AuStarMapSearchSystems::CheckedNaturalResources() const
{
	QList<int> ignored;
	for(QList<AuCheckBoxValue*>::const_iterator ci = m_NaturalResources.cbegin(); ci != m_NaturalResources.cend(); ci++)
		if((*ci)->checkState() == Qt::Checked)
			ignored.append((*ci)->Value());
	return ignored;
}

void AuStarMapSearchSystems::RangeChanged(int value)
{
	m_Search.m_Range = value;
	EnableControls();
}

void AuStarMapSearchSystems::SearchLocChanged(const QString& text)
{
	m_Search.m_SearchLoc = AuPoint(text);
	EnableControls();
}

void AuStarMapSearchSystems::GalaxyChanged(int index)
{
	if(index < 0)
		return;
	m_Search.m_Galaxy = m_Galaxy->itemData(index).toInt();
	EnableControls();
}

void AuStarMapSearchSystems::NameChanged(const QString& text)
{
	m_Search.m_Name = text;
	EnableControls();
}

void AuStarMapSearchSystems::CaseChanged(int state)
{
	m_Search.m_CaseSense = state == Qt::Checked ? Qt::CaseSensitive : Qt::CaseInsensitive;
	EnableControls();
}

void AuStarMapSearchSystems::RegExChanged(int state)
{
	m_Search.m_RegEx = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::SystemCatChanged(int state)
{
	m_Search.m_SystemCat = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::SystemNameChanged(int state)
{
	m_Search.m_SystemName = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::NaturalResourceStateChanged(int state, int value)
{
	if(state == Qt::Checked)
	{
		// Insure the list of natural resources in the filter contains the commodity.
		if(!m_Search.m_NaturalResources.contains((COMMODITYID)value))
			m_Search.m_NaturalResources.append((COMMODITYID)value);
	}
	else
	{
		// Insure the list of natural resources in the filter does not contain the commodity.
		m_Search.m_NaturalResources.removeAll((COMMODITYID)value);
	}
	EnableControls();
}

void AuStarMapSearchSystems::MinQualityChanged(int value)
{
	m_Search.m_MinQ = value;
	EnableControls();
}

void AuStarMapSearchSystems::SetLocationReference(int galaxy, const AuPoint& galaxyLoc)
{
	m_LocationSet = true;
	m_Galaxy->setCurrentIndex(m_Galaxy->findData(QVariant((GALAXY)galaxy)));
	m_SearchLoc->setText(QString("%1, %2, %3")
			.arg(galaxyLoc.x, 0,'f',4)
			.arg(galaxyLoc.y, 0,'f',4)
			.arg(galaxyLoc.z, 0,'f',4));
}

void AuStarMapSearchSystems::StarsChanged(int value)
{
	m_Search.m_Stars = value;
	EnableControls();
}

void AuStarMapSearchSystems::BlackHoleChanged(int state)
{
	m_Search.m_BlackHole = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::NeutronStarChanged(int state)
{
	m_Search.m_NeutronStar = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::StarTypeOChanged(int state)
{
	m_Search.m_StarTypeO = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::StarTypeBChanged(int state)
{
	m_Search.m_StarTypeB = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::StarTypeAChanged(int state)
{
	m_Search.m_StarTypeA = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::StarTypeFChanged(int state)
{
	m_Search.m_StarTypeF = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::StarTypeGChanged(int state)
{
	m_Search.m_StarTypeG = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::StarTypeKChanged(int state)
{
	m_Search.m_StarTypeK = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::StarTypeMChanged(int state)
{
	m_Search.m_StarTypeM = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::WorldsChanged(int value)
{
	m_Search.m_Worlds = value;
	EnableControls();
}

void AuStarMapSearchSystems::GasGiantFrigidChanged(int state)
{
	m_Search.m_GasGiantFrigid = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::GasGiantHabitableChanged(int state)
{
	m_Search.m_GasGiantHabitable = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::GasGiantOuterChanged(int state)
{
	m_Search.m_GasGiantOuter = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::GasGiantInfernoChanged(int state)
{
	m_Search.m_GasGiantInferno = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::GasGiantInnerChanged(int state)
{
	m_Search.m_GasGiantInner = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::RingFrigidChanged(int state)
{
	m_Search.m_RingFrigid = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::RingHabitableChanged(int state)
{
	m_Search.m_RingHabitable = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::RingOuterChanged(int state)
{
	m_Search.m_RingOuter = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::RingInfernoChanged(int state)
{
	m_Search.m_RingInferno = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::RingInnerChanged(int state)
{
	m_Search.m_RingInner = state == Qt::Checked;
	EnableControls();
}

void AuStarMapSearchSystems::RingworldArcsChanged(int value)
{
	m_Search.m_RingworldArcs = value;
	EnableControls();
}

void AuStarMapSearchSystems::GlobeFrigidChanged(int index)
{
	if(index < 0)
		return;
	m_Search.m_GlobeFrigid = m_GlobeFrigid->itemData(index).toUInt();
}

void AuStarMapSearchSystems::GlobeHabitableChanged(int index)
{
	if(index < 0)
		return;
	m_Search.m_GlobeHabitable = m_GlobeHabitable->itemData(index).toUInt();
}

void AuStarMapSearchSystems::GlobeOuterChanged(int index)
{
	if(index < 0)
		return;
	m_Search.m_GlobeOuter = m_GlobeOuter->itemData(index).toUInt();
}

void AuStarMapSearchSystems::GlobeInfernoChanged(int index)
{
	if(index < 0)
		return;
	m_Search.m_GlobeInferno = m_GlobeInferno->itemData(index).toUInt();
}

void AuStarMapSearchSystems::GlobeInnerChanged(int index)
{
	if(index < 0)
		return;
	m_Search.m_GlobeInner = m_GlobeInner->itemData(index).toUInt();
}

void AuStarMapSearchSystems::SaveFilter()
{
	// Load last known file folder.
	QString lastFile = g_Au.Settings().readEntry("/StarMapSearchSystems/LastSearchFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();
	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	// Ask the user for a file name.
	QString suffix(".SoHCrit");
	QString sFilter;
	sFilter.append(tr("System Search Criteria (*%1)").arg(suffix));
	QString sFile = QFileDialog::getSaveFileName(this, tr("Save System Search Criteria"), lastFile, sFilter, 0,
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);

	if(!sFile.isEmpty())
	{
		// Insure the file has an appropriate suffix.
		if(sFile.right(suffix.length()) != suffix)
			sFile += suffix;

		g_Au.Settings().writeEntry("/StarMapSearchSystems/LastSearchFile", sFile);

		// Open/create the file.
		QFile file(sFile);
		if(file.open(QFile::WriteOnly))
		{
			// Store the file data.
			QDataStream ds(&file);
			ds.setVersion(QDataStream::Qt_4_1);

			// File Type and Version Bytes
			const quint8 scritFileType = SOHCRIT_FILE_TYPE;
			const quint8 scritFileVersion = SOHCRIT_FILE_VER;
			ds << scritFileType;
			ds << scritFileVersion;

			ds << m_Search;

			file.close();
		}
	}
}

void AuStarMapSearchSystems::ClearFilter()
{
	AuStarMapSystemSearch search;	// Start with constructor defaults.
	if(m_LocationSet)
	{
		// Preserve location/range settings.
		double range = m_Search.m_Range;
		int galaxy = m_Search.m_Galaxy;
		AuPoint searchLoc = m_Search.m_SearchLoc;

		m_Search = search;

		// Restore location/range settings.
		m_Search.m_Range = range;
		m_Search.m_Galaxy = galaxy;
		m_Search.m_SearchLoc = searchLoc;
	}
	else
		m_Search = search;
	PostFilter();

	// Remove current search contents.
	while(m_Result->count())
		delete m_Result->takeItem(0);

	m_SearchResult->setText("");
}

void AuStarMapSearchSystems::LoadFilter()
{
	// Load last known file folder.
	QString lastFile = g_Au.Settings().readEntry("/StarMapSearchSystems/LastSearchFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();
	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QFileDialog fileDialog(this, tr("Load System Search Criteria"), lastFile);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	if(!g_Au.UseNativeFileDialog())
		fileDialog.setOptions(QFileDialog::DontUseNativeDialog);
	QString suffix(".SoHCrit");
	QStringList sFilters;
	sFilters.append(tr("System Search Criteria (*%1)").arg(suffix));
	fileDialog.setNameFilters(sFilters);
	fileDialog.exec();
	if(!fileDialog.selectedFiles().isEmpty())
	{
		// Store last known file folder.
		QString sFile = fileDialog.selectedFiles().first();
		g_Au.Settings().writeEntry("/StarMapSearchSystems/LastSearchFile", sFile);

		QFile file(sFile);
		if(file.open(QFile::ReadOnly))
		{
			QDataStream ds(&file);
			ds.setVersion(QDataStream::Qt_4_1);

			// File Type Byte
			quint8 scritFileType;
			ds >> scritFileType;
			if(scritFileType == SOHCRIT_FILE_TYPE)
			{
				// Version Byte
				quint8 scritFileVersion;
				ds >> scritFileVersion;

				AuStarMapSystemSearch search;	// Start with constructor defaults.
				ds >> search;
				if(m_LocationSet)
				{
					// Preserve location/range settings.
					double range = m_Search.m_Range;
					int galaxy = m_Search.m_Galaxy;
					AuPoint searchLoc = m_Search.m_SearchLoc;

					m_Search = search;

					// Restore location/range settings.
					m_Search.m_Range = range;
					m_Search.m_Galaxy = galaxy;
					m_Search.m_SearchLoc = searchLoc;
				}
				else
					m_Search = search;
			}

			file.close();
			PostFilter();
		}
	}
}

void AuStarMapSearchSystems::PostFilter()
{
	// Block signals from controls we will update.
	m_Range->blockSignals(true);
	m_SearchLoc->blockSignals(true);
	m_Galaxy->blockSignals(true);
	m_Name->blockSignals(true);
	m_Case->blockSignals(true);
	m_RegEx->blockSignals(true);
	m_SystemName->blockSignals(true);
	m_SystemCat->blockSignals(true);
	for(QList<AuCheckBoxValue*>::iterator ci = m_NaturalResources.begin(); ci != m_NaturalResources.end(); ci++)
		(*ci)->blockSignals(true);
	m_MinQuality->blockSignals(true);
	m_ResAnd->blockSignals(true);
	m_ResOr->blockSignals(true);
	m_Stars->blockSignals(true);
	m_BlackHole->blockSignals(true);
	m_NeutronStar->blockSignals(true);
	m_StarTypeO->blockSignals(true);
	m_StarTypeB->blockSignals(true);
	m_StarTypeA->blockSignals(true);
	m_StarTypeF->blockSignals(true);
	m_StarTypeG->blockSignals(true);
	m_StarTypeK->blockSignals(true);
	m_StarTypeM->blockSignals(true);
	m_Worlds->blockSignals(true);
	m_GlobeInferno->blockSignals(true);
	m_GlobeInner->blockSignals(true);
	m_GlobeHabitable->blockSignals(true);
	m_GlobeOuter->blockSignals(true);
	m_GlobeFrigid->blockSignals(true);
	m_GasGiantInferno->blockSignals(true);
	m_GasGiantInner->blockSignals(true);
	m_GasGiantHabitable->blockSignals(true);
	m_GasGiantOuter->blockSignals(true);
	m_GasGiantFrigid->blockSignals(true);
	m_RingInferno->blockSignals(true);
	m_RingInner->blockSignals(true);
	m_RingHabitable->blockSignals(true);
	m_RingOuter->blockSignals(true);
	m_RingFrigid->blockSignals(true);
	m_RingworldArcs->blockSignals(true);

	// Range
	m_Range->setValue((int)m_Search.m_Range);
	m_SearchLoc->setText(QString("%1, %2, %3")
			.arg(m_Search.m_SearchLoc.x, 0,'f',4)
			.arg(m_Search.m_SearchLoc.y, 0,'f',4)
			.arg(m_Search.m_SearchLoc.z, 0,'f',4));
	m_Galaxy->setCurrentIndex(m_Galaxy->findData(QVariant((GALAXY)m_Search.m_Galaxy)));

	// Name
	m_Name->setText(m_Search.m_Name);
	m_Case->setChecked(m_Search.m_CaseSense);
	m_RegEx->setChecked(m_Search.m_RegEx);
	m_SystemName->setChecked(m_Search.m_SystemName);
	m_SystemCat->setChecked(m_Search.m_SystemCat);

	// Natural Resources
	for(QList<AuCheckBoxValue*>::iterator ci = m_NaturalResources.begin(); ci != m_NaturalResources.end(); ci++)
	{
		Qt::CheckState nuState = m_Search.m_NaturalResources.contains((*ci)->Value()) ? Qt::Checked : Qt::Unchecked;
		(*ci)->setCheckState(nuState);
	}
	m_MinQuality->setValue(m_Search.m_MinQ);
	if(m_Search.m_AndOr)
	{
		// OR
		m_ResOr->setChecked(true);
		m_ResAnd->setChecked(false);
	}
	else
	{
		// AND
		m_ResAnd->setChecked(true);
		m_ResOr->setChecked(false);
	}

	// Stars
	m_Stars->setValue(m_Search.m_Stars);
	m_BlackHole->setChecked(m_Search.m_BlackHole);
	m_NeutronStar->setChecked(m_Search.m_NeutronStar);
	m_StarTypeO->setChecked(m_Search.m_StarTypeO);
	m_StarTypeB->setChecked(m_Search.m_StarTypeB);
	m_StarTypeA->setChecked(m_Search.m_StarTypeA);
	m_StarTypeF->setChecked(m_Search.m_StarTypeF);
	m_StarTypeG->setChecked(m_Search.m_StarTypeG);
	m_StarTypeK->setChecked(m_Search.m_StarTypeK);
	m_StarTypeM->setChecked(m_Search.m_StarTypeM);

	// Worlds
	m_Worlds->setValue(m_Search.m_Worlds);
	m_GlobeInferno->setCurrentIndex(m_GlobeInferno->findData(QVariant(m_Search.m_GlobeInferno)));
	m_GlobeInner->setCurrentIndex(m_GlobeInner->findData(QVariant(m_Search.m_GlobeInner)));
	m_GlobeHabitable->setCurrentIndex(m_GlobeHabitable->findData(QVariant(m_Search.m_GlobeHabitable)));
	m_GlobeOuter->setCurrentIndex(m_GlobeOuter->findData(QVariant(m_Search.m_GlobeOuter)));
	m_GlobeFrigid->setCurrentIndex(m_GlobeFrigid->findData(QVariant(m_Search.m_GlobeFrigid)));
	m_GasGiantInferno->setChecked(m_Search.m_GasGiantInferno);
	m_GasGiantInner->setChecked(m_Search.m_GasGiantInner);
	m_GasGiantHabitable->setChecked(m_Search.m_GasGiantHabitable);
	m_GasGiantOuter->setChecked(m_Search.m_GasGiantOuter);
	m_GasGiantFrigid->setChecked(m_Search.m_GasGiantFrigid);
	m_RingInferno->setChecked(m_Search.m_RingInferno);
	m_RingInner->setChecked(m_Search.m_RingInner);
	m_RingHabitable->setChecked(m_Search.m_RingHabitable);
	m_RingOuter->setChecked(m_Search.m_RingOuter);
	m_RingFrigid->setChecked(m_Search.m_RingFrigid);
	m_RingworldArcs->setValue(m_Search.m_RingworldArcs);

	// Unblock signals
	m_Range->blockSignals(false);
	m_SearchLoc->blockSignals(false);
	m_Galaxy->blockSignals(false);
	m_Name->blockSignals(false);
	m_Case->blockSignals(false);
	m_RegEx->blockSignals(false);
	m_SystemName->blockSignals(false);
	m_SystemCat->blockSignals(false);
	for(QList<AuCheckBoxValue*>::iterator ci = m_NaturalResources.begin(); ci != m_NaturalResources.end(); ci++)
		(*ci)->blockSignals(false);
	m_MinQuality->blockSignals(false);
	m_ResAnd->blockSignals(false);
	m_ResOr->blockSignals(false);
	m_Stars->blockSignals(false);
	m_BlackHole->blockSignals(false);
	m_NeutronStar->blockSignals(false);
	m_StarTypeO->blockSignals(false);
	m_StarTypeB->blockSignals(false);
	m_StarTypeA->blockSignals(false);
	m_StarTypeF->blockSignals(false);
	m_StarTypeG->blockSignals(false);
	m_StarTypeK->blockSignals(false);
	m_StarTypeM->blockSignals(false);
	m_Worlds->blockSignals(false);
	m_GlobeInferno->blockSignals(false);
	m_GlobeInner->blockSignals(false);
	m_GlobeHabitable->blockSignals(false);
	m_GlobeOuter->blockSignals(false);
	m_GlobeFrigid->blockSignals(false);
	m_GasGiantInferno->blockSignals(false);
	m_GasGiantInner->blockSignals(false);
	m_GasGiantHabitable->blockSignals(false);
	m_GasGiantOuter->blockSignals(false);
	m_GasGiantFrigid->blockSignals(false);
	m_RingInferno->blockSignals(false);
	m_RingInner->blockSignals(false);
	m_RingHabitable->blockSignals(false);
	m_RingOuter->blockSignals(false);
	m_RingFrigid->blockSignals(false);
	m_RingworldArcs->blockSignals(false);

	EnableControls();
}

void AuStarMapSearchSystems::SaveScannedSystemsCache()
{
	// Open/create the file.
	QString sFile(g_Au.CacheGame() + "/SS");
	QFile file(sFile);
	if(file.open(QFile::WriteOnly))
	{
		// Store the file data.
		QDataStream ds(&file);
		ds.setVersion(QDataStream::Qt_5_0);

		// File Type and Version Bytes
		const quint8 fileType = SYSTEM_SCAN_CACHE;
		const quint8 fileVersion = SYSTEM_SCAN_VER;
		ds << fileType;
		ds << fileVersion;

		DaAtomList atoms;
		m_Ether.ListAtoms(atoms);
		ds << atoms;

		file.close();
	}
	m_ScanSaved = true;
}

void AuStarMapSearchSystems::RestoreScannedSystemsCache()
{
	QString sFile(g_Au.CacheGame() + "/SS");
	QFile file(sFile);
	if(file.open(QFile::ReadOnly))
	{
		QDataStream ds(&file);
		ds.setVersion(QDataStream::Qt_5_0);

		// File Type Byte
		quint8 fileType;
		ds >> fileType;
		if(fileType == SYSTEM_SCAN_CACHE)
		{
			// Version Byte
			quint8 fileVersion;
			ds >> fileVersion;
			if(fileVersion == SYSTEM_SCAN_VER)
			{
				DaAtomList atoms;
				ds >> atoms;
				m_Ether.AddAtoms(atoms);
			}
		}
		file.close();
	}
	m_ScanSaved = true;
}

void AuStarMapSearchSystems::ResAndClicked()
{
	EnableControls();
}

void AuStarMapSearchSystems::ResOrClicked()
{
	EnableControls();
}

