//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldstore.cpp,v 1.9 2022/02/19 18:09:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubldstore.h"
//
#include "auapp.h"
#include "aucommbuy.h"
#include "aucommodityfilter.h"
#include "aucommsellcargo.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "dabd.h"
#include "daperson.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QTextBrowser>
#include <QTimer>
#include <QGridLayout>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBldStore::AuBldStore(QWidget* parent) : QWidget(parent)
{
	// Init member variables.
	//m_BuyItem = item.Clone();
	//m_EmpireOfOrigin = empireNameOfOrigin;
	//m_MaxCount = item.Count();
	//m_NominalPrice = round(item.UnitCost());
	//m_UnitPrice = price;

	m_Account = 0;
	m_BuyItem = 0;
	m_Filter = FILTER_All;
	m_IsOpen = true;
	m_Lounges = 0;
	m_MaxCount = 0;
	m_NominalPrice = 0;
	m_OfferItem = 0;
	m_Stores = 0;
	m_Timer = 0;
	m_UnitPrice = 0;

	AuCommodityFilter* cfilt = new AuCommodityFilter;
	connect(cfilt, SIGNAL(FilterChanged(COMMODITYFILTER)), SLOT(Filter(COMMODITYFILTER)));

	m_Cargo = new QListWidget(this);
	m_Cargo->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_Cargo->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_Cargo->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Cargo->setSortingEnabled(true);
	m_Cargo->setToolTip(tr("<html>"
				"Choose a commodity to buy."
				"</html>"));
	connect(m_Cargo, SIGNAL(itemSelectionChanged()), SLOT(CargoSelected()));

	m_CargoDesc = new QTextBrowser(this);
	m_CargoDesc->setReadOnly(true);

	QLabel* lbab = new QLabel(tr("Personal Account Balance"), this);
	lbab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_AccountBalance = new QLabel(this);
	m_AccountBalance->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

	QFrame* frm = new QFrame(this);
	frm->setFrameStyle(QFrame::HLine|QFrame::Sunken);

	m_OfferCombo = new QComboBox(this);
	m_OfferCombo->setToolTip(tr("<html><b>Commodity Offered</b><br>"
				"Price offer is the total amount of money or other commodities that you are offering. "
				"Select the commodity to offer.<br>"
				"%1 Unit counts show the quantity you have available, not the quantity you are offering.<br>"
				"%1 Cronodollars are taken from your personal account."
				"</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(m_OfferCombo, SIGNAL(currentIndexChanged(int)), SLOT(EnableControls()));

	m_NominalPercent = new QLabel(this);

	m_Price = new QLabel(this);

	// Quantity to purchase.
	QLabel* lbcount = new QLabel(tr("Quantity to Buy"), this);
	lbcount->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_BuyCountSpin = new QSpinBox(this);
	m_BuyCountSpin->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_BuyCountSpin->selectAll();
	m_BuyCountSpin->setMinimum(1);
	m_BuyCountSpin->setMaximum(2000000000);
	m_BuyCountSpin->setValue(1);
	m_BuyCountSpin->setToolTip(tr("<html><b>Quantity to Buy</b><br>"
				"Enter the quantity of the item that you want to buy.</html>"));
	connect(m_BuyCountSpin, SIGNAL(valueChanged(int)), SLOT(QuantityToBuyChanged(int)));

	// Unit price or equivalent.
	m_UnitPriceInfoLabel = new QLabel(tr("Price Offered Per Unit"), this);
	m_UnitPriceInfoLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QString unitPrice = QString("%L1%2").arg(m_UnitPrice, 0,'f',2).arg(g_Au.CurrencySymbol());
	m_UnitPriceLabel = new QLabel(unitPrice, this);
	m_UnitPriceLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_UnitPriceLabel->setToolTip(tr("<b>Price Offered Per Unit</b><br>"
				"Shows the equivalent amount of money per unit that you are offering, calculated from your total offer.<br>"
				"%1 When offering money, this is the amount of money offered divided by the quantity to buy.<br>"
				"%1 When offering other commodities, this is the value of the commodities offered divided by the quantity to buy. "
				"The value of the commodities is the nominal price of the item multiplied by the quantity offered.</html>")
			.arg(g_Au.MiddleDotSymbol()));

	// Offer
	QLabel* lbprice = new QLabel(tr("Offer"), this);
	lbprice->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_OfferCountSpin = new QSpinBox(this);
	m_OfferCountSpin->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_OfferCountSpin->setRange(1, 1);
	connect(m_OfferCountSpin, SIGNAL(valueChanged(int)), SLOT(EnableControls()));
	m_OfferCountSpin->setToolTip(tr("<html><b>Quantity Offered</b><br>"
				"Price offer is the total amount of money or other commodities that you are offering. "
				"</html>"));

	m_OkIcon = new QLabel(this);
	m_OkIcon->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_OkButton = new QPushButton(tr("Offer to Buy"), this);
	connect(m_OkButton, SIGNAL(clicked()), SLOT(OkPressed()));
	m_OkButton->setToolTip(tr("<html>Offers to buy the commodities in exchange for the compensation offered. "
				"</html>"));

	m_SellButton = new QPushButton(tr("Offer to Sell"), this);
	connect(m_SellButton, SIGNAL(clicked()), SLOT(SellPressed()));
	m_SellButton->setToolTip(tr("<html>Opens a private communications channel with the proprietor, "
				"to discuss sales of commodities from your gear. "
				"</html>"));

	// Build the layout.
	QHBoxLayout* cargolay = new QHBoxLayout;
	cargolay->addWidget(m_Cargo);
	cargolay->addWidget(m_CargoDesc);

	// Build the final layout.
	QGridLayout* lay = new QGridLayout();
	int row = 0;
	lay->addLayout(cargolay, row,0, 1,8);
	row++;

	lay->addWidget(cfilt, row,0, 1,8);
	row++;

	lay->addWidget(lbab, row,0, 1,3);
	lay->addWidget(m_AccountBalance, row,3, 1,5);
	row++;

	lay->addWidget(frm, row,0, 1,8);
	row++;

	lay->addWidget(m_Price, row, 1, 1,7);
	row++;

	lay->addWidget(lbcount, row, 0, 1,3);
	lay->addWidget(m_BuyCountSpin, row, 3, 1,3);
	row++;

	lay->addWidget(m_UnitPriceInfoLabel, row,0, 1,3);
	lay->addWidget(m_UnitPriceLabel, row,3, 1,3);
	lay->addWidget(m_NominalPercent, row,6, 1,2);
	row++;

	lay->addWidget(m_OkIcon, row,0, 2,2);
	lay->addWidget(lbprice, row, 2, 1,1);
	lay->addWidget(m_OfferCountSpin, row, 3, 1,3);
	lay->addWidget(m_OfferCombo, row, 6, 1,1);
	row++;

	lay->addWidget(m_OkButton, row, 3, 1,3);
	lay->addWidget(m_SellButton, row, 7, 1,1);
	row++;

	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
	EnableControls();
}

AuBldStore::~AuBldStore()
{
	if(m_BuyItem)
		delete m_BuyItem;
	if(m_OfferItem)
		delete m_OfferItem;
}

void AuBldStore::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_Timer)
	{
		m_Timer = new QTimer(this);
		connect(m_Timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_Timer->isActive())
		m_Timer->start(3000);

	// Show all my widgets.
	QGridLayout* lay = (QGridLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->show();

	UpdateContents();	// Don't wait for 3 seconds.
}

void AuBldStore::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_Timer && m_Timer->isActive())
		m_Timer->stop();

	// Hide my widgets now or they force the parent AuLabor dialog to be too big even though this page isn't visible.
	QGridLayout* lay = (QGridLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->hide();
}

void AuBldStore::UpdateContents()
{
	if(!isVisible())
		return;

	quint32 homesLarge = 0;
	quint32 homesMedium = 0;
	quint32 homesSmall = 0;
	quint32 manufacturingJobs = 0;
	quint32 proJobs = 0;
	quint32 serviceJobs = 0;
	m_Lounges = 0;
	m_Stores = 0;
	quint16 workers = 0;

	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person && person->InIndustry() != IID_None)
	{
		m_Account = person->AccountBalance();
		int offerLimit;
		if(m_Account > 2000000000)
			offerLimit = 2000000000;
		else
			offerLimit = (int)m_Account;
		m_AccountBalance->setText(tr("%L1%2").arg(m_Account, 0,'f',0).arg(g_Au.CurrencySymbol()));

		DaBd* bdOp = person->BdOp(m_CapitolOpId);
		if(bdOp)
		{
			bdOp->IndustryInfo(homesLarge, homesMedium, homesSmall, manufacturingJobs, serviceJobs, m_Stores, m_Lounges, proJobs);
			workers = bdOp->Workers();
			if(m_Stores || m_Lounges)
				ListCargo(bdOp, person->EmpireId(), m_Cargo);
			else
				m_Cargo->clear();
		}

		// List the person's personal items they might want to trade.
		// Consolidate the inventory into a list that stacks items instead of listing them one-by-one.
		// Lists items in top-level containers.
		// Does not list containers that are not empty.
		AuItemHash combinedInv;
		person->GearItemsForSale(combinedInv);
		if(combinedInv != m_Inv)
		{
			m_Inv = combinedInv;
			m_OfferCombo->clear();

			AuItem* itemm = AuItem::Create(CID_Money, 255, 0, offerLimit);
			if(itemm)
			{
				AuOfferItem moneyItem(*itemm, "");
				delete itemm;
				m_OfferCombo->addItem(QIcon(g_Au.CommodityBitmapName(CID_Money)), g_Au.CommodityName(CID_Money),
						QVariant::fromValue(moneyItem));
			}

			for(AuItemHash::const_iterator cit = combinedInv.begin(); cit != combinedInv.end(); cit++)
			{
				if(DbCommodity::IsPatent((*cit).first))
					continue;
				QString empireOfOrigin;
				const DgData& gd = g_Au.LockDataForRead();
				gd.EmpireName((*cit).second->EmpireOfOrigin(), empireOfOrigin);
				g_Au.UnlockData();
				AuOfferItem invItem(*(*cit).second, empireOfOrigin);
				m_OfferCombo->addItem(QIcon((*cit).second->BitmapName()), (*cit).second->ListDesc(empireOfOrigin),
						QVariant::fromValue(invItem));
			}
		}
	}
	pView->UnlockSceneAtom();

	m_IsOpen = workers && (m_Stores || m_Lounges);
	if(!m_Stores && !m_Lounges)
	{
		m_OkIcon->setPixmap(QPixmap(":data/WholesaleOnly.png"));
		m_OkIcon->setToolTip(tr("<html>"
					"This building does not have any stores or lounges where goods could be sold to the public. "
					"<br>"
					"%1 The blueprint of the building must include stores or lounges. "
					"<br>"
					"%1 Some industries do not open stores or lounges, even if the blueprint includes them. "
					"</html>").arg(g_Au.MiddleDotSymbol()));
	}
	else if(workers)
	{
		m_OkIcon->setPixmap(QPixmap(":data/WeAreOpen.png"));
		m_OkIcon->setToolTip(tr("<html>"
					"A worker is present. "
					"This store is open for business. "
					"</html>"));
	}
	else
	{
		m_OkIcon->setPixmap(QPixmap(":data/WeAreClosed.png"));
		m_OkIcon->setToolTip(tr("<html>"
					"No worker is present. "
					"This store is closed for business. "
					"</html>"));
	}
	EnableControls();
}

