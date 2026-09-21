//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubddesignprops.cpp,v 1.13 2022/02/19 18:25:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubddesignprops.h"
//
#include "aubdreport.h"
#include "auglobal.h"
#include "aulineedit.h"
#include "auscparts.h"
#include "auslider.h"
#include "dgdata.h"
#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QTableWidget>
#include <QTextBrowser>

AuBdDesignProps::AuBdDesignProps(const AuBdDesign& shipRec, QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/c_BdDesignDrawing.png"));
	setAuDlgTitle("Building Design Properties");
	m_DesignRec = shipRec;

	quint32 hullVol = shipRec.HullVolume();

	// Design Name
	QLabel* lbName = new QLabel(tr("Design Name"));

	m_Name = new QLineEdit(shipRec.Name(), this);
	m_Name->setToolTip(tr("<html>"
				"The design name must be no more than %1 characters "
				"and it must not contain non-printable characters "
				"or certain reserved characters such as &lt; &gt; &amp; and |."
				"</html>")
			.arg(MaxNameLength()));
	connect(m_Name, SIGNAL(textChanged(const QString&)), SLOT(NameChanged(const QString&)));

	// Design Description
	QLabel* lbDesc = new QLabel(tr("Description"));
	lbDesc->setAlignment(Qt::AlignTop|Qt::AlignLeft);

	m_Description = new QTextEdit(shipRec.Description(), this);
	m_Description->setToolTip(tr("<html>"
				"Enter a description of the design, such as its intended purpose and noteworthy features. "
				"It is ok to summarize but don't literally repeat the design analysis. "
				"</html>"));
	connect(m_Description, SIGNAL(textChanged()), SLOT(DescriptionChanged()));

	// Structure
	QLabel* lbStruc = new QLabel(tr("Structure"), this);

	m_Structure = new QComboBox(this);
	//for(int bds=BD_Invalid+1; bds < BD_Count; bds++)
		//m_Structure->addItem(BuildingStructureName((BUILDING_STRUCTURE)bds), QVariant(bds));
	m_Structure->addItem(BuildingStructureName(BD_Building), QVariant(BD_Building));
	m_Structure->addItem(BuildingStructureName(BD_StreetLight), QVariant(BD_StreetLight));
	m_Structure->addItem(BuildingStructureName(BD_Bridge), QVariant(BD_Bridge));
	m_Structure->addItem(BuildingStructureName(BD_Biodome), QVariant(BD_Biodome));
	m_Structure->addItem(BuildingStructureName(BD_AerialPlatform), QVariant(BD_AerialPlatform));
	m_Structure->addItem(BuildingStructureName(BD_GroundPlatform), QVariant(BD_GroundPlatform));
	m_Structure->addItem(BuildingStructureName(BD_SeaPlatform), QVariant(BD_SeaPlatform));
	m_Structure->addItem(BuildingStructureName(BD_SeaSurfaceBuilding), QVariant(BD_SeaSurfaceBuilding));
	m_Structure->addItem(BuildingStructureName(BD_Fence), QVariant(BD_Fence));
	m_Structure->setCurrentIndex(m_Structure->findData(shipRec.Structure()));
	m_Structure->setToolTip(tr("<b>Structure</b><br>"
				"Chooses a structure for the design.<br>"
				"<b>Building</b> is a typical building structure."
				"<br>"
				"<b>Biodome</b> is a habitable enclosure in which buildings can be constructed. "
				"<br>"
				"<b>Aerial Platform</b> is a flying structure on which buildings can be constructed. "
				"Aerial platforms are built hovering in the upper atmosphere of worlds, including airless worlds and gas giants."
				"<br>"
				"<b>Ground Platform</b> is a ground structure on which buildings can be constructed. "
				"Ground platforms are built on dry ground or under water and supercooled seas. "
				"<br>"
				"<b>Sea Platform</b> is a floating structure on which buildings can be constructed. "
				"Sea platforms are built floating on the surface of seas of water, lava, and supercooled liquids. "
				"Vulcium or adamantine must be used to build on lava. "
				"<br>"
				"<b>Sea Surface Building</b> is a floating building structure, used exclusively by <i>Drilling Rigs</i>. "
				"<br>"
				"<b>Fence or Wall</b> is used for making fences and walls. See <i>Usage Tips</i> for specific design requirements. "
				));
	connect(m_Structure, SIGNAL(currentIndexChanged(int)), SLOT(StructureChanged(int)));

	// System Modules
	QLabel* lbMaterial = new QLabel(tr("Construction Material"), this);

	m_Hull = new QComboBox(this);
	m_Hull->setToolTip(tr("<b>Construction Material</b><br>"
				"Determines the default construction material to be used for the building. "
				"Construction material affects hit points, combustibility, and appearance of the building. "
				"Other materials can be used when the building is constructed. "));
	ListModules(m_Hull, SYS_Hull, shipRec.HullModule());
	connect(m_Hull, SIGNAL(currentIndexChanged(int)), SLOT(MaterialChanged(int)));

	QLabel* lbLifeSupportMod = new QLabel(BdSystemName(SYS_LifeSupport), this);

	m_LifeSupportMod = new QComboBox(this);
	m_LifeSupportMod->setToolTip(tr("<b>Life Support</b><br>"
				"Life support module determines the capabilities of the life support system. "
				"A functioning life support system is needed for avatars to respawn in their bed. "
				"A life support system is needed to use the building in harsh environments. <br>"
				"<b>Life Support</b> maintains a habitable environment inside the building.<br>"
				"<b>Life Support Gravity</b> maintains a habitable environment with gravity inside the building."));
	ListModules(m_LifeSupportMod, SYS_LifeSupport, shipRec.LifeSupportModule());
	connect(m_LifeSupportMod, SIGNAL(currentIndexChanged(int)), SLOT(LifeSupportModuleChanged(int)));

	QLabel* lbPowerPlantMod = new QLabel(BdSystemName(SYS_PowerPlant), this);

	m_PowerPlantMod = new QComboBox(this);
	m_PowerPlantMod->setToolTip(tr("<b>Generator</b><br>"
				"Generators convert fuel to energy. "
				"Energy is stored in the battery. "
				"Energy created with a generator is shared with other buildings. "
				"A generator is needed to use the building as a power plant. "));
	ListModules(m_PowerPlantMod, SYS_PowerPlant, shipRec.PowerPlantModule());
	connect(m_PowerPlantMod, SIGNAL(currentIndexChanged(int)), SLOT(PowerPlantModuleChanged(int)));

	QLabel* lbCapacitorMod = new QLabel(BdSystemName(SYS_Capacitor), this);

	m_CapacitorMod = new QComboBox(this);
	m_CapacitorMod->setToolTip(tr("<b>Battery</b><br>"
				"Battery stores energy for use by all systems and manufacturing processes of a building. "
				"Batteries are charged by drawing power from buildings with generators. "));
	ListModules(m_CapacitorMod, SYS_Capacitor, shipRec.CapacitorModule());
	connect(m_CapacitorMod, SIGNAL(currentIndexChanged(int)), SLOT(CapacitorModuleChanged(int)));

	m_Ok = new QPushButton(tr("Ok"));
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	connect(m_Ok, SIGNAL(clicked()), SLOT(accept()));
	m_Ok->setToolTip(tr("<html>Accepts changes to the building design. "
				"If this button is disabled, check <i>Design Name</i>."
				"</html>"));

	m_Analysis = new QTextBrowser(this);
	//m_Analysis->setOpenExternalLinks(true);
	m_Analysis->setReadOnly(true);

	QLabel* lbHullVol = new QLabel(tr("<html><b>Building Volume</b></html>"), this);
	lbHullVol->setAlignment(Qt::AlignBottom|Qt::AlignLeft);

	QLabel* lbHullVolTot = new QLabel(tr("<html><b>%L1m%2 Total</b></html>").arg(hullVol).arg(g_Au.CubedSymbol()), this);
	lbHullVolTot->setAlignment(Qt::AlignBottom|Qt::AlignLeft);

	// Armor
	QLabel* lbArmor;
	{
		int value = shipRec.ArmorVolume();

		lbArmor = new QLabel(tr("Armor"), this);

		m_Armor = new QSlider(Qt::Horizontal, this);
		m_Armor->setRange(0, hullVol);
		m_Armor->setSingleStep(1);
		m_Armor->setPageStep(1);
		m_Armor->setValue(value);
		m_Armor->setToolTip(tr("<b>Armor</b><br>"
					"Adjusts the volume of building allocated to armor. "
					"<br>"
					"Building armor improves defense against physical damage."));
		connect(m_Armor, SIGNAL(valueChanged(int)), SLOT(SetArmor(int)));

		m_ArmorLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_ArmorLb, SIGNAL(editingFinished()), SLOT(ArmorEdited()));
	}

	QLabel* lbCapacitor;
	{
		int value = shipRec.CapacitorVolume();

		lbCapacitor = new QLabel(tr("Battery"), this);

		m_Capacitor = new QSlider(Qt::Horizontal, this);
		m_Capacitor->setRange(0, hullVol);
		m_Capacitor->setSingleStep(1);
		m_Capacitor->setPageStep(1);
		m_Capacitor->setValue(value);
		m_Capacitor->setToolTip(tr("<html>"
					"<b>Battery</b><br>"
					"Adjusts the volume of building allocated to battery. "
					"<br>"
					"Battery stores energy for ready use systems and manufacturing processes. "
					"Batteries are charged by drawing power from buildings with power generators. "
					"</html>"));
		connect(m_Capacitor, SIGNAL(valueChanged(int)), SLOT(SetCapacitor(int)));

		m_CapacitorLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_CapacitorLb, SIGNAL(editingFinished()), SLOT(CapacitorEdited()));
	}

	QLabel* lbHold;
	{
		int value = shipRec.HoldVolume();

		lbHold = new QLabel(tr("Commodity Storage"), this);

		m_Hold = new QSlider(Qt::Horizontal, this);
		m_Hold->setRange(0, hullVol);
		m_Hold->setSingleStep(1);
		m_Hold->setPageStep(1);
		m_Hold->setValue(value);
		m_Hold->setToolTip(tr("<html>"
					"<b>Commodity Storage</b><br>"
					"Adjusts the volume of building allocated to commodity storage. "
					"<br>"
					"Commodity storage space stores things made or purchased by this building. "
					"</html>"));
		connect(m_Hold, SIGNAL(valueChanged(int)), SLOT(SetHold(int)));

		m_HoldLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_HoldLb, SIGNAL(editingFinished()), SLOT(HoldEdited()));
	}

	QLabel* lbSkeleton;
	{
		int value = shipRec.SkeletonVolume();

		lbSkeleton = new QLabel(tr("Framework"), this);

		m_Skeleton = new QSlider(Qt::Horizontal, this);
		m_Skeleton->setRange(0, hullVol);
		m_Skeleton->setSingleStep(1);
		m_Skeleton->setPageStep(1);
		m_Skeleton->setValue(value);
		m_Skeleton->setToolTip(tr("<html>"
					"<b>Framework</b><br>"
					"Adjusts the volume of hull allocated to framework. "
					"Framework is walls, floors, ceilings, beams, columns and other support structures. "
					"<br>"
					"Framework volume adds 4x the structural hit points of other volumes. "
					"</html>"));
		connect(m_Skeleton, SIGNAL(valueChanged(int)), SLOT(SetSkeleton(int)));

		m_SkeletonLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_SkeletonLb, SIGNAL(editingFinished()), SLOT(SkeletonEdited()));
	}

	QLabel* lbArena;
	{
		int value = shipRec.ArenaVolume();

		lbArena = new QLabel(tr("Arena Space"), this);

		m_Arena = new QSlider(Qt::Horizontal, this);
		m_Arena->setRange(0, hullVol);
		m_Arena->setSingleStep(1);
		m_Arena->setPageStep(1);
		m_Arena->setValue(value);
		m_Arena->setToolTip(tr("<html>"
					"<b>Arena Space</b><br>"
					"Adjusts the volume of building allocated to arena space. "
					"An arena is added for the first %L2m%1 of arena space. "
					"Another arena is added for every %L3m%1 after that. "
					"<br>"
					"Arenas provide event space for citizens. "
					"</html>").arg(g_Au.CubedSymbol()).arg(shipRec.VolumeFirstArena()).arg(shipRec.VolumeAddArena()));
		connect(m_Arena, SIGNAL(valueChanged(int)), SLOT(SetArena(int)));

		m_ArenaLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_ArenaLb, SIGNAL(editingFinished()), SLOT(ArenaEdited()));
	}

	QLabel* lbAuditorium;
	{
		int value = shipRec.AuditoriumVolume();

		lbAuditorium = new QLabel(tr("Auditorium Space"), this);

		m_Auditorium = new QSlider(Qt::Horizontal, this);
		m_Auditorium->setRange(0, hullVol);
		m_Auditorium->setSingleStep(1);
		m_Auditorium->setPageStep(1);
		m_Auditorium->setValue(value);
		m_Auditorium->setToolTip(tr("<html>"
					"<b>Auditorium Space</b><br>"
					"Adjusts the volume of building allocated to auditorium space. "
					"An auditorium is added for the first %L2m%1 of auditorium space. "
					"Another auditorium is added for every %L3m%1 after that. "
					"<br>"
					"Auditoriums provide gathering space for citizens. "
					"</html>").arg(g_Au.CubedSymbol()).arg(shipRec.VolumeFirstAuditorium()).arg(shipRec.VolumeAddAuditorium()));
		connect(m_Auditorium, SIGNAL(valueChanged(int)), SLOT(SetAuditorium(int)));

		m_AuditoriumLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_AuditoriumLb, SIGNAL(editingFinished()), SLOT(AuditoriumEdited()));
	}

	QLabel* lbHomeLarge;
	{
		int value = shipRec.HomeLargeVolume();

		lbHomeLarge = new QLabel(tr("Home, Large Space"), this);

		m_HomeLarge = new QSlider(Qt::Horizontal, this);
		m_HomeLarge->setRange(0, hullVol);
		m_HomeLarge->setSingleStep(1);
		m_HomeLarge->setPageStep(1);
		m_HomeLarge->setValue(value);
		m_HomeLarge->setToolTip(tr("<html>"
					"<b>Large Home Space</b><br>"
					"Adjusts the volume of building allocated to large home space. "
					"A large home is added for the first %L2m%1 of large home space. "
					"Another large home is added for every %L3m%1 after that. "
					"<br>"
					"Large homes provide comfortable living space for citizens. "
					"</html>").arg(g_Au.CubedSymbol()).arg(shipRec.VolumeFirstHomeLarge()).arg(shipRec.VolumeAddHomeLarge()));
		connect(m_HomeLarge, SIGNAL(valueChanged(int)), SLOT(SetHomeLarge(int)));

		m_HomeLargeLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_HomeLargeLb, SIGNAL(editingFinished()), SLOT(HomeLargeEdited()));
	}

	QLabel* lbHomeMedium;
	{
		int value = shipRec.HomeMediumVolume();

		lbHomeMedium = new QLabel(tr("Home, Medium Space"), this);

		m_HomeMedium = new QSlider(Qt::Horizontal, this);
		m_HomeMedium->setRange(0, hullVol);
		m_HomeMedium->setSingleStep(1);
		m_HomeMedium->setPageStep(1);
		m_HomeMedium->setValue(value);
		m_HomeMedium->setToolTip(tr("<html>"
					"<b>Medium Home Space</b><br>"
					"Adjusts the volume of building allocated to medium home space. "
					"A medium home is added for the first %L2m%1 of medium home space. "
					"Another medium home is added for every %L3m%1 after that. "
					"<br>"
					"Medium homes provide satisfactory living space for citizens. "
					"</html>").arg(g_Au.CubedSymbol()).arg(shipRec.VolumeFirstHomeMedium()).arg(shipRec.VolumeAddHomeMedium()));
		connect(m_HomeMedium, SIGNAL(valueChanged(int)), SLOT(SetHomeMedium(int)));

		m_HomeMediumLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_HomeMediumLb, SIGNAL(editingFinished()), SLOT(HomeMediumEdited()));
	}

	QLabel* lbHomeSmall;
	{
		int value = shipRec.HomeSmallVolume();

		lbHomeSmall = new QLabel(tr("Home, Small Space"), this);

		m_HomeSmall = new QSlider(Qt::Horizontal, this);
		m_HomeSmall->setRange(0, hullVol);
		m_HomeSmall->setSingleStep(1);
		m_HomeSmall->setPageStep(1);
		m_HomeSmall->setValue(value);
		m_HomeSmall->setToolTip(tr("<html>"
					"<b>Small Home Space</b><br>"
					"Adjusts the volume of building allocated to small home space. "
					"A small home is added for the first %L2m%1 of small home space. "
					"Another small home is added for every %L3m%1 after that. "
					"<br>"
					"Small homes provide minimal living space for citizens. "
					"</html>").arg(g_Au.CubedSymbol()).arg(shipRec.VolumeFirstHomeSmall()).arg(shipRec.VolumeAddHomeSmall()));
		connect(m_HomeSmall, SIGNAL(valueChanged(int)), SLOT(SetHomeSmall(int)));

		m_HomeSmallLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_HomeSmallLb, SIGNAL(editingFinished()), SLOT(HomeSmallEdited()));
	}

	QLabel* lbLounge;
	{
		int value = shipRec.LoungeVolume();

		lbLounge = new QLabel(tr("Lounge Space"), this);

		m_Lounge = new QSlider(Qt::Horizontal, this);
		m_Lounge->setRange(0, hullVol);
		m_Lounge->setSingleStep(1);
		m_Lounge->setPageStep(1);
		m_Lounge->setValue(value);
		m_Lounge->setToolTip(tr("<html>"
					"<b>Lounge Space</b><br>"
					"Adjusts the volume of building allocated to lounge space. "
					"One lounge is added for the first %L2m%1 of lounge space. "
					"Another lounge is added for every %L3m%1 after that. "
					"<br>"
					"Lounges provide relaxation space for citizens. "
					"</html>").arg(g_Au.CubedSymbol()).arg(shipRec.VolumeFirstLounge()).arg(shipRec.VolumeAddLounge()));
		connect(m_Lounge, SIGNAL(valueChanged(int)), SLOT(SetLounge(int)));

		m_LoungeLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_LoungeLb, SIGNAL(editingFinished()), SLOT(LoungeEdited()));
	}

	QLabel* lbStore;
	{
		int value = shipRec.StoreVolume();

		lbStore = new QLabel(tr("Store Space"), this);

		m_Store = new QSlider(Qt::Horizontal, this);
		m_Store->setRange(0, hullVol);
		m_Store->setSingleStep(1);
		m_Store->setPageStep(1);
		m_Store->setValue(value);
		m_Store->setToolTip(tr("<html>"
					"<b>Store Space</b><br>"
					"Adjusts the volume of building allocated to store space. "
					"One store is added for the first %L2m%1 of store space. "
					"Another store is added for every %L3m%1 after that. "
					"<br>"
					"Stores provide retail shopping space for citizens. "
					"</html>").arg(g_Au.CubedSymbol()).arg(shipRec.VolumeFirstStore()).arg(shipRec.VolumeAddStore()));
		connect(m_Store, SIGNAL(valueChanged(int)), SLOT(SetStore(int)));

		m_StoreLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_StoreLb, SIGNAL(editingFinished()), SLOT(StoreEdited()));
	}

	QLabel* lbOffice;
	{
		int value = shipRec.OfficeVolume();

		lbOffice = new QLabel(tr("Office Space"), this);

		m_Office = new QSlider(Qt::Horizontal, this);
		m_Office->setRange(0, hullVol);
		m_Office->setSingleStep(1);
		m_Office->setPageStep(1);
		m_Office->setValue(value);
		m_Office->setToolTip(tr("<html>"
					"<b>Office Space</b><br>"
					"Adjusts the volume of building allocated to office space. "
					"An office is added for the first %L2m%1 of office space. "
					"Another office is added for every %L3m%1 after that. "
					"<br>"
					"Offices provide work space for office jobs. "
					"</html>").arg(g_Au.CubedSymbol()).arg(shipRec.VolumeFirstOffice()).arg(shipRec.VolumeAddOffice()));
		connect(m_Office, SIGNAL(valueChanged(int)), SLOT(SetOffice(int)));

		m_OfficeLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_OfficeLb, SIGNAL(editingFinished()), SLOT(OfficeEdited()));
	}

	QLabel* lbShop;
	{
		int value = shipRec.ShopVolume();

		lbShop = new QLabel(tr("Workshop Space"), this);

		m_Shop = new QSlider(Qt::Horizontal, this);
		m_Shop->setRange(0, hullVol);
		m_Shop->setSingleStep(1);
		m_Shop->setPageStep(1);
		m_Shop->setValue(value);
		m_Shop->setToolTip(tr("<html>"
					"<b>Workshop Space</b><br>"
					"Adjusts the volume of building allocated to workshop space. "
					"A workshop is added for the first %L2m%1 of workshop space. "
					"Another workshop is added for every %L3m%1 after that. "
					"<br>"
					"Workshops provide work space for industrial jobs. "
					"</html>").arg(g_Au.CubedSymbol()).arg(shipRec.VolumeFirstShop()).arg(shipRec.VolumeAddShop()));
		connect(m_Shop, SIGNAL(valueChanged(int)), SLOT(SetShop(int)));

		m_ShopLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_ShopLb, SIGNAL(editingFinished()), SLOT(ShopEdited()));
	}

	QLabel* lbLifeSupport;
	{
		int value = shipRec.LifeSupportVolume();

		lbLifeSupport = new QLabel(tr("Life Support"), this);

		m_LifeSupport = new QSlider(Qt::Horizontal, this);
		m_LifeSupport->setRange(0, hullVol);
		m_LifeSupport->setSingleStep(1);
		m_LifeSupport->setPageStep(1);
		m_LifeSupport->setValue(value);
		m_LifeSupport->setToolTip(tr("<html>"
					"<b>Life Support</b><br>"
					"Adjusts the volume of building allocated to life support. "
					"<br>"
					"Life support maintains a habitable environment inside the building in all environments. "
					"</html>"));
		connect(m_LifeSupport, SIGNAL(valueChanged(int)), SLOT(SetLifeSupport(int)));

		m_LifeSupportLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_LifeSupportLb, SIGNAL(editingFinished()), SLOT(LifeSupportEdited()));
	}

	QLabel* lbPowerPlant;
	{
		int value = shipRec.PowerPlantVolume();

		lbPowerPlant = new QLabel(tr("Generator"), this);

		m_PowerPlant = new QSlider(Qt::Horizontal, this);
		m_PowerPlant->setRange(0, hullVol);
		m_PowerPlant->setSingleStep(1);
		m_PowerPlant->setPageStep(1);
		m_PowerPlant->setValue(value);
		m_PowerPlant->setToolTip(tr("<html>"
					"<b>Power Plant</b><br>"
					"Adjusts the volume of building allocated to generator. "
					"A generator job is added for the first %L2m%1 of generator space. "
					"Another generator job is added for every %L3m%1 after that. "
					"<br>"
					"Generators convert fuel into energy. Energy is stored in the battery for access by other buildings. "
					"</html>").arg(g_Au.CubedSymbol()).arg(shipRec.VolumeFirstGenerator()).arg(shipRec.VolumeAddGenerator()));
		connect(m_PowerPlant, SIGNAL(valueChanged(int)), SLOT(SetPowerPlant(int)));

		m_PowerPlantLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_PowerPlantLb, SIGNAL(editingFinished()), SLOT(PowerPlantEdited()));
	}

	QLabel* lbSensors = 0;
	if(shipRec.SensorModule() != CID_Invalid)
	{
		int value = shipRec.SensorVolume();

		lbSensors = new QLabel(tr("Radar"), this);

		m_Sensors = new QSlider(Qt::Horizontal, this);
		m_Sensors->setRange(0, hullVol);
		m_Sensors->setSingleStep(1);
		m_Sensors->setPageStep(1);
		m_Sensors->setValue(value);
		m_Sensors->setToolTip(tr("<html>"
					"<b>Radar</b><br>"
					"Adjusts the volume of building allocated to radar. "
					"Radar creates one job for the radar operator. "
					"<br>"
					"Radar detects worlds, spacecraft, cities, vehicles and other objects in range of the building. "
					"</html>"));
		connect(m_Sensors, SIGNAL(valueChanged(int)), SLOT(SetSensors(int)));

		m_SensorsLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_SensorsLb, SIGNAL(editingFinished()), SLOT(SensorsEdited()));
	}
	else
	{
		m_Sensors = 0;
		m_SensorsLb = 0;
	}

	QLabel* lbShields = 0;
	if(shipRec.ShieldModule() != CID_Invalid)
	{
		int value = shipRec.ShieldVolume();

		lbShields = new QLabel(tr("Shields"), this);

		m_Shields = new QSlider(Qt::Horizontal, this);
		m_Shields->setRange(0, hullVol);
		m_Shields->setSingleStep(1);
		m_Shields->setPageStep(1);
		m_Shields->setValue(value);
		m_Shields->setToolTip(tr("<html>"
					"<b>Shields</b><br>"
					"Adjusts the volume of building allocated to shields. "
					"Shields create one job for the shield operator. "
					"<br>"
					"Shields create a protective field around all buildings that are in range of the shield. "
					"</html>"));
		connect(m_Shields, SIGNAL(valueChanged(int)), SLOT(SetShields(int)));

		m_ShieldsLb = new QLineEdit(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), this);
		connect(m_ShieldsLb, SIGNAL(editingFinished()), SLOT(ShieldsEdited()));
	}
	else
	{
		m_Shields = 0;
		m_ShieldsLb = 0;
	}

	QHash<quint16, QLabel*> lbBays;
	{
		AuScPartSet bays;
		shipRec.Bays(bays);
		if(!bays.empty())
		{
			for(QSet<quint16>::const_iterator pi = bays.cbegin(); pi != bays.cend(); pi++)
			{
				QString n = QString("%1. %2").arg(*pi).arg(g_Au.CommodityName(shipRec.PartModule(*pi)));
				QLabel* lb = new QLabel(n, this);

				int value = shipRec.BayVolume(*pi);

				AuSliderValue* sv = new AuSliderValue(Qt::Horizontal, *pi, this);
				sv->setRange(0, hullVol);
				sv->setSingleStep(1);
				sv->setPageStep(1);
				sv->setValue(value);
				sv->setToolTip(tr("<html>"
							"<b>Bay</b><br>"
							"Adjusts the volume of building allocated to this bay. "
							"One job is created for the bay operator. "
							"<br>"
							"Bays provide equipment for large scale weapons, resource harvesting equipment, tractor beams, etc. "
							"Bay equipment is directionless and it is not blocked by the building or other buildings in the city. "
							"</html>"));
				connect(sv, SIGNAL(ValueChanged(int, int)), SLOT(SetBay(int, int)));

				AuLineEditValue* svl = new AuLineEditValue(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()), *pi, this);
				connect(svl, SIGNAL(EditingFinished(int)), SLOT(BayEdited(int)));

				lbBays.insert(*pi, lb);
				m_Bays.insert(*pi, sv);
				m_BayLbs.insert(*pi, svl);
			}
		}
	}

	m_IndustriesCombo = new QComboBox(this);
	m_IndustriesCombo->setToolTip(tr("<html><b>Intended Industry</b><br>"
				"Evaluates the design for use at an industry. "
				"Reports success or failure, with a reason for failing. "
				"<br>"
				"Intended industry setting is used to filter blueprints when constructing buildings. "
				"</html>"));
	connect(m_IndustriesCombo, SIGNAL(currentIndexChanged(int)), SLOT(IndustryChanged(int)));

	m_IndustriesInfo = new QLabel(this);
	m_IndustriesInfo->setWordWrap(true);

	QScrollArea* scroll = new QScrollArea;
	{
		QGridLayout* lay = new QGridLayout;
		int row=0;
		lay->addWidget(lbHullVol, row,0);
		lay->addWidget(lbHullVolTot, row,2);
		row++;
		lay->addWidget(lbSkeleton, row,0);
		lay->addWidget(m_Skeleton, row,1);
		lay->addWidget(m_SkeletonLb, row,2);
		row++;
		lay->addWidget(lbArmor, row,0);
		lay->addWidget(m_Armor, row,1);
		lay->addWidget(m_ArmorLb, row,2);
		row++;
		lay->addWidget(lbArena, row,0);
		lay->addWidget(m_Arena, row,1);
		lay->addWidget(m_ArenaLb, row,2);
		row++;
		lay->addWidget(lbAuditorium, row,0);
		lay->addWidget(m_Auditorium, row,1);
		lay->addWidget(m_AuditoriumLb, row,2);
		row++;
		lay->addWidget(lbCapacitor, row,0);	// Battery
		lay->addWidget(m_Capacitor, row,1);
		lay->addWidget(m_CapacitorLb, row,2);
		row++;
		lay->addWidget(lbHold, row,0);	// Commodity Storage
		lay->addWidget(m_Hold, row,1);
		lay->addWidget(m_HoldLb, row,2);
		row++;
		lay->addWidget(lbPowerPlant, row,0);	// Generator
		lay->addWidget(m_PowerPlant, row,1);
		lay->addWidget(m_PowerPlantLb, row,2);
		row++;
		lay->addWidget(lbHomeLarge, row,0);
		lay->addWidget(m_HomeLarge, row,1);
		lay->addWidget(m_HomeLargeLb, row,2);
		row++;
		lay->addWidget(lbHomeMedium, row,0);
		lay->addWidget(m_HomeMedium, row,1);
		lay->addWidget(m_HomeMediumLb, row,2);
		row++;
		lay->addWidget(lbHomeSmall, row,0);
		lay->addWidget(m_HomeSmall, row,1);
		lay->addWidget(m_HomeSmallLb, row,2);
		row++;
		lay->addWidget(lbLifeSupport, row,0);
		lay->addWidget(m_LifeSupport, row,1);
		lay->addWidget(m_LifeSupportLb, row,2);
		row++;
		lay->addWidget(lbLounge, row,0);
		lay->addWidget(m_Lounge, row,1);
		lay->addWidget(m_LoungeLb, row,2);
		row++;
		lay->addWidget(lbOffice, row,0);
		lay->addWidget(m_Office, row,1);
		lay->addWidget(m_OfficeLb, row,2);
		row++;
		if(lbSensors)
		{
			lay->addWidget(lbSensors, row,0);
			lay->addWidget(m_Sensors, row,1);
			lay->addWidget(m_SensorsLb, row,2);
			row++;
		}
		if(lbShields)
		{
			lay->addWidget(lbShields, row,0);
			lay->addWidget(m_Shields, row,1);
			lay->addWidget(m_ShieldsLb, row,2);
			row++;
		}
		lay->addWidget(lbShop, row,0);
		lay->addWidget(m_Shop, row,1);
		lay->addWidget(m_ShopLb, row,2);
		row++;
		lay->addWidget(lbStore, row,0);
		lay->addWidget(m_Store, row,1);
		lay->addWidget(m_StoreLb, row,2);
		row++;
		if(!lbBays.empty())
		{
			QHash<quint16, QLabel*>::const_iterator ni = lbBays.cbegin();
			QHash<quint16, AuSliderValue*>::const_iterator si = m_Bays.cbegin();
			QHash<quint16, AuLineEditValue*>::const_iterator li = m_BayLbs.cbegin();
			while(ni != lbBays.cend() && si != m_Bays.cend() && li != m_BayLbs.cend())
			{
				lay->addWidget(*ni, row,0);
				lay->addWidget(*si, row,1);
				lay->addWidget(*li, row,2);
				row++;

				ni++;
				si++;
				li++;
			}
		}
		lay->setColumnStretch(1,1);
		QWidget* wf = new QWidget();
		wf->setLayout(lay);
		scroll->setWidget(wf);
		scroll->setWidgetResizable(true);
	}

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbName, row,0);
	lay->addWidget(m_Name, row,1, 1,2);
	row++;
	lay->addWidget(lbDesc, row,0);
	lay->addWidget(m_Description, row,1, 1,2);
	row++;
	lay->addWidget(lbStruc, row,0);
	lay->addWidget(m_Structure, row,1, 1,2);
	row++;
	lay->addWidget(lbMaterial, row,0);
	lay->addWidget(m_Hull, row,1, 1,2);
	row++;
	lay->addWidget(lbLifeSupportMod, row,0);
	lay->addWidget(m_LifeSupportMod, row,1, 1,2);
	row++;
	lay->addWidget(lbCapacitorMod, row,0);
	lay->addWidget(m_CapacitorMod, row,1, 1,2);
	row++;
	lay->addWidget(lbPowerPlantMod, row,0);
	lay->addWidget(m_PowerPlantMod, row,1, 1,2);
	row++;
	lay->addWidget(scroll, row,0, 1,3);
	lay->setColumnStretch(1,1);
	lay->setSpacing(2);
	lay->setMargin(0);

	QGridLayout* alay = new QGridLayout();
	row=0;
	alay->addWidget(m_IndustriesCombo, row,0);
	alay->addWidget(m_IndustriesInfo, row,1);
	row++;
	alay->addWidget(m_Analysis, row,0, 1,2);
	row++;
	alay->setSpacing(2);
	alay->setMargin(0);

	QWidget* controls = new QWidget(this);
	controls->setLayout(lay);

	QWidget* analysis = new QWidget(this);
	analysis->setLayout(alay);

	m_Splitter = new QSplitter;
	m_Splitter->addWidget(controls);
	m_Splitter->addWidget(analysis);
	m_Splitter->setChildrenCollapsible(false);
	m_Splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addStretch();
	blay->addWidget(m_Ok);
	blay->setSpacing(2);
	blay->setMargin(0);

	QVBoxLayout* dlay = new QVBoxLayout();
	dlay->addWidget(m_Splitter);
	dlay->addLayout(blay);

	dlay->setSpacing(2);
	dlay->setMargin(4);
	setAuDlgLayout(dlay);

	QByteArray splitterstate;
	g_Au.Settings().readEntry("Widget/BdDesignProps/Splitter", splitterstate);
	m_Splitter->restoreState(splitterstate);

	ListIndustries();
	EnableControls();
}

