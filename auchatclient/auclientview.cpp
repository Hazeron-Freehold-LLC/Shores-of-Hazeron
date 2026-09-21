//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientview.cpp,v 1.99 2022/05/04 21:36:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auclientview.h"
//
#include "aubox.h"
#include "auchooseindustry.h"
#include "auclientapp.h"
#include "auclientframe.h"
#include "auclientglobal.h"
#include "aucommstdchannels.h"
#include "audefusebomb.h"
#include "aueffect.h"
#include "aufilever.h"
#include "augame.h"
#include "auimagedialog.h"
#include "aujoystick.h"
#include "aunetworkactivity.h"
#include "ausound.h"
#include "austylesheet.h"
#include "autoolbutton.h"
#include "daanimal.h"
#include "daarena.h"
#include "daatom.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dadesigner.h"
#include "dadesignplane.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "danpc.h"
#include "daperson.h"
#include "dasc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "daworldringsection.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include <QAction>
#include <QCursor>
#include <QDir>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTextBrowser>
#include <QTimer>
#include <QToolBar>
#include <QToolTip>
#include <QVBoxLayout>
#include <QWheelEvent>

#define d_ShowFrameAnimTime				0
#define d_ShowFrameRenderTime			0
#if(d_ShowFrameRenderTime || d_ShowFrameAnimTime)
static quint32 s_FrameCounter = 0;
#endif

// Tuning parameters.
#define d_AtomControlTime				10						// Msec between atom control updates.
#define d_FrameRate						60						// Target frame rate, in frames per second.
#define d_SubtitleWordWrap				128						// Line length of narrative text.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// static
QString AuViewButton::Image(VIEWBUTTON vb)
{
	QString icons[VB_COUNT+1] =
	{
		"",								// VB_Invalid,
		":data/Agent.png",				// VB_Agent,
		":data/Architect.png",			// VB_Architect,
		":data/c_VacSuitHelmet.png",	// VB_Bio,
		":data/c_Crate.png",			// VB_Cargo,
		"",			// VB_UNUSED2,
		":data/Comm.png",				// VB_Comm,
		":data/ExitGame.png",			// VB_Exit,
		":data/ToggleFull.png",			// VB_FullScreen,
		":data/CommGovernment.png",		// VB_Government,
		":data/CommHelp.png",			// VB_Help,
		":data/ToggleAll.png",			// VB_HideWindows,
		":data/Inventory.png",			// VB_Gear,
		":data/Locator.png",			// VB_Locator,
		":data/ToggleMail.png",			// VB_Mail,
		"",			// VB_UNUSED4,
		":data/Programmer.png",			// VB_Programmer,
		":data/SpawnRecall.png",		// VB_Recall,
		":data/Settings.png",			// VB_Settings,
		":data/GovSpacecraft.png",		// VB_Spacecraft,
		":data/CommMilitary.png",		// VB_UnitOrders,
		":data/SoHLogo32.png",			// VB_About,
		":data/c_VacSuitHelmet.png",	// VB_Avatar1,
		":data/c_VacSuitHelmet.png",	// VB_Avatar2,
		":data/c_VacSuitHelmet.png",	// VB_Avatar3,
		":data/c_VacSuitHelmet.png",	// VB_Avatar4,
		":data/c_VacSuitHelmet.png",	// VB_Avatar5,
		":data/c_VacSuitHelmet.png",	// VB_Avatar6,
		":data/c_Money.png",			// VB_SpacecraftAccount,
		":data/JobBountyHunter.png",	// VB_SpacecraftBounties,
		":data/MsgDamageReport.png",	// VB_SpacecraftDamage,
		":data/OrderTakeCommand.png",	// VB_SpacecraftMission,
		":data/Captain.png",			// VB_SpacecraftPolicy,
		":data/CommCrew.png",			// VB_SpacecraftRoster,
		":data/Shelf.png",				// VB_CargoHold,
		":data/c_HeavyWeapon.png",		// VB_CargoOrdnance,
		":data/c_Spacecraft.png",		// VB_CargoTransfer,
		":data/Locator.png",			// VB_LocatorCompass,
		":data/SensorScan.png",			// VB_LocatorGPS,
		":data/RangeGalaxy.png",		// VB_LocatorStarMap,
		":data/Map.png",				// VB_LocatorWorldMap,
		":data/Speaker.png",			// VB_GovernAnnouncements,
		":data/City.png",				// VB_GovernCities,
		":data/CommCrew.png",			// VB_GovernCitizens,
		":data/GovCommerce.png",		// VB_GovernCommerce,
		":data/CommDiplomacy.png",		// VB_GovernDiplomacy,
		":data/CommGovernment.png",		// VB_GovernPolicy,
		"",		// VB_UNUSED3,
		":data/GovTaxes.png",			// VB_GovernTaxes,
		":data/CommShip.png",			// VB_Forum,
		":data/OrderRead.png",			// VB_Wiki,
		":data/GovCommerce.png",		// VB_WarDimensionStandings,
		"",		// VB_UNUSED6,
		":data/CommCrew.png",			// VB_Avatars,
		":data/c_Crate.png",			// VB_CargoManifest,
		":data/Exchange.png",			// VB_SpacecraftDesignExchange,
		":data/SoHLogo32.png",			// VB_HazeronHome,
		":data/Minion.png",				// VB_Minion,
		":data/ContactBuddies.png",		// VB_PlayersOnline,
		":data/FilmReel64.png",			// VB_Videos,
		":data/Exchange.png",			// VB_BdDesignExchange,
		":data/City.png",				// VB_Building,
		":data/c_Patent.png",			// VB_GovernPatents,
		"",			// VB_UNUSED1,
		":data/c_Container.png",		// VB_SpacecraftShipments,
		":data/it_HandCraft.png",		// VB_GearHandCraft,
		":data/RangeSystem.png",		// VB_LocatorSystemScan,
		"",	// VB_UNUSED5,
		"",								// VB_COUNT,
	};
	if(vb < 0 || vb >= sizeof(icons)/sizeof(QString))
		return "";
	return icons[vb];
}

//static
QString AuViewButton::Name(VIEWBUTTON vb)
{
	QString names[VB_COUNT+1] =
	{
		"",								// VB_Invalid,
		"Agent",						// VB_Agent,
		"Architect",					// VB_Architect,
		"Bio",							// VB_Bio,
		"Cargo Transfer",				// VB_Cargo,
		"",				// VB_UNUSED2,
		"Comm",							// VB_Comm,
		"Exit",							// VB_Exit,
		"Full Screen",					// VB_FullScreen,
		"Governance",					// VB_Government,
		"Call for Help",				// VB_Help,
		"Show or Hide All Windows",		// VB_HideWindows,
		"Gear",							// VB_Gear,
		"Locator",						// VB_Locator,
		"Mail",							// VB_Mail,
		"",	// VB_UNUSED4,
		"Programmer",					// VB_Programmer,
		"Recall Home",					// VB_Recall,
		"Settings",						// VB_Settings,
		"Spacecraft",					// VB_Spacecraft,
		"Unit Orders",					// VB_UnitOrders,
		"About...",						// VB_About,
		"Avatar Name",					// VB_Avatar1,
		"Avatar Name",					// VB_Avatar2,
		"Avatar Name",					// VB_Avatar3,
		"Avatar Name",					// VB_Avatar4,
		"Avatar Name",					// VB_Avatar5,
		"Avatar Name",					// VB_Avatar6,
		"Account",						// VB_SpacecraftAccount,
		"Bounties",						// VB_SpacecraftBounties,
		"Damage",						// VB_SpacecraftDamage,
		"Mission",						// VB_SpacecraftMission,
		"Policy",						// VB_SpacecraftPolicy,
		"Roster",						// VB_SpacecraftRoster,
		"Hold",							// VB_CargoHold,
		"Ordnance",						// VB_CargoOrdnance,
		"Transfer",						// VB_CargoTransfer,
		"Compass",						// VB_LocatorCompass,
		"GPS",							// VB_LocatorGPS,
		"Star Map",						// VB_LocatorStarMap,
		"World Map",					// VB_LocatorWorldMap,
		"Announcements",				// VB_GovernAnnouncements,
		"Cities and Bases",				// VB_GovernCities,
		"Citizens",						// VB_GovernCitizens,
		"Commerce",						// VB_GovernCommerce,
		"Diplomacy",					// VB_GovernDiplomacy,
		"Policy",						// VB_GovernPolicy,
		"",						// VB_UNUSED3,
		"Taxes",						// VB_GovernTaxes,
		"Forum",						// VB_Forum,
		"Wiki",							// VB_Wiki,
		"Empire Standings",				// VB_WarDimensionStandings,
		"",				// VB_UNUSED6,
		"Avatars",						// VB_Avatars,
		"Manifest",						// VB_CargoManifest,
		"Blueprint Exchange",			// VB_SpacecraftDesignExchange,
		"Hazeron Home",					// VB_HazeronHome,
		"Minion",						// VB_Minion,
		"Players Online",				// VB_PlayersOnline,
		"Videos",						// VB_Videos,
		"Blueprint Exchange",			// VB_BdDesignExchange,
		"Building",						// VB_Building,
		"Patents",						// VB_GovernPatents,
		"",						// VB_UNUSED1,
		"Shipments",					// VB_SpacecraftShipments,
		"Hand Craft",					// VB_GearHandCraft,
		"System Map",					// VB_LocatorSystemScan,
		"",						// VB_UNUSED5,
		"",								// VB_COUNT,
	};
	if(vb < 0 || vb >= sizeof(names)/sizeof(QString))
		return "";
	return names[vb];
}

// Name in settings is longer to group sub items.
//static
QString AuViewButton::SettingsName(VIEWBUTTON vb)
{
	QString names[VB_COUNT+1] =
	{
		"",								// VB_Invalid,
		"Agent",						// VB_Agent,
		"Architect",					// VB_Architect,
		"Bio",							// VB_Bio,
		"Cargo Transfer",				// VB_Cargo,
		"",				// VB_UNUSED2,
		"Comm",							// VB_Comm,
		"Exit",							// VB_Exit,
		"Full Screen",					// VB_FullScreen,
		"Governance",					// VB_Government,
		"Call for Help",				// VB_Help,
		"Show or Hide All Windows",		// VB_HideWindows,
		"Gear",							// VB_Gear,
		"Locator",						// VB_Locator,
		"Mail",							// VB_Mail,
		"",	// VB_UNUSED4,
		"Programmer",					// VB_Programmer,
		"Recall Home",					// VB_Recall,
		"Settings",						// VB_Settings,
		"Spacecraft",					// VB_Spacecraft,
		"Unit Orders",					// VB_UnitOrders,
		"About",						// VB_About,
		"Switch to Avatar 1",			// VB_Avatar1,
		"Switch to Avatar 2",			// VB_Avatar2,
		"Switch to Avatar 3",			// VB_Avatar3,
		"Switch to Avatar 4",			// VB_Avatar4,
		"Switch to Avatar 5",			// VB_Avatar5,
		"Switch to Avatar 6",			// VB_Avatar6,
		"Spacecraft Account",			// VB_SpacecraftAccount,
		"Spacecraft Bounties",			// VB_SpacecraftBounties,
		"Spacecraft Damage",			// VB_SpacecraftDamage,
		"Spacecraft Mission",			// VB_SpacecraftMission,
		"Spacecraft Policy",			// VB_SpacecraftPolicy,
		"Spacecraft Roster",			// VB_SpacecraftRoster,
		"Cargo Hold",					// VB_CargoHold,
		"Cargo Ordnance",				// VB_CargoOrdnance,
		"Cargo Transfer",				// VB_CargoTransfer,
		"Locator Compass",				// VB_LocatorCompass,
		"Locator GPS",					// VB_LocatorGPS,
		"Locator Star Map",				// VB_LocatorStarMap,
		"Locator World Map",			// VB_LocatorWorldMap,
		"Govern Announcements",			// VB_GovernAnnouncements,
		"Govern Cities and Bases",		// VB_GovernCities,
		"Govern Citizens",				// VB_GovernCitizens,
		"Govern Commerce",				// VB_GovernCommerce,
		"Govern Diplomacy",				// VB_GovernDiplomacy,
		"Govern Policy",				// VB_GovernPolicy,
		"",				// VB_UNUSED3,
		"Govern Taxes",					// VB_GovernTaxes,
		"Web Site Forum",				// VB_Forum,
		"Web Site Wiki",				// VB_Wiki,
		"Web Site Empire Standings",		// VB_WarDimensionStandings,
		"",		// VB_UNUSED6,
		"Web Site Avatars",				// VB_Avatars,
		"Cargo Manifest",				// VB_CargoManifest,
		"Spacecraft Blueprint Exchange",	// VB_SpacecraftDesignExchange,
		"Web Site Home Page",			// VB_HazeronHome,
		"Minion",						// VB_Minion,
		"Players Online",				// VB_PlayersOnline,
		"Web Site Videos",				// VB_Videos,
		"Building Blueprint Exchange",	// VB_BdDesignExchange,
		"Building",						// VB_Building,
		"Govern Patents",				// VB_GovernPatents,
		"",						// VB_UNUSED1,
		"Spacecraft Shipments",			// VB_SpacecraftShipments,
		"Hand Craft",					// VB_GearHandCraft,
		"Locator System Map",			// VB_LocatorSystemScan,
		"",	// VB_UNUSED5,
		"",								// VB_COUNT,
	};
	if(vb < 0 || vb >= sizeof(names)/sizeof(QString))
		return "";
	return names[vb];
}