void AuBldStore::EnableControls()
{
	// Count to buy
	int buyCount = m_BuyCountSpin->value();

	// Set limits on offer counter spin value. This will change the value in the offer count spin control if the new value is
	// not inside its valid range.
	AuOfferItem offerItem = m_OfferCombo->itemData(m_OfferCombo->currentIndex()).value<AuOfferItem>();

	COMMODITYID cid;
	float unitcost;
	quint32 maxCount;
	if(offerItem.m_Item)
	{
		cid = offerItem.m_Item->CommodityId();
		unitcost = offerItem.m_Item->UnitCost();
		maxCount = offerItem.m_Item->Count();
	}
	else
	{
		cid = CID_Invalid;
		unitcost = 0;
		maxCount = 0;
	}

	if(cid == CID_Money)
	{
		int offerLimit;
		if(m_Account > 2000000000)
			offerLimit = 2000000000;
		else
			offerLimit = (int)m_Account;
		m_OfferCountSpin->setRange(1,offerLimit);
	}
	else
		m_OfferCountSpin->setRange(1,maxCount);

	// Calculate equivalent price offered per unit.
	int offerCount = m_OfferCountSpin->value();
	if(cid == CID_Money)
	{
		// Determine the unit price.
		m_UnitPrice = (float)offerCount / (float)buyCount;
	}
	else
	{
		// Determine the equivalent unit price.
		float value = unitcost * offerCount;
		m_UnitPrice = value / (float)buyCount;
	}
	if(m_UnitPrice > 0)
		m_UnitPriceLabel->setText(QString("%L1%2").arg(m_UnitPrice, 0,'f',2).arg(g_Au.CurrencySymbol()));
	else
		m_UnitPriceLabel->setText(tr("Free"));

	if(m_NominalPrice > 0)
	{
		float nom = round(m_UnitPrice/m_NominalPrice * 100.f);
		m_NominalPercent->setText(tr("%L1% of Nominal Price").arg(nom));
	}
	else
		m_NominalPercent->setText("");

	// Enable/disabled controls.
	m_Cargo->setEnabled(m_IsOpen);
	bool enable = m_IsOpen && m_BuyItem;	// We are open and an item is selected to buy.
	m_BuyCountSpin->setEnabled(enable);
	m_OfferCombo->setEnabled(enable);
	m_OfferCountSpin->setEnabled(enable);
	m_OkButton->setEnabled(enable);
}

