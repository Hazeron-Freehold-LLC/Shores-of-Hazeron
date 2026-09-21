//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldbroker.cpp,v 1.19 2022/05/04 23:36:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubldbroker.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "aushipmentcreate.h"
#include "auview.h"
#include "dabd.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QLabel>
#include <QMessageBox>
#include <QListWidget>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBldBroker::AuBldBroker(QWidget* parent) : QWidget(parent)
{
	QLabel* lbicon = new QLabel(this);
	lbicon->setPixmap(QPixmap(":data/c_Container.png"));
	lbicon->setAlignment(Qt::AlignLeft|Qt::AlignTop);
	m_ShipmentOffices = 0;
	m_TradeOffices = 0;

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"Brokers transfer goods between cities, using <i>trade agreements</i> and <i>shipments</i>. "
				"The number of offices at a broker limits the shipments and trade agreements it can process. "
				"Shipments can be sent anywhere in the universe. "
				"<b>A broker must have airport access to process shipments.</b> "
				"</p>"
				"<p>"
				"Trade agreements are negotiated with a broker in the destination city. "
				"Shipments are coordinated with the other broker, to insure fair trade. "
				"</p>"
				"<p>"
				"One-way shipments may be sent. "
				"No coordination is needed with the destination city or base; "
				"a broker is not needed there. "
				"</p>"
				"<p>"
				"You must have authority to <i>change broker shipments and negotiate trade agreements</i> to make changes here or "
				"to see any information about shipments. "
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	m_Shipments = new QListWidget(this);
	m_Shipments->setSpacing(2);
	connect(m_Shipments, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
			SLOT(ShipmentChanged(QListWidgetItem*, QListWidgetItem*)));

	m_ShipmentsEnroute = new QListWidget(this);
	m_ShipmentsEnroute->setSpacing(2);
	m_ShipmentsEnroute->setSelectionMode(QAbstractItemView::NoSelection);

	m_TradeAgreements = new QListWidget(this);
	m_TradeAgreements->setSpacing(2);
	connect(m_TradeAgreements, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
			SLOT(TradeAgreementChanged(QListWidgetItem*, QListWidgetItem*)));

	m_CreateShipment = new QPushButton(tr("Schedule Shipment..."), this);
	m_CreateShipment->setToolTip(tr("<html>"
				"<p>"
				"Shipments are filled every six minutes from the city stockpile. "
				"When completely filled, shipments wait to be picked up. "
				"</p>"
				"<p>"
				"Shipments awaiting pick up are shipped hourly. "
				"When this broker has the capacity to handle it, the waiting order is shipped by barge. "
				"Barge shipments are shown in the list of shipments en route. "
				"</p>"
				"<p>"
				"Shipments awaiting pick up can be picked up by avatars aboard spacecraft. "
				"This enables avatars to fill repeating shipments more often. "
				"Bonuses are paid for fast delivery. "
				"</p>"
				"<p>"
				"Shipments under a trade agreement cannot be picked up by avatars. "
				"</p>"
				"<p>"
				"Delivery distance to destinations in the same galaxy is the straight line distance between them. "
				"Delivery distance to destinations in other galaxies is the straight line distance to the center of the "
				"current galaxy, then from the center of the other galaxy to the destination city. "
				"Delivery time is based on traveling the distance at 5x lightspeed. "
				"</p>"
				"</html>"));
	connect(m_CreateShipment, SIGNAL(clicked()), SLOT(CreateShipment()));

	m_DeleteShipment = new QPushButton(tr("Delete Shipment..."), this);
	m_DeleteShipment->setEnabled(false);
	m_DeleteShipment->setToolTip(tr("<html>"
				"Delete the selected shipment. "
				"Shipments cannot be deleted when en route or under a trade agreement. "
				"</html>"));
	connect(m_DeleteShipment, SIGNAL(clicked()), SLOT(DeleteShipment()));

	m_CreateTradeAgreement = new QPushButton(tr("Propose Trade..."), this);
	m_CreateTradeAgreement->setToolTip(tr("<html>"
				"Propose a new trade agreement to an outsystem city with a broker. "
				"<br>"
				"If the receiving city is not in your chain of command, someone at the receiving broker must accept the trade "
				"proposal to begin. "
				"</html>"));
	connect(m_CreateTradeAgreement, SIGNAL(clicked()), SLOT(CreateTradeAgreement()));

	m_AcceptTradeAgreement = new QPushButton(tr("Accept Proposal..."), this);
	m_AcceptTradeAgreement->setEnabled(false);
	m_AcceptTradeAgreement->setToolTip(tr("<html>"
				"<p>"
				"Accepts the selected trade proposal. "
				"</p>"
				"<p>"
				"The corresponding proposal at the other city broker will indicate <i>Agreement</i>. "
				"Trading according to the agreement will proceed to its conclusion or until the agreement is deleted. "
				"</p>"
				"</html>"));
	connect(m_AcceptTradeAgreement, SIGNAL(clicked()), SLOT(AcceptTradeAgreement()));

	m_CounterTradeAgreement = new QPushButton(tr("Counter Proposal..."), this);
	m_CounterTradeAgreement->setEnabled(false);
	m_CounterTradeAgreement->setToolTip(tr("<html>"
				"<p>"
				"Proposes a counter-offer to the selected trade proposal. "
				"</p>"
				"<p>"
				"The corresponding proposal at the other city broker will indicate <i>Countered</i>. "
				"The counter proposal appears as a new trade proposal from you. "
				"</p>"
				"</html>"));
	connect(m_CounterTradeAgreement, SIGNAL(clicked()), SLOT(CounterTradeAgreement()));

	m_DeleteTradeAgreement = new QPushButton(tr("Delete Trade..."), this);
	m_DeleteTradeAgreement->setEnabled(false);
	m_DeleteTradeAgreement->setToolTip(tr("<html>"
				"<p>"
				"Deletes a trade proposal or agreement. "
				"</p>"
				"<p>"
				"The other city broker is notified to update their trade status. "
				"Trade proposals will indicate <i>Declined</i>. "
				"Trade agreements will indicate <i>Terminated</i>. "
				"</p>"
				"</html>"));
	connect(m_DeleteTradeAgreement, SIGNAL(clicked()), SLOT(DeleteTradeAgreement()));

	m_RefreshShipments = new QPushButton(tr("Refresh"), this);
	m_RefreshShipments->setToolTip(tr("<html>"
				"Refreshes the lists of trade agreements and shipments. "
				"</html>"));
	connect(m_RefreshShipments, SIGNAL(clicked()), SLOT(RefreshShipments()));

	QVBoxLayout* tblay = new QVBoxLayout;
	tblay->addWidget(m_CreateTradeAgreement);
	tblay->addWidget(m_AcceptTradeAgreement);
	tblay->addWidget(m_CounterTradeAgreement);
	tblay->addWidget(m_DeleteTradeAgreement);
	tblay->addStretch();

	QHBoxLayout* talay = new QHBoxLayout;
	talay->addWidget(m_TradeAgreements);
	talay->addLayout(tblay);

	QWidget* twid = new QWidget(this);
	twid->setLayout(talay);

	QVBoxLayout* sblay = new QVBoxLayout;
	sblay->addWidget(m_CreateShipment);
	sblay->addWidget(m_DeleteShipment);
	sblay->addStretch();

	QHBoxLayout* salay = new QHBoxLayout;
	salay->addWidget(m_Shipments);
	salay->addLayout(sblay);

	QWidget* swid = new QWidget(this);
	swid->setLayout(salay);

	QVBoxLayout* eblay = new QVBoxLayout;
	eblay->addStretch();
	eblay->addWidget(m_RefreshShipments);

	QHBoxLayout* ealay = new QHBoxLayout;
	ealay->addWidget(m_ShipmentsEnroute);
	ealay->addLayout(eblay);

	QWidget* ewid = new QWidget(this);
	ewid->setLayout(ealay);

	m_Splitter = new QSplitter;
	m_Splitter->setChildrenCollapsible(false);
	m_Splitter->setOrientation(Qt::Vertical);
	m_Splitter->addWidget(twid);
	m_Splitter->addWidget(swid);
	m_Splitter->addWidget(ewid);
	m_Splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	QHBoxLayout* llay = new QHBoxLayout;
	llay->addWidget(lbicon);
	llay->addWidget(lbinfo, 1);
	llay->setMargin(0);
	llay->setSpacing(2);

	// Build the layout.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(llay);
	lay->addWidget(m_Splitter, 10);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);

	QByteArray splitterstate;
	g_Au.Settings().readEntry("Widget/BldBroker/Splitter", splitterstate);
	m_Splitter->restoreState(splitterstate);
}