AuBdDesignProps::~AuBdDesignProps()
{
}

void AuBdDesignProps::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
		LimitVolumeSliders(0);
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuBdDesignProps::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/DesignBuildingProperties/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/DesignBuildingProperties/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/DesignBuildingProperties/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/DesignBuildingProperties/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuBdDesignProps::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/DesignBuildingProperties/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/DesignBuildingProperties/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/DesignBuildingProperties/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/DesignBuildingProperties/SizeY",sz.height());
}

void AuBdDesignProps::UpdateReport()
{
	QString rpt(m_DesignRec.ReportAnalysis());
	if(rpt.isEmpty())
	{
		rpt = tr("<html>"
				"<p>"
				"Design contains %1 parts. "
				"<br>"
				"Design reports no building volume and no field capacity. "
				"</p>"

				"<p>"
				"Obstruction model data is needed to analyze a design and report its building volume and other details. "
				"Be sure the obstruction model is up-to-date and the design contains at least one hull part. "
				"</p>"

				"<p>"
				"Most changes to the geometry invalidate the obstruction model. "
				"Use <i>Refresh Obstructions</i> to reconstruct the obstruction model after making changes. "
				"</p>"

				"<p>"
				"Parks require only field capacity, not building volume. "
				"To make a park, create only a field. "
				"</p>"

				"</html>")
			.arg(m_DesignRec.Parts().empty() ? tr("no") : QString("%1").arg(m_DesignRec.Parts().size()));
	}
	m_Analysis->setHtml(rpt);

	int index = m_IndustriesCombo->currentIndex();
	if(index < 0)
	{
		m_IndustriesInfo->setText("");
	}
	else
	{
		INDUSTRYID ind = (INDUSTRYID)m_IndustriesCombo->itemData(index).toInt();
		if(ind != IID_None)
		{
			AuBdReport rpt;
			m_DesignRec.ReportDesign(rpt);

			const DgData& gd = g_Au.LockDataForRead();
			const DbIndustry& dbi = gd.Industry(ind);
			int result = rpt.IndustryCanUse(dbi);
			g_Au.UnlockData();
			QString resultMsg = AuBdReport::IndustryCanUseMessage(result);
			m_IndustriesInfo->setText(resultMsg);
		}
		else
			m_IndustriesInfo->setText("");

	}
}

