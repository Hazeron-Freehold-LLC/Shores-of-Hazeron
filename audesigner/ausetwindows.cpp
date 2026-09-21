//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetwindows.cpp,v 1.15 2022/02/19 18:26:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausetwindows.h"
//
#include "auapp.h"
#include "aucheckbox.h"
#include "auclientglobal.h"
#include "audockwidget.h"
#include "auframe.h"
#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QToolBar>

// These are the names of the dock bars as they will be listed for docking settings. They don't have to match 1:1 with the widgets.
static QString s_DockBarNames[] =
{
	QObject::tr("Avatars Online"),
	QObject::tr("Bio"),
	QObject::tr("Blueprint Exchange"),
	QObject::tr("Building Design"),
	QObject::tr("Cargo"),
	QObject::tr("Cargo Hold"),
	QObject::tr("Cargo Jettison"),
	QObject::tr("Cargo Ordnance"),
	QObject::tr("Cargo Transfer"),
	QObject::tr("City"),
	QObject::tr("Comm"),
	QObject::tr("Construction"),
	QObject::tr("Designer Bar"),
	QObject::tr("Designer Helper Bar"),
	QObject::tr("Designer Parts"),
	QObject::tr("Designer Texture Wrapper"),
	QObject::tr("Governance"),
	QObject::tr("Govern Announcements"),
	QObject::tr("Govern Cities"),
	QObject::tr("Govern Citizens"),
	QObject::tr("Govern Commerce"),
	QObject::tr("Govern Diplomacy"),
	QObject::tr("Govern Policy"),
	QObject::tr("Govern Taxes"),
	QObject::tr("Gear"),
	QObject::tr("Handcraft"),
	QObject::tr("Locator"),
	QObject::tr("Locator Compass"),
	QObject::tr("Locator GPS"),
	//QObject::tr("Locator Star Map"),
	QObject::tr("Locator System Map"),
	QObject::tr("Locator World Map"),
	QObject::tr("Mail"),
	QObject::tr("Spacecraft"),
	QObject::tr("Spacecraft Account"),
	QObject::tr("Spacecraft Bounties"),
	QObject::tr("Spacecraft Damage"),
	QObject::tr("Spacecraft Design"),
	QObject::tr("Spacecraft Specifications"),
	QObject::tr("Spacecraft Mission"),
	QObject::tr("Spacecraft Policy"),
	QObject::tr("Spacecraft Roster"),
	QObject::tr("Unit Orders"),
};
static int s_DockBarCount = sizeof(s_DockBarNames)/sizeof(QString);

// The object names corresponding to the s_DockBarNames.
// The object names are constructed by adding 'Dock' to the windowTitle parameter in the AuDockWidget constructor.
static QString s_DockBarObjectNames[] =
{
	QString("AvatarsOnlineDock"),
	QString("BioDock"),
	QString("Blueprint ExchangeDock"),
	QString("Building DesignDock"),
	QString("CargoDock"),
	QString("HoldDock"),
	QString("JettisonDock"),
	QString("OrdnanceDock"),
	QString("TransferDock"),
	QString("CityDock"),
	QString("CommDock"),
	QString("ConstructDock"),
	QString("DesignerBar"),
	QString("DesignerHelperBar"),
	QString("PartsDock"),
	QString("Texture WrapperDock"),
	QString("GovernDock"),
	QString("Govern AnnouncementsDock"),
	QString("Govern CitiesDock"),
	QString("Govern CitizensDock"),
	QString("Govern CommerceDock"),
	QString("Govern DiplomacyDock"),
	QString("Govern PolicyDock"),
	QString("Govern TaxesDock"),
	QString("GearDock"),
	QString("HandcraftDock"),
	QString("LocatorDock"),
	QString("CompassDock"),
	QString("GPSDock"),
	//QString("Star MapDock"),
	QString("System MapDock"),
	QString("World MapDock"),
	QString("MailDock"),
	QString("SpacecraftDock"),
	QString("AccountDock"),
	QString("BountiesDock"),
	QString("DamageDock"),
	QString("Ship DesignDock"),
	QString("SpecsDock"),
	QString("MissionDock"),
	QString("PolicyDock"),
	QString("RosterDock"),
	QString("UnitsDock"),
};
#if !defined(QT_NO_DEBUG)
static int s_DockBarObjectCount = sizeof(s_DockBarObjectNames)/sizeof(QString);
#endif