AuBldBroker::~AuBldBroker()
{
}

void AuBldBroker::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Show all my widgets.
	QVBoxLayout* lay = (QVBoxLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->show();

	RequestShipments(false);
}

void AuBldBroker::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Hide my widget now or they force the parent AuBld dialog to be too big even though this page isn't visible.
	QVBoxLayout* lay = (QVBoxLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->hide();
}

void AuBldBroker::CreateShipment()
{
	AuFrame* frame = auApp->Frame();
	AuView* view = frame->View();

	AuId shipperId, shipperSystem;
	quint32 shipperEmpire = 0;
	GALAXY galaxy = GALAXY_Count;
	AuPoint systemLoc;
	{
		DaPerson* person;
		view->LockSceneAtom(&person);
		if(person)
		{
			const DaBd* bd = person->BdOp(m_CapitolOpId);
			if(bd && bd->IndustryId() == IID_Broker)
			{
				galaxy = person->Galaxy();
				shipperId = bd->CapitolId();
				shipperEmpire = bd->EmpireId();
				const DaSolarSystem* system = person->FindParentSolarSystem();
				if(system)
				{
					shipperSystem = system->Id();
					const DaGalaxy* gal = system->FindParentGalaxy();
					if(gal)
						system->GetLocationRelativeToParent(systemLoc, gal);
				}
			}
		}
		view->UnlockSceneAtom();
	}
	if(shipperId.IsNull())
		return;

	AuShipmentCreate dlg(shipperId, shipperEmpire, false, shipperSystem, galaxy, systemLoc, this);
	if(frame->ExecDialog(dlg) != QDialog::Accepted)
		return;

	int sendWhat = dlg.SendWhat();
	COMMODITYFILTER cat = dlg.Category();
	COMMODITYID cid = dlg.CommodityId();
	quint32 amount = dlg.Amount();
	quint8 minQ = dlg.MinQ();
	quint8 repeat = dlg.Repeat();
	AuId destCityId = dlg.DestCityId();
	QString destDesc = dlg.DestDesc();
	float destDistPc = dlg.DestDistPc();
	AuId destSysId = dlg.DestSystemId();
	AuId destSectorId = dlg.DestSectorId();

	AuShipment shipment;
	if(sendWhat == 0)
		shipment = AuShipment(cid, amount, minQ, repeat, destCityId, destDesc, destDistPc, destSysId, destSectorId, shipperEmpire);
	else
		shipment = AuShipment(cat, amount, minQ, repeat, destCityId, destDesc, destDistPc, destSysId, destSectorId, shipperEmpire);

	auApp->BrokerCreateShipment(shipment, m_CapitolOpId);
}

