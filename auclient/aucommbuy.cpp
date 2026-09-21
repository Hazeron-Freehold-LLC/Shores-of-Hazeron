//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommbuy.cpp,v 1.8 2022/02/19 18:10:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommbuy.h"
//
#include "auapp.h"
#include <auglobal.h>
#include "auframe.h"
#include "auview.h"
#include <daperson.h>
#include <davehicle.h>
#include <dgdata.h>
#include <QFrame>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVariant>

// Must call qRegisterMetaType in order to use this class type in QVariant and queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_AuOfferItemMetaType = qRegisterMetaType<AuOfferItem>();

AuCommBuy::AuCommBuy(const AuItem& item, const QString& empireNameOfOrigin, unsigned int price, QWidget* parent) : AuDialog(parent)
{
	// Init member variables.
	m_BuyItem = item.Clone();
	m_EmpireOfOrigin = empireNameOfOrigin;
	m_MaxCount = item.Count();
	m_UnitPrice = price;

	m_OfferItem = 0;

	// Construct dialog.
	setAuDlgTitle("Bid");
	setAuDlgIcon(QIcon(":data/CommTrade.png"));

	// We will build the layout as we go because of this dialog's alternate appearances.
	QVBoxLayout* lay = new QVBoxLayout;

	m_OfferCombo = new QComboBox(this);
	m_OfferCombo->setToolTip(tr("<html><b>Commodity Offered</b><br>"
				"Price offer is the total amount of money or other commodities that you are offering. "
				"Select the commodity to offer.<br>"
				"%1 Commodities in your gear are marked with <b>*</b>.<br>"
				"%1 Unit counts show the quantity you have available, not the quantity you are offering.<br>"
				"%1 Cronodollars are taken from your personal account when making personal transactions.<br>"
				"%1 Cronodollars are taken from the ship's account when making cargo transactions.</html>")
			.arg(g_Au.MiddleDotSymbol()));

	m_NominalPercent = new QLabel(this);

	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		QGridLayout* play = new QGridLayout;
		int row=0;

		// Person's account balance.
		m_Account = person->AccountBalance();
		int offerLimit;
		if(m_Account > 2000000000)
			offerLimit = 2000000000;
		else
			offerLimit = (int)m_Account;
		AuItem* itemm = AuItem::Create(CID_Money, 255, 0, offerLimit);
		if(itemm)
		{
			AuOfferItem moneyItem(*itemm, "");
			delete itemm;
			m_OfferCombo->addItem(QIcon(g_Au.CommodityBitmapName(CID_Money)), g_Au.CommodityName(CID_Money),
					QVariant::fromValue(moneyItem));
		}

		QLabel* lbab = new QLabel(tr("Personal Account Balance"), this);
		lbab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		QLabel* lbabv = new QLabel(QObject::tr(" %L1%2").arg(m_Account, 0,'f',0).arg(g_Au.CurrencySymbol()), this);
		lbabv->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

		play->addWidget(lbab, row,0, 1,3);
		play->addWidget(lbabv, row,3, 1,5);
		row++;

		QFrame* frm = new QFrame(this);
		frm->setFrameStyle(QFrame::HLine|QFrame::Sunken);
		play->addWidget(frm, row,0, 1,8);
		row++;

		// Design and mission disks are not purchased as cargo using ships funds.
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship &&
				item.CommodityId() != CID_BdDesignDisk &&
				item.CommodityId() != CID_BdDesignDrawing &&
				item.CommodityId() != CID_ScDesignDisk &&
				item.CommodityId() != CID_ScDesignDrawing &&
				item.CommodityId() != CID_MissionDisk &&
				item.CommodityId() != CID_OfficerLog)
		{
			if(pView->IsPersonInCommand())
			{
				// Ship's account balance.
				double account = ship->AccountBalance();
				if(account > m_Account)
					m_Account = account;
				QLabel* lbab = new QLabel(tr("Ship Account Balance"), this);
				lbab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
				QLabel* lbabv = new QLabel(QObject::tr(" %L1%2").arg(account, 0,'f',0).arg(g_Au.CurrencySymbol()), this);
				lbabv->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

				play->addWidget(lbab, row,0, 1,3);
				play->addWidget(lbabv, row,3, 1,5);
				row++;

				// List ship cargo that they might want to trade.
				const AuItemHash& cr = ship->LockCargoForRead();
				for(AuItemHash::const_iterator ci = cr.begin(); ci != cr.end(); ci++)
				{
					// We do not permit containers with contents in the cargo hold so we don't bother checking for that.
					QString empireOfOrigin;
					const DgData& gd = g_Au.LockDataForRead();
					gd.EmpireName((*ci).second->EmpireOfOrigin(), empireOfOrigin);
					g_Au.UnlockData();

					AuOfferItem cargoItem(*(*ci).second, empireOfOrigin);
					m_OfferCombo->addItem(QIcon((*ci).second->BitmapName()), (*ci).second->ListDesc(empireOfOrigin),
							QVariant::fromValue(cargoItem));
				}
				ship->UnlockCargo();
			}

			// Cargo Space Status
			int cargoTotal = ship->CargoCapacityVolume();
			QLabel* lbct = new QLabel(tr("Total Cargo Hold"), this);
			lbct->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
			QLabel* lbctv = new QLabel(QObject::tr(" %L1").arg(cargoTotal), this);
			lbctv->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

			play->addWidget(lbct, row,0);
			play->addWidget(lbctv, row,1);
			row++;

			int cargoUsed = ship->CargoTotalVolume();
			QLabel* lbcu = new QLabel(tr("Used Cargo Hold"), this);
			lbcu->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
			QLabel* lbcuv = new QLabel(QObject::tr(" %L1").arg(cargoUsed), this);
			lbcuv->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

			play->addWidget(lbcu, row,0);
			play->addWidget(lbcuv, row,1);
			row++;

			int cargoEmpty = cargoTotal - cargoUsed;
			QLabel* lbce = new QLabel(tr("Empty Cargo Hold"), this);
			lbce->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
			QLabel* lbcev = new QLabel(QObject::tr(" %L1").arg(cargoEmpty), this);
			lbcev->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

			play->addWidget(lbce, row,0);
			play->addWidget(lbcev, row,1);
			row++;

			QWidget* spc1 = new QWidget(this);
			spc1->setMinimumWidth(10);
			play->addWidget(spc1, row, 2);

			// Fuel Cell Status
			row -= 3;	// To same row as "Total Cargo Hold"
			int fuelTotal = (int)ship->FuelCapacity();
			QLabel* lbft = new QLabel(tr("Total Fuel Cells"), this);
			lbft->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
			QLabel* lbftv = new QLabel(QObject::tr(" %L1").arg(fuelTotal), this);
			lbftv->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

			play->addWidget(lbft, row,3);
			play->addWidget(lbftv, row,4);
			row++;

			int fuelUsed = (int)ship->FuelQuantity();
			QLabel* lbfu = new QLabel(tr("Used Fuel Cells"), this);
			lbfu->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
			QLabel* lbfuv = new QLabel(QObject::tr(" %L1").arg(fuelUsed), this);
			lbfuv->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

			play->addWidget(lbfu, row,3);
			play->addWidget(lbfuv, row,4);
			row++;

			unsigned int fuelEmpty = fuelTotal - fuelUsed;
			QLabel* lbfe = new QLabel(tr("Empty Fuel Cells"), this);
			lbfe->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
			QLabel* lbfev = new QLabel(QObject::tr(" %L1").arg(fuelEmpty), this);
			lbfev->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
			if(item.CommodityId() == CID_Hydrogen)
			{
				// Don't allow purchase of more fuel than there is fuel cell space.
				if(m_MaxCount > fuelEmpty)
					m_MaxCount = fuelEmpty;
			}

			play->addWidget(lbfe, row,3);
			play->addWidget(lbfev, row,4);
			row++;

			QWidget* spc2 = new QWidget(this);
			spc2->setMinimumWidth(10);
			play->addWidget(spc2, row, 5);

			// Capacitor Status
			row -= 3;	// To same row as "Total Cargo Hold"
			int capTotal = (int)ship->CapacitorMax();
			QLabel* lbpt = new QLabel("Total Capacitor", this);
			lbpt->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
			QLabel* lbptv = new QLabel(QObject::tr(" %L1").arg(capTotal), this);
			lbptv->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

			play->addWidget(lbpt, row,6);
			play->addWidget(lbptv, row,7);
			row++;

			int capUsed = (int)ship->CapacitorLevel();
			QLabel* lbpu = new QLabel(tr("Used Capacitor"), this);
			lbpu->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
			QLabel* lbpuv = new QLabel(QObject::tr(" %L1").arg(capUsed), this);
			lbpuv->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

			play->addWidget(lbpu, row,6);
			play->addWidget(lbpuv, row,7);
			row++;

			int capEmpty = capTotal - capUsed;
			QLabel* lbpe = new QLabel(tr("Empty Capacitor"), this);
			lbpe->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
			QLabel* lbpev = new QLabel(QString(" %L1").arg(capEmpty), this);
			lbpev->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

			play->addWidget(lbpe, row,6);
			play->addWidget(lbpev, row,7);
			row++;

			QFrame* frm = new QFrame(this);
			frm->setFrameStyle(QFrame::HLine|QFrame::Sunken);
			play->addWidget(frm, row,0, 1,8);
			row++;
		}
		else
		{
			const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(person->Parent());
			if(vehicle)
			{
				// Person is sitting in a vehicle.
				// Don't allow purchase of more fuel than there is fuel cell space.
				// Max fuel that can be purchased is one because that's all it takes to refuel any vehicle.
				if(item.CommodityId() == vehicle->FuelType())
					if(m_MaxCount > 1)
						m_MaxCount = 1;
			}
		}
		lay->addLayout(play);

		// List the person's personal items they might want to trade.
		// This is done last so personal items follow items aboard the ship.
		// Consolidate the inventory into a list that stacks items instead of listing them one-by-one.
		// Lists items in top-level containers.
		// Does not list containers that are not empty.
		AuItemHash combinedInv;
		person->GearItemsForSale(combinedInv);
		for(AuItemHash::const_iterator cit = combinedInv.begin(); cit != combinedInv.end(); cit++)
		{
			QString empireOfOrigin;
			const DgData& gd = g_Au.LockDataForRead();
			gd.EmpireName((*cit).second->EmpireOfOrigin(), empireOfOrigin);
			g_Au.UnlockData();

			AuOfferItem invItem(*(*cit).second, empireOfOrigin);
			m_OfferCombo->addItem(QIcon((*cit).second->BitmapName()), (*cit).second->ListDesc(empireOfOrigin) + " *",
					QVariant::fromValue(invItem));
		}
	}
	pView->UnlockSceneAtom();
	connect(m_OfferCombo, SIGNAL(currentIndexChanged(int)), SLOT(EnableControls()));

	// Commodity
	m_NominalPrice = round(item.UnitCost());
	QGridLayout* clay = new QGridLayout;
	int row=0;
	if(item.CommodityId())
	{
		// This is a specific bid; only show one commodity.
		QString comDesc = item.HtmlToolTip();

		QLabel* lbcomi = new QLabel(this);
		lbcomi->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		QPixmap pico(item.BitmapName());
		lbcomi->setPixmap(pico.scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		lbcomi->setToolTip(comDesc);

		// Format example: 'Stone Poor Q94, 12 units'
		QString name = item.ListDesc(empireNameOfOrigin);
		if(price)
			name += tr(", %L1%2 each").arg(price).arg(AuGlobal::CurrencySymbol());
		else
			name += tr(", for free");
		QLabel* lbcommod = new QLabel(name, this);
		lbcommod->setToolTip(comDesc);

		clay->addWidget(lbcomi, row, 0);
		clay->addWidget(lbcommod, row, 1, 1,7);
		row++;

		QString nom = tr("Nominal Price Per Unit %L1%2").arg(m_NominalPrice).arg(g_Au.CurrencySymbol());
		QLabel* lbpriceN = new QLabel(nom, this);
		clay->addWidget(lbpriceN, row, 1, 1,7);
		row++;
	}

	// Quantity they can afford.
	int affordCount;
	if(price * m_MaxCount > m_Account)
		affordCount = m_Account / price;
	else
		affordCount = m_MaxCount;
	//qDebug() << "price" << price << "maxCount" << m_MaxCount << "account" << m_Account << "affordCount" << affordCount;

	// Quantity to purchase.
	QLabel* lbcount = new QLabel(tr("Quantity to Buy"), this);
	lbcount->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_BuyCountSpin = new QSpinBox(this);
	m_BuyCountSpin->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_BuyCountSpin->selectAll();
	m_BuyCountSpin->setMinimum(1);
	if(m_MaxCount > 0)
	{
		m_BuyCountSpin->setMaximum(m_MaxCount);
		m_BuyCountSpin->setValue(affordCount);
	}
	else
	{
		m_BuyCountSpin->setMaximum(2000000000);
		m_BuyCountSpin->setValue(1);
	}
	m_BuyCountSpin->setToolTip(tr("<html><b>Quantity to Buy</b><br>"
				"Enter the quantity of the item that you want to buy.</html>"));
	connect(m_BuyCountSpin, SIGNAL(valueChanged(int)), SLOT(QuantityToBuyChanged(int)));

	clay->addWidget(lbcount, row, 0, 1,3);
	clay->addWidget(m_BuyCountSpin, row, 3, 1,3);
	row++;

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

	clay->addWidget(m_UnitPriceInfoLabel, row,0, 1,3);
	clay->addWidget(m_UnitPriceLabel, row,3, 1,3);
	clay->addWidget(m_NominalPercent, row,6, 1,2);
	row++;

	// Offer
	int offerLimit;
	if(m_Account > 2000000000)
		offerLimit = 2000000000;
	else
		offerLimit = (int)m_Account;
	QLabel* lbprice = new QLabel(tr("Offer"), this);
	lbprice->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_OfferCountSpin = new QSpinBox(this);
	m_OfferCountSpin->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_OfferCountSpin->setRange(0, offerLimit);
	m_OfferCountSpin->setValue(price * affordCount);
	connect(m_OfferCountSpin, SIGNAL(valueChanged(int)), SLOT(EnableControls()));
	m_OfferCountSpin->setToolTip(tr("<html><b>Quantity Offered</b><br>"
				"Price offer is the total amount of money or other commodities that you are offering. "
				"Enter 0 to request items for free, such as fuel, for fleet ships.</html>"));

	clay->addWidget(lbprice, row, 0, 1,3);
	clay->addWidget(m_OfferCountSpin, row, 3, 1,3);
	clay->addWidget(m_OfferCombo, row, 6, 1,2);
	row++;

	m_OkButton = new QPushButton(tr("Offer Bid"));
	m_OkButton->setAutoDefault(true);
	m_OkButton->setDefault(true);
	connect(m_OkButton, SIGNAL(clicked()), SLOT(OkPressed()));
	m_OkButton->setToolTip(tr("Offers this bid on the current channel."));

	clay->addWidget(m_OkButton, row, 3, 1,3);
	row++;
	lay->addLayout(clay);
	setAuDlgLayout(lay);

	EnableControls();
}

