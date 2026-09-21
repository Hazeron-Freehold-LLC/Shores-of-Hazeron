//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommsellcargo.cpp,v 1.10 2022/02/19 18:10:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommsellcargo.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auglobal.h"
#include "auview.h"
#include "daatom.h"
#include "daperson.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QTextBrowser>
#include <QTimer>

AuCommSellCargo::AuCommSellCargo(QWidget* parent) : AuDialog(parent)
{
	m_MaxCount = 0;
	m_Price = 0;			// asking price, 1 for auctions
	m_Item = 0;
	m_Shipment = false;
	m_timer = 0;

	setAuDlgIcon(QIcon(":data/OrderGetItem.png"));

	int lastOffer = g_Au.Settings().readNumEntry("Widget/SellCargo/Offer", 1);

	QVBoxLayout* lay = new QVBoxLayout();	// This will be the final layout.

	{
		QLabel* lbsell = new QLabel();
		lbsell->setPixmap(QPixmap(":data/MsgSellOffer.png").scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));

		QLabel* lbauction = new QLabel();
		lbauction->setPixmap(QPixmap(":data/MsgAuction.png").scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));

		QLabel* lbgive = new QLabel();
		lbgive->setPixmap(QPixmap(":data/OrderGetItem.png").scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));

		m_SellRadio = new QRadioButton(tr("Offer to sell items at a stated price."), this);
		m_SellRadio->setChecked(lastOffer == 1);
		connect(m_SellRadio, SIGNAL(clicked()), SLOT(SellRadio()));

		m_AuctionRadio = new QRadioButton(tr("Offer to sell items without stating a price. Request offers."), this);
		m_AuctionRadio->setChecked(lastOffer == 2);
		connect(m_AuctionRadio, SIGNAL(clicked()), SLOT(AuctionRadio()));

		m_GiveRadio = new QRadioButton(tr("Offer to give items for free."), this);
		m_GiveRadio->setChecked(lastOffer == 3);
		connect(m_GiveRadio, SIGNAL(clicked()), SLOT(GiveRadio()));

		QGridLayout* rlay = new QGridLayout;
		int row=0;
		rlay->addWidget(lbsell, row,0);
		rlay->addWidget(m_SellRadio, row,1);
		row++;
		rlay->addWidget(lbauction, row,0);
		rlay->addWidget(m_AuctionRadio, row,1);
		row++;
		rlay->addWidget(lbgive, row,0);
		rlay->addWidget(m_GiveRadio, row,1);
		row++;
		rlay->setColumnStretch(1,10);
		rlay->setSpacing(2);
		rlay->setMargin(2);

		lay->addLayout(rlay);
	}

	// List gear items.
	QLabel* lbinv = new QLabel(tr("Personal Gear"), this);
	lay->addWidget(lbinv);

	m_Inventory = new QListWidget(this);
	m_Inventory->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_Inventory->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_Inventory->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Inventory->setSortingEnabled(true);
	connect(m_Inventory, SIGNAL(itemSelectionChanged()), SLOT(InventorySelected()));
	connect(m_Inventory, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(InventoryDoubleClicked(QListWidgetItem*)));

	m_InventoryDesc = new QTextBrowser(this);
	m_InventoryDesc->setReadOnly(true);

	QHBoxLayout* invlay = new QHBoxLayout;
	invlay->addWidget(m_Inventory);
	invlay->addWidget(m_InventoryDesc);

	lay->addLayout(invlay);

	m_Filter = FILTER_All;
	m_CommodityFilter = new AuCommodityFilter;
	connect(m_CommodityFilter, SIGNAL(FilterChanged(COMMODITYFILTER)), SLOT(Filter(COMMODITYFILTER)));
	lay->addWidget(m_CommodityFilter);

	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(!person)
	{
		pView->UnlockSceneAtom();
		return;
	}

	// List ship's cargo items.
	// Don't list cargo items if the person is not an officer of the ship.
	// Don't list cargo items if the ship is a subsidized station.
	DaVessel* ship = person->FindParentVesselInEnv();
	if(ship && pView->IsPersonInCommand() && (ship->IsPrivatelyOwned() || !ship->IsSpaceStation()))
	{
		setAuDlgTitle(tr("Offer Personal Gear or Cargo"));

		QLabel* lbcargo = new QLabel(tr("Cargo Hold"), this);
		lay->addWidget(lbcargo);

		QGridLayout* clay = new QGridLayout;
		int row=0;

		// Cargo Space Status
		int cargoTotal = ship->CargoCapacityVolume();
		QLabel* lbct = new QLabel(tr("Total Cargo Hold"), this);
		lbct->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		QLabel* lbctv = new QLabel(QString(" %1").arg(cargoTotal), this);
		lbctv->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		clay->addWidget(lbct, row,0);
		clay->addWidget(lbctv, row,1);
		row++;

		int cargoUsed = ship->CargoTotalVolume();
		QLabel* lbcu = new QLabel(tr("Used Cargo Hold"), this);
		lbcu->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		QLabel* lbcuv = new QLabel(QString(" %1").arg(cargoUsed), this);
		lbcuv->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		clay->addWidget(lbcu, row,0);
		clay->addWidget(lbcuv, row,1);
		row++;

		int cargoEmpty = cargoTotal - cargoUsed;
		QLabel* lbce = new QLabel(tr("Empty Cargo Hold"), this);
		lbce->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		QLabel* lbcev = new QLabel(QString(" %1").arg(cargoEmpty), this);
		lbcev->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		clay->addWidget(lbce, row,0);
		clay->addWidget(lbcev, row,1);
		row++;

		QWidget* spc1 = new QWidget(this);
		spc1->setMinimumWidth(10);
		clay->addWidget(spc1, row,2);

		// Fuel Cell Status
		row -= 3;	// To same row as "Total Cargo Hold"
		int fuelTotal = (int)ship->FuelCapacity();
		QLabel* lbft = new QLabel(tr("Total Fuel Cells"), this);
		lbft->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		QLabel* lbftv = new QLabel(QString(" %1").arg(fuelTotal), this);
		lbftv->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		clay->addWidget(lbft, row,3);
		clay->addWidget(lbftv, row,4);
		row++;

		int fuelUsed = (int)ship->FuelQuantity();
		QLabel* lbfu = new QLabel(tr("Used Fuel Cells"), this);
		lbfu->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		QLabel* lbfuv = new QLabel(QString(" %1").arg(fuelUsed), this);
		lbfuv->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		clay->addWidget(lbfu, row,3);
		clay->addWidget(lbfuv, row,4);
		row++;

		int fuelEmpty = fuelTotal - (int)ship->FuelQuantity();
		QLabel* lbfe = new QLabel(tr("Empty Fuel Cells"), this);
		lbfe->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		QLabel* lbfev = new QLabel(QString(" %1").arg(fuelEmpty), this);
		lbfev->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		clay->addWidget(lbfe, row,3);
		clay->addWidget(lbfev, row,4);
		row++;

		QWidget* spc2 = new QWidget(this);
		spc2->setMinimumWidth(10);
		clay->addWidget(spc2, row,5);

		// Capacitor Status
		row -= 3;	// To same row as "Total Cargo Hold"
		int capTotal = (int)ship->CapacitorMax();
		QLabel* lbpt = new QLabel(tr("Total Capacitor"), this);
		lbpt->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		QLabel* lbptv = new QLabel(QString(" %1").arg(capTotal), this);
		lbptv->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		clay->addWidget(lbpt, row,6);
		clay->addWidget(lbptv, row,7);
		row++;

		int capUsed = (int)ship->CapacitorLevel();
		QLabel* lbpu = new QLabel(tr("Used Capacitor"), this);
		lbpu->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		QLabel* lbpuv = new QLabel(QString(" %1").arg(capUsed), this);
		lbpuv->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		clay->addWidget(lbpu, row,6);
		clay->addWidget(lbpuv, row,7);
		row++;

		int capEmpty = capTotal - capUsed;
		QLabel* lbpe = new QLabel(tr("Empty Capacitor"), this);
		lbpe->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		QLabel* lbpev = new QLabel(QString(" %1").arg(capEmpty), this);
		lbpev->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		clay->addWidget(lbpe, row,6);
		clay->addWidget(lbpev, row,7);
		row++;

		// Cargo aboard
		m_Cargo = new QListWidget(this);
		m_Cargo->setEditTriggers(QAbstractItemView::NoEditTriggers);
		m_Cargo->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_Cargo->setSelectionMode(QAbstractItemView::SingleSelection);
		m_Cargo->setSortingEnabled(true);
		connect(m_Cargo, SIGNAL(itemSelectionChanged()), SLOT(CargoSelected()));
		connect(m_Cargo, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(CargoDoubleClicked(QListWidgetItem*)));

		m_CargoDesc = new QTextBrowser(this);
		m_CargoDesc->setReadOnly(true);

		QHBoxLayout* cargolay = new QHBoxLayout;
		cargolay->addWidget(m_Cargo);
		cargolay->addWidget(m_CargoDesc);

		lay->addLayout(clay);
		lay->addLayout(cargolay);
	}
	else
	{
		setAuDlgTitle(tr("Offer Personal Gear"));
		m_Cargo = 0;
	}

	pView->UnlockSceneAtom();

	QLabel* lbcount = new QLabel(tr("Count Offered"), this);
	lbcount->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_CountEdit = new QLineEdit(this);
	m_CountEdit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	connect(m_CountEdit, SIGNAL(textChanged(const QString&)), SLOT(EnableControls()));

	QGridLayout* tlay = new QGridLayout;
	int row=0;
	tlay->addWidget(lbcount, row,0);
	tlay->addWidget(m_CountEdit, row,1, 1,3);
	row++;

	// For Sale proposes a starting price.
	m_PriceLabel = new QLabel(tr("Unit Price Offered"), this);
	m_PriceLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_PriceEdit = new QLineEdit(this);
	m_PriceEdit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	connect(m_PriceEdit, SIGNAL(textChanged(const QString&)), SLOT(EnableControls()));
	m_CurrencySymbolLabel = new QLabel(AuGlobal::CurrencySymbol(), this);

	tlay->addWidget(m_PriceLabel, row,0);
	tlay->addWidget(m_PriceEdit, row,1, 1,3);
	tlay->addWidget(m_CurrencySymbolLabel, row,4);
	row++;

	// Calculate total
	m_TotalLabel = new QLabel(tr("Total Price"), this);
	m_TotalLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_TotalPrice = new QLabel(this);
	m_TotalPrice->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	tlay->addWidget(m_TotalLabel, row,0);
	tlay->addWidget(m_TotalPrice, row,1, 1,3);
	lay->addLayout(tlay);

	if(m_Cargo)
	{
		m_OkTEUButton = new QPushButton(tr("Deliver TEU"), this);
		m_OkTEUButton->setToolTip(tr("Deliver the selected TEU. "
					"The destination city accepts the shipment and it pays the delivery fee."));
		connect(m_OkTEUButton, SIGNAL(clicked()), SLOT(OkTEUPressed()));
	}
	else
		m_OkTEUButton = 0;

	m_OkButton = new QPushButton(tr("Offer Item"), this);
	m_OkButton->setToolTip(tr("Offer the selected item on the current channel."));
	m_OkButton->setAutoDefault(true);
	m_OkButton->setDefault(true);
	connect(m_OkButton, SIGNAL(clicked()), SLOT(OkPressed()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	if(m_OkTEUButton)
		blay->addWidget(m_OkTEUButton);
	blay->addWidget(m_OkButton);
	lay->addLayout(blay);

	setAuDlgLayout(lay);

	// Initial fill of the contents.
	UpdateContents();
}

AuCommSellCargo::~AuCommSellCargo()
{
	if(m_Item)
		delete m_Item;
}

void AuCommSellCargo::EnableControls()
{
	// Offer count
	int count = QVariant(m_CountEdit->text()).toInt();

	// Offer price
	int price = QVariant(m_PriceEdit->text()).toInt();

	// Update total
	int total = count * price;
	m_TotalPrice->setText(QVariant(total).toString() + AuGlobal::CurrencySymbol());

	if(IsAuction())
	{
		m_PriceLabel->setText(tr("Unit Value"));
		m_PriceEdit->setEnabled(false);
		m_TotalLabel->setText(tr("Total Value"));
		m_PriceLabel->show();
		m_PriceEdit->show();
		m_CurrencySymbolLabel->show();
		m_TotalLabel->show();
		m_TotalPrice->show();
	}
	else if(IsSale())
	{
		m_PriceLabel->setText(tr("Unit Price"));
		m_PriceEdit->setEnabled(true);
		m_TotalLabel->setText(tr("Total Price"));
		m_PriceLabel->show();
		m_PriceEdit->show();
		m_CurrencySymbolLabel->show();
		m_TotalLabel->show();
		m_TotalPrice->show();
	}
	else
	{
		m_PriceLabel->hide();
		m_PriceEdit->hide();
		m_CurrencySymbolLabel->hide();
		m_TotalLabel->hide();
		m_TotalPrice->hide();
	}

	if(m_OkTEUButton)
	{
		const AuItemTEUContainer* teuItem = dynamic_cast<const AuItemTEUContainer*>(m_Item);
		bool enableTEU = teuItem && !teuItem->DestinationCity().IsNull();
		m_OkTEUButton->setEnabled(enableTEU);
	}

	// Enable/disable the ok button.
	bool enableOk = false;
	if((int)m_MaxCount && count && count <= (int)m_MaxCount && price >= 0)
		enableOk = true;
	m_OkButton->setEnabled(enableOk);
}

void AuCommSellCargo::OkPressed()
{
	m_Shipment = false;
	QByteArray code;

	// Check for selected cargo item first.
	if(m_Cargo)
	{
		QItemSelectionModel* ism = m_Cargo->selectionModel();
		if(ism)
		{
			QModelIndexList sel = ism->selectedIndexes();
			if(!sel.isEmpty())
			{
				QModelIndex idxsel = sel.first();
				if(idxsel.isValid())
				{
					QListWidgetItem* cur = (QListWidgetItem*)idxsel.internalPointer();
					code = cur->data(Qt::UserRole).toByteArray();
				}
			}
		}
	}

	// Check for selected inventory item if no cargo item was selected.
	if(code.isEmpty())
	{
		QItemSelectionModel* ism = m_Inventory->selectionModel();
		if(ism)
		{
			QModelIndexList sel = ism->selectedIndexes();
			if(!sel.isEmpty())
			{
				QModelIndex idxsel = sel.first();
				if(idxsel.isValid())
				{
					QListWidgetItem* cur = (QListWidgetItem*)idxsel.internalPointer();
					code = cur->data(Qt::UserRole).toByteArray();
				}
			}
		}
	}

	if(!code.isEmpty())
	{
		m_EmpireOfOrigin = "";
		m_MaxCount = 0;

		if(m_Item)
			delete m_Item;
		m_Item = AuItem::Unpack(code);
		if(m_Item)
		{
			m_EmpireOfOrigin = m_Item->EmpireNameOfOrigin();
			m_MaxCount = m_Item->Count();

			// Count to sell.
			int count = QVariant(m_CountEdit->text()).toInt();
			if(count > 0 && count <= (int)m_MaxCount)
				m_Item->SetCount(count);

			// Price - Won't exist if this is an auction.
			m_Price = 0;
			int price = QVariant(m_PriceEdit->text()).toInt();
			if(price > 0)
				m_Price = price;

			if(m_Item && m_Item->Count())
				accept();
		}
	}
}

void AuCommSellCargo::OkTEUPressed()
{
	m_Shipment = true;
	QByteArray code;

	// Check for selected cargo item first.
	if(m_Cargo)
	{
		QItemSelectionModel* ism = m_Cargo->selectionModel();
		if(ism)
		{
			QModelIndexList sel = ism->selectedIndexes();
			if(!sel.isEmpty())
			{
				QModelIndex idxsel = sel.first();
				if(idxsel.isValid())
				{
					QListWidgetItem* cur = (QListWidgetItem*)idxsel.internalPointer();
					code = cur->data(Qt::UserRole).toByteArray();
				}
			}
		}
	}

	if(!code.isEmpty())
	{
		m_EmpireOfOrigin = "";
		m_MaxCount = 0;

		if(m_Item)
			delete m_Item;
		m_Item = AuItem::Unpack(code);
		if(m_Item && m_Item->Type() == AuItem::IT_TEUContainer)
		{
			m_EmpireOfOrigin = m_Item->EmpireNameOfOrigin();
			m_MaxCount = m_Item->Count();
			if(m_Item && m_Item->Count())
				accept();
		}
	}
}

void AuCommSellCargo::CargoSelected()
{
	// Clear selection from the inventory list.
	m_Inventory->blockSignals(true);
	m_Inventory->clearSelection();
	m_Inventory->blockSignals(false);

	if(m_Item)
	{
		delete m_Item;
		m_Item = 0;
	}
	m_EmpireOfOrigin = "";
	m_MaxCount = 0;

	if(m_Cargo)
	{
		QString cargotext;
		QItemSelectionModel* ism = m_Cargo->selectionModel();
		if(ism)
		{
			QModelIndexList sel = ism->selectedIndexes();
			if(!sel.isEmpty())
			{
				QModelIndex idxsel = sel.first();
				if(idxsel.isValid())
				{
					QListWidgetItem* cur = (QListWidgetItem*)idxsel.internalPointer();
					QByteArray code = cur->data(Qt::UserRole).toByteArray();
					m_Item = AuItem::Unpack(code);
					if(m_Item)
					{
						QSize packSize = m_Item->BitmapSizeToFit(100);
						cargotext = QString("<html>"
								"<img align=\"right\" width=\"%3\" height=\"%4\" src=\"qrc%1\">%2"
								"</html>")
							.arg(m_Item->BitmapName())
							.arg(m_Item->HtmlDesc())
							.arg(packSize.width())
							.arg(packSize.height());
						m_EmpireOfOrigin = m_Item->EmpireNameOfOrigin();
						m_MaxCount = m_Item->Count();
					}
				}
			}
		}
		m_CargoDesc->setHtml(cargotext);
	}

	int qualCost;
	if(m_Item)
		qualCost = m_Item->UnitCost();
	else
		qualCost = 0;

	// Unit Price
	m_PriceEdit->setText(QVariant(qualCost).toString());

	// Offer count
	m_CountEdit->setText(QVariant(m_MaxCount).toString());

	EnableControls();
}

void AuCommSellCargo::InventorySelected()
{
	// Clear selection from the cargo list.
	if(m_Cargo)
	{
		m_Cargo->blockSignals(true);
		m_Cargo->clearSelection();
		m_Cargo->blockSignals(false);
	}

	if(m_Item)
	{
		delete m_Item;
		m_Item = 0;
	}
	m_EmpireOfOrigin = "";
	m_MaxCount = 0;

	QString itemtext;
	QItemSelectionModel* ism = m_Inventory->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		if(!sel.isEmpty())
		{
			QModelIndex idxsel = sel.first();
			if(idxsel.isValid())
			{
				QListWidgetItem* cur = (QListWidgetItem*)idxsel.internalPointer();
				QByteArray code = cur->data(Qt::UserRole).toByteArray();
				m_Item = AuItem::Unpack(code);
				if(m_Item)
				{
					QSize packSize = m_Item->BitmapSizeToFit(100);
					itemtext = QString("<html>"
							"<img align=\"right\" width=\"%3\" height=\"%4\" src=\"qrc%1\">%2"
							"</html>")
						.arg(m_Item->BitmapName())
						.arg(m_Item->HtmlDesc())
						.arg(packSize.width())
						.arg(packSize.height());
					m_EmpireOfOrigin = m_Item->EmpireNameOfOrigin();
					m_MaxCount = m_Item->Count();
				}
			}
		}
	}
	m_InventoryDesc->setHtml(itemtext);

	int qualCost;
	if(m_Item)
		qualCost = m_Item->UnitCost();
	else
		qualCost = 0;

	// Unit Price
	m_PriceEdit->setText(QVariant(qualCost).toString());

	// Offer count
	m_CountEdit->setText(QVariant(m_MaxCount).toString());

	EnableControls();
}