void AuBdDesignProps::EnableControls()
{
	bool enableApply = true;
	int index = m_Structure->currentIndex();
	if(index < 0)
		enableApply = false;

	if(!m_Name->text().isEmpty() && !IsNameValid(m_Name->text().trimmed()))	// Empty name is ok
		enableApply = false;

	m_Ok->setEnabled(enableApply);
}

void AuBdDesignProps::ListModules(QComboBox* cb, SYSTEM system, COMMODITYID currentModuleCID)
{
	if(system == SYS_Hull)
	{
		cb->addItem(QIcon(g_Au.CommodityBitmapName(CID_Wood)), g_Au.CommodityName(CID_Wood), CID_WoodHullModule);
		cb->addItem(QIcon(g_Au.CommodityBitmapName(CID_Lumber)), g_Au.CommodityName(CID_Lumber), CID_LumberHullModule);
		cb->addItem(QIcon(g_Au.CommodityBitmapName(CID_Stone)), g_Au.CommodityName(CID_Stone), CID_StoneHullModule);
		cb->addItem(QIcon(g_Au.CommodityBitmapName(CID_Metal)), g_Au.CommodityName(CID_Metal), CID_MetalHullModule);
		cb->addItem(QIcon(g_Au.CommodityBitmapName(CID_Magmium)), g_Au.CommodityName(CID_Magmium), CID_MagmiumHullModule);
		cb->addItem(QIcon(g_Au.CommodityBitmapName(CID_Vulcium)), g_Au.CommodityName(CID_Vulcium), CID_VulciumHullModule);
		cb->addItem(QIcon(g_Au.CommodityBitmapName(CID_Adamantine)), g_Au.CommodityName(CID_Adamantine), CID_AdamantineHullModule);
	}
	else
	{
		cb->addItem(QIcon(":data/ScHide.png"), tr("None"), CID_Invalid);
		for(DbCommodityArray::const_iterator ci = g_Au.m_CommodityArray.cbegin(); ci != g_Au.m_CommodityArray.cend(); ci++)
			if(DbCommodity::IsBdModule(system, (*ci).m_id))
				cb->addItem(QIcon((*ci).BitmapName()), (*ci).m_name, (*ci).m_id);
	}

	int curIndex = cb->findData(currentModuleCID);
	if(curIndex >= 0)
		cb->setCurrentIndex(curIndex);
}

