//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aushipmentcreate.cpp,v 1.10 2022/02/19 18:16:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aushipmentcreate.h"
//
#include "aucity.h"
#include "auapp.h"
#include "auglobal.h"
#include "dgdata.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

class AuEmpireCity : public AuCity
{
public:
	float			m_DistPc;
	quint32			m_EmpireId;
	CITYTYPE		m_Type;

public:
	inline AuEmpireCity(CITYTYPE type, quint32 empire, float distPc, const AuCity& c) : AuCity(c)
	{
		m_DistPc = distPc;
		m_EmpireId = empire;
		m_Type = type;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCityTree::AuCityTree(QWidget* parent) : QTreeWidget(parent)
{
	setColumnCount(1);
	header()->hide();
}

void AuCityTree::mousePressEvent(QMouseEvent* event)
{
	QTreeWidget::mousePressEvent(event);
	m_MousePressItem = itemAt(event->pos());
}

void AuCityTree::mouseReleaseEvent(QMouseEvent* event)
{
	QTreeWidget::mouseReleaseEvent(event);

	if(event->button() == Qt::RightButton)
	{
		QTreeWidgetItem* item = itemAt(event->pos());
		if(item && item == m_MousePressItem)
			emit ItemRightClicked(item);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuShipmentCreate::AuShipmentCreate(const AuId& shipperId, quint32 shipperEmpire, bool shipperIsStation, const AuId& shipperSystemId,
		GALAXY shipperGalaxy, const AuPoint& shipperSystemLoc, QWidget* parent) : AuDialog(parent)
{
	m_ShipperEmpire = shipperEmpire;
	m_ShipperGalaxy = shipperGalaxy;
	m_ShipperId = shipperId;
	m_ShipperIsStation = shipperIsStation;
	m_ShipperSystemId = shipperSystemId;
	m_ShipperSystemLoc = shipperSystemLoc;

	setAuDlgTitle(tr("Schedule Shipment"));
	setAuDlgIcon(QIcon(":data/c_Container.png"));

	bool empireCities = g_Au.Settings().readBoolEntry("Widget/ShipmentCreate/EmpireCities", true);
	bool friendCities = g_Au.Settings().readBoolEntry("Widget/ShipmentCreate/FriendCities", false);
	bool neutralCities = g_Au.Settings().readBoolEntry("Widget/ShipmentCreate/NeutralCities", false);
	bool continuous = g_Au.Settings().readBoolEntry("Widget/ShipmentCreate/Continuous", false);
	quint32 repeat = g_Au.Settings().readNumEntry("Widget/ShipmentCreate/Repeat", 1);
	if(repeat < 1)
		repeat = 1;

	quint32 amt = g_Au.Settings().readNumEntry("Widget/ShipmentCreate/Quantity", 0);
	quint8 minQ = g_Au.Settings().readNumEntry("Widget/ShipmentCreate/MinQ", 1);
	if(minQ < 1)
		minQ = 1;
	int curSend = g_Au.Settings().readNumEntry("Widget/ShipmentCreate/SendWhat", 0);

	QLabel* lbamt = new QLabel(tr("Quantity"), this);
	lbamt->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	QLabel* lbto = new QLabel(tr("Destination"), this);
	lbto->setAlignment(Qt::AlignRight|Qt::AlignTop);

	m_Cities = new AuCityTree(this);
	if(m_ShipperIsStation)
		m_Cities->setToolTip(tr("<html>"
					"Destination city. "
					"</html>"));
	else
		m_Cities->setToolTip(tr("<html>"
					"Destination city. "
					"<br>"
					"<b>Cities in your current solar system are not listed.</b> "
					"</html>"));
	connect(m_Cities, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
			SLOT(CityChanged(QTreeWidgetItem*, QTreeWidgetItem*)));

	m_EmpireCities = new QCheckBox(tr("Empire Cities"), this);
	m_EmpireCities->setToolTip(tr("<html>"
				"When checked, empire cities are listed. "
				"This is based on the stance of the receiving empire toward your empire. "
				"</html>"));
	m_EmpireCities->setChecked(empireCities);
	connect(m_EmpireCities, SIGNAL(stateChanged(int)), SLOT(EmpireChanged(int)));

	m_FriendCities = new QCheckBox(tr("Friend Cities"), this);
	m_FriendCities->setToolTip(tr("<html>"
				"When checked, friendly cities are listed. "
				"This is based on the stance of the receiving empire toward your empire. "
				"</html>"));
	m_FriendCities->setChecked(friendCities);
	connect(m_FriendCities, SIGNAL(stateChanged(int)), SLOT(FriendChanged(int)));

	m_NeutralCities = new QCheckBox(tr("Neutral Cities"), this);
	m_NeutralCities->setToolTip(tr("<html>"
				"When checked, neutral cities are listed <u>if they are in the same solar system as an empire city</u>. "
				"This is based on the stance of the receiving empire toward your empire. "
				"</html>"));
	m_NeutralCities->setChecked(neutralCities);
	connect(m_NeutralCities, SIGNAL(stateChanged(int)), SLOT(NeutralChanged(int)));

	m_Ok = new QPushButton(tr("Ship It"), this);
	m_Ok->setToolTip(tr("<html>"
				"Schedules the shipment for processing. "
				"</html>"));
	connect(m_Ok, SIGNAL(clicked()), SLOT(ShipIt()));

	m_Amount = new QLineEdit(QString("%1").arg(amt), this);
	m_Amount->setToolTip(tr("<html>"
				"Amount of commodities to ship. "
				"</html>"));
	connect(m_Amount, SIGNAL(textChanged(const QString&)), SLOT(AmountChanged(const QString&)));

	m_Commodity = new QComboBox(this);
	m_Commodity->setToolTip(tr("<html>"
				"Commodity to ship. "
				"</html>"));
	connect(m_Commodity, SIGNAL(currentIndexChanged(int)), SLOT(CommodityChanged(int)));

	m_Category = new QComboBox(this);
	m_Category->setToolTip(tr("<html>"
				"Category of commodities to ship. "
				"</html>"));
	connect(m_Category, SIGNAL(currentIndexChanged(int)), SLOT(CategoryChanged(int)));

	m_CommodityRadio = new QRadioButton(tr("Commodity"), this);
	m_CommodityRadio->setChecked(curSend == 0);
	m_CommodityRadio->setToolTip(tr("<html>"
				"Fills the order with a specific commodity. "
				"</html>"));
	connect(m_CommodityRadio, SIGNAL(toggled(bool)), SLOT(RadioToggled(bool)));

	m_CategoryRadio = new QRadioButton(tr("Category"), this);
	m_CategoryRadio->setChecked(curSend == 1);
	m_CategoryRadio->setToolTip(tr("<html>"
				"Fills the order with a category of commodities. "
				"</html>"));
	connect(m_CategoryRadio, SIGNAL(toggled(bool)), SLOT(RadioToggled(bool)));

	QLabel* lbminQ = new QLabel(tr("Minimum Quality"), this);
	lbminQ->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_MinQ = new QComboBox(this);
	for(int q=1; q<=255; q++)
		m_MinQ->addItem(QString("Q%1").arg(q), q);
	m_MinQ->setToolTip(tr("<html>"
				"Minimum quality commodities to ship. "
				"</html>"));
	m_MinQ->setCurrentIndex(m_MinQ->findData(minQ));
	connect(m_MinQ, SIGNAL(currentIndexChanged(int)), SLOT(MinQChanged(int)));

	m_ContinuousCheck = new QCheckBox(tr("Continuous"), this);
	m_ContinuousCheck->setToolTip(tr("<html>"
				"When checked, the order is filled as often as possible, endlessly. "
				"</html>"));
	m_ContinuousCheck->setChecked(continuous);
	connect(m_ContinuousCheck, SIGNAL(stateChanged(int)), SLOT(ContinuousChanged(int)));

	QLabel* lbteu = new QLabel(this);
	lbteu->setAlignment(Qt::AlignHCenter|Qt::AlignBottom);
	lbteu->setPixmap(QPixmap(":data/c_Container.png"));
	lbteu->setToolTip(tr("<html>"
				"Cargo is shipped in Twenty-foot Equivalent Unit containers. "
				"Each TEU can hold %L1 units of cargo. "
				"Delivery fee is based on the number of TEU containers. "
				"</html>").arg(1360));

	quint32 teus = AuItemTEUContainer::TEUCount(amt);
	m_TEUs = new QLabel(tr("%L1 TEUs").arg(teus), this);
	m_TEUs->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
	m_TEUs->setToolTip(tr("<html>"
				"Cargo is shipped in Twenty-foot Equivalent Unit containers. "
				"Each TEU can hold %L1 units of cargo. "
				"Delivery fee is based on the number of TEU containers. "
				"</html>").arg(1360));

	QLabel* lbtimes = new QLabel(tr("times"), this);

	QLabel* lbrepeat = new QLabel(tr("Fill Order"), this);
	lbrepeat->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_Repeat = new QSpinBox(this);
	m_Repeat->setRange(1,250);
	m_Repeat->setValue(repeat);
	m_Repeat->setToolTip(tr("<html>"
				"Total number of times to fill the order. "
				"</html>"));
	connect(m_Repeat, SIGNAL(valueChanged(int)), SLOT(RepeatChanged(int)));

	QPushButton* pbreq = new QPushButton(tr("Request Empires"));
	pbreq->setToolTip(tr("<html><b>Request Empires</b><br>"
				"Requests data about all empires. "
				"This may be needed to see cities that belong to friendly and neutral empires. "
				"</html>"));
	connect(pbreq, SIGNAL(clicked(bool)), SLOT(RequestEmpireData()));

	QGroupBox* shipG = new QGroupBox(tr("Send"), this);
	{
		QVBoxLayout* tlay = new QVBoxLayout;
		tlay->addWidget(lbteu);
		tlay->addWidget(m_TEUs);
		tlay->setMargin(0);
		tlay->setSpacing(2);

		QGridLayout* lay = new QGridLayout;
		int row=0;
		lay->addWidget(m_CommodityRadio, row,0);
		lay->addWidget(m_Commodity, row,1, 1,2);
		lay->addLayout(tlay, row,3, 3,1);
		row++;
		lay->addWidget(m_CategoryRadio, row,0);
		lay->addWidget(m_Category, row,1, 1,2);
		row++;
		lay->addWidget(lbamt, row,0);
		lay->addWidget(m_Amount, row,1, 1,2);
		row++;
		lay->addWidget(lbminQ, row,0);
		lay->addWidget(m_MinQ, row,1);
		row++;
		shipG->setLayout(lay);
	}

	QVBoxLayout* cklay = new QVBoxLayout;
	cklay->addWidget(m_EmpireCities);
	cklay->addWidget(m_FriendCities);
	cklay->addWidget(m_NeutralCities);
	cklay->addStretch();
	cklay->setMargin(0);
	cklay->setSpacing(2);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbreq);
	blay->addWidget(m_Ok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(shipG, row,0, 1,4);
	row++;
	lay->addWidget(lbto, row,0);
	lay->addWidget(m_Cities, row,1, 1,2);
	lay->addLayout(cklay, row,3, 1,1);
	row++;
	lay->addWidget(m_ContinuousCheck, row,1, 1,2);
	row++;
	lay->addWidget(lbrepeat, row,0);
	lay->addWidget(m_Repeat, row,1);
	lay->addWidget(lbtimes, row,2);
	row++;
	lay->addLayout(blay, row,1, 1,3);
	row++;
	lay->setColumnStretch(1, 2);
	lay->setColumnStretch(2, 1);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuShipmentCreate::~AuShipmentCreate()
{
}

void AuShipmentCreate::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
		ListCities();
		ListCategories();
		ListCommodities();
		EnableControls();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuShipmentCreate::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/ShipmentCreate/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/ShipmentCreate/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/ShipmentCreate/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/ShipmentCreate/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuShipmentCreate::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/ShipmentCreate/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/ShipmentCreate/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/ShipmentCreate/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/ShipmentCreate/SizeY",sz.height());
}

void AuShipmentCreate::ListCategories()
{
	m_Category->blockSignals(true);
	m_Category->clear();

	QMap<QString, COMMODITYFILTER> sortedNames;
	for(int cat = FILTER_All; cat <= FILTER_Count; cat++)
	{
		COMMODITYFILTER filter = (COMMODITYFILTER)cat;
		if(!DbCommodity::IsFilterValid(filter))
			continue;
		sortedNames.insert(CommodityFilterName(filter), filter);
	}

	for(QMap<QString, COMMODITYFILTER>::const_iterator si = sortedNames.cbegin(); si != sortedNames.cend(); si++)
		m_Category->addItem(QIcon(CommodityFilterIcon(*si)), si.key(), *si);

	COMMODITYFILTER filter = (COMMODITYFILTER)g_Au.Settings().readNumEntry("Widget/ShipmentCreate/Category", FILTER_All);
	m_Category->setCurrentIndex(m_Category->findData(filter));
	m_Category->blockSignals(false);
}

void AuShipmentCreate::ListCommodities()
{
	m_Commodity->blockSignals(true);
	m_Commodity->clear();

	// Get a list of the commodities that fit into the current filter.
	QList<COMMODITYID> cidFilter;
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_Ammo);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_ArmorAndClothes);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_ConstructionMaterials);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_Food);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_Fuel);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_MedicalSupplies);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_Misc);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_SpareParts);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_Tools);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_Weapons);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_NaturalResources);

	QMap<QString, COMMODITYID> sortedNames;
	for(QList<COMMODITYID>::const_iterator fi = cidFilter.constBegin(); fi != cidFilter.constEnd(); fi++)
	{
		// The commodity array is intended to be indexable by the commodity id.
		const DbCommodity& commodity = g_Au.Commodity(*fi);
		if(!commodity.CanBeCargo())
			continue;	// Don't list things that can't be carried.
		sortedNames.insert(commodity.m_name.toUpper(), *fi);
	}

	// List commodities according to the current filter.
	for(QMap<QString, COMMODITYID>::const_iterator si = sortedNames.cbegin(); si != sortedNames.cend(); si++)
	{
		// The commodity array is intended to be indexable by the commodity id.
		const DbCommodity& commodity = g_Au.Commodity(*si);
		QPixmap px32 = QPixmap(commodity.BitmapName()).scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		QIcon icon(px32);

		m_Commodity->addItem(icon, commodity.m_name, commodity.m_id);
	}

	COMMODITYID cid = (COMMODITYID)g_Au.Settings().readNumEntry("Widget/ShipmentCreate/Commodity", CID_Ore);
	m_Commodity->setCurrentIndex(m_Commodity->findData(cid));
	m_Commodity->blockSignals(false);
}