AuCommBuy::~AuCommBuy()
{
	if(m_BuyItem)
		delete m_BuyItem;
	if(m_OfferItem)
		delete m_OfferItem;
}

void AuCommBuy::EnableControls()
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
		m_OfferCountSpin->setMaximum(offerLimit);
	}
	else
		m_OfferCountSpin->setMaximum(maxCount);

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
		m_NominalPercent->setText(tr(" %L1% of Nominal Price").arg(nom, 0,'f',0));
	}
	else
		m_NominalPercent->setText("");
}

void AuCommBuy::OkPressed()
{
	if(!m_BuyItem)
		return;

	// Count requested to buy.
	int buyCount = m_BuyCountSpin->value();
	if(buyCount >= 0 && (!m_MaxCount || buyCount <= (int)m_MaxCount))
		m_BuyItem->SetCount(buyCount);

	// Offer Commodity
	AuOfferItem offerItem = m_OfferCombo->itemData(m_OfferCombo->currentIndex()).value<AuOfferItem>();
	if(!offerItem.m_Item)
		return;
	if(m_OfferItem)
		delete m_OfferItem;
	m_OfferItem = offerItem.m_Item->Clone();
	if(!m_OfferItem)
		return;
	m_OfferEmpireOfOrigin = offerItem.m_EmpireOfOrigin;

	// Price offered.
	m_OfferItem->SetCount(m_OfferCountSpin->value());

	// Must have all of these things to accept.
	if(m_BuyItem->Count())
		accept();
}

void AuCommBuy::QuantityToBuyChanged(int buyCount)
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

	m_OfferCountSpin->setValue(offerCount);
}