void AuBdDesignProps::SetArmor(int value)
{
	m_DesignRec.SetArmorVolume(value);
	m_ArmorLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Armor);
}

void AuBdDesignProps::SetBay(int value, int partId)
{
	QHash<quint16, AuSliderValue*>::iterator bi =  m_Bays.find(partId);
	QHash<quint16, AuLineEditValue*>::iterator li =  m_BayLbs.find(partId);
	if(bi == m_Bays.cend() || li == m_BayLbs.cend())
		return;

	m_DesignRec.SetBayVolume(partId, value);
	(*li)->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(*bi);
}

void AuBdDesignProps::SetCapacitor(int value)
{
	m_DesignRec.SetCapacitorVolume(value);
	m_CapacitorLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Capacitor);
}

void AuBdDesignProps::SetHold(int value)
{
	m_DesignRec.SetHoldVolume(value);
	m_HoldLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Hold);
}

void AuBdDesignProps::SetSkeleton(int value)
{
	m_DesignRec.SetSkeletonVolume(value);
	m_SkeletonLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Skeleton);
}

void AuBdDesignProps::SetArena(int value)
{
	m_DesignRec.SetArenaVolume(value);
	m_ArenaLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Arena);
}

void AuBdDesignProps::SetAuditorium(int value)
{
	m_DesignRec.SetAuditoriumVolume(value);
	m_AuditoriumLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Auditorium);
}