void AuShipmentCreate::ListCities()
{
	m_Cities->blockSignals(true);
	QMap<QString, AuEmpireCity> cities;
	const DgData& gd = g_Au.LockDataForRead();
	DgEmpireHash::const_iterator sei = gd.m_gEmpires.find(m_ShipperEmpire);
	if(sei != gd.m_gEmpires.cend())
	{
		// Shipper empire found.
		for(DgEmpireHash::const_iterator ei = gd.m_gEmpires.cbegin(); ei != gd.m_gEmpires.cend(); ei++)
		{
			// The potential recipient's stance toward my empire.
			POLITICAL_STANCE stance = (*ei).second.PoliticalStance(m_ShipperEmpire);
			bool ckInSystem = false;
			switch(stance)
			{
			case POL_Friend:
			case POL_Vassal:
				if(!m_FriendCities->isChecked())
					continue;
				break;

			case POL_Neutral:
				if(!m_NeutralCities->isChecked())
					continue;
				ckInSystem = true;
				break;

			case POL_Enemy:
				continue;			// Don't list enemy cities at all.

			case POL_Empire:
				if(!m_EmpireCities->isChecked())
					continue;
				break;
			}

			// List the empire's cities.
			for(AuCityHash::const_iterator ci = (*ei).second.m_Cities->cbegin(); ci != (*ei).second.m_Cities->cend(); ci++)
			{
				// Don't list cities in the same system as the shipper.
				if((*ci).second.SystemId() == m_ShipperSystemId)
					continue;

				// With neutrals, we only want cities that exist in systems where shipper has cities.
				if(ckInSystem && !(*sei).second.HasCityInSystem((*ci).second.SystemId()))
					continue;

				// Calculate delivery distance in pc.
				float distPc;
				if((*ci).second.Galaxy() == m_ShipperGalaxy)
					distPc = m_ShipperSystemLoc.Distance((*ci).second.SystemLoc());				// Direct distance.
				else
					distPc = m_ShipperSystemLoc.Length() + (*ci).second.SystemLoc().Length();	// To from center of G.

				// Make a description that sounds kind of like a delivery address.
				QString desc = tr("%1 on %2, %3 system, %4 sector, %5")
					.arg((*ci).second.CityName())
					.arg((*ci).second.PlanetName())
					.arg((*ci).second.SystemName())
					.arg((*ci).second.SectorName())
					.arg((*ci).second.GalaxyName());

				cities.insert(desc, AuEmpireCity((*ci).second.CityType(), (*ei).second.m_id, distPc, (*ci).second));
			}
		}
	}
	g_Au.UnlockData();

	AuId selCity(g_Au.Settings().readEntry("Widget/ShipmentCreate/DestCity"));

	m_Cities->clear();
	QList<QTreeWidgetItem*> expand;
	QTreeWidgetItem* curCity = 0;
	QHash<AuId, QTreeWidgetItem*> atomItems;
	for(QMap<QString, AuEmpireCity>::const_iterator ci = cities.cbegin(); ci != cities.cend(); ci++)
	{
		// Top level is galaxy.
		QTreeWidgetItem* galaxy;
		QHash<AuId, QTreeWidgetItem*>::const_iterator gali = atomItems.find((*ci).GalaxyId());
		if(gali == atomItems.cend())
		{
			// Create the galaxy item.
			galaxy = new QTreeWidgetItem(m_Cities);
			galaxy->setIcon(0, QIcon(":data/RangeGalaxy.png"));
			galaxy->setText(0, (*ci).GalaxyName());
			galaxy->setData(0, Qt::UserRole, (*ci).GalaxyId().GetID());
			atomItems.insert((*ci).GalaxyId(), galaxy);
		}
		else
			galaxy = *gali;

		// Next level is sector.
		QTreeWidgetItem* sector;
		QHash<AuId, QTreeWidgetItem*>::const_iterator seci = atomItems.find((*ci).SectorId());
		if(seci == atomItems.cend())
		{
			// Create the sector item.
			sector = new QTreeWidgetItem(galaxy);
			sector->setIcon(0, QIcon(":data/RangeSector.png"));
			sector->setText(0, (*ci).SectorName());
			sector->setData(0, Qt::UserRole, (*ci).SectorId().GetID());
			if((*ci).CityId() == selCity)
				sector->setExpanded(true);
			atomItems.insert((*ci).SectorId(), sector);
		}
		else
			sector = *seci;

		// Next level is solar system.
		QTreeWidgetItem* system;
		QHash<AuId, QTreeWidgetItem*>::const_iterator sysi = atomItems.find((*ci).SystemId());
		if(sysi == atomItems.cend())
		{
			// Create the system item.
			system = new QTreeWidgetItem(sector);
			system->setIcon(0, QIcon(":data/RangeSystem.png"));
			system->setText(0, (*ci).SystemName());
			system->setData(0, Qt::UserRole, (*ci).SystemId().GetID());
			if((*ci).CityId() == selCity)
				system->setExpanded(true);
			sector->sortChildren(0, Qt::AscendingOrder);
			atomItems.insert((*ci).SystemId(), system);
		}
		else
			system = *sysi;

		/*
		// Next level is world.
		QTreeWidgetItem* world;
		QHash<AuId, QTreeWidgetItem*>::const_iterator wldi = atomItems.find((*ci).PlanetId());
		if(wldi == atomItems.cend())
		{
			// Create the world item.
			world = new QTreeWidgetItem(system);
			world->setIcon(0, QIcon(":data/RangeHail.png"));
			world->setText(0, (*ci).PlanetName());
			world->setData(0, Qt::UserRole, (*ci).PlanetId().GetID());
			world->setExpanded(true);
			system->sortChildren(0, Qt::AscendingOrder);
			atomItems.insert((*ci).PlanetId(), world);
		}
		else
			world = *wldi;
			*/

		// Create the city item.
		QIcon icon;
		const DgData& gd = g_Au.LockDataForRead();
		quint32 cityEmpire = gd.CityEmpire((*ci).CityId());
		DgEmpireHash::const_iterator ei = gd.m_gEmpires.find(cityEmpire);
		if(ei != gd.m_gEmpires.cend())
			icon = QIcon(QPixmap::fromImage((*ei).second.m_flag));
		else
			icon = QIcon(AuCity::CityIcon((*ci).m_Type));
		g_Au.UnlockData();

		QTreeWidgetItem* city = new QTreeWidgetItem(system);
		city->setIcon(0, icon);
		city->setText(0, (*ci).CityName());
		city->setData(0, Qt::UserRole, (*ci).CityId().GetID());
		city->setData(0, Qt::UserRole+1, (*ci).CityId().GetID());	// Only city has city id in role+1
		city->setData(0, Qt::UserRole+2, (*ci).m_DistPc);
		if((*ci).CityId() == selCity)
			city->setSelected(true);
		system->sortChildren(0, Qt::AscendingOrder);

		if((*ci).CityId() == selCity)
		{
			curCity = city;
			expand.append(galaxy);
			expand.append(sector);
			expand.append(system);
		}
	}
	m_Cities->sortItems(0, Qt::AscendingOrder);
	for(QList<QTreeWidgetItem*>::iterator ei = expand.begin(); ei != expand.end(); ei++)
		(*ei)->setExpanded(true);
	if(curCity)
		m_Cities->setCurrentItem(curCity);
	m_Cities->blockSignals(false);
}

