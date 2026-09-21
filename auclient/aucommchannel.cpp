//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommchannel.cpp,v 1.73 2022/04/07 22:11:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommchannel.h"
//
#include "auagentteleport.h"
#include "auapp.h"
#include "auassignberth.h"
#include "aubeamoptions.h"
#include "aubeamup.h"
#include "aubeamvehicleoptions.h"
#include "auclientglobal.h"
#include "aucomm.h"
#include "aucommannounce.h"
#include "aucommbuy.h"
#include "aucommdialog.h"
#include "aucommlog.h"
#include "aucommmission.h"
#include "aucommnotify.h"
#include "aucommopenchannel.h"
#include "aucommsellcargo.h"
#include "aucommsolicititem.h"
#include "aucommstdchannels.h"
#include "audiplomaticmessage.h"
#include "auemojidlg.h"
#include "auframe.h"
#include "auimagedialog.h"
#include "auitemdesc.h"
#include "aulineedit.h"
#include "auloadcolonists.h"
#include "aulogincomm.h"
#include "aumailnew.h"
#include "aupostbounty.h"
#include "AuSendFunds.h"
#include "auservicedialog.h"
#include "ausilverwillnotedlg.h"
#include "ausilverwilltaskdlg.h"
#include "auspecies.h"
#include "autoolbutton.h"
#include "auunloadcolonists.h"
#include "auview.h"
#include "daanimal.h"
#include "daatom.h"
#include "dacreature.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "daworld.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>
#include <QUrl>