void AuBdDesignProps::SetLounge(int value)
{
	m_DesignRec.SetLoungeVolume(value);
	m_LoungeLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Lounge);
}

void AuBdDesignProps::SetStore(int value)
{
	m_DesignRec.SetStoreVolume(value);
	m_StoreLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Store);
}

void AuBdDesignProps::SetHomeLarge(int value)
{
	m_DesignRec.SetHomeLargeVolume(value);
	m_HomeLargeLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_HomeLarge);
}

void AuBdDesignProps::SetHomeMedium(int value)
{
	m_DesignRec.SetHomeMediumVolume(value);
	m_HomeMediumLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_HomeMedium);
}

void AuBdDesignProps::SetHomeSmall(int value)
{
	m_DesignRec.SetHomeSmallVolume(value);
	m_HomeSmallLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_HomeSmall);
}

void AuBdDesignProps::SetOffice(int value)
{
	m_DesignRec.SetOfficeVolume(value);
	m_OfficeLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Office);
}

void AuBdDesignProps::SetShop(int value)
{
	m_DesignRec.SetShopVolume(value);
	m_ShopLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Shop);
}

void AuBdDesignProps::SetLifeSupport(int value)
{
	m_DesignRec.SetLifeSupportVolume(value);
	m_LifeSupportLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_LifeSupport);
}