quint32 AuShipmentCreate::Amount() const
{
	return m_Amount->text().toUInt();
}

COMMODITYFILTER AuShipmentCreate::Category() const
{
	return (COMMODITYFILTER)m_Category->itemData(m_Category->currentIndex()).toInt();
}

COMMODITYID AuShipmentCreate::CommodityId() const
{
	return (COMMODITYID)m_Commodity->itemData(m_Commodity->currentIndex()).toInt();
}

int AuShipmentCreate::SendWhat() const
{
	if(m_CommodityRadio->isChecked())
		return 0;
	return 1;
}

quint8 AuShipmentCreate::MinQ() const
{
	return m_MinQ->itemData(m_MinQ->currentIndex()).toUInt();
}

quint8 AuShipmentCreate::Repeat() const
{
	if(m_ContinuousCheck->isChecked())
		return 0;
	else
		return m_Repeat->value();
}

AuId AuShipmentCreate::DestCityId() const
{
	QTreeWidgetItem* city = m_Cities->currentItem();
	if(!city)
		return AuId(0);

	AuId cityId(city->data(0, Qt::UserRole+1).toUInt());
	return cityId;
}

QString AuShipmentCreate::DestDesc() const
{
	AuId cityId(DestCityId());
	if(cityId.IsNull())
		return QString();

	// Get the info from the city record.
	QString desc;
	const DgData& gd = g_Au.LockDataForRead();
	quint32 cityEmpire = gd.CityEmpire(cityId);
	DgEmpireHash::const_iterator ei = gd.m_gEmpires.find(cityEmpire);
	if(ei != gd.m_gEmpires.cend())
	{
		AuCityHash::const_iterator ci = (*ei).second.m_Cities->find(cityId);
		if(ci != (*ei).second.m_Cities->cend())
		{
			desc = tr("%1 on %2, %3 system, %4 sector, %5")
				.arg((*ci).second.CityName())
				.arg((*ci).second.PlanetName())
				.arg((*ci).second.SystemName())
				.arg((*ci).second.SectorName())
				.arg((*ci).second.GalaxyName());
		}
	}
	g_Au.UnlockData();

	return desc;
}

float AuShipmentCreate::DestDistPc() const
{
	QTreeWidgetItem* city = m_Cities->currentItem();
	if(!city)
		return 0;

	AuId cityId(city->data(0, Qt::UserRole+1).toUInt());
	if(cityId.IsNull())
		return 0;	// Not a city item.

	return city->data(0, Qt::UserRole+2).toFloat();
}

AuId AuShipmentCreate::DestSectorId() const
{
	AuId cityId(DestCityId());
	if(cityId.IsNull())
		return AuId(0);

	// Get the info from the city record.
	AuId id;
	const DgData& gd = g_Au.LockDataForRead();
	quint32 cityEmpire = gd.CityEmpire(cityId);
	DgEmpireHash::const_iterator ei = gd.m_gEmpires.find(cityEmpire);
	if(ei != gd.m_gEmpires.cend())
	{
		AuCityHash::const_iterator ci = (*ei).second.m_Cities->find(cityId);
		if(ci != (*ei).second.m_Cities->cend())
			id = (*ci).second.SectorId();
	}
	g_Au.UnlockData();

	return id;
}

AuId AuShipmentCreate::DestSystemId() const
{
	AuId cityId(DestCityId());
	if(cityId.IsNull())
		return AuId(0);

	// Get the info from the city record.
	AuId id;
	const DgData& gd = g_Au.LockDataForRead();
	quint32 cityEmpire = gd.CityEmpire(cityId);
	DgEmpireHash::const_iterator ei = gd.m_gEmpires.find(cityEmpire);
	if(ei != gd.m_gEmpires.cend())
	{
		AuCityHash::const_iterator ci = (*ei).second.m_Cities->find(cityId);
		if(ci != (*ei).second.m_Cities->cend())
			id = (*ci).second.SystemId();
	}
	g_Au.UnlockData();

	return id;
}

void AuShipmentCreate::EnableControls()
{
	m_Commodity->setEnabled(m_CommodityRadio->isChecked());

	m_Category->setEnabled(m_CategoryRadio->isChecked());

	bool enableOk = true;
	if(CommodityId() == CID_Invalid)
		enableOk = false;

	if(!Amount())
		enableOk = false;

	AuId cityId(DestCityId());
	if(cityId.IsNull())
		enableOk = false;

	m_Ok->setEnabled(enableOk);

	bool enableRepeat = true;
	if(m_ContinuousCheck->isChecked())
		enableRepeat = false;

	m_Repeat->setEnabled(enableRepeat);
}

void AuShipmentCreate::AmountChanged(const QString& text)
{
	quint32 amt = text.toUInt();
	quint32 teus = AuItemTEUContainer::TEUCount(amt);
	m_TEUs->setText(tr("%L1 TEUs").arg(teus));
	EnableControls();
}

void AuShipmentCreate::CityChanged(QTreeWidgetItem*, QTreeWidgetItem*)
{
	EnableControls();
}

void AuShipmentCreate::CommodityChanged(int)
{
	EnableControls();
}

void AuShipmentCreate::CategoryChanged(int)
{
	EnableControls();
}

void AuShipmentCreate::MinQChanged(int)
{
	EnableControls();
}

void AuShipmentCreate::ContinuousChanged(int)
{
	EnableControls();
}

void AuShipmentCreate::EmpireChanged(int)
{
	ListCities();
	EnableControls();
}

void AuShipmentCreate::FriendChanged(int)
{
	ListCities();
	EnableControls();
}

void AuShipmentCreate::NeutralChanged(int)
{
	ListCities();
	EnableControls();
}

void AuShipmentCreate::RepeatChanged(int)
{
	EnableControls();
}

void AuShipmentCreate::RequestEmpireData()
{
	auApp->RequestEmpiresAll();
	QMessageBox::information(this, tr("Empire Data Requested"),
			tr("Information about all empires was requested. It may take a\n"
				"minute or two to receive the information."));
	reject();
}

void AuShipmentCreate::ShipIt()
{
	g_Au.Settings().writeEntry("Widget/ShipmentCreate/EmpireCities", m_EmpireCities->isChecked());
	g_Au.Settings().writeEntry("Widget/ShipmentCreate/FriendCities", m_FriendCities->isChecked());
	g_Au.Settings().writeEntry("Widget/ShipmentCreate/NeutralCities", m_NeutralCities->isChecked());
	g_Au.Settings().writeEntry("Widget/ShipmentCreate/Continuous", m_ContinuousCheck->isChecked());
	g_Au.Settings().writeEntry("Widget/ShipmentCreate/Repeat", (int)Repeat());

	g_Au.Settings().writeEntry("Widget/ShipmentCreate/Commodity", CommodityId());
	g_Au.Settings().writeEntry("Widget/ShipmentCreate/Category", Category());
	g_Au.Settings().writeEntry("Widget/ShipmentCreate/Quantity", Amount());
	g_Au.Settings().writeEntry("Widget/ShipmentCreate/MinQ", MinQ());

	AuId cityId(DestCityId());
	if(cityId.IsNull())
		g_Au.Settings().writeEntry("Widget/ShipmentCreate/DestCity", 0);
	else
		g_Au.Settings().writeEntry("Widget/ShipmentCreate/DestCity", cityId.StringId());

	accept();
}