// Returns false if the list widget is not filled because we are waiting for info from the server.
bool AuCommSellCargo::ListInventory(const AuGear& gear, QListWidget* lw)
{
	// Insure we have the empire and specie data loaded for all items in gear, including items in containers.
	lw->clear();
	if(auApp->QueryEmpireAndSpecie(gear))
		return false;	// Don't fill the list widget until we have stopped querying for information.

	// Get a list of the commodities that fit into the current filter.
	QList<COMMODITYID> cidFilter;
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, m_Filter);

	// Consolidate the inventory into a list that stacks items instead of listing them one-by-one.
	// Lists items in top-level containers.
	// Does not list containers that are not empty.
	AuItemHash combinedInv;
	gear.CombineItemsForSale(combinedInv);

	for(AuItemHash::const_iterator cit = combinedInv.begin(); cit != combinedInv.end(); cit++)
	{
		// Apply filter to list.
		if(cidFilter.contains((*cit).second->CommodityId()))
		{
			// The item was not filtered out so it can be listed.
			QString text = (*cit).second->ListDesc();
			QByteArray code = (*cit).second->Pack();

			QPixmap px32 = QPixmap((*cit).second->BitmapName()).scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			QIcon icon(px32);

			QListWidgetItem* lwi = new QListWidgetItem(icon, text, lw);
			lwi->setData(Qt::UserRole, code);
			lwi->setToolTip((*cit).second->HtmlToolTip());
		}
	}

	return true;
}