//static
QString AuViewButton::TipBase(VIEWBUTTON vb)
{
	QString tips[VB_COUNT+1] =
	{
		"",								// VB_Invalid,
		QObject::tr("<b>Agent</b> %2<br>"
			"Show the <i>Agent</i> window for the <i>%1</i> program.").arg(g_Au.ApplicationName()),
		QObject::tr("<b>Architect</b> %2<br>"
			"Show the <i>Architect</i> window for the <i>%1</i> program.").arg(g_Au.ApplicationName()),
		QObject::tr("<b>Bio</b> %2<br>"										// VB_Bio
			"Show the <i>Bio</i> window to view information about your avatar."),
		QObject::tr("<b>Cargo</b> %2<br>"
			"Show the <i>Cargo</i> window to:<br>"
			"%1 Transfer items between your gear and the spacecraft you are aboard.<br>"
			"%1 Transfer money, citizens and cargo between two different spacecraft.<br>"
			"%1 Load ordnance into your vehicle from a spacecraft, station, or city.").arg(g_Au.MiddleDotSymbol()),
		QObject::tr("<html><b>UNUSED2</b> %2</html>"),					// VB_UNUSED2
		QObject::tr("<b>Comm</b> %2<br>"
			"Show the <i>Comm</i> window to communicate with other players, spacecraft and cities."),
		QObject::tr("<b>Exit Game</b> %2<br>"
			"Log out from <i>%1</i> and exit the game.").arg(g_Au.ApplicationName()),
		QObject::tr("<html><b>Toggle Full Screen Mode</b> %2<br>"
			"Switches between <i>full screen</i> and <i>window</i> modes."
#if defined(Q_OS_WIN)
			"<br><b>On Microsoft Windows the full screen main window has a one-pixel border.</b> "
			"If we find a way to overcome this, we will remove it."
#endif
			"</html>"),
		QObject::tr("<b>Governance</b> %2<br>"
			"Show the <i>Governance</i> window to view and change policies of your spacecraft and your empire."),
		QObject::tr("<b>Call For Help</b> %2<br>"
			"Request assistance from game staff. An agent will respond if one is available."),
		QObject::tr("<html><b>Show or Hide All Windows</b> %2<br>"
			"Shows or hides all windows, depending on whether any windows are visible when pressed.<br>"
			"%1 Hides all windows, if any are visible. Remembers which ones were visible.<br>"
			"%1 Shows all windows that were previously visible, if none are visible."
			"</html>").arg(g_Au.MiddleDotSymbol()),
		QObject::tr("<b>Gear</b> %2<br>"
			"Show the <i>Gear</i> window to interact with your personal items."),
		QObject::tr("<b>Locator</b> %2<br>"
			"Show the <i>Locator</i> window to view information about your current location, heading and environmental "
			"conditions."),
		QObject::tr("<b>Mail</b> %2<br>"
			"Show the <i>Mail</i> window to send mail to players and to receive mail from players, cities, and the "
			"design library. "
			"Cities in your chain of command send status reports to you by mail. The design library sends mail to "
			"notify you of architect commissions paid when your spacecraft designs are manufactured or building "
			"designs are built."),
		QObject::tr("<html><b>Make Item or Vehicle</b> %2<br>"
			"Enables you to create items and vehicles instantly."
			"</html>"),
		QObject::tr("<b>Programmer</b> %2<br>"
			"Show the <i>Programmer</i> window for the <i>%1</i> program.").arg(g_Au.ApplicationName()),
		QObject::tr("<html><b>Recall Home</b> %2<br>"
			"Teleports your avatar to your home location, in this order of precedence.<br>"
			"  1. <b>Spacecraft</b> where you hold a berth. "
			"You recall to your berth or standing at the helm if the ship has no berths. "
			"<b>E to exit a berth</b>. "
			"A berth provides a breathable environment. "
			"Ship life support must be functioning to recall aboard.<br>"
			"  2. <b>Home city</b>, if that city is friendly to your empire. "
			"You appear near where you stood when you declared the city your home.<br>"
			"  3. <b>Capital city</b> of your empire, if that city is controlled by your empire.<br>"
			"  4. <b>Randomly selected city</b> of your empire.<br>"
			"  5. <b>Empty world</b> that is habitable.<br>"
			"A <i>period of vulnerability</i> ensues while waiting to recall, during which you can be seen and "
			"injured.<br>"
			"<b>Cancel recall</b> by pressing the Recall Home button while waiting to recall.<br>"
			"<b><u>Equipment and money are lost if you recall to a different dimension. "
			"</b></u>"
			"Sea chest contents are not lost; your sea chest is different in each zone: conflict or neutral."
			"</html>"),
		QObject::tr("<b>Settings</b> %2<br>"
			"Show the <i>Settings</i> window to change settings that affect the <i>%1</i> program.").arg(g_Au.ApplicationName()),
		QObject::tr("<b>Spacecraft</b> %2<br>"
			"Show the <i>Spacecraft</i> window to manage the spacecraft you are aboard."),
		QObject::tr("<b>Unit Orders</b> %2<br>"
			"Show the <i>Unit Orders</i> window to communicate with your crew and troops."),
		QObject::tr("<b>About <i>%1</i>...</b> %2<br>"									// VB_About
			"Shows information about the <i>%1</i> software.").arg(g_Au.ApplicationName()),
		QObject::tr("<b>Switch Avatar</b> %2<br>"									// VB_Avatar1
			"Switches to the selected avatar."),
		QObject::tr("<b>Switch Avatar</b> %2<br>"									// VB_Avatar2
			"Switches to the selected avatar."),
		QObject::tr("<b>Switch Avatar</b> %2<br>"									// VB_Avatar3
			"Switches to the selected avatar."),
		QObject::tr("<b>Switch Avatar</b> %2<br>"									// VB_Avatar4
			"Switches to the selected avatar."),
		QObject::tr("<b>Switch Avatar</b> %2<br>"									// VB_Avatar5
			"Switches to the selected avatar."),
		QObject::tr("<b>Switch Avatar</b> %2<br>"									// VB_Avatar6
			"Switches to the selected avatar."),
		QObject::tr("<b>Spacecraft Account</b> %2<br>"
			"Show the <i>Spacecraft Account</i> window to access the money account of the spacecraft you are aboard."),
		QObject::tr("<b>Spacecraft Bounties</b> %2<br>"
			"Show the <i>Spacecraft Bounties</i> window to view bounties on the spacecraft you are aboard."),
		QObject::tr("<b>Spacecraft Damage</b> %2<br>"
			"Show the <i>Spacecraft Damage</i> window to view damage to the spacecraft you are aboard."),
		QObject::tr("<b>Spacecraft Mission</b> %2<br>"
			"Show the <i>Spacecraft Mission</i> window to access the mission of the spacecraft you are aboard."),
		QObject::tr("<b>Spacecraft Policy</b> %2<br>"
			"Show the <i>Spacecraft Policy</i> window to access policies of the spacecraft you are aboard."),
		QObject::tr("<b>Spacecraft Roster</b> %2<br>"
			"Show the <i>Spacecraft Roster</i> window to access the roster of the spacecraft you are aboard."),
		QObject::tr("<html><b>Hold</b> %2<br>"										// VB_CargoHold
			"Transfers things between your gear and the hold of the spacecraft you are aboard or to your sea chest. "
			"The room you occupy determines the kind of things that can be transferred. "
			"Enter your berth to transfer items between your gear and your sea chest.<br>"
			"%1 <b>Barracks</b> accesses the <i>Armory</i> for weapons and ammunition.<br>"
			"%1 <b>Berth</b> accesses your <i>Sea Chest</i>. You must be in your berth. "
			"Ship policy may prohibit access to sea chests by passengers.<br>"
			"%1 <b>Brig</b> accesses the <i>Feeder</i> for food and water.<br>"
			"%1 <b>Engine Room</b> accesses the <i>Fuel Coupling</i> for hydrogen fuel.<br>"
			"%1 <b>Galley</b> accesses the <i>Pantry</i> for food and drinks.<br>"
			"%1 <b>Hold</b> accesses the <i>Cargo Hold</i> for all things in the hold.<br>"
			"%1 <b>Lounge</b> accesses the <i>Bar</i> for food and drinks.<br>"
			"%1 <b>Sick Bay</b> accesses the <i>Pharmacy</i> for medical supplies.<br>"
			"%1 <b>Stateroom</b>, <b>Crew Cabin</b> and <b>Officer Cabin</b> accesses the <i>Wardrobe</i> for "
			"clothing and environment suits.<br>"
			"%1 <b>Vehicle Bay</b> accesses the <i>Garage</i> for tools, fuel and ammo."
			"</html>").arg(g_Au.MiddleDotSymbol()),
		QObject::tr("<html><b>Ordnance</b> %2<br>"									// VB_CargoOrdnance
			"Loads ammunition into the vehicle you occupy.<br>"
			"%1 Ammunition may be loaded into a turret of a spacecraft.<br>"
			"%1 Turrets load ammunition from the hold of the spacecraft.<br>"
			"%1 Ammunition may be loaded into a vehicle that has weapons.<br>"
			"%1 Vehicles must be in the flight deck of a spacecraft, on a slab in a city, or in the trade zone of a station.<br>"
			"%1 Export policies affect the ordnance choices available."
			"</html>").arg(g_Au.MiddleDotSymbol()),
		QObject::tr("<html><b>Transfer</b> %2<br>"									// VB_CargoTransfer
			"Transfers money, citizens and cargo between the spacecraft you are aboard and another "
			"spacecraft.<br>"
			"%1 You must be in the chain-of-command of the spacecraft you are aboard.<br>"
			"%1 The other spacecraft must be within trading range of your spacecraft.<br>"
			"%1 You must have authority to transfer cargo from the other spacecraft."
			"</html>").arg(g_Au.MiddleDotSymbol()),
		QObject::tr("<html><b>Compass</b> %2<br>"									// VB_LocatorCompass
				"Heading and location information."
				"</html>"),
		QObject::tr("<html><b>GPS</b> %2<br>"										// VB_LocatorGPS
				"Galactic positioning system coordinates."
				"</html>"),
		QObject::tr("<html><b>Star Map</b> %2<br>"									// VB_LocatorStarMap
				"Map of the stars known to your empire.<br>"
				"%1 Click a place to select or deselect.<br>"
				"%1 Double-click a place to center on its location.<br>"
				"%1 Mouse wheel or drag the magnifier to change distance from eye to look point. "
				"Shift key reduces wheel zoom increment.<br>"
				"%1 Click/drag to look around.<br>"
				"%1 W moves the look point forward.<br>"
				"%1 S moves the look point backward.<br>"
				"%1 A moves the look point left.<br>"
				"%1 D moves the look point right.<br>"
				"%1 C and Z rotate around the look point."
				"%1 Right-click for options."
				"</html>").arg(g_Au.MiddleDotSymbol()),
		QObject::tr("<html><b>World Map</b> %2<br>"								// VB_LocatorWorldMap
				"Map of the world, when you are at a world.<br>"
				"%1 Double-click the map to center on a location.<br>"
				"%1 Double-click your flashing icon or the map margin to center on your location.<br>"
				"%1 Mouse wheel or drag the magnifier to change magnification.<br>"
				"%1 Click/drag to move the map.<br>"
				"%1 Right-click for options."
				"</html>").arg(g_Au.MiddleDotSymbol()),
		QObject::tr("<b>Announcements</b> %2<br>"								// VB_GovernAnnouncements,
			"Show the <i>Announcements</i> window to view and change announcements of your empire."),
		QObject::tr("<b>Cities and Bases</b> %2<br>"							// VB_GovernCities,
			"Show the <i>Cities and Bases</i> window to view and change cities and military bases of your empire."),
		QObject::tr("<b>Citizens</b> %2<br>"									// VB_GovernCitizens,
			"Show the <i>Citizens</i> window to view and change citizens of your empire."),
		QObject::tr("<b>Commerce</b> %2<br>"									// VB_GovernCommerce,
			"Show the <i>Commerce</i> window to view and change commerce of your empire."),
		QObject::tr("<b>Diplomacy</b> %2<br>"									// VB_GovernDiplomacy,
			"Show the <i>Diplomacy</i> window to view and change diplomacy of your empire."),
		QObject::tr("<b>Policy</b> %2<br>"										// VB_GovernPolicy,
			"Show the <i>Policy</i> window to view and change policies of your empire."),
		QObject::tr("<b>UNUSED3</b> %2<br>"										// VB_UNUSED3,
				""),
		QObject::tr("<b>Taxes</b> %2<br>"										// VB_GovernTaxes,
			"Show the <i>Taxes</i> window to view and change taxes of your empire."),
		QObject::tr("<b>Forum</b> %2<br>"										// VB_Forum
			"Opens your web browser at the <i>%1</i> Forum. "
			"The <i>forum</i> provides a place for discussion with other players and to report problems.").arg(g_Au.ApplicationName()),
		QObject::tr("<b>Wiki</b> %2<br>"										// VB_Wiki
			"Opens your web browser at the <i>%1</i> Wiki. "
			"The <i>wiki</i> is a valuable resource to find answers.").arg(g_Au.ApplicationName()),
		QObject::tr("<b>Empire Standings</b> %2<br>"						// VB_WarDimensionStandings
			"Opens your web browser at the <i>%1</i> empire standings page. ").arg(g_Au.ApplicationName()),
		QObject::tr("<b>UNUSED6</b> %2<br>"						// VB_UNUSED6
			""),
		QObject::tr("<b>Avatars</b> %2<br>"										// VB_Avatars
			"Opens your web browser at the <i>%1</i> avatars page. "
			"The avatars page shows information about individual avatars in the game.").arg(g_Au.ApplicationName()),
		QObject::tr("<b>Manifest</b> %2<br>"								// VB_CargoManifest
			"Shows a manifest of cargo in the hold of the ship."),
		QObject::tr("<b>Spacecraft Blueprint Exchange</b> %2<br>"
			"Show the <i>Spacecraft Blueprint Exchange</i> window to view published blueprints and to publish your own "
			"blueprints."),	// VB_SpacecraftDesignExchange
		QObject::tr("<b>Hazeron Home</b> %2<br>"								// VB_HazeronHome
			"Opens your web browser at the <i>Shores of Hazeron</i> home page."),
		QObject::tr("<b>Minion</b> %2<br>"
			"Show the <i>Minion</i> window for the <i>%1</i> program.").arg(g_Au.ApplicationName()),
		QObject::tr("<b>Players Online</b> %2<br>"								// VB_PlayersOnline
			"Show the <i>Players Online</i> window to view a list of all players online."),
		QObject::tr("<b>Videos</b> %2<br>"										// VB_Videos
			"Opens your web browser at the <i>%1</i> videos page. "
			"There are videos to show how to use the designer and how to play the game.").arg(g_Au.ApplicationName()),
		QObject::tr("<b>Building Blueprint Exchange</b> %2<br>"
			"Show the <i>Building Blueprint Exchange</i> window to view published blueprints and to publish your own "
			"blueprints."),	// VB_SpacecraftDesignExchange
		QObject::tr("<html><b>Building</b> %2<br>"								// VB_Building
			"Show the <i>Building</i> window to use the building or road at your location. "
			"Window controls change based on the building or road site you occupy; "
			"you do not have to be inside a building."
			"</html>"),
		QObject::tr("<b>Patents</b> %2<br>"										// VB_GovernPatents,
			"Show the <i>Patents</i> window to view patents recorded in your empire."),
		QObject::tr("<b>UNUSED1</b> %2<br>"									// VB_UNUSED1
				""),
		QObject::tr("<b>Spacecraft Shipments</b> %2<br>"						// VB_SpacecraftShipments
			"Show the <i>Spacecraft Shipments</i> window to access the scheduled shipments of the spacecraft you are aboard."),
		QObject::tr("<b>Hand Craft</b> %2<br>"									// VB_GearHandCraft
			"Show the <i>Hand Craft</i> window to make things by hand."),
		QObject::tr("<html><p><b>System Map</b> %2<br>"							// VB_LocatorSystemScan
				"Shows a map of your current solar system. "
				"Data is limited if the system has not been surveyed by your empire. "
				"</p>"
				"<p>"
				"To survey a system, do one of these things.<br>"
				"%1 View detailed information about a sun or world on a sensor console.<br>"
				"%1 Order crew to conduct a <i>Solar System Survey Scan</i>.<br>"
				"%1 Build a city that has an observatory and a radar.<br>"
				"</p>"
				"</html>").arg(g_Au.MiddleDotSymbol()),
		QObject::tr("<b>UNUSED5</b> %2<br>"),								// VB_UNUSED5
		"",																		// VB_COUNT,
	};
	if(vb < 0 || vb >= sizeof(tips)/sizeof(QString) || tips[vb].isEmpty())
	{
#if !defined(QT_NO_DEBUG)
		qWarning() << "error: AuViewButton::TipBase vb" << vb << "Expect an 'Argument missing:' error next.";
#endif
		return "";
	}
	return tips[vb];
}