void AuBldBroker::DeleteShipment()
{
	QListWidgetItem* item = m_Shipments->currentItem();
	if(!item)
		return;
	AuShipment shipment = item->data(Qt::UserRole).value<AuShipment>();
	if(shipment.TradeAgreement())
		return;

	QString smsg;
	if(shipment.CommodityId() != CID_Invalid)
		smsg = tr("<html>"
				"Confirm delete of order for %L1 %2. "
				"</html>").arg(shipment.Amount()).arg(g_Au.CommodityName(shipment.CommodityId()));
	else
		smsg = tr("<html>"
				"Confirm delete of order for %L1 %2. "
				"</html>").arg(shipment.Amount()).arg(CommodityFilterName(shipment.CommodityFilter()));
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	auApp->BrokerDeleteShipment(shipment, m_CapitolOpId);
}

void AuBldBroker::AcceptTradeAgreement()
{
	QListWidgetItem* item = m_TradeAgreements->currentItem();
	if(!item)
		return;
	AuTradeAgreement ta = item->data(Qt::UserRole).value<AuTradeAgreement>();

	QString smsg;
	if(ta.SendCommodityId() != CID_Invalid && ta.GetCommodityId() != CID_Invalid)
		smsg = tr("<html>"
				"Confirm acceptance of proposal to trade %L1 %2 for %L3 %4. "
				"</html>")
			.arg(ta.SendAmount()).arg(g_Au.CommodityName(ta.SendCommodityId()))
			.arg(ta.GetAmount()).arg(g_Au.CommodityName(ta.GetCommodityId()));
	else if(ta.SendCommodityId() != CID_Invalid && ta.GetCommodityId() == CID_Invalid)
		smsg = tr("<html>"
				"Confirm acceptance of proposal to trade %L1 %2 for %L3 %4. "
				"</html>")
			.arg(ta.SendAmount()).arg(g_Au.CommodityName(ta.SendCommodityId()))
			.arg(ta.GetAmount()).arg(CommodityFilterName(ta.GetCommodityFilter()));
	else if(ta.SendCommodityId() == CID_Invalid && ta.GetCommodityId() != CID_Invalid)
		smsg = tr("<html>"
				"Confirm acceptance of proposal to trade %L1 %2 for %L3 %4. "
				"</html>")
			.arg(ta.SendAmount()).arg(CommodityFilterName(ta.SendCommodityFilter()))
			.arg(ta.GetAmount()).arg(g_Au.CommodityName(ta.GetCommodityId()));
	else
		smsg = tr("<html>"
				"Confirm acceptance of proposal to trade %L1 %2 for %L3 %4. "
				"</html>")
			.arg(ta.SendAmount()).arg(CommodityFilterName(ta.SendCommodityFilter()))
			.arg(ta.GetAmount()).arg(CommodityFilterName(ta.GetCommodityFilter()));
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	auApp->BrokerAcceptTradeAgreement(ta, m_CapitolOpId);
}