// The name of the allowedAreas setting location. Must match the .h files of these dock bars.
static QString s_DockBarSettings[] =
{
	QString("AvatarsOnlineAreas"),
	QString("Dock/BioAreas"),
	QString("Dock/SpacecraftDesignExchangeAreas"),
	QString("Dock/BuildingDesignAreas"),
	QString("Dock/CargoAreas"),
	QString("Dock/CargoHoldAreas"),
	QString("Dock/CargoJettisonAreas"),
	QString("Dock/CargoOrdnanceAreas"),
	QString("Dock/CargoTransferAreas"),
	QString("Dock/CityAreas"),
	QString("Dock/CommAreas"),
	QString("Dock/ConstructionAreas"),
	QString("Dock/DesignerBarAreas"),
	QString("Dock/DesignerHelperBarAreas"),
	QString("Dock/DesignerPartsAreas"),
	QString("Dock/DesignerTextureWrapAreas"),
	QString("Dock/GovernanceAreas"),
	QString("Dock/GovernAnnouncementsAreas"),
	QString("Dock/GovernCitiesAreas"),
	QString("Dock/GovernCitizensAreas"),
	QString("Dock/GovernCommerceAreas"),
	QString("Dock/GovernDiplomacyAreas"),
	QString("Dock/GovernPolicyAreas"),
	QString("Dock/GovernTaxesAreas"),
	QString("Dock/InventoryAreas"),
	QString("Dock/HandcraftAreas"),
	QString("Dock/LocatorAreas"),
	QString("Dock/LocatorCompassAreas"),
	QString("Dock/LocatorGPSAreas"),
	//QString("Dock/LocatorStarMapAreas"),
	QString("Dock/LocatorSystemMapAreas"),
	QString("Dock/LocatorWorldMapAreas"),
	QString("Dock/MailAreas"),
	QString("Dock/SpacecraftAreas"),
	QString("Dock/SpacecraftAccountAreas"),
	QString("Dock/SpacecraftBountiesAreas"),
	QString("Dock/SpacecraftDamageAreas"),
	QString("Dock/SpacecraftDesignAreas"),
	QString("Dock/SpacecraftDesignSpecificationsAreas"),
	QString("Dock/SpacecraftMissionAreas"),
	QString("Dock/SpacecraftPolicyAreas"),
	QString("Dock/SpacecraftRosterAreas"),
	QString("Dock/UnitOrdersAreas"),
};
#if !defined(QT_NO_DEBUG)
static int s_DockBarSettingsCount = sizeof(s_DockBarSettings)/sizeof(QString);
#endif