void AuShipmentCreate::RadioToggled(bool checked)
{
	Q_UNUSED(checked);
	EnableControls();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuTradeAgreementCreate::AuTradeAgreementCreate(const AuId& shipperId, quint32 shipperEmpire, const AuId& shipperSystemId,
		GALAXY shipperGalaxy, const AuPoint& shipperSystemLoc, QWidget* parent) : AuDialog(parent)
{
	m_ShipperEmpire = shipperEmpire;
	m_ShipperGalaxy = shipperGalaxy;
	m_ShipperId = shipperId;
	m_ShipperSystemId = shipperSystemId;
	m_ShipperSystemLoc = shipperSystemLoc;

	m_DestFilter = (COMMODITYFILTER)g_Au.Settings().readNumEntry("Widget/TradeAgreementCreate/GetCategory", FILTER_All);
	m_OrigFilter = (COMMODITYFILTER)g_Au.Settings().readNumEntry("Widget/TradeAgreementCreate/SendCategory", FILTER_All);
	m_DestCID = (COMMODITYID)g_Au.Settings().readNumEntry("Widget/TradeAgreementCreate/GetCommodity", CID_Ore);
	m_OrigCID = (COMMODITYID)g_Au.Settings().readNumEntry("Widget/TradeAgreementCreate/SendCommodity", CID_Ore);

	m_DestCity = AuId(g_Au.Settings().readEntry("Widget/TradeAgreementCreate/DestCity"));

	bool empireCities = g_Au.Settings().readBoolEntry("Widget/TradeAgreementCreate/EmpireCities", true);
	bool friendCities = g_Au.Settings().readBoolEntry("Widget/TradeAgreementCreate/FriendCities", false);
	bool neutralCities = g_Au.Settings().readBoolEntry("Widget/TradeAgreementCreate/NeutralCities", false);

	bool continuous = g_Au.Settings().readBoolEntry("Widget/TradeAgreementCreate/Continuous", false);
	quint32 repeat = g_Au.Settings().readNumEntry("Widget/TradeAgreementCreate/Repeat", 1);
	if(repeat < 1)
		repeat = 1;

	quint32 getamt = g_Au.Settings().readNumEntry("Widget/TradeAgreementCreate/GetQuantity", 0);
	quint8 getminQ = g_Au.Settings().readNumEntry("Widget/TradeAgreementCreate/GetMinQ", 1);
	if(getminQ < 1)
		getminQ = 1;
	int getwhat = g_Au.Settings().readNumEntry("Widget/TradeAgreementCreate/GetWhat", 0);

	quint32 sendamt = g_Au.Settings().readNumEntry("Widget/TradeAgreementCreate/SendQuantity", 0);
	quint8 sendminQ = g_Au.Settings().readNumEntry("Widget/TradeAgreementCreate/SendMinQ", 1);
	if(sendminQ < 1)
		sendminQ = 1;
	int sendwhat = g_Au.Settings().readNumEntry("Widget/TradeAgreementCreate/SendWhat", 0);

	setAuDlgTitle(tr("Propose Trade"));
	setAuDlgIcon(QIcon(":data/CommTrade.png"));

	QLabel* lbto = new QLabel(tr("Destination"), this);
	lbto->setAlignment(Qt::AlignRight|Qt::AlignTop);

	m_Cities = new AuCityTree(this);
	m_Cities->setToolTip(tr("<html>"
				"Destination broker city. "
				"<br>"
				"<b>Cities in your current solar system are not listed.</b> "
				"</html>"));
	connect(m_Cities, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
			SLOT(CityChanged(QTreeWidgetItem*, QTreeWidgetItem*)));

	m_EmpireCities = new QCheckBox(tr("Empire Cities"), this);
	m_EmpireCities->setToolTip(tr("<html>"
				"When checked, empire cities are listed. "
				"This is based on the stance of the receiving empire toward your empire. "
				"</html>"));
	m_EmpireCities->setChecked(empireCities);
	connect(m_EmpireCities, SIGNAL(stateChanged(int)), SLOT(EmpireChanged(int)));

	m_FriendCities = new QCheckBox(tr("Friend Cities"), this);
	m_FriendCities->setToolTip(tr("<html>"
				"When checked, friendly cities are listed. "
				"This is based on the stance of the receiving empire toward your empire. "
				"</html>"));
	m_FriendCities->setChecked(friendCities);
	connect(m_FriendCities, SIGNAL(stateChanged(int)), SLOT(FriendChanged(int)));

	m_NeutralCities = new QCheckBox(tr("Neutral Cities"), this);
	m_NeutralCities->setToolTip(tr("<html>"
				"When checked, neutral cities are listed <u>if they are in the same solar system as an empire city</u>. "
				"This is based on the stance of the receiving empire toward your empire. "
				"</html>"));
	m_NeutralCities->setChecked(neutralCities);
	connect(m_NeutralCities, SIGNAL(stateChanged(int)), SLOT(NeutralChanged(int)));

	m_Ok = new QPushButton(tr("Propose Trade"), this);
	m_Ok->setToolTip(tr("<html>"
				"Sends the trade proposal to the destination broker city. "
				"Someone at the destination broker must accept the proposal before trading will proceed. "
				"</html>"));
	connect(m_Ok, SIGNAL(clicked()), SLOT(ShipIt()));

	m_ContinuousCheck = new QCheckBox(tr("Continuous"), this);
	m_ContinuousCheck->setToolTip(tr("<html>"
				"When checked, the order is filled as often as possible, endlessly. "
				"</html>"));
	m_ContinuousCheck->setChecked(continuous);
	connect(m_ContinuousCheck, SIGNAL(stateChanged(int)), SLOT(ContinuousChanged(int)));

	QLabel* lbtimes = new QLabel(tr("times"), this);

	QLabel* lbrepeat = new QLabel(tr("Fill Order"), this);
	lbrepeat->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_Repeat = new QSpinBox(this);
	m_Repeat->setRange(1,250);
	m_Repeat->setValue(repeat);
	m_Repeat->setToolTip(tr("<html>"
				"Total number of times to fill the order. "
				"</html>"));
	connect(m_Repeat, SIGNAL(valueChanged(int)), SLOT(RepeatChanged(int)));

	QPushButton* pbreq = new QPushButton(tr("Request Empires"));
	pbreq->setToolTip(tr("<html><b>Request Empires</b><br>"
				"Requests data about all empires. "
				"This may be needed to see cities that belong to friendly and neutral empires. "
				"</html>"));
	connect(pbreq, SIGNAL(clicked(bool)), SLOT(RequestEmpireData()));

	QLabel* lbgetamt = new QLabel(tr("Quantity"), this);
	lbgetamt->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	QLabel* lbsendamt = new QLabel(tr("Quantity"), this);
	lbsendamt->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_GetAmount = new QLineEdit(QString("%1").arg(getamt), this);
	m_GetAmount->setToolTip(tr("<html>"
				"Amount of commodities to ship. "
				"</html>"));
	connect(m_GetAmount, SIGNAL(textChanged(const QString&)), SLOT(GetAmountChanged(const QString&)));

	m_SendAmount = new QLineEdit(QString("%1").arg(sendamt), this);
	m_SendAmount->setToolTip(tr("<html>"
				"Amount of commodities to ship. "
				"</html>"));
	connect(m_SendAmount, SIGNAL(textChanged(const QString&)), SLOT(SendAmountChanged(const QString&)));

	m_GetCommodity = new QComboBox(this);
	m_GetCommodity->setToolTip(tr("<html>"
				"Commodity to ship. "
				"</html>"));
	connect(m_GetCommodity, SIGNAL(currentIndexChanged(int)), SLOT(GetCommodityChanged(int)));

	m_SendCommodity = new QComboBox(this);
	m_SendCommodity->setToolTip(tr("<html>"
				"Commodity to ship. "
				"</html>"));
	connect(m_SendCommodity, SIGNAL(currentIndexChanged(int)), SLOT(SendCommodityChanged(int)));

	m_GetCategory = new QComboBox(this);
	m_GetCategory->setToolTip(tr("<html>"
				"Category of commodities to ship. "
				"</html>"));
	connect(m_GetCategory, SIGNAL(currentIndexChanged(int)), SLOT(GetCategoryChanged(int)));

	m_SendCategory = new QComboBox(this);
	m_SendCategory->setToolTip(tr("<html>"
				"Category of commodities to ship. "
				"</html>"));
	connect(m_SendCategory, SIGNAL(currentIndexChanged(int)), SLOT(SendCategoryChanged(int)));

	m_SendCommodityRadio = new QRadioButton(tr("Commodity"), this);
	m_SendCommodityRadio->setChecked(sendwhat == 0);
	m_SendCommodityRadio->setToolTip(tr("<html>"
				"Fills the order with a specific commodity. "
				"</html>"));

	m_SendCategoryRadio = new QRadioButton(tr("Category"), this);
	m_SendCategoryRadio->setChecked(sendwhat == 1);
	m_SendCategoryRadio->setToolTip(tr("<html>"
				"Fills the order with a category of commodities. "
				"</html>"));
	connect(m_SendCommodityRadio, SIGNAL(toggled(bool)), SLOT(SendRadioToggled(bool)));
	connect(m_SendCategoryRadio, SIGNAL(toggled(bool)), SLOT(SendRadioToggled(bool)));

	QButtonGroup* sbg = new QButtonGroup(this);
	sbg->addButton(m_SendCommodityRadio);
	sbg->addButton(m_SendCategoryRadio);

	m_GetCommodityRadio = new QRadioButton(tr("Commodity"), this);
	m_GetCommodityRadio->setChecked(getwhat == 0);
	m_GetCommodityRadio->setToolTip(tr("<html>"
				"Fills the order with a specific commodity. "
				"</html>"));

	m_GetCategoryRadio = new QRadioButton(tr("Category"), this);
	m_GetCategoryRadio->setChecked(getwhat == 1);
	m_GetCategoryRadio->setToolTip(tr("<html>"
				"Fills the order with a category of commodities. "
				"</html>"));
	connect(m_GetCommodityRadio, SIGNAL(toggled(bool)), SLOT(GetRadioToggled(bool)));
	connect(m_GetCategoryRadio, SIGNAL(toggled(bool)), SLOT(GetRadioToggled(bool)));

	QButtonGroup* gbg = new QButtonGroup(this);
	gbg->addButton(m_GetCommodityRadio);
	gbg->addButton(m_GetCategoryRadio);

	QLabel* lbgetminQ = new QLabel(tr("Minimum Quality"), this);
	lbgetminQ->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	QLabel* lbsendminQ = new QLabel(tr("Minimum Quality"), this);
	lbsendminQ->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_GetMinQ = new QComboBox(this);
	for(int q=1; q<=255; q++)
		m_GetMinQ->addItem(QString("Q%1").arg(q), q);
	m_GetMinQ->setToolTip(tr("<html>"
				"Minimum quality commodities to ship. "
				"</html>"));
	m_GetMinQ->setCurrentIndex(m_GetMinQ->findData(getminQ));
	connect(m_GetMinQ, SIGNAL(currentIndexChanged(int)), SLOT(GetMinQChanged(int)));

	m_SendMinQ = new QComboBox(this);
	for(int q=1; q<=255; q++)
		m_SendMinQ->addItem(QString("Q%1").arg(q), q);
	m_SendMinQ->setToolTip(tr("<html>"
				"Minimum quality commodities to ship. "
				"</html>"));
	m_SendMinQ->setCurrentIndex(m_SendMinQ->findData(sendminQ));
	connect(m_SendMinQ, SIGNAL(currentIndexChanged(int)), SLOT(SendMinQChanged(int)));

	QLabel* lbgetteu = new QLabel(this);
	lbgetteu->setAlignment(Qt::AlignHCenter|Qt::AlignBottom);
	lbgetteu->setPixmap(QPixmap(":data/c_Container.png"));
	lbgetteu->setToolTip(tr("<html>"
				"Cargo is shipped in Twenty-foot Equivalent Unit containers. "
				"Each TEU can hold %L1 units of cargo. "
				"Delivery fee is based on the number of TEU containers. "
				"</html>").arg(1360));

	QLabel* lbsendteu = new QLabel(this);
	lbsendteu->setAlignment(Qt::AlignHCenter|Qt::AlignBottom);
	lbsendteu->setPixmap(QPixmap(":data/c_Container.png"));
	lbsendteu->setToolTip(tr("<html>"
				"Cargo is shipped in Twenty-foot Equivalent Unit containers. "
				"Each TEU can hold %L1 units of cargo. "
				"Delivery fee is based on the number of TEU containers. "
				"</html>").arg(1360));

	quint32 getteus = AuItemTEUContainer::TEUCount(getamt);
	m_GetTEUs = new QLabel(tr("%L1 TEUs").arg(getteus), this);
	m_GetTEUs->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
	m_GetTEUs->setToolTip(tr("<html>"
				"Cargo is shipped in Twenty-foot Equivalent Unit containers. "
				"Each TEU can hold %L1 units of cargo. "
				"Delivery fee is based on the number of TEU containers. "
				"</html>").arg(1360));

	quint32 sendteus = AuItemTEUContainer::TEUCount(sendamt);
	m_SendTEUs = new QLabel(tr("%L1 TEUs").arg(sendteus), this);
	m_SendTEUs->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
	m_SendTEUs->setToolTip(tr("<html>"
				"Cargo is shipped in Twenty-foot Equivalent Unit containers. "
				"Each TEU can hold %L1 units of cargo. "
				"Delivery fee is based on the number of TEU containers. "
				"</html>").arg(1360));

	QGroupBox* shipG = new QGroupBox(tr("Offer to Send"), this);
	{
		QVBoxLayout* tlay = new QVBoxLayout;
		tlay->addWidget(lbsendteu);
		tlay->addWidget(m_SendTEUs);
		tlay->setMargin(0);
		tlay->setSpacing(2);

		QGridLayout* lay = new QGridLayout;
		int row=0;
		lay->addWidget(m_SendCommodityRadio, row,0);
		lay->addWidget(m_SendCommodity, row,1, 1,2);
		lay->addLayout(tlay, row,3, 3,1);
		row++;
		lay->addWidget(m_SendCategoryRadio, row,0);
		lay->addWidget(m_SendCategory, row,1, 1,2);
		row++;
		lay->addWidget(lbsendamt, row,0);
		lay->addWidget(m_SendAmount, row,1, 1,2);
		row++;
		lay->addWidget(lbsendminQ, row,0);
		lay->addWidget(m_SendMinQ, row,1);
		row++;
		shipG->setLayout(lay);
	}

	QGroupBox* getG = new QGroupBox(tr("Receive in Trade"), this);
	{
		QVBoxLayout* tlay = new QVBoxLayout;
		tlay->addWidget(lbgetteu);
		tlay->addWidget(m_GetTEUs);
		tlay->setMargin(0);
		tlay->setSpacing(2);

		QGridLayout* lay = new QGridLayout;
		int row=0;
		lay->addWidget(m_GetCommodityRadio, row,0);
		lay->addWidget(m_GetCommodity, row,1, 1,2);
		lay->addLayout(tlay, row,3, 3,1);
		row++;
		lay->addWidget(m_GetCategoryRadio, row,0);
		lay->addWidget(m_GetCategory, row,1, 1,2);
		row++;
		lay->addWidget(lbgetamt, row,0);
		lay->addWidget(m_GetAmount, row,1, 1,2);
		row++;
		lay->addWidget(lbgetminQ, row,0);
		lay->addWidget(m_GetMinQ, row,1);
		row++;
		getG->setLayout(lay);
	}

	QVBoxLayout* cklay = new QVBoxLayout;
	cklay->addWidget(m_EmpireCities);
	cklay->addWidget(m_FriendCities);
	cklay->addWidget(m_NeutralCities);
	cklay->addStretch();
	cklay->setMargin(0);
	cklay->setSpacing(2);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbreq);
	blay->addWidget(m_Ok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(shipG, row,0, 1,4);
	row++;
	lay->addWidget(getG, row,0, 1,4);
	row++;
	lay->addWidget(lbto, row,0);
	lay->addWidget(m_Cities, row,1, 1,2);
	lay->addLayout(cklay, row,3, 1,1);
	row++;
	lay->addWidget(m_ContinuousCheck, row,1, 1,2);
	row++;
	lay->addWidget(lbrepeat, row,0);
	lay->addWidget(m_Repeat, row,1);
	lay->addWidget(lbtimes, row,2);
	row++;
	lay->addLayout(blay, row,1, 1,3);
	row++;
	lay->setColumnStretch(1, 2);
	lay->setColumnStretch(2, 1);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuTradeAgreementCreate::AuTradeAgreementCreate(const AuTradeAgreement& proposal, const AuId& shipperId, quint32 shipperEmpire,
		const AuId& shipperSystemId, GALAXY shipperGalaxy, const AuPoint& shipperSystemLoc, QWidget* parent) : AuDialog(parent)
{
	m_ShipperEmpire = shipperEmpire;
	m_ShipperGalaxy = shipperGalaxy;
	m_ShipperId = shipperId;
	m_ShipperSystemId = shipperSystemId;
	m_ShipperSystemLoc = shipperSystemLoc;

	// Settings are acquired by reversing the sense of the origin and destination in the proposal.
	m_DestFilter = proposal.SendCommodityFilter();
	m_OrigFilter = proposal.GetCommodityFilter();
	m_DestCID = proposal.SendCommodityId();
	m_OrigCID = proposal.GetCommodityId();

	m_DestCity = proposal.OriginCity();

	bool empireCities = g_Au.Settings().readBoolEntry("Widget/TradeAgreementCreate/EmpireCities", true);
	bool friendCities = g_Au.Settings().readBoolEntry("Widget/TradeAgreementCreate/FriendCities", false);
	bool neutralCities = g_Au.Settings().readBoolEntry("Widget/TradeAgreementCreate/NeutralCities", false);

	bool continuous;
	quint32 repeat = proposal.RepeatCount();
	if(repeat < 1)
	{
		continuous = true;
		repeat = 1;
	}
	else
		continuous = false;

	quint32 getamt = proposal.SendAmount();
	quint8 getminQ = proposal.SendMinQ();
	if(getminQ < 1)
		getminQ = 1;
	int getwhat = (m_DestCID != CID_Invalid) ? 0 : 1;

	quint32 sendamt = proposal.GetAmount();
	quint8 sendminQ = proposal.GetMinQ();
	if(sendminQ < 1)
		sendminQ = 1;
	int sendwhat = (m_OrigCID != CID_Invalid) ? 0 : 1;

	setAuDlgTitle(tr("Propose Trade"));
	setAuDlgIcon(QIcon(":data/CommTrade.png"));

	QLabel* lbto = new QLabel(tr("Destination"), this);
	lbto->setAlignment(Qt::AlignRight|Qt::AlignTop);

	m_Cities = new AuCityTree(this);
	m_Cities->setToolTip(tr("<html>"
				"Destination broker city. "
				"<br>"
				"<b>Cities in your current solar system are not listed.</b> "
				"</html>"));
	connect(m_Cities, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
			SLOT(CityChanged(QTreeWidgetItem*, QTreeWidgetItem*)));

	m_EmpireCities = new QCheckBox(tr("Empire Cities"), this);
	m_EmpireCities->setToolTip(tr("<html>"
				"When checked, empire cities are listed. "
				"This is based on the stance of the receiving empire toward your empire. "
				"</html>"));
	m_EmpireCities->setChecked(empireCities);
	connect(m_EmpireCities, SIGNAL(stateChanged(int)), SLOT(EmpireChanged(int)));

	m_FriendCities = new QCheckBox(tr("Friend Cities"), this);
	m_FriendCities->setToolTip(tr("<html>"
				"When checked, friendly cities are listed. "
				"This is based on the stance of the receiving empire toward your empire. "
				"</html>"));
	m_FriendCities->setChecked(friendCities);
	connect(m_FriendCities, SIGNAL(stateChanged(int)), SLOT(FriendChanged(int)));

	m_NeutralCities = new QCheckBox(tr("Neutral Cities"), this);
	m_NeutralCities->setToolTip(tr("<html>"
				"When checked, neutral cities are listed <u>if they are in the same solar system as an empire city</u>. "
				"This is based on the stance of the receiving empire toward your empire. "
				"</html>"));
	m_NeutralCities->setChecked(neutralCities);
	connect(m_NeutralCities, SIGNAL(stateChanged(int)), SLOT(NeutralChanged(int)));

	m_Ok = new QPushButton(tr("Propose Trade"), this);
	m_Ok->setToolTip(tr("<html>"
				"Sends the trade proposal to the destination broker city. "
				"Someone at the destination broker must accept the proposal before trading will proceed. "
				"</html>"));
	connect(m_Ok, SIGNAL(clicked()), SLOT(accept()));	// Avoid ShipIt so we don't save these settings.

	m_ContinuousCheck = new QCheckBox(tr("Continuous"), this);
	m_ContinuousCheck->setToolTip(tr("<html>"
				"When checked, the order is filled as often as possible, endlessly. "
				"</html>"));
	m_ContinuousCheck->setChecked(continuous);
	connect(m_ContinuousCheck, SIGNAL(stateChanged(int)), SLOT(ContinuousChanged(int)));

	QLabel* lbtimes = new QLabel(tr("times"), this);

	QLabel* lbrepeat = new QLabel(tr("Fill Order"), this);
	lbrepeat->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_Repeat = new QSpinBox(this);
	m_Repeat->setRange(1,250);
	m_Repeat->setValue(repeat);
	m_Repeat->setToolTip(tr("<html>"
				"Total number of times to fill the order. "
				"</html>"));
	connect(m_Repeat, SIGNAL(valueChanged(int)), SLOT(RepeatChanged(int)));

	QPushButton* pbreq = new QPushButton(tr("Request Empires"));
	pbreq->setToolTip(tr("<html><b>Request Empires</b><br>"
				"Requests data about all empires. "
				"This may be needed to see cities that belong to friendly and neutral empires. "
				"</html>"));
	connect(pbreq, SIGNAL(clicked(bool)), SLOT(RequestEmpireData()));

	QLabel* lbgetamt = new QLabel(tr("Quantity"), this);
	lbgetamt->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	QLabel* lbsendamt = new QLabel(tr("Quantity"), this);
	lbsendamt->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_GetAmount = new QLineEdit(QString("%1").arg(getamt), this);
	m_GetAmount->setToolTip(tr("<html>"
				"Amount of commodities to ship. "
				"</html>"));
	connect(m_GetAmount, SIGNAL(textChanged(const QString&)), SLOT(GetAmountChanged(const QString&)));

	m_SendAmount = new QLineEdit(QString("%1").arg(sendamt), this);
	m_SendAmount->setToolTip(tr("<html>"
				"Amount of commodities to ship. "
				"</html>"));
	connect(m_SendAmount, SIGNAL(textChanged(const QString&)), SLOT(SendAmountChanged(const QString&)));

	m_GetCommodity = new QComboBox(this);
	m_GetCommodity->setToolTip(tr("<html>"
				"Commodity to ship. "
				"</html>"));
	connect(m_GetCommodity, SIGNAL(currentIndexChanged(int)), SLOT(GetCommodityChanged(int)));

	m_SendCommodity = new QComboBox(this);
	m_SendCommodity->setToolTip(tr("<html>"
				"Commodity to ship. "
				"</html>"));
	connect(m_SendCommodity, SIGNAL(currentIndexChanged(int)), SLOT(SendCommodityChanged(int)));

	m_GetCategory = new QComboBox(this);
	m_GetCategory->setToolTip(tr("<html>"
				"Category of commodities to ship. "
				"</html>"));
	connect(m_GetCategory, SIGNAL(currentIndexChanged(int)), SLOT(GetCategoryChanged(int)));

	m_SendCategory = new QComboBox(this);
	m_SendCategory->setToolTip(tr("<html>"
				"Category of commodities to ship. "
				"</html>"));
	connect(m_SendCategory, SIGNAL(currentIndexChanged(int)), SLOT(SendCategoryChanged(int)));

	m_SendCommodityRadio = new QRadioButton(tr("Commodity"), this);
	m_SendCommodityRadio->setChecked(sendwhat == 0);
	m_SendCommodityRadio->setToolTip(tr("<html>"
				"Fills the order with a specific commodity. "
				"</html>"));

	m_SendCategoryRadio = new QRadioButton(tr("Category"), this);
	m_SendCategoryRadio->setChecked(sendwhat == 1);
	m_SendCategoryRadio->setToolTip(tr("<html>"
				"Fills the order with a category of commodities. "
				"</html>"));
	connect(m_SendCommodityRadio, SIGNAL(toggled(bool)), SLOT(SendRadioToggled(bool)));
	connect(m_SendCategoryRadio, SIGNAL(toggled(bool)), SLOT(SendRadioToggled(bool)));

	QButtonGroup* sbg = new QButtonGroup(this);
	sbg->addButton(m_SendCommodityRadio);
	sbg->addButton(m_SendCategoryRadio);

	m_GetCommodityRadio = new QRadioButton(tr("Commodity"), this);
	m_GetCommodityRadio->setChecked(getwhat == 0);
	m_GetCommodityRadio->setToolTip(tr("<html>"
				"Fills the order with a specific commodity. "
				"</html>"));

	m_GetCategoryRadio = new QRadioButton(tr("Category"), this);
	m_GetCategoryRadio->setChecked(getwhat == 1);
	m_GetCategoryRadio->setToolTip(tr("<html>"
				"Fills the order with a category of commodities. "
				"</html>"));
	connect(m_GetCommodityRadio, SIGNAL(toggled(bool)), SLOT(GetRadioToggled(bool)));
	connect(m_GetCategoryRadio, SIGNAL(toggled(bool)), SLOT(GetRadioToggled(bool)));

	QButtonGroup* gbg = new QButtonGroup(this);
	gbg->addButton(m_GetCommodityRadio);
	gbg->addButton(m_GetCategoryRadio);

	QLabel* lbgetminQ = new QLabel(tr("Minimum Quality"), this);
	lbgetminQ->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	QLabel* lbsendminQ = new QLabel(tr("Minimum Quality"), this);
	lbsendminQ->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_GetMinQ = new QComboBox(this);
	for(int q=1; q<=255; q++)
		m_GetMinQ->addItem(QString("Q%1").arg(q), q);
	m_GetMinQ->setToolTip(tr("<html>"
				"Minimum quality commodities to ship. "
				"</html>"));
	m_GetMinQ->setCurrentIndex(m_GetMinQ->findData(getminQ));
	connect(m_GetMinQ, SIGNAL(currentIndexChanged(int)), SLOT(GetMinQChanged(int)));

	m_SendMinQ = new QComboBox(this);
	for(int q=1; q<=255; q++)
		m_SendMinQ->addItem(QString("Q%1").arg(q), q);
	m_SendMinQ->setToolTip(tr("<html>"
				"Minimum quality commodities to ship. "
				"</html>"));
	m_SendMinQ->setCurrentIndex(m_SendMinQ->findData(sendminQ));
	connect(m_SendMinQ, SIGNAL(currentIndexChanged(int)), SLOT(SendMinQChanged(int)));

	QLabel* lbgetteu = new QLabel(this);
	lbgetteu->setAlignment(Qt::AlignHCenter|Qt::AlignBottom);
	lbgetteu->setPixmap(QPixmap(":data/c_Container.png"));
	lbgetteu->setToolTip(tr("<html>"
				"Cargo is shipped in Twenty-foot Equivalent Unit containers. "
				"Each TEU can hold %L1 units of cargo. "
				"Delivery fee is based on the number of TEU containers. "
				"</html>").arg(1360));

	QLabel* lbsendteu = new QLabel(this);
	lbsendteu->setAlignment(Qt::AlignHCenter|Qt::AlignBottom);
	lbsendteu->setPixmap(QPixmap(":data/c_Container.png"));
	lbsendteu->setToolTip(tr("<html>"
				"Cargo is shipped in Twenty-foot Equivalent Unit containers. "
				"Each TEU can hold %L1 units of cargo. "
				"Delivery fee is based on the number of TEU containers. "
				"</html>").arg(1360));

	quint32 getteus = AuItemTEUContainer::TEUCount(getamt);
	m_GetTEUs = new QLabel(tr("%L1 TEUs").arg(getteus), this);
	m_GetTEUs->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
	m_GetTEUs->setToolTip(tr("<html>"
				"Cargo is shipped in Twenty-foot Equivalent Unit containers. "
				"Each TEU can hold %L1 units of cargo. "
				"Delivery fee is based on the number of TEU containers. "
				"</html>").arg(1360));

	quint32 sendteus = AuItemTEUContainer::TEUCount(sendamt);
	m_SendTEUs = new QLabel(tr("%L1 TEUs").arg(sendteus), this);
	m_SendTEUs->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
	m_SendTEUs->setToolTip(tr("<html>"
				"Cargo is shipped in Twenty-foot Equivalent Unit containers. "
				"Each TEU can hold %L1 units of cargo. "
				"Delivery fee is based on the number of TEU containers. "
				"</html>").arg(1360));

	QGroupBox* shipG = new QGroupBox(tr("Offer to Send"), this);
	{
		QVBoxLayout* tlay = new QVBoxLayout;
		tlay->addWidget(lbsendteu);
		tlay->addWidget(m_SendTEUs);
		tlay->setMargin(0);
		tlay->setSpacing(2);

		QGridLayout* lay = new QGridLayout;
		int row=0;
		lay->addWidget(m_SendCommodityRadio, row,0);
		lay->addWidget(m_SendCommodity, row,1, 1,2);
		lay->addLayout(tlay, row,3, 3,1);
		row++;
		lay->addWidget(m_SendCategoryRadio, row,0);
		lay->addWidget(m_SendCategory, row,1, 1,2);
		row++;
		lay->addWidget(lbsendamt, row,0);
		lay->addWidget(m_SendAmount, row,1, 1,2);
		row++;
		lay->addWidget(lbsendminQ, row,0);
		lay->addWidget(m_SendMinQ, row,1);
		row++;
		shipG->setLayout(lay);
	}

	QGroupBox* getG = new QGroupBox(tr("Receive in Trade"), this);
	{
		QVBoxLayout* tlay = new QVBoxLayout;
		tlay->addWidget(lbgetteu);
		tlay->addWidget(m_GetTEUs);
		tlay->setMargin(0);
		tlay->setSpacing(2);

		QGridLayout* lay = new QGridLayout;
		int row=0;
		lay->addWidget(m_GetCommodityRadio, row,0);
		lay->addWidget(m_GetCommodity, row,1, 1,2);
		lay->addLayout(tlay, row,3, 3,1);
		row++;
		lay->addWidget(m_GetCategoryRadio, row,0);
		lay->addWidget(m_GetCategory, row,1, 1,2);
		row++;
		lay->addWidget(lbgetamt, row,0);
		lay->addWidget(m_GetAmount, row,1, 1,2);
		row++;
		lay->addWidget(lbgetminQ, row,0);
		lay->addWidget(m_GetMinQ, row,1);
		row++;
		getG->setLayout(lay);
	}

	QVBoxLayout* cklay = new QVBoxLayout;
	cklay->addWidget(m_EmpireCities);
	cklay->addWidget(m_FriendCities);
	cklay->addWidget(m_NeutralCities);
	cklay->addStretch();
	cklay->setMargin(0);
	cklay->setSpacing(2);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbreq);
	blay->addWidget(m_Ok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(shipG, row,0, 1,4);
	row++;
	lay->addWidget(getG, row,0, 1,4);
	row++;
	lay->addWidget(lbto, row,0);
	lay->addWidget(m_Cities, row,1, 1,2);
	lay->addLayout(cklay, row,3, 1,1);
	row++;
	lay->addWidget(m_ContinuousCheck, row,1, 1,2);
	row++;
	lay->addWidget(lbrepeat, row,0);
	lay->addWidget(m_Repeat, row,1);
	lay->addWidget(lbtimes, row,2);
	row++;
	lay->addLayout(blay, row,1, 1,3);
	row++;
	lay->setColumnStretch(1, 2);
	lay->setColumnStretch(2, 1);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuTradeAgreementCreate::~AuTradeAgreementCreate()
{
}

void AuTradeAgreementCreate::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
		ListCities();
		ListCategories(m_GetCategory, m_DestFilter);
		ListCategories(m_SendCategory, m_OrigFilter);
		ListCommodities(m_GetCommodity, m_DestCID);
		ListCommodities(m_SendCommodity, m_OrigCID);
		EnableControls();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuTradeAgreementCreate::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/TradeAgreementCreate/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/TradeAgreementCreate/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/TradeAgreementCreate/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/TradeAgreementCreate/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuTradeAgreementCreate::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/SizeY",sz.height());
}

void AuTradeAgreementCreate::ListCategories(QComboBox* cb, COMMODITYFILTER filter)
{
	cb->blockSignals(true);
	cb->clear();

	QMap<QString, COMMODITYFILTER> sortedNames;
	for(int cat = FILTER_All; cat <= FILTER_Count; cat++)
	{
		COMMODITYFILTER filter = (COMMODITYFILTER)cat;
		if(!DbCommodity::IsFilterValid(filter))
			continue;
		sortedNames.insert(CommodityFilterName(filter), filter);
	}

	for(QMap<QString, COMMODITYFILTER>::const_iterator si = sortedNames.cbegin(); si != sortedNames.cend(); si++)
		cb->addItem(QIcon(CommodityFilterIcon(*si)), si.key(), *si);

	cb->setCurrentIndex(cb->findData(filter));
	cb->blockSignals(false);
}

void AuTradeAgreementCreate::ListCommodities(QComboBox* cb, COMMODITYID cid)
{
	cb->blockSignals(true);
	cb->clear();

	// Get a list of the commodities that fit into the current filter.
	QList<COMMODITYID> cidFilter;
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_Ammo);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_ArmorAndClothes);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_ConstructionMaterials);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_Food);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_Fuel);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_MedicalSupplies);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_Misc);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_SpareParts);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_Tools);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_Weapons);
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, FILTER_NaturalResources);

	QMap<QString, COMMODITYID> sortedNames;
	for(QList<COMMODITYID>::const_iterator fi = cidFilter.constBegin(); fi != cidFilter.constEnd(); fi++)
	{
		// The commodity array is intended to be indexable by the commodity id.
		const DbCommodity& commodity = g_Au.Commodity(*fi);
		if(!commodity.CanBeCargo())
			continue;	// Don't list things that can't be carried.
		sortedNames.insert(commodity.m_name.toUpper(), *fi);
	}

	// List commodities according to the current filter.
	for(QMap<QString, COMMODITYID>::const_iterator si = sortedNames.cbegin(); si != sortedNames.cend(); si++)
	{
		// The commodity array is intended to be indexable by the commodity id.
		const DbCommodity& commodity = g_Au.Commodity(*si);
		QPixmap px32 = QPixmap(commodity.BitmapName()).scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		QIcon icon(px32);

		cb->addItem(icon, commodity.m_name, commodity.m_id);
	}

	cb->setCurrentIndex(cb->findData(cid));
	cb->blockSignals(false);
}