QString AuViewButton::Tip(const QString& tipBase) const
{
	AuFrame* frame = auApp->Frame();
	QString keycommand;
	switch(m_Id)
	{
	case VB_Invalid:
	case VB_Avatar1:
	case VB_Avatar2:
	case VB_Avatar3:
	case VB_Avatar4:
	case VB_Avatar5:
	case VB_Avatar6:
	case VB_Avatars:
	case VB_Forum:
	case VB_HazeronHome:
	case VB_Help:
	case VB_UNUSED1:
	case VB_UNUSED2:
	case VB_UNUSED3:
	case VB_UNUSED4:
	case VB_UNUSED5:
	case VB_UNUSED6:
	case VB_Videos:
	case VB_WarDimensionStandings:
	case VB_Wiki:
	case VB_COUNT:
		break;
	case VB_Recall:
		{
			// Locate the current person's spawn recall binding.
			DiControlInputArray controls;
			{
				DaPerson person;
				person.EnumControlInputs(controls);
			}

			for(DiControlInputArray::const_iterator ci = controls.cbegin(); ci != controls.cend(); ci++)
				if((*ci).GetId() == DaPerson::CI_SpawnRecall)
				{
					Qt::Key key = (*ci).Key();
					if(key == Qt::Key_unknown)
						break;

					Qt::KeyboardModifiers keymod = (*ci).KeyModifier();
					keycommand = AuKeyFilter::ModifierName(keymod) + AuKeyFilter::KeyName(key);
					break;
				}
		}
		break;
	case VB_Agent:
	case VB_Architect:
	case VB_Programmer:
	case VB_Minion:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleAgentUI);
		break;
	case VB_Bio:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleCharacterUI);
		break;
	case VB_PlayersOnline:
		keycommand = frame->ControlBinding(AuFrame::CI_TogglePlayersOnlineUI);
		break;
	case VB_Building:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleBuildingUI);
		break;
	case VB_Cargo:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleCargoUI);
		break;
	case VB_Comm:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleCommUI);
		break;
	case VB_Exit:
		keycommand = frame->ControlBinding(AuFrame::CI_ExitGame);
		break;
	case VB_FullScreen:
		keycommand = frame->ControlBinding(AuFrame::CI_FullScreenToggle);
		break;
	case VB_Government:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleGovernUI);
		break;
	case VB_HideWindows:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleAllUIs);
		break;
	case VB_Gear:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleGearUI);
		break;
	case VB_GearHandCraft:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleHandCraftUI);
		break;
	case VB_Locator:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleLocatorUI);
		break;
	case VB_Mail:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleMailUI);
		break;
	case VB_Settings:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleSettingsUI);
		break;
	case VB_Spacecraft:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftUI);
		break;
	case VB_UnitOrders:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleOrdersUI);
		break;
	case VB_About:
		keycommand = frame->ControlBinding(AuFrame::CI_About);
		break;
	case VB_SpacecraftAccount:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftAccountUI);
		break;
	case VB_SpacecraftBounties:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftBountiesUI);
		break;
	case VB_SpacecraftDamage:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftDamageUI);
		break;
	case VB_SpacecraftDesignExchange:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleScExchangeUI);
		break;
	case VB_BdDesignExchange:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleBdExchangeUI);
		break;
	case VB_SpacecraftMission:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftMissionUI);
		break;
	case VB_SpacecraftPolicy:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftPolicyUI);
		break;
	case VB_SpacecraftRoster:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftRosterUI);
		break;
	case VB_SpacecraftShipments:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftShipmentsUI);
		break;
	case VB_CargoHold:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleCargoHoldUI);
		break;
	case VB_CargoManifest:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleCargoManifestUI);
		break;
	case VB_CargoOrdnance:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleCargoOrdnanceUI);
		break;
	case VB_CargoTransfer:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleCargoTransferUI);
		break;
	case VB_LocatorCompass:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleLocatorCompassUI);
		break;
	case VB_LocatorGPS:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleLocatorGPSUI);
		break;
	case VB_LocatorStarMap:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleLocatorStarMapUI);
		break;
	case VB_LocatorSystemScan:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleLocatorSystemScanUI);
		break;
	case VB_LocatorWorldMap:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleLocatorWorldMapUI);
		break;
	case VB_GovernAnnouncements:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleGovernAnnouncementsUI);
		break;
	case VB_GovernCities:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleGovernCitiesUI);
		break;
	case VB_GovernCitizens:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleGovernCitizensUI);
		break;
	case VB_GovernCommerce:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleGovernCommerceUI);
		break;
	case VB_GovernDiplomacy:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleGovernDiplomacyUI);
		break;
	case VB_GovernPatents:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleGovernPatentsUI);
		break;
	case VB_GovernPolicy:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleGovernPolicyUI);
		break;
	case VB_GovernTaxes:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleGovernTaxesUI);
		break;
	}

	if(keycommand.isEmpty())
		return tipBase.arg("");

	return tipBase.arg(QString("(%1)").arg(keycommand));
}

bool AuViewButton::IsHidden(SCENE_CONTEXT sceneContext, bool hasEmpire) const
{
	if(g_AuClient.ViewButtonHidden(m_Id))
		return true;

	switch(m_Id)
	{
	case VB_Invalid:
	case VB_UNUSED1:
	case VB_UNUSED2:
	case VB_UNUSED3:
	case VB_UNUSED4:
	case VB_UNUSED5:
	case VB_UNUSED6:
	case VB_COUNT:
		// These buttons are never visible.
		return true;

	case VB_FullScreen:
	case VB_Exit:
	case VB_Settings:
	case VB_About:
	case VB_Forum:
	case VB_Wiki:
	case VB_Videos:
	case VB_WarDimensionStandings:
	case VB_Avatars:
	case VB_HazeronHome:
		// These buttons are always visible.
		break;

	case VB_Gear:
	case VB_GearHandCraft:
		// These buttons are not visible when the scene context is unknown.
		if(sceneContext == SCENE_Unknown)
			return true;
		break;

	case VB_Avatar1:
	case VB_Avatar2:
	case VB_Avatar3:
	case VB_Avatar4:
	case VB_Avatar5:
	case VB_Avatar6:
	case VB_Bio:
	case VB_PlayersOnline:
		// These buttons are not visible in single user modes.
		if(auApp->IsSingleUserDesignerMode())
			return true;
		break;

	case VB_Government:
	case VB_GovernAnnouncements:
	case VB_GovernCities:
	case VB_GovernCitizens:
	case VB_GovernCommerce:
	case VB_GovernDiplomacy:
	case VB_GovernPatents:
	case VB_GovernPolicy:
	case VB_GovernTaxes:
		if(!hasEmpire)
			return true;
		// Fall through
	case VB_Comm:
	case VB_Mail:
	case VB_Recall:
		// These buttons are not visible in single user modes or when the scene context is unknown.
		if(sceneContext == SCENE_Unknown)
			return true;
		if(auApp->IsSingleUserDesignerMode())
			return true;
		break;
	case VB_Help:
		if(sceneContext == SCENE_Unknown)
			return true;
		if(auApp->IsSingleUserDesignerMode())
			return true;
		{
			// Minion does not get to call for help.
			const DgData& gd = g_Au.LockDataForRead();
			bool isMinion = gd.ClientPlayerIsMinion();
			g_Au.UnlockData();
			if(isMinion)
				return true;	// Hidden from Minion.
		}
		break;

	case VB_CargoManifest:
	case VB_SpacecraftAccount:
	case VB_SpacecraftMission:
	case VB_SpacecraftPolicy:
	case VB_UnitOrders:
		if(!hasEmpire)
			return true;
		// Fall through
	case VB_Cargo:
	case VB_CargoHold:
	case VB_CargoOrdnance:
	case VB_CargoTransfer:
	case VB_SpacecraftBounties:
	case VB_SpacecraftDamage:
	case VB_SpacecraftRoster:
	case VB_SpacecraftShipments:
		// These buttons are not visible in single user modes or when in the designer.
		if(sceneContext == SCENE_Unknown || sceneContext == SCENE_Designer)
			return true;
		if(auApp->IsSingleUserMode())
			return true;
		break;

	case VB_Building:
	case VB_BdDesignExchange:
		if(sceneContext == SCENE_Arena)
			return true;
		// Fall through
	case VB_Spacecraft:
	case VB_SpacecraftDesignExchange:
		// These buttons are not visible in single user modes.
		if(sceneContext == SCENE_Unknown)
			return true;
		if(auApp->IsSingleUserMode())
			return true;
		break;

	case VB_HideWindows:
		if(auApp->IsSingleUserDesignerMode())
			return true;
		break;

	case VB_Locator:
	case VB_LocatorCompass:
	case VB_LocatorGPS:
	case VB_LocatorStarMap:
	case VB_LocatorSystemScan:
	case VB_LocatorWorldMap:
		if(sceneContext == SCENE_Unknown || sceneContext == SCENE_Designer || sceneContext == SCENE_Arena)
			return true;
		break;

	case VB_Agent:
		{
			const DgData& gd = g_Au.LockDataForRead();
			bool isAgent = gd.ClientPlayerIsAgent();
			if(gd.ClientPlayerIsProgrammer())
				isAgent = false;
			g_Au.UnlockData();
			if(!isAgent)
				return true;
		}
		break;

	case VB_Architect:
		{
			const DgData& gd = g_Au.LockDataForRead();
			bool isArchitect = gd.ClientPlayerIsArchitect();
			g_Au.UnlockData();
			if(!isArchitect)
				return true;
		}
		break;

	case VB_Minion:
		{
			const DgData& gd = g_Au.LockDataForRead();
			bool isMinion = gd.ClientPlayerIsMinion();
			g_Au.UnlockData();
			if(!isMinion)
				return true;	// Only Minion sees the Minion button.
		}
		break;

	case VB_Programmer:
		{
			const DgData& gd = g_Au.LockDataForRead();
			bool isProgrammer = gd.ClientPlayerIsProgrammer();
			if(gd.ClientPlayerIsArchitect())
				isProgrammer = false;
			g_Au.UnlockData();
			if(!isProgrammer)
				return true;
		}
		break;
	}
	return false;
}

bool AuViewButton::IsVisible() const
{
	if(m_Id == AuViewButton::VB_Invalid)
		return false;
	return true;
}

QString AuViewButton::ExpandTip() const
{
	QString tip;
	switch(m_Id)
	{
	case AuViewButton::VB_About:
		tip = QObject::tr("Web Links...");
		break;
	case AuViewButton::VB_Bio:
		tip = QObject::tr("Avatar...");
		break;
	case AuViewButton::VB_Building:
		tip = QObject::tr("Building...");
		break;
	case AuViewButton::VB_Cargo:
		tip = QObject::tr("Cargo...");
		break;
	case AuViewButton::VB_Gear:
		tip = QObject::tr("Hand Craft...");
		break;
	case AuViewButton::VB_Government:
		tip = QObject::tr("Governance...");
		break;
	case AuViewButton::VB_Locator:
		tip = QObject::tr("Locator...");
		break;
	case AuViewButton::VB_Spacecraft:
		tip = QObject::tr("Spacecraft...");
		break;
	default:
		break;
	}
	return tip;
}

bool AuViewButton::HasSubs() const
{
	switch(m_Id)
	{
	case AuViewButton::VB_About:
	case AuViewButton::VB_Bio:
	case AuViewButton::VB_Building:
	case AuViewButton::VB_Cargo:
	case AuViewButton::VB_Gear:
	case AuViewButton::VB_Government:
	case AuViewButton::VB_Locator:
	case AuViewButton::VB_Spacecraft:
		return true;
	default:
		break;
	}
	return false;
}

void AuViewButton::AddToToolBar(QToolBar* tb, AuClientView* view)
{
	QIcon icon(QPixmap(Image()).scaled(32,32));
	if(HasSubs())
	{
		m_Button = new AuToolButtonExpandValue(m_Id, tb);
		m_Button->SetIcon(icon);
		m_Button->SetIconSize(QSize(32,32));
		m_Button->SetToolTip(Tip());
		m_Button->SetExpandToolTip(ExpandTip());
		m_Button->setFixedSize(32,32);
		tb->addWidget(m_Button);
		m_Button->setVisible(false);
		view->connect(m_Button, SIGNAL(Clicked(int)), SLOT(ViewIconButtonClicked(int)));
		view->connect(m_Button, SIGNAL(ExpandClicked(int)), SLOT(ViewIconButtonExpanded(int)));
	}
	else
	{
		m_Action = tb->addAction(icon, Name());
		m_Action->setData(m_Id);
		m_Action->setToolTip(Tip());
		m_Action->setVisible(false);
	}
}

