//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auservicemisc.cpp,v 1.5 2022/02/19 18:16:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auservicemisc.h"
//
#include "auapp.h"
#include "aucommchannel.h"
#include "auframe.h"
#include "auservicedialog.h"
#include "auview.h"
#include "daperson.h"
#include "davehicle.h"
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVariant>

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuServiceMisc::AuServiceMisc(AuServiceDialog* parent) : QWidget(parent)
{
	QLabel* lbinfo = new QLabel(tr("<html><p>"
				"Fuel is provided by all cities. "
				"Exterminator and house keeping services require a city with an airport terminal or repair shop. "
				"</p>"
				"<p>"
				"Exterminator removes live rats from a spacecraft. "
				"It also removes rat nests, which clears the current rat DNA of the spacecraft. "
				"The next time rats are encountered aboard the spacecraft, their DNA will be different. "
				"</p>"
				"<p>"
				"House keeper cleans up food items in the spacecraft that are edible to rats. "
				"Food items on static display are not cleaned up; they do not attract rats. "
				"</p></html>"), this);
	lbinfo->setAlignment(Qt::AlignLeft|Qt::AlignTop);
	lbinfo->setWordWrap(true);

	QPushButton* pbrefuel = new QPushButton(tr("Request Fuel"));
	pbrefuel->setIcon(QIcon(":data/MsgSolicitFuel.png"));
	pbrefuel->setToolTip(tr("<html><b>Request Fuel</b><br>"
				"Requests fuel for the spacecraft or vehicle you are aboard. "
				"Fuel is purchased like cargo. "
				"Cities respond with an offer to sell fuel. "
				"<br>"
				"This button is a shortcut to using the <b>Ask For Sales</b> button. "
				"<br>"
				"%2 Fuel is free to fleet spacecraft."
				"<br>"
				"%2 Fuel must be purchased for company spacecraft."
				"<br>"
				"%2 Vehicles get free fuel and repairs when on road slabs, near space stations, and in vehicle bays of spacecraft. "
				"Default key to request service is <b>K</b>; check vehicle control settings. "
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(pbrefuel, SIGNAL(clicked()), SLOT(RequestFuel()));

	QPushButton* pbexterminate = new QPushButton(tr("Request Exterminator"));
	pbexterminate->setIcon(QIcon(":data/Rat.png"));
	pbexterminate->setToolTip(tr("<html><b>Request Exterminator</b><br>"
				"Request removal of rats from the spacecraft.<br>"
				"%2 Live rats are removed.<br>"
				"%2 Rat nest is removed, clearing spacecraft's current rat DNA.<br>"
				"%2 Exterminator is free to fleet spacecraft.<br>"
				"%2 Exterminator costs %1 to company spacecraft."
				"<br>"
				"%2 City must have an airport terminal or repair shop. "
				"</html>")
			.arg(QString("100%1").arg(g_Au.CurrencySymbol()))
			.arg(g_Au.MiddleDotSymbol()));
	connect(pbexterminate, SIGNAL(clicked()), SLOT(RequestExterminator()));

	QPushButton* pbcleanhouse = new QPushButton(tr("Request House Keeping"));
	pbcleanhouse->setIcon(QIcon(":data/Sweep.png"));
	pbcleanhouse->setToolTip(tr("<html><b>Request House Keeping</b><br>"
				"Request clean up of debris from the spacecraft.<br>"
				"%2 Food edible to scavengers is cleaned up, if not on static display.<br>"
				"%2 House keeping is free to fleet spacecraft.<br>"
				"%2 House keeping costs %1 to company spacecraft."
				"<br>"
				"%2 City must have an airport terminal or repair shop. "
				"</html>")
			.arg(QString("100%1").arg(g_Au.CurrencySymbol()))
			.arg(g_Au.MiddleDotSymbol()));
	connect(pbcleanhouse, SIGNAL(clicked()), SLOT(RequestHouseKeeping()));

	QVBoxLayout* blay = new QVBoxLayout();
	blay->addWidget(pbrefuel);
	blay->addWidget(pbexterminate);
	blay->addWidget(pbcleanhouse);
	blay->addStretch();

	QHBoxLayout* lay = new QHBoxLayout();
	lay->addLayout(blay);
	lay->addWidget(lbinfo);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

void AuServiceMisc::RequestExterminator()
{
	AuServiceDialog* svcdlg = dynamic_cast<AuServiceDialog*>(parent());
	if(!svcdlg)
		return;
	AuCommChannel* channel = svcdlg->Channel();
	if(!channel)
		return;
	const AuView* view = auApp->Frame()->View();
	if(!view)
		return;
	if(channel->objectName() == AuCommStdChannel::TradeChannelName())
	{
		QMessageBox mb(g_Au.ApplicationName(),
				tr("Confirm request for exterminator service on the <b>Trade</b> channel. "
					"Every city that receives this message will provide the service, which could be costly. "),
				QMessageBox::Information, QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton, auApp->Frame());
		if(auApp->Frame()->ExecMessageBox(mb) != QMessageBox::Yes)
			return;
	}

	// Send a message requesting an exterminator.
	AuCommRequestExterminator msg(channel->objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
			view->SceneGalaxy(), channel->Range(), channel->Scope());
	channel->SendChat(msg);

	// Close the parent dialog.
	svcdlg->accept();
}

void AuServiceMisc::RequestHouseKeeping()
{
	AuServiceDialog* svcdlg = dynamic_cast<AuServiceDialog*>(parent());
	if(!svcdlg)
		return;
	AuCommChannel* channel = svcdlg->Channel();
	if(!channel)
		return;
	const AuView* view = auApp->Frame()->View();
	if(!view)
		return;
	if(channel->objectName() == AuCommStdChannel::TradeChannelName())
	{
		QMessageBox mb(g_Au.ApplicationName(),
				tr("Confirm request for house keeping service on the <b>Trade</b> channel. "
					"Every city that receives this message will provide the service, which could be costly. "),
				QMessageBox::Information, QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton, auApp->Frame());
		if(auApp->Frame()->ExecMessageBox(mb) != QMessageBox::Yes)
			return;
	}

	// Send a message requesting an exterminator.
	AuCommRequestHouseKeeping msg(channel->objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
			view->SceneGalaxy(), channel->Range(), channel->Scope());
	channel->SendChat(msg);

	// Close the parent dialog.
	svcdlg->accept();
}

void AuServiceMisc::RequestFuel()
{
	AuServiceDialog* svcdlg = dynamic_cast<AuServiceDialog*>(parent());
	if(!svcdlg)
		return;
	AuCommChannel* channel = svcdlg->Channel();
	if(!channel)
		return;
	AuView* view = auApp->Frame()->View();
	if(!view)
		return;
	COMMODITYID cidFuel = CID_Invalid;
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
			cidFuel = ship->FuelType();
		else
		{
			DaVehicle* vehicle = dynamic_cast<DaVehicle*>(person->Parent());
			if(vehicle)
				cidFuel = vehicle->FuelType();
		}
	}
	view->UnlockSceneAtom();

	// Send a message requesting fuel.
	if(cidFuel == CID_Invalid)
	{
		// Make an unspecific request for all kinds of fuel.
		AuCommSolicitFuelSales msg(channel->objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
				view->SceneGalaxy(), channel->Range(), channel->Scope());
		channel->SendChat(msg);
	}
	else
	{
		// Make a request for the specific kind of fuel.
		AuCommSolicitItemSales msg(cidFuel, channel->objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
				view->SceneGalaxy(), channel->Range(), channel->Scope());
		channel->SendChat(msg);
	}

	// Close the parent dialog.
	svcdlg->accept();
}
