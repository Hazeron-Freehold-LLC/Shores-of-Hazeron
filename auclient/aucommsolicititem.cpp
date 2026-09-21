//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommsolicititem.cpp,v 1.8 2022/02/19 18:10:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommsolicititem.h"
//
#include "aucommodityfilter.h"
#include <auglobal.h>
//#include <dgdata.h>
#include <QGridLayout>
#include <QIcon>
#include <QListWidget>
#include <QPushButton>

AuCommSolicitItem::AuCommSolicitItem(COMMODITYID defCommodityId, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle("Ask for Sales");
	setAuDlgIcon(QIcon(":/data/MsgSolicitSales.png"));
	m_lCID = defCommodityId;	// Init unitl Ok pressed with valid selection.
	m_SolicitType = Commodity;

	QListWidget* lw = new QListWidget(this);
	lw->setEditTriggers(QAbstractItemView::NoEditTriggers);
	lw->setObjectName("lwcommodities");
	lw->setSelectionBehavior(QAbstractItemView::SelectRows);
	lw->setSelectionMode(QAbstractItemView::SingleSelection);
	lw->setSortingEnabled(true);
	connect(lw, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(OkPressed()));
	connect(lw, SIGNAL(itemSelectionChanged()), SLOT(EnableControls()));

	m_Filter = FILTER_All;
	AuCommodityFilter* cfilt = new AuCommodityFilter;
	cfilt->setObjectName("cfilt");
	connect(cfilt, SIGNAL(FilterChanged(COMMODITYFILTER)), SLOT(FilterCommodities(COMMODITYFILTER)));

	QPushButton* pbok = new QPushButton(tr("Ask For Item"));
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setObjectName("pbok");
	pbok->setToolTip(tr("Ask for sales of the selected item."));
	connect(pbok, SIGNAL(clicked()), SLOT(OkPressed()));

	QPushButton* pball = new QPushButton(tr("All Items"));
	pball->setIcon(QIcon(CommodityFilterIcon(FILTER_All)));
	pball->setToolTip(tr("Ask for sales of all items."));
	connect(pball, SIGNAL(clicked()), SLOT(AllPressed()));

	QPushButton* pbammo = new QPushButton(tr("Ammunition"));
	pbammo->setIcon(QIcon(CommodityFilterIcon(FILTER_Ammo)));
	pbammo->setToolTip(tr("Ask for sales of ammunition."));
	connect(pbammo, SIGNAL(clicked()), SLOT(AmmoPressed()));

	QPushButton* pbarmor = new QPushButton(tr("Armor"));
	pbarmor->setIcon(QIcon(CommodityFilterIcon(FILTER_ArmorAndClothes)));
	pbarmor->setToolTip(tr("Ask for sales of armor and clothing."));
	connect(pbarmor, SIGNAL(clicked()), SLOT(ArmorPressed()));

	QPushButton* pbconst = new QPushButton(tr("Construction Material"));
	pbconst->setIcon(QIcon(CommodityFilterIcon(FILTER_ConstructionMaterials)));
	pbconst->setToolTip(tr("Ask for sales of construction material."));
	connect(pbconst, SIGNAL(clicked()), SLOT(ConstructionMaterialPressed()));

	QPushButton* pbfood = new QPushButton(tr("Food"));
	pbfood->setIcon(QIcon(CommodityFilterIcon(FILTER_Food)));
	pbfood->setToolTip(tr("Ask for sales of food."));
	connect(pbfood, SIGNAL(clicked()), SLOT(FoodPressed()));

	QPushButton* pbfuel = new QPushButton(tr("Fuel"));
	pbfuel->setIcon(QIcon(CommodityFilterIcon(FILTER_Fuel)));
	pbfuel->setToolTip(tr("Ask for sales of fuel."));
	connect(pbfuel, SIGNAL(clicked()), SLOT(FuelPressed()));

	QPushButton* pbmed = new QPushButton(tr("Medical Supplies"));
	pbmed->setIcon(QIcon(CommodityFilterIcon(FILTER_MedicalSupplies)));
	pbmed->setToolTip(tr("Ask for sales of medical supplies."));
	connect(pbmed, SIGNAL(clicked()), SLOT(MedPressed()));

	QPushButton* pbparts = new QPushButton(tr("Spare Parts"));
	pbparts->setIcon(QIcon(CommodityFilterIcon(FILTER_SpareParts)));
	pbparts->setToolTip(tr("Ask for sales of spare parts."));
	connect(pbparts, SIGNAL(clicked()), SLOT(SparePartsPressed()));

	QPushButton* pbpatents = new QPushButton(tr("Patents"));
	pbpatents->setIcon(QIcon(CommodityFilterIcon(FILTER_Patents)));
	pbpatents->setToolTip(tr("Ask for sales of patents."));
	connect(pbpatents, SIGNAL(clicked()), SLOT(PatentsPressed()));

	QPushButton* pbtools = new QPushButton(tr("Tools"));
	pbtools->setIcon(QIcon(CommodityFilterIcon(FILTER_Tools)));
	pbtools->setToolTip(tr("Ask for sales of tools."));
	connect(pbtools, SIGNAL(clicked()), SLOT(ToolsPressed()));

	QPushButton* pbweapons = new QPushButton(tr("Weapons"));
	pbweapons->setIcon(QIcon(CommodityFilterIcon(FILTER_Weapons)));
	pbweapons->setToolTip(tr("Ask for sales of weapons."));
	connect(pbweapons, SIGNAL(clicked()), SLOT(WeaponsPressed()));

	QPushButton* pbnat = new QPushButton(tr("Natural Resources"));
	pbnat->setIcon(QIcon(CommodityFilterIcon(FILTER_NaturalResources)));
	pbnat->setToolTip(tr("Ask for sales of natural resources."));
	connect(pbnat, SIGNAL(clicked()), SLOT(NaturalResourcesPressed()));

	QPushButton* pbmisc = new QPushButton(tr("Miscellaneous"));
	pbmisc->setIcon(QIcon(CommodityFilterIcon(FILTER_Misc)));
	pbmisc->setToolTip(tr("Ask for sales of miscellaneous items."));
	connect(pbmisc, SIGNAL(clicked()), SLOT(MiscPressed()));

	QPushButton* pbshipments = new QPushButton(tr("Shipments"));
	pbshipments->setIcon(QIcon(":data/c_Container.png"));
	pbshipments->setToolTip(tr("<b>Ask for Shipments</b><br>"
				"Asks for shipments that are ready for pickup. "
				"You must be aboard a spacecraft. "
				"You must be an officer of the ship."));
	connect(pbshipments, SIGNAL(clicked(bool)), SLOT(ShipmentsPressed()));

	QGridLayout* lay = new QGridLayout();
	int row = 0;	// Using this makes inserting and rearranging rows easier.
	lay->addWidget(pball, row++,0);
	lay->setRowStretch(row, 1);
	row++;	// stretchy row
	lay->addWidget(pbfood, row++,0);
	lay->addWidget(pbfuel, row++,0);
	lay->addWidget(pbtools, row++,0);
	lay->addWidget(pbconst, row++,0);
	lay->addWidget(pbparts, row++,0);
	lay->addWidget(pbmed, row++,0);
	lay->addWidget(pbarmor, row++,0);
	lay->addWidget(pbweapons, row++,0);
	lay->addWidget(pbammo, row++,0);
	lay->addWidget(pbpatents, row++,0);
	lay->addWidget(pbnat, row++,0);
	lay->addWidget(pbmisc, row++,0);
	lay->addWidget(pbshipments, row++,0);
	lay->addWidget(cfilt, 0,1);
	lay->addWidget(lw, 1,1, row-2,1);
	lay->addWidget(pbok, row-1,1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	ListCommodities();

	EnableControls();
}

void AuCommSolicitItem::EnableControls()
{
	bool enableok = false;
	QListWidget* lw = findChild<QListWidget*>("lwcommodities");
	if(lw)
	{
		QItemSelectionModel* ism = lw->selectionModel();
		if(ism && !ism->selection().isEmpty())
			enableok = true;
	}

	QPushButton* pbok = findChild<QPushButton*>("pbok");
	if(pbok)
		pbok->setEnabled(enableok);
}

void AuCommSolicitItem::OkPressed()
{
	QListWidget* lw = findChild<QListWidget*>("lwcommodities");
	if(lw)
	{
		// Get selection values so they can be retrieved after the widget is
		// gone.
		QItemSelectionModel* ism = lw->selectionModel();
		if(ism)
		{
			QModelIndexList sel = ism->selectedIndexes();
			if(!sel.isEmpty())
			{
				QModelIndex idxsel = sel.first();
				if(idxsel.isValid())
				{
					QListWidgetItem* cur =
						(QListWidgetItem*)idxsel.internalPointer();
					m_lCID = (COMMODITYID)cur->data(Qt::UserRole).toInt();
					m_SolicitType = Commodity;
					accept();
				}
			}
		}
	}
}

void AuCommSolicitItem::AllPressed()
{
	m_SolicitType = All;
	accept();
}

void AuCommSolicitItem::AmmoPressed()
{
	m_SolicitType = Ammo;
	accept();
}

void AuCommSolicitItem::ArmorPressed()
{
	m_SolicitType = Armor;
	accept();
}

void AuCommSolicitItem::PatentsPressed()
{
	m_SolicitType = Patents;
	accept();
}

void AuCommSolicitItem::ConstructionMaterialPressed()
{
	m_SolicitType = ConstructionMaterial;
	accept();
}

void AuCommSolicitItem::FoodPressed()
{
	m_SolicitType = Food;
	accept();
}

void AuCommSolicitItem::FuelPressed()
{
	m_SolicitType = Fuel;
	accept();
}

void AuCommSolicitItem::MedPressed()
{
	m_SolicitType = Medical;
	accept();
}

void AuCommSolicitItem::MiscPressed()
{
	m_SolicitType = Misc;
	accept();
}

void AuCommSolicitItem::SparePartsPressed()
{
	m_SolicitType = SpareParts;
	accept();
}

void AuCommSolicitItem::ToolsPressed()
{
	m_SolicitType = Tools;
	accept();
}

void AuCommSolicitItem::WeaponsPressed()
{
	m_SolicitType = Weapons;
	accept();
}

void AuCommSolicitItem::NaturalResourcesPressed()
{
	m_SolicitType = NaturalResources;
	accept();
}

void AuCommSolicitItem::FilterCommodities(COMMODITYFILTER filter)
{
	m_Filter = filter;
	ListCommodities();
}

void AuCommSolicitItem::ListCommodities()
{
	QListWidget* lw = findChild<QListWidget*>("lwcommodities");
	if(!lw)
		return;
	lw->clear();

	// Get a list of the commodities that fit into the current filter.
	QList<COMMODITYID> cidFilter;
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, m_Filter);

	// List commodities according to the current filter.
	QListWidgetItem* lwidef=0;
	for(QList<COMMODITYID>::const_iterator fi = cidFilter.constBegin(); fi != cidFilter.constEnd(); fi++)
	{
		// The commodity array is intended to be indexable by the commodity id.
		const DbCommodity& commodity = g_Au.Commodity(*fi);
		if(!commodity.CanBeCargo())
			continue;	// Don't list things that can't be carried.

		QPixmap px32 = QPixmap(commodity.BitmapName()).scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		QIcon icon(px32);

		QListWidgetItem* lwi = new QListWidgetItem(icon, commodity.m_name, lw);
		lwi->setData(Qt::UserRole, commodity.m_id);
		if(commodity.m_id == m_lCID)
			lwidef = lwi;
	}
	if(lwidef)
	{
		lwidef->setSelected(true);
		lw->setCurrentItem(lwidef);
		lw->scrollToItem(lwidef);
	}
}

void AuCommSolicitItem::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SolicitItem/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SolicitItem/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SolicitItem/SizeX", szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SolicitItem/SizeY", szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);

	AuCommodityFilter* cfilt = findChild<AuCommodityFilter*>("cfilt");
	if(cfilt)
		cfilt->RestoreFilter("Widget/SolicitItem/FilterInventory");
}

void AuCommSolicitItem::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SolicitItem/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SolicitItem/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SolicitItem/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SolicitItem/SizeY",sz.height());

	AuCommodityFilter* cfilt = findChild<AuCommodityFilter*>("cfilt");
	if(cfilt)
		cfilt->SaveFilter("Widget/SolicitItem/FilterInventory");
}

void AuCommSolicitItem::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuCommSolicitItem::ShipmentsPressed()
{
	m_SolicitType = Shipments;
	accept();
}