void AuViewButton::SetVisible(bool visible)
{
	if(m_Action)
		m_Action->setVisible(visible);
	else if(m_Button)
		m_Button->setVisible(visible);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

QColor AuRenderMsg::Color() const
{
	// Select a base color that may be modulated according to range.
	QColor color;
	if(Type() == AuCommPacket::SpotTargetOrTerrain)
	{
		// Spot messages are colored according to the political stance of the spotter toward the target.
		color = PoliticalStanceColor(StanceTowardTarget(), IsTargetCivilian());
	}
	else
	{
		// Start with a color based on message scope.
		color = g_Au.Settings().readColorEntry(QString("OnScreenMessageColors/%1").arg(m_Scope),
				AuCommStdChannel::ScopeColor(m_Scope));
	}

	// Log, Narrator and Intercom messages are not modulated according to range.
	if(m_Scope != AuCommStdChannel::Thoughts && !AuCommPacket::IsNarratorMessage(Type()) && m_Scope != AuCommStdChannel::Intercom)
	{
		// Modulate the color according to message range.
		// The color gets dimmer as the actual range gets farther.
		switch(m_Range)
		{
		default:
		case AuCommStdChannel::Close:
			break;
		case AuCommStdChannel::Hail:
			color.setHsvF(color.hueF(), color.saturationF(), color.valueF() - 3.f/100.f);
			break;
		case AuCommStdChannel::SolarSystem:
			color.setHsvF(color.hueF(), color.saturationF(), color.valueF() - 7.f/100.f);
			break;
		case AuCommStdChannel::Sector:
			color.setHsvF(color.hueF(), color.saturationF(), color.valueF() - 12.f/100.f);
			break;
		case AuCommStdChannel::Galaxy:
			color.setHsvF(color.hueF(), color.saturationF(), color.valueF() - 18.f/100.f);
			break;
		case AuCommStdChannel::Universe:
			color.setHsvF(color.hueF(), color.saturationF(), color.valueF() - 24.f/100.f);
			break;
		}
	}

	return color;
}

QString AuRenderMsg::OnScreenMessage() const
{
	if(AuCommPacket::IsNarratorMessage(Type()))
	{
		// Story line narration.
		if(Type() == AuCommPacket::NarrateTitle)
			return Text();
		else if(Type() == AuCommPacket::NarrateChoice)
			return "* " + Text();
		else if(Text().length() > d_SubtitleWordWrap)
			return "    " + Text();	// Indent text that word-wraps.
		else
			return Text();
	}
	else if(FromName().isEmpty())
		return Text();
	else
		return FromName() + ": " + Text();
}

int AuRenderMsg::ToneToPlay()
{
	// This message will only respond once with a real sound effect. Subsequent calls will return SOUND_None.
	if(!m_ToneSounded)
	{
		m_ToneSounded = true;					// Set so we won't respond with a sound if called again.
		if(AuCommPacket::IsThought(m_Type))
			return SOUND_ToneThought;
		else if(m_Type == AuCommPacket::ChannelOpened)
			return SOUND_ChannelOpened;
		else if(m_Type == AuCommPacket::RequestHelp)
			return SOUND_ToneHelp;
		else if(AuCommPacket::IsNarratorMessage(m_Type))
			return SOUND_ToneNarrator;
		else if(AuCommPacket::IsSOSMessage(m_Type))
			return SOUND_ToneSOS;
		else if(m_Type == AuCommPacket::Announcement ||
				m_Type == AuCommPacket::Chat ||
				// has it's own audio m_Type == AuCommPacket::ChatAudio ||
				// usually preceded by a chat message m_Type == AuCommPacket::ChatChoice ||
				// usually preceded by a chat message m_Type == AuCommPacket::ChatContinued ||
				m_Type == AuCommPacket::ChatStory ||
				m_Type == AuCommPacket::CityChat ||
				m_Type == AuCommPacket::CityMessage ||
				m_Type == AuCommPacket::CitySurrenderAccepted ||
				m_Type == AuCommPacket::CitySurrenderOffered ||
				m_Type == AuCommPacket::DiplomaticMessage ||
				m_Type == AuCommPacket::HailReply ||
				m_Type == AuCommPacket::HailRequest ||
				m_Type == AuCommPacket::InviteToEmpire ||
				m_Type == AuCommPacket::Message ||
				m_Type == AuCommPacket::OfferAvatar ||
				m_Type == AuCommPacket::RequestBerthAny ||
				m_Type == AuCommPacket::RequestBerthCaptain ||
				m_Type == AuCommPacket::RequestBerthCaptainSlot ||
				m_Type == AuCommPacket::RequestBerthCrew ||
				m_Type == AuCommPacket::RequestBerthOfficer ||
				m_Type == AuCommPacket::RequestBerthPassenger ||
				m_Type == AuCommPacket::RequestBerthTroop ||
				m_Type == AuCommPacket::Silverwill)
			return SOUND_ToneChat;
	}
	return SOUND_None;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuClientView::AuClientView(QWidget* parent) : QWidget(parent),
	m_MM(!auApp->IsSingleUserMode()),
	m_RenderInfo(0, auApp->Sound(), m_MM, m_ModelCache, g_AuClient.FontAtom()),
	m_SceneEther("AuClientView")
{
	setFocusPolicy(Qt::StrongFocus);
	//setAttribute(Qt::WA_OpaquePaintEvent);

	m_AgentBits = 0;
	m_ModalDlgCount = 0;
	m_InCity = false;
	m_IsShipCommand = false;

	m_RenderInfo.SetTopViewInfo(AuAtomRenderInfo::VI_RocksSome);

	m_SceneAtomEmpire = 0;
	m_SceneAtomEnvType = 0;
	m_SceneAtomTransition = true;
	m_SceneAtomWorldBuildingCount = 0;
	m_SceneAtomWorldBuildingLimit = 0;
	m_SceneAtomWorldBuildingVolume = 0;
	m_SceneContext = SCENE_Unknown;
	m_SceneGalaxy = GALAXY_Count;

	m_RenderInfo.m_ViewAlt = g_Au.Settings().readNumEntry("Camera/TopDownAltitude", m_RenderInfo.m_ViewAlt);
	if(m_RenderInfo.m_ViewAlt < 75 || m_RenderInfo.m_ViewAlt > 2400)
		m_RenderInfo.m_ViewAlt = 300;

	m_RenderInfo.m_ViewAngle1st = g_Au.Settings().readNumEntry("Camera/1stPersonAngle", m_RenderInfo.m_ViewAngle1st*RAD2DEGf);
	if(m_RenderInfo.m_ViewAngle1st < 0 || m_RenderInfo.m_ViewAngle1st > 45)
		m_RenderInfo.m_ViewAngle1st = 0;
	m_RenderInfo.m_ViewAngle1st *= DEG2RADf;

	m_RenderInfo.m_ViewAngle3rd = g_Au.Settings().readNumEntry("Camera/3rdPersonAngle", m_RenderInfo.m_ViewAngle3rd*RAD2DEGf);
	if(m_RenderInfo.m_ViewAngle3rd < 0 || m_RenderInfo.m_ViewAngle3rd > 60)
		m_RenderInfo.m_ViewAngle3rd = 20;
	m_RenderInfo.m_ViewAngle3rd *= DEG2RADf;

	m_RenderInfo.m_ViewDist1st = g_Au.Settings().readNumEntry("Camera/1stPersonDistance", m_RenderInfo.m_ViewDist1st);
	if(m_RenderInfo.m_ViewDist1st < 0 || m_RenderInfo.m_ViewDist1st > 10)
		m_RenderInfo.m_ViewDist1st = 0;

	m_RenderInfo.m_ViewDist3rd = g_Au.Settings().readNumEntry("Camera/3rdPersonDistance", m_RenderInfo.m_ViewDist3rd);
	if(m_RenderInfo.m_ViewDist3rd < 1 || m_RenderInfo.m_ViewDist3rd > 20)
		m_RenderInfo.m_ViewDist3rd = 10;

	m_bWaitingForScene = false;		// True when waiting for scene atom to init.

	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	// Restore squads from settings.
	for(int squadIndex=0; squadIndex<10; squadIndex++)
	{
		m_Squad[squadIndex].clear();
		QString squadName = QString("Squad%1").arg(squadIndex);
		int count = g_Au.Settings().readNumEntry(QString("/%1/Count").arg(squadName), 0);
		for(int unit=0; unit<count; unit++)
		{
			int id = g_Au.Settings().readNumEntry(QString("/%1/Unit%2Id").arg(squadName).arg(unit), 0);
			QString name = g_Au.Settings().readEntry(QString("/%1/Unit%2Name").arg(squadName).arg(unit), "");
			int rank = g_Au.Settings().readNumEntry(QString("/%1/Unit%2Rank").arg(squadName).arg(unit), 0);
			QString rankAbbr = g_Au.Settings().readEntry(QString("/%1/Unit%2RankAbbr").arg(squadName).arg(unit), "");
			QString rankName = g_Au.Settings().readEntry(QString("/%1/Unit%2RankName").arg(squadName).arg(unit), "");
			NPC_ROLE npcRole = (NPC_ROLE)g_Au.Settings().readNumEntry(QString("/%1/Unit%2Role").arg(squadName).arg(unit), 0);
			if(id && !name.isEmpty())
				m_Squad[squadIndex].Add(AuSelectedUnit(AuId(id), name, rank, rankAbbr, rankName, npcRole));
		}
	}

	// View Buttons
	QToolBar* tb = new QToolBar(this);
	tb->setOrientation(Qt::Vertical);
	connect(tb, SIGNAL(actionTriggered(QAction*)), SLOT(ActionClicked(QAction*)));
	{
		const DgData& gd = g_Au.LockDataForRead();
		bool isAgent = gd.ClientPlayerIsAgent();
		g_Au.UnlockData();
		class AuViewButton sceneClient[] =
		{
			AuViewButton(AuViewButton::VB_About),
			AuViewButton(AuViewButton::VB_Mail),
			AuViewButton(AuViewButton::VB_Comm),
			AuViewButton(AuViewButton::VB_Spacecraft),
			AuViewButton(AuViewButton::VB_UnitOrders),
			AuViewButton(AuViewButton::VB_Gear),
			AuViewButton(AuViewButton::VB_Cargo),
			AuViewButton(AuViewButton::VB_Locator),
			AuViewButton(AuViewButton::VB_Bio),
			AuViewButton(AuViewButton::VB_Building),
			AuViewButton(AuViewButton::VB_Government),
			AuViewButton(AuViewButton::VB_Architect),
			AuViewButton(AuViewButton::VB_Programmer),
			AuViewButton(AuViewButton::VB_Agent),
			//AuViewButton(AuViewButton::VB_Minion),
			AuViewButton(AuViewButton::VB_PlayersOnline),
			AuViewButton(AuViewButton::VB_Help),
			AuViewButton(AuViewButton::VB_Recall,
					isAgent ?  tr("<html><b>Recall Home</b> %2<br>"
						"Teleports your avatar to your home location, in this order of precedence.<br>"
						"  1. <b>Spacecraft</b> where you hold a berth. "
						"You recall to your berth or standing at the helm if the ship has no berths. "
						"<b>E to exit a berth</b>. "
						"A berth provides a breathable environment. "
						"Ship life support must be functioning to recall aboard.<br>"
						"  2. <b>Home city</b>, if that city is friendly to your empire. "
						"You appear near where you stood when you declared the city your home.<br>"
						"  3. <b>Capital city</b> of your empire, if that city is controlled by your empire.<br>"
						"  4. <b>Randomly selected city</b> of your empire.<br>"
						"  5. <b>Empty world</b> that is habitable.<br>"
						"A <i>period of vulnerability</i> ensues while waiting to recall, during which you can be seen and "
						"injured.<br>"
						"<b>Cancel recall</b> by pressing the Recall Home button while waiting to recall.<br>"
						"<b><u>Equipment and money are lost if you recall to a different dimension. "
						"</b></u>"
						"Sea chest contents are not lost; your sea chest is different in each zone: conflict or neutral.<br>"
						"<b>Agent Power</b> When agent power is enabled, you do not lose your stuff."
						"</html>") : ""),
			AuViewButton(AuViewButton::VB_HideWindows),
			AuViewButton(AuViewButton::VB_FullScreen),
			AuViewButton(AuViewButton::VB_Settings),
			AuViewButton(AuViewButton::VB_Exit),
		};

		int buttons = sizeof(sceneClient)/sizeof(AuViewButton);
		for(int but=0; but<buttons; but++)
		{
			sceneClient[but].AddToToolBar(tb, this);
			m_ViewButtons.append(sceneClient[but]);
		}
	}

	AuNetworkActivity* na = new AuNetworkActivity(this);
	na->setFixedSize(48,48);
	na->setToolTip(tr("<html>Network activity is shown as blue outgoing and red incoming sprites.</html>"));

	QVBoxLayout* vlay = new QVBoxLayout;
	vlay->addWidget(na);
	vlay->addStretch();

	m_Browser = new QTextBrowser;
	m_Browser->setOpenExternalLinks(true);

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(tb);
	lay->addWidget(m_Browser);
	lay->addLayout(vlay);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);

	m_SceneTimer = new QTimer(this);
	connect(m_SceneTimer, SIGNAL(timeout()), this, SLOT(AnimateScene()));
	m_SceneTimer->start(500);
}

AuClientView::~AuClientView()
{
	//qDebug() << "AuClientView::~AuClientView";
	// Save last known whereabouts of the scene atom for display on the login screen.
	// Does nothing if there is no scene atom.
	SaveWhereabouts();
}

// emits the SceneContextChanged signal if the view context actually changes.
void AuClientView::SetSceneContext(SCENE_CONTEXT sceneContext)
{
	if(m_SceneContext != sceneContext)
	{
		//qDebug() << "AuClientView::SetSceneContext changed from" << m_SceneContext << "to" << sceneContext;
		m_SceneContext = sceneContext;
		emit SceneContextChanged(m_SceneContext);
	}
}

void AuClientView::AnimateScene()
{
	m_SceneEther.DeleteTrashAtom();		// Delete a trash atom from my scene ether. There is no separate trash thread to do it.

	SyncScene();						// Sync in this thread.
	RenderCommMessages();
	UpdateViewButtonVisibility();
	if(m_bWaitingForScene)
	{
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("View: Waiting for %1...").arg(m_idWaitScene.StringId()));
		//qDebug("AuClientView::AnimateScene waiting");

		// Check to see if the scene atom has arrived.
		if(!m_idWaitScene.IsNull() && !m_SceneEther.FindNormalAtom(m_idWaitScene))
		{
			//qDebug("AuClientView::AnimateScene - waiting for scene atom...still waiting");
			return;
		}

		// The scene atom is now present so we can finally change the scene.
		//qDebug("AuClientView::AnimateScene - waiting for scene atom...SCENE "
				//"ATOM HAS ARRIVED");
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("View: Scene atom has arrived"));
		SetSceneAtom(m_idWaitScene);
	}
	else if(m_idSceneAtom.IsNull())
	{
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("View: Waiting..."));
		SetSceneContext(SCENE_Unknown);
		return;	// When scene atom is null, wait for one from the main frame.
	}
	else if(m_idSceneAtom != m_idWaitScene)
	{
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt",
					QString("View: Scene atom changed from %1 to %2.")
					.arg(m_idSceneAtom.StringId()).arg(m_idWaitScene.StringId()));

		if(!m_idWaitScene.IsNull() && m_SceneEther.FindNormalAtom(m_idWaitScene))
			SetSceneAtom(m_idWaitScene);	// The scene atom is now ready to render the scene so change the scene.
	}

	SetSceneContext(SCENE_Universe);

	// Render narrative audio.
	auApp->PlayQueuedSounds(g_Au.SoundEffectsGain());
}