AuSetWindows::AuSetWindows(QWidget *parent) : QWidget(parent)
{
#if !defined(QT_NO_DEBUG)
	Q_ASSERT(s_DockBarCount == s_DockBarObjectCount);	// Don't let these get out of sync.
	Q_ASSERT(s_DockBarCount == s_DockBarSettingsCount);	// Don't let these get out of sync.
#endif
	QCheckBox* cknativecolor = new QCheckBox(tr("Use Native Color Dialog"), this);
	cknativecolor->setChecked(g_Au.UseNativeColorDialog());
	cknativecolor->setToolTip(tr("<html>"
				"Chooses which dialog to use when a color selection is requested from the user. "
				"<br>"
				"%1 When checked, the dialog provided by the operating system is used. "
				"<b>On some systems, the system dialog does not work after %2 initializes the OpenGL scene.</b> "
				"<br>"
				"%1 When unchecked, the dialog provided by Qt is used. "
				"</html>").arg(g_Au.MiddleDotSymbol()).arg(g_Au.ApplicationName()));
	connect(cknativecolor, SIGNAL(stateChanged(int)), SLOT(UseNativeColorChanged(int)));

	QPushButton* pbcolor = new QPushButton(tr("Test Color Dialog"), this);
	connect(pbcolor, SIGNAL(clicked()), SLOT(TestColorDialog()));

	QCheckBox* cknativefile = new QCheckBox(tr("Use Native File Dialog"), this);
	cknativefile->setChecked(g_Au.UseNativeFileDialog());
	cknativefile->setToolTip(tr("<html>"
				"Chooses which dialog to use when a file or folder selection is requested from the user. "
				"<br>"
				"%1 When checked, the dialog provided by the operating system is used. "
				"<b>On some systems, the system dialog does not work after %2 initializes the OpenGL scene.</b> "
				"<br>"
				"%1 When unchecked, the dialog provided by Qt is used. "
				"</html>").arg(g_Au.MiddleDotSymbol()).arg(g_Au.ApplicationName()));
	connect(cknativefile, SIGNAL(stateChanged(int)), SLOT(UseNativeFileChanged(int)));

	QPushButton* pbfile = new QPushButton(tr("Test File Dialog"), this);
	connect(pbfile, SIGNAL(clicked()), SLOT(TestFileDialog()));

	QCheckBox* cknativefont = new QCheckBox(tr("Use Native Font Dialog"), this);
	cknativefont->setChecked(g_Au.UseNativeFontDialog());
	cknativefont->setToolTip(tr("<html>"
				"Chooses which dialog to use when a font selection is requested from the user. "
				"<br>"
				"%1 When checked, the dialog provided by the operating system is used. "
				"<b>On some systems, the system dialog does not work after %2 initializes the OpenGL scene.</b> "
				"<br>"
				"%1 When unchecked, the dialog provided by Qt is used. "
				"</html>").arg(g_Au.MiddleDotSymbol()).arg(g_Au.ApplicationName()));
	connect(cknativefont, SIGNAL(stateChanged(int)), SLOT(UseNativeFontChanged(int)));

	QPushButton* pbfont = new QPushButton(tr("Test Font Dialog"), this);
	connect(pbfont, SIGNAL(clicked()), SLOT(TestFontDialog()));

	QCheckBox* ckstorydialog = new QCheckBox(tr("Use Story Dialog"), this);
	ckstorydialog->setChecked(g_AuClient.ShowStoryDialog());
	ckstorydialog->setToolTip(tr("<html>"
				"Enables the story dialog to show communications from stories and story characters. "
				"This helps readability when a story sends large amounts of text. "
				"<br>"
				"%1 When checked, story communications appear in a dialog window and they appear as comm messages. "
				"<br>"
				"%1 When unchecked, story communications appear only as comm messages; a dialog window is not shown."
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(ckstorydialog, SIGNAL(stateChanged(int)), SLOT(UseStoryDialogChanged(int)));

	QPushButton* pbresetw = new QPushButton("Reset Saved Window Positions");
	pbresetw->setToolTip(tr("Resets the positions of all windows. The last visible size and\n"
				"position of all windows is cleared; they will appear at their\n"
				"default positions and sizes when they are next displayed. This\n"
				"does not affect windows that are visible right now because they\n"
				"will save their current position and size when they are closed."));
	connect(pbresetw, SIGNAL(clicked()), SLOT(ResetWidgets()));

	bool bAllowTransparency = true;	// Allow by default
#ifdef Q_OS_WIN
		OSVERSIONINFO osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);
		if(osvi.dwMajorVersion < 6)
			bAllowTransparency = false;	// OS is NOT Windows Vista or later
#endif
	QLabel *lbactive, *lbless0, *lbmore0, *lbinactive, *lbless1, *lbmore1;
	QSlider *slactive, *slinactive;
	if(bAllowTransparency)
	{
		// Transparency of active window
		lbactive = new QLabel(tr("<b>Transparency of Active Window</b>"), this);
		lbactive->setAlignment(Qt::AlignCenter);

		slactive = new QSlider(Qt::Horizontal, this);
		slactive->setRange(0, 95);
		slactive->setSingleStep(5);
		slactive->setPageStep(5);
		int actTrans = 100 - g_Au.WindowOpacity(true);
		slactive->setValue(actTrans);
		connect(slactive, SIGNAL(valueChanged(int)), SLOT(SetActiveTrans(int)));

		lbless0 = new QLabel(tr("Less Transparent"), this);

		lbmore0 = new QLabel(tr("More Transparent"), this);
		lbmore0->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		// Transparency of inactive windows

		lbinactive = new QLabel(tr("<b>Transparency of Inactive Windows</b>"), this);
		lbinactive->setAlignment(Qt::AlignCenter);

		slinactive = new QSlider(Qt::Horizontal, this);
		slinactive->setRange(0, 95);
		slinactive->setSingleStep(5);
		slinactive->setPageStep(5);
		int inactTrans = 100 - g_Au.WindowOpacity(false);
		slinactive->setValue(inactTrans);
		connect(slinactive, SIGNAL(valueChanged(int)), SLOT(SetInactiveTrans(int)));

		lbless1 = new QLabel(tr("Less Transparent"), this);

		lbmore1 = new QLabel(tr("More Transparent"), this);
		lbmore1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	}
	else
	{
		// Transparency not allowed for this OS.
		SetActiveTrans(0);
		SetInactiveTrans(0);
	}

	// Dockbars
	QScrollArea* dockbarScroller = new QScrollArea;
	{
		QLabel* lbdock = new QLabel(tr("<html><b>Dock Window</b></html>"));
		lbdock->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		//QLabel* lbvert1 = new QLabel(tr("<html><b>Vertical</b></html>"));
		//QLabel* lbvert2 = new QLabel(tr("<html><b>Title Bar</b></html>"));
		QLabel* lbdockside = new QLabel(tr("<html><b>Allow Dock At Sides</b></html>"));
		lbdockside->setAlignment(Qt::AlignCenter);
		QLabel* lbleft = new QLabel(tr("<html><b>Left</b></html>"));
		lbleft->setAlignment(Qt::AlignCenter);
		QLabel* lbright = new QLabel(tr("<html><b>Right</b></html>"));
		lbright->setAlignment(Qt::AlignCenter);
		QLabel* lbtop = new QLabel(tr("<html><b>Top</b></html>"));
		lbtop->setAlignment(Qt::AlignCenter);
		QLabel* lbbottom = new QLabel(tr("<html><b>Bottom</b></html>"));
		lbbottom->setAlignment(Qt::AlignCenter);

		QWidget* dockbars = new QWidget;
		QGridLayout* lay = new QGridLayout(dockbars);

		int row=0;
		//lay->addWidget(lbvert1, row,1, 1,2);
		lay->addWidget(lbdockside, row,3, 1,8);
		row++;
		lay->addWidget(lbdock, row,0);
		//lay->addWidget(lbvert2, row,1, 1,2);
		lay->addWidget(lbleft, row,3, 1,2);
		lay->addWidget(lbright, row,5, 1,2);
		lay->addWidget(lbtop, row,7, 1,2);
		lay->addWidget(lbbottom, row,9, 1,2);
		row++;
		for(int dockBar=0; dockBar<s_DockBarCount; dockBar++)
		{
			QLabel* lbdb = new QLabel(s_DockBarNames[dockBar]);
			lbdb->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

			//AuCheckBoxValue* ckdbtitle;
			AuCheckBoxValue* ckdbleft;
			AuCheckBoxValue* ckdbright;
			AuCheckBoxValue* ckdbtop;
			AuCheckBoxValue* ckdbbottom;

			if(s_DockBarObjectNames[dockBar] == "DesignerBar" || s_DockBarObjectNames[dockBar] == "DesignerHelperBar")
			{
				// QToolBars are handled differently than QDockWidgets.
				int dockSides = Qt::LeftToolBarArea|Qt::RightToolBarArea|Qt::TopToolBarArea|Qt::BottomToolBarArea;
				dockSides = g_Au.Settings().readNumEntry(s_DockBarSettings[dockBar], dockSides);

				//AuCheckBoxValue* ckdbtitle = new AuCheckBoxValue("", dockBar);
				ckdbleft = new AuCheckBoxValue("", dockBar);
				ckdbleft->setCheckState((dockSides & Qt::LeftToolBarArea) ? Qt::Checked : Qt::Unchecked);
				connect(ckdbleft, SIGNAL(StateChanged(int, int)), SLOT(ToolBarLeftChanged(int, int)));

				ckdbright = new AuCheckBoxValue("", dockBar);
				ckdbright->setCheckState((dockSides & Qt::RightToolBarArea) ? Qt::Checked : Qt::Unchecked);
				connect(ckdbright, SIGNAL(StateChanged(int, int)), SLOT(ToolBarRightChanged(int, int)));

				ckdbtop = new AuCheckBoxValue("", dockBar);
				ckdbtop->setCheckState((dockSides & Qt::TopToolBarArea) ? Qt::Checked : Qt::Unchecked);
				connect(ckdbtop, SIGNAL(StateChanged(int, int)), SLOT(ToolBarTopChanged(int, int)));

				ckdbbottom = new AuCheckBoxValue("", dockBar);
				ckdbbottom->setCheckState((dockSides & Qt::BottomToolBarArea) ? Qt::Checked : Qt::Unchecked);
				connect(ckdbbottom, SIGNAL(StateChanged(int, int)), SLOT(ToolBarBottomChanged(int, int)));
			}
			else
			{
				// This default value for dockSides came from the default constructor of AuDockWidget.
				int dockSides = Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea|Qt::BottomDockWidgetArea;
				dockSides = g_Au.Settings().readNumEntry(s_DockBarSettings[dockBar], dockSides);
		
				//AuCheckBoxValue* ckdbtitle = new AuCheckBoxValue("", dockBar);
				ckdbleft = new AuCheckBoxValue("", dockBar);
				ckdbleft->setCheckState((dockSides & Qt::LeftDockWidgetArea) ? Qt::Checked : Qt::Unchecked);
				connect(ckdbleft, SIGNAL(StateChanged(int, int)), SLOT(DockLeftChanged(int, int)));

				ckdbright = new AuCheckBoxValue("", dockBar);
				ckdbright->setCheckState((dockSides & Qt::RightDockWidgetArea) ? Qt::Checked : Qt::Unchecked);
				connect(ckdbright, SIGNAL(StateChanged(int, int)), SLOT(DockRightChanged(int, int)));

				ckdbtop = new AuCheckBoxValue("", dockBar);
				ckdbtop->setCheckState((dockSides & Qt::TopDockWidgetArea) ? Qt::Checked : Qt::Unchecked);
				connect(ckdbtop, SIGNAL(StateChanged(int, int)), SLOT(DockTopChanged(int, int)));

				ckdbbottom = new AuCheckBoxValue("", dockBar);
				ckdbbottom->setCheckState((dockSides & Qt::BottomDockWidgetArea) ? Qt::Checked : Qt::Unchecked);
				connect(ckdbbottom, SIGNAL(StateChanged(int, int)), SLOT(DockBottomChanged(int, int)));
			}

			lay->addWidget(lbdb, row,0);
			//lay->addWidget(ckdbtitle, row,2);
			lay->addWidget(ckdbleft, row,4);
			lay->addWidget(ckdbright, row,6);
			lay->addWidget(ckdbtop, row,8);
			lay->addWidget(ckdbbottom, row,10);
			row++;
		}
		lay->setSpacing(2);
		lay->setMargin(2);
		dockbars->setLayout(lay);

		dockbarScroller->setWidget(dockbars);
		dockbarScroller->setWidgetResizable(true);
	}

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(cknativecolor, row,0);
	lay->addWidget(pbcolor, row,1);
	row++;
	lay->addWidget(cknativefile, row,0);
	lay->addWidget(pbfile, row,1);
	row++;
	lay->addWidget(cknativefont, row,0);
	lay->addWidget(pbfont, row,1);
	row++;
	lay->addWidget(ckstorydialog, row,0);
	row++;
	if(bAllowTransparency)
	{
		lay->addWidget(lbactive, row,0, 1,2);
		row++;
		lay->addWidget(slactive, row,0, 1,2);
		row++;
		lay->addWidget(lbless0, row,0, 1,1);
		lay->addWidget(lbmore0, row,1, 1,1);
		row++;
		lay->addWidget(lbinactive, row,0, 1,2);
		row++;
		lay->addWidget(slinactive, row,0, 1,2);
		row++;
		lay->addWidget(lbless1, row,0, 1,1);
		lay->addWidget(lbmore1, row,1, 1,1);
		row++;
	}
	lay->addWidget(pbresetw, row,0, 1,2);
	row++;
	lay->addWidget(dockbarScroller, row,0, 1,2);

	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuSetWindows::~AuSetWindows()
{
}

void AuSetWindows::ResetWidgets()
{
	// All widgets store their sizes and positions under the Widget key.
	g_Au.Settings().remove("Widget");
}

void AuSetWindows::SetActiveTrans(int value)
{
	g_Au.SetWindowOpacity(true, 100 - value);
}

void AuSetWindows::SetInactiveTrans(int value)
{
	g_Au.SetWindowOpacity(false, 100 - value);
}

void AuSetWindows::DockBottomChanged(int state, int value)
{
	// This default value for dockSides came from the default constructor of AuDockWidget.
	int dockSides = Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea|Qt::BottomDockWidgetArea;
	dockSides = g_Au.Settings().readNumEntry(s_DockBarSettings[value], dockSides);

	// Toggle the appropriate state.
	if(state == Qt::Checked)
		dockSides |= Qt::BottomDockWidgetArea;
	else
		dockSides &= ~Qt::BottomDockWidgetArea;

	// Set the allowed areas in the dock widget. Its SetAllowedAreas saves the setting.
	// The dock widget will not exist yet when the settings are displayed at the login screen.
	AuDockWidget* dockwidget = auApp->Frame()->findChild<AuDockWidget*>(s_DockBarObjectNames[value]);
	if(dockwidget)
		dockwidget->SetAllowedAreas((Qt::DockWidgetArea)dockSides);
	else
		g_Au.Settings().writeEntry(s_DockBarSettings[value], dockSides);
}

void AuSetWindows::DockLeftChanged(int state, int value)
{
	// This default value for dockSides came from the default constructor of AuDockWidget.
	int dockSides = Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea|Qt::BottomDockWidgetArea;
	dockSides = g_Au.Settings().readNumEntry(s_DockBarSettings[value], dockSides);

	// Toggle the appropriate state.
	if(state == Qt::Checked)
		dockSides |= Qt::LeftDockWidgetArea;
	else
		dockSides &= ~Qt::LeftDockWidgetArea;

	// Set the allowed areas in the dock widget. Its SetAllowedAreas saves the setting.
	// The dock widget will not exist yet when the settings are displayed at the login screen.
	AuDockWidget* dockwidget = auApp->Frame()->findChild<AuDockWidget*>(s_DockBarObjectNames[value]);
	if(dockwidget)
		dockwidget->SetAllowedAreas((Qt::DockWidgetArea)dockSides);
	else
		g_Au.Settings().writeEntry(s_DockBarSettings[value], dockSides);
}

void AuSetWindows::DockRightChanged(int state, int value)
{
	// This default value for dockSides came from the default constructor of AuDockWidget.
	int dockSides = Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea|Qt::BottomDockWidgetArea;
	dockSides = g_Au.Settings().readNumEntry(s_DockBarSettings[value], dockSides);

	// Toggle the appropriate state.
	if(state == Qt::Checked)
		dockSides |= Qt::RightDockWidgetArea;
	else
		dockSides &= ~Qt::RightDockWidgetArea;

	// Set the allowed areas in the dock widget. Its SetAllowedAreas saves the setting.
	// The dock widget will not exist yet when the settings are displayed at the login screen.
	AuDockWidget* dockwidget = auApp->Frame()->findChild<AuDockWidget*>(s_DockBarObjectNames[value]);
	if(dockwidget)
		dockwidget->SetAllowedAreas((Qt::DockWidgetArea)dockSides);
	else
		g_Au.Settings().writeEntry(s_DockBarSettings[value], dockSides);
}

void AuSetWindows::DockTopChanged(int state, int value)
{
	// This default value for dockSides came from the default constructor of AuDockWidget.
	int dockSides = Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea|Qt::BottomDockWidgetArea;
	dockSides = g_Au.Settings().readNumEntry(s_DockBarSettings[value], dockSides);

	// Toggle the appropriate state.
	if(state == Qt::Checked)
		dockSides |= Qt::TopDockWidgetArea;
	else
		dockSides &= ~Qt::TopDockWidgetArea;

	// Set the allowed areas in the dock widget. Its SetAllowedAreas saves the setting.
	// The dock widget will not exist yet when the settings are displayed at the login screen.
	AuDockWidget* dockwidget = auApp->Frame()->findChild<AuDockWidget*>(s_DockBarObjectNames[value]);
	if(dockwidget)
		dockwidget->SetAllowedAreas((Qt::DockWidgetArea)dockSides);
	else
		g_Au.Settings().writeEntry(s_DockBarSettings[value], dockSides);
}

void AuSetWindows::ToolBarBottomChanged(int state, int value)
{
	int dockSides = Qt::LeftToolBarArea|Qt::RightToolBarArea|Qt::TopToolBarArea|Qt::BottomToolBarArea;
	dockSides = g_Au.Settings().readNumEntry(s_DockBarSettings[value], dockSides);

	// Toggle the appropriate state.
	if(state == Qt::Checked)
		dockSides |= Qt::BottomToolBarArea;
	else
		dockSides &= ~Qt::BottomToolBarArea;

	// Set the allowed areas in the tool bar widget.
	// The tool bar widget will not exist yet when the settings are displayed at the login screen.
	QToolBar* bar = auApp->Frame()->findChild<QToolBar*>(s_DockBarObjectNames[value]);
	if(bar)
		bar->setAllowedAreas((Qt::ToolBarArea)dockSides);

	g_Au.Settings().writeEntry(s_DockBarSettings[value], dockSides);
}

void AuSetWindows::ToolBarLeftChanged(int state, int value)
{
	int dockSides = Qt::LeftToolBarArea|Qt::RightToolBarArea|Qt::TopToolBarArea|Qt::BottomToolBarArea;
	dockSides = g_Au.Settings().readNumEntry(s_DockBarSettings[value], dockSides);

	// Toggle the appropriate state.
	if(state == Qt::Checked)
		dockSides |= Qt::LeftToolBarArea;
	else
		dockSides &= ~Qt::LeftToolBarArea;

	// Set the allowed areas in the tool bar widget.
	// The tool bar widget will not exist yet when the settings are displayed at the login screen.
	QToolBar* bar = auApp->Frame()->findChild<QToolBar*>(s_DockBarObjectNames[value]);
	if(bar)
		bar->setAllowedAreas((Qt::ToolBarArea)dockSides);

	g_Au.Settings().writeEntry(s_DockBarSettings[value], dockSides);
}

void AuSetWindows::ToolBarRightChanged(int state, int value)
{
	int dockSides = Qt::LeftToolBarArea|Qt::RightToolBarArea|Qt::TopToolBarArea|Qt::BottomToolBarArea;
	dockSides = g_Au.Settings().readNumEntry(s_DockBarSettings[value], dockSides);

	// Toggle the appropriate state.
	if(state == Qt::Checked)
		dockSides |= Qt::RightToolBarArea;
	else
		dockSides &= ~Qt::RightToolBarArea;

	// Set the allowed areas in the tool bar widget.
	// The tool bar widget will not exist yet when the settings are displayed at the login screen.
	QToolBar* bar = auApp->Frame()->findChild<QToolBar*>(s_DockBarObjectNames[value]);
	if(bar)
		bar->setAllowedAreas((Qt::ToolBarArea)dockSides);

	g_Au.Settings().writeEntry(s_DockBarSettings[value], dockSides);
}

void AuSetWindows::ToolBarTopChanged(int state, int value)
{
	int dockSides = Qt::LeftToolBarArea|Qt::RightToolBarArea|Qt::TopToolBarArea|Qt::BottomToolBarArea;
	dockSides = g_Au.Settings().readNumEntry(s_DockBarSettings[value], dockSides);

	// Toggle the appropriate state.
	if(state == Qt::Checked)
		dockSides |= Qt::TopToolBarArea;
	else
		dockSides &= ~Qt::TopToolBarArea;

	// Set the allowed areas in the tool bar widget.
	// The tool bar widget will not exist yet when the settings are displayed at the login screen.
	QToolBar* bar = auApp->Frame()->findChild<QToolBar*>(s_DockBarObjectNames[value]);
	if(bar)
		bar->setAllowedAreas((Qt::ToolBarArea)dockSides);

	g_Au.Settings().writeEntry(s_DockBarSettings[value], dockSides);
}

void AuSetWindows::UseStoryDialogChanged(int checkState)
{
	g_AuClient.SetStoryDialog(checkState == Qt::Checked);
}

void AuSetWindows::UseNativeColorChanged(int checkState)
{
	g_Au.SetUseNativeColorDialog(checkState == Qt::Checked);
}

void AuSetWindows::UseNativeFileChanged(int checkState)
{
	g_Au.SetUseNativeFileDialog(checkState == Qt::Checked);
}

void AuSetWindows::UseNativeFontChanged(int checkState)
{
	g_Au.SetUseNativeFontDialog(checkState == Qt::Checked);
}

void AuSetWindows::TestColorDialog()
{
	QColorDialog::getColor(QColor(), this, tr("Select Color"),
			g_Au.UseNativeColorDialog() ? QColorDialog::ColorDialogOptions() : QColorDialog::DontUseNativeDialog);
}

void AuSetWindows::TestFileDialog()
{
	QFileDialog::getOpenFileName(this, tr("Select File"), "", "All Files (*.*)", 0,
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
}

void AuSetWindows::TestFontDialog()
{
	bool ok;
	QFontDialog::getFont(&ok, QFont(), this, tr("Select Font"),
			g_Au.UseNativeFontDialog() ? QFontDialog::FontDialogOptions() : QFontDialog::DontUseNativeDialog);
}