void AuTradeAgreementCreate::ListCities()
{
	m_Cities->blockSignals(true);
	QMap<QString, AuEmpireCity> cities;
	const DgData& gd = g_Au.LockDataForRead();
	DgEmpireHash::const_iterator sei = gd.m_gEmpires.find(m_ShipperEmpire);
	if(sei != gd.m_gEmpires.cend())
	{
		// Shipper empire found.
		for(DgEmpireHash::const_iterator ei = gd.m_gEmpires.cbegin(); ei != gd.m_gEmpires.cend(); ei++)
		{
			// The potential recipient's stance toward my empire.
			POLITICAL_STANCE stance = (*ei).second.PoliticalStance(m_ShipperEmpire);
			bool ckInSystem = false;
			switch(stance)
			{
			case POL_Friend:
			case POL_Vassal:
				if(!m_FriendCities->isChecked())
					continue;
				break;

			case POL_Neutral:
				if(!m_NeutralCities->isChecked())
					continue;
				ckInSystem = true;
				break;

			case POL_Enemy:
				continue;			// Don't list enemy cities at all.

			case POL_Empire:
				if(!m_EmpireCities->isChecked())
					continue;
				break;
			}

			// List the empire's cities.
			for(AuCityHash::const_iterator ci = (*ei).second.m_Cities->cbegin(); ci != (*ei).second.m_Cities->cend(); ci++)
			{
				// Don't list cities in the same system as the shipper.
				if((*ci).second.SystemId() == m_ShipperSystemId)
					continue;
				if(!(*ci).second.Facilities().contains(IID_Broker))
					continue;		// Must have a broker at the destination.

				// With neutrals, we only want cities that exist in systems where shipper has cities.
				if(ckInSystem && !(*sei).second.HasCityInSystem((*ci).second.SystemId()))
					continue;

				// Calculate delivery distance in pc.
				float distPc;
				if((*ci).second.Galaxy() == m_ShipperGalaxy)
					distPc = m_ShipperSystemLoc.Distance((*ci).second.SystemLoc());				// Direct distance.
				else
					distPc = m_ShipperSystemLoc.Length() + (*ci).second.SystemLoc().Length();	// To from center of G.

				// Make a description that sounds kind of like a delivery address.
				QString desc = tr("%1 on %2, %3 system, %4 sector, %5")
					.arg((*ci).second.CityName())
					.arg((*ci).second.PlanetName())
					.arg((*ci).second.SystemName())
					.arg((*ci).second.SectorName())
					.arg((*ci).second.GalaxyName());

				cities.insert(desc, AuEmpireCity((*ci).second.CityType(), (*ei).second.m_id, distPc, (*ci).second));
			}
		}
	}
	g_Au.UnlockData();

	m_Cities->clear();
	QList<QTreeWidgetItem*> expand;
	QTreeWidgetItem* curCity = 0;
	QHash<AuId, QTreeWidgetItem*> atomItems;
	for(QMap<QString, AuEmpireCity>::const_iterator ci = cities.cbegin(); ci != cities.cend(); ci++)
	{
		// Top level is galaxy.
		QTreeWidgetItem* galaxy;
		QHash<AuId, QTreeWidgetItem*>::const_iterator gali = atomItems.find((*ci).GalaxyId());
		if(gali == atomItems.cend())
		{
			// Create the galaxy item.
			galaxy = new QTreeWidgetItem(m_Cities);
			galaxy->setIcon(0, QIcon(":data/RangeGalaxy.png"));
			galaxy->setText(0, (*ci).GalaxyName());
			galaxy->setData(0, Qt::UserRole, (*ci).GalaxyId().GetID());
			atomItems.insert((*ci).GalaxyId(), galaxy);
		}
		else
			galaxy = *gali;

		// Next level is sector.
		QTreeWidgetItem* sector;
		QHash<AuId, QTreeWidgetItem*>::const_iterator seci = atomItems.find((*ci).SectorId());
		if(seci == atomItems.cend())
		{
			// Create the sector item.
			sector = new QTreeWidgetItem(galaxy);
			sector->setIcon(0, QIcon(":data/RangeSector.png"));
			sector->setText(0, (*ci).SectorName());
			sector->setData(0, Qt::UserRole, (*ci).SectorId().GetID());
			if((*ci).CityId() == m_DestCity)
				sector->setExpanded(true);
			atomItems.insert((*ci).SectorId(), sector);
		}
		else
			sector = *seci;

		// Next level is solar system.
		QTreeWidgetItem* system;
		QHash<AuId, QTreeWidgetItem*>::const_iterator sysi = atomItems.find((*ci).SystemId());
		if(sysi == atomItems.cend())
		{
			// Create the system item.
			system = new QTreeWidgetItem(sector);
			system->setIcon(0, QIcon(":data/RangeSystem.png"));
			system->setText(0, (*ci).SystemName());
			system->setData(0, Qt::UserRole, (*ci).SystemId().GetID());
			if((*ci).CityId() == m_DestCity)
				system->setExpanded(true);
			sector->sortChildren(0, Qt::AscendingOrder);
			atomItems.insert((*ci).SystemId(), system);
		}
		else
			system = *sysi;

		/*
		// Next level is world.
		QTreeWidgetItem* world;
		QHash<AuId, QTreeWidgetItem*>::const_iterator wldi = atomItems.find((*ci).PlanetId());
		if(wldi == atomItems.cend())
		{
			// Create the world item.
			world = new QTreeWidgetItem(system);
			world->setIcon(0, QIcon(":data/RangeHail.png"));
			world->setText(0, (*ci).PlanetName());
			world->setData(0, Qt::UserRole, (*ci).PlanetId().GetID());
			world->setExpanded(true);
			system->sortChildren(0, Qt::AscendingOrder);
			atomItems.insert((*ci).PlanetId(), world);
		}
		else
			world = *wldi;
			*/

		// Create the city item.
		QIcon icon;
		const DgData& gd = g_Au.LockDataForRead();
		quint32 cityEmpire = gd.CityEmpire((*ci).CityId());
		DgEmpireHash::const_iterator ei = gd.m_gEmpires.find(cityEmpire);
		if(ei != gd.m_gEmpires.cend())
			icon = QIcon(QPixmap::fromImage((*ei).second.m_flag));
		else
			icon = QIcon(AuCity::CityIcon((*ci).m_Type));
		g_Au.UnlockData();

		QTreeWidgetItem* city = new QTreeWidgetItem(system);
		city->setIcon(0, icon);
		city->setText(0, (*ci).CityName());
		city->setData(0, Qt::UserRole, (*ci).CityId().GetID());
		city->setData(0, Qt::UserRole+1, (*ci).CityId().GetID());	// Only city has city id in role+1
		city->setData(0, Qt::UserRole+2, (*ci).m_DistPc);
		if((*ci).CityId() == m_DestCity)
			city->setSelected(true);
		system->sortChildren(0, Qt::AscendingOrder);

		if((*ci).CityId() == m_DestCity)
		{
			curCity = city;
			expand.append(galaxy);
			expand.append(sector);
			expand.append(system);
		}
	}
	m_Cities->sortItems(0, Qt::AscendingOrder);
	for(QList<QTreeWidgetItem*>::iterator ei = expand.begin(); ei != expand.end(); ei++)
		(*ei)->setExpanded(true);
	if(curCity)
		m_Cities->setCurrentItem(curCity);
	m_Cities->blockSignals(false);
}