// Returns false if the list widget is not filled because we are waiting for info from the server.
bool AuCommSellCargo::ListCargo(DaVessel* ship, QListWidget* lw)
{
	// Insure we have the empire and specie data loaded for all items in cargo, not including items in containers.
	lw->clear();
	const AuItemHash& cr = ship->LockCargoForRead();
	if(auApp->QueryEmpireAndSpecie(cr))
	{
		ship->UnlockCargo();
		return false;	// Don't fill the list widget until we have stopped querying for information.
	}

	// Get a list of the commodities that fit into the current filter.
	QList<COMMODITYID> cidFilter;
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, m_Filter);

	for(AuItemHash::const_iterator cit = cr.begin(); cit != cr.end(); cit++)
	{
		// Apply filter to list.
		if(!cidFilter.contains((*cit).second->CommodityId()))
			continue;

		QString text = (*cit).second->ListDesc();
		QByteArray code = (*cit).second->Pack();

		QPixmap px32 = QPixmap((*cit).second->BitmapName()).scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		QIcon icon(px32);

		QListWidgetItem* lwi = new QListWidgetItem(icon, text, lw);
		lwi->setData(Qt::UserRole, code);
		lwi->setToolTip((*cit).second->HtmlToolTip());
	}
	ship->UnlockCargo();

	if(m_Filter == FILTER_All || m_Filter == FILTER_Fuel)
	{
		// List Hydrogen fuel as cargo.
		int fuelQty = (int)ship->FuelQuantity();
		if(fuelQty)
		{
			AuItem* fuelItem = AuItem::Create(CID_Hydrogen, ship->FuelQuality(), ship->EmpireId(), fuelQty);
			if(fuelItem)
			{
				QString text = QObject::tr("%1 in fuel cells").arg(fuelItem->ListDesc());
				QByteArray code = fuelItem->Pack();

				QPixmap px32 = QPixmap(fuelItem->BitmapName()).scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				QIcon icon(px32);

				QListWidgetItem* lwi = new QListWidgetItem(icon, text, lw);
				lwi->setData(Qt::UserRole, code);
				lwi->setToolTip(fuelItem->HtmlToolTip());
				delete fuelItem;
			}
		}

		// List Electricity in capacitor as cargo.
		int powerQty = (int)ship->CapacitorLevel();
		if(powerQty)
		{
			AuItem* powerItem = AuItem::Create(CID_Electricity, 255, ship->EmpireId(), powerQty);
			if(powerItem)
			{
				QString text = QObject::tr("%1 in capacitor").arg(powerItem->ListDesc());
				QByteArray code = powerItem->Pack();

				QPixmap px32 = QPixmap(powerItem->BitmapName()).scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				QIcon icon(px32);

				QListWidgetItem* lwi = new QListWidgetItem(icon, text, lw);
				lwi->setData(Qt::UserRole, code);
				lwi->setToolTip(powerItem->HtmlToolTip());
				delete powerItem;
			}
		}
	}

	return true;
}