AuCommChannel::AuCommChannel(const QString& name, AuCommStdChannel::SCOPE scope, QWidget* parent) : QWidget(parent)
{
	m_MissionDlg = 0;
	m_SaidIndex = -1;
	m_Scope = scope;
	m_SilverwillTaskId = 0;
	m_UnseenChat = false;
	setObjectName(name);

	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	bool isProgrammer = gd.ClientPlayerIsProgrammer();
	g_Au.UnlockData();

	m_Transcript = new AuCommDialog(IsNarrator(), this);
	connect(m_Transcript, SIGNAL(activated(const QModelIndex&)),
			SLOT(MessageActivated(const QModelIndex&)));	// Enter and Double-Click
	connect(m_Transcript, SIGNAL(itemSelectionChanged()), SLOT(SelectionChanged()));
	connect(m_Transcript, SIGNAL(ItemRightClicked(QTableWidgetItem*)), SLOT(MessageClicked()));

	QToolButton* pannounce = 0;
	QToolButton* pbagree = 0;
	QToolButton* pbdetonate = 0;
	QToolButton* pbdiplo = 0;
	QToolButton* pbeamup = 0;
	QToolButton* pbinven = 0;
	QToolButton* pbloadcol = 0;
	QToolButton* pbname = 0;
	QToolButton* pbountyspot = 0;
	QToolButton* pbrepair = 0;
	QToolButton* pbrevoke = 0;
	QToolButton* pbsales = 0;
	QToolButton* pbsell = 0;
	QToolButton* pbunloadcol = 0;
	QToolButton* phail = 0;
	QToolButton* phailcomment = 0;
	QToolButton* phailreply = 0;
	QToolButton* phgtask = 0;
	QToolButton* preqberth = 0;	// request berth
	QToolButton* tborder = 0;

	// Broadcast range row doesn't apply to Voice and Intercom channels.
	// Broadcast Ranges: Close, Hail, System, Sector, Galaxy
	QHBoxLayout* rangeButtonBar = 0;
	m_Range = 0;
	m_RangeSetting = AuCommStdChannel::Close;
	m_Say = 0;
	QToolButton* clearChat = 0;
	QToolButton* emoji = 0;
	QToolButton* sendChat = 0;
	QHBoxLayout* buttonBar = 0;
	if(!IsNarrator())
	{
		QSize rangeIconSize(10,10);
		QPixmap pic = QPixmap(AuCommStdChannel::ChannelIconName(name, scope))
			.scaled(rangeIconSize.width(), rangeIconSize.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

		QPushButton* pbchan = new QPushButton(QIcon(pic), name);
		pbchan->setDefault(false);
		pbchan->setToolTip(tr("<html><b>Transcript...</b><br>"
					"Display the transcript of messages recorded on this channel.</html>"));
		connect(pbchan, SIGNAL(clicked()), SLOT(ShowLog()));

		if(scope != AuCommStdChannel::Thoughts)
		{
			rangeButtonBar = new QHBoxLayout;
			rangeButtonBar->setSpacing(2);
			rangeButtonBar->addWidget(pbchan);

			if(AuCommStdChannel::ChannelHasRange(name))
			{
				m_RangeSetting = AuCommStdChannel::DefaultRange(name);

				m_Range = new QToolButton(this);
				m_Range->setAutoRaise(true);
				m_Range->setIconSize(rangeIconSize);
				SetRange(m_RangeSetting);
				connect(m_Range, SIGNAL(clicked(bool)), SLOT(RangeMenu()));
				rangeButtonBar->addWidget(m_Range);
			}

			m_Say = new AuLineEdit(this);
			m_Say->setMaxLength(AuCommChat::MaxChatText());
			m_Say->setToolTip(tr("<html>"
						"<p>"
						"Up/down arrows for input history."
						"</p>"
						"</html>"));
			connect(m_Say, SIGNAL(textChanged(const QString&)), SLOT(TextChanged(const QString&)));
			connect(m_Say, SIGNAL(returnPressed()), SLOT(ReturnPressed()));
			connect(m_Say, SIGNAL(UpArrowPressed()), SLOT(UpArrowPressed()));
			connect(m_Say, SIGNAL(DownArrowPressed()), SLOT(DownArrowPressed()));

			emoji = new QToolButton(this);
			emoji->setIcon(QIcon(":data/Emoji.png"));
			emoji->setToolTip(tr("<html><p><b>Emoji</b><br> "
						"Adds emojis to the text. "
						"</p>"
						"</html>"));
			emoji->setAutoRaise(true);
			emoji->setIconSize(rangeIconSize);
			connect(emoji, SIGNAL(clicked(bool)), SLOT(EmojiPressed()));

			sendChat = new QToolButton(this);
			sendChat->setIcon(QIcon(":data/OrderText.png"));
			sendChat->setToolTip(tr("<html><p><b>Send Text</b><br> "
						"Transmits the text that has been entered into the chat box. "
						"</p>"
						"<p>"
						"Your mouth moves when you chat on any channel. "
						"Emojis change your current visage. "
						"Read <b>Emojis</b> in the wiki for detailed information. "
						"</p>"
						"</html>"));
			sendChat->setAutoRaise(true);
			sendChat->setIconSize(rangeIconSize);
			connect(sendChat, SIGNAL(clicked(bool)), SLOT(ReturnPressed()));

			clearChat = new QToolButton(this);
			clearChat->setIcon(QIcon(":data/Sweep.png"));
			clearChat->setToolTip(tr("<b>Clear Chat</b><br>"
						"Clears text that has been entered into the chat box without broadcasting the text."));
			clearChat->setAutoRaise(true);
			clearChat->setIconSize(rangeIconSize);
			connect(clearChat, SIGNAL(clicked(bool)), SLOT(ClearChatEntry()));

			// Help channel does not have buttons for commerce.
			if(m_Scope != AuCommStdChannel::Help)
			{
				if(!auApp->IsSingleUserDesignerMode())
				{
					pbsell = new QToolButton(this);
					pbsell->setIcon(QIcon(":data/OrderGetItem.png"));
					pbsell->setToolTip(tr("<b>Offer Items...</b><br>"
								"Offer items, for free or for a price.<br>"
								"<i>Tips</i><br>"
								"%1 Offer <b>Cargo</b> while aboard a spacecraft. You must be an officer of the ship.<br>"
								"%1 Offer <b>Person Items</b> to others.<br>"
								"%1 Transmit on the <b>Trade channel</b> to initiate contact with a city.")
							.arg(g_Au.MiddleDotSymbol()));
					pbsell->setAutoRaise(true);
					connect(pbsell, SIGNAL(clicked(bool)), SLOT(ForSale()));
				}

				pbsales = new QToolButton(this);
				pbsales->setIcon(QIcon(":data/MsgSolicitSales.png"));
				pbsales->setToolTip(tr("<b>Ask For Items...</b><br>"
							"Ask for a specific item or category of items.<br>"
							"<i>Tips</i><br>"
							"%1 Ask for <b>Cargo</b> while aboard a spacecraft. You must be an officer of the ship.<br>"
							"%1 Ask for <b>Person Items</b> from indigenous and other people.<br>"
							"%1 Ask for <b>Fuel</b> for a vehicle while parked on a road slab.<br>"
							"%1 Transmit on the <b>Trade channel</b> to initiate contact with a city.")
						.arg(g_Au.MiddleDotSymbol()));
				pbsales->setAutoRaise(true);
				connect(pbsales, SIGNAL(clicked(bool)), SLOT(SolicitSales()));

				if(!auApp->IsSingleUserDesignerMode())
				{
					pbrevoke = new QToolButton(this);
					pbrevoke->setIcon(QIcon(":data/MsgRevoke.png"));
					pbrevoke->setToolTip(tr("<b>Revoke</b><br>"
								"Revokes your last 'Bid' or other kind of offer placed on this channel.<br>"
								"<i>Tips</i><br>"
								"%1 The offer is deactivated; it can no longer be accepted by anyone.<br>"
								"%1 You will have no active bids or other offers on this channel.")
							.arg(g_Au.MiddleDotSymbol()));
					pbrevoke->setAutoRaise(true);
					connect(pbrevoke, SIGNAL(clicked(bool)), SLOT(RevokeOffer()));

					tborder = new QToolButton(this);
					tborder->setIcon(QIcon(":data/GovSpacecraft.png"));
					tborder->setToolTip(tr("<b>Spacecraft Mission Order...</b><br>"
								"Issues a mission order.<br>"
								"<i>Tips</i><br>"
								"%1 Transmit on the <b>Crew channel</b> or the <b>Intercom channel</b> to issue an order to the "
								"crew of your ship.<br>"
								"%1 Transmit on the <b>Fleet channel</b> to issue an order to spacecraft in your chain-of-command.")
							.arg(g_Au.MiddleDotSymbol()));
					tborder->setAutoRaise(true);
					connect(tborder, SIGNAL(clicked(bool)), SLOT(MissionOrder()));

					preqberth = new QToolButton(this);
					preqberth->setIcon(QIcon(":data/RequestBerth.png"));
					preqberth->setToolTip(tr("<b>Request Berth...</b><br>"
								"Request an available berth on a spacecraft. An officer aboard a spacecraft may accept the request. "
								"A ship may accept requests from friendly or neutral players.<br>"
								"<i>Tips</i><br>"
								"%1 Transmit on the <b>Intercom channel</b> to contact the ship you are aboard.<br>"
								"%1 A Captain's berth request is always accepted by the ship when the berth is unoccupied.<br>"
								"%1 Berth acceptance policies determine if requests for other kinds of berths will be accepted by the "
								"ship.")
							.arg(g_Au.MiddleDotSymbol()));
					preqberth->setAutoRaise(true);
					connect(preqberth, SIGNAL(clicked(bool)), SLOT(RequestBerth()));
				}

				if(m_Scope == AuCommStdChannel::Intercom ||
						m_Scope == AuCommStdChannel::CrewMates ||
						m_Scope == AuCommStdChannel::Fleet ||
						m_Scope == AuCommStdChannel::Company ||
						AuCommStdChannel::IsHailChannel(name))
				{
					pbinven = new QToolButton(this);
					pbinven->setIcon(QIcon(":data/CommManifestShip.png"));
					if(isAgent)
						pbinven->setToolTip(tr("<b>Status Report</b><br>"
									"Request a status report from a spacecraft.<br>"
									"<i>Tips</i><br>"
									"%1 Reports ship's fund, berth occupancy, commodities in the hold, "
									"fuel and capacitor level.<br>"
									"%1 A spacecraft will only respond to people in its chain of command.<br>"
									"%1 <b>Agent Power</b> All spacecraft respond to agents when agent power is enabled.")	// Only difference from non-agent tip.
								.arg(g_Au.MiddleDotSymbol()));
					else
						pbinven->setToolTip(tr("<b>Status Report</b><br>"
									"Request a status report from a spacecraft.<br>"
									"<i>Tips</i><br>"
									"%1 Reports ship's fund, berth occupancy, commodities in the hold, "
									"fuel and capacitor level.<br>"
									"%1 A spacecraft will only respond to people in its chain of command.")
								.arg(g_Au.MiddleDotSymbol()));
					pbinven->setAutoRaise(true);
					connect(pbinven, SIGNAL(clicked(bool)), SLOT(CityInventory()));
				}
				else if(AuCommStdChannel::IsTradeChannel(name))
				{
					pbinven = new QToolButton(this);
					pbinven->setIcon(QIcon(":data/CommManifestCity.png"));
					if(isAgent)
						pbinven->setToolTip(tr("<b>Status Report</b><br>"
									"Request a status report from a building or city.<br>"
									"<i>Tips</i><br>"
									"%1 Airport Terminals report passengers and services available.<br>"
									"%1 Base Air Terminals report officers, crew and troops ready for deployment.<br>"
									"%1 Brokers report commodities available.<br>"
									"%1 Cities report population and inventory of commodities in storage.<br>"
									"%1 Cities also report when the next inbound shipment is expected.<br>"
									"%1 Cities only respond to people in their chain of command.<br>"
									"%1 <b>Agent Power</b> All cities respond to agents when agent power is enabled.<br>"	// Only difference from non-agent tip.
									"%1 Transmit on the <b>Trade channel</b> to initiate contact with a city.")
								.arg(g_Au.MiddleDotSymbol()));
					else
						pbinven->setToolTip(tr("<b>Status Report</b><br>"
									"Request a status report from a building or city.<br>"
									"<i>Tips</i><br>"
									"%1 Airport Terminals report passengers and services available.<br>"
									"%1 Base Air Terminals report officers, crew and troops ready for deployment.<br>"
									"%1 Brokers report commodities available.<br>"
									"%1 Cities report population and inventory of commodities in storage.<br>"
									"%1 Cities also report when the next inbound shipment is expected.<br>"
									"%1 Cities only respond to people in their chain of command.<br>"
									"%1 Transmit on the <b>Trade channel</b> to initiate contact with a city.")
								.arg(g_Au.MiddleDotSymbol()));
					pbinven->setAutoRaise(true);
					connect(pbinven, SIGNAL(clicked(bool)), SLOT(CityInventory()));
				}
				else if(!AuCommStdChannel::IsStandardChannel(name) && m_Scope == AuCommStdChannel::Everyone)
				{
					// No remaining standard channels know how to respond to inventory requests. Non-standard channels might include
					// the private channels opened by cities in reply to communications.
					pbinven = new QToolButton(this);
					pbinven->setIcon(QIcon(":data/CommManifest.png"));
					if(isAgent)
						pbinven->setToolTip(tr("<b>Status Report</b><br>"
									"Request a status report from a spacecraft, building or city.<br>"
									"<i>Tips</i><br>"
									"%1 Airport Terminals report passengers and services available.<br>"
									"%1 Base Air Terminals report officers, crew and troops ready for deployment.<br>"
									"%1 Brokers report commodities available.<br>"
									"%1 Cities report population and inventory of commodities in storage.<br>"
									"%1 Cities also report when the next import cycle is expected.<br>"
									"%1 Spacecraft report ship's fund, berth occupancy, commodities in the hold, "
									"fuel and capacitor level.<br>"
									"%1 Cities and spacecraft only respond to people in their chain of command.<br>"
									"%1 <b>Agent Power</b> All cities and spacecraft respond to agents when agent power is enabled.<br>"	// Only difference from non-agent tip.
									"%1 Transmit on the <b>Trade channel</b> to initiate contact with a city.")
								.arg(g_Au.MiddleDotSymbol()));
					else
						pbinven->setToolTip(tr("<b>Status Report</b><br>"
									"Request a status report from a spacecraft, building or city.<br>"
									"<i>Tips</i><br>"
									"%1 Airport Terminals report passengers and services available.<br>"
									"%1 Base Air Terminals report officers, crew and troops ready for deployment.<br>"
									"%1 Brokers report commodities available.<br>"
									"%1 Cities report population and inventory of commodities in storage.<br>"
									"%1 Cities also report when the next import cycle is expected.<br>"
									"%1 Spacecraft report ship's fund, berth occupancy, commodities in the hold, "
									"fuel and capacitor level.<br>"
									"%1 Cities and spacecraft only respond to people in their chain of command.<br>"
									"%1 Transmit on the <b>Trade channel</b> to initiate contact with a city.")
								.arg(g_Au.MiddleDotSymbol()));
					pbinven->setAutoRaise(true);
					connect(pbinven, SIGNAL(clicked(bool)), SLOT(CityInventory()));
				}

				if(AuCommStdChannel::IsTradeChannel(name) ||
						(!AuCommStdChannel::IsStandardChannel(name) && m_Scope == AuCommStdChannel::Everyone))
				{
					pbloadcol = new QToolButton(this);
					pbloadcol->setIcon(QIcon(":data/MsgLoadColonists.png"));
					pbloadcol->setToolTip(tr("<b>Load Crew, Passengers, Troops or Cryo Travellers...</b><br>"
								"Offer empty crew berths to crew looking for a ship.<br>"
								"Offer empty stateroom berths or vehicle seats to passengers looking for transport.<br>"
								"Offer empty troop berths to troops waiting for deployment.<br>"
								"Offer empty cryo berths to travellers looking for a ship.<br>"
								"<i>Tips</i><br>"
								"%1 Loads <b>Crew</b>, <b>Passengers</b>, <b>Troops</b> or <b>Cryo Travellers</b> while aboard "
								"a spacecraft. "
								"You must be an officer of the ship.<br>"
								"%1 Loads <b>Passengers</b> into a vehicle that has empty seats.<br>"
								"%1 <b>Troops</b> must be at bases, not yet assigned to a spacecraft, and members of your "
								"empire.<br>"
								"%1 Transmit on the <b>Trade channel</b> to initiate contact with a city.")
							.arg(g_Au.MiddleDotSymbol()));
					pbloadcol->setAutoRaise(true);
					connect(pbloadcol, SIGNAL(clicked(bool)), SLOT(LoadColonists()));

					pbunloadcol = new QToolButton(this);
					pbunloadcol->setIcon(QIcon(":data/MsgUnloadColonists.png"));
					pbunloadcol->setToolTip(tr("<b>Debark Passengers or Cryo Travellers...</b><br>"
								"Offers to unload passengers from your ship or vehicle.<br>"
								"Offers to unload cryo travellers from your ship.<br>"
								"<i>Tips</i><br>"
								"%1 Unloads <b>Passengers</b> or <b>Cryo Travellers</b> from a spacecraft. "
								"You must be an officer of the ship.<br>"
								"%1 Unloads <b>Passengers</b> from a vehicle to the city you are in.<br>"
								"%1 Transmit on the <b>Trade channel</b> to initiate contact with a city.")
							.arg(g_Au.MiddleDotSymbol()));
					pbunloadcol->setAutoRaise(true);
					connect(pbunloadcol, SIGNAL(clicked(bool)), SLOT(UnloadColonists()));

					pbrepair = new QToolButton(this);
					pbrepair->setIcon(QIcon(":data/ShipRefit.png"));
					pbrepair->setToolTip(tr("<b>Service Spacecraft...</b><br>"
								"Request repair, refit, modules and other services for your spacecraft from a city. "
								"<br>"
								"<i>Tips</i><br>"
								"%1 You must be an officer of the ship.<br>"
								"%1 Transmit on the <b>Trade channel</b> to initiate contact with a city.")
							.arg(g_Au.MiddleDotSymbol()));
					pbrepair->setAutoRaise(true);
					connect(pbrepair, SIGNAL(clicked(bool)), SLOT(RequestService()));

					pbdiplo = new QToolButton(this);
					pbdiplo->setIcon(QIcon(":data/CommDiplomacy.png"));
					pbdiplo->setToolTip(tr("<html><b>Diplomatic Message...</b><br>"
								"Sends a chat message that will be forwarded to a city's chain of command via mail.<br>"
								"<i>Tips</i><br>"
								"%1 All cities that receive this message will forward it to their chain of command. "
								"Use range or a channel to limit which cities receive the message, to reduce spam.<br>"
								"%1 Each city can forward only one diplomatic message per five minutes.<br>"
								"%1 Enemy cities forward diplomatic messages even though they do not respond to you."
								"</html>").arg(g_Au.MiddleDotSymbol()));
					pbdiplo->setAutoRaise(true);
					connect(pbdiplo, SIGNAL(clicked(bool)), SLOT(DiplomaticMessage()));
				}

				if(!auApp->IsSingleUserDesignerMode())
				{
					pbeamup = new QToolButton(this);
					pbeamup->setIcon(QIcon(":data/MsgTransport.png"));
					pbeamup->setToolTip(tr("<b>Request Transporter Beam...</b><br>"
								"Request beam over via matter transporter. A ship may accept requests from players.<br>"
								"<i>Tips</i><br>"
								"%1 Transmit on the <b>Crew</b> channel to contact your ship. "
								"You must be listed on the ship's roster in any berth except Passenger.<br>"
								"%1 Transmit on the <b>Hail</b> channel to contact a friendly or neutral station in orbit.<br>"
								"%1 Transmit on the <b>Voice</b> channel for destination options in a transporter room.")
							.arg(g_Au.MiddleDotSymbol()));
					pbeamup->setAutoRaise(true);
					connect(pbeamup, SIGNAL(clicked(bool)), SLOT(BeamOver()));
				}

				if(m_Scope != AuCommStdChannel::Intercom)
				{
					phail = new QToolButton(this);
					phail->setAutoRaise(true);
					phail->setIcon(QIcon(":data/CommHail.png"));
					if(objectName() == AuCommStdChannel::VoiceChannelName())
					{
						phail->setToolTip(tr("<b>Hello</b><br>"
									"Starts a conversation with the person or citizen you are facing.<br>"
									"<i>Tips</i><br>"
									"%1 You do not have to be exactly targeted on a citizen.<br>"
									"%1 Hailing reaches certain game-specific listeners on other channels. Check tips.")
								.arg(g_Au.MiddleDotSymbol()));
					}
					else if(objectName() == AuCommStdChannel::HailChannelName())
					{
						phail->setToolTip(tr("<b>Hail Spacecraft and Airport Terminals</b><br>"
									"Establishes communications with with airport terminals, base air terminals, "
									"spacecraft and stations.<br>"
									"<i>Tips</i><br>"
									"%1 Stations listen on the <b>Hail</b> channel.<br>"
									"%1 Airport Terminals listen on the <b>Hail</b> channel.<br>"
									"%1 Base Air Terminals listen on the <b>Hail</b> channel. "
									"They only reply if the caller is in their chain of command.<br>"
									"%1 Replies are sent on a separate channel that they open to you.<br>"
									"%1 Hailing reaches certain game-specific listeners on other channels. Check tips.")
								.arg(g_Au.MiddleDotSymbol()));
					}
					else if(objectName() == AuCommStdChannel::FleetChannelName())
					{
						phail->setToolTip(tr("<b>Hail Fleet</b><br>"
									"Establishes communication with all spacecraft in your chain of command. "
									"<i>Tips</i><br>"
									"%1 Fleet spacecraft listen on the <b>Fleet</b> channel and the channel that matches their "
									"name.<br>"
									"%1 Each fleet spacecraft answers on a separate channel that they open to you. "
									"The channel name is the name of the spacecraft.<br>"
									"%1 Hailing reaches certain game-specific listeners on other channels. Check tips.")
								.arg(g_Au.MiddleDotSymbol()));
					}
					else if(objectName() == AuCommStdChannel::CompanyChannelName())
					{
						phail->setToolTip(tr("<b>Hail Company</b><br>"
									"Establishes communication with all spacecraft in your company.<br>"
									"<i>Tips</i><br>"
									"%1 Company spacecraft listen on the <b>Company</b> channel and the channel that matches their "
									"name.<br>"
									"%1 Each company spacecraft answers on a separate channel that they open to you. "
									"The channel name is the name of the spacecraft.<br>"
									"%1 Hailing reaches certain game-specific listeners on other channels. Check tips.")
								.arg(g_Au.MiddleDotSymbol()));
					}
					else if(objectName() == AuCommStdChannel::TradeChannelName())
					{
						phail->setToolTip(tr("<b>Hail Cities and Brokers</b><br>"
									"Establishes communication with brokers and cities. "
									"Whether you are standing in a retail store looking to buy something or aboard a "
									"spacecraft wanting to trade cargo, this is how you get started.<br>"
									"Regains contact with cities after they stop listening on a channel. Cities stop "
									"listening after a few minutes of inactivity.<br>"
									"<i>Tips</i><br>"
									"%1 Each city answers on a separate channel that they open to you. "
									"The channel name begins with the city name.<br>"
									"%1 Hailing reaches certain game-specific listeners on other channels. Check tips.")
								.arg(g_Au.MiddleDotSymbol()));
					}
					else if(objectName() == AuCommStdChannel::CrewChannelName())
					{
						phail->setToolTip(tr("<b>Hail Crew</b><br>"
									"Establishes contact with the ship on which you hold a berth.<br>"
									"When successful, the ship appears in orbit overhead; then it hails you.<br>"
									"<i>Tips</i><br>"
									"%1 Hailing reaches certain game-specific listeners on other channels. Check tips.")
								.arg(g_Au.MiddleDotSymbol()));
					}
					else
					{
						phail->setToolTip(tr("<b>Hail</b><br>"
									"Establishes contact with anyone listening on the channel.<br>"
									"<i>Tips</i><br>"
									"%1 Hailing reaches certain game-specific listeners on other channels. Check tips.")
								.arg(g_Au.MiddleDotSymbol()));
					}
					connect(phail, SIGNAL(clicked(bool)), SLOT(HailRequest()));

					phailcomment = new QToolButton(this);
					phailcomment->setAutoRaise(true);
					phailcomment->setIcon(QIcon(":data/CommHailComment.png"));
					phailcomment->setToolTip(tr("<b>Hail With Comment...</b><br>"
								"Same as <b>Hail</b>, except you can enter text that will be appended to the hail message."));
					connect(phailcomment, SIGNAL(clicked(bool)), SLOT(HailRequestComment()));

					phailreply = new QToolButton(this);
					phailreply->setIcon(QIcon(":data/CommAnnounce.png"));
					phailreply->setToolTip(tr("<b>Announce...</b><br>"
								"Acknowledges reception of a hail message. "
								"Provides informative response choices, such as location and posture.<br>"
								"<i>Tips</i><br>"
								"%1 Provides a standard response to a hail.<br>"
								"%1 Replies to a citizen who says 'Hello'.")
							.arg(g_Au.MiddleDotSymbol()));
					phailreply->setAutoRaise(true);
					connect(phailreply, SIGNAL(clicked(bool)), SLOT(Announce()));
				}
			}

			if(AuCommStdChannel::IsHailChannel(name))
			{
				pbdetonate = new QToolButton(this);
				pbdetonate->setIcon(QIcon(":data/CommDetonate.png"));
				pbdetonate->setToolTip(tr("<html><b>Detonate</b><br>"
							"Triggers detonation of munitions that have been armed.</html>"));
				pbdetonate->setAutoRaise(true);
				connect(pbdetonate, SIGNAL(clicked(bool)), SLOT(Detonate()));
			}

			if(!auApp->IsSingleUserDesignerMode())
			{
				if(isAgent)
				{
					pbagree = new QToolButton(this);
					pbagree->setToolTip(tr("<b>Accept</b><br>"
								"Accepts bids and other kinds of offers.<br>"
								"<i>Tips</i><br>"
	"%1 <b>Agent Power</b> Accepts help requests. "								// Only difference from non-agent tip.
	"Shows Agent UI and configures the Teleport page to go to the sender of the message. "	// Only difference from non-agent tip.
	"No comm message is sent by accepting the help request.<br>"				// Only difference from non-agent tip.
								"%1 Accepts bids to complete a sale; you must be the seller. "
								"Only the last bid from each buyer can be accepted. Each bid can be "
								"accepted only once. The transaction will be completed if possible. "
								"i.e. Buyer has the money; seller has the merchandise; buyer's ship "
								"has room for the merchandise; bid hasn't been revoked; etc.<br>"
								"%1 Accepts berth requests. You must be an officer of the ship.<br>"
								"%1 Accepts offers to repair or refit your spacecraft. You must be an officer of the ship.<br>"
								"%1 Accepts contact requests.<br>"
								"%1 Accepts mission orders.<br>"
								"%1 Accepts requests to join your empire. You must be the emperor or vice-emperor.<br>"
								"%1 Accepts invitations to join a different empire.<br>"
								"%1 Accepts government office assignments.<br>"
								"%1 Accepts 'Load Passengers' requests to transfer passengers from your ship to another ship. You "
								"must be an officer of the ship.<br>"
								"%1 Accepts 'Load Troops' requests to transfer troops from your ship to another ship. You "
								"%1 Accepts 'Load Cryo Traveller' requests to transfer travellers from your ship to another ship. "
								"You must be an officer of the ship.<br>"
								"%1 Accepts 'Unload Passengers' requests to transfer passengers to your ship from another ship. You "
								"%1 Accepts 'Unload Cryo Travellers' requests to transfer travellers to your ship from another "
								"ship. You must be an officer of the ship.")
							.arg(g_Au.MiddleDotSymbol()));
				}
				else if(m_Scope != AuCommStdChannel::Help)
				{
					pbagree = new QToolButton(this);
					pbagree->setToolTip(tr("<b>Accept</b><br>"
								"Accepts bids and other kinds of offers.<br>"
								"<i>Tips</i><br>"
								"%1 Accepts bids to complete a sale; you must be the seller. "
								"Only the last bid from each buyer can be accepted. Each bid can be "
								"accepted only once. The transaction will be completed if possible. "
								"i.e. Buyer has the money; seller has the merchandise; buyer's ship "
								"has room for the merchandise; bid hasn't been revoked; etc.<br>"
								"%1 Accepts berth requests. You must be an officer of the ship.<br>"
								"%1 Accepts offers to repair or refit your spacecraft. You must be an officer of the ship.<br>"
								"%1 Accepts contact requests.<br>"
								"%1 Accepts mission orders.<br>"
								"%1 Accepts requests to join your empire. You must be the emperor or vice-emperor.<br>"
								"%1 Accepts invitations to join a different empire.<br>"
								"%1 Accepts government office assignments.<br>"
								"%1 Accepts 'Load Passengers' requests to transfer passengers from your ship to another ship. You "
								"must be an officer of the ship.<br>"
								"%1 Accepts 'Load Troops' requests to transfer troops from your ship to another ship. You "
								"%1 Accepts 'Load Cryo Traveller' requests to transfer travellers from your ship to another ship. "
								"You must be an officer of the ship.<br>"
								"must be an officer of the ship.<br>"
								"%1 Accepts 'Unload Passengers' requests to transfer passengers to your ship from another ship. You "
								"%1 Accepts 'Unload Cryo Travellers' requests to transfer travellers to your ship from another "
								"must be an officer of the ship.")
							.arg(g_Au.MiddleDotSymbol()));
				}

				if(pbagree)
				{
					pbagree->setIcon(QIcon(":data/MsgAccept.png"));
					pbagree->setAutoRaise(true);
					connect(pbagree, SIGNAL(clicked(bool)), SLOT(Accept()));
				}
			}

			// Non-log channels have a lot of button.s
			buttonBar = new QHBoxLayout;
			const int buttonSpacing = 20;
			if(phail)
				buttonBar->addWidget(phail);
			if(phailcomment)
				buttonBar->addWidget(phailcomment);
			if(phailreply)
				buttonBar->addWidget(phailreply);
			if(tborder)
				buttonBar->addWidget(tborder);
			if(preqberth)
				buttonBar->addWidget(preqberth);
			if(pbeamup)
				buttonBar->addWidget(pbeamup);
			if(pbinven)
				buttonBar->addWidget(pbinven);
			if(phail || phailcomment || phailreply || tborder || preqberth || pbeamup || pbinven)
				buttonBar->addSpacing(buttonSpacing);

			if(pbrepair)
				buttonBar->addWidget(pbrepair);
			if(pbloadcol)
				buttonBar->addWidget(pbloadcol);
			if(pbunloadcol)
				buttonBar->addWidget(pbunloadcol);
			if(pbdiplo)
				buttonBar->addWidget(pbdiplo);
			if(pbrepair || pbloadcol || pbunloadcol || pbdiplo)
				buttonBar->addSpacing(buttonSpacing);

			if(pbsell)
				buttonBar->addWidget(pbsell);
			if(pbsales)
				buttonBar->addWidget(pbsales);
			if(pbsell || pbsales)
				buttonBar->addSpacing(buttonSpacing);

			if(pbrevoke)
				buttonBar->addWidget(pbrevoke);
			if(pbagree)
				buttonBar->addWidget(pbagree);

			if(pbdetonate)
			{
				buttonBar->addSpacing(buttonSpacing);
				buttonBar->addWidget(pbdetonate);
			}
		}
		else
		{
			// Thoughts channel has very few buttons.
			pbname = new QToolButton(this);
			pbname->setIcon(QIcon(":data/NameSpecie.png"));
			if(isAgent)
				pbname->setToolTip(tr("<b>Name Specie...</b><br>"
							"Assigns a name to the last creature specie that was spotted.<br>"
							"<i>Tips</i><br>"
					"%1 <b>Agent Power</b> Can rename any specie even if it already has "	// Only difference from non-agent tip.
					"a name, sentient or otherwise.<br>"									// Only difference from non-agent tip.
							"%1 Species that are already named can be renamed only by the person who originally named them.<br>"
							"%1 Sentient species can only be named by themselves. They cannot be named by other species.")
						.arg(g_Au.MiddleDotSymbol()));
			else
				pbname->setToolTip(tr("<b>Name Specie...</b><br>"
							"Assigns a name to the last creature specie that was spotted.<br>"
							"<i>Tips</i><br>"
							"%1 Species that are already named can be renamed only by the person who originally named them.<br>"
							"%1 Sentient species can only be named by themselves. They cannot be named by other species.")
						.arg(g_Au.MiddleDotSymbol()));
			pbname->setAutoRaise(true);
			connect(pbname, SIGNAL(clicked(bool)), SLOT(NameTarget()));

			pbountyspot = new QToolButton(this);
			pbountyspot->setIcon(QIcon(":data/JobBountyHunter.png"));
			pbountyspot->setToolTip(tr("<html><b>Post Bounty On Target...</b>"
						"<p>Post a bounty on the last person, spacecraft or city that was spotted.</p></html>"));
			pbountyspot->setAutoRaise(true);
			connect(pbountyspot, SIGNAL(clicked(bool)), SLOT(PostBountySpotted()));

			if(isProgrammer)
			{
				pannounce = new QToolButton(this);
				pannounce->setIcon(QIcon(":data/CommHailComment.png"));
				pannounce->setToolTip(tr("<html><b>Universal Announcement...</b>"
							"<p>Make an announcement that appears in the <i>Thoughts</i> channel of every avatar.</p></html>"));
				pannounce->setAutoRaise(true);
				connect(pannounce, SIGNAL(clicked(bool)), SLOT(Announcement()));

				// Silverwill
				phgtask = new QToolButton(this);
				phgtask->setIcon(QIcon(":data/Silverwill.png"));
				phgtask->setToolTip(tr("<html><b>Make Silverwill Task...</b>"
							"<p>"
							"Creats a new task in Silverwill."
							"</p>"

							"<p>"
							"Right click a message on any channel to make a Silverwill task from the message. "
							"</p>"
							"</html>"));
				phgtask->setAutoRaise(true);
				connect(phgtask, SIGNAL(clicked(bool)), SLOT(NewSilverwillTask()));
			}

			buttonBar = new QHBoxLayout;
			buttonBar->addWidget(pbchan);
			buttonBar->addWidget(pbname);
			buttonBar->addWidget(pbountyspot);
			if(pannounce)
				buttonBar->addWidget(pannounce);
			if(phgtask)
				buttonBar->addWidget(phgtask);
		}

		buttonBar->addStretch(1);
		//buttonBar->setIconSize(QSize(16,16));
		//buttonBar->setMovable(false);
	}

	QGridLayout* lay = new QGridLayout;
	int row = 0;
	lay->addWidget(m_Transcript, row,0, 1,2);
	if(!IsNarrator())
	{
		if(scope != AuCommStdChannel::Thoughts)
		{
			// Log channel does not have a 'say' box or range buttons.
			row++;
			if(rangeButtonBar)
			{
				rangeButtonBar->addWidget(emoji);
				rangeButtonBar->addWidget(m_Say);
				rangeButtonBar->addWidget(sendChat);
				rangeButtonBar->addWidget(clearChat);
				lay->addLayout(rangeButtonBar, row,0, 1,2);
			}
			else
			{
				lay->addWidget(emoji, row,0);
				lay->addWidget(m_Say, row,1);
				lay->addWidget(sendChat, row,2);
				lay->addWidget(clearChat, row,3);
			}
		}

		// All non-narrator channels have a button bar, even the Log channel.
		row++;
		lay->addLayout(buttonBar, row,0, 1,2);
	}
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

void AuCommChannel::CloseChannel()
{
	QString channelName = objectName();
	CloseChannel(channelName);			// Close the channel.
}

//static
void AuCommChannel::CloseChannel(const QString& channelName)
{
	// Notify server that we are no longer listening on this channel.
	auApp->CommCloseChannel(channelName);
}

// ::EnterPressed
void AuCommChannel::ReturnPressed()
{
	if(m_Say)
	{
		// Get the chat text from the edit control.
		QString text = m_Say->text();
		m_Say->blockSignals(true);
		m_Say->setText("");
		m_Say->blockSignals(false);

		if(!text.isEmpty())
		{
			// Replace emoji key phrases with utf8 emojis.
			text = g_AuClient.ReplaceEmojiTags(text);

			// Keep a history of the things said.
			if(m_Said.empty() || m_Said.last() != text)
			{
				m_Said.append(text);
				if(m_Said.size() > 100)
					m_Said.removeFirst();
			}
			m_SaidIndex = -1;

			const AuView* view = auApp->Frame()->View();
			AuCommChat msg(text, objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
					m_RangeSetting, m_Scope);
			SendChat(msg);

			if(g_Au.IsSceneSolo() && text == "STEAMWORKS")
				emit Steamworks();
		}
	}
}

void AuCommChannel::TextChanged(const QString& text)
{
	Q_UNUSED(text);
	m_SaidIndex = -1;
}

void AuCommChannel::DownArrowPressed()
{
	if(m_Said.empty())
		return;

	if(m_SaidIndex >= m_Said.size() - 1)
	{
		// Last step forward out of history.
		m_SaidIndex = -1;
		m_Say->blockSignals(true);
		m_Say->setText(m_AboutToSay);
		m_Say->blockSignals(false);
	}
	else if(m_SaidIndex >= 0)
	{
		// Step forward through history.
		m_SaidIndex++;
		m_Say->blockSignals(true);
		m_Say->setText(m_Said[m_SaidIndex]);
		m_Say->blockSignals(false);
	}
}

void AuCommChannel::UpArrowPressed()
{
	if(m_Said.empty())
		return;

	if(m_SaidIndex == -1)
	{
		// First step back into the history.
		// Preserve current edit control contents.
		m_AboutToSay = m_Say->text();
		m_SaidIndex = m_Said.size() - 1;
		m_Say->blockSignals(true);
		m_Say->setText(m_Said.last());
		m_Say->blockSignals(false);
	}
	else if(m_SaidIndex > 0)
	{
		// Step backward through history.
		m_SaidIndex--;
		m_Say->blockSignals(true);
		m_Say->setText(m_Said[m_SaidIndex]);
		m_Say->blockSignals(false);
	}
}

void AuCommChannel::SendChat(AuCommPacket& msg)
{
	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();
	if(!view)
		return;

	// Avoid accidentally chatting when entity or animal is mode enabled.
	if(m_Scope != AuCommStdChannel::Thoughts && Range() > AuCommStdChannel::SolarSystem)
	{
		if(AuCommStdChannel::IsGalacticChannel(objectName()) &&
				view->SceneAtomId() == AuId::Haxus() &&
				view->PersonAgentBits() & (AGENT_Entity|AGENT_Animal|AGENT_Incognito))
		{
			QMessageBox mb(g_Au.ApplicationName(),
					tr("Chat as %1 on <b>%2</b> channel?").arg(msg.FromName()).arg(msg.ChannelName()),
					QMessageBox::Information, QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton, pFrame);
			if(pFrame->ExecMessageBox(mb) != QMessageBox::Yes)
				return;
		}
		else if(pFrame->IsAFK())
		{
			QMessageBox mb(QMessageBox::Information, g_Au.ApplicationName(),
					tr("Chat on <b>%1</b> channel? The Avatars Online window indicates that you are away from your keyboard.")
					.arg(msg.ChannelName()));
			QPushButton* pbafk = mb.addButton(tr("Transmit and Turn Off AFK"), QMessageBox::AcceptRole);
			QPushButton* pbok = mb.addButton(tr("Transmit and Remain AFK"), QMessageBox::AcceptRole);
			mb.addButton(tr("Cancel"), QMessageBox::RejectRole);
			pFrame->ExecMessageBox(mb);
			if(mb.clickedButton() == pbafk)
			{
				// Turn off AFK setting.
				pFrame->DisableAFK();
			}
			else if(mb.clickedButton() == pbok)
			{
				// Do nothing.
			}
			else
				return;
		}
	}

	if(msg.Type() == AuCommPacket::SpotTargetOrTerrain && m_Scope == AuCommStdChannel::Everyone)
	{
		QMessageBox mb(g_Au.ApplicationName(),
				tr("<html>Confirm broadcast of your galactic coordinates<br>on open channel <b>%1</b>.</html>").arg(objectName()),
				QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, auApp->Frame());
		if(auApp->Frame()->ExecMessageBox(mb) != QMessageBox::Ok)
			return;
	}

	// Insure the range is correct. We sometimes get these from objects that don't know what range this channel is set to.
	// Init both transmission and reception ranges. Reception could get limited at the receiving end, but it is expected to
	// start out at the transmission range. Atoms that receive the message reply at reception range, but it is not actually
	// reduced for them like when a message is forwarded to a client in DtChatChannel (look for SetRange).
	if(AuCommStdChannel::ChannelHasRange(msg.ChannelName()))
	{
		msg.SetReceptionRange(Range());
		msg.SetTransmissionRange(Range());
	}
	else
	{
		msg.SetReceptionRange(AuCommStdChannel::DefaultRange(msg.ChannelName()));
		msg.SetTransmissionRange(AuCommStdChannel::DefaultRange(msg.ChannelName()));
	}

	// Set scope specific data.
	//AuCommStdChannel::SCOPE scope = msg.Scope();
	if(m_Scope == AuCommStdChannel::CrewMates)
		msg.SetScopeData(view->SceneAtomScopeShip().GetID());	// Ship Id needed to identify Ship crewmates. IsValid fails if 0.
	else if(m_Scope == AuCommStdChannel::Company)
		msg.SetScopeData(view->SceneAtomOwnerId().GetID());		// Owner id needed.
	else
		msg.SetScopeData(view->SceneAtomEmpire());				// Empire Id needed by Friends|Empire|Fleet|Government messages.

	if(msg.IsValid())
	{
		// The message appears valid. Send it, if it's not just a thought, which might have been generated locally.
		// Announcements are Thoughts but they must be allowed anyway.
		if(m_Scope != AuCommStdChannel::Thoughts || msg.Type() == AuCommPacket::Announcement)
			auApp->SendChatMessage(msg);

		// When spotting, we can't depend on the message coming back from the server. The sender might have a channel selected that
		// they are not permitted to transmit on. Then the spot command doesn't work and nobody knows why. So, we shortcut the
		// process here and give it right to the view. If the message does happen to come back from the server, the frame knows not
		// to send it to the view again.
		if(msg.Type() == AuCommPacket::SpotTargetOrTerrain)
		{
			// We must give the Log channel the log message because it was not transmitted to the server at all.
			msg.SetReceptionRange(AuCommStdChannel::Close);
			if(msg.Scope() == AuCommStdChannel::Thoughts)
				ChatReceived(msg);
			view->ChatReceived(&msg);
		}
	}
	else
	{
		qWarning() << "AuCommChannel::SendChat - INVALID MESSAGE" << msg.Type() << msg.Text();
	}
}

void AuCommChannel::ChatReceived(const AuCommPacket& msg)
{
	m_Transcript->ChatReceived(msg);

	if(g_AuClient.PopupNotify() &&
			(msg.Scope() != AuCommStdChannel::Thoughts && !msg.IsNarratorMessage() && !msg.IsSenderAnonymous() &&
			 msg.Type() != AuCommPacket::Silverwill))
	{
		// Check for avatar's name in the message, preceeded by @. e.g. @Haxus
		const DgData& gd = g_Au.LockDataForRead();
		AuId avatarId(gd.ClientUnitId());
		QString avatarName(gd.ClientUnitName(avatarId));
		g_Au.UnlockData();
		if(msg.FromId() != avatarId)
		{
			// Some people want the @ sign to be required.
			QString checkName;
			if(g_AuClient.PopupNotify() == 2)
				checkName = QString("@") + avatarName;
			else
				checkName = avatarName;

			if(msg.Text().contains(checkName, Qt::CaseInsensitive))
			{
				AuCommNotify* n = new AuCommNotify(avatarName, msg, auApp->Frame());
				n->show();					// Be free, little widget.
			}
		}
	}

	// Log channel messages.
	{
		InitLog();

		QString logFileName = LogFileName();
		QFile logFile(logFileName);
		if(logFile.open(QIODevice::Append))
		{
			QTextStream ts(&logFile);

			ts
#if defined(Q_OS_WIN)
				<< "\r\n"
#else
				<< "\n"
#endif
				<< msg.FromName() << ' ' << msg.FromId().StringId() << ','
				<< ' ' << msg.TypeName() << ','
				<< ' ' << AuCommStdChannel::RangeName(msg.ReceptionRange()) << tr(" range") << ','
				<< ' ' << AuTime::Now().UTC().toString() << ','
#if defined(Q_OS_WIN)
				<< "\r\n"
#else
				<< "\n"
#endif
				<< ' ' << msg.Text()
#if defined(Q_OS_WIN)
				<< "\r\n";
#else
				<< "\n";
#endif

			logFile.close();
		}
	}

	if(!isVisible())
		m_UnseenChat = true;
}

void AuCommChannel::RangeClose()
{
	SetRange(AuCommStdChannel::Close);
}

void AuCommChannel::RangeGalaxy()
{
	SetRange(AuCommStdChannel::Galaxy);
}

void AuCommChannel::RangeUniverse()
{
	SetRange(AuCommStdChannel::Universe);
}

void AuCommChannel::RangeHail()
{
	SetRange(AuCommStdChannel::Hail);
}

void AuCommChannel::RangeSector()
{
	SetRange(AuCommStdChannel::Sector);
}

void AuCommChannel::RangeSystem()
{
	SetRange(AuCommStdChannel::SolarSystem);
}

void AuCommChannel::SetRange(AuCommStdChannel::RANGE range)
{
	m_RangeSetting = range;
	m_Range->setIcon(QIcon(AuCommStdChannel::RangeIcon(m_RangeSetting)));
	m_Range->setToolTip(tr("<b>%1 Range</b><br>%2")
		.arg(AuCommStdChannel::RangeName(m_RangeSetting))
		.arg(AuCommStdChannel::RangeDesc(m_RangeSetting)));
}

void AuCommChannel::SelectionChanged()
{
	emit MessageSelectionChanged();
}

// true if any message is selected
bool AuCommChannel::IsMessageSelected() const
{
	return m_Transcript->IsMessageSelected();
}

void AuCommChannel::Accept()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	// Accept Messages:
	// Left column is the message accepted. Right column is the response generated.
	// 	Bid:			Accept
	// 	BidBarter:		AcceptBarter
	// 	BidDesign:		AcceptDesign
	//	InviteTeleport:	AcceptTeleport
	//	InviteToEmpire:	AcceptInvitation
	//	RequestBerthXX:	AcceptBerthXX
	//	BoardColonists:	ColonistsBoarded
	//	DebarkColonists:	ColonistsDebarked
	//	BoardTroops:	TroopsBoarded
	//	OfferRepairAll: AcceptRepairAll
	//	OfferRepairSystem: AcceptRepairSystem
	// The buyer name can't be determined locally.
	switch(SelectedMessageType())
	{
	case AuCommPacket::OfferRefit:
		{
			// Locate the ship the scene atom is aboard.
			if(!view->IsPersonInCommand())
				break;

			// Scene atom is an officer of the ship they are aboard.
			AuId shipId;
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
				if(ship)
					shipId = ship->Id();
			}
			view->UnlockSceneAtom();
			if(shipId != m_Transcript->SelectedMessageRepairShipId())
				break;	// They changed ships? How clever.

			AuRefitShip refit;
			m_Transcript->SelectedMessageRefit(refit);
			double estCost = m_Transcript->SelectedMessageEstCost();
			quint32 estSecs = m_Transcript->SelectedMessageEstSeconds();
			AuId cityId = m_Transcript->SelectedMessageFromId();

			AuCommAcceptRefit msg(cityId, shipId, refit, estCost, estSecs,
					objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
					m_RangeSetting, m_Scope);
			SendChat(msg);
		}
		break;

	case AuCommPacket::ChatChoice:
	case AuCommPacket::NarrateChoice:
		{
			const QItemSelectionModel* ism = m_Transcript->selectionModel();
			if(ism)
			{
				QModelIndexList sel = ism->selectedIndexes();
				if(!sel.isEmpty())
				{
					QModelIndex idxsel = sel.first();
					MessageActivated(idxsel);
				}
			}
		}
		break;

	case AuCommPacket::MissionNew:
		AcceptNewMission();
		break;
	case AuCommPacket::MissionOrderAdd:
		AcceptAddMissionOrder();
		break;
	case AuCommPacket::MissionOrderInsert:
		AcceptInsertMissionOrder();
		break;
	case AuCommPacket::MissionOrderNew:
		AcceptNewMissionOrder();
		break;
	case AuCommPacket::MissionProperty:
		AcceptMissionProperty();
		break;

	case AuCommPacket::Bid:
		{
			AuItem* item = m_Transcript->SelectedMessageItem();
			if(item)
			{
				quint32 price = m_Transcript->SelectedMessagePrice();
				AuId buyerId = m_Transcript->SelectedMessageFromId();
				QString buyerName = m_Transcript->SelectedMessageFromName();
				QString empireNameOfOrigin = m_Transcript->SelectedMessageEmpireOfOrigin();

				AuCommAccept msg(buyerId, buyerName, *item, empireNameOfOrigin, price,
						objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
						m_RangeSetting, m_Scope);
				SendChat(msg);

				delete item;
			}
		}
		break;

	case AuCommPacket::BidBarter:
		{
			AuItem* item = m_Transcript->SelectedMessageItem();
			if(item)
			{
				AuItem* offerItem = m_Transcript->SelectedMessageOfferItem();
				if(offerItem)
				{
					AuId buyerId = m_Transcript->SelectedMessageFromId();
					QString buyerName = m_Transcript->SelectedMessageFromName();
					QString empireNameOfOrigin = m_Transcript->SelectedMessageEmpireOfOrigin();

					QString offerEmpireNameOfOrigin = m_Transcript->SelectedMessageOfferEmpireOfOrigin();

					AuCommAcceptBarter msg(buyerId, buyerName, *item, empireNameOfOrigin, *offerItem, offerEmpireNameOfOrigin,
							objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
							m_RangeSetting, m_Scope);
					SendChat(msg);

					delete offerItem;
				}

				delete item;
			}
		}
		break;

	case AuCommPacket::BidPickupShipment:
		{
			AuShipment shipment;
			if(!m_Transcript->SelectedMessageShipment(shipment))
				break;
			AuId buyerId = m_Transcript->SelectedMessageFromId();
			QString buyerName = m_Transcript->SelectedMessageFromName();
			AuCommAcceptPickupShipment msg(buyerId, buyerName, shipment, objectName(), view->SceneAtomId(), view->SceneAtomName(),
					view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
			SendChat(msg);
		}
		break;

	case AuCommPacket::OfferAvatar:
		{
			AuId avatarId = m_Transcript->SelectedMessageFromId();
			QString avatarName = m_Transcript->SelectedMessageFromName();

			AuCommAcceptAvatar msg(avatarId, avatarName,
					objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
					m_RangeSetting, m_Scope);
			SendChat(msg);
		}
		break;

	case AuCommPacket::CitySurrenderOffered:
		{
			quint32 acceptingEmpireId = m_Transcript->SelectedMessageToEmpireId();
			if(acceptingEmpireId == view->SceneAtomEmpire())
			{
				AuId surrenderingAvatarId = m_Transcript->SelectedMessageFromId();
				quint32 surrenderingEmpireId = m_Transcript->SelectedMessageFromEmpireId();
				AuIdList cityIds;
				m_Transcript->SelectedMessageCityIds(cityIds);

				// From/to reversed when we send it back, now it's from me, to the surrendering empire.
				AuCommCitySurrenderAccepted msg(acceptingEmpireId, surrenderingAvatarId, surrenderingEmpireId, cityIds,
						objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
						m_RangeSetting, m_Scope);
				SendChat(msg);
			}
		}
		break;

		/*
	case AuCommPacket::InviteTeleport:
		{
			AuId toId = m_Transcript->SelectedMessageFromId();
			if(toId != view->SceneAtomId())
			{
				AuCommAcceptTeleport msg(toId, objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
						view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
		}
		break;
		*/

	case AuCommPacket::InviteToEmpire:
		{
			AuId toId = m_Transcript->SelectedMessageFromId();
			if(toId != view->SceneAtomId())
			{
				// Cannot accept invitation from myself.
				quint32 fromEmpireId = m_Transcript->SelectedMessageFromEmpireId();

				AuCommAcceptInviteToEmpire msg(toId, fromEmpireId, objectName(), view->SceneAtomId(), view->SceneAtomName(),
						view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
		}
		break;

	case AuCommPacket::RequestBerthAny:
		// Select an available berth slot to assign to the request.
		{
			// Locate the ship the scene atom is aboard.
			bool availOfficer = false;
			bool availCrew = false;
			bool availPassenger = false;
			bool availTroop = false;
			AuId shipId;
			QString shipName;
			bool shipIsPrivatelyOwned = false;
			bool sameEnv = false;
			bool sameEmp = false;

			// Lock the root and find the scene atom.
			if(view->IsPersonInCommand())
			{
				DaPerson* person;
				view->LockSceneAtom(&person);
				if(person)
				{
					// Must be in the same solar system or the same galaxy if in the same empire to accept a berth.
					if(person->EmpireId() == m_Transcript->SelectedMessageFromEmpireId())
					{
						sameEmp = true;
						sameEnv = person->Galaxy() == m_Transcript->SelectedMessageFromGalaxy();
					}
					else
						sameEnv = person->FindEnvironmentAtom()->FindIdRecursive(m_Transcript->SelectedMessageFromId());

					DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
					if(ship && sameEnv)
					{
						// Parent atom is a spaceship.
						shipId = ship->Id();
						shipName = ship->NameWithEmpire();
						shipIsPrivatelyOwned = ship->IsPrivatelyOwned();

						// Scene atom is an officer of the ship they are aboard.
						// Only an officer of the ship can accept berth requests.
						availOfficer = (ship->OfficerBerthsAvailable()) ? true:false;
						availCrew = (ship->CrewBerthsAvailable()) ? true:false;
						availPassenger = (ship->PassengerBerthsAvailable()) ? true:false;
						availTroop = (ship->TroopBerthsAvailable()) ? true:false;
					}
				}
				view->UnlockSceneAtom();
			}

			if(availOfficer || availCrew || availPassenger || availTroop)
			{
				AuId toId = m_Transcript->SelectedMessageFromId();
				QString toName = m_Transcript->SelectedMessageFromName();

				// 'Any Berth' and 'Captain's Berth' options are not available in this case.
				AuAssignBerth assignDlg(false, availOfficer, availCrew, availPassenger, availTroop, false, pFrame);
				int result = auApp->Frame()->ExecDialog(assignDlg);
				if(result == QDialog::Accepted)
				{
					switch(assignDlg.TypeAssigned())
					{
					case AuRosterEntry::Invalid:
					case AuRosterEntry::Captain:
					case AuRosterEntry::CaptainSlot:
					case AuRosterEntry::OfficerAI:
					case AuRosterEntry::PassengerAI:
						// Not valid return value from AuAssignBerth dialog.
						Q_ASSERT(false);
						break;
					case AuRosterEntry::Crew:
						{
							AuCommAcceptBerthCrew msg(toId, toName, shipId, shipName, shipIsPrivatelyOwned,
									objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
									view->SceneGalaxy(), m_RangeSetting,
									m_Scope);
							SendChat(msg);
						}
						break;
					case AuRosterEntry::OfficerHuman:
						{
							AuCommAcceptBerthOfficer msg(toId, toName, shipId, shipName, shipIsPrivatelyOwned,
									objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
									view->SceneGalaxy(), m_RangeSetting,
									m_Scope);
							SendChat(msg);
						}
						break;
					case AuRosterEntry::PassengerHuman:
						{
							AuCommAcceptBerthPassenger msg(toId, toName, shipId, shipName, shipIsPrivatelyOwned,
									objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
									view->SceneGalaxy(), m_RangeSetting,
									m_Scope);
							SendChat(msg);
						}
						break;
					case AuRosterEntry::Troop:
						{
							AuCommAcceptBerthTroop msg(toId, toName, shipId, shipName, shipIsPrivatelyOwned,
									objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
									view->SceneGalaxy(), m_RangeSetting,
									m_Scope);
							SendChat(msg);
						}
						break;
					}
				}
			}
			else if(!sameEnv)
			{
				if(sameEmp)
				{
					QMessageBox mb(g_Au.ApplicationName(), tr("You cannot accept a berth request from a distant galaxy, "
								"when the requestor is in your empire."),
							QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
					auApp->Frame()->ExecMessageBox(mb);
				}
				else
				{
					QMessageBox mb(g_Au.ApplicationName(), tr("You cannot accept a berth request from a distant solar system, "
								"when the requestor is not in your empire."),
							QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
					auApp->Frame()->ExecMessageBox(mb);
				}
			}
			else
			{
				QMessageBox mb(g_Au.ApplicationName(), tr("The ship has no empty berths."),
						QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
				auApp->Frame()->ExecMessageBox(mb);
			}
		}
		break;

	case AuCommPacket::RequestBerthOfficer:
		{
			// Locate the ship the scene atom is aboard.
			// Lock the root and find the scene atom.
			if(view->IsPersonInCommand())
			{
				// Only an officer of the ship can accept berth requests.
				bool anyEmptyBerth = false;
				bool sameEnv = false;
				bool sameEmp = false;
				DaPerson* person;
				view->LockSceneAtom(&person);
				if(person)
				{
					// Must be in the same solar system or the same galaxy if in the same empire to accept a berth.
					if(person->EmpireId() == m_Transcript->SelectedMessageFromEmpireId())
					{
						sameEmp = true;
						sameEnv = person->Galaxy() == m_Transcript->SelectedMessageFromGalaxy();
					}
					else
						sameEnv = person->FindEnvironmentAtom()->FindIdRecursive(m_Transcript->SelectedMessageFromId());

					DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
					if(ship && sameEnv)
					{
						// Parent atom is a spaceship.
						if(ship->OfficerBerthsAvailable())
						{
							// Berth is available and the scene atom is an officer of the ship.
							anyEmptyBerth = true;
							AuId toId = m_Transcript->SelectedMessageFromId();
							QString toName = m_Transcript->SelectedMessageFromName();

							AuCommAcceptBerthOfficer msg(toId, toName, ship->Id(), ship->NameWithEmpire(), ship->IsPrivatelyOwned(),
									objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
									view->SceneGalaxy(), m_RangeSetting,
									m_Scope);
							SendChat(msg);
						}
					}
				}
				view->UnlockSceneAtom();

				if(!sameEnv)
				{
					if(sameEmp)
					{
						QMessageBox mb(g_Au.ApplicationName(), tr("You cannot accept a berth request from a distant galaxy, "
									"when the requestor is in your empire."),
								QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
						auApp->Frame()->ExecMessageBox(mb);
					}
					else
					{
						QMessageBox mb(g_Au.ApplicationName(), tr("You cannot accept a berth request from a distant solar system, "
									"when the requestor is not in your empire."),
								QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
						auApp->Frame()->ExecMessageBox(mb);
					}
				}
				else if(!anyEmptyBerth)
				{
					QMessageBox mb(g_Au.ApplicationName(), tr("The ship has no empty officer berths."),
							QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
					auApp->Frame()->ExecMessageBox(mb);
				}
			}
		}
		break;

	case AuCommPacket::RequestBerthCrew:
		{
			// Locate the ship the scene atom is aboard.
			// Lock the root and find the scene atom.
			AuView* pView = auApp->Frame()->View();
			if(pView->IsPersonInCommand())
			{
				bool anyEmptyBerth = false;
				bool sameEnv = false;
				bool sameEmp = false;
				DaPerson* person;
				pView->LockSceneAtom(&person);
				if(person)
				{
					// Must be in the same solar system or the same galaxy if in the same empire to accept a berth.
					if(person->EmpireId() == m_Transcript->SelectedMessageFromEmpireId())
					{
						sameEmp = true;
						sameEnv = person->Galaxy() == m_Transcript->SelectedMessageFromGalaxy();
					}
					else
						sameEnv = person->FindEnvironmentAtom()->FindIdRecursive(m_Transcript->SelectedMessageFromId());

					DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
					if(ship && sameEnv)
					{
						// Parent atom is a spaceship.
						if(ship->CrewBerthsAvailable())
						{
							// Berth is available and the scene atom is an officer of the ship.
							// Only an officer of the ship can accept berth requests.
							anyEmptyBerth = true;
							AuId toId = m_Transcript->SelectedMessageFromId();
							QString toName = m_Transcript->SelectedMessageFromName();

							AuCommAcceptBerthCrew msg(toId, toName, ship->Id(), ship->NameWithEmpire(), ship->IsPrivatelyOwned(),
									objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
									view->SceneGalaxy(), m_RangeSetting,
									m_Scope);
							SendChat(msg);
						}
					}
				}
				pView->UnlockSceneAtom();

				if(!sameEnv)
				{
					if(sameEmp)
					{
						QMessageBox mb(g_Au.ApplicationName(), tr("You cannot accept a berth request from a distant galaxy, "
									"when the requestor is in your empire."),
								QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
						auApp->Frame()->ExecMessageBox(mb);
					}
					else
					{
						QMessageBox mb(g_Au.ApplicationName(), tr("You cannot accept a berth request from a distant solar system, "
									"when the requestor is not in your empire."),
								QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
						auApp->Frame()->ExecMessageBox(mb);
					}
				}
				else if(!anyEmptyBerth)
				{
					QMessageBox mb(g_Au.ApplicationName(), tr("The ship has no empty crew berths."),
							QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
					auApp->Frame()->ExecMessageBox(mb);
				}
			}
		}
		break;
	case AuCommPacket::RequestBerthPassenger:
		{
			// Locate the ship the scene atom is aboard.
			// Lock the root and find the scene atom.
			AuView* pView = auApp->Frame()->View();
			if(pView->IsPersonInCommand())
			{
				bool anyEmptyBerth = false;
				bool sameEnv = false;
				bool sameEmp = false;
				DaPerson* person;
				pView->LockSceneAtom(&person);
				if(person)
				{
					// Must be in the same solar system or the same galaxy if in the same empire to accept a berth.
					if(person->EmpireId() == m_Transcript->SelectedMessageFromEmpireId())
					{
						sameEmp = true;
						sameEnv = person->Galaxy() == m_Transcript->SelectedMessageFromGalaxy();
					}
					else
						sameEnv = person->FindEnvironmentAtom()->FindIdRecursive(m_Transcript->SelectedMessageFromId());

					DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
					if(ship && sameEnv)
					{
						// Parent atom is a spaceship.
						if(ship->PassengerBerthsAvailable())
						{
							// Berth is available and the scene atom is an officer of the ship.
							// Only an officer of the ship can accept berth requests.
							anyEmptyBerth = true;
							AuId toId = m_Transcript->SelectedMessageFromId();
							QString toName = m_Transcript->SelectedMessageFromName();

							AuCommAcceptBerthPassenger msg(toId, toName, ship->Id(), ship->NameWithEmpire(),
									ship->IsPrivatelyOwned(),
									objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
									view->SceneGalaxy(), m_RangeSetting,
									m_Scope);
							SendChat(msg);
						}
					}
				}
				pView->UnlockSceneAtom();

				if(!sameEnv)
				{
					if(sameEmp)
					{
						QMessageBox mb(g_Au.ApplicationName(), tr("You cannot accept a berth request from a distant galaxy, "
									"when the requestor is in your empire."),
								QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
						auApp->Frame()->ExecMessageBox(mb);
					}
					else
					{
						QMessageBox mb(g_Au.ApplicationName(), tr("You cannot accept a berth request from a distant solar system, "
									"when the requestor is not in your empire."),
								QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
						auApp->Frame()->ExecMessageBox(mb);
					}
				}
				else if(!anyEmptyBerth)
				{
					QMessageBox mb(g_Au.ApplicationName(), tr("The ship has no empty passenger berths."),
							QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
					auApp->Frame()->ExecMessageBox(mb);
				}
			}
		}
		break;

	case AuCommPacket::RequestBerthTroop:
		{
			// Locate the ship the scene atom is aboard.
			// Lock the root and find the scene atom.
			AuView* pView = auApp->Frame()->View();
			if(pView->IsPersonInCommand())
			{
				bool anyEmptyBerth = false;
				bool sameEnv = false;
				bool sameEmp = false;
				DaPerson* person;
				pView->LockSceneAtom(&person);
				if(person)
				{
					// Must be in the same solar system or the same galaxy if in the same empire to accept a berth.
					if(person->EmpireId() == m_Transcript->SelectedMessageFromEmpireId())
					{
						sameEmp = true;
						sameEnv = person->Galaxy() == m_Transcript->SelectedMessageFromGalaxy();
					}
					else
						sameEnv = person->FindEnvironmentAtom()->FindIdRecursive(m_Transcript->SelectedMessageFromId());

					DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
					if(ship && sameEnv)
					{
						// Parent atom is a spaceship.
						if(ship->TroopBerthsAvailable())
						{
							// Berth is available and the scene atom is an officer of the ship.
							// Only an officer of the ship can accept berth requests.
							anyEmptyBerth = true;
							AuId toId = m_Transcript->SelectedMessageFromId();
							QString toName = m_Transcript->SelectedMessageFromName();

							AuCommAcceptBerthTroop msg(toId, toName, ship->Id(), ship->NameWithEmpire(), ship->IsPrivatelyOwned(),
									objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
									view->SceneGalaxy(), m_RangeSetting,
									m_Scope);
							SendChat(msg);
						}
					}
				}
				pView->UnlockSceneAtom();

				if(!sameEnv)
				{
					if(sameEmp)
					{
						QMessageBox mb(g_Au.ApplicationName(), tr("You cannot accept a berth request from a distant galaxy, "
									"when the requestor is in your empire."),
								QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
						auApp->Frame()->ExecMessageBox(mb);
					}
					else
					{
						QMessageBox mb(g_Au.ApplicationName(), tr("You cannot accept a berth request from a distant solar system, "
									"when the requestor is not in your empire."),
								QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
						auApp->Frame()->ExecMessageBox(mb);
					}
				}
				else if(!anyEmptyBerth)
				{
					QMessageBox mb(g_Au.ApplicationName(), tr("The ship has no empty troop berths."),
							QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
					auApp->Frame()->ExecMessageBox(mb);
				}
			}
		}
		break;

	case AuCommPacket::BoardCryo:
		{
			// A BoardCryo message is selected.
			// Someone wants cryo travellers. Accepting their request enables me to
			// transfer my cryo travellers to their vessel, if they're in range.
			unsigned int cryoAboard = 0;
			bool hasTransporter = false;
			QStringList cryoSpecies;

			// Lock the root and find the scene atom.
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person && person->Id() != m_Transcript->SelectedMessageFromId())
			{
				DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
				if(ship)
				{
					// Parent atom is a spaceship.
					if(view->IsPersonInCommand())
					{
						// Scene atom is an officer of the ship they are aboard.
						// Only an officer of the ship can unload passengers.
						cryoAboard = ship->CryoBerthsOccupied(cryoSpecies);
						hasTransporter = ship->MaxTransporterRange() > ship->RoughRadius();
					}
				}
			}
			view->UnlockSceneAtom();

			if(!cryoAboard)
				return;

			AuUnloadColonists dlg(cryoAboard, 0, 0, false, pFrame);
			int result = auApp->Frame()->ExecDialog(dlg);
			if(result != QDialog::Accepted)
				return;
			Q_ASSERT(dlg.CryoUnloaded() > 0 && dlg.CryoUnloaded() <= cryoAboard);
			
			AuId toId = m_Transcript->SelectedMessageFromId();

			AuCommCryoBoarded msg(toId, dlg.CryoUnloaded(), hasTransporter, cryoSpecies,
					objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
					m_RangeSetting, m_Scope);
			SendChat(msg);
		}
		break;

	case AuCommPacket::BoardPassengers:
		{
			// A BoardColonists message is selected.
			// Someone wants colonists. Accepting their request enables me to
			// transfer my colonists to their vessel, if they're in range.
			unsigned int passengersAboard = 0;
			bool isInVehicle = false;
			bool hasTransporter = false;

			// Lock the root and find the scene atom.
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person && person->Id() != m_Transcript->SelectedMessageFromId())
			{
				DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
				if(ship)
				{
					// Parent atom is a spaceship.
					if(view->IsPersonInCommand())
					{
						// Scene atom is an officer of the ship they are aboard.
						// Only an officer of the ship can unload passengers.
						passengersAboard = ship->ColonistsAboard();
						isInVehicle = false;
						hasTransporter = ship->MaxTransporterRange() > ship->RoughRadius();
					}
				}
				else
				{
					DaVehicle* vehicle = dynamic_cast<DaVehicle*>(person->Parent());
					if(vehicle)
					{
						// Parent atom is a vehicle.
						if(vehicle->IsControlSeat(person->VehicleSeatIndex()))
						{
							// Scene atom is in a control seat of the vehicle.
							passengersAboard = vehicle->ColonistsAboard();
							isInVehicle = true;
						}
					}
				}
			}
			view->UnlockSceneAtom();

			if(!passengersAboard)
				return;

			AuUnloadColonists dlg(0, passengersAboard, 0, isInVehicle, pFrame);
			int result = auApp->Frame()->ExecDialog(dlg);
			if(result != QDialog::Accepted)
				return;
			Q_ASSERT(dlg.PassengersUnloaded() > 0 && dlg.PassengersUnloaded() <= passengersAboard);
			
			AuId toId = m_Transcript->SelectedMessageFromId();

			AuCommPassengersBoarded msg(toId, dlg.PassengersUnloaded(), hasTransporter,
					objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
					m_RangeSetting, m_Scope);
			SendChat(msg);
		}
		break;

	case AuCommPacket::DebarkCryo:
		{
			// A DebarkCryo message is selected.
			// Someone has offered to unload some cryo travellers. Accepting their
			// unload enables me to transfer those cryo travellers to my vessel, if they're in range.
			unsigned int openStateroomBerths = 0;
			bool hasTransporter = false;

			// Lock the root and find the scene atom.
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person && person->Id() != m_Transcript->SelectedMessageFromId())
			{
				DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
				if(ship)
				{
					// Parent atom is a spaceship.
					if(view->IsPersonInCommand())
					{
						// Scene atom is an officer of the ship they are aboard.
						// Only an officer of the ship can load cryo travellers.
						openStateroomBerths = ship->CryoBerthsAvailable();
						hasTransporter = ship->MaxTransporterRange() > ship->RoughRadius();
					}
				}
			}
			view->UnlockSceneAtom();

			if(!openStateroomBerths)
				return;

			// Limit the number I can accept to the count unloaded.
			unsigned int colonistsUnloaded = m_Transcript->SelectedMessageCount();
			if(openStateroomBerths > colonistsUnloaded)
				openStateroomBerths = colonistsUnloaded;

			AuLoadColonists dlg(0, 0, openStateroomBerths, 0, 0, false, pFrame);
			int result = auApp->Frame()->ExecDialog(dlg);
			if(result != QDialog::Accepted)
				return;
			
			Q_ASSERT(dlg.CryoLoaded() > 0 &&
					dlg.CryoLoaded() <= openStateroomBerths &&
					dlg.CryoLoaded() <= colonistsUnloaded);

			AuId toId = m_Transcript->SelectedMessageFromId();

			QStringList cryoSpecies;	// We don't know what they are offering.
			AuCommCryoDebarked msg(toId, dlg.CryoLoaded(), hasTransporter, cryoSpecies,
					objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
					m_RangeSetting, m_Scope);
			SendChat(msg);
		}
		break;

	case AuCommPacket::DebarkPassengers:
		{
			// A DebarkColonists message is selected.
			// Someone has offered to unload some passengers. Accepting their
			// unload enables me to transfer those passengers to my vessel, if they're in range.
			unsigned int openStateroomBerths = 0;
			bool isInVehicle = false;
			bool hasTransporter = false;

			// Lock the root and find the scene atom.
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person && person->Id() != m_Transcript->SelectedMessageFromId())
			{
				DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
				if(ship)
				{
					// Parent atom is a spaceship.
					if(view->IsPersonInCommand())
					{
						// Scene atom is an officer of the ship they are aboard.
						// Only an officer of the ship can load passengers.
						openStateroomBerths = ship->PassengerBerthsAvailable();
						isInVehicle = false;
						hasTransporter = ship->MaxTransporterRange() > ship->RoughRadius();
					}
				}
				else
				{
					DaVehicle* vehicle = dynamic_cast<DaVehicle*>(person->Parent());
					if(vehicle)
					{
						// Parent atom is a vehicle.
						if(vehicle->IsControlSeat(person->VehicleSeatIndex()))
						{
							// Scene atom is in a control seat of the vehicle.
							openStateroomBerths = vehicle->EmptySeats();
							isInVehicle = true;
						}
					}
				}
			}
			view->UnlockSceneAtom();

			if(!openStateroomBerths)
				return;

			// Limit the number I can accept to the count unloaded.
			unsigned int colonistsUnloaded = m_Transcript->SelectedMessageCount();
			if(openStateroomBerths > colonistsUnloaded)
				openStateroomBerths = colonistsUnloaded;

			AuLoadColonists dlg(0, 0, 0, openStateroomBerths, 0, isInVehicle, pFrame);
			int result = auApp->Frame()->ExecDialog(dlg);
			if(result != QDialog::Accepted)
				return;
			
			Q_ASSERT(dlg.PassengersLoaded() > 0 &&
					dlg.PassengersLoaded() <= openStateroomBerths &&
					dlg.PassengersLoaded() <= colonistsUnloaded);

			AuId toId = m_Transcript->SelectedMessageFromId();

			AuCommPassengersDebarked msg(toId, dlg.PassengersLoaded(), 0, hasTransporter,
					objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
					m_RangeSetting, m_Scope);
			SendChat(msg);
		}
		break;

	case AuCommPacket::BoardTroops:
		/*
		{
			// A BoardTroops message is selected.
			// Someone wants troops. Accepting their request enables me to
			// transfer my troops to their vessel, if they're in range.
			unsigned int troopsAboard = 0;
			bool isInVehicle = false;

			// Lock the root and find the scene atom.
			AuView* pView = auApp->Frame()->View();
			pView->LockSceneAtom();
			DaPerson* person = pView->SceneAtom();
			if(person && person->Id() != m_Transcript->SelectedMessageFromId())
			{
				DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
				if(ship)
				{
					// Parent atom is a spaceship.
					if(pView->IsPersonInCommand())
					{
						// Scene atom is an officer of the ship they are aboard.
						// Only an officer of the ship can unload troops.
						troopsAboard = ship->TroopsAboard();
						isInVehicle = false;
					}
				}
				else
				{
					DaVehicle* vehicle = dynamic_cast<DaVehicle*>(person->Parent());
					if(vehicle)
					{
						// Parent atom is a vehicle.
						if(vehicle->IsControlSeat(person->VehicleSeatIndex()))
						{
							// Scene atom is in a control seat of the vehicle.
							troopsAboard = vehicle->TroopsAboard();
							isInVehicle = true;
						}
					}
				}
			}
			pView->UnlockSceneAtom();

			if(!troopsAboard)
				return;

			AuUnloadColonists dlg(0, 0, troopsAboard, isInVehicle, pFrame);
			int result = pView->ExecDialog(dlg);
			if(result != QDialog::Accepted)
				return;
			
			Q_ASSERT(dlg.TroopsUnloaded() > 0 && dlg.TroopsUnloaded() <= troopsAboard);

			// Text of TroopsBoarded message is...
			//	count|loaderId
			toId = m_Transcript->SelectedMessageFromId();
			text = QObject::tr("%1|%2").arg(dlg.TroopsUnloaded()).arg(toId.GetID());
			type = AuCommMessage::TroopsBoarded;
		}
		*/
		break;
	case AuCommPacket::OfferUpgradeInstall:
		// An OfferUpgradeInstall message is selected.
		// These messages only originate from cities.
		{
			AuItem* item = m_Transcript->SelectedMessageItem();
			if(item)
			{
				QString empireNameOfOrigin = m_Transcript->SelectedMessageEmpireOfOrigin();
				quint16 systemWhich = m_Transcript->SelectedMessageSystemWhich();
				quint16 installSecs = m_Transcript->SelectedMessageInstallSecs();
				quint32 price = m_Transcript->SelectedMessagePrice();

				AuCommAcceptUpgradeInstall msg(*item, empireNameOfOrigin, systemWhich, installSecs, price,
						objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
						m_RangeSetting, m_Scope);
				SendChat(msg);

				delete item;
			}
		}
		break;

	case AuCommPacket::OfferRepairAll:
		// An OfferRepairXXX message is selected.
		// These messages only originate from cities.
		// The person must be aboard the ship to which the offer is directed.
		if(m_Transcript->SelectedMessageRepairShipId() == view->SceneAtomInShip())
		{
			AuId buyerId = m_Transcript->SelectedMessageFromId();
			QString buyerName = m_Transcript->SelectedMessageFromName();
			AuId cityId = m_Transcript->SelectedMessageCityId();
			quint32 price = m_Transcript->SelectedMessagePrice();
			quint16 repairSecs = m_Transcript->SelectedMessageRepairSecs();
			QList<COMMODITYID> noQ, shortQ, noSupply, enough;
			m_Transcript->SelectedMessageMissingParts(noQ, shortQ, noSupply, enough);
			quint8 repairQ = m_Transcript->SelectedMessageMinQ();

			AuCommAcceptRepairAll msg(view->SceneAtomInShip(), buyerId, buyerName, cityId, price, repairSecs, noQ, shortQ, noSupply,
					enough, repairQ, objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
					view->SceneGalaxy(), m_RangeSetting, m_Scope);
			SendChat(msg);
		}
		break;

	case AuCommPacket::OfferRepairSystem:
		// An OfferRepairXXX message is selected.
		// These messages only originate from cities.
		// The person must be aboard the ship to which the offer is directed.
		if(m_Transcript->SelectedMessageRepairShipId() == view->SceneAtomInShip())
		{
			AuId buyerId = m_Transcript->SelectedMessageFromId();
			QString buyerName = m_Transcript->SelectedMessageFromName();
			AuId cityId = m_Transcript->SelectedMessageCityId();
			quint8 system = m_Transcript->SelectedMessageSystem();
			quint16 systemWhich = m_Transcript->SelectedMessageSystemWhich();
			QString systemDesc = m_Transcript->SelectedMessageSystemDesc();
			quint32 price = m_Transcript->SelectedMessagePrice();
			quint16 repairSecs = m_Transcript->SelectedMessageRepairSecs();
			QList<COMMODITYID> noQ, shortQ, noSupply, enough;
			m_Transcript->SelectedMessageMissingParts(noQ, shortQ, noSupply, enough);
			quint8 repairQ = m_Transcript->SelectedMessageMinQ();

			AuCommAcceptRepairSystem msg(view->SceneAtomInShip(), buyerId, buyerName, cityId, system, systemWhich, systemDesc,
					price, repairSecs, noQ, shortQ, noSupply, enough, repairQ, objectName(), view->SceneAtomId(),
					view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
			SendChat(msg);
		}
		break;

	case AuCommPacket::RequestHelp:
		{
			const DgData& gd = g_Au.LockDataForRead();
			bool isAgent = gd.ClientPlayerIsAgent();
			g_Au.UnlockData();
			if(isAgent)
			{
				// Reply with a canned acceptance message.
				/*
				AuCommChat msg(tr("Help request acknowledged. "
							"I will be off line for a minute while I move to your location. "
							"Please wait."),
						objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
						m_RangeSetting, m_Scope);
				SendChat(msg);
				*/

				emit HelpRequestAccepted(m_Transcript->SelectedMessageFromId(), m_Transcript->SelectedMessageFromName());
			}
		}
		break;

	case AuCommPacket::HailRequest:
		OpenChannelToSender();
		break;

	case AuCommPacket::PreBoard:
		// A pre-boarding announcement is selected. These messages originate from spacecraft when loading passengers.
		// PreBoard fields: minutes|berths|class|shipId|shipName|locType|locName|locLongLat|nextStop|berthType
		// berthType is 0 for passenger berth, 1 for troop berth.
		// Reply with a request for the appropriate berth type.
		{
			// Get governing credentials of person.
			quint32 personEmpire = view->SceneAtomEmpire();

			// 1=1stCl pass, 2=2ndCl pass, 3=3rdCl pass, 4=troop, 5=crew
			int preBoardBerthType = m_Transcript->SelectedMessagePreBoardBerthType();
			if(preBoardBerthType >= 1 && preBoardBerthType <= 3)
			{
				AuCommRequestBerthPassenger msg(personEmpire, objectName(), view->SceneAtomId(), view->SceneAtomName(),
						view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			else if(preBoardBerthType == 4)
			{
				AuCommRequestBerthTroop msg(personEmpire, objectName(), view->SceneAtomId(), view->SceneAtomName(),
						view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			else if(preBoardBerthType == 5)
			{
				AuCommRequestBerthCrew msg(personEmpire, objectName(), view->SceneAtomId(), view->SceneAtomName(),
						view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
		}
		break;

	case AuCommPacket::Auction:
	case AuCommPacket::ForSale:
		{
			AuCommThought mymsg(tr("I should make an offer. Right-click to proceed with the transaction."),
					view->SceneAtomId(), view->SceneAtomName());
			SendChat(mymsg);
		}
		break;

	case AuCommPacket::Accept:
	case AuCommPacket::AcceptAvatar:
	case AuCommPacket::AcceptBarter:
	case AuCommPacket::AcceptBerthCaptain:
	case AuCommPacket::AcceptBerthCaptainSlot:
	case AuCommPacket::AcceptBerthCrew:
	case AuCommPacket::AcceptBerthOfficer:
	case AuCommPacket::AcceptBerthPassenger:
	case AuCommPacket::AcceptBerthTroop:
	case AuCommPacket::AcceptInviteToEmpire:
	case AuCommPacket::AcceptPickupShipment:
	case AuCommPacket::AcceptRefit:
	case AuCommPacket::AcceptRepairAll:
	case AuCommPacket::AcceptRepairSystem:
	case AuCommPacket::AcceptTeleport:
	case AuCommPacket::AcceptUpgradeInstall:
	case AuCommPacket::Announcement:
	case AuCommPacket::BeamAtom:
	case AuCommPacket::BeamCoordinate:
	case AuCommPacket::BeamOver:
	case AuCommPacket::BeamPlanet:
	case AuCommPacket::BeamShip:
	case AuCommPacket::BeamTransporter:
	case AuCommPacket::BeamVehicleAtom:
	case AuCommPacket::BeamVehicleCoordinate:
	case AuCommPacket::BeamVehicleOver:
	case AuCommPacket::BeamVehiclePlanet:
	case AuCommPacket::BeamVehicleShip:
	case AuCommPacket::BeamVehicleTransporter:
	case AuCommPacket::BoardCrew:
	case AuCommPacket::BoardOfficers:
	case AuCommPacket::ChannelOpened:
	case AuCommPacket::Chat:
	case AuCommPacket::ChatAudio:
	case AuCommPacket::ChatContinued:
	case AuCommPacket::ChatImage:
	case AuCommPacket::ChatStory:
	case AuCommPacket::ChoicePicked:
	case AuCommPacket::CitizenOrder:
	case AuCommPacket::CitizenPosture:
	case AuCommPacket::CityChat:
	case AuCommPacket::CityMessage:
	case AuCommPacket::CitySurrenderAccepted:
	case AuCommPacket::Combine:
	case AuCommPacket::ContactReply:
	case AuCommPacket::ContactShip:
	case AuCommPacket::CrewBoarded:
	case AuCommPacket::CrewBoardingComplete:
	case AuCommPacket::CryoBoarded:
	case AuCommPacket::CryoBoardingComplete:
	case AuCommPacket::CryoDebarked:
	case AuCommPacket::DamageInflicted:
	case AuCommPacket::Detonate:
	case AuCommPacket::DiplomaticMessage:
	case AuCommPacket::FiringAtTarget:
	case AuCommPacket::HailReply:
	case AuCommPacket::Harvest:
	case AuCommPacket::InventoryItem:
	case AuCommPacket::InviteTeleport:
	case AuCommPacket::Jettison:
	case AuCommPacket::Message:
	case AuCommPacket::Narrate:
	case AuCommPacket::NarrateAudio:
	case AuCommPacket::NarrateImage:
	case AuCommPacket::NarrateTitle:
	case AuCommPacket::NarrateVideo:
	case AuCommPacket::OfferTEU:
	case AuCommPacket::OfficerBoarded:
	case AuCommPacket::OpenTEU:
	case AuCommPacket::PassengerBoardingComplete:
	case AuCommPacket::PassengersBoarded:
	case AuCommPacket::PassengersDebarked:
	case AuCommPacket::RepairPartsMissing:
	case AuCommPacket::RequestBerthCaptain:
	case AuCommPacket::RequestBerthCaptainSlot:
	case AuCommPacket::RequestExterminator:
	case AuCommPacket::RequestHouseKeeping:
	case AuCommPacket::RequestInventory:
	case AuCommPacket::RequestRefit:
	case AuCommPacket::RequestRepairAll:
	case AuCommPacket::RequestRepairSystem:
	case AuCommPacket::RequestUpgradeSystem:
	case AuCommPacket::RevokeBid:
	case AuCommPacket::SOSAttack:
	case AuCommPacket::SOSFuelMarooned:
	case AuCommPacket::SOSFuelShip:
	case AuCommPacket::SOSFuelStation:
	case AuCommPacket::SOSManeuverDriveAdrift:
	case AuCommPacket::SOSManeuverDriveMarooned:
	case AuCommPacket::SOSManeuverDriveStation:
	case AuCommPacket::ShipStasisWait:
	case AuCommPacket::ShipStasisDimension:
	case AuCommPacket::ShipmentItem:
	case AuCommPacket::Silverwill:
	case AuCommPacket::SolicitAllSales:
	case AuCommPacket::SolicitAmmoSales:
	case AuCommPacket::SolicitArmorSales:
	case AuCommPacket::SolicitConstructionMaterialSales:
	case AuCommPacket::SolicitFoodSales:
	case AuCommPacket::SolicitFuelSales:
	case AuCommPacket::SolicitItemSales:
	case AuCommPacket::SolicitMedSales:
	case AuCommPacket::SolicitMiscSales:
	case AuCommPacket::SolicitNaturalResourceSales:
	case AuCommPacket::SolicitPartsSales:
	case AuCommPacket::SolicitPatentSales:
	case AuCommPacket::SolicitShipments:
	case AuCommPacket::SolicitToolSales:
	case AuCommPacket::SolicitWeaponSales:
	case AuCommPacket::SpotTargetOrTerrain:
	case AuCommPacket::Thought:
	case AuCommPacket::ThoughtMailAlert:
	case AuCommPacket::ThoughtMessage:
	case AuCommPacket::TroopBoardingComplete:
	case AuCommPacket::TroopsBoarded:
	case AuCommPacket::CommTypes:
		break;	// Message selected is not valid for accept button
	}
}

void AuCommChannel::HailRequest()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	AuCommHailRequest msg(view->SceneAtomEmpire(), QString(), objectName(), view->SceneAtomId(), view->SceneAtomName(),
			view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
	SendChat(msg);
}

void AuCommChannel::HailRequestComment()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	bool ok;
	QString addText = QInputDialog::getText(this, g_Au.ApplicationName(),
			tr("Enter text to add to the hail message."), QLineEdit::Normal, "", &ok).trimmed();
	if(!ok)
		return;
	if(addText.length() > AuCommChat::MaxChatText())
		addText = addText.left(AuCommChat::MaxChatText()).trimmed();

	AuCommHailRequest msg(view->SceneAtomEmpire(), addText, objectName(), view->SceneAtomId(), view->SceneAtomName(),
			view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
	SendChat(msg);
}

void AuCommChannel::Announce()
{
	HailReply();
}

void AuCommChannel::HailReply()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	AuId systemId;
	DaPerson* fromAtom;
	view->LockSceneAtom(&fromAtom);
	if(fromAtom)
	{
		const DaAtom* system = fromAtom->FindEnvironmentAtom();
		if(system)
			systemId = system->Id();
	}
	view->UnlockSceneAtom();

	AuCommAnnounce dlg(m_Transcript->SelectedMessageFromName(), pFrame);
	int result = pFrame->ExecDialog(dlg);
	if(result == QDialog::Accepted)
	{
		QString msgText = dlg.MessageText().simplified();
		if(!msgText.isEmpty())
		{
			AuCommHailReply msg(msgText, dlg.SystemId(), objectName(), view->SceneAtomId(), view->SceneAtomName(),
					view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
			SendChat(msg);
		}
	}
}

void AuCommChannel::BeamOver()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	// In a transporter room, the player may be eligible for advanced transporter destination options.
	bool inTransporterRoom = false;
	bool onVehicle = false;
	VEHICLE_SPOT_TYPE spotType = SPOT_Invalid;	// Vehicle parking spot type for vehicle transport.
	AuId opId;	// Id of AI crewman manning the transporter console in the room, if any. Can be null. Ship id for vehicle trans.
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		onVehicle = person->IsAboardVehicle();
		const DaCapsule* ship = person->FindParentCapsuleInEnv();
		if(ship)
		{
			if(onVehicle)
			{
				// Check for vehicle that can be transported.
				if(objectName() == AuCommStdChannel::HailChannelName() ||
						objectName() == AuCommStdChannel::IntercomChannelName() ||
						objectName() == AuCommStdChannel::VoiceChannelName() ||
						(objectName() == AuCommStdChannel::CrewChannelName() &&
						 ship->BerthTypeOccupied(person->Id()) != AuRosterEntry::Invalid))
				{
					// Person is on the hail channel or they are on the crew channel and they hold a berth on the ship.
					spotType = person->InVehicleTransporter();
					if(spotType != SPOT_Invalid)
					{
						// Person is in a vehicle that can be transported from the ship.
						opId = ship->Id();					// Ship handles vehicle beam down messages.
					}
				}
			}
			else if(person->InTransporterRoom())
			{
				// Person is aboard a ship in a transporter room.
				if(objectName() == AuCommStdChannel::VoiceChannelName() ||
						objectName() == AuCommStdChannel::IntercomChannelName() ||
						(objectName() == AuCommStdChannel::CrewChannelName() &&
						 ship->BerthTypeOccupied(person->Id()) != AuRosterEntry::Invalid))
				{
					// Person is on the voice channel or they are on the crew channel and they hold a berth on the ship.
					inTransporterRoom = true;
					DaSentient* crew = ship->FindTransporterOperator(person->CapsuleRoomId());
					if(crew)
						opId = crew->Id();					// Need AI id to route message specifically to them, like an order.
				}
			}
		}
	}
	view->UnlockSceneAtom();

	if(onVehicle)
	{
		if(spotType != SPOT_Invalid)
		{
			// Offer vehicle transport options. It's ok if the opId is null; a human could process this command also.
			AuBeamVehicleOptions dlg(pFrame);
			int result = auApp->Frame()->ExecDialog(dlg);
			if(result == QDialog::Accepted)
			{
				switch(dlg.BeamDestination())
				{
				case 0:	// Atom
					{
						AuCommBeamVehicleAtom msg(opId, spotType, 0, dlg.CityId(), dlg.CityName(), dlg.Target(), objectName(),
								view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
								m_RangeSetting, m_Scope);
						SendChat(msg);
					}
					break;
				case 1:	// Coordinate
					{
						AuCommBeamVehicleCoordinate msg(opId, spotType, 0, dlg.CoordX(), dlg.CoordY(), dlg.CoordZ(), objectName(),
								view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
								m_RangeSetting, m_Scope);
						SendChat(msg);
					}
					break;
				case 2:	// Planet
					{
						AuCommBeamVehiclePlanet msg(opId, spotType, 0, dlg.Longitude(), dlg.Latitude(), objectName(),
								view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
								m_RangeSetting, m_Scope);
						SendChat(msg);
					}
					break;
				case 3:	// Ship
					{
						AuCommBeamVehicleShip msg(opId, spotType, 0, dlg.ShipId(), dlg.ShipName(), objectName(),
								view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
								m_RangeSetting, m_Scope);
						SendChat(msg);
					}
					break;
				case 4:	// Transporter
					{
						AuCommBeamVehicleTransporter msg(opId, spotType, 0, dlg.TransporterId(), dlg.TransporterName(),
								dlg.TransporterWhich(), objectName(), view->SceneAtomId(), view->SceneAtomName(),
								view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
						SendChat(msg);
					}
					break;
				default:
					break;
				}
			}
		}
		else
		{
			AuCommBeamOver msg(1, objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
					view->SceneGalaxy(), m_RangeSetting, m_Scope);
			SendChat(msg);
		}
	}
	else if(inTransporterRoom)
	{
		// Offer transport options. It's ok if the opId is null; a human could process this command also.
		AuBeamOptions dlg(pFrame);
		int result = auApp->Frame()->ExecDialog(dlg);
		if(result == QDialog::Accepted)
		{
			switch(dlg.BeamDestination())
			{
			case 0:	// Atom
				{
					AuCommBeamAtom msg(opId, dlg.BeamCount(), dlg.BeamDirection(), dlg.CityId(), dlg.CityName(), dlg.Target(),
							objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
							m_RangeSetting, m_Scope);
					SendChat(msg);
				}
				break;
			case 1:	// Coordinate
				{
					AuCommBeamCoordinate msg(opId, dlg.BeamCount(), dlg.BeamDirection(), dlg.CoordX(), dlg.CoordY(), dlg.CoordZ(),
							objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
							m_RangeSetting, m_Scope);
					SendChat(msg);
				}
				break;
			case 2:	// Planet
				{
					AuCommBeamPlanet msg(opId, dlg.BeamCount(), dlg.BeamDirection(), dlg.Longitude(), dlg.Latitude(),
							objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
							m_RangeSetting, m_Scope);
					SendChat(msg);
				}
				break;
			case 3:	// Ship
				{
					AuCommBeamShip msg(opId, dlg.BeamCount(), dlg.BeamDirection(), dlg.ShipId(), dlg.ShipName(), objectName(),
							view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), m_RangeSetting,
							m_Scope);
					SendChat(msg);
				}
				break;
			case 4:	// Transporter
				{
					AuCommBeamTransporter msg(opId, dlg.BeamCount(), dlg.BeamDirection(), dlg.TransporterId(),
							dlg.TransporterName(), dlg.TransporterWhich(), objectName(), view->SceneAtomId(),
							view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
					SendChat(msg);
				}
				break;
			default:
				break;
			}
		}
	}
	else
	{
		AuBeamUp dlg(pFrame);
		int result = auApp->Frame()->ExecDialog(dlg);
		if(result == QDialog::Accepted)
		{
			AuCommBeamOver msg(dlg.BeamCount(), objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
					view->SceneGalaxy(), m_RangeSetting, m_Scope);
			SendChat(msg);
		}
	}
}

void AuCommChannel::Bid()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	AuItem* item = 0;
	unsigned int price = 0;
	QString empireNameOfOrigin;
	AuCommPacket::TYPE type = SelectedMessageType();
	if(type == AuCommPacket::Auction)
	{
		item = m_Transcript->SelectedMessageItem();
		empireNameOfOrigin = m_Transcript->SelectedMessageEmpireOfOrigin();

		// Calculate the quality base price of the commodity.
		if(item)
			price = item->UnitCost();
	}
	else if(type == AuCommPacket::ForSale)
	{
		item = m_Transcript->SelectedMessageItem();
		price = m_Transcript->SelectedMessagePrice();
		empireNameOfOrigin = m_Transcript->SelectedMessageEmpireOfOrigin();
	}

	if(!item)
		return;

	AuCommBuy dlg(*item, empireNameOfOrigin, price, pFrame);
	int result = auApp->Frame()->ExecDialog(dlg);
	delete item;
	if(result == QDialog::Accepted && dlg.Item() && dlg.Item()->Count() && dlg.OfferItem())
	{
		// Bidding on a commodity.
		if(dlg.OfferItem()->CommodityId() == CID_Money)
		{
			// Offering money.
			AuCommBid msg(*dlg.Item(), empireNameOfOrigin, dlg.OfferItem()->Count(),
					objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
					m_RangeSetting, m_Scope);
			SendChat(msg);
		}
		else
		{
			// Offering barter.
			AuCommBidBarter msg(*dlg.Item(), empireNameOfOrigin, *dlg.OfferItem(), dlg.OfferEmpireOfOrigin(),
					objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
					m_RangeSetting, m_Scope);
			SendChat(msg);
		}
	}
}

void AuCommChannel::BidPickupShipment()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	AuShipment shipment;
	if(!m_Transcript->SelectedMessageShipment(shipment))
		return;

	AuCommBidPickupShipment msg(shipment, objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
			view->SceneGalaxy(), m_RangeSetting, m_Scope);
	SendChat(msg);
}

void AuCommChannel::SendFunds()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	QString empireNameOfOrigin;
	const DgData& gd = g_Au.LockDataForRead();
	AuId fromId = gd.ClientUnitId();	// Sender's Unit Id
	bool isProgrammer = gd.ClientPlayerIsProgrammer();	// Programmer can give themself free money.
	g_Au.UnlockData();
	AuId msgFromId = m_Transcript->SelectedMessageFromId();
	QString msgFromName = m_Transcript->SelectedMessageFromName();
	if(!msgFromId.IsNull() && (msgFromId != fromId || isProgrammer))
	{
		// The dialog sends the funds to msgFromId if it is accepted.
		AuSendFunds sendFunds(tr("Transfer Funds to Sender"), msgFromName, msgFromId, pFrame);
		int result = auApp->Frame()->ExecDialog(sendFunds);
		if(result == QDialog::Accepted)
		{
			AuCommChat msg(tr("Funds were sent to %1. We will each receive a receipt by mail.").arg(msgFromName),
					objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
					m_RangeSetting, m_Scope);
			SendChat(msg);
		}
	}
}

void AuCommChannel::ForSale()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	AuCommSellCargo dlg(pFrame);
	int result = auApp->Frame()->ExecDialog(dlg);
	if(result == QDialog::Accepted)
	{
		if(dlg.IsShipment())
		{
			AuCommOfferTEU msg(*dlg.Item(), objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
					view->SceneGalaxy(), m_RangeSetting, m_Scope);
			SendChat(msg);
		}
		else if(dlg.IsAuction())
		{
			AuCommAuction msg(*dlg.Item(), dlg.EmpireOfOrigin(), objectName(), view->SceneAtomId(), view->SceneAtomName(),
					view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
			SendChat(msg);
		}
		else
		{
			AuCommForSale msg(*dlg.Item(), dlg.EmpireOfOrigin(), dlg.Price(), objectName(), view->SceneAtomId(),
					view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
			SendChat(msg);
		}
	}
}

void AuCommChannel::RequestService()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	AuServiceDialog dlg(this);
	pFrame->ExecDialog(dlg);
}

void AuCommChannel::SolicitSales()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	COMMODITYID defCommodityId = m_Transcript->SelectedMessageCommodityId();

	AuCommSolicitItem dlg(defCommodityId, pFrame);
	int result = auApp->Frame()->ExecDialog(dlg);
	if(result == QDialog::Accepted)
	{
		// The text of the message is the commodity id.
		switch(dlg.Solicitation())
		{
		case AuCommSolicitItem::All:
			{
				AuCommSolicitAllSales msg(objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
						view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuCommSolicitItem::Ammo:
			{
				AuCommSolicitAmmoSales msg(objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
						view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuCommSolicitItem::Armor:
			{
				AuCommSolicitArmorSales msg(objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
						view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuCommSolicitItem::Commodity:
			{
				AuCommSolicitItemSales msg(dlg.CommodityId(), objectName(), view->SceneAtomId(), view->SceneAtomName(),
						view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuCommSolicitItem::ConstructionMaterial:
			{
				AuCommSolicitConstructionMaterialSales msg(objectName(), view->SceneAtomId(), view->SceneAtomName(),
						view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuCommSolicitItem::Food:
			{
				AuCommSolicitFoodSales msg(objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
						view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuCommSolicitItem::Fuel:
			{
				AuCommSolicitFuelSales msg(objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
						view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuCommSolicitItem::Medical:
			{
				AuCommSolicitMedSales msg(objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
						view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuCommSolicitItem::Misc:
			{
				AuCommSolicitMiscSales msg(objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
						view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuCommSolicitItem::NaturalResources:
			{
				AuCommSolicitNaturalResourceSales msg(objectName(), view->SceneAtomId(), view->SceneAtomName(),
						view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuCommSolicitItem::SpareParts:
			{
				AuCommSolicitPartsSales msg(objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
						view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuCommSolicitItem::Patents:
			{
				AuCommSolicitPatentSales msg(objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
						view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuCommSolicitItem::Tools:
			{
				AuCommSolicitToolSales msg(objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
						view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuCommSolicitItem::Weapons:
			{
				AuCommSolicitWeaponSales msg(objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
						view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuCommSolicitItem::Shipments:
			{
				AuCommSolicitShipments msg(objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
						view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		}
	}
}

// Caller must delete returned item.
AuItem* AuCommChannel::SelectedMessageItem() const
{
	return m_Transcript->SelectedMessageItem();
}

// Caller must delete returned item.
AuItem* AuCommChannel::SelectedMessageOfferItem() const
{
	return m_Transcript->SelectedMessageOfferItem();
}

QString AuCommChannel::SelectedMessageEmpireOfOrigin() const
{
	return m_Transcript->SelectedMessageEmpireOfOrigin();
}

AuId AuCommChannel::SelectedMessageSenderId() const
{
	return m_Transcript->SelectedMessageFromId();
}

QString AuCommChannel::SelectedMessageSenderName() const
{
	return m_Transcript->SelectedMessageFromName();
}

AuCommPacket::TYPE AuCommChannel::SelectedMessageType() const
{
	return m_Transcript->SelectedMessageType();
}

void AuCommChannel::RevokeOffer()
{
	const AuView* view = auApp->Frame()->View();
	AuCommRevokeBid msg(objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
			m_RangeSetting, m_Scope);
	SendChat(msg);
}

void AuCommChannel::FocusChatInput()
{
	if(m_Say)
	{
		m_Say->activateWindow();
		m_Say->raise();
		m_Say->setFocus();
	}
}

void AuCommChannel::NotifyServer()
{
	QString name = objectName();
	auApp->CommChannelOpened(name);
}

void AuCommChannel::MessageActivated(const QModelIndex& index)
{
	// Get information about the comm message at the row indexed.
	QModelIndex index0 = index.sibling(index.row(), 0);

	// Extract the packet from the byte array.
	QByteArray bamsg = index0.data(Qt::UserRole).toByteArray();	// This is a copy of the original comm message, in a byte array.
	QDataStream in(bamsg);
	in.setVersion(QDataStream::Qt_4_1);
	quint8 msgType;
	in >> msgType;
	AuCommPacket* msgPacket = AuCommPacket::CreateCommPacket((AuCommPacket::TYPE)msgType);
	in >> *msgPacket;

	AuId msgFromId;
	QString msgFromName;
	QString msgText;
	if(msgPacket)
	{
		msgFromId = msgPacket->FromId();
		msgFromName = msgPacket->FromName();
		msgText = msgPacket->Text();

		// Message packet is no longer needed.
		delete msgPacket;
		msgPacket = 0;
	}

	switch((AuCommPacket::TYPE)msgType)
	{
	case AuCommPacket::MissionNew:
		AcceptNewMission();
		break;
	case AuCommPacket::MissionOrderAdd:
		AcceptAddMissionOrder();
		break;
	case AuCommPacket::MissionOrderInsert:
		AcceptInsertMissionOrder();
		break;
	case AuCommPacket::MissionOrderNew:
		AcceptNewMissionOrder();
		break;
	case AuCommPacket::MissionProperty:
		AcceptMissionProperty();
		break;
	case AuCommPacket::Auction:
	case AuCommPacket::ForSale:
	case AuCommPacket::ShipmentItem:
		Bid();
		break;
	case AuCommPacket::ChatChoice:
		{
			quint32 storyInstance = index0.data(Qt::UserRole+1).toInt();
			if(!storyInstance)
				break;
			qint8 value = index0.data(Qt::UserRole+2).toInt();

			m_Transcript->clearSelection();

			// Echo the choice picked, as if the local dude said it.
			const AuView* view = auApp->Frame()->View();
			AuCommChoicePicked answer(msgText, objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
					view->SceneGalaxy(), m_RangeSetting, m_Scope);
			SendChat(answer);

			emit ChoicePicked(storyInstance, value, msgFromId);
		}
		break;
	case AuCommPacket::NarrateChoice:
		{
			quint32 storyInstance = index0.data(Qt::UserRole+1).toInt();
			if(!storyInstance)
				break;
			qint8 value = index0.data(Qt::UserRole+2).toInt();

			m_Transcript->clearSelection();

			// Echo the choice picked, as if the local dude said it.
			const AuView* view = auApp->Frame()->View();
			AuCommChoicePicked answer(msgText, objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
					view->SceneGalaxy(), m_RangeSetting, m_Scope);
			SendChat(answer);

			emit ChoicePicked(storyInstance, value, msgFromId);
		}
		break;
	case AuCommPacket::ChatImage:
	case AuCommPacket::NarrateImage:
		{
			AuImageDialog* idlg = new AuImageDialog(msgText, m_Transcript->SelectedMessageImage(), this);
			idlg->show();
		}
		break;

	case AuCommPacket::HailRequest:
		OpenChannelToSender();
		break;

	case AuCommPacket::HailReply:	// Only ships send HailReply message. Send the ship an order.
		MissionOrder(m_Transcript->SelectedMessageFromName(), m_Transcript->SelectedMessageFromId(),
				m_Transcript->SelectedMessageFromSystem());
		break;

	case AuCommPacket::Accept:
	case AuCommPacket::AcceptAvatar:
	case AuCommPacket::AcceptBarter:
	case AuCommPacket::AcceptBerthCaptain:
	case AuCommPacket::AcceptBerthCaptainSlot:
	case AuCommPacket::AcceptBerthCrew:
	case AuCommPacket::AcceptBerthOfficer:
	case AuCommPacket::AcceptBerthPassenger:
	case AuCommPacket::AcceptBerthTroop:
	case AuCommPacket::AcceptInviteToEmpire:
	case AuCommPacket::AcceptPickupShipment:
	case AuCommPacket::AcceptRefit:
	case AuCommPacket::AcceptRepairAll:
	case AuCommPacket::AcceptRepairSystem:
	case AuCommPacket::AcceptTeleport:
	case AuCommPacket::AcceptUpgradeInstall:
	case AuCommPacket::Announcement:
	case AuCommPacket::BeamAtom:
	case AuCommPacket::BeamCoordinate:
	case AuCommPacket::BeamOver:
	case AuCommPacket::BeamPlanet:
	case AuCommPacket::BeamShip:
	case AuCommPacket::BeamTransporter:
	case AuCommPacket::BeamVehicleAtom:
	case AuCommPacket::BeamVehicleCoordinate:
	case AuCommPacket::BeamVehicleOver:
	case AuCommPacket::BeamVehiclePlanet:
	case AuCommPacket::BeamVehicleShip:
	case AuCommPacket::BeamVehicleTransporter:
	case AuCommPacket::Bid:
	case AuCommPacket::BidBarter:
	case AuCommPacket::BidPickupShipment:
	case AuCommPacket::BoardCrew:
	case AuCommPacket::BoardCryo:
	case AuCommPacket::BoardOfficers:
	case AuCommPacket::BoardPassengers:
	case AuCommPacket::BoardTroops:
	case AuCommPacket::ChannelOpened:
	case AuCommPacket::Chat:
	case AuCommPacket::ChatAudio:
	case AuCommPacket::ChatContinued:
	case AuCommPacket::ChatStory:
	case AuCommPacket::ChoicePicked:
	case AuCommPacket::CitizenOrder:
	case AuCommPacket::CitizenPosture:
	case AuCommPacket::CityChat:
	case AuCommPacket::CityMessage:
	case AuCommPacket::CitySurrenderAccepted:
	case AuCommPacket::CitySurrenderOffered:
	case AuCommPacket::Combine:
	case AuCommPacket::ContactReply:
	case AuCommPacket::ContactShip:
	case AuCommPacket::CrewBoarded:
	case AuCommPacket::CrewBoardingComplete:
	case AuCommPacket::CryoBoarded:
	case AuCommPacket::CryoBoardingComplete:
	case AuCommPacket::CryoDebarked:
	case AuCommPacket::DamageInflicted:
	case AuCommPacket::DebarkCryo:
	case AuCommPacket::DebarkPassengers:
	case AuCommPacket::Detonate:
	case AuCommPacket::DiplomaticMessage:
	case AuCommPacket::FiringAtTarget:
	case AuCommPacket::Harvest:
	case AuCommPacket::InventoryItem:
	case AuCommPacket::InviteTeleport:
	case AuCommPacket::InviteToEmpire:
	case AuCommPacket::Jettison:
	case AuCommPacket::Message:
	case AuCommPacket::Narrate:
	case AuCommPacket::NarrateAudio:
	case AuCommPacket::NarrateTitle:
	case AuCommPacket::NarrateVideo:
	case AuCommPacket::OfferAvatar:
	case AuCommPacket::OfferRefit:
	case AuCommPacket::OfferRepairAll:
	case AuCommPacket::OfferRepairSystem:
	case AuCommPacket::OfferTEU:
	case AuCommPacket::OfferUpgradeInstall:
	case AuCommPacket::OfficerBoarded:
	case AuCommPacket::OpenTEU:
	case AuCommPacket::PassengerBoardingComplete:
	case AuCommPacket::PassengersBoarded:
	case AuCommPacket::PassengersDebarked:
	case AuCommPacket::PreBoard:
	case AuCommPacket::RepairPartsMissing:
	case AuCommPacket::RequestBerthAny:
	case AuCommPacket::RequestBerthCaptain:
	case AuCommPacket::RequestBerthCaptainSlot:
	case AuCommPacket::RequestBerthCrew:
	case AuCommPacket::RequestBerthOfficer:
	case AuCommPacket::RequestBerthPassenger:
	case AuCommPacket::RequestBerthTroop:
	case AuCommPacket::RequestExterminator:
	case AuCommPacket::RequestHelp:
	case AuCommPacket::RequestHouseKeeping:
	case AuCommPacket::RequestInventory:
	case AuCommPacket::RequestRefit:
	case AuCommPacket::RequestRepairAll:
	case AuCommPacket::RequestRepairSystem:
	case AuCommPacket::RequestUpgradeSystem:
	case AuCommPacket::RevokeBid:
	case AuCommPacket::SOSAttack:
	case AuCommPacket::SOSFuelMarooned:
	case AuCommPacket::SOSFuelShip:
	case AuCommPacket::SOSFuelStation:
	case AuCommPacket::SOSManeuverDriveAdrift:
	case AuCommPacket::SOSManeuverDriveMarooned:
	case AuCommPacket::SOSManeuverDriveStation:
	case AuCommPacket::ShipStasisWait:
	case AuCommPacket::ShipStasisDimension:
	case AuCommPacket::Silverwill:
	case AuCommPacket::SolicitAllSales:
	case AuCommPacket::SolicitAmmoSales:
	case AuCommPacket::SolicitArmorSales:
	case AuCommPacket::SolicitConstructionMaterialSales:
	case AuCommPacket::SolicitFoodSales:
	case AuCommPacket::SolicitFuelSales:
	case AuCommPacket::SolicitItemSales:
	case AuCommPacket::SolicitMedSales:
	case AuCommPacket::SolicitMiscSales:
	case AuCommPacket::SolicitNaturalResourceSales:
	case AuCommPacket::SolicitPartsSales:
	case AuCommPacket::SolicitPatentSales:
	case AuCommPacket::SolicitShipments:
	case AuCommPacket::SolicitToolSales:
	case AuCommPacket::SolicitWeaponSales:
	case AuCommPacket::SpotTargetOrTerrain:
	case AuCommPacket::Thought:
	case AuCommPacket::ThoughtMailAlert:
	case AuCommPacket::ThoughtMessage:
	case AuCommPacket::TroopBoardingComplete:
	case AuCommPacket::TroopsBoarded:
	case AuCommPacket::CommTypes:
		break;	// Message doesn't do anything when activated.
	}
}

void AuCommChannel::MessageClicked()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	quint32 personEmpireId = view->SceneAtomEmpire();
	AuId personId = view->SceneAtomId();
	QString personName = view->SceneAtomName();
	AuId personSectorId = view->SceneSector();
	GALAXY personGalaxy = view->SceneGalaxy();

	AuId contactId = m_Transcript->SelectedMessageFromId();
	QString contactName = m_Transcript->SelectedMessageFromName();
	bool contactIsMe = contactId == personId;

	QString msgText = m_Transcript->SelectedMessageText();
	QUrl url = UrlFromString(msgText);

	// Show a pop-up menu of options.
	QMenu menu(this);

	// First option for agent is teleport, for consistency.
	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	bool isProgrammer = gd.ClientPlayerIsProgrammer();
	g_Au.UnlockData();
	if(isAgent)
	{
		bool addedAny = false;
		if(!contactIsMe)
		{
				QAction* teleport = menu.addAction(QIcon(":data/MsgTransport.png"), tr("Teleport to %1").arg(contactName));
				connect(teleport, SIGNAL(triggered()), SLOT(TeleportToSender()));
				addedAny = true;
		}

		if(isProgrammer)
		{
#if !defined(QT_NO_DEBUG)
			QAction* hgtask = menu.addAction(QIcon(":data/Silverwill.png"), tr("Make Silverwill Task..."));
			connect(hgtask, SIGNAL(triggered()), SLOT(MakeSilverwillTaskFromMessage()));

			if(m_SilverwillTaskId)
			{
				QAction* hgnote = menu.addAction(QIcon(":data/Silverwill.png"),
						tr("Add Note to Silverwill Task %1...").arg(m_SilverwillTaskId));
				connect(hgnote, SIGNAL(triggered()), SLOT(MakeSilverwillNoteFromMessage()));
			}
			addedAny = true;
#endif
		}

		if(addedAny)
			menu.addSeparator();
	}

	// Links go at the top.
	QAction* openLink = 0;
	if(url != QUrl())
	{
		QIcon ic;
		if(msgText.toUpper().contains("HAZERON.COM") || msgText.toUpper().contains("DIGIPROSE.COM"))
			ic = QIcon(":data/OrderGame.png");
		else
			ic = QIcon(":data/OrderWeb.png");

		openLink = menu.addAction(ic, tr("Open Link in Browser"));
		menu.addSeparator();
	}

	// Add menu items to examine objects associated with the selected message.
	AuItem* item = SelectedMessageItem();
	if(item)
	{
		QAction* itemdesc = menu.addAction(QIcon(item->BitmapName()), tr("Inspect %1...").arg(item->SayName()));
		connect(itemdesc, SIGNAL(triggered()), SLOT(ItemDescription()));
		delete item;
	}

	AuItem* offerItem = SelectedMessageOfferItem();
	if(offerItem)
	{
		QAction* itemdesc = menu.addAction(QIcon(offerItem->BitmapName()), tr("Inspect %1...").arg(offerItem->SayName()));
		connect(itemdesc, SIGNAL(triggered()), SLOT(OfferItemDescription()));
		delete offerItem;
	}

	// Add menu items based on selected message type.
	switch(SelectedMessageType())
	{
	case AuCommPacket::Auction:
	case AuCommPacket::ForSale:
		{
			QAction* bid = menu.addAction(QIcon(AuCommBidBarter::CommPacketIcon()), tr("Offer a Bid..."));
			connect(bid, SIGNAL(triggered()), SLOT(Bid()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::ShipmentItem:
		{
			QAction* bid = menu.addAction(QIcon(AuCommBidPickupShipment::CommPacketIcon()), tr("Offer to Pickup Shipment"));
			connect(bid, SIGNAL(triggered()), SLOT(BidPickupShipment()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::ChatChoice:
	case AuCommPacket::NarrateChoice:
		{
			QAction* accept = menu.addAction(QIcon(AuCommChatChoice::CommPacketIcon()), tr("Accept Choice"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::MissionNew:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept New Mission"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::MissionOrderAdd:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Add Mission Order"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::MissionOrderInsert:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Insert Mission Order"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::MissionOrderNew:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept New Mission Order"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::MissionProperty:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Mission Change"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::Bid:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Bid"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::BidBarter:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Trade"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::OfferAvatar:
		if(!contactIsMe)
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Avatar"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::OfferRefit:
		if(!contactIsMe)
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Refit"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::CitySurrenderOffered:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept City Surrender"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

		/*
	case AuCommPacket::InviteTeleport:
		if(!contactIsMe)
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Invitation to Teleport"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;
		*/

	case AuCommPacket::InviteToEmpire:
		if(!contactIsMe)
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Invitation to Join Empire"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::RequestBerthAny:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Berth Request..."));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::RequestBerthOfficer:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Officer Berth Request"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::RequestBerthCrew:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Crew Berth Request"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::RequestBerthPassenger:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Passenger Berth Request"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

		break;

	case AuCommPacket::RequestBerthTroop:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Troop Berth Request"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::BoardPassengers:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Debark Passengers..."));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::DebarkPassengers:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Board Passengers..."));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::BoardCryo:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Debark Cryo Travellers..."));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::DebarkCryo:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Board Cryo Travellers..."));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::BoardTroops:
		/*
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Debark Troops..."));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		*/
		break;

	case AuCommPacket::OfferUpgradeInstall:
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Upgrade"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::OfferRepairAll:
	case AuCommPacket::OfferRepairSystem:
		if(m_Transcript->SelectedMessageRepairShipId() == view->SceneAtomInShip())
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Repairs"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::PreBoard:
		// A pre-boarding announcement is selected. These messages originate from spacecraft when loading passengers.
		{
			QAction* accept = menu.addAction(QIcon(AuCommAccept::CommPacketIcon()), tr("Accept Pre Boarding"));
			connect(accept, SIGNAL(triggered()), SLOT(Accept()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::HailReply:
	case AuCommPacket::HailRequest:
		{
			QAction* reply = menu.addAction(QIcon(AuCommHailReply::CommPacketIcon()), tr("Reply to Hail..."));
			connect(reply, SIGNAL(triggered()), SLOT(HailReply()));
		}
		menu.addSeparator();
		break;

	case AuCommPacket::Accept:
	case AuCommPacket::AcceptAvatar:
	case AuCommPacket::AcceptBarter:
	case AuCommPacket::AcceptBerthCaptain:
	case AuCommPacket::AcceptBerthCaptainSlot:
	case AuCommPacket::AcceptBerthCrew:
	case AuCommPacket::AcceptBerthOfficer:
	case AuCommPacket::AcceptBerthPassenger:
	case AuCommPacket::AcceptBerthTroop:
	case AuCommPacket::AcceptInviteToEmpire:
	case AuCommPacket::AcceptPickupShipment:
	case AuCommPacket::AcceptRefit:
	case AuCommPacket::AcceptRepairAll:
	case AuCommPacket::AcceptRepairSystem:
	case AuCommPacket::AcceptTeleport:
	case AuCommPacket::AcceptUpgradeInstall:
	case AuCommPacket::Announcement:
	case AuCommPacket::BeamAtom:
	case AuCommPacket::BeamCoordinate:
	case AuCommPacket::BeamOver:
	case AuCommPacket::BeamPlanet:
	case AuCommPacket::BeamShip:
	case AuCommPacket::BeamTransporter:
	case AuCommPacket::BeamVehicleAtom:
	case AuCommPacket::BeamVehicleCoordinate:
	case AuCommPacket::BeamVehicleOver:
	case AuCommPacket::BeamVehiclePlanet:
	case AuCommPacket::BeamVehicleShip:
	case AuCommPacket::BeamVehicleTransporter:
	case AuCommPacket::BidPickupShipment:
	case AuCommPacket::BoardCrew:
	case AuCommPacket::BoardOfficers:
	case AuCommPacket::ChannelOpened:
	case AuCommPacket::Chat:
	case AuCommPacket::ChatAudio:
	case AuCommPacket::ChatContinued:
	case AuCommPacket::ChatImage:
	case AuCommPacket::ChatStory:
	case AuCommPacket::ChoicePicked:
	case AuCommPacket::CitizenOrder:
	case AuCommPacket::CitizenPosture:
	case AuCommPacket::CityChat:
	case AuCommPacket::CityMessage:
	case AuCommPacket::CitySurrenderAccepted:
	case AuCommPacket::Combine:
	case AuCommPacket::ContactReply:
	case AuCommPacket::ContactShip:
	case AuCommPacket::CrewBoarded:
	case AuCommPacket::CrewBoardingComplete:
	case AuCommPacket::CryoBoarded:
	case AuCommPacket::CryoBoardingComplete:
	case AuCommPacket::CryoDebarked:
	case AuCommPacket::DamageInflicted:
	case AuCommPacket::Detonate:
	case AuCommPacket::DiplomaticMessage:
	case AuCommPacket::FiringAtTarget:
	case AuCommPacket::Harvest:
	case AuCommPacket::InventoryItem:
	case AuCommPacket::InviteTeleport:
	case AuCommPacket::Jettison:
	case AuCommPacket::Message:
	case AuCommPacket::Narrate:
	case AuCommPacket::NarrateAudio:
	case AuCommPacket::NarrateImage:
	case AuCommPacket::NarrateTitle:
	case AuCommPacket::NarrateVideo:
	case AuCommPacket::OfferTEU:
	case AuCommPacket::OfficerBoarded:
	case AuCommPacket::OpenTEU:
	case AuCommPacket::PassengerBoardingComplete:
	case AuCommPacket::PassengersBoarded:
	case AuCommPacket::PassengersDebarked:
	case AuCommPacket::RepairPartsMissing:
	case AuCommPacket::RequestBerthCaptain:
	case AuCommPacket::RequestBerthCaptainSlot:
	case AuCommPacket::RequestExterminator:
	case AuCommPacket::RequestHelp:
	case AuCommPacket::RequestHouseKeeping:
	case AuCommPacket::RequestInventory:
	case AuCommPacket::RequestRefit:
	case AuCommPacket::RequestRepairAll:
	case AuCommPacket::RequestRepairSystem:
	case AuCommPacket::RequestUpgradeSystem:
	case AuCommPacket::RevokeBid:
	case AuCommPacket::SOSAttack:
	case AuCommPacket::SOSFuelMarooned:
	case AuCommPacket::SOSFuelShip:
	case AuCommPacket::SOSFuelStation:
	case AuCommPacket::SOSManeuverDriveAdrift:
	case AuCommPacket::SOSManeuverDriveMarooned:
	case AuCommPacket::SOSManeuverDriveStation:
	case AuCommPacket::ShipStasisWait:
	case AuCommPacket::ShipStasisDimension:
	case AuCommPacket::Silverwill:
	case AuCommPacket::SolicitAllSales:
	case AuCommPacket::SolicitAmmoSales:
	case AuCommPacket::SolicitArmorSales:
	case AuCommPacket::SolicitConstructionMaterialSales:
	case AuCommPacket::SolicitFoodSales:
	case AuCommPacket::SolicitFuelSales:
	case AuCommPacket::SolicitItemSales:
	case AuCommPacket::SolicitMedSales:
	case AuCommPacket::SolicitMiscSales:
	case AuCommPacket::SolicitNaturalResourceSales:
	case AuCommPacket::SolicitPartsSales:
	case AuCommPacket::SolicitPatentSales:
	case AuCommPacket::SolicitShipments:
	case AuCommPacket::SolicitToolSales:
	case AuCommPacket::SolicitWeaponSales:
	case AuCommPacket::SpotTargetOrTerrain:
	case AuCommPacket::Thought:
	case AuCommPacket::ThoughtMailAlert:
	case AuCommPacket::ThoughtMessage:
	case AuCommPacket::TroopBoardingComplete:
	case AuCommPacket::TroopsBoarded:
	case AuCommPacket::CommTypes:
		break;	// Message selected is not valid for accept button
	}

	// Add menu items based on the sender of the selected message.
	QAction* addbuddy = 0;
	QAction* addsquelch = 0;
	QAction* invite = 0;
	QAction* sendmail = 0;
	if(!auApp->IsSingleUserMode())
	{
		QAction* openWebPage = menu.addAction(QIcon(":data/c_VacSuitHelmet.png"), tr("View Web Page for %1...")
				.arg(contactName));
		connect(openWebPage, SIGNAL(triggered()), SLOT(ViewWebPageOfSender()));

		if(!contactIsMe)
		{
			QAction *openchannel = menu.addAction(QIcon(":data/CommDirect.png"), tr("Open Channel to %1...")
					.arg(contactName));
			connect(openchannel, SIGNAL(triggered()), SLOT(OpenChannelToSender()));
		}
	}

	sendmail = menu.addAction(QIcon(":data/MailMessage.png"), tr("Send Mail to %1...")
			.arg(contactName));

	if(!contactIsMe)
	{
		QAction* sendFunds = menu.addAction(QIcon(":data/c_Money.png"), tr("Send Funds to %1...")
				.arg(contactName));
		connect(sendFunds, SIGNAL(triggered()), SLOT(SendFunds()));

		//inviteTeleport = menu.addAction(QIcon(":data/InviteTeleport.png"), tr("Invite %1 to Teleport Here").arg(contactName));

		const DgData& gd = g_Au.LockDataForRead();
		if(gd.EmpireCitizenCanInviteCitizens(personEmpireId, personId))
			invite = menu.addAction(QIcon(":data/CommGovernment.png"), tr("Invite %1 to Join My Empire").arg(contactName));
		g_Au.UnlockData();
	}

	QAction* postBounty = menu.addAction(QIcon(":data/JobBountyHunter.png"), tr("Post Bounty on %1...")
			.arg(contactName));
	connect(postBounty, SIGNAL(triggered()), SLOT(PostBounty()));

	if(!contactIsMe)
	{
		if(!auApp->IsSingleUserMode())
		{
			QAction* offerAvatar = menu.addAction(QIcon(":data/Passenger.png"), tr("Offer %1 to be %2's Avatar")
					.arg(personName)
					.arg(contactName));
			connect(offerAvatar, SIGNAL(triggered()), SLOT(OfferAvatarToSender()));

			menu.addSeparator();
		}
	}

	if(!auApp->IsSingleUserDesignerMode())
	{
		// Buddy list menu items.
		addbuddy = menu.addAction(QIcon(":data/ContactNew.png"), tr("Add %1 to Contact List")
				.arg(contactName));

		addsquelch = menu.addAction(QIcon(":data/ContactBlocked.png"), tr("Add %1 to Block List")
				.arg(contactName));
	}

	QAction* transEmo = menu.addAction(QIcon(":data/Emoji.png"), tr("Translate Emojis..."));

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;

	if(action == addbuddy)
		AddBuddy(personId, personName, contactId, contactName);
	else if(action == addsquelch)
		AddSquelch(personId, personName, contactId, contactName);
	else if(action == invite)
		InviteToEmpire(personEmpireId, personId, personName, personSectorId, personGalaxy, contactId, contactName);
	//else if(action == inviteTeleport)
		//InviteTeleport(personId, personName, personSectorId, personGalaxy, contactId, contactName);
	else if(action == sendmail)
		SendMailMessage(personId, contactId, contactName);
	else if(action == openLink)
		QDesktopServices::openUrl(url);
	else if(action == transEmo)
	{
		QString trans;
		QStringList words = g_Au.SplitWordsAndEmojis(msgText);
		for(QStringList::const_iterator wi = words.cbegin(); wi != words.cend(); wi++)
		{
			int ei = g_Au.Emojis().EmojiIndex(*wi);
			if(ei >= 0)
				trans += g_Au.Emojis()[ei].Tag();
			else
				trans += *wi;
			trans += " ";
		}
		QMessageBox mb(g_Au.ApplicationName(), trans.trimmed(),
			QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
		auApp->Frame()->ExecMessageBox(mb);
	}
}

// T/F the contact was added
//static
bool AuCommChannel::AddContact(const AuId& contactId, const QString& contactName, bool bIgnore)
{
	bool bRetVal = false;

	const DgData& gd = g_Au.LockDataForRead();
	QString sAcctName = gd.GetAcctName();
	QByteArray sAcctPassword = gd.GetAcctPassCrypt();
	bool bEmpty = gd.GetContacts().isEmpty();
	g_Au.UnlockData();

	if(bEmpty && !auApp->IsSingleUserMode())
	{
		// If it is empty, we may not have requested the contacts list yet.
		AuLoginComm login;
		login.RequestContacts(sAcctName,sAcctPassword);
	}

	// First check to see if this contact is already in the list, don't send the update if it exists
	const DgData& gd1 = g_Au.LockDataForRead();
	bool exists=false;
	for(QList<AuContact>::const_iterator it = gd1.GetContacts().begin(); it != gd1.GetContacts().end() && !exists; it++)
	{
		if((*it).m_id == contactId)
			exists = true;	// don't care whether it is a buddy or squelchy. if in the list, don't add again.
	}
	g_Au.UnlockData();

	if(!exists)	// not in the list, send it to the database
	{
		AuContact contact;
		contact.m_id = contactId;
		contact.m_name = contactName;
		contact.m_bIgnore = bIgnore;

		if(!auApp->IsSingleUserMode())
		{
			AuLoginComm login;
			login.UpdateContact(sAcctName, sAcctPassword, contact, false);
		}

		DgData& gdwrite = g_Au.LockDataForWrite();
		gdwrite.AddContact(contact);
		g_Au.UnlockData();

		bRetVal = true;
	}

	return bRetVal;
}

void AuCommChannel::AddBuddy(const AuId& playerId, const QString& playerName, const AuId& contactId, const QString& contactName)
{
	if(AddContact(contactId, contactName, false))
	{
		AuCommThought mymsg(tr("%1 is now on my contact list.").arg(contactName), playerId, playerName);
		SendChat(mymsg);
	}
	else
	{
		AuCommThought mymsg(tr("%1 is already in my list! They can only be listed once, as either a contact or a block.")
				.arg(contactName),
				playerId, playerName);
		SendChat(mymsg);
	}
}

void AuCommChannel::AddSquelch(const AuId& playerId, const QString& playerName, const AuId& contactId, const QString& contactName)
{
	if(AddContact(contactId, contactName, true))
	{
		AuCommThought mymsg(tr("Messages from %1 will now be ignored.").arg(contactName), playerId, playerName);
		SendChat(mymsg);
		auApp->SendAddSquelch(contactId);
	}
	else
	{
		AuCommThought mymsg(tr("%1 is already in my list! They can only be listed once, as either a contact or a block.")
				.arg(contactName),
				playerId, playerName);
		SendChat(mymsg);
	}
}

void AuCommChannel::SendMailMessage(const AuId& playerId, const AuId& contactId, const QString& contactName)
{
	QList<AuContact> qlContacts;
	AuContact contact(contactId, contactName);
	qlContacts.append(contact);
	AuMailNew* mail = new AuMailNew(this, playerId, &qlContacts, contactName);
	mail->show();
}

void AuCommChannel::InviteTeleport(AuId& personId, QString& personName, AuId& personSectorId, GALAXY personGalaxy, AuId& contactId,
		QString& contactName)
{
	// Send invitation message to contact
	AuCommInviteTeleport msg(contactId, contactName, objectName(), personId, personName, personSectorId, personGalaxy,
			m_RangeSetting, m_Scope);
	SendChat(msg);
}

void AuCommChannel::InviteToEmpire(quint32 personEmpireId, AuId& personId, QString& personName, AuId& personSectorId,
		GALAXY personGalaxy, AuId& contactId, QString& contactName)
{
	// Send invitation message to contact
	AuCommInviteToEmpire msg(contactId, contactName, personEmpireId, objectName(), personId, personName, personSectorId,
			personGalaxy, m_RangeSetting, m_Scope);
	SendChat(msg);
}

void AuCommChannel::SpotTargetOrTerrain(const QString& targetDesc, POLITICAL_STANCE stanceTowardTarget, bool targetIsCivilian)
{
	const AuView* view = auApp->Frame()->View();
	AuCommSpotTargetOrTerrain msg(targetDesc, stanceTowardTarget, targetIsCivilian, objectName(), view->SceneAtomId(),
			view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
	SendChat(msg);
}

void AuCommChannel::CityInventory()
{
	const AuView* view = auApp->Frame()->View();
	AuCommRequestInventory msg(objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
			m_RangeSetting, m_Scope);
	SendChat(msg);
}

void AuCommChannel::RequestBerth()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	// Offer a dialog to let the user choose the preferred type of berth.
	// 'Any Berth' and 'Captain's Cabin' options are available in this case.
	AuAssignBerth requestDlg(true, true, true, true, true, true, pFrame);
	int result = auApp->Frame()->ExecDialog(requestDlg);
	if(result == QDialog::Accepted)
	{
		// Get governing credentials of person.
		quint32 personEmpire = view->SceneAtomEmpire();

		switch(requestDlg.TypeAssigned())
		{
		case AuRosterEntry::CaptainSlot:
		case AuRosterEntry::OfficerAI:
		case AuRosterEntry::PassengerAI:
			// Not valid return value from AuAssignBerth dialog.
			Q_ASSERT(false);
			break;
		case AuRosterEntry::Invalid:
			{
				AuCommRequestBerthAny msg(personEmpire, objectName(), view->SceneAtomId(), view->SceneAtomName(),
						view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuRosterEntry::Captain:
			{
				AuCommRequestBerthCaptain msg(personEmpire, objectName(), view->SceneAtomId(), view->SceneAtomName(),
						view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuRosterEntry::Crew:
			{
				AuCommRequestBerthCrew msg(personEmpire, objectName(), view->SceneAtomId(), view->SceneAtomName(),
						view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuRosterEntry::OfficerHuman:
			{
				AuCommRequestBerthOfficer msg(personEmpire, objectName(), view->SceneAtomId(), view->SceneAtomName(),
						view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuRosterEntry::PassengerHuman:
			{
				AuCommRequestBerthPassenger msg(personEmpire, objectName(), view->SceneAtomId(), view->SceneAtomName(),
						view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		case AuRosterEntry::Troop:
			{
				AuCommRequestBerthTroop msg(personEmpire, objectName(), view->SceneAtomId(), view->SceneAtomName(),
						view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			break;
		}
	}
}

void AuCommChannel::LoadColonists()
{
	unsigned int openOfficerBerths = 0;
	unsigned int openCrewBerths = 0;
	unsigned int openCryoBerths = 0;
	unsigned int openStateroomBerths = 0;
	unsigned int openTroopBerths = 0;
	bool isAboardShip = false;
	bool isInVehicle = false;
	bool isOfficer = false;
	AuId loaderId;

	// Lock the root and find the scene atom.
	AuView* view = auApp->Frame()->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
		if(ship)
		{
			// Parent atom is a spaceship.
			isAboardShip = true;
			if(view->IsPersonInCommand())
			{
				// Scene atom is an officer of the ship they are aboard.
				// Only an officer of the ship can load colonists.
				openCrewBerths = ship->CrewBerthsAvailable();
				openCryoBerths = ship->CryoBerthsAvailable();
				openOfficerBerths = ship->OfficerBerthsAvailable();
				openStateroomBerths = ship->PassengerBerthsAvailable();
				openTroopBerths = ship->TroopBerthsAvailable();
				isOfficer = true;
				loaderId = person->Id();
			}
		}
		else
		{
			DaVehicle* vehicle = dynamic_cast<DaVehicle*>(person->Parent());
			if(vehicle)
			{
				// Parent atom is a spaceship.
				isInVehicle = true;
				if(vehicle->IsControlSeat(person->VehicleSeatIndex()))
				{
					// Scene atom is in a control seat of the vehicle.
					// NOTE: Officers and Crew cannot be loaded into a vehicle.
					openStateroomBerths = vehicle->EmptySeats();
					openTroopBerths = openStateroomBerths;
					isOfficer = true;
					loaderId = person->Id();
				}
			}
		}
	}
	view->UnlockSceneAtom();

	if(!isAboardShip && !isInVehicle)
	{
		QMessageBox mb(g_Au.ApplicationName(),
				tr("Only an officer aboard a spacecraft or the driver of a vehicle can\n"
					"load passengers or troops."),
			QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
		auApp->Frame()->ExecMessageBox(mb);
	}
	else if(openOfficerBerths || openCrewBerths || openCryoBerths || openStateroomBerths || openTroopBerths)
	{
		// NOTE: Officers and Crew cannot be loaded into a vehicle.
		AuFrame* pFrame = auApp->Frame();
		AuLoadColonists dlg(openOfficerBerths, openCrewBerths, openCryoBerths, openStateroomBerths, openTroopBerths, isInVehicle,
				pFrame);
		int result = auApp->Frame()->ExecDialog(dlg);
		if(result == QDialog::Accepted)
		{
			if(dlg.CrewLoaded())
			{
				Q_ASSERT(dlg.CrewLoaded() <= openCrewBerths);
				AuCommBoardCrew msg(loaderId, dlg.CrewLoaded(),
						objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
						m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			else if(dlg.CryoLoaded())
			{
				Q_ASSERT(dlg.CryoLoaded() <= openCryoBerths);
				AuCommBoardCryo msg(loaderId, dlg.CryoLoaded(),
						objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
						m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			else if(dlg.OfficersLoaded())
			{
				Q_ASSERT(dlg.OfficersLoaded() <= openOfficerBerths);
				AuCommBoardOfficers msg(loaderId, dlg.OfficersLoaded(),
						objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
						m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			else if(dlg.PassengersLoaded())
			{
				Q_ASSERT(dlg.PassengersLoaded() <= openStateroomBerths);
				// BoardColonists: count|loaderId
				AuCommBoardPassengers msg(loaderId, dlg.PassengersLoaded(),
						objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
						m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			else if(dlg.TroopsLoaded())
			{
				Q_ASSERT(dlg.TroopsLoaded() <= openTroopBerths);
				// BoardTroops: count|loaderId
				AuCommBoardTroops msg(loaderId, dlg.TroopsLoaded(),
						objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
						m_RangeSetting, m_Scope);
				SendChat(msg);
			}
		}
	}
	else if(isOfficer)
	{
		if(isAboardShip)
		{
			QMessageBox mb(g_Au.ApplicationName(), tr("There are no empty crew, stateroom or troop berths aboard the spacecraft."),
				QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
			auApp->Frame()->ExecMessageBox(mb);
		}
		else
		{
			QMessageBox mb(g_Au.ApplicationName(), tr("There are no empty seats in the vehicle."),
				QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
			auApp->Frame()->ExecMessageBox(mb);
		}
	}
	else
	{
		if(isAboardShip)
		{
			QMessageBox mb(g_Au.ApplicationName(), tr("Only an officer of the spacecraft can load crew, passengers or troops."),
				QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
			auApp->Frame()->ExecMessageBox(mb);
		}
		else
		{
			QMessageBox mb(g_Au.ApplicationName(), tr("Only the driver of the vehicle can load passengers."),
				QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
			auApp->Frame()->ExecMessageBox(mb);
		}
	}
}

void AuCommChannel::UnloadColonists()
{
	unsigned int passengersAboard = 0;
	unsigned int cryoAboard = 0;
	bool isAboardShip = false;
	bool isInVehicle = false;
	bool isOfficer = false;
	AuId unloaderId;

	// Lock the root and find the scene atom.
	AuView* view = auApp->Frame()->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
		if(ship)
		{
			// Parent atom is a spaceship.
			isAboardShip = true;
			if(view->IsPersonInCommand())
			{
				// Scene atom is an officer of the ship they are aboard.
				// Only an officer of the ship can unload passengers.
				passengersAboard = ship->ColonistsAboard();
				cryoAboard = ship->CryoBerthsOccupied();
				isOfficer = true;
				unloaderId = person->Id();
			}
		}
		else
		{
			DaVehicle* vehicle = dynamic_cast<DaVehicle*>(person->Parent());
			if(vehicle)
			{
				isInVehicle = true;
				if(vehicle->IsControlSeat(person->VehicleSeatIndex()))
				{
					// Scene atom is the driver of the vehicle they are aboard.
					// Only the driver of the vehicle can unload passengers.
					passengersAboard = vehicle->ColonistsAboard();
					isOfficer = true;
					unloaderId = person->Id();
				}
			}
		}
	}
	view->UnlockSceneAtom();

	if(!isAboardShip && !isInVehicle)
	{
		QMessageBox mb(g_Au.ApplicationName(), tr("Only an officer aboard a spacecraft or the driver of a vehicle can\n"
					"debark passengers."),
			QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
		auApp->Frame()->ExecMessageBox(mb);
	}
	else if(passengersAboard || cryoAboard)
	{
		AuFrame* pFrame = auApp->Frame();
		AuUnloadColonists dlg(cryoAboard, passengersAboard, 0, isInVehicle, pFrame);
		int result = auApp->Frame()->ExecDialog(dlg);
		if(result == QDialog::Accepted)
		{
			if(dlg.PassengersUnloaded())
			{
				AuCommDebarkPassengers msg(unloaderId, dlg.PassengersUnloaded(),
						objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
						m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			else if(dlg.CryoUnloaded())
			{
				AuCommDebarkCryo msg(unloaderId, dlg.CryoUnloaded(),
						objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
						m_RangeSetting, m_Scope);
				SendChat(msg);
			}
		}
	}
	else if(isOfficer)
	{
		if(isAboardShip)
		{
			QMessageBox mb(g_Au.ApplicationName(),
					tr("There are no passengers or cryo travellers aboard the spacecraft to debark."),
				QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
			auApp->Frame()->ExecMessageBox(mb);
		}
		else
		{
			QMessageBox mb(g_Au.ApplicationName(), tr("There are no passengers aboard the vehicle to debark."),
				QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
			auApp->Frame()->ExecMessageBox(mb);
		}
	}
	else
	{
		if(isAboardShip)
		{
			QMessageBox mb(g_Au.ApplicationName(), tr("Only an officer of the spacecraft can debark passengers."),
				QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
			auApp->Frame()->ExecMessageBox(mb);
		}
		else
		{
			QMessageBox mb(g_Au.ApplicationName(), tr("Only the driver of the vehicle can debark passengers."),
				QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
			auApp->Frame()->ExecMessageBox(mb);
		}
	}
}

void AuCommChannel::setVisible(bool visible)
{
	QWidget::setVisible(visible);
	if(visible)
		m_UnseenChat = false;

	// Update text highlighting on tabs.
	AuComm* comm = dynamic_cast<AuComm*>(parent());
	if(comm)
		comm->Animate();
}

void AuCommChannel::MissionOrder()
{
	// Call with no recipient id to go to all ships in my command on the channel.
	// No system id results in routing information relative to my ship, not the receiving ship.
	if(SelectedMessageType() == AuCommPacket::HailReply)
		MissionOrder(m_Transcript->SelectedMessageFromName(), m_Transcript->SelectedMessageFromId(),
				m_Transcript->SelectedMessageFromSystem());
	else
		MissionOrder("", AuId(0), AuId(0));
}

void AuCommChannel::MissionOrder(const QString& toName, const AuId& toId, const AuId& toSystemId)
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	AuCommMission modlg(view->SceneAtomId(), toName, toId, toSystemId, this);
	m_MissionDlg = &modlg;
	pFrame->ExecDialog(modlg);
	m_MissionDlg = 0;
}

void AuCommChannel::AcceptAddMissionOrder()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;
	if(!view->IsPersonInCommand())
		return;		// Person does not have the authority to accept the order.

	const AuMissionOrder* order = m_Transcript->SelectedMessageMissionOrder();
	if(!order->IsValid())
		return;

	auApp->MissionAppendOrder(*order);

	QString text = tr("Aye, add %1.").arg(order->ListName());
	AuCommChat msg(text, objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
			m_RangeSetting, m_Scope);
	SendChat(msg);
}

void AuCommChannel::AcceptInsertMissionOrder()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;
	if(!view->IsPersonInCommand())
		return;		// Person does not have the authority to accept the order.

	const AuMissionOrder* order = m_Transcript->SelectedMessageMissionOrder();
	if(!order->IsValid())
		return;

	auApp->MissionInsertOrder(*order);

	QString text = tr("Aye, %1 now current.").arg(order->ListName());
	AuCommChat msg(text, objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
			m_RangeSetting, m_Scope);
	SendChat(msg);
}

void AuCommChannel::AcceptNewMissionOrder()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;
	if(!view->IsPersonInCommand())
		return;		// Person does not have the authority to accept the order.

	const AuMissionOrder* order = m_Transcript->SelectedMessageMissionOrder();
	if(!order->IsValid())
		return;

	auApp->MissionNew(*order);

	QString text = tr("Aye, %1.").arg(order->ListName());
	AuCommChat msg(text, objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
			m_RangeSetting, m_Scope);
	SendChat(msg);
}

void AuCommChannel::AcceptMissionProperty()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;
	if(!view->IsPersonInCommand())
		return;		// Person does not have the authority to accept the order.

	quint8 property = m_Transcript->SelectedMessageMissionProperty();
	quint8 value = m_Transcript->SelectedMessageMissionValue();

	switch(property)
	{
	case 0:	// Request for mission.
		{
			AuMission mission;
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				DaVessel* ship = person->FindParentVesselInEnv();
				if(ship)
				{
					mission = ship->LockMission();
					ship->UnlockMission();
				}
			}
			view->UnlockSceneAtom();

			if(mission.IsEmpty())
			{
				QString text = tr("Mission is %1, %2. No orders.").arg(mission.IsActive()?tr("active"):tr("on hold"))
					.arg(mission.Type() == AuMission::Single?tr("do once"):tr("cyclic"));
				AuCommChat msg(text, objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
						view->SceneGalaxy(), m_RangeSetting,
						m_Scope);
				SendChat(msg);
			}
			else
			{
				QString text = tr("Mission is %1, %2.").arg(mission.IsActive()?tr("active"):tr("on hold"))
					.arg(mission.Type() == AuMission::Single?tr("do once"):tr("cyclic"));
				AuCommChat msg(text, objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
						view->SceneGalaxy(), m_RangeSetting,
						m_Scope);
				SendChat(msg);

				QStringList orderDesc;
				mission.ListDescriptionsWithDoOnce(orderDesc);
				for(QStringList::const_iterator mi = orderDesc.constBegin(); mi != orderDesc.constEnd(); mi++)
				{
					AuCommChatContinued msg(*mi, objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
							view->SceneGalaxy(),
							m_RangeSetting, m_Scope);
					SendChat(msg);
				}
			}
		}
		break;
	case 1:	// Active state
		{
			// Send the mission activate command request to the server.
			auApp->MissionSetActive(value);
			if(value)
			{
				AuCommChat msg(tr("Aye, activate mission."),
						objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
						m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			else
			{
				AuCommChat msg(tr("Aye, hold mission."),
						objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
						m_RangeSetting, m_Scope);
				SendChat(msg);
			}
		}
		break;
	case 2:	// Cycle state
		{
			// Send the mission cycle command request to the server.
			auApp->MissionSetCycle(value);
			if(value)
			{
				AuCommChat msg(tr("Aye, cycle mission."),
						objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
						m_RangeSetting, m_Scope);
				SendChat(msg);
			}
			else
			{
				AuCommChat msg(tr("Aye, do mission once."),
						objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
						m_RangeSetting, m_Scope);
				SendChat(msg);
			}
		}
		break;
	}
}

void AuCommChannel::AcceptNewMission()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;
	if(!view->IsPersonInCommand())
		return;		// Person does not have the authority to accept the order.

	AuMission mission;
	QString missionName = m_Transcript->SelectedMessageMission(mission);
	if(mission.IsEmpty())
		return;

	auApp->MissionNew(mission);

	QString text = tr("Aye, %1 mission received, %2 steps.").arg(missionName).arg(mission.Count());
	AuCommChat msg(text, objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
			m_RangeSetting, m_Scope);
	SendChat(msg);
}

void AuCommChannel::ClearChatEntry()
{
	if(m_Say)
		m_Say->clear();
}

void AuCommChannel::NameTarget()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;
	if(pView->SpottedAtom().IsNull())
		return;

	// Admin is allowed to name/rename anything, including sentient races.
	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	g_Au.UnlockData();

	// Get the DNA string from the last spotted creature.
	int errCode = 0;			// 1=target not found, 2=rename not allowed, 3=no scene atom, 4=not a creature
	AuSpecies species;
	bool isWildAnimal = false;
	AuId sceneAtomId;
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		sceneAtomId = person->Id();
		const DaAtom* target = person->Ether()->FindNormalAtom(pView->SpottedAtom());
		if(target)
		{
			const DaCreature* creature = dynamic_cast<const DaCreature*>(target);
			if(creature)
			{
				// Get the DNA from the creature.
				species = creature->Species();
			
				// Determine if the creature is a wild animal.
				const DaAnimal* animal = dynamic_cast<const DaAnimal*>(creature);
				if(animal && animal->IsWild())
					isWildAnimal = true;

				// Prevent naming of other sentient races (people and citizens) besides the scene atom's own race.
				if(!isAgent && dynamic_cast<const DaSentient*>(creature) && person->Species() != species)
					errCode = 2;	// Cannot name person or citizen specie that does not match the scene atom's specie.
			}
			else
				errCode = 4;
		}
		else
			errCode = 1;
	}
	else
		errCode = 3;
	pView->UnlockSceneAtom();

	switch(errCode)
	{
	case 0:
		NameSpecies(species, isWildAnimal, sceneAtomId, this);
		break;

	case 1:
		{
			QMessageBox mb(g_Au.ApplicationName(), tr("The target is no longer in the scene."),
					QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
			pFrame->ExecMessageBox(mb);
		}
		break;

	case 2:
		{
			QMessageBox mb(g_Au.ApplicationName(), tr("You are not permitted to name the target specie. "
						"Sentient creatures can only be named by their own species."),
					QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
			pFrame->ExecMessageBox(mb);
		}
		break;

	case 3:
		{
			QMessageBox mb(g_Au.ApplicationName(), tr("Something went wrong. Try again."),
				QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
			pFrame->ExecMessageBox(mb);
		}
		break;

	case 4:
		{
			QMessageBox mb(g_Au.ApplicationName(), tr("The target is not a creature. Only creature species can be named."),
				QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
			pFrame->ExecMessageBox(mb);
		}
		break;
	}
}

// static
void AuCommChannel::NameSpecies(const AuSpecies& species, bool isWildAnimal, const AuId& sceneAtomId, QWidget* inputBoxParent)
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	// Insure species is valid.
	QString specieDesc = species.Description();
	if(specieDesc.isEmpty())
		return;

	// We have to test for a blank name rather than existence of the specie because the client requests every specie in the
	// scene. When the specie has not been named, the server replies with an empty specie record so the client will stop
	// asking for it.
	// Admin is allowed to name/rename anything, including sentient races.
	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	QString specieName = gd.SpeciesName(species, true);
	AuId specieNamer = gd.SpeciesNamedBy(species);
	g_Au.UnlockData();

	// We got a DNA string. Check to see if it is already named.
	int errCode = 0;	// 1=specie already named, 2=name not valid
	if(isAgent || specieName.isEmpty() || specieNamer == sceneAtomId || specieNamer.IsNull())
	{
		// The specie has not been named yet.
		bool ok;
		if(specieName.isEmpty())
			specieName = QInputDialog::getText(inputBoxParent, g_Au.ApplicationName(),
					tr("Enter a name for the '%1' specie.").arg(specieDesc), QLineEdit::Normal, "", &ok);
		else
			specieName = QInputDialog::getText(inputBoxParent, g_Au.ApplicationName(),
					tr("Enter a name for the '%1' specie. The current name of the specie is '%2'.")
					.arg(specieDesc).arg(specieName), QLineEdit::Normal, specieName, &ok);
		if(ok)
		{
			specieName = specieName.trimmed();
			if(IsNameValid(specieName))
				auApp->NameSpecies(species, specieName, isWildAnimal);	// Tell the server to name the specie.
			else
				errCode = 2;	// The name is invalid.
		}
	}
	else
		errCode = 1;	// The specie has already been named.

	switch(errCode)
	{
	default:
		break;

	case 1:
		{
			QMessageBox mb(g_Au.ApplicationName(), tr("The target specie is already named '%1'. Only the person who originally "
						"named that specie is permitted to change its name.").arg(specieName),
				QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, pFrame);
			pFrame->ExecMessageBox(mb);
		}
		break;

	case 2:
		{
			QMessageBox mb(g_Au.ApplicationName(),
					tr("The name is invalid. A valid name must be less than %1 characters and "
						"it must not contain certain reserved characters such as | and newline.")
					.arg(MaxNameLength()+1),
					QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, pFrame);
			pFrame->ExecMessageBox(mb);
		}
		break;
	}
}

void AuCommChannel::Detonate()
{
	const AuView* view = auApp->Frame()->View();
	AuCommDetonate msg(objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(),
			m_RangeSetting, m_Scope);
	SendChat(msg);
}

void AuCommChannel::PostBounty()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	AuId victimId = m_Transcript->SelectedMessageFromId();
	QString victimName = m_Transcript->SelectedMessageFromName();
	if(victimId.IsNull() || victimId == AuId(1))
		return;													// No bounty on null id or anonymous message id.

	double account = 0;
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
		account = person->AccountBalance();
	view->UnlockSceneAtom();
	if(account <= 0)
		return;													// No money.

	AuPostBounty dlg(victimName, account, 0, pFrame);
	int result = auApp->Frame()->ExecDialog(dlg);
	if(result == QDialog::Accepted)
	{
		QString reason = dlg.Reason();
		double reward = dlg.Reward();
		auApp->PostBounty(victimId, victimName, reason, reward);
	}
}

void AuCommChannel::PostBountySpotted()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;
	AuId victimId = view->SpottedAtom();
	if(victimId.IsNull())
		return;													// No bounty on null id.

	// Get the name of the victim. It must be a DaUnit atom.
	QString victimName;
	double account = 0;
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		account = person->AccountBalance();
		if(account > 0)
		{
			// Person has money.
			const DaAtom* target = person->Ether()->FindNormalAtom(victimId);
			if(target)
			{
				const DaUnit* unit = dynamic_cast<const DaUnit*>(target);
				if(unit)
					victimName = unit->Name();
			}
		}
	}
	view->UnlockSceneAtom();
	if(victimName.isEmpty())
		return;													// Victim atom must not be one that can have a bounty.

	AuPostBounty dlg(victimName, account, 1, pFrame);
	int result = auApp->Frame()->ExecDialog(dlg);
	if(result == QDialog::Accepted)
	{
		QString reason = dlg.Reason();
		double reward = dlg.Reward();
		auApp->PostBounty(victimId, victimName, reason, reward);
	}
}

void AuCommChannel::DiplomaticMessage()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	const AuView* view = pFrame->View();
	if(!view)
		return;

	AuDiplomaticMessage dlg(pFrame);
	int result = auApp->Frame()->ExecDialog(dlg);
	if(result == QDialog::Accepted)
	{
		AuCommDiplomaticMessage msg(dlg.MessageText(), objectName(), view->SceneAtomId(), view->SceneAtomName(),
				view->SceneSector(), view->SceneGalaxy(), m_RangeSetting, m_Scope);
		SendChat(msg);
	}
}

void AuCommChannel::SensorContactsReceived(QByteArray bablips)
{
	if(m_MissionDlg)
		m_MissionDlg->SensorContactsReceived(bablips);
}

void AuCommChannel::OfferAvatarToSender()
{
	AuView* view = auApp->Frame()->View();
	if(!view)
		return;

	AuId fromId = m_Transcript->SelectedMessageFromId();
	if(fromId.IsNull())
		return;
	QString fromName = m_Transcript->SelectedMessageFromName();

	AuCommOfferAvatar msg(fromId, fromName, objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
			view->SceneGalaxy(), m_RangeSetting, m_Scope);
	SendChat(msg);
}

void AuCommChannel::Announcement()
{
	bool ok;
	QString announcement = QInputDialog::getText(this, g_Au.ApplicationName(),
			tr("Enter an announcement that will appear in the Thoughts channel of every avatar."), QLineEdit::Normal, "", &ok);
	if(!ok)
		return;

	AuCommAnnouncement msg(announcement);
	SendChat(msg);
}

void AuCommChannel::ItemDescription()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;
	AuItem* item = SelectedMessageItem();
	if(item)
	{
		AuItemDesc* dlg = new AuItemDesc(*item, this);
		connect(dlg, SIGNAL(finished(int)), dlg, SLOT(deleteLater()));
		dlg->show();
		delete item;
	}
}

void AuCommChannel::OfferItemDescription()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;
	AuItem* item = SelectedMessageOfferItem();
	if(item)
	{
		AuItemDesc* dlg = new AuItemDesc(*item, this);
		connect(dlg, SIGNAL(finished(int)), dlg, SLOT(deleteLater()));
		dlg->show();
		delete item;
	}
}

void AuCommChannel::TeleportToSender()
{
	AuId contactId = m_Transcript->SelectedMessageFromId();
	if(!contactId.IsNull())
		AuAgentTeleport::GoToDestination(contactId, false, false);
}

QString AuCommChannel::LogFileName() const
{
	QDir logDir(g_Au.CacheGame());
	QString logFileName = logDir.absoluteFilePath(QString("Transcript.%1.txt").arg(objectName()));
	return logFileName;
}

void AuCommChannel::InitLog()
{
	// Do nothing if the file already exists.
	QString logFileName = LogFileName();
	if(QFileInfo::exists(logFileName))
		return;

	QFile logFile(logFileName);
	if(logFile.open(QIODevice::WriteOnly))
	{
		QTextStream ts(&logFile);

		ts << objectName() << " Channel Transcript";
		//ts << AuTime::Now().UTC().toString();

#if defined(Q_OS_WIN)
		ts << "\r\n";
#else
		ts << "\n";
#endif

		logFile.close();
	}
}

void AuCommChannel::ShowLog()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	InitLog();

	QString logFileName = LogFileName();
	if(!QFileInfo::exists(logFileName))
	{
		QMessageBox mb(g_Au.ApplicationName(),
				tr("The transcript file for this channel could not be created. "
					"The file name is <b>%1</b>.").arg(logFileName),
				QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, pFrame);
		pFrame->ExecMessageBox(mb);
		return;
	}

	QFile logFile(logFileName);
	if(!logFile.open(QIODevice::ReadOnly))
	{
		QMessageBox mb(g_Au.ApplicationName(),
				tr("The transcript file for this channel could not be opened. "
					"The file name is <b>%1</b>")
				.arg(logFileName),
				QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, pFrame);
		pFrame->ExecMessageBox(mb);
		return;
	}

	// Display the contents of the file.
	QString content = logFile.readAll();
	AuCommLog* commLog = new AuCommLog(objectName(), AuCommStdChannel::ChannelIconName(objectName(), m_Scope), content, pFrame);
	pFrame->ExecDialog(*commLog);
}

// Make a Silverwill task from the current message.
void AuCommChannel::MakeSilverwillTaskFromMessage()
{
	AuId fromId = m_Transcript->SelectedMessageFromId();
	QString fromName = m_Transcript->SelectedMessageFromName();
	QString text = m_Transcript->SelectedMessageText();
	if(text.left(11).toUpper() == ("SILVERWILL,"))
		text = text.mid(11).trimmed();
	if(text.isEmpty())
		return;
	text = text[0].toUpper() + text.mid(1);

	AuSilverwillTaskDlg dlg(fromId, fromName, text, this);
	if(auApp->Frame()->ExecDialog(dlg) != QDialog::Accepted)
		return;
	m_SilverwillTaskId = dlg.TaskId();
	m_SilverwillTaskText = dlg.TaskTitle();
}

void AuCommChannel::MakeSilverwillNoteFromMessage()
{
	if(!m_SilverwillTaskId)
		return;
	QString fromName = m_Transcript->SelectedMessageFromName();
	QString text = m_Transcript->SelectedMessageText();
	if(text.left(11).toUpper() == ("SILVERWILL,"))
		text = text.mid(11).trimmed();
	if(text.isEmpty())
		return;
	text = text[0].toUpper() + text.mid(1);

	AuSilverwillNoteDlg dlg(fromName, m_SilverwillTaskId, m_SilverwillTaskText, text, this);
	auApp->Frame()->ExecDialog(dlg);
}

void AuCommChannel::NewSilverwillTask()
{
	const DgData& gd = g_Au.LockDataForRead();
	AuId fromId = gd.ClientUnitId();
	QString fromName = gd.ClientUnitName(fromId);
	g_Au.UnlockData();

	AuSilverwillTaskDlg dlg(fromId, fromName, "", this);
	if(auApp->Frame()->ExecDialog(dlg) != QDialog::Accepted)
		return;
	m_SilverwillTaskId = dlg.TaskId();
	m_SilverwillTaskText = dlg.TaskTitle();
}

bool AuCommChannel::HasMessageAboutAvatar() const
{
	const DgData& gd = g_Au.LockDataForRead();
	QString avatarName(gd.ClientUnitName(gd.ClientUnitId()));
	g_Au.UnlockData();
	return m_Transcript->HasMessageAbout(avatarName);
}

void AuCommChannel::OpenChannelToSender()
{
	if(IsNarrator())
		return;
	AuId fromId = m_Transcript->SelectedMessageFromId();
	emit OpenChannelToSender(fromId);
}

void AuCommChannel::ViewWebPageOfSender()
{
	if(IsNarrator())
		return;
	AuId fromId = m_Transcript->SelectedMessageFromId();
	QUrl url(QString("https://www.hazeron.com/EmpireStandings/p%1.php").arg(fromId.StringId()));
	QDesktopServices::openUrl(url);
}

void AuCommChannel::ChannelSpotTargetOrTerrain(QString targetDesc, int stanceTowardTarget, bool targetIsCivilian)
{
	if(!isVisible())
		return;
	SpotTargetOrTerrain(targetDesc, (POLITICAL_STANCE)stanceTowardTarget, targetIsCivilian);
}

void AuCommChannel::EmojiPressed()
{
	AuEmojiDlg dlg(this);
	connect(&dlg, SIGNAL(Emoji(QString)), SLOT(Emoji(QString)));
	dlg.exec();
}

void AuCommChannel::Emoji(QString emoji)
{
	QString text = m_Say->text();
	m_Say->blockSignals(true);
	m_Say->setText(text + emoji);
	m_Say->blockSignals(false);
}

void AuCommChannel::RangeMenu()
{
	// Show popup of range options.
	QMenu menu(this);

	AuCommStdChannel::RANGE range;

	// Hail and Trade channels only go up to solar system range.
	if(!AuCommStdChannel::IsHailChannel(objectName()) && !AuCommStdChannel::IsTradeChannel(objectName()))
	{
		// Universe range
		range = AuCommStdChannel::Universe;
		menu.addAction(QIcon(AuCommStdChannel::RangeIcon(range)), AuCommStdChannel::RangeName(range),
				this, SLOT(RangeUniverse()));

		// Galaxy range
		range = AuCommStdChannel::Galaxy;
		menu.addAction(QIcon(AuCommStdChannel::RangeIcon(range)), AuCommStdChannel::RangeName(range),
				this, SLOT(RangeGalaxy()));

		// Sector range
		range = AuCommStdChannel::Sector;
		menu.addAction(QIcon(AuCommStdChannel::RangeIcon(range)), AuCommStdChannel::RangeName(range),
				this, SLOT(RangeSector()));
	}

	// Solar system range
	range = AuCommStdChannel::SolarSystem;
	menu.addAction(QIcon(AuCommStdChannel::RangeIcon(range)), AuCommStdChannel::RangeName(range),
			this, SLOT(RangeSystem()));

	// Hail range
	range = AuCommStdChannel::Hail;
	menu.addAction(QIcon(AuCommStdChannel::RangeIcon(range)), AuCommStdChannel::RangeName(range),
			this, SLOT(RangeHail()));

	// Close range
	range = AuCommStdChannel::Close;
	menu.addAction(QIcon(AuCommStdChannel::RangeIcon(range)), AuCommStdChannel::RangeName(range),
			this, SLOT(RangeClose()));

	menu.exec(QCursor::pos());
}

void AuCommChannel::StoryChoicePickedCleanup(quint32 storyInstance, qint8 choiceId, AuId fromId)
{
	Q_UNUSED(choiceId);
	Q_UNUSED(fromId);

	// Remove all the choices for the story line now that one has been picked.
	for(int idx=0; idx<m_Transcript->rowCount();)
	{
		QTableWidgetItem* it = m_Transcript->item(idx, 0);

		// Extract the packet from the byte array.
		QByteArray itmsg = it->data(Qt::UserRole).toByteArray();	// Copy of original message in a byte array.
		QDataStream in(itmsg);
		in.setVersion(QDataStream::Qt_4_1);
		quint8 itMsgType;
		in >> itMsgType;

		if((itMsgType == AuCommPacket::ChatChoice || itMsgType == AuCommPacket::NarrateChoice) &&
				it->data(Qt::UserRole+1) == storyInstance)
		{
			// Found a choice message associated with the story instance.
			m_Transcript->removeRow(idx);
		}
		else
			idx++;
	}
}