quint32 AuTradeAgreementCreate::GetAmount() const
{
	return m_GetAmount->text().toUInt();
}

COMMODITYFILTER AuTradeAgreementCreate::GetCategory() const
{
	return (COMMODITYFILTER)m_GetCategory->itemData(m_GetCategory->currentIndex()).toInt();
}

COMMODITYID AuTradeAgreementCreate::GetCommodityId() const
{
	return (COMMODITYID)m_GetCommodity->itemData(m_GetCommodity->currentIndex()).toInt();
}

int AuTradeAgreementCreate::GetWhat() const
{
	if(m_GetCommodityRadio->isChecked())
		return 0;
	return 1;
}

quint8 AuTradeAgreementCreate::GetMinQ() const
{
	return m_GetMinQ->itemData(m_GetMinQ->currentIndex()).toUInt();
}

quint32 AuTradeAgreementCreate::SendAmount() const
{
	return m_SendAmount->text().toUInt();
}

COMMODITYFILTER AuTradeAgreementCreate::SendCategory() const
{
	return (COMMODITYFILTER)m_SendCategory->itemData(m_SendCategory->currentIndex()).toInt();
}

COMMODITYID AuTradeAgreementCreate::SendCommodityId() const
{
	return (COMMODITYID)m_SendCommodity->itemData(m_SendCommodity->currentIndex()).toInt();
}