void AuBldStore::QuantityToBuyChanged(int buyCount)
{
	quint32 offerCount = (quint32)(m_UnitPrice * buyCount);
	if(offerCount > (quint32)m_OfferCountSpin->maximum())
	{
		// Calculate values based on maximum offer count.
		offerCount = m_OfferCountSpin->maximum();

		// Calculate equivalent price offered per unit.
		AuOfferItem offerItem = m_OfferCombo->itemData(m_OfferCombo->currentIndex()).value<AuOfferItem>();
		if(offerItem.m_Item)
		{
			if(offerItem.m_Item->CommodityId() == CID_Money)
			{
				// Determine the unit price.
				m_UnitPrice = (float)offerCount / (float)buyCount;
			}
			else
			{
				// Determine the equivalent unit price.
				float value = offerItem.m_Item->UnitCost() * offerCount;
				m_UnitPrice = value / (float)buyCount;
			}
			if(m_UnitPrice > 0)
				m_UnitPriceLabel->setText(QString("%L1%2").arg(m_UnitPrice, 0,'f',2).arg(g_Au.CurrencySymbol()));
			else
				m_UnitPriceLabel->setText(tr("Free"));
		}
		else
		{
			m_UnitPrice = 0;
			m_UnitPriceLabel->setText(tr("Unknown"));
		}
	}

	//m_OfferCountSpin->setValue(offerCount);
	EnableControls();
}