void AuBldBroker::CounterTradeAgreement()
{
	QListWidgetItem* item = m_TradeAgreements->currentItem();
	if(!item)
		return;
	AuTradeAgreement ta = item->data(Qt::UserRole).value<AuTradeAgreement>();
	if(!ta.IsProposal())
		return;

	AuFrame* frame = auApp->Frame();
	AuView* view = frame->View();

	AuId shipperId, shipperSystem, shipperSector;
	quint32 shipperEmpire = 0;
	GALAXY galaxy = GALAXY_Count;
	QString shipperDesc;
	AuPoint systemLoc;
	{
		DaPerson* person;
		view->LockSceneAtom(&person);
		if(person)
		{
			const DaBd* bd = person->BdOp(m_CapitolOpId);
			if(bd && bd->IndustryId() == IID_Broker)
			{
				galaxy = person->Galaxy();
				shipperId = bd->CapitolId();
				shipperEmpire = bd->EmpireId();
				const DaWorldLL* world = bd->FindParentWorldInEnv();
				const DaSolarSystem* system = person->FindParentSolarSystem();
				if(world && system)
				{
					shipperSystem = system->Id();
					const DaSector* sector = system->FindParentSector();
					if(sector)
						shipperSector = sector->Id();
					const DaGalaxy* gal = system->FindParentGalaxy();
					if(gal && sector)
					{
						system->GetLocationRelativeToParent(systemLoc, gal);
						shipperDesc = tr("%1 on %2, %3 system, %4 sector, %5")
							.arg(bd->CityName())
							.arg(world->Name())
							.arg(system->Name())
							.arg(sector->SectorName())
							.arg(gal->Name());
					}
				}
			}
		}
		view->UnlockSceneAtom();
	}
	if(shipperId.IsNull())
		return;
	if(shipperId != ta.DestinationCity())
		return;	// Only the destination city can counter the offer.

	// Prepare a counter offer.
	AuTradeAgreementCreate dlg(ta, shipperId, shipperEmpire, shipperSystem, galaxy, systemLoc, this);
	if(frame->ExecDialog(dlg) != QDialog::Accepted)
		return;

	quint8 repeat = dlg.Repeat();
	AuId destCityId = dlg.DestCityId();
	QString destDesc = dlg.DestDesc();
	float destDistPc = dlg.DestDistPc();
	AuId destSysId = dlg.DestSystemId();
	AuId destSectorId = dlg.DestSectorId();

	int getWhat = dlg.GetWhat();
	COMMODITYFILTER getcat = dlg.GetCategory();
	COMMODITYID getcid = dlg.GetCommodityId();
	quint32 getamount = dlg.GetAmount();
	quint8 getminQ = dlg.GetMinQ();

	int sendWhat = dlg.SendWhat();
	COMMODITYFILTER sendcat = dlg.SendCategory();
	COMMODITYID sendcid = dlg.SendCommodityId();
	quint32 sendamount = dlg.SendAmount();
	quint8 sendminQ = dlg.SendMinQ();

	AuTradeAgreement tac;
	if(sendWhat == 0 && getWhat == 0)
	{
		// cid for cid
		tac = AuTradeAgreement(sendcid, sendamount, sendminQ, shipperId, shipperDesc, shipperSystem, shipperSector,
				getcid, getamount, getminQ, destCityId, destDesc, destSysId, destSectorId, repeat, destDistPc);
	}
	else if(sendWhat == 0 && getWhat == 1)
	{
		// cid for cat
		tac = AuTradeAgreement(sendcid, sendamount, sendminQ, shipperId, shipperDesc, shipperSystem, shipperSector,
				getcat, getamount, getminQ, destCityId, destDesc, destSysId, destSectorId, repeat, destDistPc);
	}
	else if(sendWhat == 1 && getWhat == 0)
	{
		// cat for cid
		tac = AuTradeAgreement(sendcat, sendamount, sendminQ, shipperId, shipperDesc, shipperSystem, shipperSector,
				getcid, getamount, getminQ, destCityId, destDesc, destSysId, destSectorId, repeat, destDistPc);
	}
	else
	{
		// cat for cat
		tac = AuTradeAgreement(sendcat, sendamount, sendminQ, shipperId, shipperDesc, shipperSystem, shipperSector,
				getcat, getamount, getminQ, destCityId, destDesc, destSysId, destSectorId, repeat, destDistPc);
	}

	auApp->BrokerCounterTradeAgreement(ta, tac, m_CapitolOpId);
}