void AuClientView::UpdateViewButtonVisibility()
{
	bool hasEmpire = SceneAtomEmpire();
	for(QList<AuViewButton>::iterator bi = m_ViewButtons.begin(); bi != m_ViewButtons.end(); bi++)
		(*bi).SetVisible((*bi).IsVisible() && !(*bi).IsHidden(SceneContext(), hasEmpire));
}

void AuClientView::TabKeyPressed(Qt::KeyboardModifiers mods)
{
	Q_UNUSED(mods);
}

// Commands this window to use a particular atom as the scene.
// Triggers the model to begin acquiring the necessary data.
// It may take a while before the new scene atom is ready.
// While waiting, m_bWaitingForScene and m_idWaitScene are set.
// The new scene can be set up to look at a particular sub-atom.
// If 'idScene' cannot render a scene, the next highest parent atom that can render a scene is used.
void AuClientView::SetSceneAtom(const AuId& idScene)
{
	//qDebug() << "AuClientView::SetSceneAtom" << idScene;
	// Tell the app which atom we want as the scene atom.
	// It may take some time for my scene data to catch up.
	auApp->SetAvatarId(idScene);
	m_bWaitingForScene = false;
	m_idWaitScene = idScene;									// Remember the atoms we really want to look at.

	if(idScene.IsNull())
	{
		m_idSceneAtom = idScene;
		DpConsole::RestoreAdjustments(m_idSceneAtom);			// Restore the scene atom's console screen adjustments.

		UpdateSceneInfo(0);

		//qDebug() << "AuClientView::SetSceneAtom emit SceneAtomChanged";
		emit SceneAtomChanged();
		return;
	}

	// Check if the scene atom has shown up in my local data yet.
	m_SceneLock.lockForRead();
	DaPerson* pSceneAtom = dynamic_cast<DaPerson*>(m_SceneEther.FindNormalAtom(idScene));
	if(!pSceneAtom)
	{
		//qDebug("AuClientView::SetSceneAtom - NOT FOUND, waiting for scene...");
		m_SceneLock.unlock();
		m_bWaitingForScene = true;
		return;
	}

	if(!pSceneAtom->Is(m_idSceneAtom))
	{
		// A new atom becomes the scene atom.
		m_idSceneAtom = pSceneAtom->Id();
		DpConsole::RestoreAdjustments(m_idSceneAtom);		// Restore the scene atom's console screen adjustments.

		// Check to see if the name matches what we got from the login. It will be different when a new character was just created.
		const DgData& gd = g_Au.LockDataForRead();
		AuId avId = gd.ClientUnitId();
		QString avName = gd.ClientUnitName(avId);
		g_Au.UnlockData();
		if(avName != pSceneAtom->Name())
			auApp->RequestAvatars();
	}

	// Get agent bits now for agent ui that will receive the SceneAtomChanged signal.
	UpdateSceneInfo(pSceneAtom);

	m_SceneLock.unlock();

	emit SceneAtomChanged();
}

void AuClientView::LockSceneAtom(DaPerson** sceneAtom)
{
	m_SceneLock.lockForRead();
	if(!m_idSceneAtom.IsNull())
		*sceneAtom = dynamic_cast<DaPerson*>(m_SceneEther.FindNormalAtom(m_idSceneAtom));
	else
		*sceneAtom = 0;
}

void AuClientView::LockSceneAtomForWrite(DaPerson** sceneAtom)
{
	// Set up the scene atom pointers.
	m_SceneLock.lockForWrite();
	if(!m_idSceneAtom.IsNull())
		*sceneAtom = dynamic_cast<DaPerson*>(m_SceneEther.FindNormalAtom(m_idSceneAtom));
	else
		*sceneAtom = 0;
}

void AuClientView::UnlockSceneAtom()
{
	m_SceneLock.unlock();
}

// Called by ProcessAtomControls. 'sceneAtom' may be null.
void AuClientView::UpdateSceneInfo(DaPerson* sceneAtom)
{
	//qDebug() << "AuClientView::UpdateSceneInfo scene" << sceneAtom;
	m_SceneInfoLock.lockForWrite();
	m_SceneAtomInShip = AuId(0);
	m_IsShipCommand = false;
	m_SceneAtomWorldBuildingCount = 0;
	m_SceneAtomWorldBuildingLimit = 0;
	m_SceneAtomWorldBuildingVolume = 0;
	if(sceneAtom)
	{
		const DaSector* sector = sceneAtom->FindParentSector();
		if(sector)
		{
			m_idSceneSector = sector->Id();
			m_SceneGalaxy = sector->Galaxy();
		}
		else
		{
			m_idSceneSector = AuId(0);
			m_SceneGalaxy = GALAXY_Count;
		}

		const DaSolarSystem* system = sceneAtom->FindParentSolarSystemInEnv();
		if(system)
			m_idSceneSolarSystem = system->Id();
		else
			m_idSceneSolarSystem = AuId();

		m_SceneAtomTransition = sceneAtom->IsInTransition();
		if(m_SceneAtomEmpire != sceneAtom->EmpireId())
		{
			// The scene atom's empire changed so we must request the empire record again. We don't receive all the data for empires
			// that we are not a member of. This gets us all the data for the empire record in case we already had an abbreviated
			// version of it.
			m_SceneAtomEmpire = sceneAtom->EmpireId();
			auApp->RequestEmpire(m_SceneAtomEmpire);	// Does nothing if empire is 0.
		}
		m_SceneAtomName = sceneAtom->Name();
		m_SceneAtomScopeShip = sceneAtom->SpawnShip();
		if(sceneAtom->InDesignPreview())
		{
			// In design preview, grab the id of the preview ship so we can give orders to the preview crew.
			const DaAtom* envAtom = sceneAtom->FindEnvironmentAtom();
			if(envAtom)
			{
				DaAtomList vessels;
				envAtom->FindVesselsRecursive(vessels);
				if(!vessels.empty())
					m_SceneAtomScopeShip = vessels.first()->Id();
			}
		}

		// Get owner id for Company context messages.
		m_SceneAtomOwnerId = sceneAtom->Id();
		if(sceneAtom->SpawnShipIsPrivate() && AuRosterEntry::IsOfficer(sceneAtom->SpawnShipBerthType()))
		{
			// Scene atom is a ship's officer aboard a private ship. Can we find the ship?
			const DaAtom* envAtom = sceneAtom->FindEnvironmentAtom();
			if(envAtom)
			{
				const DaVessel* ship = dynamic_cast<const DaVessel*>(envAtom->FindIdRecursive(sceneAtom->SpawnShip()));
				if(ship && ship->IsPrivatelyOwned() && !ship->OwnerId().IsNull())
					m_SceneAtomOwnerId = ship->OwnerId();
			}
		}

		const DaWorldLL* world = sceneAtom->FindParentWorldInEnv();
		if(world)
		{
			m_SceneAtomWorldBuildingCount = world->BuildingCount();
			m_SceneAtomWorldBuildingLimit = world->BuildingVolumeLimit();
			m_SceneAtomWorldBuildingVolume = world->BuildingVolume();
			if(!world->HasAtmosphere())
				m_SceneAtomEnvType = 4;	// moon
			else if(world->IsGasGiant() || (sceneAtom->IsEyeUnderWater() && sceneAtom->OrbitZone() != OZONE_Inferno))
				m_SceneAtomEnvType = 3;	// under water
			else if(world->CanHaveFlora())
				m_SceneAtomEnvType = 2;	// plants
			else
				m_SceneAtomEnvType = 1;	// desert
		}
		else
			m_SceneAtomEnvType = 0;	// space

		m_AgentBits = sceneAtom->m_AgentBits;
		m_InCity = (sceneAtom->InIndustry() != IID_None && !sceneAtom->IsInSpacecraft());	// In city and not in spacecraft

		if(sceneAtom->IsInsideSpacecraft())
		{
			const DaVessel* ship = sceneAtom->FindParentVesselInEnv();
			if(ship)
			{
				m_IsShipCommand = ship->InChainOfCommand(sceneAtom);
				m_SceneAtomInShip = ship->Id();
			}
		}
	}
	else
	{
		m_idSceneSector = AuId();
		m_idSceneSolarSystem = AuId();
		m_SceneGalaxy = GALAXY_Count;

		//m_SceneAtomEmpire = 0;
		m_SceneAtomName = "";
		m_SceneAtomScopeShip = AuId();
		m_SceneAtomTransition = true;

		m_AgentBits = 0;
		m_InCity = false;
	}
	m_SceneInfoLock.unlock();
}

// Returns true if the selection actually changes.
bool AuClientView::SelectUnit(const AuId& id, const QString& name, int rank, const QString& rankAbbr, const QString& rankName,
		NPC_ROLE npcRole, bool selectIt)
{
	if(selectIt)
	{
		// Select the unit by adding it to my list of selected units.
		if(!m_Selected.Contains(id))
		{
			m_Selected.Add(AuSelectedUnit(id, name, rank, rankAbbr, rankName, npcRole));
			return true;
		}
	}
	else
	{
		// Deselect the unit by removing it from my list of selected units.
		if(m_Selected.Contains(id))
		{
			m_Selected.Remove(id);
			return true;
		}
	}
	return false;
}

bool AuClientView::SelectUnitToggle(DaPerson* sceneAtom, DaAtom* pSelect)
{
	// Only select citizens for now.
	DaUnit* unit = dynamic_cast<DaUnit*>(pSelect);
	if(!unit)
		return false;
	if(unit->EmpireId() != sceneAtom->EmpireId())
		return false;

	// Rank sort so the first selected unit is the highest ranking unit. He's the dude who acknowledges orders for the group.
	QString name = pSelect->Name();
	int rank;
	QString rankAbbr, rankName;
	DaCitizen* citizen = dynamic_cast<DaCitizen*>(unit);
	NPC_ROLE npcRole;
	if(citizen)
	{
		rank = citizen->Rank() + 1;
		rankAbbr = citizen->RankAbbr();
		rankName = citizen->RankName();
		if(citizen->IsNpcOfficer())
			rank += 30;	// Officer ranks count higher than the ranks of crew or troops.
		else if(citizen->IsNpcCrew())
			rank += 20;	// Crew ranks count higher than the ranks of troops.
		else if(citizen->IsNpcTroop())
			rank += 10;	// Troop ranks count higher than the ranks of civilians.

		if(name.left(rankAbbr.length()) == rankAbbr)
			name = name.mid(rankAbbr.length()+1);

		npcRole = citizen->NpcRole();
	}
	else
	{
		rank = 0;
		npcRole = ROLE_Count;
	}

	SelectUnitToggle(pSelect->Id(), name, rank, rankAbbr, rankName, npcRole);
	return true;
}

void AuClientView::SelectUnitToggle(const AuId& id, const QString& name, int rank, const QString& rankAbbr, const QString& rankName,
		NPC_ROLE npcRole)
{
	if(m_Selected.Contains(id))
		m_Selected.Remove(id);
	else
		m_Selected.Add(AuSelectedUnit(id, name, rank, rankAbbr, rankName, npcRole));
}

void AuClientView::DeselectAllUnits()
{
	m_Selected.clear();
}

// Add a new atom to my display objects.
void AuClientView::AtomAdded(DaAtom* atom)
{
	DaPerson* sceneAtom;
	LockSceneAtomForWrite(&sceneAtom);
	DaAtom* vwPar = m_SceneEther.FindNormalAtom(atom->ParentId());	// Find the view's copy of the parent atom.
	if(!vwPar)
	{
		if(!atom->ParentId().IsNull())
			qWarning() << "error: AuClientView::AtomAdded parentId" << atom->ParentId() << "not found in view";
	}
	m_SceneEther.AddAtom(atom);

	// Dirty the galaxy sector locs cache when adding an atom. We only ever add sectors to galaxy atoms.
	DaGalaxy* galaxy = dynamic_cast<DaGalaxy*>(vwPar);
	if(galaxy)
	{
		galaxy->InvalidateSectorLocCache();			// Cache will be recreated when needed.
	}
	else
	{
		// Dirty the terrain cache when adding a new city atom.
		DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(vwPar);
		if(llworld)
		{
			DaBd* newbd = dynamic_cast<DaBd*>(atom);
			if(newbd)
			{
				if(newbd->HasEnclosedEnvironment())
					llworld->InvalidateEnclosureCache();
				AuMatrix4x4 mBld(newbd->Transform());
				mBld.Translate(newbd->SiteCenter());
				double siteRadius = newbd->SiteBoxRadius().Length() + 10*FeetPerMeter();
				llworld->BuildingChanged(mBld, siteRadius, m_MM);	// Trigger update of terrain.
			}
		}
	}

	UnlockSceneAtom();
}

// Remove the rows from my display objects.
void AuClientView::AtomDeparted(const AuId& atomId)
{
	m_SceneEther.TrashAtom(atomId);
}

void AuClientView::AtomParentChanged(const DaAtom* atom)
{
	DaPerson* sceneAtom;
	LockSceneAtomForWrite(&sceneAtom);
	DaAtom* pMyChild = m_SceneEther.FindNormalAtom(atom->Id());
	if(!pMyChild)
	{
		UnlockSceneAtom();
		return;
	}
	AuAtomPos apos(atom->Pos());
	AuAtomPos mpos(pMyChild->Pos());
	pMyChild->ChangePos(apos);
	pMyChild->InvalidateEnvironment();
	UnlockSceneAtom();
}