int AuTradeAgreementCreate::SendWhat() const
{
	if(m_SendCommodityRadio->isChecked())
		return 0;
	return 1;
}

quint8 AuTradeAgreementCreate::SendMinQ() const
{
	return m_SendMinQ->itemData(m_SendMinQ->currentIndex()).toUInt();
}

quint8 AuTradeAgreementCreate::Repeat() const
{
	if(m_ContinuousCheck->isChecked())
		return 0;
	else
		return m_Repeat->value();
}

AuId AuTradeAgreementCreate::DestCityId() const
{
	QTreeWidgetItem* city = m_Cities->currentItem();
	if(!city)
		return AuId(0);

	AuId cityId(city->data(0, Qt::UserRole+1).toUInt());
	return cityId;
}

QString AuTradeAgreementCreate::DestDesc() const
{
	AuId cityId(DestCityId());
	if(cityId.IsNull())
		return QString();

	// Get the info from the city record.
	QString desc;
	const DgData& gd = g_Au.LockDataForRead();
	quint32 cityEmpire = gd.CityEmpire(cityId);
	DgEmpireHash::const_iterator ei = gd.m_gEmpires.find(cityEmpire);
	if(ei != gd.m_gEmpires.cend())
	{
		AuCityHash::const_iterator ci = (*ei).second.m_Cities->find(cityId);
		if(ci != (*ei).second.m_Cities->cend())
		{
			desc = tr("%1 on %2, %3 system, %4 sector, %5")
				.arg((*ci).second.CityName())
				.arg((*ci).second.PlanetName())
				.arg((*ci).second.SystemName())
				.arg((*ci).second.SectorName())
				.arg((*ci).second.GalaxyName());
		}
	}
	g_Au.UnlockData();

	return desc;
}