void AuBldBroker::DeleteTradeAgreement()
{
	QListWidgetItem* item = m_TradeAgreements->currentItem();
	if(!item)
		return;
	AuTradeAgreement ta = item->data(Qt::UserRole).value<AuTradeAgreement>();

	QString smsg;
	if(ta.SendCommodityId() != CID_Invalid && ta.GetCommodityId() != CID_Invalid)
		smsg = tr("<html>"
				"Confirm delete of proposal to trade %L1 %2 for %L3 %4. "
				"</html>")
			.arg(ta.SendAmount()).arg(g_Au.CommodityName(ta.SendCommodityId()))
			.arg(ta.GetAmount()).arg(g_Au.CommodityName(ta.GetCommodityId()));
	else if(ta.SendCommodityId() != CID_Invalid && ta.GetCommodityId() == CID_Invalid)
		smsg = tr("<html>"
				"Confirm delete of proposal to trade %L1 %2 for %L3 %4. "
				"</html>")
			.arg(ta.SendAmount()).arg(g_Au.CommodityName(ta.SendCommodityId()))
			.arg(ta.GetAmount()).arg(CommodityFilterName(ta.GetCommodityFilter()));
	else if(ta.SendCommodityId() == CID_Invalid && ta.GetCommodityId() != CID_Invalid)
		smsg = tr("<html>"
				"Confirm delete of proposal to trade %L1 %2 for %L3 %4. "
				"</html>")
			.arg(ta.SendAmount()).arg(CommodityFilterName(ta.SendCommodityFilter()))
			.arg(ta.GetAmount()).arg(g_Au.CommodityName(ta.GetCommodityId()));
	else
		smsg = tr("<html>"
				"Confirm delete of proposal to trade %L1 %2 for %L3 %4. "
				"</html>")
			.arg(ta.SendAmount()).arg(CommodityFilterName(ta.SendCommodityFilter()))
			.arg(ta.GetAmount()).arg(CommodityFilterName(ta.GetCommodityFilter()));
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	auApp->BrokerDeleteTradeAgreement(ta, m_CapitolOpId);
}

void AuBldBroker::RefreshShipments()
{
	RequestShipments(true);
}

void AuBldBroker::RequestShipments(bool sayResult)
{
	auApp->BrokerRequestShipments(sayResult, m_CapitolOpId);
}