void AuCommSellCargo::Filter(COMMODITYFILTER filter)
{
	m_Filter = filter;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		ListInventory(*person->LockGearForRead(), m_Inventory);
		person->UnlockGear();
		if(m_Cargo)
		{
			DaVessel* ship = person->FindParentVesselInEnv();
			if(ship)
				ListCargo(ship, m_Cargo);
		}
	}
	pView->UnlockSceneAtom();
}

void AuCommSellCargo::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SellCargo/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SellCargo/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SellCargo/SizeX", szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SellCargo/SizeY", szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);

	m_CommodityFilter->RestoreFilter("Widget/SellCargo/Filter");
}

void AuCommSellCargo::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SellCargo/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SellCargo/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SellCargo/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SellCargo/SizeY",sz.height());

	m_CommodityFilter->SaveFilter("Widget/SellCargo/Filter");
}

void AuCommSellCargo::setVisible(bool visible)
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

void AuCommSellCargo::CargoDoubleClicked(QListWidgetItem* /*item*/)
{
	OkPressed();
}

void AuCommSellCargo::InventoryDoubleClicked(QListWidgetItem* /*item*/)
{
	OkPressed();
}

void AuCommSellCargo::UpdateContents()
{
	bool anyQuery = false;	// Set true if any query is pending for empire or specie info from the server.

	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		// List the person's gear and contents of containers, to one level of depth.
		// We list contents but only to one level. Other code doesn't recurse deeply looking for the things I might offer to
		// sell so it wouldn't work until we fix everybody.
		if(!ListInventory(*person->LockGearForRead(), m_Inventory))
			anyQuery = true;
		person->UnlockGear();

		if(m_Cargo)
		{
			// List ship's cargo items.
			// Don't list cargo items if the person is not an officer of the ship.
			// Don't list cargo items if the ship is a subsidized station.
			DaVessel* ship = person->FindParentVesselInEnv();
			if(ship && pView->IsPersonInCommand() && (ship->IsPrivatelyOwned() || !ship->IsSpaceStation()))
				if(!ListCargo(ship, m_Cargo))	// List the ship's cargo but not the contents of containers.
					anyQuery = true;
		}
	}
	pView->UnlockSceneAtom();

	EnableControls();

	// Prepare to keep updating my contents until no more queries of the server are needed.
	if(anyQuery)
	{
		// A query is pending for empire or specie data. Start a timer to try again until we have all the data.
		if(!m_timer)
		{
			m_timer = new QTimer(this);
			connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
		}
		if(!m_timer->isActive())
			m_timer->start(2000);
	}
	else
	{
		// No query was pending so stop the update timer.
		if(m_timer)
			m_timer->stop();
	}
}

const QString AuCommSellCargo::EmpireOfOrigin() const
{
	return m_EmpireOfOrigin;
}

bool AuCommSellCargo::IsAuction() const
{
	return m_AuctionRadio->isChecked();
}

bool AuCommSellCargo::IsGiveAway() const
{
	return m_GiveRadio->isChecked();
}

bool AuCommSellCargo::IsSale() const
{
	return m_SellRadio->isChecked();
}

bool AuCommSellCargo::IsShipment() const
{
	return m_Shipment;
}

void AuCommSellCargo::AuctionRadio()
{
	g_Au.Settings().writeEntry("Widget/SellCargo/Offer", 2);
	EnableControls();
}

void AuCommSellCargo::GiveRadio()
{
	g_Au.Settings().writeEntry("Widget/SellCargo/Offer", 3);
	EnableControls();
}

void AuCommSellCargo::SellRadio()
{
	g_Au.Settings().writeEntry("Widget/SellCargo/Offer", 1);
	EnableControls();
}

unsigned int AuCommSellCargo::Price() const
{
	if(IsGiveAway())
		return 0;
	return m_Price;
}
