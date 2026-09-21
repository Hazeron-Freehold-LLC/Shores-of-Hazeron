//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspacecraftshipments.cpp,v 1.10 2022/05/04 23:36:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auspacecraftshipments.h"
//
#include "auchristendialog.h"
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "aushipmentcreate.h"
#include "auview.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dgdata.h"
#include <QLabel>
#include <QMessageBox>
#include <QListWidget>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

void AuSpacecraftShipmentsDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftShipmentsUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSpacecraftShipments::AuSpacecraftShipments(QWidget* parent) : QWidget(parent)
{
	QLabel* lbicon = new QLabel(this);
	lbicon->setPixmap(QPixmap(":data/c_Container.png"));
	lbicon->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"Space stations can arrange transport for shipments of cargo to cities and bases anywhere in the universe. "
				"</p>"
				"<p>"
				"This enables a station harvesting natural resources to send accumulated goods to a city. "
				"Go to a broker to create shipments between cities. "
				"</p>"
				"<p>"
				"You must be aboard a space station to make changes here. "
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	m_Shipments = new QListWidget(this);
	m_Shipments->setSpacing(2);
	m_Shipments->setToolTip(tr("<html>"
				"<p>"
				"Shipments are filled every six minutes from the hold. "
				"When completely filled, shipments wait to be picked up. "
				"</p>"
				"<p>"
				"Shipments awaiting transport are shipped hourly. "
				"When a space station has the capacity to handle it, the waiting order is shipped by barge. "
				"Barge shipments are shown in the list of shipments that are en route. "
				"</p>"
				"<p>"
				"Shipments awaiting transport can be picked up at any time by spacecraft operated by avatars. "
				"This enables repeating shipments to be filled more frequently. "
				"Bonuses are paid for fast delivery. "
				"</p>"
				"<p>"
				"Each space station can process two shipments and track two shipment en route. "
				"</p>"
				"</html>"));
	connect(m_Shipments, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
			SLOT(ShipmentChanged(QListWidgetItem*, QListWidgetItem*)));

	m_ShipmentsEnroute = new QListWidget(this);
	m_ShipmentsEnroute->setSpacing(2);
	m_ShipmentsEnroute->setSelectionMode(QAbstractItemView::NoSelection);
	m_ShipmentsEnroute->setToolTip(tr("<html>"
				"<p>"
				"Shipments en route to their destinations. "
				"</p>"
				"<p>"
				"Delivery distance to cities and bases in the same galaxy is the straight line distance between them. "
				"Delivery distance to cities and bases in other galaxies is the straight line distance to the center of the "
				"current galaxy, then from the center of the destination galaxy to the destination city. "
				"Delivery time is based on traveling the distance at 5x lightspeed. "
				"</p>"
				"<p>"
				"Each space station can process two shipments and track two shipment en route. "
				"</p>"
				"</html>"));

	m_CreateShipment = new QPushButton(tr("Schedule Shipment..."), this);
	m_CreateShipment->setToolTip(tr("<html>"
				"Schedule a new shipment. "
				"<br>"
				"You must be in the chain of command of this space station to create shipments. "
				"</html>"));
	connect(m_CreateShipment, SIGNAL(clicked()), SLOT(CreateShipment()));

	m_DeleteShipment = new QPushButton(tr("Delete Shipment..."), this);
	m_DeleteShipment->setEnabled(false);
	m_DeleteShipment->setToolTip(tr("<html>"
				"Delete a shipment that is being processed. "
				"Shipments that are en route cannot be deleted. "
				"<br>"
				"You must be in the chain of command of this space station to delete shipments. "
				"</html>"));
	connect(m_DeleteShipment, SIGNAL(clicked()), SLOT(DeleteShipment()));

	m_RefreshShipments = new QPushButton(tr("Refresh Shipments"), this);
	m_RefreshShipments->setToolTip(tr("<html>"
				"Refresh the list of scheduled shipments. "
				"<br>"
				"You must be in the chain of command of this space station to view shipments. "
				"</html>"));
	connect(m_RefreshShipments, SIGNAL(clicked()), SLOT(RefreshShipments()));

	m_Splitter = new QSplitter;
	m_Splitter->setOrientation(Qt::Vertical);
	m_Splitter->addWidget(m_Shipments);
	m_Splitter->addWidget(m_ShipmentsEnroute);
	m_Splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	QHBoxLayout* llay = new QHBoxLayout;
	llay->addWidget(lbicon);
	llay->addWidget(lbinfo, 1);
	llay->setMargin(0);
	llay->setSpacing(2);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(m_RefreshShipments);
	blay->addStretch();
	blay->addWidget(m_DeleteShipment);
	blay->addWidget(m_CreateShipment);
	blay->setMargin(0);
	blay->setSpacing(2);

	// Build the layout.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(llay);
	lay->addWidget(m_Splitter, 10);
	lay->addLayout(blay);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);

	QByteArray splitterstate;
	g_Au.Settings().readEntry("Widget/SpacecraftShipments/Splitter", splitterstate);
	m_Splitter->restoreState(splitterstate);
}