void AuBldStore::OkPressed()
{
	if(!m_BuyItem)
		return;
	int buyCount = m_BuyCountSpin->value();
	if(buyCount <= 0 || buyCount > (int)m_BuyItem->Count())
		return;
	int offerCount = m_OfferCountSpin->value();
	if(offerCount <= 0)
		return;

	// Offer Commodity
	AuOfferItem offerItem = m_OfferCombo->itemData(m_OfferCombo->currentIndex()).value<AuOfferItem>();
	if(!offerItem.m_Item)
		return;
	if(m_OfferItem)
		delete m_OfferItem;
	m_OfferItem = offerItem.m_Item->Clone();
	if(!m_OfferItem)
		return;

	m_BuyItem->SetCount(buyCount);
	m_OfferItem->SetCount(offerCount);

	auApp->StoreOfferToBuy(*m_BuyItem, *m_OfferItem, m_CapitolOpId);
}

void AuBldStore::Filter(COMMODITYFILTER filter)
{
	m_Filter = filter;
	UpdateContents();
}

void AuBldStore::CargoSelected()
{
	if(m_BuyItem)
	{
		delete m_BuyItem;
		m_BuyItem = 0;
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
					m_BuyItem = AuItem::Unpack(code);
					if(m_BuyItem)
					{
						QSize packSize = m_BuyItem->BitmapSizeToFit(100);
						cargotext = QString("<html>"
								"<img align=\"right\" width=\"%3\" height=\"%4\" src=\"qrc%1\">%2"
								"</html>")
							.arg(m_BuyItem->BitmapName())
							.arg(m_BuyItem->HtmlDesc())
							.arg(packSize.width())
							.arg(packSize.height());
						m_EmpireOfOrigin = m_BuyItem->EmpireNameOfOrigin();
						m_MaxCount = m_BuyItem->Count();
					}
				}
			}
		}
		m_CargoDesc->setHtml(cargotext);
	}

	if(m_BuyItem)
	{
		m_NominalPrice = round(m_BuyItem->UnitCost());

		// Quantity they can afford.
		int affordCount;
		if(m_NominalPrice * m_MaxCount > m_Account)
			affordCount = m_Account / m_NominalPrice;
		else
			affordCount = m_MaxCount;

		if(affordCount < m_BuyCountSpin->value())
			m_BuyCountSpin->setValue(affordCount);

		if(m_MaxCount)
			m_BuyCountSpin->setMaximum(m_MaxCount);
		else
			m_BuyCountSpin->setMaximum(2000000000);

		int offerLimit;
		if(m_Account > 2000000000)
			offerLimit = 2000000000;
		else
			offerLimit = (int)m_Account;
		m_OfferCountSpin->setRange(1, offerLimit);

		// Nominal Unit Price at Auction
		int qualCost = m_BuyItem->UnitCost();
		QString nom = tr("Nominal Price Per Unit %L1%2").arg(qualCost).arg(g_Au.CurrencySymbol());
		m_Price->setText(nom);
	}
	else
	{
		m_Price->setText("");
		m_UnitPriceLabel->setText("");
		m_NominalPercent->setText("");
	}

	EnableControls();
}