float AuTradeAgreementCreate::DestDistPc() const
{
	QTreeWidgetItem* city = m_Cities->currentItem();
	if(!city)
		return 0;

	AuId cityId(city->data(0, Qt::UserRole+1).toUInt());
	if(cityId.IsNull())
		return 0;	// Not a city item.

	return city->data(0, Qt::UserRole+2).toFloat();
}

AuId AuTradeAgreementCreate::DestSectorId() const
{
	AuId cityId(DestCityId());
	if(cityId.IsNull())
		return AuId(0);

	// Get the info from the city record.
	AuId id;
	const DgData& gd = g_Au.LockDataForRead();
	quint32 cityEmpire = gd.CityEmpire(cityId);
	DgEmpireHash::const_iterator ei = gd.m_gEmpires.find(cityEmpire);
	if(ei != gd.m_gEmpires.cend())
	{
		AuCityHash::const_iterator ci = (*ei).second.m_Cities->find(cityId);
		if(ci != (*ei).second.m_Cities->cend())
			id = (*ci).second.SectorId();
	}
	g_Au.UnlockData();

	return id;
}

AuId AuTradeAgreementCreate::DestSystemId() const
{
	AuId cityId(DestCityId());
	if(cityId.IsNull())
		return AuId(0);

	// Get the info from the city record.
	AuId id;
	const DgData& gd = g_Au.LockDataForRead();
	quint32 cityEmpire = gd.CityEmpire(cityId);
	DgEmpireHash::const_iterator ei = gd.m_gEmpires.find(cityEmpire);
	if(ei != gd.m_gEmpires.cend())
	{
		AuCityHash::const_iterator ci = (*ei).second.m_Cities->find(cityId);
		if(ci != (*ei).second.m_Cities->cend())
			id = (*ci).second.SystemId();
	}
	g_Au.UnlockData();

	return id;
}

void AuTradeAgreementCreate::EnableControls()
{
	m_GetCommodity->setEnabled(m_GetCommodityRadio->isChecked());
	m_SendCommodity->setEnabled(m_SendCommodityRadio->isChecked());

	m_GetCategory->setEnabled(m_GetCategoryRadio->isChecked());
	m_SendCategory->setEnabled(m_SendCategoryRadio->isChecked());

	bool enableOk = true;
	if(GetCommodityId() == CID_Invalid)
		enableOk = false;
	if(SendCommodityId() == CID_Invalid)
		enableOk = false;

	if(!GetAmount())
		enableOk = false;
	if(!SendAmount())
		enableOk = false;

	AuId cityId(DestCityId());
	if(cityId.IsNull())
		enableOk = false;

	m_Ok->setEnabled(enableOk);

	bool enableRepeat = true;
	if(m_ContinuousCheck->isChecked())
		enableRepeat = false;

	m_Repeat->setEnabled(enableRepeat);
}

void AuTradeAgreementCreate::CityChanged(QTreeWidgetItem*, QTreeWidgetItem*)
{
	EnableControls();
}

void AuTradeAgreementCreate::GetAmountChanged(const QString& text)
{
	quint32 amt = text.toUInt();
	quint32 teus = AuItemTEUContainer::TEUCount(amt);
	m_GetTEUs->setText(tr("%L1 TEUs").arg(teus));
	EnableControls();
}

void AuTradeAgreementCreate::GetCategoryChanged(int)
{
	EnableControls();
}

void AuTradeAgreementCreate::GetCommodityChanged(int)
{
	EnableControls();
}

void AuTradeAgreementCreate::GetMinQChanged(int)
{
	EnableControls();
}

void AuTradeAgreementCreate::GetRadioToggled(bool checked)
{
	Q_UNUSED(checked);
	EnableControls();
}

void AuTradeAgreementCreate::SendAmountChanged(const QString& text)
{
	quint32 amt = text.toUInt();
	quint32 teus = AuItemTEUContainer::TEUCount(amt);
	m_SendTEUs->setText(tr("%L1 TEUs").arg(teus));
	EnableControls();
}

void AuTradeAgreementCreate::SendCategoryChanged(int)
{
	EnableControls();
}

void AuTradeAgreementCreate::SendCommodityChanged(int)
{
	EnableControls();
}

void AuTradeAgreementCreate::SendMinQChanged(int)
{
	EnableControls();
}

void AuTradeAgreementCreate::SendRadioToggled(bool checked)
{
	Q_UNUSED(checked);
	EnableControls();
}

void AuTradeAgreementCreate::ContinuousChanged(int)
{
	EnableControls();
}

void AuTradeAgreementCreate::EmpireChanged(int)
{
	ListCities();
	EnableControls();
}

void AuTradeAgreementCreate::FriendChanged(int)
{
	ListCities();
	EnableControls();
}

void AuTradeAgreementCreate::NeutralChanged(int)
{
	ListCities();
	EnableControls();
}

void AuTradeAgreementCreate::RepeatChanged(int)
{
	EnableControls();
}

void AuTradeAgreementCreate::RequestEmpireData()
{
	auApp->RequestEmpiresAll();
	QMessageBox::information(this, tr("Empire Data Requested"),
			tr("Information about all empires was requested. It may take a\n"
				"minute or two to receive the information."));
	reject();
}

void AuTradeAgreementCreate::ShipIt()
{
	AuId cityId(DestCityId());
	if(cityId.IsNull())
		g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/DestCity", 0);
	else
		g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/DestCity", cityId.StringId());

	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/EmpireCities", m_EmpireCities->isChecked());
	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/FriendCities", m_FriendCities->isChecked());
	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/NeutralCities", m_NeutralCities->isChecked());
	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/Continuous", m_ContinuousCheck->isChecked());
	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/Repeat", (int)Repeat());

	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/GetCommodity", GetCommodityId());
	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/GetCategory", GetCategory());
	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/GetQuantity", GetAmount());
	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/GetMinQ", GetMinQ());

	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/SendCommodity", SendCommodityId());
	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/SendCategory", SendCategory());
	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/SendQuantity", SendAmount());
	g_Au.Settings().writeEntry("Widget/TradeAgreementCreate/SendMinQ", SendMinQ());

	accept();
}