AuSpacecraftShipments::~AuSpacecraftShipments()
{
}

void AuSpacecraftShipments::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	EnableControls();
	RequestShipments(false);
}

void AuSpacecraftShipments::SplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/SpacecraftShipments/Splitter", m_Splitter->saveState());
}

void AuSpacecraftShipments::CreateShipment()
{
	AuFrame* frame = auApp->Frame();
	AuView* view = frame->View();

	AuId shipperId;
	quint32 shipperEmpire = 0;
	GALAXY galaxy = GALAXY_Count;
	AuPoint systemLoc;
	{
		DaPerson* person;
		view->LockSceneAtom(&person);
		if(person)
		{
			const DaVessel* ves = person->FindParentVessel();
			if(ves && ves->IsSpaceStation())
			{
				galaxy = person->Galaxy();
				shipperId = ves->Id();
				shipperEmpire = ves->EmpireId();
				const DaSolarSystem* system = person->FindParentSolarSystem();
				if(system)
				{
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

	AuShipmentCreate dlg(shipperId, shipperEmpire, true, AuId(0), galaxy, systemLoc, this);
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

	auApp->ShipCreateShipment(shipment);
}

void AuSpacecraftShipments::DeleteShipment()
{
	QListWidgetItem* item = m_Shipments->currentItem();
	if(!item)
		return;
	AuShipment shipment = item->data(Qt::UserRole).value<AuShipment>();

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

	auApp->ShipDeleteShipment(shipment);
}

void AuSpacecraftShipments::RefreshShipments()
{
	RequestShipments(true);
}

void AuSpacecraftShipments::RequestShipments(bool sayResult)
{
	auApp->ShipRequestShipments(sayResult);
}

void AuSpacecraftShipments::Shipment(AuShipment shipment)
{
	QString text = shipment.ListDesc();

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

void AuSpacecraftShipments::ShipmentsClear()
{
	m_Shipments->clear();
	m_ShipmentsEnroute->clear();
	EnableControls();
}

void AuSpacecraftShipments::ShipmentChanged(QListWidgetItem*, QListWidgetItem*)
{
	EnableControls();
}

void AuSpacecraftShipments::EnableControls()
{
	// Determine if the person is in the chain-of-command of the ship.
	AuView* view = auApp->Frame()->View();
	if(!view)
		return;

	bool enableDelete = false;
	QListWidgetItem* item = m_Shipments->currentItem();
	if(item)
		enableDelete = true;

	// Lock the root and find the scene atom.
	int brokerLevels = 2;
	bool onStation = false;
	bool inCommand = false;
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		const DaVessel* ves = person->FindParentVessel();
		if(ves)
		{
			onStation = ves->IsSpaceStation();
			inCommand = ves->InChainOfCommand(person);
		}
	}
	view->UnlockSceneAtom();
	bool enableCreate = m_Shipments->count() < brokerLevels;

	m_CreateShipment->setEnabled(enableCreate && onStation && inCommand);
	m_DeleteShipment->setEnabled(enableDelete && onStation && inCommand);
	m_CreateShipment->setEnabled(onStation && inCommand);
	m_RefreshShipments->setEnabled(onStation && inCommand);
}


QSize AuSpacecraftShipments::sizeHint() const
{
	return QSize(1000,800);
}