void AuBldStore::ListCargo(DaBd* bdOp, quint32 buyerEmpire, QListWidget* lw)
{
	quint32 empireId = bdOp->EmpireId();
	POLITICAL_STANCE stanceToBuyer = PoliticalStance(empireId, buyerEmpire);

	// Get a list of the commodities that fit into the current filter.
	QList<COMMODITYID> cidFilter;
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, m_Filter);

	switch(bdOp->IndustryId())
	{
	default:	
		if(m_Stores)
			break;						// Buildings with stores can sell anything they have in stock.
		else if(!m_Lounges)
		{
			// No stores and no lounges. How did we even get here?
			cidFilter.clear();
			break;
		}
		// Non-cantinas can have lounges. They sell cantina items.
		[[fallthrough]]; // Fall through to the cantina.
	case IID_Cantina:
		// Remove commodities that are not sold at a cantina.
		{
			QList<COMMODITYID>::iterator fi = cidFilter.begin();
			while(fi != cidFilter.end())
				if(g_Au.Commodity(*fi).IsCantinaItem())
					fi++;
				else
					fi = cidFilter.erase(fi);
		}
		break;
	case IID_Grocery:
		// Remove commodities that are not sold at a grocery.
		{
			QList<COMMODITYID>::iterator fi = cidFilter.begin();
			while(fi != cidFilter.end())
				if(g_Au.Commodity(*fi).IsGroceryItem())
					fi++;
				else
					fi = cidFilter.erase(fi);
		}
		break;
	case IID_RetailStore:
		// Remove commodities that are not sold at a retail store.
		{
			QList<COMMODITYID>::iterator fi = cidFilter.begin();
			while(fi != cidFilter.end())
				if(g_Au.Commodity(*fi).IsRetailItem())
					fi++;
				else
					fi = cidFilter.erase(fi);
		}
		break;
	}

	// Remove things from the filter list that are not ok to sell to the buyer, according to empire export policies.
	if(stanceToBuyer != POL_Empire && stanceToBuyer != POL_Vassal)
	{
		QList<COMMODITYID>::iterator fi = cidFilter.begin();
		while(fi != cidFilter.end())
		{
			const DgData& gd = g_Au.LockDataForRead();
			EXPORT_POLICY expol = gd.EmpireCommerce(empireId, *fi);
			g_Au.UnlockData();
			switch(expol)
			{
			case EXPOL_ToNobody:			// Cannot be exported to any other empires.
				fi = cidFilter.erase(fi);
				break;
			case EXPOL_ToEnemy:				// Can be exported to any empire.
				fi++;
				break;
			case EXPOL_ToFriend:			// Can be exported only to friend empires.
				if(stanceToBuyer != POL_Friend)
					fi = cidFilter.erase(fi);
				else
					fi++;
				break;
			case EXPOL_ToNeutral:			// Can be exported to friend or neutral empires.
				if(stanceToBuyer != POL_Friend && stanceToBuyer != POL_Neutral)
					fi = cidFilter.erase(fi);
				else
					fi++;
				break;
			default:
				fi++;
				break;
			}
		}
	}

	// Watch for changes.
	// Insure we have the empire and specie data loaded for all items in cargo, not including items in containers.
	{
		QStringList allCargo;
		const AuItemHash& cr = bdOp->LockCargoForRead();
		auApp->QueryEmpireAndSpecie(cr);
		for(AuItemHash::const_iterator cit = cr.begin(); cit != cr.end(); cit++)
		{
			if(!cidFilter.contains((*cit).second->CommodityId()))
				continue;
			QString text = (*cit).second->ListDesc();
			allCargo.append(text);
		}
		bdOp->UnlockCargo();
		if(allCargo == m_AllCargo)
			return;
		m_AllCargo = allCargo;
	}

	// List commodities.
	{
		lw->clear();

		const AuItemHash& cr = bdOp->LockCargoForRead();
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
		}
		bdOp->UnlockCargo();
	}
}

void AuBldStore::SellPressed()
{
	auApp->StartSalesDialog();
}