void AuClientView::AtomChanged(DaAtom* atom)
{
	DaPerson* sceneAtom;
	LockSceneAtomForWrite(&sceneAtom);
	bool invalidEnv = false;
	DaAtom* pMyAtom;
	if(atom->Is(m_idSceneAtom))
	{
		// Refreshing the scene atom.
		pMyAtom = sceneAtom;
		if(!pMyAtom)
		{
			UnlockSceneAtom();
			return;
		}
		invalidEnv = true;
	}
	else
	{
		pMyAtom = m_SceneEther.FindNormalAtom(atom->Id());
		if(!pMyAtom)
		{
			UnlockSceneAtom();
			return;
		}

		if(sceneAtom)
		{
			DaVehicle* vehicle = dynamic_cast<DaVehicle*>(pMyAtom);
			if(vehicle)
			{
				if(vehicle == sceneAtom->Parent())
					invalidEnv = true;	// Scene atom's parent vehicle changed.
			}
			else
			{
				DaCapsule* ship = dynamic_cast<DaCapsule*>(pMyAtom);
				if(ship)
				{
					if(ship == sceneAtom->Parent() || ship->FindIdRecursive(m_idSceneAtom))
						invalidEnv = true;	// Scene atom's spacecraft or building changed.
				}
			}
		}
	}

	// Refresh view atom from model data.
	pMyAtom->Refresh(atom, &m_RenderInfo);
	if(invalidEnv)
		sceneAtom->InvalidateEnvironment();
	UnlockSceneAtom();
}

void AuClientView::AtomsCleared()
{
	//qDebug() << "AuClientView::AtomsCleared";
	m_SceneLock.lockForWrite();
	m_SceneEther.DeleteEther();
	m_SceneLock.unlock();
	m_SceneAtomTransition = true;
}

QString AuClientView::DesignAnalysis()
{
	QString da;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		if(sceneAtom->IsInsideSpacecraft())
		{
			// Person is not in a design environment.
			// Report the design analysis of the ship they are aboard.
			const DaVessel* ship = sceneAtom->FindParentVesselInEnv();
			if(ship)
			{
				QString empireName;
				const DgData& gd = g_Au.LockDataForRead();
				gd.EmpireName(ship->EmpireId(), empireName);
				bool isAgent = gd.ClientPlayerIsAgent();		// Show design tips to agents.
				g_Au.UnlockData();

				if(ship->WhereMade().isEmpty())
					da = QString("<b>%1</b><br>%2<br>%3<br>Manufactured %4<hr>")
						.arg(ship->Name())
						.arg(empireName)
						.arg(ship->IsPrivatelyOwned()?"Company":"Fleet")
						.arg(ship->CreatedTime().UTC().toLocalTime().date().toString());
				else
					da = QString("<b>%1</b><br>%2<br>%3<br>Manufactured %4<br>%5<hr>")
						.arg(ship->Name())
						.arg(empireName)
						.arg(ship->IsPrivatelyOwned()?"Company":"Fleet")
						.arg(ship->CreatedTime().UTC().toLocalTime().date().toString())
						.arg(ship->WhereMade());
				da += ship->DesignReport(ship->QualityMade(), isAgent);
			}
		}
	}
	UnlockSceneAtom();
	return da;
}

void AuClientView::ChatReceived(AuCommPacket* msg)
{
	switch(msg->Type())
	{
	default:
		break;

	case AuCommPacket::ChatAudio:
		{
			// ChatAudio messages must attach the sound to the originating atom in the scene.
			const AuCommChatAudio* msgChatAudio = dynamic_cast<const AuCommChatAudio*>(msg);
			if(msgChatAudio && auApp->Sound())
			{
				DaPerson* sceneAtom;
				LockSceneAtom(&sceneAtom);
				if(sceneAtom)
				{
					DaNpc* speaker = dynamic_cast<DaNpc*>(m_SceneEther.FindNormalAtom(msg->FromId()));
					if(speaker)
						speaker->QueueSound(msgChatAudio->SoundData(), *auApp->Sound());
				}
				UnlockSceneAtom();
			}
		}
		break;

	case AuCommPacket::NarrateAudio:
		if(auApp->Sound())
		{
			// Narrator audio is processed by the (this) view.
			const AuCommNarrateAudio* msgNarrateAudio = dynamic_cast<const AuCommNarrateAudio*>(msg);
			if(msgNarrateAudio)
				auApp->QueueSound(msgNarrateAudio->SoundData());
		}
		break;

	case AuCommPacket::ChatImage:
		{
			const AuCommChatImage* msgChatImage = dynamic_cast<const AuCommChatImage*>(msg);
			if(msgChatImage)
			{
				AuImageDialog* idlg = new AuImageDialog(msgChatImage->ImageTitle(), msgChatImage->Text(), msgChatImage->Image(),
						this);
				idlg->show();
			}
		}
		break;
	case AuCommPacket::NarrateImage:
		{
			const AuCommNarrateImage* msgNarrateImage = dynamic_cast<const AuCommNarrateImage*>(msg);
			if(msgNarrateImage)
			{
				AuImageDialog* idlg = new AuImageDialog(msgNarrateImage->ImageTitle(), msgNarrateImage->Text(),
						msgNarrateImage->Image(), this);
				idlg->show();
			}
		}
		break;
	}

	// Filter comm messages. Not all of them are wanted on the screen.
	// Check filter based on channel.
	if(AuCommStdChannel::IsStandardChannel(msg->ChannelName()))
	{
		// Standard channels may be individually filtered out.
		if(!g_Au.Settings().readNumEntry(QString("OnScreenMessageChannels/%1").arg(msg->ChannelName()), 1))
			return;	// The standard channel messages are not wanted on the screen.
	}
	else
	{
		// Non-standard channels may be filtered out.
		if(!g_Au.Settings().readNumEntry("OnScreenMessages/Show Non-Standard", 1))
			return;	// Non-standard channel messages are not wanted on the screen.
	}

	// Check filter based on message type.
	if(!g_Au.Settings().readNumEntry(QString("OnScreenMessageTypes/%1").arg(msg->Type()), msg->OnScreenByDefault()))
		return;		// The message type is not wanted on the screen.

	//qDebug("AuClientView::ChatReceived");
	AddMessage(AuRenderMsg(msg));
}

/*
void AuClientView::PlayClientSound(int sound, float gain, float pitch)
{
	Q_UNUSED(pitch);
	// Modulate gain using individual sound setting.
	gain *= g_Au.SoundGain((SOUND_EFFECT)sound);
	if(gain == 0)
		return;

	// Check for overridden sound wav.
	AuSoundOverride soundO;
	bool overridden;
	if(g_Au.SoundOverride((SOUND_EFFECT)sound, soundO))
		overridden = !soundO.FileName().isEmpty();
	else
		overridden = false;
	if(overridden)
	{
		//qDebug() << "AuClientView::PlayClientSound" << sound << "gain" << gain << "overridden" << soundO.FileName();
		if(!m_Sound)
			m_Sound = new QSoundEffect(this);
		m_Sound->setSource(QUrl::fromLocalFile(soundO.FileName()));
		m_Sound->setVolume(gain);
		m_Sound->play();
	}
	else
	{
		const AuGLSoundInfo& si = AuGLEffects::SoundInfo((SOUND_EFFECT)sound);
		//qDebug() << "AuClientView::PlayClientSound" << sound << "gain" << gain << "file" << si.FileName();
		if(!m_Sound)
			m_Sound = new QSoundEffect(this);
		m_Sound->setSource(QUrl::fromLocalFile(si.FileName()));
		m_Sound->setVolume(gain);
		m_Sound->play();
	}
}
*/

void AuClientView::RenderCommMessages()
{
	//qDebug("AuClientView::RenderCommMessages %d messages intensity %d", m_Messages.count(), m_MessageIntensity);
	m_MessageLock.lock();
	QVector<AuRenderMsg> msgs(m_Messages);				// Recent comm messages.
	m_Messages.clear();
	m_MessageLock.unlock();
	if(msgs.empty())
		return;

	QString nuHtml;
	for(QVector<AuRenderMsg>::iterator mi = msgs.begin(); mi != msgs.end(); mi++)
	{
		QColor color = (*mi).Color();
		QString size;
		switch((*mi).Range())
		{
		case AuCommStdChannel::Close:
			size = "<big><big><big><big>%1</big></big></big></big>";
			break;
		case AuCommStdChannel::Hail:
			size = "<big><big><big>%1</big></big></big>";
			break;
		case AuCommStdChannel::SolarSystem:
			size = "<big><big>%1</big></big>";
			break;
		case AuCommStdChannel::Sector:
			size = "<big>%1</big>";
			break;
		case AuCommStdChannel::Galaxy:
		default:
			size = "%1";
			break;
		case AuCommStdChannel::Universe:
			size = "<small>%1</small>";
			break;
		}

		// Story line narration.
		if((*mi).Type() == AuCommPacket::NarrateTitle)
			size = "<big><big><big><big><big>%1</big></big></big></big></big>";

		// NOTE: Each message will return a sound effect to play only once. Subsequent ToneToPlay calls will return SOUND_None.
		//       That way each message does not make a sound in every frame that we render it, only the first frame that it is
		//       rendered.
		SOUND_EFFECT sound = (SOUND_EFFECT)(*mi).ToneToPlay();
		if(sound != SOUND_None)
			auApp->PlayClientSound(sound);

		color.setRedF(color.redF());
		color.setGreenF(color.greenF());
		color.setBlueF(color.blueF());

		QString text = (*mi).OnScreenMessage();
		if(!nuHtml.isEmpty())
			nuHtml += "<br>";
		nuHtml += size.arg(QString("<span style=\"color: %2;\">%1</span>").arg(text).arg(color.name()));
	}
	QString curHtml = m_Browser->toHtml();
	QScrollBar* scroll = m_Browser->verticalScrollBar();
	int scrollValue = scroll ? scroll->value() : 0;							// current position of the vertical scroll bar
	bool scrollAtMax = scroll ? scrollValue == scroll->maximum() : false;	// true if scroll bar is currently at the max.
	if(scrollAtMax)
	{
		m_Browser->setHtml(curHtml + nuHtml);
		scroll->setValue(scroll->maximum());
	}
	else if(scroll)
	{
		m_Browser->setHtml(curHtml + nuHtml);
		scroll->setValue(scrollValue);
	}
	else
	{
		m_Browser->setHtml(curHtml + nuHtml);
		scroll = m_Browser->verticalScrollBar();	// did we grow a scroll bar? Can this happen?
		if(scroll)
			scroll->setValue(scroll->maximum());
	}
}

void AuClientView::SpotTargetOrTerrain(bool logOnly)
{
	Q_UNUSED(logOnly);
}

void AuClientView::SpawnRecall()
{
	if(SceneAtomInTransition())
		return;

	// Forward the spawn command to the model for processing.
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DiControlInputArray aActiveControls;
		aActiveControls.append(DiControlInput(0, DaPerson::CI_SpawnRecall));
		auApp->ProcessControlInput(sceneAtom->Id(), aActiveControls, AuGearPos(), 0, 0);
	}
	UnlockSceneAtom();
}

void AuClientView::SpawnTie()
{
	// Spawn tie depends on scene atom being in a city.
	if(m_InCity && !SceneAtomInTransition())
	{
		// Forward the spawn command to the model for processing.
		DaPerson* sceneAtom;
		LockSceneAtom(&sceneAtom);
		if(sceneAtom)
		{
			DiControlInputArray aActiveControls;
			aActiveControls.append(DiControlInput(0, DaPerson::CI_SpawnTie));
			auApp->ProcessControlInput(sceneAtom->Id(), aActiveControls, AuGearPos(), 0, 0);
		}
		UnlockSceneAtom();
	}
}

void AuClientView::SquadCreate(int squadIndex)
{
	Q_ASSERT(squadIndex >= 0 && squadIndex < 10);
	m_Squad[squadIndex] = m_Selected;

	// Store squad in settings.
	QString squadName = QString("Squad%1").arg(squadIndex);
	g_Au.Settings().writeEntry(QString("/%1/Count").arg(squadName), m_Selected.count());
	int unit=0;
	for(AuSelectedUnitArray::const_iterator si = m_Selected.constBegin(); si != m_Selected.constEnd(); si++, unit++)
	{
		g_Au.Settings().writeEntry(QString("/%1/Unit%2Id").arg(squadName).arg(unit), (int)(*si).Id().GetID());
		g_Au.Settings().writeEntry(QString("/%1/Unit%2Name").arg(squadName).arg(unit), (*si).Name());
		g_Au.Settings().writeEntry(QString("/%1/Unit%2Rank").arg(squadName).arg(unit), (*si).Rank());
		g_Au.Settings().writeEntry(QString("/%1/Unit%2RankAbbr").arg(squadName).arg(unit), (*si).RankAbbr());
		g_Au.Settings().writeEntry(QString("/%1/Unit%2RankName").arg(squadName).arg(unit), (*si).RankName());
		g_Au.Settings().writeEntry(QString("/%1/Unit%2Role").arg(squadName).arg(unit), (*si).NpcRole());
	}

	if(squadIndex == 0)
		squadIndex = 10;
	LocalLogMessage(tr("Squad %1 formed with %2 units.").arg(squadIndex).arg(m_Selected.count()));
}

void AuClientView::SquadSelect(int squadIndex, bool addToCurrentSelection)
{
	Q_ASSERT(squadIndex >= 0 && squadIndex < 10);
	int squadNum = squadIndex;
	if(squadNum == 0)
		squadNum = 10;
	if(addToCurrentSelection)
	{
		// Add each squad member to the current selection if they are not already selected.
		for(AuSelectedUnitArray::const_iterator si = m_Squad[squadIndex].constBegin(); si != m_Squad[squadIndex].constEnd(); si++)
			if(!m_Selected.Contains((*si).Id()))
				m_Selected.append(*si);
		//LocalLogMessage(tr("Squad %1 added to current selection.").arg(squadNum));
		LocalLogMessage(tr("Squad %1 selected.").arg(squadNum));
	}
	else
	{
		// Replace the current selection with the squad selection set.
		m_Selected = m_Squad[squadIndex];
		LocalLogMessage(tr("Squad %1 selected.").arg(squadNum));
	}
}

