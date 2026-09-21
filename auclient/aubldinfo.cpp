//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldinfo.cpp,v 1.9 2022/04/08 16:26:52 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubldinfo.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auview.h"
#include "dgdata.h"
#include <QTextBrowser>
#include <QVBoxLayout>

AuBldInfo::AuBldInfo(bool atCapitol, QWidget* parent) : QWidget(parent)
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	QTextBrowser* tb = new QTextBrowser(this);
	tb->setOpenExternalLinks(true);
	tb->setReadOnly(true);
	QString text;
	if(atCapitol)
		text = tr("<p>This window changes based on the selected building. "
				"</p>"

				"<p><i>Tips</i>"
				"<br>"
				"%1 Select a building that is under construction to help build it. "
				"<br>"
				"%1 Select a building that makes things to change what it makes and make more of them. "
				"<br>"
				"%1 Select other buildings to access custom services. "
				"Not all services can be accessed from a capitol building. "
				"</p>"

				"<p>"
				"These buildings offer custom services or settings. "
				"Many of their services can be accessed on tabs at this capitol building. "
				"Some services can only be accessed at the building site. "
				"</p>")
			.arg(g_Au.MiddleDotSymbol());
	else
		text = tr("<p>This window changes based on the building or road you occupy. "
				"You do not have to be inside a building; stand anywhere at the site.</p>"

				"<p><i>Tips</i>"
				"<br>"
				"%1 Go to a building or road that is under construction to help build it. "
				"<br>"
				"%1 Go to a building that makes things to change what it makes and make more of them. "
				"<br>"
				"%1 Stores buy and sell personal items. "
				"Most buildings can have a store, if their blueprint includes one. "
				"A <i>Store</i> tab appears if the building could have a store. "
				"</p>"

				"<p>"
				"These buildings display tabs to access custom services or settings. "
				"</p>")
			.arg(g_Au.MiddleDotSymbol());

	// Start a table of information about each industry that offers a service.
	text += QString("<table valign=\"middle\"><tbody>");

	const DgData& gd = g_Au.LockDataForRead();
	text += "<tr>";
	text += tr("<td><img src=\"qrc%1\"></td><td><b>%2</b><br>%3</td>")
		.arg(gd.Industry(IID_AirportRepairShop).BitmapName())
		.arg(gd.IndustryName(IID_AirportRepairShop))
		.arg(tr("Repairs spacecraft. Recovers fleet and company spacecraft after they are destroyed."));

	text += tr("<td><img src=\"qrc%1\"></td><td><b>%2</b><br>%3</td>")
		.arg(gd.Industry(IID_Arena).BitmapName())
		.arg(gd.IndustryName(IID_Arena))
		.arg(tr("Simulates solo and multiplayer spacecraft battles. "
					"Players can join the same match from any arena in the galaxy."));
	text += "</tr>";

	text += "<tr>";
	text += tr("<td><img src=\"qrc%1\"></td><td><b>%2</b><br>%3</td>")
		.arg(gd.Industry(IID_Bank).BitmapName())
		.arg(gd.IndustryName(IID_Bank))
		.arg(tr("Provides personal loans and other financial services."));

	text += tr("<td><img src=\"qrc%1\"></td><td><b>%2</b><br>%3</td>")
		.arg(gd.Industry(IID_Broker).BitmapName())
		.arg(gd.IndustryName(IID_Broker))
		.arg(tr("Buys and sells bulk cargo if you hail on the Trade channel. They also ship goods to outsystem cities."));
	text += "</tr>";

	text += "<tr>";
	text += tr("<td><img src=\"qrc%1\"></td><td><b>%2</b><br>%3</td>")
		.arg(gd.Industry(IID_Cantina).BitmapName())
		.arg(gd.IndustryName(IID_Cantina))
		.arg(tr("Sells prepared food and drinks."));

	text += tr("<td><img src=\"qrc%1\"></td><td><b>%2</b><br>%3</td>")
		.arg(gd.Industry(IID_Capitol).BitmapName())
		.arg(gd.IndustryName(IID_Capitol))
		.arg(tr("Manages all construction, manufacturing, spacecraft factories, "
					"storage, and most building services in a city."));
	text += "</tr>";

	text += "<tr>";
	text += tr("<td><img src=\"qrc%1\"></td><td><b>%2</b><br>%3</td>")
		.arg(gd.Industry(IID_DesignStudio).BitmapName())
		.arg(gd.IndustryName(IID_DesignStudio))
		.arg(tr("Provides a multiplayer 3D modeling environment for creating new blueprints of buildings and spacecraft."));

	text += tr("<td><img src=\"qrc%1\"></td><td><b>%2</b><br>%3</td>")
		.arg(gd.Industry(IID_Farm).BitmapName())
		.arg(gd.IndustryName(IID_Farm))
		.arg(tr("Grows food and raises animals. There are settings to control the DNA of animal production."));
	text += "</tr>";

	text += "<tr>";
	text += tr("<td><img src=\"qrc%1\"></td><td><b>%2</b><br>%3</td>")
		.arg(gd.Industry(IID_Grocery).BitmapName())
		.arg(gd.IndustryName(IID_Grocery))
		.arg(tr("Sells food, drinks, and medical supplies."));

	text += tr("<td><img src=\"qrc%1\"></td><td><b>%2</b><br>%3</td>")
		.arg(gd.Industry(IID_Hangar).BitmapName())
		.arg(gd.IndustryName(IID_Hangar))
		.arg(tr("Stores spacecraft. There are controls to mothball spacecraft, for long term storage, and to recover them."));
	text += "</tr>";

	text += "<tr>";
	text += tr("<td><img src=\"qrc%1\"></td><td><b>%2</b><br>%3</td>")
		.arg(gd.Industry(IID_Hospital).BitmapName())
		.arg(gd.IndustryName(IID_Hospital))
		.arg(tr("Heals sick and injured people."));

	text += tr("<td><img src=\"qrc%1\"></td><td><b>%2</b><br>%3</td>")
		.arg(gd.Industry(IID_BaseHeadquarters).BitmapName())
		.arg(gd.IndustryName(IID_BaseHeadquarters))
		.arg(tr("Manages all military buildings on the world."));
	text += "</tr>";

	text += "<tr>";
	text += tr("<td><img src=\"qrc%1\"></td><td><b>%2</b><br>%3</td>")
		.arg(gd.Industry(IID_RetailStore).BitmapName())
		.arg(gd.IndustryName(IID_RetailStore))
		.arg(tr("Sells a wide variety of things: clothing, tools, weapons, ammunition, "
					"construction supplies, spare parts, fuel, and more."));

	text += tr("<td><img src=\"qrc%1\"></td><td><b>%2</b><br>%3</td>")
		.arg(gd.Industry(IID_SpacecraftFactory).BitmapName())
		.arg(gd.IndustryName(IID_SpacecraftFactory))
		.arg(tr("Makes fleet spacecraft for the empire. "
					"Makes and sells company spacecraft to private individuals."));
	text += "</tr>";

	text += "<tr>";
	text += tr("<td><img src=\"qrc%1\"></td><td><b>%2</b><br>%3</td>")
		.arg(gd.Industry(IID_StorageTank).BitmapName())
		.arg(gd.IndustryName(IID_StorageTank))
		.arg(tr("Stores liquid and gas commodities. There is a setting to control what is accumulated."));

	text += tr("<td><img src=\"qrc%1\"></td><td><b>%2</b><br>%3</td>")
		.arg(gd.Industry(IID_Warehouse).BitmapName())
		.arg(gd.IndustryName(IID_Warehouse))
		.arg(tr("Stores dry commodities. There is a setting to control what is accumulated."));
	text += "</tr>";

	g_Au.UnlockData();
	text += "</tbody></table>";
	tb->setHtml(text);

	// Build the layout.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(tb);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuBldInfo::~AuBldInfo()
{
}

void AuBldInfo::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Show all my widgets.
	QVBoxLayout* lay = (QVBoxLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->show();
}

void AuBldInfo::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Hide my widget now or they force the parent AuLabor dialog to be too big even though this page isn't visible.
	QVBoxLayout* lay = (QVBoxLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->hide();
}