void AuBldBroker::Shipment(AuShipment shipment)
{
	QString text = shipment.ListDesc();
	if(shipment.NeedsOffice())
		m_ShipmentOffices++;

	QListWidgetItem* lwi;
	if(shipment.IsEnroute())
	{
		QPixmap px(":data/c_Container.png");
		QIcon icon(px.scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation));

		lwi = new QListWidgetItem(icon, text, m_ShipmentsEnroute);
	}
	else
	{
		QPixmap px;
		if(shipment.CommodityId() != CID_Invalid)
			px = QPixmap(g_Au.CommodityBitmapName(shipment.CommodityId()));
		else
			px = QPixmap(CommodityFilterIcon(shipment.CommodityFilter()));
		QIcon icon(px.scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation));

		lwi = new QListWidgetItem(icon, text, m_Shipments);
	}
	lwi->setData(Qt::UserRole, QVariant::fromValue(shipment));
	lwi->setToolTip(shipment.HtmlToolTip());
	EnableControls();
}

void AuBldBroker::ShipmentsClear()
{
	m_Shipments->clear();
	m_ShipmentsEnroute->clear();
	m_ShipmentOffices = 0;
	EnableControls();
}

void AuBldBroker::ShipmentChanged(QListWidgetItem*, QListWidgetItem*)
{
	EnableControls();
}

void AuBldBroker::EnableControls()
{
	// Shipments that are associated with a trade agreement cannot be deleted.
	bool delOk = false;
	QListWidgetItem* item = m_Shipments->currentItem();
	if(item)
	{
		AuShipment shipment = item->data(Qt::UserRole).value<AuShipment>();
		if(!shipment.TradeAgreement())
			delOk = true;
	}
	m_DeleteShipment->setEnabled(delOk);

	// Lock the root and find the scene atom.
	int brokerLevels = 0;
	AuView* view = auApp->Frame()->View();
	if(!view)
		return;
	AuId cityId;
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		DaBd* bd = person->BdOp(m_CapitolOpId);
		if(bd && bd->IndustryId() == IID_Broker)
		{
			brokerLevels = bd->Offices();
			cityId = bd->CapitolId();
		}
	}
	view->UnlockSceneAtom();
	bool enableCreate = (m_ShipmentOffices + m_TradeOffices) < brokerLevels;
	m_CreateShipment->setEnabled(enableCreate);
	m_CreateTradeAgreement->setEnabled(enableCreate);

	item = m_TradeAgreements->currentItem();
	m_DeleteTradeAgreement->setEnabled(item);
	if(item)
	{
		AuTradeAgreement ta = item->data(Qt::UserRole).value<AuTradeAgreement>();
		m_AcceptTradeAgreement->setEnabled(ta.IsProposal() && cityId == ta.DestinationCity());
		m_CounterTradeAgreement->setEnabled(ta.IsProposal() && cityId == ta.DestinationCity());
	}
	else
	{
		m_AcceptTradeAgreement->setEnabled(false);
		m_CounterTradeAgreement->setEnabled(false);
	}
}

void AuBldBroker::SplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/BldBroker/Splitter", m_Splitter->saveState());
}