void AuClientView::SquadIssueOrder(CITIZEN_ORDER order, const QString& orderText)
{
	if(m_Selected.isEmpty())
		return;

	DaPerson* person;
	LockSceneAtom(&person);
	AuId sceneParent;
	AuPoint sceneLoc;
	AuId orderAtom;
	AuPoint orderLoc;
	if(person)
	{
		// Build a roster of the troops in the scene.
		sceneParent = person->ParentId();	// Scene atom's parent atom.
		sceneLoc = person->Location();		// Scene atom's location relative to the parent atom.
		// Issue the order to the selected troops as a comm message.
		// The orderAtom and orderLoc depend upon the type of order issued, as defined in audef.h.
		switch(order)
		{
		case ORDER_BattleStation:		// Target atom is the parent of the atom receiving the order. It is set on the server.
		case ORDER_CloseDoor:			// Target atom is the parent of the atom receiving the order. It is set on the server.
		case ORDER_DropRider:			// Target atom is the parent of the atom receiving the order. It is set on the server.
		case ORDER_EnterSurgery:		// Target atom is the parent of the atom receiving the order. It is set on the server.
		case ORDER_Garrison:			// Target atom is the parent of the atom receiving the order. It is set on the server.
		case ORDER_GetOut:				// Target atom is the parent of the atom receiving the order. It is set on the server.
		case ORDER_OpenDoor:			// Target atom is the parent of the atom receiving the order. It is set on the server.
		case ORDER_Stay:				// Target atom is the parent of the atom receiving the order. It is set on the server.
		case ORDER_Sleep:				// Target atom is the parent of the atom receiving the order. It is set on the server.
		case ORDER_SleepDayDream:
		case ORDER_SleepEat:
		case ORDER_SleepExercise:
		case ORDER_SleepGame:
		case ORDER_SleepLetter:
		case ORDER_SleepMovie:
		case ORDER_SleepMusic:
		case ORDER_SleepPine:
		case ORDER_SleepPhone:
		case ORDER_SleepRead:
		case ORDER_SleepTalk:
		case ORDER_SleepText:
		case ORDER_SleepTV:
		case ORDER_SleepWeb:
		case ORDER_DamageReport:
		case ORDER_SurveyScanPlanet:
		case ORDER_SurveyScanSystem:
		case ORDER_TacticalScan:
		case ORDER_TakeCommand:
		case ORDER_ReleaseCommand:
		case ORDER_HengeCeremony:
		case ORDER_GoToRoom:
			break;
		case ORDER_AttackTarget:		// Target atom is the atom to attack.
		case ORDER_FollowTarget:		// Target atom is the atom to follow.
		case ORDER_MountTarget:			// Target atom is the atom to mount.
		case ORDER_GetItem:				// Target atom is the atom to get.
		case ORDER_GoToTarget:			// Target atom is the atom to move close to.
		case ORDER_GoToTargetExactly:	// Target atom is the atom to move close to.
			orderAtom = SpottedAtom();
			orderLoc = SpottedLocation();
			m_OrderId = orderAtom;
			m_OrderLoc = orderLoc;
			m_OrderTime.Set();
			break;
		case ORDER_ComeHere:			// Target atom is the parent atom of the atom who issued the order.
		case ORDER_ComeHereExactly:		// Target atom is the parent atom of the atom who issued the order.
		case ORDER_GetIn:				// Target atom is the parent atom of the atom who issued the order.
			orderAtom = sceneParent;
			orderLoc = sceneLoc;
			break;
		case ORDER_FollowMe:			// Target atom is the atom who issued the order.
		case ORDER_MountMe:				// Target atom is the atom who issued the order.
			orderAtom = SceneAtomId();
			orderLoc = AuPoint(0,0);
			break;
		}
	}
	UnlockSceneAtom();

	AuIdList ordersTo;
	for(AuSelectedUnitArray::const_iterator ti = m_Selected.constBegin(); ti != m_Selected.constEnd(); ti++)
		ordersTo.append((*ti).Id());
	if(!ordersTo.isEmpty())
	{
		// The channel will adjust the range in my message to the channel range setting.
		AuClientFrame* pFrame = auClientApp->ClientFrame();
		AuCommCitizenOrder msg(ordersTo, order, orderAtom, orderLoc.x, orderLoc.y, orderLoc.z, orderText,
				pFrame->OrdersChannel(), m_idSceneAtom, SceneAtomName(), SceneSector(), SceneGalaxy(), AuCommStdChannel::Universe,
				AuCommStdChannel::Scope(pFrame->OrdersChannel()));
		pFrame->SendChat(msg);
	}
}

void AuClientView::SquadIssuePosture(CITIZEN_POSTURE posture)
{
	AuIdList ordersTo;
	for(AuSelectedUnitArray::const_iterator ti = m_Selected.constBegin(); ti != m_Selected.constEnd(); ti++)
		ordersTo.append((*ti).Id());
	if(!ordersTo.isEmpty())
	{
		// The channel will adjust the range in my message to the channel range setting.
		AuClientFrame* pFrame = auClientApp->ClientFrame();
		AuCommCitizenPosture msg(ordersTo, posture,
				pFrame->OrdersChannel(), m_idSceneAtom, SceneAtomName(), SceneSector(), SceneGalaxy(), AuCommStdChannel::Universe,
				AuCommStdChannel::Scope(pFrame->OrdersChannel()));
		pFrame->SendChat(msg);
	}
}

void AuClientView::LocalLogMessage(const QString& text)
{
	// NOTE: ChatReceived locks the scene atom only when the message type is AuCommMessage::ChatAudio.
	AuCommThought msg(text, m_idSceneAtom, SceneAtomName());

	// Send the message to the frame so it ends up in the comm Thoughts channel.
	AuClientFrame* pFrame = auClientApp->ClientFrame();
	pFrame->ChatReceived(msg);
}

// Saves last known whereabouts string for current scene atom before exiting.
void AuClientView::SaveWhereabouts()
{
	if(m_idSceneAtom.IsNull())
		return;

	// Do not save whereabouts in single-user designer mode.
	if(auApp->IsSingleUserDesignerMode())
		return;

	QString worldName, slonglat, scity, sdev, shipname, shipinfo, sysNameDesc, secDesc, secName, oat, wx, wind;
	QString sysPos, secPos, galPos, galName;
	INDUSTRYID iid;
	SPACECRAFT_STRUCTURE shipStruct;
	BODY_TYPE bodyType;
	GetLocationInfo(worldName, bodyType, slonglat, scity, sdev, iid, shipname, shipinfo, shipStruct, sysNameDesc, secDesc, secName,
			oat, wx, wind, sysPos, secPos, galPos, galName);

	QString whereabouts;

	// Ship Info
	if(!shipname.isEmpty())
	{
		whereabouts += "\n" + shipname;
		if(!shipinfo.isEmpty())
			whereabouts += " " + shipinfo;
	}

	// City with development name.
	if(!scity.isEmpty())
	{
		whereabouts += "\n" + scity;
		if(!sdev.isEmpty())
			whereabouts += " " + sdev;
	}

	// Planet with latitude/longitude location.
	if(!worldName.isEmpty())
	{
		whereabouts += "\n" + worldName;
		if(!slonglat.isEmpty())
			whereabouts += " " + slonglat;
	}

	// System with sector coordinates.
	if(!sysNameDesc.isEmpty())
		whereabouts += "\n" + sysNameDesc;

	// Sector with coordinates.
	if(!secName.isEmpty())
		whereabouts += "\n" + secName + " " + secDesc;

	if(!galName.isEmpty())
		whereabouts += "\n" + galName;

	// Coordinates
	if(!sysPos.isEmpty() || !secPos.isEmpty() || !galPos.isEmpty())
	{
		whereabouts += tr("\nCoordinates");
		if(!sysPos.isEmpty())
			whereabouts += "\n  (" + sysPos + ")au in Solar System";

		if(!secPos.isEmpty())
			whereabouts += "\n  (" + secPos + ")pc in Sector";

		if(!galPos.isEmpty())
			whereabouts += "\n  (" + galPos + ")pc in Galaxy";
	}

	// Drop the leading newline.
	whereabouts = whereabouts.mid(1);

	QString section = QString("/Whereabouts/%1").arg(m_idSceneAtom.StringId());
	g_Au.Settings().writeEntry(section, whereabouts);
}

void AuClientView::PageAgent()
{
	AuClientFrame* pFrame = auClientApp->ClientFrame();
	if(!pFrame)
		return;

	// The idly curious often press this button to see what it does. This also makes it tougher to spam the help message.
	QString smsg = tr("<html><b>Help!</b><br><br>A message will be broadcast at universe range on the Help channel. "
			"The Help channel appears on the Comm window.<br><br>"
			"An agent will respond if one is available.</html>");
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Information, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, auApp->Frame());
	if(pFrame->ExecMessageBox(mb) == QMessageBox::Ok)
	{
		AuCommRequestHelp msg(AuCommStdChannel::HelpChannelName(), m_idSceneAtom, SceneAtomName(), SceneSector(), SceneGalaxy(),
				AuCommStdChannel::Universe);
		pFrame->SendChat(msg);
	}
}

void AuClientView::ActionClicked(QAction* action)
{
	ViewButtonClicked((AuViewButton::VIEWBUTTON)action->data().toInt());
}

void AuClientView::ViewIconButtonClicked(int value)
{
	//qDebug() << "AuClientView::ViewIconButtonClicked" << value;
	ViewButtonClicked((AuViewButton::VIEWBUTTON)value);
}