void AuBdDesignProps::SetPowerPlant(int value)
{
	m_DesignRec.SetPowerPlantVolume(value);
	m_PowerPlantLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_PowerPlant);
}

void AuBdDesignProps::SetSensors(int value)
{
	m_DesignRec.SetSensorVolume(value);
	m_SensorsLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Sensors);
}

void AuBdDesignProps::SetShields(int value)
{
	m_DesignRec.SetShieldVolume(value);
	m_ShieldsLb->setText(QString("%L1m%2").arg(value).arg(g_Au.CubedSymbol()));
	LimitVolumeSliders(m_Shields);
}

void AuBdDesignProps::SplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/BdDesignProps/Splitter", m_Splitter->saveState());
}

void AuBdDesignProps::StructureChanged(int index)
{
	m_DesignRec.SetStructure((BUILDING_STRUCTURE)m_Structure->itemData(index).toInt());
	UpdateReport();
	EnableControls();
}

void AuBdDesignProps::CapacitorModuleChanged(int index)
{
	m_DesignRec.SetCapacitorModule((COMMODITYID)m_CapacitorMod->itemData(index).toInt());
	UpdateReport();
}

void AuBdDesignProps::MaterialChanged(int index)
{
	m_DesignRec.SetHullModule((COMMODITYID)m_Hull->itemData(index).toInt());
	UpdateReport();
}

void AuBdDesignProps::LifeSupportModuleChanged(int index)
{
	m_DesignRec.SetLifeSupportModule((COMMODITYID)m_LifeSupportMod->itemData(index).toInt());
	UpdateReport();
}

void AuBdDesignProps::NameChanged(const QString& text)
{
	m_DesignRec.SetName(text.trimmed());
	UpdateReport();
	EnableControls();
}

void AuBdDesignProps::DescriptionChanged()
{
	QString text;
	if(!m_Description->toPlainText().isEmpty())
		text = m_Description->toHtml();
	m_DesignRec.SetDescription(text);
	UpdateReport();
}

void AuBdDesignProps::PowerPlantModuleChanged(int index)
{
	m_DesignRec.SetPowerPlantModule((COMMODITYID)m_PowerPlantMod->itemData(index).toInt());
	UpdateReport();
}

int AuBdDesignProps::SumVolumeSliders() const
{
	int cursum = 0;
	cursum += m_Arena->value();
	cursum += m_Armor->value();
	cursum += m_Auditorium->value();
	for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
		cursum += (*bi)->value();
	cursum += m_Capacitor->value();
	cursum += m_Hold->value();
	cursum += m_HomeLarge->value();
	cursum += m_HomeMedium->value();
	cursum += m_HomeSmall->value();
	cursum += m_LifeSupport->value();
	cursum += m_Lounge->value();
	cursum += m_Office->value();
	cursum += m_PowerPlant->value();
	if(m_Sensors)
		cursum += m_Sensors->value();
	if(m_Shields)
		cursum += m_Shields->value();
	cursum += m_Shop->value();
	cursum += m_Skeleton->value();
	cursum += m_Store->value();
	return cursum;
}