void AuBldBroker::CreateTradeAgreement()
{
	AuFrame* frame = auApp->Frame();
	AuView* view = frame->View();

	AuId personId;
	AuId shipperId, shipperSystem, shipperSector;
	quint32 shipperEmpire = 0;
	GALAXY galaxy = GALAXY_Count;
	QString shipperDesc;
	AuPoint systemLoc;
	{
		DaPerson* person;
		view->LockSceneAtom(&person);
		if(person)
		{
			personId = person->Id();
			const DaBd* bd = person->BdOp(m_CapitolOpId);
			if(bd && bd->IndustryId() == IID_Broker)
			{
				galaxy = person->Galaxy();
				shipperId = bd->CapitolId();
				shipperEmpire = bd->EmpireId();
				const DaWorldLL* world = bd->FindParentWorldInEnv();
				const DaSolarSystem* system = person->FindParentSolarSystem();
				if(world && system)
				{
					shipperSystem = system->Id();
					const DaSector* sector = system->FindParentSector();
					if(sector)
						shipperSector = sector->Id();
					const DaGalaxy* gal = system->FindParentGalaxy();
					if(gal && sector)
					{
						system->GetLocationRelativeToParent(systemLoc, gal);
						shipperDesc = tr("%1 on %2, %3 system, %4 sector, %5")
							.arg(bd->CityName())
							.arg(world->Name())
							.arg(system->Name())
							.arg(sector->SectorName())
							.arg(gal->Name());
					}
				}
			}
		}
		view->UnlockSceneAtom();
	}
	if(shipperId.IsNull())
		return;

	AuTradeAgreementCreate dlg(shipperId, shipperEmpire, shipperSystem, galaxy, systemLoc, this);
	if(frame->ExecDialog(dlg) != QDialog::Accepted)
		return;

	quint8 repeat = dlg.Repeat();
	AuId destCityId = dlg.DestCityId();
	QString destDesc = dlg.DestDesc();
	float destDistPc = dlg.DestDistPc();
	AuId destSysId = dlg.DestSystemId();
	AuId destSectorId = dlg.DestSectorId();

	int getWhat = dlg.GetWhat();
	COMMODITYFILTER getcat = dlg.GetCategory();
	COMMODITYID getcid = dlg.GetCommodityId();
	quint32 getamount = dlg.GetAmount();
	quint8 getminQ = dlg.GetMinQ();

	int sendWhat = dlg.SendWhat();
	COMMODITYFILTER sendcat = dlg.SendCategory();
	COMMODITYID sendcid = dlg.SendCommodityId();
	quint32 sendamount = dlg.SendAmount();
	quint8 sendminQ = dlg.SendMinQ();

	AuTradeAgreement ta;
	if(sendWhat == 0 && getWhat == 0)
	{
		// cid for cid
		ta = AuTradeAgreement(sendcid, sendamount, sendminQ, shipperId, shipperDesc, shipperSystem, shipperSector, 
				getcid, getamount, getminQ, destCityId, destDesc, destSysId, destSectorId, repeat, destDistPc);
	}
	else if(sendWhat == 0 && getWhat == 1)
	{
		// cid for cat
		ta = AuTradeAgreement(sendcid, sendamount, sendminQ, shipperId, shipperDesc, shipperSystem, shipperSector, 
				getcat, getamount, getminQ, destCityId, destDesc, destSysId, destSectorId, repeat, destDistPc);
	}
	else if(sendWhat == 1 && getWhat == 0)
	{
		// cat for cid
		ta = AuTradeAgreement(sendcat, sendamount, sendminQ, shipperId, shipperDesc, shipperSystem, shipperSector, 
				getcid, getamount, getminQ, destCityId, destDesc, destSysId, destSectorId, repeat, destDistPc);
	}
	else
	{
		// cat for cat
		ta = AuTradeAgreement(sendcat, sendamount, sendminQ, shipperId, shipperDesc, shipperSystem, shipperSector, 
				getcat, getamount, getminQ, destCityId, destDesc, destSysId, destSectorId, repeat, destDistPc);
	}

	// If the destination city is in the person's chain of command, we don't have to wait for acceptance; we can just send the
	// final agreement.
	const DgData& gd = g_Au.LockDataForRead();
	bool inChain = gd.IsEmpireCityInDomain(shipperEmpire, destCityId, personId);
	g_Au.UnlockData();
	if(inChain)
		ta.Accept();

	auApp->BrokerCreateTradeAgreement(ta, m_CapitolOpId);
}

void AuBldBroker::TradeAgreement(AuTradeAgreement ta)
{
	QString text = ta.ListDesc();
	if(ta.NeedsOffice())
		m_TradeOffices++;

	QPixmap px(":data/CommTrade.png");
	QIcon icon(px.scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation));

	QListWidgetItem* lwi = new QListWidgetItem(icon, text, m_TradeAgreements);
	lwi->setData(Qt::UserRole, QVariant::fromValue(ta));
	lwi->setToolTip(ta.HtmlToolTip());
	EnableControls();
}

void AuBldBroker::TradeAgreementsClear()
{
	m_TradeAgreements->clear();
	m_TradeOffices = 0;
	EnableControls();
}

void AuBldBroker::TradeAgreementChanged(QListWidgetItem*, QListWidgetItem*)
{
	EnableControls();
}