void AuClientView::ViewIconButtonExpanded(int value)
{
	//qDebug() << "AuClientView::ViewIconButtonExpanded" << value;
	QMenu menu;
	switch((AuViewButton::VIEWBUTTON)value)
	{
	case AuViewButton::VB_Invalid:
	case AuViewButton::VB_Agent:
	case AuViewButton::VB_Architect:
	case AuViewButton::VB_Avatar1:
	case AuViewButton::VB_Avatar2:
	case AuViewButton::VB_Avatar3:
	case AuViewButton::VB_Avatar4:
	case AuViewButton::VB_Avatar5:
	case AuViewButton::VB_Avatar6:
	case AuViewButton::VB_Avatars:
	case AuViewButton::VB_BdDesignExchange:
	case AuViewButton::VB_CargoHold:
	case AuViewButton::VB_CargoManifest:
	case AuViewButton::VB_CargoOrdnance:
	case AuViewButton::VB_CargoTransfer:
	case AuViewButton::VB_Comm:
	case AuViewButton::VB_Exit:
	case AuViewButton::VB_Forum:
	case AuViewButton::VB_FullScreen:
	case AuViewButton::VB_GearHandCraft:
	case AuViewButton::VB_GovernAnnouncements:
	case AuViewButton::VB_GovernCities:
	case AuViewButton::VB_GovernCitizens:
	case AuViewButton::VB_GovernCommerce:
	case AuViewButton::VB_GovernDiplomacy:
	case AuViewButton::VB_GovernPatents:
	case AuViewButton::VB_GovernPolicy:
	case AuViewButton::VB_GovernTaxes:
	case AuViewButton::VB_HazeronHome:
	case AuViewButton::VB_Help:
	case AuViewButton::VB_HideWindows:
	case AuViewButton::VB_LocatorCompass:
	case AuViewButton::VB_LocatorGPS:
	case AuViewButton::VB_LocatorStarMap:
	case AuViewButton::VB_LocatorSystemScan:
	case AuViewButton::VB_LocatorWorldMap:
	case AuViewButton::VB_Mail:
	case AuViewButton::VB_Minion:
	case AuViewButton::VB_PlayersOnline:
	case AuViewButton::VB_Programmer:
	case AuViewButton::VB_Recall:
	case AuViewButton::VB_Settings:
	case AuViewButton::VB_SpacecraftAccount:
	case AuViewButton::VB_SpacecraftBounties:
	case AuViewButton::VB_SpacecraftDamage:
	case AuViewButton::VB_SpacecraftDesignExchange:
	case AuViewButton::VB_SpacecraftMission:
	case AuViewButton::VB_SpacecraftPolicy:
	case AuViewButton::VB_SpacecraftRoster:
	case AuViewButton::VB_SpacecraftShipments:
	case AuViewButton::VB_UNUSED1:
	case AuViewButton::VB_UNUSED2:
	case AuViewButton::VB_UNUSED3:
	case AuViewButton::VB_UNUSED4:
	case AuViewButton::VB_UNUSED5:
	case AuViewButton::VB_UNUSED6:
	case AuViewButton::VB_UnitOrders:
	case AuViewButton::VB_Videos:
	case AuViewButton::VB_WarDimensionStandings:
	case AuViewButton::VB_Wiki:
	case AuViewButton::VB_COUNT:
		break;

	case AuViewButton::VB_Cargo:
		{
			QMenu menu(this);
			QAction* a;

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_CargoManifest)),
					AuViewButton::Name(AuViewButton::VB_CargoManifest));
			a->setData(AuViewButton::VB_CargoManifest);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_CargoHold)),
					AuViewButton::Name(AuViewButton::VB_CargoHold));
			a->setData(AuViewButton::VB_CargoHold);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_CargoTransfer)),
					AuViewButton::Name(AuViewButton::VB_CargoTransfer));
			a->setData(AuViewButton::VB_CargoTransfer);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_CargoOrdnance)),
					AuViewButton::Name(AuViewButton::VB_CargoOrdnance));
			a->setData(AuViewButton::VB_CargoOrdnance);

			QAction* ch = menu.exec(QCursor::pos());
			if(ch)
				ActionClicked(ch);
		}
		break;

	case AuViewButton::VB_Gear:
		{
			QMenu menu(this);
			QAction* a;

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_Gear)),
					AuViewButton::Name(AuViewButton::VB_Gear));
			a->setData(AuViewButton::VB_Gear);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_GearHandCraft)),
					AuViewButton::Name(AuViewButton::VB_GearHandCraft));
			a->setData(AuViewButton::VB_GearHandCraft);

			QAction* ch = menu.exec(QCursor::pos());
			if(ch)
				ActionClicked(ch);
		}
		break;

	case AuViewButton::VB_Government:
		{
			QMenu menu(this);
			QAction* a;

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_GovernAnnouncements)),
					AuViewButton::Name(AuViewButton::VB_GovernAnnouncements));
			a->setData(AuViewButton::VB_GovernAnnouncements);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_GovernPolicy)),
					AuViewButton::Name(AuViewButton::VB_GovernPolicy));
			a->setData(AuViewButton::VB_GovernPolicy);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_GovernDiplomacy)),
					AuViewButton::Name(AuViewButton::VB_GovernDiplomacy));
			a->setData(AuViewButton::VB_GovernDiplomacy);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_GovernCommerce)),
					AuViewButton::Name(AuViewButton::VB_GovernCommerce));
			a->setData(AuViewButton::VB_GovernCommerce);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_GovernPatents)),
					AuViewButton::Name(AuViewButton::VB_GovernPatents));
			a->setData(AuViewButton::VB_GovernPatents);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_GovernCitizens)),
					AuViewButton::Name(AuViewButton::VB_GovernCitizens));
			a->setData(AuViewButton::VB_GovernCitizens);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_GovernCities)),
					AuViewButton::Name(AuViewButton::VB_GovernCities));
			a->setData(AuViewButton::VB_GovernCities);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_GovernTaxes)),
					AuViewButton::Name(AuViewButton::VB_GovernTaxes));
			a->setData(AuViewButton::VB_GovernTaxes);

			QAction* ch = menu.exec(QCursor::pos());
			if(ch)
				ActionClicked(ch);
		}
		break;

	case AuViewButton::VB_Locator:
		{
			QMenu menu(this);
			QAction* a;

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_LocatorCompass)),
					AuViewButton::Name(AuViewButton::VB_LocatorCompass));
			a->setData(AuViewButton::VB_LocatorCompass);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_LocatorWorldMap)),
					AuViewButton::Name(AuViewButton::VB_LocatorWorldMap));
			a->setData(AuViewButton::VB_LocatorWorldMap);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_LocatorSystemScan)),
					AuViewButton::Name(AuViewButton::VB_LocatorSystemScan));
			a->setData(AuViewButton::VB_LocatorSystemScan);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_LocatorStarMap)),
					AuViewButton::Name(AuViewButton::VB_LocatorStarMap));
			a->setData(AuViewButton::VB_LocatorStarMap);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_LocatorGPS)),
					AuViewButton::Name(AuViewButton::VB_LocatorGPS));
			a->setData(AuViewButton::VB_LocatorGPS);

			QAction* ch = menu.exec(QCursor::pos());
			if(ch)
				ActionClicked(ch);
		}
		break;

	case AuViewButton::VB_Spacecraft:
		{
			QMenu menu(this);
			QAction* a;

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_SpacecraftMission)),
					AuViewButton::Name(AuViewButton::VB_SpacecraftMission));
			a->setData(AuViewButton::VB_SpacecraftMission);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_SpacecraftPolicy)),
					AuViewButton::Name(AuViewButton::VB_SpacecraftPolicy));
			a->setData(AuViewButton::VB_SpacecraftPolicy);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_SpacecraftRoster)),
					AuViewButton::Name(AuViewButton::VB_SpacecraftRoster));
			a->setData(AuViewButton::VB_SpacecraftRoster);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_SpacecraftAccount)),
					AuViewButton::Name(AuViewButton::VB_SpacecraftAccount));
			a->setData(AuViewButton::VB_SpacecraftAccount);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_SpacecraftBounties)),
					AuViewButton::Name(AuViewButton::VB_SpacecraftBounties));
			a->setData(AuViewButton::VB_SpacecraftBounties);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_SpacecraftDamage)),
					AuViewButton::Name(AuViewButton::VB_SpacecraftDamage));
			a->setData(AuViewButton::VB_SpacecraftDamage);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_SpacecraftShipments)),
					AuViewButton::Name(AuViewButton::VB_SpacecraftShipments));
			a->setData(AuViewButton::VB_SpacecraftShipments);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_SpacecraftDesignExchange)),
					AuViewButton::Name(AuViewButton::VB_SpacecraftDesignExchange));
			a->setData(AuViewButton::VB_SpacecraftDesignExchange);

			QAction* ch = menu.exec(QCursor::pos());
			if(ch)
				ActionClicked(ch);
		}
		break;

	case AuViewButton::VB_Building:
		{
			QMenu menu(this);
			QAction* a;

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_Building)),
					AuViewButton::Name(AuViewButton::VB_Building));
			a->setData(AuViewButton::VB_Building);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_BdDesignExchange)),
					AuViewButton::Name(AuViewButton::VB_BdDesignExchange));
			a->setData(AuViewButton::VB_BdDesignExchange);

			QAction* ch = menu.exec(QCursor::pos());
			if(ch)
				ActionClicked(ch);
		}
		break;

	case AuViewButton::VB_Bio:
		{
			bool requestAvatars = false;
			QMenu menu(this);

			const DgData& gd = g_Au.LockDataForRead();
			AuId curId = gd.ClientUnitId();
			AuId avId;
			avId = gd.ClientUnitId(0);
			if(!avId.IsNull())
			{
				QString avName = gd.ClientUnitName(avId);
				if(avName.isEmpty())
					requestAvatars = true;
				else
				{
					avName += QString(" (%1)").arg(avId.StringId());
					QAction* a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_Avatar1)), avName);
					if(curId == avId)
						a->setEnabled(false);
					a->setData(AuViewButton::VB_Avatar1);
				}
			}

			avId = gd.ClientUnitId(1);
			if(!avId.IsNull())
			{
				QString avName = gd.ClientUnitName(avId);
				if(avName.isEmpty())
					requestAvatars = true;
				else
				{
					avName += QString(" (%1)").arg(avId.StringId());
					QAction* a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_Avatar2)), avName);
					if(curId == avId)
						a->setEnabled(false);
					a->setData(AuViewButton::VB_Avatar2);
				}
			}

			avId = gd.ClientUnitId(2);
			if(!avId.IsNull())
			{
				QString avName = gd.ClientUnitName(avId);
				if(avName.isEmpty())
					requestAvatars = true;
				else
				{
					avName += QString(" (%1)").arg(avId.StringId());
					QAction* a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_Avatar3)), avName);
					if(curId == avId)
						a->setEnabled(false);
					a->setData(AuViewButton::VB_Avatar3);
				}
			}

			avId = gd.ClientUnitId(3);
			if(!avId.IsNull())
			{
				QString avName = gd.ClientUnitName(avId);
				if(avName.isEmpty())
					requestAvatars = true;
				else
				{
					avName += QString(" (%1)").arg(avId.StringId());
					QAction* a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_Avatar4)), avName);
					if(curId == avId)
						a->setEnabled(false);
					a->setData(AuViewButton::VB_Avatar4);
				}
			}

			avId = gd.ClientUnitId(4);
			if(!avId.IsNull())
			{
				QString avName = gd.ClientUnitName(avId);
				if(avName.isEmpty())
					requestAvatars = true;
				else
				{
					avName += QString(" (%1)").arg(avId.StringId());
					QAction* a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_Avatar5)), avName);
					if(curId == avId)
						a->setEnabled(false);
					a->setData(AuViewButton::VB_Avatar5);
				}
			}

			avId = gd.ClientUnitId(5);
			if(!avId.IsNull())
			{
				QString avName = gd.ClientUnitName(avId);
				if(avName.isEmpty())
					requestAvatars = true;
				else
				{
					avName += QString(" (%1)").arg(avId.StringId());
					QAction* a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_Avatar6)), avName);
					if(curId == gd.ClientUnitId(5))
						a->setEnabled(false);
					a->setData(AuViewButton::VB_Avatar6);
				}
			}
			g_Au.UnlockData();

			if(requestAvatars)
				auApp->RequestAvatars();

			QAction* ch = menu.exec(QCursor::pos());
			if(ch)
				ActionClicked(ch);
		}
		break;

	case AuViewButton::VB_About:
		{
			QMenu menu(this);
			QAction* a;

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_HazeronHome)),
					AuViewButton::Name(AuViewButton::VB_HazeronHome));
			a->setData(AuViewButton::VB_HazeronHome);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_Wiki)),
					AuViewButton::Name(AuViewButton::VB_Wiki));
			a->setData(AuViewButton::VB_Wiki);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_Forum)),
					AuViewButton::Name(AuViewButton::VB_Forum));
			a->setData(AuViewButton::VB_Forum);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_Videos)),
					AuViewButton::Name(AuViewButton::VB_Videos));
			a->setData(AuViewButton::VB_Videos);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_Avatars)),
					AuViewButton::Name(AuViewButton::VB_Avatars));
			a->setData(AuViewButton::VB_Avatars);

			a = menu.addAction(QIcon(AuViewButton::Image(AuViewButton::VB_WarDimensionStandings)),
					AuViewButton::Name(AuViewButton::VB_WarDimensionStandings));
			a->setData(AuViewButton::VB_WarDimensionStandings);

			QAction* ch = menu.exec(QCursor::pos());
			if(ch)
				ActionClicked(ch);
		}
		break;
	}
}

void AuClientView::ViewButtonClicked(AuViewButton::VIEWBUTTON button)
{
	//qDebug() << "AuClientView::ViewButtonClicked" << AuViewButton::Name(button);
	AuClientFrame* pFrame = auClientApp->ClientFrame();

	QAction a(this);
	switch(button)
	{
#if defined(QT_NO_DEBUG)
	default:	// Omitted in debug build so I get a warning when an enum is added. Default case needed for safety in release.
#endif
	case AuViewButton::VB_Invalid:
	case AuViewButton::VB_COUNT:
	case AuViewButton::VB_UNUSED1:
	case AuViewButton::VB_UNUSED2:
	case AuViewButton::VB_UNUSED3:
	case AuViewButton::VB_UNUSED4:
	case AuViewButton::VB_UNUSED5:
	case AuViewButton::VB_UNUSED6:
		return;
	case AuViewButton::VB_Agent:
	case AuViewButton::VB_Architect:
	case AuViewButton::VB_Programmer:
	case AuViewButton::VB_Minion:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleAgentUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Avatar1:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(SwitchToAvatar1()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Avatar2:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(SwitchToAvatar2()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Avatar3:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(SwitchToAvatar3()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Avatar4:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(SwitchToAvatar4()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Avatar5:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(SwitchToAvatar5()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Avatar6:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(SwitchToAvatar6()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Bio:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleCharacterUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Building:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleBuildingUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_PlayersOnline:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(TogglePlayersOnlineUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Cargo:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleHoldUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_CargoHold:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleCargoHoldUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_CargoManifest:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleCargoManifestUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_CargoOrdnance:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleCargoOrdnanceUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_CargoTransfer:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleCargoTransferUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Comm:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleCommUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Exit:
		connect(&a, SIGNAL(triggered()), auApp, SLOT(closeAllWindows()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_FullScreen:
		connect(&a, SIGNAL(triggered()), SLOT(ToggleFullScreen()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Government:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleGovernUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_GovernAnnouncements:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleGovernAnnouncementsUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_GovernCities:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleGovernCitiesUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_GovernCitizens:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleGovernCitizensUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_GovernCommerce:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleGovernCommerceUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_GovernDiplomacy:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleGovernDiplomacyUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_GovernPatents:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleGovernPatentsUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_GovernPolicy:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleGovernPolicyUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_GovernTaxes:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleGovernTaxesUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Help:
		connect(&a, SIGNAL(triggered()), this, SLOT(PageAgent()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_HideWindows:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleAllUis()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Gear:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleGearUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_GearHandCraft:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleHandCraftUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Locator:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleLocatorUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_LocatorCompass:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleLocatorCompassUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_LocatorGPS:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleLocatorGPSUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_LocatorStarMap:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleLocatorStarMapUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_LocatorSystemScan:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleLocatorSystemScanUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_LocatorWorldMap:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleLocatorWorldMapUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Mail:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleMailUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Recall:
		connect(&a, SIGNAL(triggered()), this, SLOT(SpawnRecall()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Settings:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleSettingsUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Spacecraft:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleSpacecraftUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_SpacecraftAccount:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleSpacecraftAccountUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_SpacecraftBounties:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleSpacecraftBountiesUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_SpacecraftDamage:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleSpacecraftDamageUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_BdDesignExchange:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleBdExchangeUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_SpacecraftDesignExchange:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleSpacecraftDesignExchangeUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_SpacecraftMission:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleSpacecraftMissionUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_SpacecraftPolicy:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleSpacecraftPolicyUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_SpacecraftRoster:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleSpacecraftRosterUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_SpacecraftShipments:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleSpacecraftShipmentsUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_UnitOrders:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleOrdersUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_About:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(HelpAbout()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Forum:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ShowForum()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_HazeronHome:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ShowHomePage()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Wiki:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ShowWiki()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Videos:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ShowVideos()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_WarDimensionStandings:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ShowWarDimensionStandings()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Avatars:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ShowAvatars()), Qt::QueuedConnection);
		break;
	}
	a.trigger();
}

void AuClientView::ToggleFullScreen()
{
	emit ToggleScreenReady();
}

void AuClientView::AvatarOnLine(AuId id)
{
	// Check to see if the avatar is on the player's buddy list.
	const DgData& gd = g_Au.LockDataForRead();
	bool isBuddy=false;
	for(QList<AuContact>::const_iterator it = gd.GetContacts().begin(); it != gd.GetContacts().end(); it++)
	{
		if((*it).m_id != id)
			continue;
		if((*it).m_bIgnore)
			continue;					// This one is squelched, not exactly a buddy.
		isBuddy = true;
		break;
	}
	g_Au.UnlockData();
	if(!isBuddy)
		return;
	auApp->PlayClientSound(SOUND_BuddyOnLine);
}

void AuClientView::SyncScene()
{
	QList<AuSyncItem> sync;
	g_AuClient.TakeSyncQueue(sync);
	for(QList<AuSyncItem>::iterator si = sync.begin(); si != sync.end(); si++)
	{
		switch((*si).m_Type)
		{
		case AuSyncItem::AtomAdded:
			AtomAdded((*si).m_Atom);	// takes ownership of the atom.
			break;
		case AuSyncItem::AtomChanged:
			AtomChanged((*si).m_Atom);
			delete (*si).m_Atom;
			break;
		case AuSyncItem::AtomDeparted:
			AtomDeparted((*si).m_AtomId);
			break;
		case AuSyncItem::AtomParentChanged:
			AtomParentChanged((*si).m_Atom);
			AtomChanged((*si).m_Atom);
			delete (*si).m_Atom;
			break;
		case AuSyncItem::AtomsCleared:
			AtomsCleared();
			break;
		}
	}
}

void AuClientView::SetSelectedBd(const AuId& idBd)
{
	m_RenderInfo.SetSelectedBd(idBd);
}

void AuClientView::SetNumberOfMessages(int iNumber)
{
	Q_UNUSED(iNumber);
}

QImage AuClientView::SceneAtomImage() const
{
	return QImage();
}