void AuBdDesignProps::LimitVolumeSliders(QSlider* control)
{
	// Get the sum of the volume sliders.
	quint32 cursum = SumVolumeSliders();
	//qDebug() << "cursum" << cursum << "hullVol" << m_DesignRec.HullVolume();
	if(cursum > m_DesignRec.HullVolume())
	{
		// We must make adjustments so the sum does not exceed the hull volume.
		// Block widget signals so we don't die a recursive death here.
		m_Arena->blockSignals(true);
		m_Armor->blockSignals(true);
		m_Auditorium->blockSignals(true);
		for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
			(*bi)->blockSignals(true);
		m_Capacitor->blockSignals(true);
		m_Hold->blockSignals(true);
		m_HomeLarge->blockSignals(true);
		m_HomeMedium->blockSignals(true);
		m_HomeSmall->blockSignals(true);
		m_LifeSupport->blockSignals(true);
		m_Lounge->blockSignals(true);
		m_Office->blockSignals(true);
		m_PowerPlant->blockSignals(true);
		if(m_Sensors)
			m_Sensors->blockSignals(true);
		if(m_Shields)
			m_Shields->blockSignals(true);
		m_Shop->blockSignals(true);
		m_Skeleton->blockSignals(true);
		m_Store->blockSignals(true);

		// The amount to remove determines our strategy for balancing the other sliders.
		int remove = cursum - m_DesignRec.HullVolume();
		if(m_Skeleton != control && m_Skeleton->value() >= remove)
		{
			// We can take it all from skeleton.
			m_Skeleton->setValue(m_Skeleton->value() - remove);
		}
		else if(remove > 250)
		{
			// Removing enough to take a ratio from each control. The else leg is way slow when adjusting millions.
			// Get the total removable cubic meters so we can calculate each slider's share of the amount to be removed.
			int totalRemovable = 0;
			if(m_Arena != control)
				totalRemovable += m_Arena->value();
			if(m_Armor != control)
				totalRemovable += m_Armor->value();
			if(m_Auditorium != control)
				totalRemovable += m_Auditorium->value();
			for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
				if(*bi != control)
					totalRemovable += (*bi)->value();
			if(m_Capacitor != control)
				totalRemovable += m_Capacitor->value();
			if(m_Hold != control)
				totalRemovable += m_Hold->value();
			if(m_HomeLarge != control)
				totalRemovable += m_HomeLarge->value();
			if(m_HomeMedium != control)
				totalRemovable += m_HomeMedium->value();
			if(m_HomeSmall != control)
				totalRemovable += m_HomeSmall->value();
			if(m_LifeSupport != control)
				totalRemovable += m_LifeSupport->value();
			if(m_Lounge != control)
				totalRemovable += m_Lounge->value();
			if(m_Office != control)
				totalRemovable += m_Office->value();
			if(m_PowerPlant != control)
				totalRemovable += m_PowerPlant->value();
			if(m_Sensors && m_Sensors != control)
				totalRemovable += m_Sensors->value();
			if(m_Shields && m_Shields != control)
				totalRemovable += m_Shields->value();
			if(m_Shop != control)
				totalRemovable += m_Shop->value();
			if(m_Skeleton != control)
				totalRemovable += m_Skeleton->value();
			if(m_Store != control)
				totalRemovable += m_Store->value();

			// Remove each control's share of the total that can be removed.
			int removed = 0;
			if(removed < remove && m_Arena != control && m_Arena->value())
			{
				int v = m_Arena->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Arena->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Armor != control && m_Armor->value())
			{
				int v = m_Armor->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Armor->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Auditorium != control && m_Auditorium->value())
			{
				int v = m_Auditorium->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Auditorium->setValue(v - r);
				removed += r;
			}
			for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); removed < remove && bi != m_Bays.cend(); bi++)
			{
				if(*bi == control)
					continue;

				int v = (*bi)->value();
				if(!v)
					continue;

				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				(*bi)->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Capacitor != control && m_Capacitor->value())
			{
				int v = m_Capacitor->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Capacitor->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Hold != control && m_Hold->value())
			{
				int v = m_Hold->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Hold->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_HomeLarge != control && m_HomeLarge->value())
			{
				int v = m_HomeLarge->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_HomeLarge->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_HomeMedium != control && m_HomeMedium->value())
			{
				int v = m_HomeMedium->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_HomeMedium->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_HomeSmall != control && m_HomeSmall->value())
			{
				int v = m_HomeSmall->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_HomeSmall->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_LifeSupport != control && m_LifeSupport->value())
			{
				int v = m_LifeSupport->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_LifeSupport->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Lounge != control && m_Lounge->value())
			{
				int v = m_Lounge->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Lounge->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Office != control && m_Office->value())
			{
				int v = m_Office->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Office->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_PowerPlant != control && m_PowerPlant->value())
			{
				int v = m_PowerPlant->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_PowerPlant->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Sensors && m_Sensors != control && m_Sensors->value())
			{
				int v = m_Sensors->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Sensors->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Shields && m_Shields != control && m_Shields->value())
			{
				int v = m_Shields->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Shields->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Shop != control && m_Shop->value())
			{
				int v = m_Shop->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Shop->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Skeleton != control && m_Skeleton->value())
			{
				int v = m_Skeleton->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Skeleton->setValue(v - r);
				removed += r;
			}
			if(removed < remove && m_Store != control && m_Store->value())
			{
				int v = m_Store->value();
				float f = (float)v / (float)totalRemovable;
				int r = (int)ceilf(f * remove);
				if(!r)
					r = 1;
				else if(r + removed > remove)
					r = remove - removed;
				m_Store->setValue(v - r);
				removed += r;
			}

			if(removed < remove)
				qWarning() << "error: AuBdDesignProps::LimitVolumeSliders removed" << removed << "remove" << remove;
		}
		else
		{
			// Removing a small amount.
			// Adjust sliders by taking one off the highest slider that is not the control slider, until we are in limits.
			while(cursum > m_DesignRec.HullVolume())
			{
				// Take one off the highest slider, except the 'control' slider.
				QSlider* highest = 0;

				if(m_Arena != control && (!highest || m_Arena->value() > highest->value()))
					highest = m_Arena;

				if(m_Armor != control && (!highest || m_Armor->value() > highest->value()))
					highest = m_Armor;

				if(m_Auditorium != control && (!highest || m_Auditorium->value() > highest->value()))
					highest = m_Auditorium;

				for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
					if(*bi != control && (!highest || (*bi)->value() > highest->value()))
						highest = *bi;

				if(m_Capacitor != control && (!highest || m_Capacitor->value() > highest->value()))
					highest = m_Capacitor;

				if(m_Hold != control && (!highest || m_Hold->value() > highest->value()))
					highest = m_Hold;

				if(m_HomeLarge != control && (!highest || m_HomeLarge->value() > highest->value()))
					highest = m_HomeLarge;

				if(m_HomeMedium != control && (!highest || m_HomeMedium->value() > highest->value()))
					highest = m_HomeMedium;

				if(m_HomeSmall != control && (!highest || m_HomeSmall->value() > highest->value()))
					highest = m_HomeSmall;

				if(m_LifeSupport != control && (!highest || m_LifeSupport->value() > highest->value()))
					highest = m_LifeSupport;

				if(m_Lounge != control && (!highest || m_Lounge->value() > highest->value()))
					highest = m_Lounge;

				if(m_Office != control && (!highest || m_Office->value() > highest->value()))
					highest = m_Office;

				if(m_PowerPlant != control && (!highest || m_PowerPlant->value() > highest->value()))
					highest = m_PowerPlant;

				if(m_Sensors && m_Sensors != control && (!highest || m_Sensors->value() > highest->value()))
					highest = m_Sensors;

				if(m_Shields && m_Shields != control && (!highest || m_Shields->value() > highest->value()))
					highest = m_Shields;

				if(m_Shop != control && (!highest || m_Shop->value() > highest->value()))
					highest = m_Shop;

				if(m_Skeleton != control && (!highest || m_Skeleton->value() > highest->value()))
					highest = m_Skeleton;

				if(m_Store != control && (!highest || m_Store->value() > highest->value()))
					highest = m_Store;

				if(!highest)
					break;	// wtf?

				highest->setValue(highest->value()-1);
				cursum--;
			}
		}

		// Unblock widget signals.
		m_Arena->blockSignals(false);
		m_Armor->blockSignals(false);
		m_Auditorium->blockSignals(false);
		for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
			(*bi)->blockSignals(false);
		m_Capacitor->blockSignals(false);
		m_Hold->blockSignals(false);
		m_HomeLarge->blockSignals(false);
		m_HomeMedium->blockSignals(false);
		m_HomeSmall->blockSignals(false);
		m_LifeSupport->blockSignals(false);
		m_Lounge->blockSignals(false);
		m_Office->blockSignals(false);
		m_PowerPlant->blockSignals(false);
		if(m_Sensors)
			m_Sensors->blockSignals(false);
		if(m_Shields)
			m_Shields->blockSignals(false);
		m_Shop->blockSignals(false);
		m_Skeleton->blockSignals(false);
		m_Store->blockSignals(false);

		// Update values in design record to match sliders.
		m_DesignRec.SetArenaVolume(m_Arena->value());
		m_DesignRec.SetArmorVolume(m_Armor->value());
		m_DesignRec.SetAuditoriumVolume(m_Auditorium->value());
		for(QHash<quint16, AuSliderValue*>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
			m_DesignRec.SetBayVolume(bi.key(), (*bi)->value());
		m_DesignRec.SetCapacitorVolume(m_Capacitor->value());
		m_DesignRec.SetHoldVolume(m_Hold->value());
		m_DesignRec.SetHomeLargeVolume(m_HomeLarge->value());
		m_DesignRec.SetHomeMediumVolume(m_HomeMedium->value());
		m_DesignRec.SetHomeSmallVolume(m_HomeSmall->value());
		m_DesignRec.SetLifeSupportVolume(m_LifeSupport->value());
		m_DesignRec.SetLoungeVolume(m_Lounge->value());
		m_DesignRec.SetOfficeVolume(m_Office->value());
		m_DesignRec.SetPowerPlantVolume(m_PowerPlant->value());
		if(m_Sensors)
			m_DesignRec.SetSensorVolume(m_Sensors->value());
		if(m_Shields)
			m_DesignRec.SetShieldVolume(m_Shields->value());
		m_DesignRec.SetShopVolume(m_Shop->value());
		m_DesignRec.SetSkeletonVolume(m_Skeleton->value());
		m_DesignRec.SetStoreVolume(m_Store->value());

		// Update text values.
		m_ArenaLb->setText(QString("%L1m%2").arg(m_DesignRec.ArenaVolume()).arg(g_Au.CubedSymbol()));
		m_ArmorLb->setText(QString("%L1m%2").arg(m_DesignRec.ArmorVolume()).arg(g_Au.CubedSymbol()));
		m_AuditoriumLb->setText(QString("%L1m%2").arg(m_DesignRec.AuditoriumVolume()).arg(g_Au.CubedSymbol()));
		for(QHash<quint16, AuLineEditValue*>::iterator li = m_BayLbs.begin(); li != m_BayLbs.end(); li++)
			(*li)->setText(QString("%L1m%2").arg(m_DesignRec.BayVolume(li.key())).arg(g_Au.CubedSymbol()));
		m_CapacitorLb->setText(QString("%L1m%2").arg(m_DesignRec.CapacitorVolume()).arg(g_Au.CubedSymbol()));
		m_HoldLb->setText(QString("%L1m%2").arg(m_DesignRec.HoldVolume()).arg(g_Au.CubedSymbol()));
		m_HomeLargeLb->setText(QString("%L1m%2").arg(m_DesignRec.HomeLargeVolume()).arg(g_Au.CubedSymbol()));
		m_HomeMediumLb->setText(QString("%L1m%2").arg(m_DesignRec.HomeMediumVolume()).arg(g_Au.CubedSymbol()));
		m_HomeSmallLb->setText(QString("%L1m%2").arg(m_DesignRec.HomeSmallVolume()).arg(g_Au.CubedSymbol()));
		m_LifeSupportLb->setText(QString("%L1m%2").arg(m_DesignRec.LifeSupportVolume()).arg(g_Au.CubedSymbol()));
		m_LoungeLb->setText(QString("%L1m%2").arg(m_DesignRec.LoungeVolume()).arg(g_Au.CubedSymbol()));
		m_OfficeLb->setText(QString("%L1m%2").arg(m_DesignRec.OfficeVolume()).arg(g_Au.CubedSymbol()));
		m_PowerPlantLb->setText(QString("%L1m%2").arg(m_DesignRec.PowerPlantVolume()).arg(g_Au.CubedSymbol()));
		if(m_SensorsLb)
			m_SensorsLb->setText(QString("%L1m%2").arg(m_DesignRec.SensorVolume()).arg(g_Au.CubedSymbol()));
		if(m_ShieldsLb)
			m_ShieldsLb->setText(QString("%L1m%2").arg(m_DesignRec.ShieldVolume()).arg(g_Au.CubedSymbol()));
		m_ShopLb->setText(QString("%L1m%2").arg(m_DesignRec.ShopVolume()).arg(g_Au.CubedSymbol()));
		m_SkeletonLb->setText(QString("%L1m%2").arg(m_DesignRec.SkeletonVolume()).arg(g_Au.CubedSymbol()));
		m_StoreLb->setText(QString("%L1m%2").arg(m_DesignRec.StoreVolume()).arg(g_Au.CubedSymbol()));
	}
	else if(cursum < m_DesignRec.HullVolume())
	{
		// Adjust skeleton to use all unallocated hull volume.
		int skeleton = m_Skeleton->value() + (m_DesignRec.HullVolume() - cursum);
		m_Skeleton->blockSignals(true);
		m_Skeleton->setValue(skeleton);
		m_Skeleton->blockSignals(false);
		m_DesignRec.SetSkeletonVolume(m_Skeleton->value());
		m_SkeletonLb->setText(QString("%L1m%2").arg(m_DesignRec.SkeletonVolume()).arg(g_Au.CubedSymbol()));
	}

	UpdateReport();
}

