//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austeamworksui.cpp,v 1.5 2022/02/19 18:17:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austeamworksui.h"
//
#include "auapp.h"
#include "auglobal.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

AuSteamworksUI::AuSteamworksUI(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Steamworks"));
	setAuDlgIcon(QIcon(":data/Steamworks.png"));

	QLabel* lbicon = new QLabel(this);
	lbicon->setPixmap(QPixmap(":data/Steamworks.png"));

	QPushButton* pbequip = new QPushButton(tr("Create Personal Equipment"), this);
	pbequip->setToolTip(tr("<html>"
				"<p>"
				"Creates a grav pack in your possession. "
				"It contains useful items, including weapons, ammo and an environment suit. "
				"</p>"
				"</html>"));
	connect(pbequip, SIGNAL(clicked()), auApp, SLOT(SteamEquipAvatar()));

	QPushButton* pbship = new QPushButton(tr("Create Starship"), this);
	pbship->setToolTip(tr("<html>"
				"<p>"
				"Creates a starship in orbit directly overhead. "
				"Crews and fuels the ship. "
				"Moves you aboard, near the helm. "
				"Makes you the captain. "
				"</p>"
				"<p>"
				"Creates a new empire if you have not already done so, with you as its emperor. "
				"</p>"
				"</html>"));
	connect(pbship, SIGNAL(clicked()), auApp, SLOT(SteamCreateStarship()));

	QPushButton* pbstation = new QPushButton(tr("Create Space Station"), this);
	pbstation->setToolTip(tr("<html>"
				"<p>"
				"Creates a space station in orbit directly overhead. "
				"Crews and fuels the ship. "
				"Moves you aboard, near the helm. "
				"</p>"
				"<p>"
				"Creates a new empire if you have not already done so, with you as its emperor. "
				"</p>"
				"</html>"));
	connect(pbstation, SIGNAL(clicked()), auApp, SLOT(SteamCreateStation()));

	QLabel* lbinfo = new QLabel(tr("<html>"
			"<p>"
			"The Targoss tutorial starts right away. "
			"It is kind of long. "
			"If you want it to stop, open the Bio (F8) window, go to the Jobs tab, and drop the story. "
			"</p>"
			"<p>"
			"Game flow generally goes like this:"
			"<br>"
			"%1 Explore the environment. Learn to play. "
			"<br>"
			"%1 Create an empire by handcrafting a flag. "
			"<br>"
			"%1 Build a city using the Construction window. "
			"Start with a mine to make stone, using the 'Quarry by Haxus' design. "
			"<br>"
			"%1 Manufacture things. "
			"<br>"
			"%1 Expand and manage the city. "
			"<br>"
			"%1 Develop technology in the form of patents. "
			"<br>"
			"%1 Build spacecraft from the things your city manufactures. "
			"<br>"
			"%1 Explore other worlds then other solar systems. "
			"Other galaxies take a long time to reach but it can be done. "
			"</p>"
			"<p>"
			"These buttons will shortcut that process quite a bit. "
			"<br>"
			"%1 <b>Create Personal Equipment</b> so you have an environment suit and personal weapons. "
			"They aren't needed to make a spacecraft but they can be handy."
			"<br>"
			"%1 <b>Create Starship</b> to skip building a city. "
			"It can take days to build up a city to where it can make starships. "
			"<br>"
			"%1 Use the Spacecraft Mission window to issue commands. The crew will carry them out."
			"<br>"
			"%1 Look at the Star Map. Right-click to issue mission orders. "
			"<br>"
			"%1 Enter the transporter room. "
			"Use the Crew or Voice comm channel to ask the transporter operator to beam you somewhere. "
			"Alternately, you can right-click on the world map to ask for transport. "
			"<br>"
			"%1 Use the Crew channel to call for beam up, back to the ship. "
			"<br>"
			"%1 <b>Create Space Station</b> if you just want to see one, or for target practice. "
			"Wear an EV suit and EVA through space between your ship and the station. "
			"Try it using the transporter. "
			"</p>"
			"</html>").arg(g_Au.MiddleDotSymbol()), this);
	lbinfo->setWordWrap(true);

	QVBoxLayout* blay = new QVBoxLayout;
	blay->addWidget(lbicon);
	blay->addStretch();
	blay->addWidget(pbequip);
	blay->addWidget(pbship);
	blay->addWidget(pbstation);
	blay->setMargin(2);
	blay->setSpacing(2);

	QVBoxLayout* ilay = new QVBoxLayout;
	ilay->addWidget(lbinfo);
	ilay->setMargin(2);
	ilay->setSpacing(2);

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(ilay);

	setAuDlgLayout(lay);
}

AuSteamworksUI::~AuSteamworksUI()
{
}