quint32 AuBdDesignProps::VolumeValue(const QString& s) const
{
	QString s1(s);
	s1.replace("+", "");
	s1.replace("-", "");
	s1.replace(".", "");
	s1.replace(",", "");
	s1.replace("m", "");
	s1.replace(g_Au.CubedSymbol(), "");
	quint32 vol = s1.toUInt();
	if(vol > m_DesignRec.HullVolume())
		vol = m_DesignRec.HullVolume();
	return vol;
}

void AuBdDesignProps::ArmorEdited()
{
	quint32 value = VolumeValue(m_ArmorLb->text());
	m_Armor->setValue(value);
}

void AuBdDesignProps::BayEdited(int partId)
{
	QHash<quint16, AuSliderValue*>::iterator bi =  m_Bays.find(partId);
	QHash<quint16, AuLineEditValue*>::iterator li =  m_BayLbs.find(partId);
	if(bi == m_Bays.cend() || li == m_BayLbs.cend())
		return;

	quint32 value = VolumeValue((*li)->text());
	(*bi)->setValue(value);
}

void AuBdDesignProps::CapacitorEdited()
{
	quint32 value = VolumeValue(m_CapacitorLb->text());
	m_Capacitor->setValue(value);
}

void AuBdDesignProps::HoldEdited()
{
	quint32 value = VolumeValue(m_HoldLb->text());
	m_Hold->setValue(value);
}

void AuBdDesignProps::SkeletonEdited()
{
	quint32 value = VolumeValue(m_SkeletonLb->text());
	m_Skeleton->setValue(value);
}

void AuBdDesignProps::ArenaEdited()
{
	quint32 value = VolumeValue(m_ArenaLb->text());
	m_Arena->setValue(value);
}

void AuBdDesignProps::AuditoriumEdited()
{
	quint32 value = VolumeValue(m_AuditoriumLb->text());
	m_Auditorium->setValue(value);
}

void AuBdDesignProps::LoungeEdited()
{
	quint32 value = VolumeValue(m_LoungeLb->text());
	m_Lounge->setValue(value);
}

void AuBdDesignProps::StoreEdited()
{
	quint32 value = VolumeValue(m_StoreLb->text());
	m_Store->setValue(value);
}

void AuBdDesignProps::HomeLargeEdited()
{
	quint32 value = VolumeValue(m_HomeLargeLb->text());
	m_HomeLarge->setValue(value);
}

void AuBdDesignProps::HomeMediumEdited()
{
	quint32 value = VolumeValue(m_HomeMediumLb->text());
	m_HomeMedium->setValue(value);
}

void AuBdDesignProps::HomeSmallEdited()
{
	quint32 value = VolumeValue(m_HomeSmallLb->text());
	m_HomeSmall->setValue(value);
}

void AuBdDesignProps::OfficeEdited()
{
	quint32 value = VolumeValue(m_OfficeLb->text());
	m_Office->setValue(value);
}

void AuBdDesignProps::ShopEdited()
{
	quint32 value = VolumeValue(m_ShopLb->text());
	m_Shop->setValue(value);
}

void AuBdDesignProps::LifeSupportEdited()
{
	quint32 value = VolumeValue(m_LifeSupportLb->text());
	m_LifeSupport->setValue(value);
}

void AuBdDesignProps::PowerPlantEdited()
{
	quint32 value = VolumeValue(m_PowerPlantLb->text());
	m_PowerPlant->setValue(value);
}

void AuBdDesignProps::SensorsEdited()
{
	quint32 value = VolumeValue(m_SensorsLb->text());
	m_Sensors->setValue(value);
}

void AuBdDesignProps::ShieldsEdited()
{
	quint32 value = VolumeValue(m_ShieldsLb->text());
	m_Shields->setValue(value);
}

void AuBdDesignProps::ListIndustries()
{
	// Alphabetize industries.
	QMap<QString, INDUSTRYID> ind;
	const DgData& gd = g_Au.LockDataForRead();
	for(DbIndustryArray::const_iterator ii = gd.m_bIndustryArray.cbegin(); ii != gd.m_bIndustryArray.cend(); ii++)
		if((*ii).IndustryId() != IID_None && (*ii).Structure() != BD_Invalid)
			ind.insert((*ii).m_name, (*ii).IndustryId());
	g_Au.UnlockData();

	m_IndustriesCombo->blockSignals(true);
	m_IndustriesCombo->addItem(tr("<Not Specific>"), IID_None);
	for(QMap<QString, INDUSTRYID>::const_iterator si = ind.cbegin(); si != ind.cend(); si++)
		m_IndustriesCombo->addItem(si.key(), *si);
	int curi = m_IndustriesCombo->findData(m_DesignRec.IndustryId());
	if(curi < 0)
		curi = m_IndustriesCombo->findData(IID_None);
	m_IndustriesCombo->setCurrentIndex(curi);
	m_IndustriesCombo->blockSignals(false);
}

void AuBdDesignProps::IndustryChanged(int index)
{
	if(index < 0)
	{
		m_DesignRec.SetIndustryId(IID_None);
	}
	else
	{
		INDUSTRYID ind = (INDUSTRYID)m_IndustriesCombo->itemData(index).toInt();
		m_DesignRec.SetIndustryId(ind);
	}

	UpdateReport();
	EnableControls();
}
