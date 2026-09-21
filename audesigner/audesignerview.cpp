//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignerview.cpp,v 1.165 2022/05/05 15:58:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audesignerview.h"
//
#include "aubddesignprops.h"
#include "aubdfinalizedlg.h"
#include "aubdgearsave.h"
#include "aubdlibdeletedlg.h"
#include "aubdlibopendlg.h"
#include "aubdlibsavedlg.h"
#include "aubdpreview.h"
#include "aubox.h"
#include "auchooseindustry.h"
#include "auclientglobal.h"
#include "aucommstdchannels.h"
#include "audefusebomb.h"
#include "audesignerapp.h"
#include "audesignerframe.h"
#include "audesignergearopen.h"
#include "audesignergearsave.h"
#include "audesignlibopendlg.h"
#include "audesignlibsavedlg.h"
#include "aueffect.h"
#include "auelapsedtimer.h"
#include "aufilever.h"
#include "aufinalizereportdlg.h"
#include "augame.h"
#include "auglmodelmaker.h"
#include "auimagedialog.h"
#include "auitdesignprops.h"
#include "auitemmanipulator.h"
#include "auitfinalizedlg.h"
#include "auitlibdeletedlg.h"
#include "auitlibopendlg.h"
#include "auitlibsavedlg.h"
#include "aujoystick.h"
#include "auscdesignprops.h"
#include "ausclibdeletedlg.h"
#include "auscfinalizedlg.h"
#include "auscparts.h"
#include "auscpreview.h"
#include "auselcontiguousdlg.h"
#include "ausetcontrols.h"
#include "ausound.h"
#include "austylesheet.h"
#include "daanimal.h"
#include "daarena.h"
#include "daatom.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dacitizen.h"
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
#include "ddbdbuildzone.h"
#include "ddbdcitizenpost.h"
#include "ddbdfence.h"
#include "ddbdlivestockpost.h"
#include "ddbdpreview.h"
#include "ddbdshrub.h"
#include "ddbdsite.h"
#include "ddbdtree.h"
#include "ddgrid3pt.h"
#include "ddgridmove.h"
#include "ddplanbdbuilding.h"
#include "ddplanbdfence.h"
#include "ddplanclearcraters.h"
#include "ddplanclearglobe.h"
#include "ddplangeolandscape.h"
#include "ddplangeoroad.h"
#include "ddpreviewfence.h"
#include "ddscalign.h"
#include "ddscalignliftstop.h"
#include "ddscberth.h"
#include "ddscberthname.h"
#include "ddsccaptname.h"
#include "ddscconsole.h"
#include "ddscconsoleengineer.h"
#include "ddscconsolefirecontrol.h"
#include "ddscconsolemedic.h"
#include "ddscconsoletransporter.h"
#include "ddsccontrailemitter.h"
#include "ddsccopysymmetrical.h"
#include "ddsccutface.h"
#include "ddsccuttingjig.h"
#include "ddscdecaljig.h"
#include "ddscdivide.h"
#include "ddscdooraccess.h"
#include "ddscdoorjig.h"
#include "ddsceffectintensity.h"
#include "ddscempireflag.h"
#include "ddscempirename.h"
#include "ddscenvvoidextrusion.h"
#include "ddscenvvoidspindle.h"
#include "ddscexportstl.h"
#include "ddscextrusion.h"
#include "ddscface.h"
#include "ddscgametable.h"
#include "ddscglow.h"
#include "ddscimport3ds.h"
#include "ddscimportstl.h"
#include "ddscjigcut.h"
#include "ddscladderpath.h"
#include "ddsclandingsystem.h"
#include "ddsclaunchtube.h"
#include "ddsclaunchtubejig.h"
#include "ddsclightbulb.h"
#include "ddsclightbulbprops.h"
#include "ddsclightlensprops.h"
#include "ddscloopcut.h"
#include "ddscmakedecal.h"
#include "ddscmakedoor.h"
#include "ddscmakehull.h"
#include "ddscmakelandinggear.h"
#include "ddscmakelightlens.h"
#include "ddscmakemesh.h"
#include "ddscmakenavlightlens.h"
#include "ddscmakeroom.h"
#include "ddscmakerotatingbeaconlens.h"
#include "ddscmakestrobelens.h"
#include "ddscmaketurbolift.h"
#include "ddscmaketurret.h"
#include "ddscmakewindow.h"
#include "ddscmove.h"
#include "ddscmovedoorswitches.h"
#include "ddscopacity.h"
#include "ddscpaint.h"
#include "ddscparkgroundvehicle.h"
#include "ddscparkrocket.h"
#include "ddscparkspacecraft.h"
#include "ddscparkspacevehicle.h"
#include "ddscparkwatervehicle.h"
#include "ddscpasteat.h"
#include "ddscpasteradial.h"
#include "ddscpodbay.h"
#include "ddscpolish.h"
#include "ddscpunchface.h"
#include "ddscpushpull.h"
#include "ddscquad.h"
#include "ddscremotecamera.h"
#include "ddscroomprops.h"
#include "ddscroomvoidextrusion.h"
#include "ddscroomvoidspindle.h"
#include "ddscrotate.h"
#include "ddscscreen.h"
#include "ddscselfillumination.h"
#include "ddscshipname.h"
#include "ddscsketch.h"
#include "ddscspherelatlong.h"
#include "ddscsphereuniform.h"
#include "ddscspindle.h"
#include "ddscstation.h"
#include "ddscstationengineer.h"
#include "ddscstationfirecontrol.h"
#include "ddscstationgunner.h"
#include "ddscstationmedic.h"
#include "ddscstationtransporter.h"
#include "ddscstretch.h"
#include "ddsctexturecylindrical.h"
#include "ddsctexturepalette.h"
#include "ddsctextureprojected.h"
#include "ddsctexturespherical.h"
#include "ddsctransporterpadlocs.h"
#include "ddsctriangle.h"
#include "ddsctrooppost.h"
#include "ddsctubing.h"
#include "ddscturretbase.h"
#include "ddscturretgun.h"
#include "ddscvoidassociate.h"
#include "ddscvoidprops.h"
#include "ddscwalkpath.h"
#include "ddscweaponemitloc.h"
#include "ddscwindowjig.h"
#include "ddsiteproperties.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "dtgl.h"
#include "dttarget.h"
#include <QAction>
#include <QCursor>
#include <QDir>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QToolTip>
#include <QWheelEvent>

// Switches useful for testing.
//static int s_Test = 0;
static bool s_Animate = true;			// Ctrl+1 - AnimateDeviceObjects
static bool s_Create = true;			// Ctrl+2 - CreateDeviceObjects
static bool s_RenderComm = true;		// Ctrl+3 - RenderCommMessages
static bool s_RenderCursor = true;		// Ctrl+4 - RenderCursor
static bool s_RenderEnv = true;			// Ctrl+5 - RenderEnvironment
static bool s_RenderFade = true;		// Ctrl+6 - ShowScreenFadeEffect and ShowSpawnRecallWait
static bool s_RenderScene = true;		// Ctrl+7 - RenderScene
static bool s_RenderTactical = true;	// Ctrl+8 - RenderTactical
static bool s_SightTarget = true;		// Ctrl+9 - SightTarget

// Tuning parameters.
#define d_AtomControlTime				10						// Msec between atom control updates.
#define d_FrameRate						60						// Target frame rate, in frames per second.
#define d_SubtitleWordWrap				128						// Line length of narrative text.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Table of control inputs.
DiControlInputArray s_ViewControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_ViewDefaults[] =
{
	// General
	DiControlInput(QObject::tr("<b>General</b>"),
			AuDesignerView::CI_Title, Qt::Key_unknown,
			Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Deselect Units and End Draw Command"),
			AuDesignerView::CI_EndCommand, Qt::Key_Escape, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Input Mode Toggle, Avatar vs Hand"),
			AuDesignerView::CI_InputModeToggle, Qt::Key_Return, Qt::NoModifier,
			DiControlInput::MT_ButtonRight, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Hand Input Mode, Temporarily"),
			QObject::tr("Temporarily releases the cursor from avatar input mode. "
				"Enables quick access to your avatar's hand to grab items, open doors, activate in-game control consoles, "
				"and operate window and dialog controls."),
			AuDesignerView::CI_InputModeHandTemporary, Qt::Key_Control, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Show Console UI or Game Table UI"),
			QObject::tr("When seated at a console, the Console UI is displayed. "
				"When seated at a game table, the Game Table UI is displayed."),
			AuDesignerView::CI_ConsoleUI, Qt::Key_Tab, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Spot Target, Report On Comm Channel"),
			QObject::tr("Information about the target is called out on the currently selected comm channel."),
			AuDesignerView::CI_SpotTargetComm, Qt::Key_AsciiTilde, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Spot Target, Report Subconsciously"),
			QObject::tr("Information about the target is called out on the <i>Thoughts</i> comm channel."),
			AuDesignerView::CI_SpotTargetLog, Qt::Key_QuoteLeft, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),

	// Screen
	DiControlInput(QObject::tr("<b>Screen</b>"),
			AuDesignerView::CI_Title, Qt::Key_unknown,
			Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Screen Elements Toggle"),
			QObject::tr("Screen elements are things like the crosshair, health bars, direction indicators and chat messages. "
				"This switch is useful when preparing to make a screen shot."),
			AuDesignerView::CI_ScreenElementsToggle, Qt::Key_Escape, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("HUD Intensity Decrease"),
			AuDesignerView::CI_HudIntensityDecrease, Qt::Key_Comma, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("HUD Intensity Increase"),
			AuDesignerView::CI_HudIntensityIncrease, Qt::Key_Period, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Screen Message Intensity"),
			QObject::tr("On screen message intensity can be cycled between 4 intensities, one of which is <i>off</i>."),
			AuDesignerView::CI_MessageIntensity, Qt::Key_Slash, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Save Screen Grab"),
			QObject::tr("Screen shots are saved to a folder configured using Settings."),
			AuDesignerView::CI_ScreenShot, Qt::Key_P, Qt::AltModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),

	// Person View
	DiControlInput(QObject::tr("<b>Person View</b>"),
			AuDesignerView::CI_Title, Qt::Key_unknown,
			Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("First Person vs Third Person View Toggle"),
			AuDesignerView::CI_FirstThirdPersonViewToggle, Qt::Key_Backspace, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("View Angle Decrease"),
			AuDesignerView::CI_ViewAngleDecrease, Qt::Key_BracketLeft, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("View Angle Increase"),
			AuDesignerView::CI_ViewAngleIncrease, Qt::Key_BracketRight, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("View Distance Decrease"),
			AuDesignerView::CI_ViewDistanceDecrease, Qt::Key_BracketLeft, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("View Distance Increase"),
			AuDesignerView::CI_ViewDistanceIncrease, Qt::Key_BracketRight, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Free Camera, <i>Programmer Only</i>"),			// Programmer Only
			AuDesignerView::CI_FreeCamera, Qt::Key_Pause, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),

	// Top Down View
	DiControlInput(QObject::tr("<b>Top Down View</b>"),
			AuDesignerView::CI_Title, Qt::Key_unknown,
			Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Top Down View vs Person View Toggle"),
			AuDesignerView::CI_TopDownViewToggle, Qt::Key_Backspace, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Terrain, Next Info Overlay"),
			QObject::tr("Top down view information can be cycled between 8 overlays, one of which is <i>none</i>."),
			AuDesignerView::CI_TopDownNextInfo, Qt::Key_Backslash, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Terrain, Previous Info Overlay"),
			QObject::tr("Top down view information can be cycled between 8 overlays, one of which is <i>none</i>."),
			AuDesignerView::CI_TopDownPrevInfo, Qt::Key_Bar, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),

	// Unit Orders
	DiControlInput(QObject::tr("<b>Unit Orders</b>"),
			QObject::tr("Unit orders correspond to the orders that can be issued using the <i>Unit Orders</i> window."),
			AuDesignerView::CI_Title, Qt::Key_unknown,
			Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Attack Target"),
			AuDesignerView::CI_SquadOrderAttackTarget, Qt::Key_A, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Close Door"),
			AuDesignerView::CI_SquadOrderCloseDoor, Qt::Key_C, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Come Here"),
			AuDesignerView::CI_SquadOrderComeHere, Qt::Key_H, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Drop Rider"),
			AuDesignerView::CI_SquadOrderDropRider, Qt::Key_Q, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Enter Surgery"),
			AuDesignerView::CI_SquadOrderEnterSurgery, Qt::Key_Y, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Follow Me"),
			AuDesignerView::CI_SquadOrderFollowMe, Qt::Key_M, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Follow Target"),
			AuDesignerView::CI_SquadOrderFollowTarget, Qt::Key_F, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Garrison"),
			AuDesignerView::CI_SquadOrderGarrison, Qt::Key_R, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Get In"),
			AuDesignerView::CI_SquadOrderGetIn, Qt::Key_I, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Get Item or Dying Creature"),
			AuDesignerView::CI_SquadOrderGetItem, Qt::Key_G, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Get Out or Dismount"),
			AuDesignerView::CI_SquadOrderGetOut, Qt::Key_O, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Go To Station"),
			AuDesignerView::CI_SquadOrderBattleStation, Qt::Key_N, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Go To Target"),
			AuDesignerView::CI_SquadOrderGoToTarget, Qt::Key_T, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Mount Me"),
			AuDesignerView::CI_SquadOrderMountMe, Qt::Key_U, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Mount Target"),
			AuDesignerView::CI_SquadOrderMountTarget, Qt::Key_J, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Open Door"),
			AuDesignerView::CI_SquadOrderOpenDoor, Qt::Key_P, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Release The Con"),
			AuDesignerView::CI_SquadOrderReleaseCommand, Qt::Key_X, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Stay"),
			AuDesignerView::CI_SquadOrderStay, Qt::Key_S, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Take The Con"),
			AuDesignerView::CI_SquadOrderTakeCommand, Qt::Key_Z, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),

	// Unit Postures
	DiControlInput(QObject::tr("<b>Unit Postures</b>"),
			QObject::tr("Unit postures correspond to the posture settings on the <i>Unit Orders</i> window."),
			AuDesignerView::CI_Title, Qt::Key_unknown,
			Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Berserk Posture"),
			AuDesignerView::CI_SquadPostureBerserk, Qt::Key_K, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Offensive Posture"),
			AuDesignerView::CI_SquadPostureOffensive, Qt::Key_V, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Defend Area Posture"),
			AuDesignerView::CI_SquadPostureDefendArea, Qt::Key_D, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Defend Self Posture"),
			AuDesignerView::CI_SquadPostureDefendSelf, Qt::Key_E, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Hold Fire Posture"),
			AuDesignerView::CI_SquadPostureHoldFire, Qt::Key_L, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),

	// Squads
	DiControlInput(QObject::tr("<b>Squads</b>"),
			AuDesignerView::CI_Title, Qt::Key_unknown,
			Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Create Squad 1"),
			AuDesignerView::CI_SquadCreate1, Qt::Key_Exclam, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Create Squad 2"),
			AuDesignerView::CI_SquadCreate2, Qt::Key_At, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Create Squad 3"),
			AuDesignerView::CI_SquadCreate3, Qt::Key_NumberSign, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Create Squad 4"),
			AuDesignerView::CI_SquadCreate4, Qt::Key_Dollar, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Create Squad 5"),
			AuDesignerView::CI_SquadCreate5, Qt::Key_Percent, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Create Squad 6"),
			AuDesignerView::CI_SquadCreate6, Qt::Key_AsciiCircum, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Create Squad 7"),
			AuDesignerView::CI_SquadCreate7, Qt::Key_Ampersand, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Create Squad 8"),
			AuDesignerView::CI_SquadCreate8, Qt::Key_Asterisk, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Create Squad 9"),
			AuDesignerView::CI_SquadCreate9, Qt::Key_ParenLeft, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Create Squad 10"),
			AuDesignerView::CI_SquadCreate0, Qt::Key_ParenRight, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Select Squad 1"),
			AuDesignerView::CI_SquadSelect1, Qt::Key_1, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Select Squad 2"),
			AuDesignerView::CI_SquadSelect2, Qt::Key_2, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Select Squad 3"),
			AuDesignerView::CI_SquadSelect3, Qt::Key_3, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Select Squad 4"),
			AuDesignerView::CI_SquadSelect4, Qt::Key_4, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Select Squad 5"),
			AuDesignerView::CI_SquadSelect5, Qt::Key_5, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Select Squad 6"),
			AuDesignerView::CI_SquadSelect6, Qt::Key_6, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Select Squad 7"),
			AuDesignerView::CI_SquadSelect7, Qt::Key_7, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Select Squad 8"),
			AuDesignerView::CI_SquadSelect8, Qt::Key_8, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Select Squad 9"),
			AuDesignerView::CI_SquadSelect9, Qt::Key_9, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Select Squad 10"),
			AuDesignerView::CI_SquadSelect0, Qt::Key_0, Qt::ControlModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),

	// Designer
	DiControlInput(QObject::tr("<b>Designer</b>"),
			AuDesignerView::CI_Title, Qt::Key_unknown,
			Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier, DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Grab Selection"),
			AuDesignerView::CI_DesignerGrab, Qt::Key_G, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Rotate Selection"),
			AuDesignerView::CI_DesignerRotate, Qt::Key_R, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Stretch Selection"),
			AuDesignerView::CI_DesignerStretch, Qt::Key_S, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Push/Pull Faces"),
			QObject::tr("Face mode only."),
			AuDesignerView::CI_DesignerPushPull, Qt::Key_P, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("X Drag Constraint"),
			AuDesignerView::CI_DragConstrainX, Qt::Key_X, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Y Drag Constraint"),
			AuDesignerView::CI_DragConstrainY, Qt::Key_Y, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Z Drag Constraint"),
			AuDesignerView::CI_DragConstrainZ, Qt::Key_Z, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
};
int s_NumViewDefaults = sizeof(s_ViewDefaults)/sizeof(DiControlInput);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// static
QString AuViewButton::Image(VIEWBUTTON vb)
{
	QString icons[VB_COUNT+1] =
	{
		"",								// VB_Invalid,
		":data/Agent.png",				// VB_Agent,
		":data/Architect.png",			// VB_Architect,
		":data/ArenaExit.png",			// VB_ArenaExit,
		":data/ArenaRecall.png",		// VB_ArenaRecall,
		":data/ArenaStatus.png",		// VB_ArenaStatus,
		":data/c_VacSuitHelmet.png",	// VB_Bio,
		":data/c_Crate.png",			// VB_Cargo,
		":data/RangeUniverse.png",		// VB_SaveSolitaire,
		":data/Comm.png",				// VB_Comm,
		"",	// VB_UNUSED3,
		"",	// VB_UNUSED5,
		":data/ExitGame.png",			// VB_Exit,
		":data/ToggleFull.png",			// VB_FullScreen,
		":data/CommGovernment.png",		// VB_Government,
		":data/CommHelp.png",			// VB_Help,
		":data/ToggleAll.png",			// VB_HideWindows,
		":data/Inventory.png",			// VB_Gear,
		":data/Locator.png",			// VB_Locator,
		":data/ToggleMail.png",			// VB_Mail,
		"",			// VB_UNUSED6,
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
		"",		// VB_UNUSED4,
		":data/GovTaxes.png",			// VB_GovernTaxes,
		":data/CommShip.png",			// VB_Forum,
		":data/OrderRead.png",			// VB_Wiki,
		":data/GovCommerce.png",		// VB_WarDimensionStandings,
		"",		// VB_UNUSED8,
		":data/CommCrew.png",			// VB_Avatars,
		":data/c_Crate.png",			// VB_CargoManifest,
		":data/Exchange.png",			// VB_SpacecraftDesignExchange,
		":data/SoHLogo32.png",			// VB_HazeronHome,
		":data/Minion.png",				// VB_Minion,
		":data/ContactBuddies.png",		// VB_PlayersOnline,
		":data/FilmReel64.png",			// VB_Videos,
		":data/Exchange.png",			// VB_BdDesignExchange,
		":data/City.png",				// VB_Building,
		":data/ConstructionSign.png",	// VB_ConstructionBd,
		":data/c_Patent.png",			// VB_GovernPatents,
		"",			// VB_UNUSED1,
		":data/c_Container.png",		// VB_SpacecraftShipments,
		":data/it_HandCraft.png",		// VB_GearHandCraft,
		":data/RangeSystem.png",		// VB_LocatorSystemScan,
		":data/c_ScDesignDrawing.png",	// VB_SpacecraftSpecs,
		"",	// VB_UNUSED7,
		"",								// VB_COUNT,
	};
	if(vb == VB_About)
		return auApp->Icon32();
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
		"Arena Exit",					// VB_ArenaExit,
		"Arena Recall",					// VB_ArenaRecall,
		"Arena Status",					// VB_ArenaStatus,
		"Bio",							// VB_Bio,
		"Cargo Transfer",				// VB_Cargo,
		"Save Game",					// VB_SaveSolitaire,
		"Comm",							// VB_Comm,
		"",		// VB_UNUSED3,
		"",						// VB_UNUSED5,
		"Exit",							// VB_Exit,
		"Full Screen",					// VB_FullScreen,
		"Governance",					// VB_Government,
		"Call for Help",				// VB_Help,
		"Show or Hide All Windows",		// VB_HideWindows,
		"Gear",							// VB_Gear,
		"Locator",						// VB_Locator,
		"Mail",							// VB_Mail,
		"",	// VB_UNUSED6,
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
		"",						// VB_UNUSED4,
		"Taxes",						// VB_GovernTaxes,
		"Forum",						// VB_Forum,
		"Wiki",							// VB_Wiki,
		"Empire Standings",				// VB_WarDimensionStandings,
		"",				// VB_UNUSED8,
		"Avatars",						// VB_Avatars,
		"Manifest",						// VB_CargoManifest,
		"Blueprint Exchange",			// VB_SpacecraftDesignExchange,
		"Hazeron Home",					// VB_HazeronHome,
		"Minion",						// VB_Minion,
		"Players Online",				// VB_PlayersOnline,
		"Videos",						// VB_Videos,
		"Blueprint Exchange",			// VB_BdDesignExchange,
		"Building",						// VB_Building,
		"Construction",					// VB_ConstructionBd,
		"Patents",						// VB_GovernPatents,
		"",						// VB_UNUSED1,
		"Shipments",					// VB_SpacecraftShipments,
		"Hand Craft",					// VB_GearHandCraft,
		"System Map",					// VB_LocatorSystemScan,
		"Specifications",				// VB_SpacecraftSpecs,
		"",						// VB_UNUSED7,
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
		"Arena Exit",					// VB_ArenaExit,
		"Arena Recall",					// VB_ArenaRecall,
		"Arena Status",					// VB_ArenaStatus,
		"Bio",							// VB_Bio,
		"Cargo Transfer",				// VB_Cargo,
		"Save Game",					// VB_SaveSolitaire,
		"Comm",							// VB_Comm,
		"",		// VB_UNUSED3,
		"",						// VB_UNUSED5,
		"Exit",							// VB_Exit,
		"Full Screen",					// VB_FullScreen,
		"Governance",					// VB_Government,
		"Call for Help",				// VB_Help,
		"Show or Hide All Windows",		// VB_HideWindows,
		"Gear",							// VB_Gear,
		"Locator",						// VB_Locator,
		"Mail",							// VB_Mail,
		"",	// VB_UNUSED6,
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
		"",				// VB_UNUSED4,
		"Govern Taxes",					// VB_GovernTaxes,
		"Web Site Forum",				// VB_Forum,
		"Web Site Wiki",				// VB_Wiki,
		"Web Site Empire Standings",		// VB_WarDimensionStandings,
		"",		// VB_UNUSED8,
		"Web Site Avatars",				// VB_Avatars,
		"Cargo Manifest",				// VB_CargoManifest,
		"Spacecraft Blueprint Exchange",	// VB_SpacecraftDesignExchange,
		"Web Site Home Page",			// VB_HazeronHome,
		"Minion",						// VB_Minion,
		"Players Online",				// VB_PlayersOnline,
		"Web Site Videos",				// VB_Videos,
		"Building Blueprint Exchange",	// VB_BdDesignExchange,
		"Building",						// VB_Building,
		"Construction",					// VB_ConstructionBd,
		"Govern Patents",				// VB_GovernPatents,
		"",						// VB_UNUSED1,
		"Spacecraft Shipments",			// VB_SpacecraftShipments,
		"Hand Craft",					// VB_GearHandCraft,
		"Locator System Map",			// VB_LocatorSystemScan,
		"Spacecraft Specifications",	// VB_SpacecraftSpecs,
		"",	// VB_UNUSED7,
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
		QObject::tr("<b>Exit Arena</b> %2<br>"
			"Exits the arena. "
			"Returns you to the place where you entered the arena."),
		QObject::tr("<b>Arena Sideline</b> %2<br>"
			"Moves your avatar to the sidelines of the arena."),
		QObject::tr("<b>Arena Status</b> %2<br>"
			"Reports status of the current match in the arena."),
		QObject::tr("<b>Bio</b> %2<br>"										// VB_Bio
			"Show the <i>Bio</i> window to view information about your avatar."),
		QObject::tr("<b>Cargo</b> %2<br>"
			"Show the <i>Cargo</i> window to:<br>"
			"%1 Transfer items between your gear and the spacecraft you are aboard.<br>"
			"%1 Transfer money, citizens and cargo between two different spacecraft.<br>"
			"%1 Load ordnance into your vehicle from a spacecraft, station, or city.").arg(g_Au.MiddleDotSymbol()),
		QObject::tr("<b>Save Game</b> %2<br>"
				"Creates a save of the game. "
				"Saved games can be restored when the program starts. "
				"</html>"),							// VB_SaveSolitaire
		QObject::tr("<b>Comm</b> %2<br>"
			"Show the <i>Comm</i> window to communicate with other players, spacecraft and cities."),
		QObject::tr("<b>UNUSED3</b> %2</html>"),					// VB_UNUSED3
		QObject::tr("<b>UNUSED5</b> %2</html>"),	// VB_UNUSED5
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
			"<b>Cancel recall</b> by pressing the Recall Home button while waiting to recall."
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
		QObject::tr("<b>UNUSED4</b> %2<br>"										// VB_UNUSED4,
				""),
		QObject::tr("<b>Taxes</b> %2<br>"										// VB_GovernTaxes,
			"Show the <i>Taxes</i> window to view and change taxes of your empire."),
		QObject::tr("<b>Forum</b> %2<br>"										// VB_Forum
			"Opens your web browser at the <i>Shores of Hazeron</i> Forum. "
			"The <i>forum</i> provides a place for discussion with other players and to report problems."),
		QObject::tr("<b>Wiki</b> %2<br>"										// VB_Wiki
			"Opens your web browser at the <i>Shores of Hazeron</i> Wiki. "
			"The <i>wiki</i> is a valuable resource to find answers."),
		QObject::tr("<b>Empire Standings</b> %2<br>"						// VB_WarDimensionStandings
			"Opens your web browser at the <i>Shores of Hazeron</i> empire standings page."),
		QObject::tr("<b>UNUSED8</b> %2<br>"						// VB_UNUSED8
			""),
		QObject::tr("<b>Avatars</b> %2<br>"										// VB_Avatars
			"Opens your web browser at the <i>Shores of Hazeron</i> avatars page. "
			"The avatars page shows information about individual avatars in the game."),
		QObject::tr("<html><b>Manifest</b> %2<br>"								// VB_CargoManifest
			"Shows a manifest of cargo in the hold of the ship. "
			"</html>"),
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
			"Opens your web browser at the <i>Shore of Hazeron</i> videos page. "
			"There are videos to show how to use the designer and how to play the game."),
		QObject::tr("<b>Building Blueprint Exchange</b> %2<br>"
			"Show the <i>Building Blueprint Exchange</i> window to view published blueprints and to publish your own "
			"blueprints."),	// VB_SpacecraftDesignExchange
		QObject::tr("<html><b>Building</b> %2<br>"								// VB_Building
			"Show the <i>Building</i> window to use the building or road at your location. "
			"Window controls change based on the building or road site you occupy; "
			"you do not have to be inside a building."
			"</html>"),
		QObject::tr("<b>Construction</b> %2<br>"								// VB_ConstructionVb
			"Show the <i>Construction</i> window to plan roads and buildings and to found cities. "
			"You must be in the chain of command of a city to develop the city. "
			"Your empire's policies may permit you to found cities even if you are not in the chain of command."),
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
		QObject::tr("<b>Spacecraft Specifications</b> %2<br>"						// VB_SpacecraftSpecs
			"Show the <i>Spacecraft Specifications</i> window to view the design specifications of the spacecraft you are aboard."),
		QObject::tr("<b>UNUSED7</b> %2<br>"),								// VB_UNUSED7
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
	case VB_ArenaExit:
	case VB_ArenaRecall:
	case VB_ArenaStatus:
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
	case VB_SaveSolitaire:
	case VB_UNUSED1:
	case VB_UNUSED3:
	case VB_UNUSED4:
	case VB_UNUSED5:
	case VB_UNUSED6:
	case VB_UNUSED7:
	case VB_UNUSED8:
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
					keycommand = AuBindControls::ModifierName(keymod) + AuBindControls::KeyName(key);
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
	case VB_ConstructionBd:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleConstructionUI);
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
	case VB_SpacecraftSpecs:
		keycommand = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftSpecsUI);
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
	case VB_UNUSED3:	// Old city construction.
	case VB_UNUSED4:
	case VB_UNUSED5:
	case VB_UNUSED6:
	case VB_UNUSED7:
	case VB_UNUSED8:
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

	case VB_SaveSolitaire:
		if(!g_Au.IsSceneSolo())
			return true;
		break;

	case VB_PlayersOnline:
		// These buttons are not visible in single user modes.
		if(auApp->IsSingleUserMode())
			return true;
		break;
	case VB_Avatar1:
	case VB_Avatar2:
	case VB_Avatar3:
	case VB_Avatar4:
	case VB_Avatar5:
	case VB_Avatar6:
	case VB_Bio:
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
	case VB_Recall:
		// These buttons are not visible in single user modes or when the scene context is unknown.
		if(sceneContext == SCENE_Unknown)
			return true;
		if(auApp->IsSingleUserDesignerMode())
			return true;
		break;
	case VB_Mail:
		// These buttons are not visible in single user modes or when the scene context is unknown.
		if(sceneContext == SCENE_Unknown)
			return true;
		if(auApp->IsSingleUserDesignerMode())
			return true;
		break;
	case VB_Help:
		if(sceneContext == SCENE_Unknown)
			return true;
		if(auApp->IsSingleUserMode())
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

	case VB_SpacecraftAccount:
	case VB_SpacecraftMission:
	case VB_SpacecraftPolicy:
	case VB_UnitOrders:
		if(!hasEmpire)
			return true;
		// Fall through
	case VB_Cargo:
	case VB_CargoHold:
	case VB_CargoManifest:
	case VB_CargoOrdnance:
	case VB_CargoTransfer:
	case VB_SpacecraftBounties:
	case VB_SpacecraftDamage:
	case VB_SpacecraftRoster:
	case VB_SpacecraftShipments:
	case VB_SpacecraftSpecs:
		// These buttons are not visible in single user modes or when in the designer.
		if(sceneContext == SCENE_Unknown || sceneContext == SCENE_Designer)
			return true;
		break;

	case VB_Building:
		if(sceneContext == SCENE_Arena)
			return true;
		if(sceneContext == SCENE_Unknown)
			return true;
		break;
	case VB_BdDesignExchange:
		if(sceneContext == SCENE_Arena)
			return true;
		if(sceneContext == SCENE_Unknown)
			return true;
		if(auApp->IsSingleUserDesignerMode())
			return true;
		break;

	case VB_Spacecraft:
		if(sceneContext == SCENE_Unknown)
			return true;
		break;
	case VB_SpacecraftDesignExchange:
		if(sceneContext == SCENE_Unknown)
			return true;
		if(auApp->IsSingleUserDesignerMode())
			return true;
		break;

	case VB_ArenaExit:
	case VB_ArenaRecall:
	case VB_ArenaStatus:
		if(sceneContext != SCENE_Arena)
			return true;
		// These buttons are not visible in single user modes.
		if(auApp->IsSingleUserDesignerMode())
			return true;
		break;

	case VB_ConstructionBd:
		if(sceneContext == SCENE_Unknown || sceneContext == SCENE_Designer || sceneContext == SCENE_Arena)
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

AuSprite::AuSprite()
{
	// Choose a picture.
	int roll = g_Au.Dice().Roll(1,256);
	if(roll < 2)
		m_pic = 7;
	else if(roll < 4)
		m_pic = 6;
	else if(roll < 8)
		m_pic = 5;
	else if(roll < 16)
		m_pic = 4;
	else if(roll < 32)
		m_pic = 3;
	else if(roll < 64)
		m_pic = 2;
	else if(roll < 128)
		m_pic = 1;
	else
		m_pic = 0;

	// Init rotation.
	m_rot = g_Au.Dice().Roll(1,360,-1);

	// Final coord in the range [-15..15], [-15..15], -2000, which is the bottom of the tube.
	AuPoint pos = AuPoint(15 - m_pic*2, 0).RotateZ(m_rot*DEG2RADf);
	m_xpos = pos.x;
	m_ypos = pos.y;
	m_zpos = g_Au.Dice().Roll(1,1999,-2001);
}

void AuSprite::Move(float speed)
{
	// Accelerate in my current direction.
	float mySpeed = (float)(m_pic + 1) * 3 + speed;
	m_zpos += mySpeed;
	if(m_zpos > 0)
		m_zpos -= 2000;	// Return to the bottom of the tube.
	else if(m_zpos < -2000)
		m_zpos += 2000;

	if(m_pic&1)
	{
		m_rot += (10 - m_pic);
		if(m_rot >= 360)
			m_rot -= 360;
	}
	else
	{
		m_rot -= (10 - m_pic);
		if(m_rot < 0)
			m_rot += 360;
	}
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDesignerView::AuDesignerView(QWidget* parent) : QOpenGLWidget(parent), m_SceneEther("AuDesignerView")
{
	m_MM = new AuGLModelMaker(!auApp->IsSingleUserDesignerMode());
	m_ModelCache = new AuGLModelCache();
	m_RenderInfo = new AuAtomRenderInfo(this, auApp->Sound(), *m_MM, *m_ModelCache, g_AuClient.FontAtom());

	setFocusPolicy(Qt::StrongFocus);
	setObjectName("AuView");
	setAttribute(Qt::WA_OpaquePaintEvent);

	m_ActiveKeyMods = Qt::NoModifier;
	m_ActiveMouseMods = Qt::NoModifier;
	m_AnchorLoc = QPoint(-1,-1);
	m_Animated.start();		// Init to a valid time.
	m_AgentBits = 0;
	m_AtomKeyMods = Qt::NoModifier;
	m_AtomMouseMods = Qt::NoModifier;
	m_bFreeCamera = true;
	m_bFreeMouse = false;
	m_BusyAngle = 0;
	m_CameraAnimSpeed = 0;
	m_Cursor = CURSOR_Unknown;
	m_DesignMode = DaDesignBase::InvalidMode;
	m_DragConstraint = 0;
	m_ModalDlgCount = 0;
	m_InCity = false;
	m_InputMode = Character;
	m_IsShipCommand = false;
	m_IsWarping = true;
	m_ItemManipulator = 0;
	m_KickStarter = 0;
	m_Net1Down = 0;
	m_Net1Up = 0;
	m_Net2Down = 0;
	m_Net2Up = 0;
	m_Net3Down = 0;
	m_Net3Up = 0;
	m_Net4Down = 0;
	m_Net4Up = 0;
	m_OnLLWorld = false;
	m_Fade = 0;						// Init to full fade effect.
	m_fCameraSpeed = 0;
	m_GameUI = 0;
	m_GameUITime.start();

	m_GLTextureUnits = 0;
	m_GLThread = 0;
	m_GLUniformLocs = 0;

	m_HandInputTemporary = false;

	// Hide the on-screen buttons.
	m_InputModeChanged = AuElapsedTimer();
	m_SizeChanged = AuElapsedTimer();

	m_IsLefty = false;
	m_MessageIntensity = 3;
	m_MouseButtonsAtom = Qt::NoButton;
	m_MouseButtonsUI = Qt::NoButton;
	m_MouseDragging = false;
	m_MouseGrabbed = false;
	m_MouseLast = QPoint(0,0);
	m_MousePress = QPoint(0,0);
	m_MousePressed = false;
	m_MouseSnapped = false;
	m_MouseVecAtom = QPoint(0,0);
	m_pCommand[0] = 0;
	m_pCommand[1] = 0;

	m_pControlsAtom = 0;
	m_pControlsUI = new DiControlInputArray;
	EnumControlInputs(*m_pControlsUI);							// Grab current UI controls right away.

	m_PersonStationId = 0;

	SelectPreferredFirstThirdPersonView();
	m_RenderInfo->SetTopViewInfo(AuAtomRenderInfo::VI_RocksSome);

	// Far clip must allow for the largest object that gets rendered in the near pass, which is a ringworld arc section plus both
	// of its neighboring sections, ugh.
	m_rFarClip = DaWorldRingSection::MinimumSectionLength() * 2;
	m_rNearClip = 2.5;
	m_SceneAtomEmpire = 0;
	m_SceneAtomEnvType = 0;
	m_SceneAtomTransition = true;
	m_SceneAtomWorldBuildingCount = 0;
	m_SceneAtomWorldBuildingLimit = 0;
	m_SceneAtomWorldBuildingVolume = 0;
	m_SceneAtomWorldFenceCount = 0;
	m_SceneAtomWorldStreetLightCount = 0;
	m_SceneContext = SCENE_Unknown;
	m_SceneGalaxy = GALAXY_Count;
	m_SceneRendered = false;
	m_SizeFoVY = 0;
	m_SpaceAngle = g_Au.Dice().Roll(1,360,-1);
	m_SpawnTieEffect = 0;			// Init to no spawn tie effect.
	m_TargetDNA = 0;
	m_TargetIsCivilian = false;
	m_TargetLevel = 0;
	m_TargetPart = 0;
	m_TargetShield = false;
	m_TargetStance = POL_Neutral;
	m_TargetThread = 0;
	m_texBusy = 0;
	m_texFadeEffect = 0;
	m_texNet = 0;
	m_texOrder = 0;
	m_texServerHandoff = 0;
	m_texSpawnWait = 0;
	m_texWarpSides = 0;
	m_texWormholeSides = 0;
	m_texWormholeSpace = 0;

	m_RenderPaused = false;

	m_ViewButtonAt = AuViewButton::VB_Invalid;
	m_ViewButtonColumns = 0;
	for(int g=0; g<4; g++)
		m_ViewButtonGlow[g] = 0;
	m_ViewButtonNear = AuViewButton::VB_Invalid;
	m_ViewButtonPressed = AuViewButton::VB_Invalid;
	m_ViewButtonTex = 0;

	m_View1stPref = g_Au.Settings().readBoolEntry("Camera/1stPersonViewPref", true);
	m_ViewDeckRel = 0;

	m_RenderInfo->m_ViewAlt = g_Au.Settings().readNumEntry("Camera/TopDownAltitude", m_RenderInfo->m_ViewAlt);
	if(m_RenderInfo->m_ViewAlt < 75 || m_RenderInfo->m_ViewAlt > 2400)
		m_RenderInfo->m_ViewAlt = 300;

	m_RenderInfo->m_ViewAngle1st = g_Au.Settings().readNumEntry("Camera/1stPersonAngle", m_RenderInfo->m_ViewAngle1st*RAD2DEGf);
	if(m_RenderInfo->m_ViewAngle1st < 0 || m_RenderInfo->m_ViewAngle1st > 45)
		m_RenderInfo->m_ViewAngle1st = 0;
	m_RenderInfo->m_ViewAngle1st *= DEG2RADf;

	m_RenderInfo->m_ViewAngle3rd = g_Au.Settings().readNumEntry("Camera/3rdPersonAngle", m_RenderInfo->m_ViewAngle3rd*RAD2DEGf);
	if(m_RenderInfo->m_ViewAngle3rd < 0 || m_RenderInfo->m_ViewAngle3rd > 60)
		m_RenderInfo->m_ViewAngle3rd = 20;
	m_RenderInfo->m_ViewAngle3rd *= DEG2RADf;

	m_RenderInfo->m_ViewDist1st = g_Au.Settings().readNumEntry("Camera/1stPersonDistance", m_RenderInfo->m_ViewDist1st);
	if(m_RenderInfo->m_ViewDist1st < 0 || m_RenderInfo->m_ViewDist1st > 10)
		m_RenderInfo->m_ViewDist1st = 0;

	m_RenderInfo->m_ViewDist3rd = g_Au.Settings().readNumEntry("Camera/3rdPersonDistance", m_RenderInfo->m_ViewDist3rd);
	if(m_RenderInfo->m_ViewDist3rd < 1 || m_RenderInfo->m_ViewDist3rd > 20)
		m_RenderInfo->m_ViewDist3rd = 10;

	m_WormholeCenterOffset = 0;
	m_WormholeSpeed = 1;
	m_WormholeTunnel = 0;

	m_NumberOfMessages = g_Au.Settings().readNumEntry("OnScreenMessages/Total NumberOfMessages", 10);

	m_IsDesignMode = false;
	m_IsDesignPreview = false;
	m_bWaitingForScene = false;		// True when waiting for scene atom to init.

	OnViewResetCamera();

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

	connect(this, SIGNAL(DesignerGrabQueued()), this, SLOT(DesignerMove()), Qt::QueuedConnection);
	connect(this, SIGNAL(DesignerRotateQueued()), this, SLOT(DesignerRotate()), Qt::QueuedConnection);
	connect(this, SIGNAL(DesignerStretchQueued()), this, SLOT(DesignerStretch()), Qt::QueuedConnection);
}

AuDesignerView::~AuDesignerView()
{
	//qDebug() << "AuDesignerView::~AuView";
	m_SceneRendered = false;	// Keeps ProcessControls and UpdateTarget from doing anything if they called by the model or
								// control threads after this.

	StopThreads();

	// Save last known whereabouts of the scene atom for display on the login screen.
	// Does nothing if there is no scene atom.
	SaveWhereabouts();

	if(m_pCommand[0])
		delete m_pCommand[0];
	if(m_pCommand[1])
		delete m_pCommand[1];
	if(m_pControlsAtom)
		delete m_pControlsAtom;
	if(m_pControlsUI)
		delete m_pControlsUI;

	delete m_RenderInfo;
	delete m_ModelCache;
	delete m_MM;
}

void AuDesignerView::StopThreads()
{
	if(m_TargetThread)
	{
		DtTarget* tt = m_TargetThread;
		m_TargetThread = 0;
		//qDebug() << "AuDesignerView::StopThreads Killing target thread";
		tt->Kill();
		tt->wait(1000);
		//qDebug() << "AuDesignerView::StopThreads target thread running" << tt->isRunning();
	}
}

void AuDesignerView::initializeGL()
{
	//qDebug("AuDesignerView::initializeGL");

	//QSurfaceFormat fmt = format();
	//qDebug() << "AuDesignerView::initializeGL swapInt" << fmt.swapInterval() << "swapBehave" << fmt.swapBehavior()
		//<< "opaquePaint" << testAttribute(Qt::WA_OpaquePaintEvent);

	m_RenderInfo->InitializeGL();

	m_RenderInfo->re->glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_GLTextureUnits);
	// GL_MAX_UNIFORM_LOCATIONS is undefined on MacOS. 1024 is minimum per OpenGL. It is only used for info anyway, not needed.
#if !defined(Q_OS_MACOS)
	m_RenderInfo->re->glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &m_GLUniformLocs);
#endif

	// Init all lights. The arbvp1 shader profile fails if we don't do this.
	m_RenderInfo->re->SetAmbientLight(0,0,0,0);
	for(int isun=0; isun<MAX_SUNS; isun++)
		m_RenderInfo->re->SetSun(isun, 1, QColor(255,255,255), 0,0,0, 0);
	m_RenderInfo->re->DisableSuns();
	for(unsigned int ilight=0; ilight<m_RenderInfo->re->MaxLights(); ilight++)
		m_RenderInfo->re->SetLight(ilight, .2f,.03f, QColor(255,255,255), 0,0,0);
	m_RenderInfo->re->DisableLights();

	// Create textures for every industry.
	for(int iid=1; iid<IID_Count; iid++)
	{
		QString imgname(DbIndustry::BitmapName((INDUSTRYID)iid));
		if(imgname.isEmpty())
			continue;
		QImage bmiid(imgname);
		if(bmiid.isNull())
		{
			if(DbIndustry::BitmapName((INDUSTRYID)iid))
				qWarning("error: AuDesignerView::initializeGL failed to load industry bitmap %s", DbIndustry::BitmapName((INDUSTRYID)iid));
			continue;
		}
		QImage bmt(bmiid.width()+2, bmiid.height()+2, QImage::Format_ARGB32);
		if(bmt.isNull())
			continue;
		bmt.fill(Qt::transparent);

		QPainter p(&bmt);
		p.translate(0,bmiid.height()+2);		// The bitmap must be drawn up-side-down to appear right-side-up on sprites.
		p.scale(1,-1);
		p.drawImage(1,1, bmiid);
		p.end();

		//bmt.save(QString("%1.png").arg(iid));
		//qDebug("Created image of industry %d for %s", iid, DbIndustry::BitmapName((INDUSTRYID)iid));
		QString texName = QString("IID%1").arg(iid);
		m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, texName, bmt);
	}

	// Start the targeting thread.
	m_TargetThread = new DtTarget(this);
	m_TargetThread->start(QThread::LowPriority);

	// Get the rendering going. Doing nothing at all, or simply calling update() at this point, leaves some people with a blank
	// non-animating scene until they toggle full screen vs windowed mode.
	if(!m_KickStarter)
	{
		m_KickStarter = new QTimer(this);
		connect(m_KickStarter, SIGNAL(timeout()), this, SLOT(AnimateScene()));
	}
	m_KickStarter->start(1000);

	connect(this, SIGNAL(RequestEndCommand()), SLOT(EndCurrentCommand()), Qt::QueuedConnection);
	connect(this, SIGNAL(RequestScreenShot()), SLOT(ScreenShot()), Qt::QueuedConnection);

	setMouseTracking(true);

	CreateViewButtons();

	if(!m_SpawnWaitVAO.VAO())
	{
		// Create VAO of spawn wait indicator.
		double rad = 1;
		float s = 0;
		float t = 0;
		float u = 1;
		float v = 1;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-rad,-rad,0, s,t);
		tri0.m_V1 = AuPointTex(-rad,rad,0, s,v);
		tri0.m_V2 = AuPointTex(rad,rad,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(rad,rad,0, u,v);
		tri1.m_V1 = AuPointTex(rad,-rad,0, u,t);
		tri1.m_V2 = AuPointTex(-rad,-rad,0, s,t);

		QList<AuTriTex> tris;
		tris << tri0 << tri1;
		m_SpawnWaitVAO.CreateForTexture(*m_RenderInfo->re, tris, GL_STATIC_DRAW);
	}

	// Create static textures used by the view itself.
	if(!m_texBusy)
		m_texBusy = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "BusyThread", ":data/Busy.png");

	if(!m_texFadeEffect)
		m_texFadeEffect = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "FadeEffect", ":data/FadeEffect.png");

	if(!m_texNet)
		m_texNet = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "NetActivity", ":data/NetActivity.png",
				GL_LINEAR, GL_LINEAR, false, 2);

	if(!m_texOrder)
		m_texOrder = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "GoToOrder", ":data/GoToSpot.png");

	if(!m_texServerHandoff)
		m_texServerHandoff = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "WaitInd", ":data/ServerHandoff.png");

	if(!m_texSpawnWait)
		m_texSpawnWait = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "SpawnWait", ":data/SpawnWait.png");

	if(!m_texWarpSides)
		m_texWarpSides = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "WarpSides",
				":data/EffWarp1.png", GL_LINEAR, GL_LINEAR, true);

	if(!m_texWormholeSides)
		m_texWormholeSides = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "WormholeSides",
				":data/TerNoise.png", GL_LINEAR, GL_LINEAR, true);

	if(!m_texWormholeSpace)
		m_texWormholeSpace = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "WormholeSpace",
				":data/WormholeSpace.png", GL_LINEAR, GL_LINEAR, true);

	// Create thread to do the rendering work.
	if(false && context()->supportsThreadedOpenGL())
	{
		m_GLThread = new DtGL(this);

		// Some GL implementations require that the currently bound context is made non-current before we set up sharing,
		// so we doneCurrent here and makeCurrent down below while setting up our own context.
		doneCurrent();
		m_GLThread->context = new QOpenGLContext();
		m_GLThread->context->setFormat(context()->format());
		m_GLThread->context->setShareContext(context());
		m_GLThread->context->create();
		m_GLThread->context->moveToThread(m_GLThread);
		makeCurrent();

		m_GLThread->surface = new QOffscreenSurface();
		m_GLThread->surface->setFormat(m_GLThread->context->format());
		m_GLThread->surface->create();
		m_GLThread->surface->moveToThread(m_GLThread);

		m_GLThread->start();
	}
}

void AuDesignerView::paintGL()
{
	bool showWait = m_bWaitingForScene || m_SceneAtomTransition || !auApp->IsSceneConnected();

	//qDebug() << "AuDesignerView::paintGL";
	RenderScene();

	if(showWait)
	{
		//qDebug() << "AuDesignerView::paintGL waitingForScene" << m_bWaitingForScene << "inTransition" << m_SceneAtomTransition
			//<< "sceneConnected" << auApp->IsSceneConnected();
		//if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			//g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("View: Wait, connected %1, sceneAtom %2, in transition %3")
					//.arg(auApp->IsSceneConnected()?"yes":"no").arg(sceneAtom?"yes":"no")
					//.arg(sceneAtom&&sceneAtom->IsInTransition()?"yes":"no"));

		// Do not hold onto the mouse when waiting.
		ReleaseMouse();
		if(m_IsWarping)
			ShowWarpIndicator();
		else
			ShowWormholeIndicator();

		// Reset full fade so we fade-in after showing the wait indicator.
		m_Fade = 0;
	}
	else
	{
		//qDebug() << "AuDesignerView::paintGL";
		m_WormholeSpeed = 1;
		if(m_RenderInfo->mm.IsBusy())
			ShowBusyIndicator();

		RenderWindowSize();

		RenderControlButtons();
	}

	ShowNetActivity();

	RenderViewButtons();

	// Before this was done by AuModel.  ProcessControls();

	// Repaint the display.
	update();
	//qDebug() << "AuDesignerView::paintGL done";
}

void AuDesignerView::RenderScene()
{
	if(m_RenderPaused)
		return;
	SyncScene();									// Sync in this thread, with current gl context.
	AnimateScene();									// Position everybody for the snapshot and reset the scene animate time.
	DaPerson* sceneAtom;
	if(StartScene(&sceneAtom))						// Prepare devices for scene rendering and lock the scene atom.
	{
		// We have a scene atom.
		CreateDeviceObjects(sceneAtom);				// Tell atoms to create scene data they need.
		RenderScene(sceneAtom);						// Render the objects in the scene.
		FinishScene();								// Unlock the scene atom.
		ShowOrderLoc();
	}
}

void AuDesignerView::resizeEvent(QResizeEvent* event)
{
	//qDebug() << "AuDesignerView::resizeEvent start";
	PauseRendering();
	m_SizeChanged.restart();

	QOpenGLWidget::resizeEvent(event);
	//qDebug() << "AuDesignerView::resizeEvent end";
}

void AuDesignerView::PauseRendering()
{
	//qDebug() << "AuDesignerView::PauseRendering";
	m_RenderPaused = true;
#if defined(Q_OS_MACOS)
	QTimer::singleShot(1000, [=](){ResumeRendering();});
#else
	QTimer::singleShot(0, [=](){ResumeRendering();});
#endif
}

void AuDesignerView::ResumeRendering()
{
	//qDebug() << "AuDesignerView::ResumeRendering";
	m_RenderPaused = false;
	update();
}

// emits the SceneContextChanged signal if the view context actually changes.
void AuDesignerView::SetSceneContext(SCENE_CONTEXT sceneContext)
{
	if(m_SceneContext != sceneContext)
	{
		//qDebug() << "AuDesignerView::SetSceneContext changed from" << m_SceneContext << "to" << sceneContext;
		m_SceneContext = sceneContext;
		emit SceneContextChanged(m_SceneContext);
	}
}

void AuDesignerView::AnimateSceneAtoms(DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	if(!s_Animate)
		return;

	if(!sceneAtom)
		return;

	DaAtom* envAtom = sceneAtom->FindEnvironmentAtom();
	if(!envAtom)
		return;

	envAtom->AnimateDeviceObjects(*m_RenderInfo, sceneAtom, lastAnim);
}

void AuDesignerView::AnimateScene()
{
	m_SceneEther.DeleteTrashAtom();		// Delete a trash atom from my scene ether. There is no separate trash thread to do it.

	// Track elapsed time between frames. This is the actual time it took to perform one frame render and animate cycle.
	AuElapsedTimer lastAnim(m_Animated);
	m_Animated.restart();

	if(m_ViewButtonAt != AuViewButton::VB_Invalid && m_ViewButtonEntered.isValid())
	{
		if(m_ViewButtonEntered.elapsed() > 1000)
		{
			// Locate the button to get the tooltip text.
			m_ViewButtonEntered = AuElapsedTimer();
			for(QList<AuViewButton>::const_iterator bi = m_ViewButtons.cbegin(); bi != m_ViewButtons.cend(); bi++)
				if((*bi).m_Id == m_ViewButtonAt)
				{
					QToolTip::showText(mapToGlobal(m_MouseLast), (*bi).Tip());
					break;
				}
		}
	}

	if(m_bWaitingForScene)
	{
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("View: Waiting for %1...").arg(m_idWaitScene.StringId()));
		//qDebug("AuDesignerView::AnimateScene waiting");

		// Check to see if the scene atom has arrived.
		if(!m_idWaitScene.IsNull() && !m_SceneEther.FindAtom(m_idWaitScene))
		{
			//qDebug("AuDesignerView::AnimateScene - waiting for scene atom...still waiting");
			return;
		}

		// The scene atom is now present so we can finally change the scene.
		//qDebug("AuDesignerView::AnimateScene - waiting for scene atom...SCENE "
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

		if(!m_idWaitScene.IsNull() && m_SceneEther.FindAtom(m_idWaitScene))
			SetSceneAtom(m_idWaitScene);	// The scene atom is now ready to render the scene so change the scene.
	}

	// Track mouse movements.
	if(!m_MouseGrabbed)
	{
		if(IsCharacterInputMode())
		{
			if(underMouse())
				ActivateWindow();
			else
				m_bFreeMouse = true;
		}
		else if(Command())
		{
			// In UI input mode with a command running.
			// Keep my window on top and active whenever the mouse is over it so the command will get key commands.
			if(underMouse())
				ActivateWindow();
			else if(!m_HandInputTemporary)
				m_bFreeMouse = true;
		}
	}

	// GL context is already current.
	bool fadeToDeath = false;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);

	// Insure the appropriate cursor is set.
	SetCursor(sceneAtom);

	// Animate scene atom device objects, e.g. console textures.
	if(sceneAtom)
	{
		//qDebug() << "animate scene atom" << sceneAtom->Id() << sceneAtom->Name()
			//<< "inDesign" << sceneAtom->InDesignEnvironment();
		if(!m_GLThread)
			AnimateSceneAtoms(sceneAtom, lastAnim);

		if(sceneAtom->IsBodyDyingOrDead())
		{
			int tilDead = sceneAtom->CurrentHitPoints() - DpBody::DeadHitPoints();
			if(!sceneAtom->IsBodyDead() && tilDead > 10)
				fadeToDeath = true;
		}

		bool inde = sceneAtom->InDesignEnvironment();
		bool ina = sceneAtom->InArena();

		UnlockSceneAtom();

		// Notify the frame if our scene context changed.
		if(inde)
			SetSceneContext(SCENE_Designer);
		else if(ina)
			SetSceneContext(SCENE_Arena);
		else
			SetSceneContext(SCENE_Universe);
	}
	else
	{
		UnlockSceneAtom();
		SetSceneContext(SCENE_Unknown);
	}

	if(m_bFreeCamera)
	{
		// This is a reasonably smooth increment in which to respond and to animate the camera.
		m_tCamera.Translate(0.0, 0.0, -m_fCameraSpeed);
		m_tSceneCamera = m_tCamera;
	}
	else
	{
		// When the camera is not free, insure it remains in sync with the scene atom.
		if(!m_idSceneAtom.IsNull())
			ResetCamera();

		if(m_tCamera != m_tSceneCamera)
		{
			if(m_tCamera.IsIdentity())
				m_tCamera = m_tSceneCamera;
			else
			{
				// Smoothly transition from old camera to new camera.
				AuPoint oldLoc = m_tCamera.Translation();
				AuPoint newLoc = m_tSceneCamera.Translation();
				AuPoint moveVec = newLoc - oldLoc;
				double distToMove = moveVec.Length();				// Distance in feet.
				const double distAtSpeed = m_CameraAnimSpeed * lastAnim.elapsed()/1000.;	// Feet moved during time elapsed
				if(distAtSpeed >= distToMove || IsFuzzyZero(distToMove))
					m_tCamera = m_tSceneCamera;
				else
				{
					// Interpolate move and rotation based on time elapsed.
					double frac = distAtSpeed / distToMove;			// Fraction of distance traversed.
					newLoc = oldLoc + moveVec * frac;
					AuPoint oldRot = m_tCamera.GetRotation();
					AuPoint newRot = m_tSceneCamera.GetRotation();
					AuPoint rotVec = newRot - oldRot;
					newRot = oldRot + rotVec * frac;
					m_tCamera.LoadIdentity();
					m_tCamera.Translate(newLoc);
					m_tCamera.Rotate(newRot);
				}
			}
		}
	}

	if(!m_idSceneAtom.IsNull())
	{
		// Decay comm messages.
		m_MessageLock.lock();
		if(!m_Messages.empty() && !m_Animated.SameStartSec(m_MessageDecay))
		{
			m_MessageDecay = m_Animated;
			QVector<AuRenderMsg>::iterator mi = m_Messages.begin();
			while(mi != m_Messages.end())
			{
				if((*mi).DecayMessage())
				{
					if(AuCommPacket::IsNarratorMessage((*mi).Type()))
					{
						// When decaying narrative, we change the text to blank and the type to a regular chat message.
						// This causes a little shift as the new block appears at the top, one blank line low, and then scrolls up
						// a few seconds later. It catches the eye better than having the text simply blip to something new, which
						// easily goes unnoticed.
						(*mi).SetText(" ");
						(*mi).SetType(AuCommPacket::Chat);
						mi++;
					}
					else
					{
						// Remove the message and its associated time.
						mi = m_Messages.erase(mi);
					}
				}
				else
				{
					// Keep the message and move to the next.
					mi++;
				}
			}
		}
		m_MessageLock.unlock();
	}

	// Decay render text items that have expired.
	m_RenderInfo->re->DecayRenderText();

	if(m_KickStarter)
	{
		// The kick start timer is no longer needed now that we're rendering. The next call to update() will trigger the next
		// paintGL when vsync is finished and we're off.
		m_KickStarter->stop();
		m_KickStarter->deleteLater();
		m_KickStarter = 0;
	}

	// Track ctrl key state for input to atom.
	INPUTMODE inputMode = AssessInputMode();
	if(inputMode != m_InputMode)
	{
		// Input mode changed.
		if(inputMode == Character)
		{
			// Put cursor in center of the screen when input is to atom.
			// Doing so here prevents a lurch that is otherwise caused when we think the mouse has moved a bunch.
			QRect rc = rect();
			QPoint pmid;
			pmid.setX((rc.left() + rc.right())/2);
			pmid.setY((rc.top() + rc.bottom())/2);
			QPoint pgmid = mapToGlobal(pmid);
			m_ControlLock.lock();
			QCursor::setPos(pgmid);
			m_MouseVecAtom = QPoint(0,0);	// Init movement vector for control input.
			//m_MousePos = pgmid;
			m_MousePosLast = pgmid;
			m_ControlLock.unlock();
			//qDebug() << "AuDesignerView::AnimateScene zero vec" << m_MouseVecAtom;
		}
		m_InputMode = inputMode;
		m_InputModeChanged.restart();
	}

	if(m_Fade < 1 && !fadeToDeath)
	{
		// Fade to full brightness within 3 seconds.
		float fadeTime = 3.f;
		m_Fade += ((float)lastAnim.elapsed()/1000.f) / fadeTime;
		if(m_Fade > 1)
			m_Fade = 1;
	}

	// Render narrative audio.
	auApp->PlayQueuedSounds(g_Au.SoundEffectsGain() * m_Fade);
}

void AuDesignerView::focusInEvent(QFocusEvent* e)
{
	//qDebug() << "AuDesignerView::focusInEvent";
	QOpenGLWidget::focusInEvent(e);

	// When we regain focus we no longer know the true state of all keys.
	// We don't do this when we lose focus or the Tab key is lost as it seems to generate a focusOutEvent straight away.
	m_ControlLock.lock();
	m_ActiveKeys.clear();
	m_AtomKeys.clear();
	m_ControlLock.unlock();

	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	if(pFrame)
		pFrame->ClearActiveKeys();
}

void AuDesignerView::focusOutEvent(QFocusEvent* e)
{
	//qDebug() << "AuDesignerView::focusOutEvent";
	QOpenGLWidget::focusOutEvent(e);

	ReleaseMouse();
}

void AuDesignerView::TabKeyPressed(Qt::KeyboardModifiers mods)
{
	m_ControlLock.lock();
	m_ActiveKeys.insert(pair<Qt::Key, AuElapsedTimer>(Qt::Key_Tab, AuElapsedTimer()));	// Insert a null time value when the key first goes down.
	m_ActiveKeyMods = mods;
	m_AtomKeys.insert(pair<Qt::Key, AuElapsedTimer>(Qt::Key_Tab, AuElapsedTimer()));	// Insert a null time value when the key first goes down.
	m_AtomKeyMods = m_ActiveKeyMods;
	m_ControlLock.unlock();
}

void AuDesignerView::keyPressEvent(QKeyEvent *e)
{
	//qDebug() << "AuDesignerView::keyPressEvent" << e->key() << e->isAutoRepeat() << "UI" << IsUIInputMode()
		//<< "char" << IsCharacterInputMode();
	QOpenGLWidget::keyPressEvent(e);

	// Pass the key event to the current command in case it wants to completely absorb it. e.g. Backspace, Return, Enter, coords.
	if(IsUIInputMode())
	{
		if(Command() && Command()->OnKeyDown(e))
			return;
	}

	// Ignore repeats of the same key. The autorepeat bool is on when we get the initial key press event for the key.
	if(e->isAutoRepeat())
		return;

	/* Sound Test
	if(e->key() == Qt::Key_Minus)
	{
		QString fileName("/home/daveh/dev/au/stories/3/welcome2.wav");
		QByteArray soundData;
		QFile soundFile(fileName);
		if(soundFile.open(QIODevice::ReadOnly))
		{
			// Successfully opened the file. Read the audio data.
			soundData = soundFile.readAll();
			soundFile.close();
			QueueSound(soundData);
		}
		else
			qWarning() << "error: AuDesignerView::keyPressEvent failed to open sound file:" << fileName;
	}
	*/

	// Track keys down.
	m_ControlLock.lock();
	m_ActiveKeys.insert(pair<Qt::Key, AuElapsedTimer>((Qt::Key)e->key(), AuElapsedTimer()));	// Insert null time value when the key first goes down.
	m_ActiveKeyMods = e->modifiers();
	m_AtomKeys.insert(pair<Qt::Key, AuElapsedTimer>((Qt::Key)e->key(), AuElapsedTimer()));	// Insert null time value when the key first goes down.
	m_AtomKeyMods = m_ActiveKeyMods;
	m_ControlLock.unlock();
}

void AuDesignerView::keyReleaseEvent(QKeyEvent *e)
{
	//qDebug() << "AuDesignerView::keyReleaseEvent" << e->key() << e->isAutoRepeat();
	QOpenGLWidget::keyReleaseEvent(e);

	// Pass the key event to the current command in case it wants to completely absorb it. e.g. Backspace, Return, Enter, coords.
	bool handled = false;
	if(Command() && IsUIInputMode())
		handled = Command()->OnKeyUp(e);

	// Ignore repeats. The autorepeat bool is off on the final release of the key.
	if(e->isAutoRepeat())
		return;

	// When the camera is free, it has its own keyboard interface.
	if(!handled)
		ProcessCameraKeyCommand(e);

	// Track keys that are currently down.
	m_ControlLock.lock();
	unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>::iterator ki = m_ActiveKeys.find((Qt::Key)e->key());
	if(ki != m_ActiveKeys.end())
		m_ActiveKeys.erase(ki);
	m_ActiveKeyMods = e->modifiers();
	m_ControlLock.unlock();
}

void AuDesignerView::mouseDoubleClickEvent(QMouseEvent* e)
{
	m_ControlLock.lock();
	m_ActiveMouseMods = e->modifiers();
	m_AtomMouseMods = m_ActiveMouseMods;
	m_ControlLock.unlock();

	// View and control button hits ignore all other inputs.
	if(m_ViewButtonPressed == AuViewButton::VB_Invalid && m_ControlButtonPressed == AuControlButton())
	{
		if(m_WormholeSpeed < 50.f)
			m_WormholeSpeed += 1000.f;
		else
			m_WormholeSpeed -= 1000.f;

		if(IsUIInputMode())
		{
			if(Command())
				Command()->OnButtonDblClk(e);
			else if(IsInNewDesignEnvironment())
				ProcessDesignerMouseDoubleClick(e);
		}
	}

	QOpenGLWidget::mouseDoubleClickEvent(e);

	//SetCursor();
}

void AuDesignerView::mousePressEvent(QMouseEvent* e)
{
#if !defined(Q_OS_MACOS)
	setFocus();
#endif

	m_ControlLock.lock();
	m_ActiveMouseMods = e->modifiers();
	m_AtomMouseMods = m_ActiveMouseMods;
	m_ControlLock.unlock();

	// View and control button hits ignore all other inputs.
	if(ViewButtonIsEnabled(m_ViewButtonAt))
		m_ViewButtonPressed = m_ViewButtonAt;
	else
		m_ViewButtonPressed = AuViewButton::VB_Invalid;
	m_ControlButtonPressed = m_ControlButtonAt;
	if(m_ViewButtonPressed == AuViewButton::VB_Invalid && m_ControlButtonPressed == AuControlButton())
	{
		ProcessAtomMousePress(e);
		ProcessUIMousePress(e);

		QPoint point = e->pos();
		m_MouseDragging = false;			// This will be set true if the mouse moves more than a few pixels.
		m_MouseLast = point;				// Last known location of the mouse pointer.
		m_MousePress = point;
		m_MousePressed = true;

		if(IsUIInputMode())
		{
			if(Command())
				Command()->OnButtonDown(e);
			else if(IsInNewDesignEnvironment())
				ProcessDesignerMousePress(e);
		}
	}

	QOpenGLWidget::mousePressEvent(e);

	//SetCursor();
}

void AuDesignerView::mouseReleaseEvent(QMouseEvent* e)
{
	m_ControlLock.lock();
	m_ActiveMouseMods = e->modifiers();
	m_ControlLock.unlock();

	// View button hit ignores all other inputs.
	if(m_ViewButtonPressed != AuViewButton::VB_Invalid)
	{
		if(m_ViewButtonPressed == m_ViewButtonAt)
			ViewButtonClicked(m_ViewButtonPressed);
		m_ViewButtonPressed = AuViewButton::VB_Invalid;
	}
	else if(m_ControlButtonPressed != AuControlButton())
	{
		if(m_ControlButtonPressed == m_ControlButtonAt)
			ControlButtonClicked(m_ControlButtonPressed);
		m_ControlButtonPressed = AuControlButton();
	}
	else
	{
		ProcessAtomMouseRelease(e);
		ProcessUIMouseRelease(e);
		m_MousePressed = false;

		if(IsUIInputMode())
		{
			if(Command())
				Command()->OnButtonUp(e);
			else if(IsInNewDesignEnvironment())
				ProcessDesignerMouseRelease(e);
		}

		m_MouseDragging = false;			// Mouse is no longer dragging.
	}

	QOpenGLWidget::mouseReleaseEvent(e);

	//SetCursor();
}

void AuDesignerView::mouseMoveEvent(QMouseEvent* e)
{
	QOpenGLWidget::mouseMoveEvent(e);

	m_ControlLock.lock();
	m_ActiveMouseMods = e->modifiers();
	//m_MousePos = e->globalPos();
	m_ControlLock.unlock();
	//if(IsCharacterInputMode() && !m_bFreeMouse)
		//qDebug() << "AuDesignerView::mouseMoveEvent mousePos" << m_MousePos;

	// View button hit ignores all other inputs.
	if(m_ViewButtonPressed == AuViewButton::VB_Invalid && m_ControlButtonPressed == AuControlButton())
	{
		// Not pressing an on-screen button.
		ProcessCameraMouseMove(e);

		m_MouseLast = e->pos();	// Last known location of the mouse pointer.

		// Mouse dragging is triggered if the mouse moves more than a few pixels.
		if(m_MousePressed && (labs(e->pos().x() - m_MousePress.x()) > 6 || labs(e->pos().y() - m_MousePress.y()) > 6))
			m_MouseDragging = true;

		if(IsUIInputMode())
		{
			if(Command())
				Command()->OnMouseMove(e);
			else if(IsInNewDesignEnvironment())
				ProcessDesignerMouseMove(e);
		}
	}
	else
	{
		//qDebug() << "AuDesignerView::mouseMoveEvent near" << m_ViewButtonNear << "at" << m_ViewButtonAt;
		m_MouseLast = e->pos();	// Last known location of the mouse pointer.
	}

	//SetCursor();
}

void AuDesignerView::wheelEvent(QWheelEvent *e)
{
	ProcessAtomWheelEvent(e);

#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
	int zDelta = e->angleDelta().y();
#else
	int zDelta = e->delta();
#endif

	// Adjust speed of wormhole animation, for fun.
	if(m_WormholeSpeed < 50.f)
	{
		if(zDelta < 0)
		{
			m_WormholeSpeed -= .25f;
			if(m_WormholeSpeed < -30.f)
				m_WormholeSpeed = -30.f;
		}
		else
		{
			m_WormholeSpeed += .25f;
			if(m_WormholeSpeed > 30.f)
				m_WormholeSpeed = 30.f;
		}
	}

	if(Command())
		Command()->OnMouseWheel(e);			// Running command gets wheel precedence over free camera, which is a debug aid.
	else if(m_bFreeCamera)
	{
		// Free camera is only available to programmers so it is not configurable.
		if(zDelta > 0)
		{
			if(e->modifiers()&Qt::ShiftModifier)
			{
				if(e->modifiers()&Qt::ControlModifier)
					ChangeCameraSpeed(100.0f);
				else
					ChangeCameraSpeed(0.1f);
			}
			else
			{
				bool bfast = false;
				if(e->modifiers() & Qt::ControlModifier)
					bfast = true;

				double dist = 1.0;
				if(bfast)
					dist *= 10.0;
	
				MoveForward(dist);
			}
		}
		else if (zDelta < 0)
		{
			if (e->modifiers()&Qt::ShiftModifier)
			{
				if(e->modifiers()&Qt::ControlModifier)
					ChangeCameraSpeed(-100.0f);
				else
					ChangeCameraSpeed(-0.1f);
			}
			else
			{
				bool bfast = false;
				if(e->modifiers() & Qt::ControlModifier)
					bfast = true;

				double dist = 1.0;
				if(bfast)
					dist *= 10.0;

				MoveForward(-dist);
			}
		}
	}

	QOpenGLWidget::wheelEvent(e);
}

void AuDesignerView::SetupProjection()
{
	double nearClip;
	if(m_RenderInfo->IsViewTopDown() && IsOnLLWorld())
		nearClip = m_RenderInfo->m_ViewAlt/50.f;			// Stop water from colliding with shoreline so bad.
	else
		nearClip = m_rNearClip;

	// Apply perspective transformation that accounts for aspect ratio of screen.
	double aspect = AuGLEffects::WindowAspectRatio(rect());	// aspect 1 = 90, aspect .5 = 180
	double fovy = AuGLEffects::WindowFieldOfViewY(aspect, g_Au.FieldOfViewY());
	m_tProjection.LoadIdentity();
	m_tProjection.Perspective(fovy, aspect, nearClip, m_rFarClip);
	m_tFarProjection.LoadIdentity();
	m_tFarProjection.Perspective(fovy, aspect, 32000, FeetPerSector());
	double fovx = fovy / aspect;
	g_Au.SetFieldOfViewXCos(cos(fovx));
}

void AuDesignerView::OnViewResetCamera() 
{
	SelectPreferredFirstThirdPersonView();
	ResetCamera();
	StopCamera();
	m_tCamera = m_tSceneCamera;
}

// Commands this window to use a particular atom as the scene.
// Triggers the model to begin acquiring the necessary data.
// It may take a while before the new scene atom is ready.
// While waiting, m_bWaitingForScene and m_idWaitScene are set.
// The new scene can be set up to look at a particular sub-atom.
// If 'idScene' cannot render a scene, the next highest parent atom that can render a scene is used.
void AuDesignerView::SetSceneAtom(const AuId& idScene)
{
	//qDebug() << "AuDesignerView::SetSceneAtom" << idScene;
	// Tell the app which atom we want as the scene atom.
	// It may take some time for my scene data to catch up.
	auApp->SetAvatarId(idScene);
	m_bWaitingForScene = false;
	m_idWaitScene = idScene;									// Remember the atoms we really want to look at.

	if(idScene.IsNull())
	{
		m_idSceneAtom = idScene;
		m_SceneRendered = false;
		DpConsole::RestoreAdjustments(m_idSceneAtom);			// Restore the scene atom's console screen adjustments.

		UpdateSceneInfo(0);

		//qDebug() << "AuDesignerView::SetSceneAtom emit SceneAtomChanged";
		emit SceneAtomChanged();
		return;
	}

	// Check if the scene atom has shown up in my local data yet.
	m_SceneLock.lockForRead();
	DaPerson* pSceneAtom = dynamic_cast<DaPerson*>(m_SceneEther.FindAtom(idScene));
	if(!pSceneAtom)
	{
		//qDebug("AuDesignerView::SetSceneAtom - NOT FOUND, waiting for scene...");
		m_SceneLock.unlock();
		m_bWaitingForScene = true;
		return;
	}

	SelectPreferredFirstThirdPersonView();
	m_bFreeCamera = false;										// Be sure the camera is not in free look mode.

	if(!pSceneAtom->Is(m_idSceneAtom))
	{
		// A new atom becomes the scene atom.
		m_idSceneAtom = pSceneAtom->Id();
		m_SceneRendered = false;
		DpConsole::RestoreAdjustments(m_idSceneAtom);		// Restore the scene atom's console screen adjustments.

		// Configure control inputs for new scene atom.
		m_ControlLock.lock();
		if(m_pControlsAtom)
			m_pControlsAtom->clear();
		else
			m_pControlsAtom = new DiControlInputArray;
		pSceneAtom->EnumControlInputs(*m_pControlsAtom);
		m_ControlLock.unlock();

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

	// Reset camera from the new scene atom's perspective.
	OnViewResetCamera();
}

void AuDesignerView::CreateDeviceObjects(DaPerson* sceneAtom)
{
	if(!s_Create)
		return;

	// Create environment device objects, which includes the scene atom.
	DaAtom *pEnvironment = sceneAtom->FindEnvironmentAtom();
	if(pEnvironment)
		pEnvironment->CreateDeviceObjects(*m_RenderInfo);
	else
		sceneAtom->CreateDeviceObjects(*m_RenderInfo);

	if(Command())
		Command()->CreateDeviceObjects(*m_RenderInfo->re);
}

/*
void AuDesignerView::DeleteDeviceObjects(DaAtom& root, bool staticToo)
{
	root.DeleteDeviceObjects(*m_RenderInfo);

	if(Command())
		Command()->DeleteDeviceObjects(*m_RenderInfo->re);

	// OpenAL audio sources
	m_RenderInfo->re->StopSound(m_AudioSource);				// Stop the audio source and delete it.
	while(!m_Sounds.isEmpty())
	{
		m_RenderInfo->re->DeleteSound(m_Sounds.first().AudioBuffer());	// Delete the OpenAL sound buffer.
		m_Sounds.pop_front();							// Erase the sound entry from the array.
	}

	// Delete all textures associated with the static user id.
	if(staticToo)
		m_RenderInfo->re->DeleteTextures(STATIC_TEXTURE_ID);
}
*/

bool AuDesignerView::StartScene(DaPerson** sceneAtom)
{
	LockSceneAtom(sceneAtom);
	if(!*sceneAtom || !isVisible())
	{
		// Release grab when we're not visible, e.g. screensaver kicks in.
		UnlockSceneAtom();		// Unlock scene atom.
		return false;
	}

	// Configure for solid rendered display.
	m_RenderInfo->re->glFrontFace(GL_CW);			// Front faces are those drawn clockwise.
	m_RenderInfo->re->glCullFace(GL_BACK);		// Cull back-facing faces.
	m_RenderInfo->re->glEnable(GL_CULL_FACE);		// Enable face culling.
	m_RenderInfo->re->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Enable depth calculations
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);

	// Clear the color and depth buffers. (Clears the background.)
	m_RenderInfo->re->glClearColor(0,0,0,1);
	m_RenderInfo->re->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return true;
}

void AuDesignerView::FinishScene()
{
	m_SceneRendered = true;
	UnlockSceneAtom();
}

void AuDesignerView::RenderScene(DaPerson* sceneAtom)
{
	// Setup scene projection matrices.
	SetupProjection();

	// Configure scene render info.
	m_RenderInfo->fade = m_Fade * g_Au.SoundEffectsGain();	// Fade overall sound volume with screen fade.
	m_RenderInfo->textBrightness = (float)m_MessageIntensity/3.f;
	if(sceneAtom->IsInCapsule())
		m_RenderInfo->viewDeck = m_ViewDeckRel;
	else
		m_RenderInfo->viewDeck = 0;
	m_RenderInfo->m_ShowPaths = g_Au.m_DebugSwitches & 0x00020000;

	DaAtom *pEnvironment = sceneAtom->FindEnvironmentAtom();
	if(pEnvironment && s_RenderEnv)
	{
		if(IsInNewDesignEnvironment())
		{
			m_RenderInfo->m_DesignerShowBacks = g_AuClient.DesignerShowBacks();
			m_RenderInfo->m_DesignerShowEdges = g_AuClient.DesignerShowEdges();
			m_RenderInfo->m_DesignerShowFaces = g_AuClient.DesignerShowFaces();
			m_RenderInfo->m_DesignerShowGround = g_AuClient.DesignerShowGround();
			m_RenderInfo->m_DesignerShowSkyBox = g_AuClient.DesignerShowSkyBox();

			// Insure design plane visibility settings are kept in sync with local settings.
			DaDesignPlane* plane = pEnvironment->FindDesignPlane(sceneAtom->Id());
			if(plane)
			{
				plane->m_GridOnBits = g_AuClient.DesignerShowGrid();
				plane->SetGridSpacing(g_AuClient.DesignerGridSpacingFt(), g_AuClient.DesignerGridSpacingOm(),
						g_AuClient.DesignerGridSpacingNoM());
				plane->m_GridRad = g_AuClient.DesignerGridRadius();
				plane->ChangePos(AuAtomPos(pEnvironment->Id(), DesignPlane()));
			}
		}

		pEnvironment->RenderEnvironment(m_tProjection, m_tFarProjection, m_tCamera.Inverse(), *m_RenderInfo, sceneAtom);
	}

	if(s_RenderScene)
	{
		AuMatrix4x4 tDevicePersp, tDeviceOrtho;
		GetPixelTransforms(tDevicePersp, tDeviceOrtho);

		m_TargetLock.lockForRead();
		AuId targetId(m_TargetId);
		int targetLevel = m_TargetLevel;
		AuPoint targetLoc(m_TargetLoc);
		unsigned int targetPart = m_TargetPart;
		POLITICAL_STANCE targetStance = m_TargetStance;
		m_TargetLock.unlock();
		DaAtom* targetAtom;
		if(!targetId.IsNull() && pEnvironment)
			targetAtom = pEnvironment->FindIdRecursive(targetId);
		else
			targetAtom = 0;

		// Omit some screen elements when viewing through a remote camera.
		if(!sceneAtom->RemoteCameraId())
			sceneAtom->RenderScene(m_tProjection, m_tFarProjection, m_tCamera.Inverse(), *m_RenderInfo, targetAtom, targetLevel,
					targetLoc, targetPart, targetStance, tDevicePersp, tDeviceOrtho, g_AuClient.HUDIntensity());

		if(!m_SceneAtomImgTime.isValid() || m_SceneAtomImgTime.elapsed() > 1000)
		{
			// The self portrait image is used mainly by the bio window. It only updates itself every 1000ms so I don't
			// need to update the image any more often than that.
			m_SceneImgLock.lockForWrite();
			m_SceneAtomImage = sceneAtom->SelfPortrait(*m_RenderInfo, QSize(512, 512), QColor(0,0,0,0));
			m_SceneImgLock.unlock();
			//m_SceneAtomImage.save(QString("minime%1-%2.png")
					//.arg(QTime::currentTime().minute())
					//.arg(QTime::currentTime().second()));
			m_SceneAtomImgTime.restart();
		}
	}

	// Add "cursor" objects to the scene.
	if(Command() && IsUIInputMode() && s_RenderCursor)
		Command()->RenderCursor(m_tProjection, m_tCamera.Inverse(), sceneAtom, m_MouseLast, *m_RenderInfo, m_MouseSnapped);

	// Render tactical elements, such as unit identifiers and top-down view stuff.
	if(s_RenderTactical && pEnvironment)
		RenderTactical(sceneAtom, pEnvironment);

	// Display the drag selection rectangle.
	if(m_MouseDragging && !Command() && IsUIInputMode() && !IsDesignMode() && !m_bFreeCamera)
	{
		if(IsInNewDesignEnvironment() && DragSelectedParts())
		{
			// Dragging selected elements. They handle their own cursor.
		}
		else
		{
			float r = 43.f/255.f;
			float g = 134.f/255.f;
			float b = 231.f/255.f;

			// Set up projection so we can draw in pixels.
			AuMatrix4x4 projection;
			projection.LoadIdentity();
			projection.Translate(-1,1,0);				// Offset to upper left origin.
			projection.Scale(2./width(),-2./height(),1);	// Invert Y. Scale to pixels.

			m_RenderInfo->re->GLSL().LineShader().Select();
			m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(projection);

			m_RenderInfo->re->glDepthMask(false);
			m_RenderInfo->re->glDisable(GL_DEPTH_TEST);
			m_RenderInfo->re->glEnable(GL_BLEND);
			m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			m_RenderInfo->re->glDisable(GL_CULL_FACE);	// Disable face culling so the order of vertices doesn't matter.
			m_RenderInfo->re->GLSL().LineShader().SetColor(r,g,b,.08f);

			// Compile a list of vertex locations.
			AuPointArray verts;
			verts.append(AuPoint(m_MousePress.x(), m_MousePress.y(), 0));
			verts.append(AuPoint(m_MousePress.x(), m_MouseLast.y(), 0));
			verts.append(AuPoint(m_MouseLast.x(), m_MouseLast.y(), 0));
			verts.append(AuPoint(m_MouseLast.x(), m_MouseLast.y(), 0));
			verts.append(AuPoint(m_MouseLast.x(), m_MousePress.y(), 0));

			// Make a VAO of the vertex locations.
			AuGLVAO vao;
			vao.CreateForLines(*m_RenderInfo->re, verts, true, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_TRIANGLES);
				vao.DeselectGL();
			}

			m_RenderInfo->re->glEnable(GL_CULL_FACE);		// Enable face culling.

			m_RenderInfo->re->GLSL().LineShader().SetColor(r,g,b,.4f);

			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			m_RenderInfo->re->glDisable(GL_BLEND);
			m_RenderInfo->re->glEnable(GL_DEPTH_TEST);
			m_RenderInfo->re->glDepthMask(true);

			m_RenderInfo->re->GLSL().LineShader().Deselect();
		}
	}

	if(s_RenderComm)
		RenderCommMessages();
	RenderOverlayText();

	// Screen fade effect.
	if(s_RenderFade)
	{
		bool isAwaitingSpawnRecall = false;
		bool didSpawnTie = false;
		m_IsLefty = sceneAtom->IsLeftHanded();
		if(sceneAtom->IsBodyDyingOrDead())
		{
			int tilDead = sceneAtom->CurrentHitPoints() - DpBody::DeadHitPoints();
			if(sceneAtom->IsBodyDead() || tilDead <= 10)
			{
				// Clarity comes in the last few seconds.
				m_Fade = 1;
			}
			else
			{
				m_Fade = 1.f - ((float)sceneAtom->CurrentHitPoints() - 10) / (float)DpBody::DeadHitPoints();
				if(m_Fade < 0)
					m_Fade = 0;
			}
		}
		else if(sceneAtom->IsAwaitingSpawnRecall())
		{
			isAwaitingSpawnRecall = true;
		}

		if(sceneAtom->DidSpawnTie())
		{
			sceneAtom->ClearSpawnTie();
			didSpawnTie = true;
		}

		if(m_Fade < 1)
		{
			// Paint the fade effect over the entire screen, before other
			// indicators such as network disconnect and exit.
			ShowScreenFadeEffect();
		}
		else if(isAwaitingSpawnRecall)
		{
			// Show recall home waiting indicator.
			ShowSpawnRecallWait();
		}
		else if(didSpawnTie)
		{
			m_SpawnTieEffect = 1;	// Full on to fade away from here.
		}
	}

	if(m_SpawnTieEffect)
	{
		ShowSpawnTieEffect();
		m_SpawnTieEffect -= .015f;
		if(m_SpawnTieEffect < 0)
			m_SpawnTieEffect = 0;
	}
	//if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		//g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("View: Done rendering the scene."));
}

// Display a visual cue of where the last go to order was clicked.
void AuDesignerView::ShowOrderLoc()
{
	if(!m_RenderInfo->re)
		return;
	if(m_OrderId.IsNull())
		return;
	if(!m_RenderInfo->m_ShowHUD)
		return;
	if(!m_OrderTime.isValid())
		return;
	float ms = m_OrderTime.elapsed();
	if(ms >= 2000)
	{
		m_OrderId = AuId(0);
		m_OrderTime = AuElapsedTimer();
		return;
	}
	float alpha = 1.f - ms / 2000.f;	// Fade out over a 2 sec span.

	float rad = 1;
	if(!m_OrderVAO.VAO())
	{
		// Create VAO of busy indicator.
		float s = 0;
		float t = 0;
		float u = 1;
		float v = 1;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-rad,-rad,0, s,t);
		tri0.m_V1 = AuPointTex(-rad,rad,0, s,v);
		tri0.m_V2 = AuPointTex(rad,rad,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(rad,rad,0, u,v);
		tri1.m_V1 = AuPointTex(rad,-rad,0, u,t);
		tri1.m_V2 = AuPointTex(-rad,-rad,0, s,t);

		QList<AuTriTex> tris;
		tris << tri0 << tri1;
		m_OrderVAO.CreateForTexture(*m_RenderInfo->re, tris, GL_STATIC_DRAW);
	}

	// Must get the environment location of the m_OrderLoc, which is currently relative to m_OrderId.
	bool hasOrderLoc = false;
	AuPoint orderLoc;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaAtom* env = sceneAtom->FindEnvironmentAtom();
		if(env)
		{
			DaAtom* ord = env->FindIdRecursive(m_OrderId);
			if(ord)
			{
				AuMatrix4x4 envM(ord->EnvTransform());
				orderLoc = envM.Project(m_OrderLoc);	// Order location relative to the environment.
				orderLoc = sceneAtom->EnvTransform().UnProject(orderLoc);	// Relative to scene atom.
				hasOrderLoc = true;
			}
		}
	}
	UnlockSceneAtom();
	if(!hasOrderLoc)
		return;

	// Model view projection.
	AuMatrix4x4 pmv = m_tProjection * m_tCamera.Inverse();
	pmv.Translate(orderLoc);

	// Set up for additive blending.
	m_RenderInfo->re->glEnable(GL_BLEND);
	m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	m_RenderInfo->re->glDisable(GL_DEPTH_TEST);

	m_RenderInfo->re->GLSL().TextureShader().Select();
	m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(pmv);
	m_RenderInfo->re->GLSL().TextureShader().SetTexture(m_texOrder);
	m_RenderInfo->re->GLSL().TextureShader().SetColor(1,1,1, alpha);

	m_OrderVAO.SelectGL();
	m_OrderVAO.DrawGL();
	m_OrderVAO.DeselectGL();

	m_RenderInfo->re->GLSL().TextureShader().Deselect();

	// Restore rendering states
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);
	m_RenderInfo->re->glDisable(GL_BLEND);
}

// Display a visual cue suggestive of wormhole travel.
void AuDesignerView::ShowWormholeIndicator()
{
	// Create initial sprites.
	while(m_Sprites.count() < 100)
		m_Sprites.append(AuSprite());

	m_RenderInfo->re->glDisable(GL_DEPTH_TEST);

	// Backdrop wormhole space.
	AuMatrix4x4 mv;
	float xy = 1.4142f;
	if(m_WormholeSpeed < 50)
		m_SpaceAngle += .5f;	// Rotate slowly.
	if(AuTime::CurrentHourIsOdd())
		mv.RotateZ(m_SpaceAngle*DEG2RAD);
	else
		mv.RotateZ(-m_SpaceAngle*DEG2RAD);

	m_RenderInfo->re->GLSL().TextureShader().Select();
	m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(mv);
	m_RenderInfo->re->GLSL().TextureShader().SetColor(.1f,.5f,.5f,1);
	m_RenderInfo->re->GLSL().TextureShader().SetTexture(m_texWormholeSides);

	QList<AuTriTex> tris;
	tris << AuTriTex(AuPointTex(-xy, -xy, 0, 0,0),
			AuPointTex(-xy, xy, 0, 0,1),
			AuPointTex(xy, xy, 0, 1,1));
	tris << AuTriTex(AuPointTex(xy, xy, 0, 1,1),
			AuPointTex(xy, -xy, 0, 1,0),
			AuPointTex(-xy, -xy, 0, 0,0));

	AuGLVAO vao;
	vao.CreateForTexture(*m_RenderInfo->re, tris, GL_STREAM_DRAW);
	if(vao.VAO())
	{
		vao.SelectGL();
		vao.DrawGL();
		vao.DeselectGL();
		vao.DeleteGL();
	}

	// Tunnel wall.
	double aspect = AuGLEffects::WindowAspectRatio(rect());	// aspect 1 = 90, aspect .5 = 180
	double fovy = AuGLEffects::WindowFieldOfViewY(aspect, g_Au.FieldOfViewY());
	AuMatrix4x4 projection;
	projection.Perspective(fovy, aspect, 1, 10000);

	// Rotate based on time.
	if(m_WormholeSpeed < 50)
	{
		float time = AuTime::CurrentMinCycle() * 60.f;	// 0 to 60 seconds
		float freq = .05f;		// cycles per second
		float periods = time * freq;
		float angle = periods * PIf * 2.f;
		float osc = cos(angle);
		m_WormholeCenterOffset = osc * 14;	// +/- 14 feet.
	}
	mv.Translate(0, m_WormholeCenterOffset, 0);
	m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(projection * mv);
	m_RenderInfo->re->GLSL().TextureShader().SetTexture(m_texWormholeSides);

	// Set texturing mode to Modulate polygon color for blending.
	m_RenderInfo->re->glEnable(GL_BLEND);
	m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// Additive blending for extra brightness.

	{
		// Move in my current direction.
		if(m_WormholeSpeed < 50)
		{
			float particleSpeed = 2.f + m_WormholeSpeed;	// Speed slightly slower than slowest particles (the little blue ones).
			m_WormholeTunnel += particleSpeed / 200;		// Length per pattern iteration. 10 iteration along the 2000' tube.
		}

		m_RenderInfo->re->GLSL().TextureShader().SetColor(.15f,.025f,1,1);

		tris.clear();

		AuPoint preb;											// Previous base point.
		AuPoint pret;											// Previous top point.
		float pbs = 0, pbt = 0;
		float pts = 0, ptt = 0;
		int sides=24;
		for(int rad=100; rad>=12; rad/=2)
		{
			for(int side=0; side<=sides; side++)
			{
				float frac = (float)side / (float)sides;
				float angle = PIf*2 * frac;					// Angle of this pass.
				AuPoint curb = AuPoint(rad, 0, -2000).RotateZ(angle);
				AuPoint curt = AuPoint(rad, 0, 0).RotateZ(angle);
				float cbs = m_WormholeTunnel+10;
				float cbt = frac*2;
				float cts = m_WormholeTunnel;
				float ctt = cbt;
				if(side)
				{
					// Not the first loop iteration so we have preb and pret points to make a quad.
					tris << AuTriTex(AuPointTex(preb, pbs,pbt),
							AuPointTex(curb, cbs,cbt),
							AuPointTex(curt, cts,ctt));
					tris << AuTriTex(AuPointTex(curt, cts,ctt),
							AuPointTex(pret, pts,ptt),
							AuPointTex(preb, pbs,pbt));
				}
				preb = curb;
				pret = curt;
				pbs = cbs;
				pbt = cbt;
				pts = cts;
				ptt = ctt;
			}
		}

		vao.CreateForTexture(*m_RenderInfo->re, tris, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL();
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	// Draw sprites.
	tris.clear();
	QVector<AuSprite>::iterator it = m_Sprites.begin();
	while(it != m_Sprites.end())
	{
		float size = ((*it).m_pic + 1.f)/24.f * 50;

		float ts = 32.f/256.f * (*it).m_pic;
		float tt = 0;
		float tu = 32.f/256.f * ((*it).m_pic+1);
		float tv = 32.f/48.f;

		float rotang = (*it).m_rot*DEG2RADf;
		AuPoint v0 = AuPoint((*it).m_xpos, (*it).m_ypos, (*it).m_zpos) + AuPoint(-size, -size).RotateZ(rotang);
		AuPoint v1 = AuPoint((*it).m_xpos, (*it).m_ypos, (*it).m_zpos) + AuPoint(-size, size).RotateZ(rotang);
		AuPoint v2 = AuPoint((*it).m_xpos, (*it).m_ypos, (*it).m_zpos) + AuPoint(size, size).RotateZ(rotang);
		AuPoint v3 = AuPoint((*it).m_xpos, (*it).m_ypos, (*it).m_zpos) + AuPoint(size, -size).RotateZ(rotang);

		tris << AuTriTex(AuPointTex(v0, ts,tt),
				AuPointTex(v1, ts,tv),
				AuPointTex(v2, tu,tv));
		tris << AuTriTex(AuPointTex(v2, tu,tv),
				AuPointTex(v3, tu,tt),
				AuPointTex(v0, ts,tt));

		// Move the sprite.
		if(m_WormholeSpeed < 50)
			(*it).Move(m_WormholeSpeed);
		it++;
	}

	// Now draw the wormhole line.
	float ts = 0;
	float tt = 32.f/48.f;
	float tu = 1;
	float tv = 1;
	tris << AuTriTex(AuPointTex(-1,-1,-2000, ts,tt),
			AuPointTex(1,1,-2000, ts,tv),
			AuPointTex(1,1,0, tu,tv));
	tris << AuTriTex(AuPointTex(1,1,0, tu,tv),
			AuPointTex(-1,-1,0, tu,tt),
			AuPointTex(-1,-1,-2000, ts,tt));

	vao.CreateForTexture(*m_RenderInfo->re, tris, GL_STREAM_DRAW);
	if(vao.VAO())
	{
		m_RenderInfo->re->glDisable(GL_CULL_FACE);		// Disable face culling.

		// Sprite effects with motion blur.
		m_RenderInfo->re->GLSL().TextureShader().SetTexture(m_texServerHandoff);
		m_RenderInfo->re->GLSL().TextureShader().SetColor(1,1,1,1);

		vao.SelectGL();
		vao.DrawGL();
		vao.DeselectGL();
		vao.DeleteGL();

		m_RenderInfo->re->glEnable(GL_CULL_FACE);		// Enable face culling.
	}

	m_RenderInfo->re->glDisable(GL_BLEND);
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);

	m_RenderInfo->re->GLSL().TextureShader().Deselect();

	// Render mouse thumb twiddling instructions.
	ShowMouseThumbTwiddleInstruction();
}

void AuDesignerView::ShowMouseThumbTwiddleInstruction()
{
	return;	// turned off
	float intFact = (float)m_MessageIntensity/3.f;

	QColor color(AuStyleSheet::HazeronGold(2));
	color.setRedF(color.redF() * intFact);
	color.setGreenF(color.greenF() * intFact);
	color.setBlueF(color.blueF() * intFact);

	QColor shadColor(AuStyleSheet::HazeronGold(0));
	shadColor.setRedF(shadColor.redF() * intFact);
	shadColor.setGreenF(shadColor.greenF() * intFact);
	shadColor.setBlueF(shadColor.blueF() * intFact);

	QColor highColor(AuStyleSheet::HazeronGold(5));
	highColor.setRedF(highColor.redF() * intFact);
	highColor.setGreenF(highColor.greenF() * intFact);
	highColor.setBlueF(highColor.blueF() * intFact);

	// This text is centered.
	int cpos = width()/2;
	QFontMetrics fm(g_AuClient.FontAtom());
	int vpos = height() - (fm.height() + 10);

	QString text;
	if(m_WormholeSpeed < 50)
		text = tr("Wheel | Double Click");
	else
		text = tr("Double Click");

	m_RenderInfo->re->glEnable(GL_BLEND);
	m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw the shadow side.
	m_RenderInfo->re->RenderText(cpos+1, vpos+1, text, g_AuClient.FontAtom(), shadColor, width(), height(), Qt::AlignCenter, 0);
	// Draw the highlight.
	m_RenderInfo->re->RenderText(cpos-1, vpos-1, text, g_AuClient.FontAtom(), highColor, width(), height(), Qt::AlignCenter, 0);
	// Draw the full colored text on top of the pile.
	m_RenderInfo->re->RenderText(cpos, vpos, text, g_AuClient.FontAtom(), color, width(), height(), Qt::AlignCenter, 0);

	m_RenderInfo->re->glDisable(GL_BLEND);
}

// Display a visual cue suggestive of warp travel.
void AuDesignerView::ShowWarpIndicator()
{
	m_RenderInfo->re->glDisable(GL_DEPTH_TEST);

	// Backdrop wormhole space.
	AuMatrix4x4 mv;
	//float xy = 1.4142f;
	//if(m_WormholeSpeed < 50)
		//m_SpaceAngle += .5f;	// Rotate slowly.
	//if(AuTime::CurrentHourIsOdd())
		//mv.RotateZ(m_SpaceAngle*DEG2RAD);
	//else
		//mv.RotateZ(-m_SpaceAngle*DEG2RAD);
	float xy=1;

	m_RenderInfo->re->GLSL().TextureShader().Select();
	m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(mv);
	m_RenderInfo->re->GLSL().TextureShader().SetColor(1,1,1,1);
	m_RenderInfo->re->GLSL().TextureShader().SetTexture(m_texWormholeSpace);

	QList<AuTriTex> tris;
	tris << AuTriTex(AuPointTex(-xy, -xy, 0, 0,0),
			AuPointTex(-xy, xy, 0, 0,1),
			AuPointTex(xy, xy, 0, 1,1));
	tris << AuTriTex(AuPointTex(xy, xy, 0, 1,1),
			AuPointTex(xy, -xy, 0, 1,0),
			AuPointTex(-xy, -xy, 0, 0,0));

	AuGLVAO vao;
	vao.CreateForTexture(*m_RenderInfo->re, tris, GL_STREAM_DRAW);
	if(vao.VAO())
	{
		vao.SelectGL();
		vao.DrawGL();
		vao.DeselectGL();
		vao.DeleteGL();
	}

	// Tunnel wall.
	double aspect = AuGLEffects::WindowAspectRatio(rect());	// aspect 1 = 90, aspect .5 = 180
	double fovy = AuGLEffects::WindowFieldOfViewY(aspect, g_Au.FieldOfViewY());
	AuMatrix4x4 projection;
	projection.Perspective(fovy, aspect, 1, 10000);

	m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(projection * mv);

	// Set texturing mode to Modulate polygon color for blending.
	m_RenderInfo->re->glEnable(GL_BLEND);
	m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// Additive blending for extra brightness.

	{
		// Move in my current direction.
		if(m_WormholeSpeed < 50)
		{
			float particleSpeed = 2.f + m_WormholeSpeed;	// Speed slightly slower than slowest particles (the little blue ones).
			m_WormholeTunnel += particleSpeed / 200;		// Length per pattern iteration. 10 iteration along the 2000' tube.
		}

		m_RenderInfo->re->GLSL().TextureShader().SetColor(.25f,.25f,1,1);

		tris.clear();

		AuPoint preb;											// Previous base point.
		AuPoint pret;											// Previous top point.
		float pbs = 0, pbt = 0;
		float pts = 0, ptt = 0;
		int sides=24;
		m_RenderInfo->re->GLSL().TextureShader().SetTexture(m_texWarpSides);
		for(int rad=400; rad>=50; rad/=2)
		{
			for(int side=0; side<=sides; side++)
			{
				float frac = (float)side / (float)sides;
				float angle = PIf*2 * frac;					// Angle of this pass.
				AuPoint curb = AuPoint(rad, 0, -5000).RotateZ(angle);
				AuPoint curt = AuPoint(rad, 0, 0).RotateZ(angle);
				float cbs = m_WormholeTunnel+10;
				float cbt = frac*2;
				float cts = m_WormholeTunnel;
				float ctt = cbt;
				if(side)
				{
					// Not the first loop iteration so we have preb and pret points to make a quad.
					tris << AuTriTex(AuPointTex(preb, pbs,pbt),
							AuPointTex(curb, cbs,cbt),
							AuPointTex(curt, cts,ctt));
					tris << AuTriTex(AuPointTex(curt, cts,ctt),
							AuPointTex(pret, pts,ptt),
							AuPointTex(preb, pbs,pbt));
				}
				preb = curb;
				pret = curt;
				pbs = cbs;
				pbt = cbt;
				pts = cts;
				ptt = ctt;
			}
		}

		vao.CreateForTexture(*m_RenderInfo->re, tris, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL();
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	m_RenderInfo->re->glDisable(GL_BLEND);
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);

	m_RenderInfo->re->GLSL().TextureShader().Deselect();

	// Render mouse thumb twiddling instructions.
	ShowMouseThumbTwiddleInstruction();
}

void AuDesignerView::ShowBusyIndicator()
{
	if(!g_AuClient.ShowThreads())
		return;
	if(!m_RenderInfo->re)
		return;
	if(!m_RenderInfo->m_ShowHUD)
		return;

	float rad = .025f;
	if(!m_BusyVAO.VAO())
	{
		// Create VAO of busy indicator.
		float s = 0;
		float t = 0;
		float u = .25;
		float v = 1;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-rad,-rad,0, s,t);
		tri0.m_V1 = AuPointTex(-rad,rad,0, s,v);
		tri0.m_V2 = AuPointTex(rad,rad,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(rad,rad,0, u,v);
		tri1.m_V1 = AuPointTex(rad,-rad,0, u,t);
		tri1.m_V2 = AuPointTex(-rad,-rad,0, s,t);

		QList<AuTriTex> tris;
		tris << tri0 << tri1;
		m_BusyVAO.CreateForTexture(*m_RenderInfo->re, tris, GL_STATIC_DRAW);
	}

	// Model view projection.
	AuMatrix4x4 model;
	model.Translate(.95, .925, 0);
	m_BusyAngle -= 1;
	while(m_BusyAngle < 360)
		m_BusyAngle += 360;
	model.RotateZ(m_BusyAngle*DEG2RAD);

	// Texture offset.
	int frame = (int)(m_BusyAngle/6) % 8;
	if(frame >= 4)
		frame = 7 - frame;
	float offset = frame * .25f;

	// Set up for additive blending.
	m_RenderInfo->re->glEnable(GL_BLEND);
	m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	m_RenderInfo->re->glDisable(GL_DEPTH_TEST);

	m_RenderInfo->re->GLSL().TexOffsetShader().Select();
	m_RenderInfo->re->GLSL().TexOffsetShader().SetProjectionModelView(model);
	m_RenderInfo->re->GLSL().TexOffsetShader().SetTexture(m_texBusy);
	m_RenderInfo->re->GLSL().TexOffsetShader().SetColor(1,1,1, 1);
	m_RenderInfo->re->GLSL().TexOffsetShader().SetDiscardAlpha(0);
	m_RenderInfo->re->GLSL().TexOffsetShader().SetTextureOffset(offset, 0);

	m_BusyVAO.SelectGL();
	m_BusyVAO.DrawGL();
	m_BusyVAO.DeselectGL();

	m_RenderInfo->re->GLSL().TexOffsetShader().Deselect();

	float gldAspect = AuGLEffects::WindowAspectRatio(rect());
	float txht = rad;
	m_RenderInfo->RenderNumber(m_RenderInfo->mm.BusyThreadCount(), (.95f + rad) / gldAspect, .925f - txht/2.f, txht, 0,0,
			242.f/255.f, 243.f/255.f, 224.f/255.f, .5f);

	// Restore rendering states
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);
	m_RenderInfo->re->glDisable(GL_BLEND);
}

void AuDesignerView::ShowNetActivity()
{
	if(!g_AuClient.ShowNet())
		return;
	if(!m_RenderInfo->m_ShowHUD)
		return;
	if(!m_RenderInfo->re)
		return;

	int goingUp1 = g_Au.Net1Uploading();
	int goingDown1 = g_Au.Net1Downloading();
	int goingUp2 = g_Au.Net2Uploading();
	int goingDown2 = g_Au.Net2Downloading();
	int goingUp3 = g_Au.Net3Uploading();
	int goingDown3 = g_Au.Net3Downloading();
	int goingUp4 = g_Au.Net4Uploading();
	int goingDown4 = g_Au.Net4Downloading();
	//goingUp1 = 100;
	//goingDown1 = 100;
	//goingUp2 = 100;
	//goingDown2 = 100;
	//goingUp3 = 100;
	//goingDown3 = 100;
	//goingUp4 = 100;
	//goingDown4 = 100;
	if(!goingUp1 && !goingDown1 &&
			!goingUp2 && !goingDown2 &&
			!goingUp3 && !goingDown3 &&
			!goingUp4 && !goingDown4)
		return;
	//qDebug() << "AuDesignerView::ShowNetActivity net1" << goingUp1 << goingDown1 << "net2" << goingUp2 << goingDown2;

	float rad = .025f;
	if(!m_NetVAO.VAO())
	{
		// Create VAO of net activity indicator.
		float s = 0;
		float t = 0;
		float u = 1;
		float v = 1;

		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-rad,-rad,0, s,t);
		tri0.m_V1 = AuPointTex(-rad,rad,0, s,v);
		tri0.m_V2 = AuPointTex(rad,rad,0, u,v);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(rad,rad,0, u,v);
		tri1.m_V1 = AuPointTex(rad,-rad,0, u,t);
		tri1.m_V2 = AuPointTex(-rad,-rad,0, s,t);

		QList<AuTriTex> tris;
		tris << tri0 << tri1;
		m_NetVAO.CreateForTexture(*m_RenderInfo->re, tris, GL_STATIC_DRAW);
	}

	// Set up for additive blending.
	m_RenderInfo->re->glEnable(GL_BLEND);
	m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	m_RenderInfo->re->glDisable(GL_DEPTH_TEST);

	m_RenderInfo->re->GLSL().TextureShader().Select();
	m_RenderInfo->re->GLSL().TextureShader().SetTexture(m_texNet);

	const int frames = 40;			// Frames of animation along the movement route of the sprite.
	const float alphaDropoff = 80;	// This number is based on the AuClientGlobal::SetNet functions resetting channel values to 100.
	const int netDecay = 4;			// How many ticks per frame we decay from the network SetNet function values.
	if(goingUp1)
	{
		g_Au.DecayNet1Uploading(netDecay);
		// Model view projection.
		m_Net1Up++;
		int frame = m_Net1Up%frames;
		float offset = frame * .1f/frames;
		AuMatrix4x4 model;
		model.Translate(.95, .925, 0);
		model.RotateZ((90-15)*DEG2RAD);
		model.Translate(cos(45*DEG2RAD)*offset,sin(45*DEG2RAD)*offset,0);
		m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(model);

		// Color and transparency.
		float alpha;
		if(goingUp1 < alphaDropoff)
			alpha = sinf(goingUp1 / alphaDropoff * PI/2);
		else
			alpha = 1;
		m_RenderInfo->re->GLSL().TextureShader().SetColor(128.f/255.f, 229.f/255.f, 255.f/255.f, alpha);

		m_NetVAO.SelectGL();
		m_NetVAO.DrawGL();
		m_NetVAO.DrawGL();
		m_NetVAO.DeselectGL();
	}
	else
		m_Net1Up = 0;

	if(goingDown1)
	{
		g_Au.DecayNet1Downloading(netDecay);
		// Model view projection.
		m_Net1Down++;
		int frame = m_Net1Down%frames;
		float offset = (frames - frame - 1) * .1f/frames;
		AuMatrix4x4 model;
		model.Translate(.95, .925, 0);
		model.RotateZ((90+15)*DEG2RAD);
		model.Translate(cos(45*DEG2RAD)*offset,sin(45*DEG2RAD)*offset,0);
		model.RotateZ(PI);
		m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(model);

		// Color and transparency.
		float alpha;
		if(goingDown1 < alphaDropoff)
			alpha = sinf(goingDown1 / alphaDropoff * PI/2);
		else
			alpha = 1;
		m_RenderInfo->re->GLSL().TextureShader().SetColor(255.f/255.f, 128.f/255.f, 128.f/255.f, alpha);

		m_NetVAO.SelectGL();
		m_NetVAO.DrawGL();
		m_NetVAO.DrawGL();
		m_NetVAO.DeselectGL();
	}
	else
		m_Net1Down = 0;

	if(goingUp2)
	{
		g_Au.DecayNet2Uploading(netDecay);
		// Model view projection.
		m_Net2Up++;
		int frame = m_Net2Up%frames;
		float offset = frame * .1f/frames;
		AuMatrix4x4 model;
		model.Translate(.95, .925, 0);
		model.RotateZ(15*DEG2RAD);
		model.Translate(cos(45*DEG2RAD)*offset,sin(45*DEG2RAD)*offset,0);
		m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(model);

		// Color and transparency.
		float alpha;
		if(goingUp2 < alphaDropoff)
			alpha = sinf(goingUp2 / alphaDropoff * PI/2);
		else
			alpha = 1;
		m_RenderInfo->re->GLSL().TextureShader().SetColor(128.f/255.f, 229.f/255.f, 255.f/255.f, alpha);

		m_NetVAO.SelectGL();
		m_NetVAO.DrawGL();
		m_NetVAO.DrawGL();
		m_NetVAO.DeselectGL();
	}
	else
		m_Net2Up = 0;

	if(goingDown2)
	{
		g_Au.DecayNet2Downloading(netDecay);
		// Model view projection.
		m_Net2Down++;
		int frame = m_Net2Down%frames;
		float offset = (frames - frame - 1) * .1f/frames;
		AuMatrix4x4 model;
		model.Translate(.95, .925, 0);
		model.RotateZ(-15*DEG2RAD);
		model.Translate(cos(45*DEG2RAD)*offset,sin(45*DEG2RAD)*offset,0);
		model.RotateZ(PI);
		m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(model);

		// Color and transparency.
		float alpha;
		if(goingDown2 < alphaDropoff)
			alpha = sinf(goingDown2 / alphaDropoff * PI/2);
		else
			alpha = 1;
		m_RenderInfo->re->GLSL().TextureShader().SetColor(255.f/255.f, 128.f/255.f, 128.f/255.f, alpha);

		m_NetVAO.SelectGL();
		m_NetVAO.DrawGL();
		m_NetVAO.DrawGL();
		m_NetVAO.DeselectGL();
	}
	else
		m_Net2Down = 0;

	if(goingUp3)
	{
		g_Au.DecayNet3Uploading(netDecay);
		// Model view projection.
		m_Net3Up++;
		int frame = m_Net3Up%frames;
		float offset = frame * .1f/frames;
		AuMatrix4x4 model;
		model.Translate(.95, .925, 0);
		model.RotateZ((180+15)*DEG2RAD);
		model.Translate(cos(45*DEG2RAD)*offset,sin(45*DEG2RAD)*offset,0);
		m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(model);

		// Color and transparency.
		float alpha;
		if(goingUp3 < alphaDropoff)
			alpha = sinf(goingUp3 / alphaDropoff * PI/2);
		else
			alpha = 1;
		m_RenderInfo->re->GLSL().TextureShader().SetColor(128.f/255.f, 229.f/255.f, 255.f/255.f, alpha);

		m_NetVAO.SelectGL();
		m_NetVAO.DrawGL();
		m_NetVAO.DrawGL();
		m_NetVAO.DeselectGL();
	}
	else
		m_Net3Up = 0;

	if(goingDown3)
	{
		g_Au.DecayNet3Downloading(netDecay);
		// Model view projection.
		m_Net3Down++;
		int frame = m_Net3Down%frames;
		float offset = (frames - frame - 1) * .1f/frames;
		AuMatrix4x4 model;
		model.Translate(.95, .925, 0);
		model.RotateZ((180-15)*DEG2RAD);
		model.Translate(cos(45*DEG2RAD)*offset,sin(45*DEG2RAD)*offset,0);
		model.RotateZ(PI);
		m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(model);

		// Color and transparency.
		float alpha;
		if(goingDown3 < alphaDropoff)
			alpha = sinf(goingDown3 / alphaDropoff * PI/2);
		else
			alpha = 1;
		m_RenderInfo->re->GLSL().TextureShader().SetColor(255.f/255.f, 128.f/255.f, 128.f/255.f, alpha);

		m_NetVAO.SelectGL();
		m_NetVAO.DrawGL();
		m_NetVAO.DrawGL();
		m_NetVAO.DeselectGL();
	}
	else
		m_Net3Down = 0;

	if(goingUp4)
	{
		g_Au.DecayNet4Uploading(netDecay);
		// Model view projection.
		m_Net4Up++;
		int frame = m_Net4Up%frames;
		float offset = frame * .1f/frames;
		AuMatrix4x4 model;
		model.Translate(.95, .925, 0);
		model.RotateZ((-90-15)*DEG2RAD);
		model.Translate(cos(45*DEG2RAD)*offset,sin(45*DEG2RAD)*offset,0);
		m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(model);

		// Color and transparency.
		float alpha;
		if(goingUp4 < alphaDropoff)
			alpha = sinf(goingUp4 / alphaDropoff * PI/2);
		else
			alpha = 1;
		m_RenderInfo->re->GLSL().TextureShader().SetColor(128.f/255.f, 229.f/255.f, 255.f/255.f, alpha);

		m_NetVAO.SelectGL();
		m_NetVAO.DrawGL();
		m_NetVAO.DrawGL();
		m_NetVAO.DeselectGL();
	}
	else
		m_Net4Up = 0;

	if(goingDown4)
	{
		g_Au.DecayNet4Downloading(netDecay);
		// Model view projection.
		m_Net4Down++;
		int frame = m_Net4Down%frames;
		float offset = (frames - frame - 1) * .1f/frames;
		AuMatrix4x4 model;
		model.Translate(.95, .925, 0);
		model.RotateZ((-90+15)*DEG2RAD);
		model.Translate(cos(45*DEG2RAD)*offset,sin(45*DEG2RAD)*offset,0);
		model.RotateZ(PI);
		m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(model);

		// Color and transparency.
		float alpha;
		if(goingDown4 < alphaDropoff)
			alpha = sinf(goingDown4 / alphaDropoff * PI/2);
		else
			alpha = 1;
		m_RenderInfo->re->GLSL().TextureShader().SetColor(255.f/255.f, 128.f/255.f, 128.f/255.f, alpha);

		m_NetVAO.SelectGL();
		m_NetVAO.DrawGL();
		m_NetVAO.DrawGL();
		m_NetVAO.DeselectGL();
	}
	else
		m_Net4Down = 0;

	m_RenderInfo->re->GLSL().TextureShader().Deselect();

	// Restore rendering states
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);
	m_RenderInfo->re->glDisable(GL_BLEND);
}

void AuDesignerView::ShowScreenFadeEffect()
{
	//qDebug() << "AuDesignerView::ShowScreenFadeEffect" << m_Fade;

	// Rotate/position based on time.
	float time = AuTime::CurrentMinCycle() * 60.f;	// 0 to 60 seconds
	float freq = .25f;		// cycles per second
	float periods = time * freq;
	float angle = periods * PIf * 2.f;
	if(AuTime::CurrentHourIsOdd())
		angle = -angle;
	const float fadeOut = 1.f - m_Fade;	// 1 = full fade out, 0 = no fade out
	const float xoff = cos(angle) * fadeOut;
	const float yoff = sin(angle) * fadeOut;

	freq = .125f;			// cycles per second
	periods = time * freq;
	angle = periods * 360.f * DEG2RADf;
	if(AuTime::CurrentHourIsOdd())
		angle = -angle;

	double size = fadeOut + 1.414213562f;	// +sqrt(2)

	AuMatrix4x4 mv;
	mv.Translate(xoff,yoff,0);
	mv.RotateZ(angle);
	mv.Scale(size, size, size);

	m_RenderInfo->re->GLSL().TextureShader().Select();
	m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(mv);
	m_RenderInfo->re->GLSL().TextureShader().SetColor(m_Fade, m_Fade, m_Fade, fadeOut);
	m_RenderInfo->re->GLSL().TextureShader().SetTexture(m_texFadeEffect);

	// Set texturing mode to Modulate polygon color for blending.
	m_RenderInfo->re->glDisable(GL_DEPTH_TEST);
	m_RenderInfo->re->glEnable(GL_BLEND);
	m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_SpawnWaitVAO.SelectGL();
	m_SpawnWaitVAO.DrawGL();
	m_SpawnWaitVAO.DeselectGL();

	m_RenderInfo->re->glDisable(GL_BLEND);
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);

	m_RenderInfo->re->GLSL().TextureShader().Deselect();
}

void AuDesignerView::ShowSpawnRecallWait()
{
	// Position based on time.
	float time = AuTime::CurrentMinCycle() * 60.f;	// 0 to 60 seconds
	float freq = .10f;		// cycles per second
	float periods = time * freq;
	float angle = periods * PIf * 2.f;
	if(AuTime::CurrentHourIsOdd())
		angle = -angle;

	// Scale based on time.
	freq = .05f;		// cycles per second
	float periods2 = time * freq;
	float angle2 = periods2 * PIf * 2.f;
	double size = cos(angle2);

	const float xoff = cos(angle) * size;
	const float yoff = sin(angle) * size;

	size = fabs(size) + 1;

	AuMatrix4x4 mv;
	mv.Translate(xoff,yoff,0);
	mv.Scale(size, size, 1);

	m_RenderInfo->re->GLSL().TextureShader().Select();
	m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(mv);
	m_RenderInfo->re->GLSL().TextureShader().SetColor(.43f, .38f, .36f, fabs(cos(angle)));
	m_RenderInfo->re->GLSL().TextureShader().SetTexture(m_texSpawnWait);

	// Set texturing mode to Modulate polygon color for blending.
	m_RenderInfo->re->glDisable(GL_DEPTH_TEST);
	m_RenderInfo->re->glEnable(GL_BLEND);
	m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_SpawnWaitVAO.SelectGL();
	m_SpawnWaitVAO.DrawGL();
	m_SpawnWaitVAO.DeselectGL();

	m_RenderInfo->re->glDisable(GL_BLEND);
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);

	m_RenderInfo->re->GLSL().TextureShader().Deselect();
}

void AuDesignerView::ShowSpawnTieEffect()
{
	double size = (1.f - m_SpawnTieEffect)*2 + 1;

	AuMatrix4x4 mv;
	mv.Scale(size, size, size);

	m_RenderInfo->re->GLSL().TextureShader().Select();
	m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(mv);
	m_RenderInfo->re->GLSL().TextureShader().SetColor(.43f, .38f, .36f, m_SpawnTieEffect);
	m_RenderInfo->re->GLSL().TextureShader().SetTexture(m_texSpawnWait);

	// Set texturing mode to Modulate polygon color for blending.
	m_RenderInfo->re->glDisable(GL_DEPTH_TEST);
	m_RenderInfo->re->glEnable(GL_BLEND);
	m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_SpawnWaitVAO.SelectGL();
	m_SpawnWaitVAO.DrawGL();
	m_SpawnWaitVAO.DeselectGL();

	m_RenderInfo->re->glDisable(GL_BLEND);
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);

	m_RenderInfo->re->GLSL().TextureShader().Deselect();
}

void AuDesignerView::MoveForward(double distance)
{
	m_tCamera.Translate(0.0, 0.0, -distance);
}

void AuDesignerView::Roll(double rAngle)
{
	m_tCamera.RotateZ(rAngle);
}

void AuDesignerView::Pitch(double rAngle)
{
	m_tCamera.RotateX(rAngle);
}

void AuDesignerView::Yaw(double rAngle)
{
	m_tCamera.RotateY(rAngle);
}

void AuDesignerView::PanHorizontal(double feet)
{
	m_tCamera.Translate(feet, 0.0, 0.0);
}

void AuDesignerView::PanVertical(double feet)
{
	m_tCamera.Translate(0.0, feet, 0.0);
}

void AuDesignerView::ChangeCameraSpeed(float ftpersec)
{
	m_fCameraSpeed += ftpersec;
}

void AuDesignerView::StopCamera()
{
    m_fCameraSpeed = 0.0f;
}

void AuDesignerView::ResetCamera()
{
	//qDebug("AuDesignerView::ResetCamera");
	if(!m_idSceneAtom.IsNull())
	{
		// Setup camera in context of scene.
		DaPerson* sceneAtom;
		LockSceneAtom(&sceneAtom);
		if(sceneAtom)
		{
			// Turn off top-down view when not in atmosphere range of a planet.
			if(sceneAtom->IsBodyDyingOrDead())
			{
				SelectPreferredFirstThirdPersonView();
				int tilDead = sceneAtom->CurrentHitPoints() - DpBody::DeadHitPoints();
				if(sceneAtom->IsBodyDead() || tilDead <= 10)
				{
					// When almost dead the camera departs the body.
					if(tilDead < 0)
						tilDead = 0;
					int camSpeeds[] = {8192, 4096, 2048, 1024, 512, 256, 128, 64, 32, 16, 8};	// From DaCreature::SetupCamera
					m_CameraAnimSpeed = camSpeeds[tilDead]/2;		// Slow motion camera animation when dying or dead.
				}

				sceneAtom->SetupCamera(m_tSceneCamera, m_rNearClip, m_rFarClip, *m_RenderInfo);
			}
			else
			{
				m_CameraAnimSpeed = 1000;	// Fast camera animation between top-down and first-person view.
				sceneAtom->SetupCamera(m_tSceneCamera, m_rNearClip, m_rFarClip, *m_RenderInfo);

				// When not dying or dead, and not top-down view, snap camera to body instantly.
				if(!m_RenderInfo->IsViewTopDown())
					m_tCamera = m_tSceneCamera;
			}
		}
		UnlockSceneAtom();
	}
	else
	{
		m_tSceneCamera.LoadIdentity();
		m_tSceneCamera.Translate(0.0, -20.0, 0.0);
		m_tSceneCamera.RotateX(90.0 * DEG2RAD);
		m_rFarClip = DaWorldRingSection::MinimumSectionLength() * 2;
		m_rNearClip = 2.5;
	}
}

void AuDesignerView::SelectPreferredFirstThirdPersonView()
{
	if(m_View1stPref)
		m_RenderInfo->SetViewMode(AuAtomRenderInfo::VM_FirstPerson);
	else
		m_RenderInfo->SetViewMode(AuAtomRenderInfo::VM_ThirdPerson);
}

void AuDesignerView::ToggleFirstThirdPersonView() 
{
	m_View1stPref = !m_View1stPref;
	g_Au.Settings().writeEntry("Camera/1stPersonViewPref", m_View1stPref);
	SelectPreferredFirstThirdPersonView();
	ResetCamera();
	StopCamera();
}

void AuDesignerView::ToggleTopDownView() 
{
	if(m_RenderInfo->IsViewTopDown())
	{
		SelectPreferredFirstThirdPersonView();
		m_OverlayText = "";
	}
	else
	{
		m_RenderInfo->SetViewMode(AuAtomRenderInfo::VM_TopDown);
		if(IsInDesignEnvironment())
			m_OverlayText = "";
		else
		{
			m_OverlayText = m_RenderInfo->TopViewInfoName();
			m_OverlayTime.restart();
		}
	}
	ResetCamera();
	StopCamera();
}

void AuDesignerView::SetDesignMode(bool isDesignMode)
{
	// Don't do any work if this is not a change.
	if(m_IsDesignMode == isDesignMode)
		return;	// Not a change.

	// Change the design mode.
	m_IsDesignMode = isDesignMode;
}

DaAtom *AuDesignerView::FindAtomAt(DaPerson* sceneAtom, const QPoint &pxFind, unsigned int& uiPoly, AuPoint &fPolyLoc) const
{
	if(sceneAtom)
	{
		AuMatrix4x4 tDevicePersp, tDeviceOrtho;
		GetPixelTransforms(tDevicePersp, tDeviceOrtho);
		// Polygon index and a location within the polygon are returned by FindAtomAtPixel.
		DaAtom *pa = sceneAtom->FindAtomAtPixel(pxFind, tDevicePersp, tDeviceOrtho, uiPoly, fPolyLoc);
		return pa;
	}
	return 0;
}

DaAtom *AuDesignerView::FindAtomClosestTo(DaPerson* sceneAtom, const QPoint &pxFind) const
{
	if(sceneAtom)
	{
		AuMatrix4x4 tDevicePersp, tDeviceOrtho;
		GetPixelTransforms(tDevicePersp, tDeviceOrtho);
		DaAtom *pa = sceneAtom->FindClosestAtom(pxFind, tDevicePersp, tDeviceOrtho, AuMatrix4x4());
		return pa;
	}
	return 0;
}

DaAtom *AuDesignerView::FindControlAt(DaPerson* sceneAtom, const QPoint &pxFind, qint16& idPanel, AuPoint &fPanelLoc, AuPoint& hitLoc) const
{
	if(!sceneAtom)
		return 0;

	// Panel id and a location within the panel are returned by FindControlAtPixel.
	AuMatrix4x4 tDevicePersp, tDeviceOrtho;
	GetPixelTransforms(tDevicePersp, tDeviceOrtho);
	double zDepth = 1;											// Init to "not found" state.
	DaAtom *pa = sceneAtom->FindControlAtPixel(pxFind, tDevicePersp, tDeviceOrtho, idPanel, fPanelLoc, zDepth);
	if(!pa)
		return 0;

	// Do not activate controls that are too far away. We're a little generous here for game play so this isn't literally limited
	// to a person's reach distance.
	AuPoint fpxHit(pxFind.x(), pxFind.y(), zDepth);
	hitLoc = tDeviceOrtho.UnProject(fpxHit);
	double dist = hitLoc.Length();
	//qDebug() << "AuDesignerView::FindControlAt" << pa->Id() << pa->Name() << "panel" << idPanel << "dist" << dist <<
		//"panelLoc" << fPanelLoc;
	if(dist < .9 || dist > 10*FeetPerMeter())
		return 0;

	return pa;
}

// Get the transforms required to project world locations to device pixel locations.
void AuDesignerView::GetPixelTransforms(AuMatrix4x4 &tDevice, AuMatrix4x4 &tPersp, AuMatrix4x4 &tOrtho) const
{
	// Configure the device transform for this window.
	QRect rcClient = rect();
	double aspect = AuGLEffects::WindowAspectRatio(rcClient);	// aspect 1 = 90, aspect .5 = 180
	double fovy = AuGLEffects::WindowFieldOfViewY(aspect, g_Au.FieldOfViewY());
	tDevice.LoadIdentity();
	tDevice.Scale(rcClient.width()/2., -rcClient.height()/2., rcClient.width()/2.);
	tDevice.Translate(1, -1, 0);
	tDevice.Scale(aspect, 1, 1);

	// Create ortho and perspective camera transformations.
	tOrtho = m_tCamera.Inverse();
	tPersp.LoadIdentity();
	tPersp.Perspective(fovy, 1, m_rNearClip, m_rFarClip);
	tPersp *= tOrtho;
}

// Get the combined transforms required to project world locations to device pixel locations.
void AuDesignerView::GetPixelTransforms(AuMatrix4x4 &tDevicePersp, AuMatrix4x4 &tDeviceOrtho) const
{
	// Get the transforms required to project world locations to device pixel locations.
	AuMatrix4x4 tDevice, tPersp, tOrtho;
	GetPixelTransforms(tDevice, tPersp, tOrtho);

	// Create combined device transformations for perspective and ortho.
	tDeviceOrtho = tDevice * tOrtho;
	tDevicePersp = tDevice * tPersp;
}

QPoint AuDesignerView::ToPixel(const AuPoint &fLoc) const
{
	AuPoint fProj = ToFPixel(fLoc);
	QPoint pixel(round(fProj.x), round(fProj.y));

	return(pixel);
}

AuPoint AuDesignerView::ToFPixel(const AuPoint &fLoc) const
{
	AuMatrix4x4 tDevicePersp, tDeviceOrtho;
	GetPixelTransforms(tDevicePersp, tDeviceOrtho);

	// Project the point through the combined camera and device transformation.
	AuPoint fProj = tDevicePersp.Project(fLoc);

	return(fProj);
}

// Returns a point relative to the environment.
AuPoint AuDesignerView::FromPixel(const AuPoint &fpxLoc) const
{
	AuMatrix4x4 tDevicePersp, tDeviceOrtho;
	GetPixelTransforms(tDevicePersp, tDeviceOrtho);

	// Project the point through the combined camera and device transformation.
	AuPoint fProj = tDevicePersp.UnProject(fpxLoc);

	return fProj;
}

AuPoint AuDesignerView::FromPixelToDesignPlane(const QPoint& pxLoc, bool roundToDesignGrid, const AuPoint* refLoc)
{
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);

	AuPoint loc;
	if(sceneAtom)
		loc = FromPixelToDesignPlane(sceneAtom, pxLoc, roundToDesignGrid, refLoc);

	UnlockSceneAtom();
	return loc;
}

// The point returned is relative to the environment.
// The scene must already be locked when calling this function.
AuPoint AuDesignerView::FromPixelToDesignPlane(DaPerson* sceneAtom, const QPoint& pxLoc, bool roundToDesignGrid,
		const AuPoint* refLoc)
{
	m_MouseSnapped = false;
	AuPoint vRet = FromPixel(pxLoc);	// Default return value.
	if(sceneAtom)
	{
		if(IsInNewDesignEnvironment())
		{
			// New style designer.
			DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
			if(pde)
			{
				// Attempt to snap to existing shapes.
				QPointF viewPoint = PixelToViewport(this, pxLoc);

				if(g_AuClient.DesignerSnapToVertices() &&
						pde->FindVertexAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPoint,
							m_RenderInfo->DesignerPartObjectSelection(), m_RenderInfo->DesignerObjectMode(),
							m_RenderInfo->DesignerObjectIsolate(), &vRet))
					m_MouseSnapped = true;
				else if(g_AuClient.DesignerSnapToEdges() &&
						pde->FindEdgeAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPoint,
							m_RenderInfo->DesignerPartObjectSelection(), m_RenderInfo->DesignerObjectMode(),
							m_RenderInfo->DesignerObjectIsolate(), &vRet))
					m_MouseSnapped = true;

				if(!m_MouseSnapped)
				{
					// Cursor did not snap to a shape.
					AuMatrix4x4 tperson;
					sceneAtom->GetTransformRelativeToParent(tperson, pde);
					AuMatrix4x4 twp = tperson.Inverse() * DesignPlane();

					// Project the point through the combined camera and device matrix, into the design plane.
					AuMatrix4x4 tDevicePersp, tDeviceOrtho;
					GetPixelTransforms(tDevicePersp, tDeviceOrtho);
					AuPoint fProj;
					if(tDevicePersp.UnProjectIntoPlane(fProj, pxLoc, twp))
					{
						// The final result is relative to the design environment.
						vRet = tperson.Project(fProj);

						if(roundToDesignGrid)
						{
							// Apply design plane rounding and ortho restraints.
							if(refLoc)
							{
								double gridSpacing = g_AuClient.DesignerGridSpacingFt();
								AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
								const unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>& frameKeys = pFrame->LockActiveKeys();
								bool shifted = frameKeys.find(Qt::Key_Shift) != frameKeys.cend();
								pFrame->UnlockActiveKeys();
								if(shifted)
								{
									// Ortho round the point in the coordinate space of the design plane.
									vRet = DesignPlane().UnProject(vRet);				// Loc now in design plane coordinate space.
									AuPoint ref = DesignPlane().UnProject(*refLoc);		// refLoc now in design plane coordinates.
									if(g_AuClient.DesignerSnapToGrid() && gridSpacing > 0.)
										vRet.Ortho(ref, gridSpacing, false, true);
									else
										vRet.Ortho(ref, false, true);
									vRet = DesignPlane().Project(vRet);					// Loc now in design studio coordinates.
								}
								else if(g_AuClient.DesignerSnapToGrid() && gridSpacing > 0.)
								{
									// Round the point in the coordinate space of the design plane.
									vRet = DesignPlane().UnProject(vRet);				// Loc now in design plane coordinate space.
									vRet.Round(gridSpacing);
									vRet = DesignPlane().Project(vRet);					// Loc now in design studio coordinates.
								}
							}
							else
							{
								if(g_AuClient.DesignerSnapToGrid())
								{
									double gridSpacing = g_AuClient.DesignerGridSpacingFt();
									if(gridSpacing > 0.)
									{
										// Round the point in the coordinate space of the design plane.
										vRet = DesignPlane().UnProject(vRet);			// Loc now in design plane coordinate space.
										vRet.Round(gridSpacing);
										vRet = DesignPlane().Project(vRet);				// Loc now in design studio coordinates.
									}
								}
							}
						}

						// Polish the result.
						if(IsFuzzyZero(vRet.x))
							vRet.x = 0.0;
						if(IsFuzzyZero(vRet.y))
							vRet.y = 0.0;
						if(IsFuzzyZero(vRet.z))
							vRet.z = 0.0;
					}
				}
			}
		}
	}
	return vRet;
}

// The point returned is relative to the environment.
// The scene must already be locked when calling this function.
AuPoint AuDesignerView::FromPixelToPlane(const QPoint& pxLoc, const AuMatrix4x4& plane, bool roundToDesignGrid, const AuPoint* refLoc/*=0*/)
{
	Q_UNUSED(plane);
	Q_UNUSED(roundToDesignGrid);
	Q_UNUSED(refLoc);
	AuPoint vRet = FromPixel(pxLoc);	// Default return value.
	return vRet;
}

// Result is relative to the ship.
AuPoint AuDesignerView::FromPixelToScenePlane(DaAtom* parentAtSomeLevel, DaPerson* sceneAtom, const AuPoint& fpxLoc)
{
	AuPoint vRet = FromPixel(fpxLoc);	// Default return value.
	if(sceneAtom)
	{
		DaAtom* env = sceneAtom->FindEnvironmentAtom();
		if(env)
		{
			// This was cobbed from FromPixelToDesignPlane.
			// The plane is oriented to the person.
			AuMatrix4x4 twp;
			twp.LoadIdentity();
			if(sceneAtom->IsRider())
				twp.Translate(0,0,-sceneAtom->Parent()->GroundHeightAboveCG() + .1);	// Lower plane to slightly above the floor.
			else
				twp.Translate(0,0,-sceneAtom->GroundHeightAboveCG() + .1);	// Lower plane to slightly above the floor.

			AuMatrix4x4 tDevicePersp, tDeviceOrtho;
			GetPixelTransforms(tDevicePersp, tDeviceOrtho);

			// Project the point through the combined camera and device matrix.
			AuPoint fProj;
			if(tDevicePersp.UnProjectIntoPlane(fProj, fpxLoc, twp))
			{
				// Result is relative to the person.
				// Make the final result relative to the ship.
				twp.LoadIdentity();
				sceneAtom->GetTransformRelativeToParent(twp, parentAtSomeLevel);
				vRet = twp.Project(fProj);
				if(IsFuzzyZero(vRet.x))
					vRet.x = 0.0;
				if(IsFuzzyZero(vRet.y))
					vRet.y = 0.0;
				if(IsFuzzyZero(vRet.z))
					vRet.z = 0.0;
			}
		}
	}
	return vRet;
}

DdCommand* AuDesignerView::Command()
{
	if(m_pCommand[1])
		return m_pCommand[1];
	return m_pCommand[0];
}

const DdCommand* AuDesignerView::Command() const
{
	if(m_pCommand[1])
		return m_pCommand[1];
	return m_pCommand[0];
}

void AuDesignerView::StartCommand(DdCommand* pCommand)
{
	if(IsModalDlgUp())
		return;
	EndCommand(pCommand->RunLevel());	// End commands at the new command's run level and above.
	if(pCommand->RunLevel())
		m_pCommand[1] = pCommand;
	else
		m_pCommand[0] = pCommand;
	pCommand->Start();					// Inits the prompt on the designer toolbar.
	//m_bFreeMouse = false;				// Let scene grab control again.

	// Make sure this is the topmost widget for input.
	activateWindow();
	ActivateWindow();
}

void AuDesignerView::EndCurrentCommand()
{
	if(m_pCommand[1])
		EndCommand(1);
	EndCommand(0);
}

void AuDesignerView::EndCommands()
{
	EndCommand(0);
}

void AuDesignerView::EndCommand(int runLevel)
{
	m_DragConstraint = 0;
	m_RenderInfo->DragEnd();

	for(int killLevel = runLevel; killLevel <= 1; killLevel++)
	{
		if(m_pCommand[killLevel])
		{
			//qDebug() << "AuDesignerView::EndCommand";
			makeCurrent();

			m_pCommand[killLevel]->DeleteDeviceObjects(*m_RenderInfo->re);
			delete m_pCommand[killLevel];
			m_pCommand[killLevel] = 0;

			// Qt doc says don't bother calling this.
			//doneCurrent();
		}
	}

	// There could still be a command running depending on run level.
	if(Command())
		Command()->UpdatePrompt();		// Update prompt for running command.
	else
		CommandPromptChanged("");		// Clear prompt.
}

void AuDesignerView::OnViewFreeflyingcamera() 
{
	m_bFreeCamera = !m_bFreeCamera;
	ResetCamera();
	StopCamera();
	m_tCamera = m_tSceneCamera;
}

DaWorldLL* AuDesignerView::FindTerrainLocationAtPixel(DaPerson* sceneAtom, const QPoint &pxFind, AuPoint &fLoc) const
{
	if(!sceneAtom)
		return 0;

	AuMatrix4x4 tDevicePersp, tDeviceOrtho;
	GetPixelTransforms(tDevicePersp, tDeviceOrtho);
	DaWorldLL* pa = sceneAtom->FindTerrainLocationAtPixel(pxFind, tDevicePersp, tDeviceOrtho, fLoc);
	if(g_Au.m_LogMessages & LOG_AnimalEncounter)
	{
		if(pa)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt",
					QString("View: cursor at (%1, %2) found %3 at (%4, %5, %6).")
					.arg(pxFind.x())
					.arg(pxFind.y())
					.arg(pa->Name())
					.arg(fLoc.x)
					.arg(fLoc.y)
					.arg(fLoc.z));
		else
			g_Au.Log(g_Au.CacheApp() + "/soh.txt",
					QString("View: cursor at (%1, %2) not in terrain.")
					.arg(pxFind.x())
					.arg(pxFind.y()));
	}
	/*
	if(pa)
	{
		qDebug("%s (%s) location (%g,%g,%g) at pixel.\n", 
			pa->Name(), 
			pa->Id().StringId(), 
			fLoc.x, 
			fLoc.y, 
			fLoc.z);
		qDebug("%s location (%g,%g,%g in %s).\n", 
			sceneAtom->Name(), 
			sceneAtom->Location().x, 
			sceneAtom->Location().y, 
			sceneAtom->Location().z,
			sceneAtom->ParentId().StringId() 
			);
	}
	*/
	return pa;
}

DaWorldLL* AuDesignerView::FindTerrainLocationAtPixel(DaPerson* sceneAtom, const QPoint &pxFind, AuPoint &fLoc,
		DaBd** platform) const
{
	if(!sceneAtom)
	{
		platform = 0;
		return 0;
	}

	// Prefer platforms.
	AuMatrix4x4 tDevicePersp, tDeviceOrtho;
	GetPixelTransforms(tDevicePersp, tDeviceOrtho);
	*platform = sceneAtom->FindPlatformLocationAtPixel(pxFind, tDevicePersp, tDeviceOrtho, fLoc);
	if(*platform)
		return (*platform)->FindParentWorldInEnv();

	// Platform not found; check terrain.
	return FindTerrainLocationAtPixel(sceneAtom, pxFind, fLoc);
}

void AuDesignerView::PlanManufacturingProcess(int iProcessID)
{
	if(!Command())
		return;

	if(dynamic_cast<DdPlanBdBuilding*>(Command()))
	{
		DdPlanBdBuilding* pplan = (DdPlanBdBuilding*)Command();
		pplan->SetManufacturingProcess(iProcessID);
	}
}

void AuDesignerView::PlanManufacturingWorkerOk(bool enableWorker)
{
	if(!Command())
		return;

	if(dynamic_cast<DdPlanBdBuilding*>(Command()))
	{
		DdPlanBdBuilding* pplan = (DdPlanBdBuilding*)Command();
		//qDebug("AuDesignerView::PlanManufacturingWorkerOk enableWorker %c", enableWorker?'Y':'N');
		pplan->SetManufacturingWorkerOk(enableWorker);
	}
}

void AuDesignerView::PlanClearDevelopments() 
{
	if(IsInDesignEnvironment())
		return;

	StartCommand(new DdPlanClearGlobe(this));
}

void AuDesignerView::PlanClearCraters() 
{
	if(IsInDesignEnvironment())
		return;

	StartCommand(new DdPlanClearCraters(this));
}

void AuDesignerView::LockSceneAtom(DaPerson** sceneAtom)
{
	m_SceneLock.lockForRead();
	if(!m_idSceneAtom.IsNull())
		*sceneAtom = dynamic_cast<DaPerson*>(m_SceneEther.FindAtom(m_idSceneAtom));
	else
		*sceneAtom = 0;
}

void AuDesignerView::LockSceneAtomForWrite(DaPerson** sceneAtom)
{
	// Set up the scene atom pointers.
	m_SceneLock.lockForWrite();
	if(!m_idSceneAtom.IsNull())
		*sceneAtom = dynamic_cast<DaPerson*>(m_SceneEther.FindAtom(m_idSceneAtom));
	else
		*sceneAtom = 0;
}

void AuDesignerView::UnlockSceneAtom()
{
	m_SceneLock.unlock();
}

// When the camera is free, it has its own keyboard interface.
bool AuDesignerView::ProcessCameraKeyCommand(QKeyEvent *e)
{
	//qDebug("AuDesignerView::ProcessCameraKeyCommand key %x", e->key());
	// Remaining keys depend on free camera state.
	if(!m_bFreeCamera)
		return false;

	// Free camera is only available to programmers so it is not configurable.
	// With camera, control button reduces increment.
	bool bfast = true;
	if(e->modifiers() & Qt::ControlModifier)
		bfast = false;

	double dist = 1.0;
	if(bfast)
		dist *= 10.0;

	double angle = 1.0 * DEG2RAD;
	if(bfast)
		angle = 5.0 * DEG2RAD;

	bool bhandled = true;
	switch(e->key())
	{
	// Velocity.
	case Qt::Key_Minus:		// Decelerate
		if(bfast)
			ChangeCameraSpeed(-100.0f);
		else
			ChangeCameraSpeed(-0.1f);
		break;
	case Qt::Key_Plus:		// Accelerate
		if(bfast)
			ChangeCameraSpeed(100.0f);
		else
			ChangeCameraSpeed(0.1f);
		break;
	case Qt::Key_Asterisk:	// Stop
	case Qt::Key_5:			// Stop
		StopCamera();
		break;

	// Movement.
	case Qt::Key_Left:
		PanHorizontal(-dist);
		break;
	case Qt::Key_Right:
		PanHorizontal(dist);
		break;
	case Qt::Key_Up:
		PanVertical(dist);
		break;
	case Qt::Key_Down:
		PanVertical(-dist);
		break;
	case Qt::Key_7:
		MoveForward(dist);
		break;
	case Qt::Key_9:
		MoveForward(-dist);
		break;

	// Rotation.
	case Qt::Key_2:
		Pitch(-angle);
		break;
	case Qt::Key_8:
		Pitch(angle);
		break;
	case Qt::Key_4:
		Yaw(angle);
		break;
	case Qt::Key_6:
		Yaw(-angle);
		break;
	case Qt::Key_1:
		Roll(angle);
		break;
	case Qt::Key_3:
		Roll(-angle);
		break;

	default:
		bhandled = false;
		break;
	}

	return bhandled;
}

// Called by ProcessAtomControls. 'sceneAtom' may be null.
void AuDesignerView::UpdateSceneInfo(DaPerson* sceneAtom)
{
	//qDebug() << "AuDesignerView::UpdateSceneInfo scene" << sceneAtom;
	m_SceneInfoLock.lockForWrite();
	m_SceneAtomInShip = AuId(0);
	bool isDesignPreview = false;
	m_IsShipCommand = false;
	m_SceneAtomWorldBuildingCount = 0;
	m_SceneAtomWorldBuildingLimit = 0;
	m_SceneAtomWorldBuildingVolume = 0;
	m_SceneAtomWorldFenceCount = 0;
	m_SceneAtomWorldStreetLightCount = 0;
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
			m_idSceneSector = AuId();
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
			const DgData& gd = g_Au.LockDataForRead();
			bool hasEmp = gd.HasEmpire(m_SceneAtomEmpire);
			g_Au.UnlockData();
			if(!hasEmp)
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
			m_SceneAtomWorldFenceCount = world->FenceCount();
			m_SceneAtomWorldStreetLightCount = world->StreetLightCount();
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
		m_OnLLWorld = world && sceneAtom->IsInAtmosphere();
		if(sceneAtom->HasConsole())
			m_PersonStationId = sceneAtom->StationId();
		else
			m_PersonStationId = 0;
		m_PersonPose = sceneAtom->CurrentGeneralPose();

		// Design studio info: design mode and whether we are even in one.
		if(sceneAtom->InDesignEnvironment())
		{
			const DaDesignBase* pde = sceneAtom->FindParentDesignBaseInEnv();
			if(pde)
			{
				m_DesignMode = pde->DesignMode();
				isDesignPreview = pde->IsPreview();
			}
			else
				m_DesignMode = DaDesignBase::InvalidMode;		// Not really in a design environment.
		}
		else
			m_DesignMode = DaDesignBase::InvalidMode;			// Not in a design environment.

		if(sceneAtom->IsInsideSpacecraft())
		{
			const DaVessel* ship = sceneAtom->FindParentVesselInEnv();
			if(ship)
			{
				m_IsShipCommand = ship->InChainOfCommand(sceneAtom);
				m_SceneAtomInShip = ship->Id();
				// We want the wormhole effect when a wormhole drive is hot, otherwise we want the warp effect for all other
				// server transitions.
				m_IsWarping = !(ship->HasWormholeFTLDrive() && ship->IsFTLDriveHot(ship->FTLWormholeDrive()));
			}
		}
		else
			m_IsWarping = true;
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
		m_OnLLWorld = false;
		m_PersonStationId = 0;
		m_PersonPose = AuPose();

		m_DesignMode = DaDesignBase::InvalidMode;					// Not in a design environment.
	}
	m_SceneInfoLock.unlock();

	if(m_IsDesignPreview != isDesignPreview)
	{
		m_IsDesignPreview = isDesignPreview;
		if(m_IsDesignPreview)
			EndCommands();
		emit DesignerPreviewChanged();
	}
}

// Called by AuModel (another thread) to process atom and UI controls.
void AuDesignerView::ProcessControls()
{
	//qDebug() << "AuDesignerView::ProcessControls" << AuTime::Now();
	if(!m_SceneRendered)
	{
		//qDebug() << "AuDesignerView::ProcessControls scene not rendered yet.";
		return;
	}

	if(IsCharacterInputMode() && !m_bFreeMouse)
	{
		// Calculate mouse movement since last time through here.
		QRect rc = rect();
		QPoint pmid;
		pmid.setX((rc.left() + rc.right())/2);
		pmid.setY((rc.top() + rc.bottom())/2);
		QPoint globalMid = mapToGlobal(pmid);
		QPoint tlmid = pmid - QPoint(5,5);
		QRect rcmid(tlmid, QSize(11,11));
		QPoint mousePos = QCursor::pos();

		// Keep the cursor in the center of the screen when input is to the scene atom.
		m_ControlLock.lock();

		// Get mouse movement vector.
		QPoint mvec = mousePos - m_MousePosLast;

		// Keep cursor in center of screen.
		if(!rcmid.contains(mousePos))
		{
			QCursor::setPos(globalMid);
			mousePos = globalMid;
		}
		m_MousePosLast = mousePos;

		if(mvec.x() || mvec.y())
		{
			//qDebug() << "AuDesignerView::ProcessControls mvec" << mvec;
			if(m_MouseGrabbed || underMouse())
			{
				// Accumulate movement vector for control input. This enables us to keep accumulating movement and restoring the
				// mouse to the middle of the screen as often as necessary before ProcessAtomControls gets around to doing
				// anything with it.
				m_MouseVecAtom += mvec;
				//qDebug() << "AuDesignerView::ProcessControls mouseVec" << m_MouseVecAtom;
			}
			else
			{
				// Don't keep trying to reposition the mouse.
				m_bFreeMouse = true;

				// This widget is no longer the topmost widget under the mouse cursor.
				QCursor::setPos(mousePos);	// Put the mouse cursor back if the destination was not above this window.
			}
		}

		m_ControlLock.unlock();
	}
	else
	{
		// Resume regular mouse event processing.
		m_ControlLock.lock();
		m_MouseVecAtom = QPoint(0,0);
		m_ControlLock.unlock();
	}

	// Process control inputs to user interface.
	ProcessUIControls();

	// Process control inputs to scene atom.
	DaPerson* person;
	LockSceneAtom(&person);
	ProcessAtomControls(person);	// 'person' can be null.
	UnlockSceneAtom();

	// The controls have been sampled once. Start the clock on how long controls have been held down.
	ResetActiveKeyTimes();
}

// Process controls that affect the scene atom.
// The scene has been locked.
// This is called by the AuModel thread.
void AuDesignerView::ProcessAtomControls(DaPerson* sceneAtom)
{
	// Scene info wants to know if the scene atom is null.
	UpdateSceneInfo(sceneAtom);

	// Don't feed input to the scene atom or to control panels while the camera is free or there is no scene atom.
	if(!sceneAtom || !IsCharacterInputMode() || sceneAtom->IsInTransition())
		return;		// No input goes to the scene atom when the camera is free or scene transitioning.

	// Atom control timer keeps us from sending control updates too frequently.
	m_ControlLock.lock();
	if(m_AtomControlTime.isValid() && m_AtomControlTime.elapsed() < d_AtomControlTime)
	{
		m_ControlLock.unlock();
		return;
	}
	int msElapsed;
	if(m_AtomControlTime.isValid())
		msElapsed = m_AtomControlTime.elapsed();
	else
		msElapsed = 0;
	m_AtomControlTime.restart();
	m_ControlLock.unlock();

	// Get selection and activation values from the inventory ui widget.
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	Q_ASSERT(pFrame);
	AuGearPos gearPos = pFrame->GetGearSelectionPos();

	m_TargetLock.lockForRead();
	AuId targetId(m_TargetId);
	int targetLevel = m_TargetLevel;
	AuPoint targetLoc(m_TargetLoc);
	unsigned int targetPart = m_TargetPart;
	bool targetShield = m_TargetShield;
	m_TargetLock.unlock();

	m_ControlLock.lock();
	if(!m_pControlsAtom)
	{
		m_ControlLock.unlock();
		return;
	}

	// Sample the controls requested by the scene atom for activation.
	Qt::KeyboardModifiers keyMods = m_pControlsAtom->FilterModState(m_AtomKeys, m_AtomKeyMods);
	DiControlInputArray aActiveControls;
	m_pControlsAtom->SampleKeyInput(aActiveControls, m_AtomKeys, keyMods, targetId, targetPart, targetShield, targetLoc,
			targetLevel);
	//qDebug() << "m_AtomKeys" << m_AtomKeys.count() << m_AtomKeys << "activeControls" << aActiveControls.count();

	Qt::KeyboardModifiers mouseMods = m_pControlsAtom->FilterModState(m_AtomKeys, m_AtomMouseMods);
	if(m_MouseButtonsAtom != Qt::NoButton)
	{
		// SampleMouseInput returns false when the input is not streaming.
		if(!m_pControlsAtom->SampleMouseInput(aActiveControls, m_MouseButtonsAtom, mouseMods, QPoint(0,0), targetId, targetPart,
					targetShield, targetLoc, targetLevel))
			m_MouseButtonsAtom = Qt::NoButton;
	}
	if(m_MouseVecAtom != QPoint(0,0))
	{
		// Calculate mouse move vector in 100ths of degrees.
		if(width())
		{
			float degPerPixel = (float)g_AuClient.MouseLookSensitivity() / (float)width();
			float degx = degPerPixel * (float)m_MouseVecAtom.x() * 300.f;	// 300ths of a degree to rotate
			float degy = degPerPixel * (float)m_MouseVecAtom.y() * 300.f;	// 300ths of a degree to rotate
			QPoint moveDegs(round(degx), round(degy));	// 300ths of degrees. SampleMouseInput constrains values to limits.
			//qDebug() << "AuDesignerView::ProcessAtomControls mouseVec" << m_MouseVecAtom << "turnDeg" << moveDegs/300
				//<< "degsPerPixel" << degPerPixel << AuTime::Now();
			m_pControlsAtom->SampleMouseInput(aActiveControls, Qt::NoButton, mouseMods, moveDegs, targetId, targetPart,
					targetShield, targetLoc, targetLevel);
		}
		m_MouseVecAtom = QPoint(0,0);
	}

	// Sample joystick input.
	// Don't poll joystick with a modal dialog up; it conflicts with grabbing the joystick while configuring controls.
	if(!IsModalDlgUp() && pFrame->Joystick())
		m_pControlsAtom->SampleJoystickInput(aActiveControls, pFrame->Joystick(), targetId, targetPart, targetShield, targetLoc,
				targetLevel);

	// Clear activation ranges so they can accumulate input again.
	m_pControlsAtom->ClearRanges();

	m_ControlLock.unlock();

	// The scene atom must return to standing position if it is crouching and no crouch key is depressed.
	// Scene atom is a person.
	DaAnimal* mount = dynamic_cast<DaAnimal*>(sceneAtom->Parent());
	if(mount)
	{
		if(mount->IsCrouched())
		{
			// Scene atom is in crouching pose.
			if(!aActiveControls.HasId(DaPerson::CI_Crouch))
			{
				// No crouch button depressed so send stand command.
				aActiveControls.append(DiControlInput(-1, DaPerson::CI_Stand));
			}
		}
	}
	else
	{
		if(sceneAtom->IsCrouched())
		{
			// Scene atom is in crouching pose.
			if(!aActiveControls.HasId(DaPerson::CI_Crouch))
			{
				// No crouch button depressed so send stand command.
				aActiveControls.append(DiControlInput(-1, DaPerson::CI_Stand));
			}
		}
	}

	// Forward the control inputs to the model for processing.
	auApp->ProcessControlInput(sceneAtom->Id(), aActiveControls, gearPos, 0, msElapsed);

	// Syncronize with current key states.
	m_ControlLock.lock();
	m_AtomKeyMods = m_ActiveKeyMods;
	m_AtomMouseMods = m_ActiveMouseMods;
	unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash> atomKeys(m_AtomKeys);
	m_AtomKeys.clear();
	for(unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>::iterator aki = m_ActiveKeys.begin(); aki != m_ActiveKeys.end(); aki++)
	{
		if(atomKeys.find((*aki).first) != atomKeys.end())
		{
			if(!(*aki).second.isValid())
				(*aki).second.start();							// We have processed this active key so it needs a start time.
			m_AtomKeys.insert(pair<Qt::Key, AuElapsedTimer>((*aki).first, (*aki).second));	// Replace atom key entry with current time.
		}
		else
			m_AtomKeys.insert(pair<Qt::Key, AuElapsedTimer>((*aki).first, AuElapsedTimer()));			// Insert new key start.
	}
	m_ControlLock.unlock();
}

// Process controls that affect the user interface.
void AuDesignerView::ProcessUIControls()
{
	m_ControlLock.lock();
	Q_ASSERT(m_pControlsUI);

	// Sample the controls requested by the user for activation.
	Qt::KeyboardModifiers keyMods = m_pControlsUI->FilterModState(m_ActiveKeys, m_ActiveKeyMods);
	DiControlInputArray aActiveControls;
	m_pControlsUI->SampleKeyInput(aActiveControls, m_ActiveKeys, keyMods, AuId(0), 0, false, AuPoint(0,0), 0);

	// The temporary hand input mode requires special handling to track the key state, mainly because this is used to take focus
	// from the view window and we are likely to miss the keyRelease event. We must use the key state from the frame for this one.
	aActiveControls.RemoveId(CI_InputModeHandTemporary);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	const unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>& frameKeys = pFrame->LockActiveKeys();
	for(DiControlInputArray::iterator ci = m_pControlsUI->begin(); ci != m_pControlsUI->end(); ci++)
		if((*ci).GetId() == CI_InputModeHandTemporary)
		{
			if((*ci).Key() != Qt::Key_unknown && (*ci).TestKeyTriggered(frameKeys, keyMods))
				aActiveControls.append(*ci);
			break;
		}
	pFrame->UnlockActiveKeys();

	Qt::KeyboardModifiers mouseMods = m_pControlsUI->FilterModState(m_ActiveKeys, m_ActiveMouseMods);
	if(m_MouseButtonsUI != Qt::NoButton)
	{
		// SampleMouseInput returns false when the input is not streaming.
		if(!m_pControlsUI->SampleMouseInput(aActiveControls, m_MouseButtonsUI, mouseMods, QPoint(0,0), AuId(0), 0, false,
					AuPoint(0,0), 0))
			m_MouseButtonsUI = Qt::NoButton;
	}
	if(m_MouseVecUI != QPoint(0,0))
	{
		m_pControlsUI->SampleMouseInput(aActiveControls, Qt::NoButton, mouseMods, m_MouseVecUI, AuId(0), 0, false, AuPoint(0,0), 0);
		m_MouseVecUI = QPoint(0,0);
	}

	// Sample joystick input.
	// Don't poll joystick with a modal dialog up; it conflicts with grabbing the joystick while configuring controls.
	Q_ASSERT(pFrame);
	if(!IsModalDlgUp() && pFrame->Joystick())
		m_pControlsUI->SampleJoystickInput(aActiveControls, pFrame->Joystick(), AuId(0), 0, false, AuPoint(0,0), 0);

	m_ControlLock.unlock();

	// Process the active UI controls.
	//qDebug() << "AuDesignerView::ProcessUIControls active" << aActiveControls.count() << "keys" << m_ActiveKeys.count();
	m_HandInputTemporary = false;
	ProcessControlInput(aActiveControls);
}

void AuDesignerView::ResetActiveKeyTimes()
{
	// The controls have been sampled once. Start the clock on how long controls have been held down.
	m_ControlLock.lock();
	for(unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>::iterator aki = m_ActiveKeys.begin(); aki != m_ActiveKeys.end(); aki++)
		if(!(*aki).second.isValid())
			(*aki).second.start();				// Key just got pressed. Start the hold-down timer on the key.
	m_ControlLock.unlock();
}

// Process mouse events that affect the scene atom.
bool AuDesignerView::ProcessAtomMousePress(QMouseEvent* e)
{
	if(IsCharacterInputMode())
	{
		m_ControlLock.lock();
		m_MouseButtonsAtom = e->buttons();
		m_ControlLock.unlock();
	}
	return false;
}

// Process mouse events that affect the scene atom.
bool AuDesignerView::ProcessAtomMouseRelease(QMouseEvent* /*e*/)
{
	m_ControlLock.lock();
	m_MouseButtonsAtom = Qt::NoButton;
	m_ControlLock.unlock();
	return false;
}

// Process mouse events that affect the scene atom.
bool AuDesignerView::ProcessAtomWheelEvent(QWheelEvent* e)
{
	if(!IsCharacterInputMode())
		return false;	// Input is not going to an atom.
	
	if(!m_pControlsAtom)
		return false;	// No atom or controls to receive the input.

	// Get selection and activation values from the inventory ui widget.
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	Q_ASSERT(pFrame);
	AuGearPos gearPos = pFrame->GetGearSelectionPos();

	AuId sceneId;
	if(!SceneAtomInTransition())
		sceneId = SceneAtomId();

	// Some parent atoms are controlled by the scene atom. Send its rotations
	// with the control packet because they are handled locally for smoothness.
	if(!sceneId.IsNull())
	{
		m_TargetLock.lockForRead();
		AuId targetId(m_TargetId);
		int targetLevel = m_TargetLevel;
		AuPoint targetLoc(m_TargetLoc);
		unsigned int targetPart = m_TargetPart;
		bool targetShield = m_TargetShield;
		m_TargetLock.unlock();

		// Sample the controls requested by the scene atom for activation.
		DiControlInputArray aActiveControls;
		m_ControlLock.lock();
		if(m_pControlsAtom)
			m_pControlsAtom->SampleWheelInput(aActiveControls, e, targetId, targetPart, targetShield, targetLoc, targetLevel);
		m_ControlLock.unlock();

		// Forward the control inputs to the model for processing.
		auApp->ProcessControlInput(sceneId, aActiveControls, gearPos, 0, 0);
	}

	return false;
}

// Process mouse events that affect the user interface of the game.
bool AuDesignerView::ProcessUIMousePress(QMouseEvent* e)
{
	m_MouseButtonsUI = e->buttons();
	return false;
}

// Process mouse events that affect the user interface of the game.
bool AuDesignerView::ProcessUIMouseRelease(QMouseEvent* e)
{
	m_MouseButtonsUI = Qt::NoButton;

	// UI is only affected when in UI input mode.
	// Character input mode does not affect UI or consoles.
	if(!IsUIInputMode())
		return false;

	QPoint point = e->pos();
	if(e->button() == Qt::LeftButton)
	{
		// Make sure this is the topmost widget for input.
		ActivateWindow();

		// In design mode we do not operate controls.
		if(IsDesignMode())
			return false;
		if(Command())
			return false;	// We do not operate controls while running draw commands, like placing a building.

		if(!m_idSceneAtom.IsNull())
		{
			// We have a scene atom.
			QPoint pxv = point - m_MousePress;
			if(abs(pxv.x()) < 5 && abs(pxv.y()) < 5)
			{
				// The cursor didn't move much between button down and up.
				// Attempt to select an atom in the scene.
				if(SceneAtomInTransition())
					return false;

				DaPerson* sceneAtom;
				LockSceneAtom(&sceneAtom);
				if(!sceneAtom)
				{
					// Call UnlockSceneAtom in case the function decides to actually lock a mutex
					UnlockSceneAtom();
					return false;		// Return false if we failed to get the scene atom
				}

				// Look for a control to operate.
				qint16 idPanel;
				AuPoint fPanelLoc;	// Location within the panel.
				AuPoint hitLoc;		// Location relative to camera.
				DaAtom* pa = FindControlAt(sceneAtom, point, idPanel, fPanelLoc, hitLoc);

				// Globe and Terrain are controls for foraging commodities from the environment.
				qint8 chValue = 0;
				const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(pa);
				if(idPanel != CID_Invalid && llworld)
				{
					// Impose a limit to how much can be foraged from the same area in a certain time period.
					// Ideally, this would result in a slower rate of foraging than a mine or logging camp.
					// The panel id is the COMMODITYID foraged.
					AuPoint globeLoc;
					sceneAtom->GetLocationRelativeToParent(globeLoc, llworld);
					AuForageHistory forageAttempt = AuForageHistory((COMMODITYID)idPanel, pa->Id(), globeLoc);
					
					// Check for any recent forage activity that prevents this attempt.
					// To manage the list, we will throw away and ignore items that have grown too old.
					QList<AuForageHistory>::iterator fi = m_ForageHistory.begin();
					while(fi != m_ForageHistory.end())
					{
						if((*fi).IsExpired())
						{
							// Discard the forage history item that has grown too old.
							fi = m_ForageHistory.erase(fi);
						}
						else
						{
							// The forage history item has not expired. See if it blocks the current forage attempt.
							bool foragePrevented = (*fi).PreventsForageOnGlobe(forageAttempt);
							if(foragePrevented)
							{
								// The terrain cannot be foraged again. More time must pass. Tell the user why.
								QString text;
								COMMODITYID cid = forageAttempt.CommodityId();
								switch(cid)
								{
								default:
									text = tr("All the readily accessible %1 has been gleaned from this area.")
										.arg(g_Au.CommodityName(cid));
									break;

								// Rock Resources
								case CID_Water:
									text = tr("The bottle is filling with %1 slowly.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Bolite:
								case CID_Ice:
									text = tr("The frozen mass of %1 cracked but a piece did not break away.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Coal:
								case CID_Stone:
									text = tr("Searching the boulders and rubble did not reveal another usable chunk of %1.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_CrudeOil:
								case CID_Viathol:
								case CID_Polytaride:
									text = tr("The pool of %1 is now empty. Oozing fissures have been exposed.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Crystals:
								case CID_Eludium:
									text = tr("Splinters and fragments crumble as you search. "
											"A good shard of %1 could not be found.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Gems:
								case CID_Lumenite:
									text = tr("Clear bright %1 could not be found among many tantalizing bits and pieces.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_GoldOre:
								case CID_Minerals:
								case CID_Ore:
								case CID_Radioactives:
								case CID_Vulcanite:
								case CID_Adamantite:
									text = tr("The rich vein of %1 could not be followed into the substrate.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Phlogiston:
									text = tr("The roaring %1 vent has subsided. "
											"The pressure is now too low to easily collect more from the seeping cracks.")
										.arg(g_Au.CommodityName(cid));
									break;

								// Plant Resources
								case CID_Beans:					// Shrub
								case CID_Grain:					// Shrub
								case CID_Grapes:				// Shrub
								case CID_Hops:					// Shrub
								case CID_Vegetable:				// Shrub
									text = tr("The remaining clusters of %1 are not ripe.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Fiber:					// Shrub or Tree
									text = tr("The branches are bare of %1.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Fruit:					// Shrub or Tree
								case CID_Herbs:					// Shrub or Tree
								case CID_Nuts:					// Shrub or Tree
								case CID_Spices:				// Shrub or Tree
									text = tr("The branches have been picked clean of ripe %1.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Hay:					// Shrub
									text = tr("The %1 needs to grow before it can be harvested again.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Wood:					// Tree
									text = tr("The branch creaked but a %1 did not break away.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Seed:					// Ground cover
									text = tr("The tiny plants are bare of seeds.");
									break;
								}

								LocalLogMessage(text);
								pa = 0;
								break;
							}
							fi++;
						}
					}

					// Remember successful forage attempts.
					if(pa)
						m_ForageHistory.append(forageAttempt);
				}
				else
				{
					const DaItem* item = dynamic_cast<const DaItem*>(pa);
					if(item)
					{
						// Watch for armed bombs and static display items.
						QString empireName;
						const DgData& gd = g_Au.LockDataForRead();
						bool isAgent = gd.ClientPlayerIsAgent();		// Show design tips to agents.
						g_Au.UnlockData();
						bool isControllable = item->IsStaticDisplayControllableBy(m_idSceneAtom, m_SceneAtomEmpire);
						if(isControllable || (item->IsStaticDisplay() && isAgent))
						{
							// Display a modeless dialog to manipulate the item.
							pa = 0;
							QString whatItem = item->SpotDescription(SceneAtomEmpire());
							if(isControllable)
							{
								if(!item->Inscription().isEmpty())
									LocalLogMessage(tr("%1 displayed by %2, selected for manipulation. "
												"The inscription says, \"%3\"")
											.arg(whatItem)
											.arg(item->OwnerName())
											.arg(item->Inscription()));
								else
									LocalLogMessage(tr("%1 displayed by %2, selected for manipulation.")
											.arg(whatItem)
											.arg(item->OwnerName()));
							}
							else
							{
								if(!item->Inscription().isEmpty())
									LocalLogMessage(tr("%1 displayed by %2, selected for manipulation by agent. "
												"The inscription says, \"%3\"")
											.arg(whatItem)
											.arg(item->OwnerName())
											.arg(item->Inscription()));
								else
									LocalLogMessage(tr("%1 displayed by %2, selected for manipulation by agent.")
											.arg(whatItem)
											.arg(item->OwnerName()));
							}
							ShowItemManipulator(*item);
						}
						else if(item->IsStaticDisplay())
						{
							pa = 0;
							QString whatItem = item->SpotDescription(SceneAtomEmpire());
							if(!item->Inscription().isEmpty())
								LocalLogMessage(tr("%1 displayed by %2. The inscription says, \"%3\"")
										.arg(whatItem)
										.arg(item->OwnerName())
										.arg(item->Inscription()));
							else
								LocalLogMessage(tr("%1 displayed by %2.")
										.arg(whatItem)
										.arg(item->OwnerName()));
						}
						else if(item->IsArmedDetonatorBomb() && !item->IsArmedBy(m_idSceneAtom))
						{
							AuId bombId(item->Id());
							UnlockSceneAtom();
							pa = 0;
							AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
							AuDefuseBomb defuse(bombId, pFrame);
							if(auApp->Frame()->ExecDialog(defuse) == QDialog::Accepted)
							{
								chValue = defuse.CutWireIndex();					// Attempt to defuse the bomb.
								LockSceneAtom(&sceneAtom);
								pa = m_SceneEther.FindAtom(bombId);			// Restore our pointer to the bomb atom.
							}
							else
								LockSceneAtom(&sceneAtom);
						}
					}
				}

				if(pa)
				{
					DiControlInputArray sampleMouse;
					pa->SampleControlPanelInput(idPanel, fPanelLoc, sampleMouse, false);
					
					// Get selection and activation values from the ui.
					AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
					Q_ASSERT(pFrame);
					AuGearPos gearPos = pFrame->GetGearSelectionPos();

					// Forward the control inputs to the model for processing.
					auApp->ProcessControlInput(pa->Id(), sampleMouse, gearPos, chValue, 0);
				}
				else if(!m_ItemManipulator || !m_ItemManipulator->isVisible())
				{
					// Static display item not selected at cursor.
					// No control panel found at cursor so try selecting.
					// Don't select or deselect when running a command.
					if(!Command())
					{
						/*
						unsigned int uiPoly;
						AuPoint fPolyLoc;
						DaAtom *pa = FindAtomAt(point, uiPoly, fPolyLoc);
						if(pa)
						{
							if(!SelectUnitToggle(pa))
								DeselectAllUnits();	// Atom at cursor was ignored so deselect everybody.
						}
						else
						{
							// No atom found at cursor so deselect.
							DeselectAllUnits();
						}
						*/
						int unitsSelected = 0;

						DaAtom *pEnvironment = sceneAtom->FindEnvironmentAtom();

						double aspect = AuGLEffects::WindowAspectRatio(rect());	// aspect 1 = 90, aspect .5 = 180
						double fovy = AuGLEffects::WindowFieldOfViewY(aspect, g_Au.FieldOfViewY());
						AuMatrix4x4 tPersp;
						tPersp.Perspective(fovy, aspect, m_rNearClip, m_rFarClip);
						AuMatrix4x4 tProj;
						tProj.Scale(width()/2.,-height()/2.,1);	// Invert Y. Scale to pixels.
						tProj.Translate(1,-1,0);				// Offset to upper left origin.
						tProj *= tPersp;

						// Multiply by inverse of subatom transformation (Translation, rotation, and scale.)
						AuMatrix4x4 myEnvM;
						sceneAtom->GetTransformRelativeToParent(myEnvM, pEnvironment);
						AuMatrix4x4 tmodel = m_tCamera.Inverse() * myEnvM.Inverse();

						bool clickedEnemy = false;
						QList<AuId> atomIds = m_SceneEther.AtomIds();	// List of all atom ids subordinate to root at all levels.
						for(QList<AuId>::const_iterator si = atomIds.cbegin(); si != atomIds.cend(); si++)
						{
							const DaUnit* unit = dynamic_cast<const DaUnit*>(m_SceneEther.FindLiveAtom(*si));
							if(!unit)
								continue;	// Only allow select of unit atoms.

							// Found a unit in the scene. Where is it on the screen?
							// Get the reverse transformation back up to the parent.
							AuMatrix4x4 tsel;
							unit->GetTransformRelativeToParent(tsel, pEnvironment);
							AuMatrix4x4 tsub(tmodel * tsel);

							const DaCreature* creature = dynamic_cast<const DaCreature*>(unit);
							if(creature)
							{
								// Calculate the creature atom's location relative to the scene atom's eye.
								AuPoint sceneLoc = tsub.Translation();
								if(sceneLoc.z > 0)
									continue;	// Location is behind the scene atom's eye.
								if(!m_RenderInfo->IsViewTopDown() && sceneLoc.z < -300)
									continue;	// Too far away to select.

								// Calculate the unit atom's pixel location.
								sceneLoc = tProj.Project(sceneLoc);
								QPoint pixelLoc;
								pixelLoc.setX((int)round(sceneLoc.x));
								pixelLoc.setY((int)round(sceneLoc.y));
								QRect selRect(pixelLoc - QPoint(20,20), QSize(40,40));

								// Select the unit if the mouse clicked inside either of the selection rectangles.
								if(selRect.contains(point))
								{
									if(unit->UnitCanBeSelected(sceneAtom, m_RenderInfo->IsViewTopDown()))
									{
										// Rank sort so the first selected unit is the highest ranking unit. He's the dude who
										// acknowledges orders for the group.
										QString name = unit->Name();
										int rank;
										QString rankAbbr, rankName;
										const DaCitizen* citizen = dynamic_cast<const DaCitizen*>(unit);
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
										}
										else
											rank = 0;

										SelectUnitToggle(unit->Id(), name, rank, rankAbbr, rankName, unit->NpcRole());
										if(m_Selected.Contains(unit->Id()))
											LocalLogMessage(tr("%1 selected.").arg(unit->Name()));
										else
											LocalLogMessage(tr("%1 deselected.").arg(unit->Name()));
										unitsSelected++;
									}
									else if(unit->EmpireId() != sceneAtom->EmpireId())
									{
										// The creature cannot be selected. Can it be targetted for attack?
										bool attackUnit = false;
										if(!unit->EmpireId())
											attackUnit = true;	// A creature with no empire id is likely a wild animal.
										else
										{
											// Check my empire's stance toward the empire who owns the creature that was clicked.
											POLITICAL_STANCE stance = PoliticalStance(sceneAtom->EmpireId(), unit->EmpireId());
											if(stance == POL_Enemy)
												attackUnit = true;
										}
										if(attackUnit)
										{
											clickedEnemy = true;
											m_TargetLock.lockForWrite();
											m_TargetSpotId = unit->Id();
											m_TargetSpotLoc = AuPoint(0,0);
											m_TargetLock.unlock();
										}
									}
									break;
								}
							}
							else
							{
								// Calculate the unit atom's location relative to the scene atom's eye.
								AuPoint sceneLoc = tsub.Translation();
								if(sceneLoc.z > 0)
									continue;	// Location is behind the scene atom's eye.
								if(!m_RenderInfo->IsViewTopDown() && sceneLoc.z < -300)
									continue;	// Too far away to select.

								// Calculate the unit atom's pixel location.
								sceneLoc = tProj.Project(sceneLoc);
								QPoint pixelLoc;
								pixelLoc.setX((int)round(sceneLoc.x));
								pixelLoc.setY((int)round(sceneLoc.y));
								QRect selRect(pixelLoc - QPoint(20,20), QSize(40,40));

								// Select the unit if the mouse clicked inside the selection rectangle.
								if(selRect.contains(point))
								{
									if(unit->UnitCanBeSelected(sceneAtom, m_RenderInfo->IsViewTopDown()))
									{
										SelectUnitToggle(unit->Id(), unit->Name(), 0, "", "", unit->NpcRole());
										if(m_Selected.Contains(unit->Id()))
											LocalLogMessage(tr("%1 selected.").arg(unit->Name()));
										else
											LocalLogMessage(tr("%1 deselected.").arg(unit->Name()));
										unitsSelected++;
									}
									else if(unit->EmpireId() != sceneAtom->EmpireId())
									{
										// The unit cannot be selected. Can it be targetted for attack?
										bool attackUnit = false;
										if(!unit->EmpireId())
											attackUnit = true;	// A unit with no empire id is likely a wild animal.
										else
										{
											// Check my empire's stance toward the empire who owns the unit that was clicked.
											POLITICAL_STANCE stance = PoliticalStance(sceneAtom->EmpireId(), unit->EmpireId());
											if(stance == POL_Enemy)
												attackUnit = true;
										}
										if(attackUnit)
										{
											clickedEnemy = true;
											m_TargetLock.lockForWrite();
											m_TargetSpotId = unit->Id();
											m_TargetSpotLoc = AuPoint(0,0);
											m_TargetLock.unlock();
										}
									}
									break;
								}
							}
						}

						// If no atom was found at the cursor location, deselect everybody.
						if(!unitsSelected)
						{
							// No new units were selected or deselected.
							if(!m_Selected.isEmpty())
							{
								// Some units are currently selected.
								if(clickedEnemy)
								{
									// Issue an attack order to the selected units.
									SquadIssueOrder(ORDER_AttackTarget);
								}
								else
								{
									// Did not click on an enemy unit.
									// Issue a move order to the selected units.

									// Prefer to find a location on board the scene atom's ship.
									bool movedAboardShip = false;
									if(sceneAtom->IsInSpacecraft())
									{
										DaVessel* v = sceneAtom->FindParentVesselInEnv();
										if(v)
										{
											// Ship designs have an arbitrary deck structure and up orientation.
											// Project the point into the person's invisible plane, through their origin,
											// based on their current orientation.
											AuPoint loc = FromPixelToScenePlane(v, sceneAtom, point);	// Loc is rel to env.
											ROOMTYPE roomType;
											QString roomName;
											quint16 roomId = v->InsideRoomType(loc, roomType, roomName);
											if(roomId)
											{
												// The hit location is inside a room.
												m_TargetLock.lockForWrite();
												m_TargetSpotId = v->Id();
												m_TargetSpotLoc = loc;
												m_TargetLock.unlock();
												SquadIssueOrder(ORDER_GoToTarget);
												movedAboardShip = true;
											}
										}
									}
									else if(sceneAtom->IsInBd())
									{
										DaBd* v = sceneAtom->FindParentBdInEnv();
										if(v)
										{
											// New style bd designs have an arbitrary deck structure and up orientation.
											// Project the point into the person's invisible plane, through their origin,
											// based on their current orientation.
											AuPoint loc = FromPixelToScenePlane(v, sceneAtom, point);	// Loc is rel to env.
											ROOMTYPE roomType;
											QString roomName;
											quint16 roomId = v->InsideRoomType(loc, roomType, roomName);
											if(roomId)
											{
												// The hit location is inside a room.
												m_TargetLock.lockForWrite();
												m_TargetSpotId = v->Id();
												m_TargetSpotLoc = loc;
												m_TargetLock.unlock();
												SquadIssueOrder(ORDER_GoToTarget);
												movedAboardShip = true;
											}
										}
									}

									// Didn't move aboard a ship so look for a terrain location.
									if(!movedAboardShip)
									{
										AuPoint loc;
										DaWorldLL* pTerrain = FindTerrainLocationAtPixel(sceneAtom, point, loc);
										if(pTerrain)
										{
											m_TargetLock.lockForWrite();
											m_TargetSpotId = pTerrain->Id();
											m_TargetSpotLoc = loc;
											m_TargetLock.unlock();
											SquadIssueOrder(ORDER_GoToTarget);
										}
									}
								}
							}
						}
					}
				}
				UnlockSceneAtom();
			}
			else
			{
				// The cursor moved enough to test for area selection of units in the scene.
				// Don't select or deselect when running a command.
				if(!Command())
				{
					bool select = (e->modifiers() & Qt::ShiftModifier) ? false : true;
					int unitsSelected = 0;
					DaPerson* sceneAtom;
					LockSceneAtom(&sceneAtom);
					if(sceneAtom)
					{
						// Calculate the selection rectangle in pixels.
						int selx = abs(point.x() - m_MousePress.x());
						int sely = abs(point.y() - m_MousePress.y());
						QPoint topLeft;
						if(point.x() < m_MousePress.x())
							topLeft.setX(point.x());
						else
							topLeft.setX(m_MousePress.x());
						if(point.y() < m_MousePress.y())
							topLeft.setY(point.y());
						else
							topLeft.setY(m_MousePress.y());
						QRect selRect(topLeft, QSize(selx,sely));

						DaAtom *pEnvironment = sceneAtom->FindEnvironmentAtom();

						double aspect = AuGLEffects::WindowAspectRatio(rect());	// aspect 1 = 90, aspect .5 = 180
						double fovy = AuGLEffects::WindowFieldOfViewY(aspect, g_Au.FieldOfViewY());
						AuMatrix4x4 tPersp;
						tPersp.Perspective(fovy, aspect, m_rNearClip, m_rFarClip);
						AuMatrix4x4 tProj;
						tProj.Scale(width()/2.,-height()/2.,1);	// Invert Y. Scale to pixels.
						tProj.Translate(1,-1,0);				// Offset to upper left origin.
						tProj *= tPersp;

						// Multiply by inverse of subatom transformation (Translation, rotation, and scale.)
						AuMatrix4x4 myEnvM;
						sceneAtom->GetTransformRelativeToParent(myEnvM, pEnvironment);
						AuMatrix4x4 tmodel = m_tCamera.Inverse() * myEnvM.Inverse();

						QList<AuId> atomIds = m_SceneEther.AtomIds();	// List of all atom ids subordinate to root at all levels.
						for(QList<AuId>::const_iterator si = atomIds.cbegin(); si != atomIds.cend(); si++)
						{
							const DaUnit* unit = dynamic_cast<const DaUnit*>(m_SceneEther.FindLiveAtom(*si));
							if(!unit)
								continue;	// Only allow select of unit atoms.
							if(!unit->UnitCanBeSelected(sceneAtom, m_RenderInfo->IsViewTopDown()))
								continue;

							// Found a unit in the scene. Where is it on the screen?
							// Get the reverse transformation back up to the parent.
							AuMatrix4x4 tsel;
							unit->GetTransformRelativeToParent(tsel, pEnvironment);
							AuMatrix4x4 tsub(tmodel * tsel);

							const DaCitizen* citizen = dynamic_cast<const DaCitizen*>(unit);
							if(citizen)
							{
								// Calculate the citizen atom's location relative to the scene atom's eye.
								AuPoint sceneLoc = tsub.Translation();
								if(sceneLoc.z > 0)
									continue;	// Location is behind the scene atom's eye.
								if(!m_RenderInfo->IsViewTopDown() && sceneLoc.z < -150)
									continue;	// Too far away to select.

								// Calculate the unit atom's pixel location.
								sceneLoc = tProj.Project(sceneLoc);
								QPoint pixelLoc;
								pixelLoc.setX((int)round(sceneLoc.x));
								pixelLoc.setY((int)round(sceneLoc.y));

								// Select the unit if its origin or eye is inside the selection rectangle.
								if(selRect.contains(pixelLoc))
								{
									// Rank sort so the first selected unit is the highest ranking unit. He's the dude who
									// acknowledges orders for the group.
									int rank = citizen->Rank() + 1;
									if(citizen->IsNpcOfficer())
										rank += 30;	// Officer ranks count higher than the ranks of crew or troops.
									else if(citizen->IsNpcCrew())
										rank += 20;	// Crew ranks count higher than the ranks of troops.
									else if(citizen->IsNpcTroop())
										rank += 10;	// Troop ranks count higher than the ranks of civilians.

									if(SelectUnit(citizen->Id(), citizen->Name(), rank, citizen->RankAbbr(), citizen->RankName(),
												citizen->NpcRole(), select))
										unitsSelected++;
								}
							}
							else
							{
								// The unit is not a citizen.
								// Calculate the unit atom's location relative to the scene atom's eye.
								AuPoint sceneLoc = tsub.Translation();
								if(sceneLoc.z > 0)
									continue;	// Location is behind the scene atom's eye.
								if(!m_RenderInfo->IsViewTopDown() && sceneLoc.z < -150)
									continue;	// Too far away to select.

								// Calculate the unit atom's pixel location.
								sceneLoc = tProj.Project(sceneLoc);
								QPoint pixelLoc;
								pixelLoc.setX((int)round(sceneLoc.x));
								pixelLoc.setY((int)round(sceneLoc.y));

								// Select the unit if its origin or eye is inside the selection rectangle.
								if(selRect.contains(pixelLoc))
									if(SelectUnit(unit->Id(), unit->Name(), 0, "", "", unit->NpcRole(), select))
										unitsSelected++;
							}
						}
					}
					UnlockSceneAtom();
					if(unitsSelected)
					{
						if(select)
							LocalLogMessage(tr("%1 units selected.").arg(unitsSelected));
						else
							LocalLogMessage(tr("%1 units deselected.").arg(unitsSelected));
					}
				}
			}
		}
	}
	return false;
}

// Returns true if the selection actually changes.
bool AuDesignerView::SelectUnit(const AuId& id, const QString& name, int rank, const QString& rankAbbr, const QString& rankName,
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

bool AuDesignerView::SelectUnitToggle(DaPerson* sceneAtom, DaAtom* pSelect)
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

void AuDesignerView::SelectUnitToggle(const AuId& id, const QString& name, int rank, const QString& rankAbbr,
		const QString& rankName, NPC_ROLE npcRole)
{
	if(m_Selected.Contains(id))
		m_Selected.Remove(id);
	else
		m_Selected.Add(AuSelectedUnit(id, name, rank, rankAbbr, rankName, npcRole));
}

void AuDesignerView::DeselectAllUnits()
{
	m_Selected.clear();
}

void AuDesignerView::SetCursor()
{
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	SetCursor(sceneAtom);
	UnlockSceneAtom();
}

void AuDesignerView::SetCursor(DaPerson* sceneAtom)
{
	bool grab = false;
	if(Command())
	{
		Command()->SetCursor(m_tCamera, sceneAtom, m_MouseLast);
		m_Cursor = CURSOR_Unknown;
	}
	else if(IsDesignMode())
	{
		if(m_Cursor != CURSOR_Arrow)
		{
			setCursor(QCursor(Qt::ArrowCursor));
			m_Cursor = CURSOR_Arrow;
		}
	}
	else if(IsCharacterInputMode())
	{
		// Mouse input is controlling an atom (person).
		if(m_IsDesignMode)
		{
			// In design mode, use hand to interact with environment.
			if(g_Au.LeftRightCursor() == 1 || (g_Au.LeftRightCursor() == 0 && m_IsLefty))
			{
				if(m_Cursor != CURSOR_LeftHand)
				{
					QCursor curs(QPixmap(":data/HandLeft.png"), 30,3);
					setCursor(curs);
					m_Cursor = CURSOR_LeftHand;
				}
			}
			else
			{
				if(m_Cursor != CURSOR_RightHand)
				{
					QCursor curs(QPixmap(":data/Hand.png"), 1,3);
					setCursor(curs);
					m_Cursor = CURSOR_RightHand;
				}
			}
		}
		else
		{
			if(m_Cursor != CURSOR_None)
			{
				setCursor(QCursor(Qt::BlankCursor));
				m_Cursor = CURSOR_None;
			}

			// Grab the mouse when appropriate.
			grab = g_AuClient.MouseGrab() && hasFocus();
			if(!m_SceneRendered || !auApp->IsSceneConnected() || !sceneAtom || sceneAtom->IsInTransition())
				grab = false;
		}
	}
	else
	{
		// Must be UI input mode.
		// Input is not to atom (person) so use hand cursor to interact
		// with environment.
		if(g_Au.LeftRightCursor() == 1 || (g_Au.LeftRightCursor() == 0 && m_IsLefty))
		{
			if(m_Cursor != CURSOR_LeftHand)
			{
				QCursor curs(QPixmap(":data/HandLeft.png"), 30,3);
				setCursor(curs);
				m_Cursor = CURSOR_LeftHand;
			}
		}
		else
		{
			if(m_Cursor != CURSOR_RightHand)
			{
				QCursor curs(QPixmap(":data/Hand.png"), 1,3);
				setCursor(curs);
				m_Cursor = CURSOR_RightHand;
			}
		}
	}

	if(grab)
		GrabMouse();	// We want the mouse to be grabbed.
	else
		ReleaseMouse();	// We don't want to grab the mouse.
}

bool AuDesignerView::ProcessCameraMouseMove(QMouseEvent* e)
{
	if(!m_bFreeCamera)
		return false;

	// Free camera is only available to programmers so it is not configurable.
	QPoint point = e->pos();
	QPoint pxVec = point - m_MouseLast;
	if (e->buttons() & Qt::RightButton)
	{
		QRect rcClient = rect();
		double rX = (double)pxVec.x() / (double)rcClient.width();
		double rY = (double)pxVec.y() / (double)rcClient.height();

		// With camera, control button reduces increment.
		bool bfast = true;
		if(e->modifiers() & Qt::ControlModifier)
			bfast = false;

		double dist = 1.0;
		if(bfast)
			dist *= 10.0;

		double anglex = rX * PI;
		double angley = rY * PI;
		if(!bfast)
		{
			anglex /= 5.0;
			angley /= 5.0;
		}

		if(e->modifiers() & Qt::ShiftModifier)
		{
			double dist = 10.0;

			// Shift key is pan.
			if (rX > 0)
				PanHorizontal(dist);
			else if (rX < 0)
				PanHorizontal(-dist);
			if (rY > 0)
				PanVertical(-dist);
			else if (rY < 0)
				PanVertical(dist);
		}
		else if(e->modifiers() & Qt::AltModifier)
		{
			// Alt key is roll.
			Roll(-angley);
		}
		else
		{
			// No alt key is rotate.
			Yaw(-anglex);
			Pitch(-angley);
		}
	}
	return false;
}

// Add a new atom to my display objects.
void AuDesignerView::AtomAdded(DaAtom* atom)
{
	DaPerson* sceneAtom;
	LockSceneAtomForWrite(&sceneAtom);
	DaAtom* vwPar = m_SceneEther.FindAtom(atom->ParentId());	// Find the view's copy of the parent atom.
	if(!vwPar)
	{
		if(!atom->ParentId().IsNull())
			qWarning() << "error: AuDesignerView::AtomAdded parentId" << atom->ParentId() << "not found in view";
	}
	m_SceneEther.AddAtom(atom);

	// Dirty the galaxy sector locs cache when adding an atom. We only ever add sectors to galaxy atoms.
	DaGalaxy* galaxy = dynamic_cast<DaGalaxy*>(vwPar);
	if(galaxy)
		galaxy->InvalidateSectorLocCache();			// Cache will be recreated when needed.

	if(atom->Is(m_idSceneAtom))
	{
		m_ControlLock.lock();
		if(m_pControlsAtom)
			m_pControlsAtom->clear();
		else
			m_pControlsAtom = new DiControlInputArray;
		atom->EnumControlInputs(*m_pControlsAtom);	// Get controls based on new scene atom's situation.
		m_ControlLock.unlock();
	}
	UnlockSceneAtom();
}

// Remove the rows from my display objects.
void AuDesignerView::AtomDeparted(const AuId& atomId)
{
	DaAtom* sub = m_SceneEther.FindAtom(atomId);
	if(sub)
		sub->DeleteDeviceObjects(*m_RenderInfo);	// Clean up the atom's rendering resources recursively.
	m_SceneEther.TrashAtom(atomId);
}

void AuDesignerView::AtomParentChanged(const DaAtom* atom)
{
	DaPerson* sceneAtom;
	LockSceneAtomForWrite(&sceneAtom);
	DaAtom* pMyChild = m_SceneEther.FindAtom(atom->Id());
	if(!pMyChild)
	{
		UnlockSceneAtom();
		return;
	}
	AuAtomPos apos(atom->Pos());
	AuAtomPos mpos(pMyChild->Pos());
	pMyChild->ChangePos(apos);
	pMyChild->InvalidateEnvironment();

	// Controls must be updated when the scene atom's parent changes to or from a vehicle.
	if(atom->Is(m_idSceneAtom))
	{
		if(apos.ParentId() != mpos.ParentId())
		{
			m_ControlLock.lock();
			if(m_pControlsAtom)
				m_pControlsAtom->clear();
			else
				m_pControlsAtom = new DiControlInputArray;
			pMyChild->EnumControlInputs(*m_pControlsAtom);			// Get controls based on child atom's new parent.
			m_ControlLock.unlock();
		}
	}
	UnlockSceneAtom();
}

void AuDesignerView::AtomChanged(DaAtom* atom)
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

		// Controls must be updated when the scene atom enters or exits a console.
		// Note that the console can change without entering or exiting at the captain's chair/command console.
		DaPerson* changedPerson = (DaPerson*)atom;
		if(sceneAtom->HasConsole() != changedPerson->HasConsole() ||
				(sceneAtom->HasConsole() && changedPerson->HasConsole() &&
				 sceneAtom->CurrentGeneralPose() != changedPerson->CurrentGeneralPose()))
		{
			m_ControlLock.lock();
			if(m_pControlsAtom)
				m_pControlsAtom->clear();
			else
				m_pControlsAtom = new DiControlInputArray;
			changedPerson->EnumControlInputs(*m_pControlsAtom);	// Get controls based on changed atom's situation.
			m_ControlLock.unlock();
		}
	}
	else
	{
		pMyAtom = m_SceneEther.FindAtom(atom->Id());
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
	pMyAtom->Refresh(atom, m_RenderInfo);
	if(invalidEnv)
		sceneAtom->InvalidateEnvironment();
	UnlockSceneAtom();

	// Kick user interfaces to update in response to changes in the designer, i.e. texture unwrapper.
	if(IsInNewDesignEnvironment() && !IsDesignPreview())
		emit DesignerSelectionChanged();
}

void AuDesignerView::AtomsCleared()
{
	m_SceneLock.lockForWrite();
	m_SceneEther.DeleteDeviceObjects(*m_RenderInfo);
	m_SceneEther.DeleteEther();
	m_SceneLock.unlock();
	m_SceneAtomTransition = true;
	m_SceneRendered = false;
}

// Warn against losing changes to the current design. Provide an option to cancel.
// Returns true if it's ok to proceed, false if user wants to cancel.
bool AuDesignerView::DesignerConfirmLoseChanges()
{
	// No confirmation required when design cannot be saved.
	if(!DesignIsSavable())
		return true;

	if(!IsInDesignEnvironment())
		return true;

	// Determine if current design has been modified.
	bool designIsModified = false;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesignBase* pde = sceneAtom->FindParentDesignBaseInEnv();
		if(pde)
			designIsModified = pde->IsDesignModified();
	}
	UnlockSceneAtom();

	// No confirmation required when design has not been modified.
	if(!designIsModified)
		return true;

	// Ask the user to confirm an action that will cause them to lose their changes.
	QString smsg = tr("<html>The current design was modified. "
			"Changes could be lost.</html>");
	QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Information, QMessageBox::Ok, QMessageBox::Cancel,
			QMessageBox::NoButton, auApp->Frame());
	int result = auApp->Frame()->ExecMessageBox(mb);
	return result == QMessageBox::Ok;
}

bool AuDesignerView::DesignIsGravityOn() const
{
	return auApp->DesignIsGravityOn();
}

bool AuDesignerView::DesignIsRenderShaded() const
{
	return auApp->DesignIsRenderShaded();
}

bool AuDesignerView::DoesSceneOwnDesign()
{
	if(!IsInDesignEnvironment())
		return false;

	bool isOwner = false;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesignBase* pde = sceneAtom->FindParentDesignBaseInEnv();
		if(pde)
			isOwner = (pde->Owner() == m_idSceneAtom) ? true : false;
	}
	UnlockSceneAtom();
	return isOwner;
}

bool AuDesignerView::DesignIsSavable()
{
	//if(!IsDesignMode())
		//return 0;

	if(SceneAtomId().IsNull())
		return false;

	bool isSavable = false;
	switch((DaDesignBase::DESIGNMODE)DesignMode())
	{
	case DaDesignBase::InvalidMode:
	case DaDesignBase::DESIGN_Unknown:
		isSavable = true;
		break;
	case DaDesignBase::DESIGN_Spacecraft:
	case DaDesignBase::DESIGN_Building:
	case DaDesignBase::DESIGN_Vehicle:
	case DaDesignBase::DESIGN_Item:
		isSavable = !DesignerIsViewOnly();
		break;
	}
	return isSavable;
}

QString AuDesignerView::DesignAnalysis()
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
					da = QString("<b>%1</b><br>%2<br>%3<br>Blueprint Number %4<br>Manufactured %5<hr>")
						.arg(ship->Name())
						.arg(empireName)
						.arg(ship->IsPrivatelyOwned()?"Company":"Fleet")
						.arg(ship->DesignId())
						.arg(ship->CreatedTime().UTC().toLocalTime().date().toString());
				else
					da = QString("<b>%1</b><br>%2<br>%3<br>Blueprint Number %4<br>Manufactured %5<br>%6<hr>")
						.arg(ship->Name())
						.arg(empireName)
						.arg(ship->IsPrivatelyOwned()?"Company":"Fleet")
						.arg(ship->DesignId())
						.arg(ship->CreatedTime().UTC().toLocalTime().date().toString())
						.arg(ship->WhereMade());
				da += ship->DesignReport(ship->QualityMade(), isAgent);
			}
		}
	}
	UnlockSceneAtom();
	return da;
}

// Look and perceive what this creature sees.
//NOTE: The scene is expected to already be locked.
// 'descAlso' must be true to update the text description of the target, which takes extra work.
void AuDesignerView::UpdateTarget(DaPerson* sceneAtom, bool descAlso)
{
	if(!sceneAtom || !m_SceneRendered || !s_SightTarget)
		return;

	// Locate closest object that is in front of the eye.
	const DaAtom* sightedAtom;
	AuPoint sightedLoc;			// SightTarget output parameter, relative to scene atom.
	unsigned int sightedPart = 0;	// SightTarget output parameter.
	bool sightedShield = false;
	if(g_Au.m_DebugSwitches & 0x04000000)
		sightedAtom = 0;
	else if(m_RenderInfo->IsViewTopDown())
	{
		// In top-down view we will report the terrain the scene atom is in, as if they spotted the ground under their feet.
		if(sceneAtom->IsInAtmosphere())
		{
			// Scene atom is inside the atmosphere of a planet. It must be a new-style world because there was no terrain patch.
			sightedAtom = sceneAtom->FindParentPlanetInEnv();
			sightedPart = 0;
			sightedShield = false;
		}
		else
		{
			// Scene atom is not subordinate to terrain at any level. Check for a spacecraft they might be aboard.
			sightedAtom = sceneAtom->FindParentCapsuleInEnv();
			if(sightedAtom)
			{
				// The scene atom is aboard a spacecraft.
				// 'sightedPart' will be 0 if the hull was hit or the detail id if a detail was hit, per DaVessel::SightTarget.
				sightedPart = 0;
				sightedShield = false;
			}
		}
	}
	else
	{
		// Not in top-down view so spot the target at the crosshair location.
		//qDebug() << "AuDesignerView::UpdateTarget sighting...";
		float shotgunRange, spreadAngle;
		if(sceneAtom->CurrentWeaponIsShotgun(shotgunRange, spreadAngle))
			sightedAtom = sceneAtom->SightShotgunTarget(sightedLoc, sightedPart, sightedShield, shotgunRange, spreadAngle);
		else
			sightedAtom = sceneAtom->SightTargetInSight(sightedLoc, sightedPart, sightedShield);
		//qDebug() << "AuDesignerView::UpdateTarget sighting done.";
	}

	POLITICAL_STANCE sightedStance = POL_Neutral;	// Default if we otherwise don't care about the item in the crosshair.
	bool sightedIsCivilian = true;
	AuId targetId;
	bool targetIsDirt = false;
	if(sightedAtom)
	{
		//qDebug() << "AuDesignerView::UpdateTarget sightedAtom" << sightedAtom->Id() << sightedAtom->Name()
			//<< "sightedPart" << sightedPart;
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(sightedAtom);
		if(llworld)
		{
			// The sighted atom is a globe world.
			targetId = llworld->Id();						// Targetted on dirt.
			targetIsDirt = true;

			// Buildings and other developments.
			AuMatrix4x4 sceneM;
			sceneAtom->GetTransformRelativeToParent(sceneM, llworld);
			AuGlobeLLF spotLL;
			if(llworld->LLOfLoc(spotLL, sceneM.Project(sightedLoc)))
			{
#if !defined(QT_NO_DEBUG)
				if(descAlso)
				{
					bool hideRocks = false;
					int hideGrass = 0;
					bool hideShrubs = false;
					bool hideTrees = false;
					WORLDTEX_TYPE pavementType = WORLDTEX_Count;
					COMMODITYID farmShrub = CID_Invalid;
					COMMODITYID orchardTree = CID_Invalid;
					double farmShrubMSL, orchardTreeMSL;
					double terAltMSL = llworld->TerrainAltitudeMSLPave(AuGlobeLL().FromGlobeLLF(spotLL), hideRocks, hideGrass,
							hideShrubs, hideTrees, pavementType, farmShrub, farmShrubMSL, orchardTree, orchardTreeMSL);
					qDebug() << "D AuDesignerView::UpdateTarget terrain altMSL" << terAltMSL << "hideRocks" << hideRocks
						<< "hideGrass" << hideGrass << "hideShrubs" << hideShrubs << "hideTrees" << hideTrees
						<< "pave" << pavementType << "farmShrub" << g_Au.CommodityName(farmShrub)
						<< "orchardTree" << g_Au.CommodityName(orchardTree);
				}
#endif
			}
		}
		else
		{
			targetId = sightedAtom->Id();	// Targetted on an atom.
			const DaPerson* sightedAvatar = dynamic_cast<const DaPerson*>(sightedAtom);
			if(sightedAvatar)
			{
				sightedStance = PoliticalStance(sceneAtom->EmpireId(), sightedAvatar->EmpireId());
				sightedIsCivilian = sightedAvatar->IsCivilian();
			}
			else
			{
				const DaUnit* sightedUnit = dynamic_cast<const DaUnit*>(sightedAtom);
				if(sightedUnit)
				{
					sightedStance = PoliticalStance(sceneAtom->EmpireId(), sightedUnit->EmpireId());
					sightedIsCivilian = sightedUnit->IsCivilian();
				}
			}
		}

		if(descAlso)
		{
			// Get DNA of targeted creature.
			const DaCreature* creature = dynamic_cast<const DaCreature*>(sightedAtom);
			if(creature)
			{
				if(m_TargetDNA)
					delete m_TargetDNA;
				m_TargetDNA = creature->DNAnew();
			}

			if(llworld)
			{
				// We sight globe worlds so they blocks shots, but we really don't want to "target" on dirt.
				// Calculate target location relative to planet.
				m_TargetLock.lockForWrite();
				m_TargetSpotId = llworld->Id();
				m_TargetLock.unlock();
				AuMatrix4x4 tScene;
				if(sceneAtom->GetTransformRelativeToParent(tScene, llworld))
				{
					m_TargetLock.lockForWrite();
					m_TargetSpotLoc = tScene.Project(sightedLoc);
					m_TargetLock.unlock();
				}
			}
			else
			{
				// The sighted atom is not a terrain patch or a globe world.
				m_TargetLock.lockForWrite();
				m_TargetSpotId = targetId;
				m_TargetLock.unlock();

				// Calculate the sighted location relative to the target atom.
				const DaAtom* envAtom = sceneAtom->FindEnvironmentAtom();
				Q_ASSERT(envAtom);

				// Calculate target location relative to envAtom.
				AuPoint loc;
				AuMatrix4x4 tScene;
				if(sceneAtom->GetTransformRelativeToParent(tScene, envAtom))
					loc = tScene.Project(sightedLoc);			// Sighted location relative to the environment.
				AuMatrix4x4 tTarget;
				if(sightedAtom->GetTransformRelativeToParent(tTarget, envAtom))
				{
					m_TargetLock.lockForWrite();
					m_TargetSpotLoc = tTarget.UnProject(loc);	// Sighted location relative to the target atom.
					m_TargetLock.unlock();
				}
			}

			QString targetDesc = sightedAtom->TargetDesc(SpottedLocation(), sceneAtom->EmpireId(), targetId, sightedStance);
//#if !defined(QT_NO_DEBUG)
			//targetDesc = QString("%1 %2").arg(sightedAtom->Id().GetID()).arg(targetDesc);
//#endif
			m_TargetLock.lockForWrite();
			m_TargetDesc = targetDesc;
			m_TargetLock.unlock();
		}
	}
	else if(descAlso)
	{
		m_TargetLock.lockForWrite();
		m_TargetDesc = "";
		m_TargetLock.unlock();
	}

	// The result affects our target acquisition info.
	m_TargetLock.lockForWrite();
	if(targetId.IsNull())
	{
		// Not targeted on anything. Decrease target level of last target.
		if(m_TargetLevel)
			m_TargetLevel /= 2;
		if(m_TargetLevel == 0)
		{
			// Lost the target.
			m_TargetId = AuId();	// null target id
			m_TargetLevel = 0;
			m_TargetLoc = AuPoint(0,0);
			m_TargetPart = 0;
			m_TargetShield = false;
			m_TargetStance = POL_Neutral;
			m_TargetIsCivilian = false;
		}
	}
	else if(targetIsDirt)
	{
		// Targeted on dirt. Decrease target level.
		if(m_TargetLevel)
			m_TargetLevel /= 2;
		if(m_TargetLevel == 0)
		{
			// Lost the target.
			m_TargetId = targetId;	// dirt target id
			m_TargetLevel = 0;
			m_TargetLoc = sightedLoc;
			m_TargetPart = 0;
			m_TargetShield = false;
			m_TargetStance = POL_Neutral;
			m_TargetIsCivilian = false;
		}
	}
	else if(targetId == m_TargetId)
	{
		// Still targeted on the same atom. Increase the target level.
		if(m_TargetPart == sightedPart)
		{
			// Still targeted on the same part of the same atom.
			if(m_TargetLevel)
				m_TargetLevel *= 2;
			else
				m_TargetLevel = 1;
		}
		else
		{
			// Targeted on different part of same atom.
			m_TargetLevel++;
			m_TargetPart = sightedPart;
		}

		if(m_TargetLevel > 15)
			m_TargetLevel = 15;
		m_TargetLoc = sightedLoc;	// sighted loc on target
		m_TargetShield = sightedShield;
		m_TargetStance = sightedStance;
		m_TargetIsCivilian = sightedIsCivilian;
	}
	else
	{
		// Targeted on a different atom.
		m_TargetId = targetId;
		m_TargetLevel /= 3;
		m_TargetPart = sightedPart;
		m_TargetLoc = sightedLoc;	// sighted loc on target
		m_TargetShield = sightedShield;
		m_TargetStance = sightedStance;
		m_TargetIsCivilian = sightedIsCivilian;
	}

#if !defined(QT_NO_DEBUG)
	if(descAlso && !m_TargetId.IsNull())
	{
		if(sightedAtom)
			qDebug() << "D AuDesignerView::UpdateTarget atom sighted" << *sightedAtom
				<< "parent" << *sightedAtom->Parent()
				<< "targetId" << m_TargetId << "lvl" << m_TargetLevel
				<< "part" << m_TargetPart << "shield" << m_TargetShield << "at" << sightedLoc << "targLoc" << m_TargetLoc
				<< "dist" << m_TargetLoc.Length() << "'" << round(m_TargetLoc.Length() / FeetPerMeter()) << "m"
				<< "spotId" << m_TargetSpotId;
		else
			qDebug() << "D AuDesignerView::UpdateTarget atom NOT SIGHTED targetId" << m_TargetId << "lvl" << m_TargetLevel << "part"
				<< m_TargetPart << "shield" << m_TargetShield << "spotId" << m_TargetSpotId;
	}
#endif
	m_TargetLock.unlock();
}

// Make sure this is the topmost widget for input.
void AuDesignerView::ActivateWindow()
{
#if !defined(Q_OS_MACOS)
	//AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	//if(!pFrame->isActiveWindow())
	{
		//qDebug() << "AuDesignerView::ActivateWindow" << AuTime::Now();
		//activateWindow();
		raise();
		setFocus();
	}
#endif
}

QString AuDesignerView::TargetDescription()
{
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	UpdateTarget(sceneAtom, true);					// Do the extra work of getting the description in m_TargetDesc.
	UnlockSceneAtom();
	m_TargetLock.lockForRead();
	QString targetDesc(m_TargetDesc);
	m_TargetLock.unlock();
	return targetDesc;
}

void AuDesignerView::ChatReceived(AuCommPacket* msg)
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
					DaNpc* speaker = dynamic_cast<DaNpc*>(m_SceneEther.FindAtom(msg->FromId()));
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
		// Show the image if it's not going to appear on the story dialog. It's confusing to have it appear in both places.
		if(!g_AuClient.ShowStoryDialog())
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
		// Show the image if it's not going to appear on the story dialog. It's confusing to have it appear in both places.
		if(!g_AuClient.ShowStoryDialog())
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

	//qDebug("AuDesignerView::ChatReceived");
	AddMessage(AuRenderMsg(msg));
}

void AuDesignerView::RenderOverlayText()
{
	if(m_OverlayText.isEmpty())
		return;
	if(m_OverlayTime.elapsed()/d_MSecPerSec > 2)
	{
		m_OverlayText = "";
		return;
	}
	if(!m_RenderInfo->m_ShowHUD)
		return;
	if(!m_MessageIntensity)
		return;
	if(!m_RenderInfo->re)
		return;

	// Recent communications overlay the scene.
	double pxPerUnit = (double)rect().height()/2.;	// OpenGL screen height is 2 units, -1 to 1.
	int vbRadius = ViewButtonRadius() * pxPerUnit;
	int vbMargin = vbRadius / 2.;
	int left = vbMargin + (vbRadius*2 + vbMargin) * m_ViewButtonColumns;

	m_RenderInfo->re->glEnable(GL_BLEND);
	m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	QColor color(32,192,64);
	float ang = (float)m_OverlayTime.elapsed()/2000.f * PIf/2.f;	// 0 to 90 degrees
	float alpha = cosf(ang);
	if(alpha > 0)
		color.setAlphaF(alpha);
	else
		color.setAlphaF(0);

	float intFact = (float)m_MessageIntensity/3.f;
	color.setRedF(color.redF() * intFact);
	color.setGreenF(color.greenF() * intFact);
	color.setBlueF(color.blueF() * intFact);

	QColor shadColor(0,0,0);
	shadColor.setAlphaF(color.alphaF());

	// This text is centered.
	int cpos = width()/2;
	int vpos = height()*3/4;
	int wrapWidth = width() - left*2;

	// Draw the text in black for drop shadow.
	m_RenderInfo->re->RenderText(cpos, vpos, m_OverlayText, g_AuClient.FontAtom(), shadColor, width(), height(),
			Qt::AlignCenter|Qt::TextWordWrap, wrapWidth/2);
	// Draw the colored text on top of the black text.
	m_RenderInfo->re->RenderText(cpos-1, vpos-2, m_OverlayText, g_AuClient.FontAtom(), color, width(), height(),
			Qt::AlignCenter|Qt::TextWordWrap, wrapWidth/2);

	m_RenderInfo->re->glDisable(GL_BLEND);
}

void AuDesignerView::RenderCommMessages()
{
	//qDebug("AuDesignerView::RenderCommMessages %d messages intensity %d", m_Messages.count(), m_MessageIntensity);
	if(!m_RenderInfo->m_ShowHUD)
		return;
	if(!m_RenderInfo->re)
		return;
	m_MessageLock.lock();
	if(m_Messages.empty())
	{
		m_MessageLock.unlock();
		return;
	}

	// Recent communications overlay the scene.
	int rowPos = 50;	// Skip past the ship name and health bar that are often at the top.
	double pxPerUnit = (double)rect().height()/2.;	// OpenGL screen height is 2 units, -1 to 1.
	int vbRadius = ViewButtonRadius() * pxPerUnit;
	int vbMargin = vbRadius / 2.;
	int left = vbMargin + (vbRadius*2 + vbMargin) * m_ViewButtonColumns;

	int iLeft = m_Messages.count()-m_NumberOfMessages;
	for(int i=0; i<iLeft; i++)
		m_Messages.remove(0);

	m_RenderInfo->re->glEnable(GL_BLEND);
	m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Render recent messages.
	for(QVector<AuRenderMsg>::iterator mi = m_Messages.begin(); mi != m_Messages.end(); mi++)
	{
		bool isCentered = false;
		QColor color = (*mi).Color();
		const QFont* pfont;
		switch((*mi).Range())
		{
		default:
			// Error condition should stand out as a problem with this large bold font.
			pfont = &g_AuClient.FontTitle();
			break;
		case AuCommStdChannel::Close:
			pfont = &g_AuClient.FontClose();
			break;
		case AuCommStdChannel::Hail:
			pfont = &g_AuClient.FontHail();
			break;
		case AuCommStdChannel::SolarSystem:
			pfont = &g_AuClient.FontSystem();
			break;
		case AuCommStdChannel::Sector:
			pfont = &g_AuClient.FontSector();
			break;
		case AuCommStdChannel::Galaxy:
			pfont = &g_AuClient.FontGalaxy();
			break;
		case AuCommStdChannel::Universe:
			pfont = &g_AuClient.FontUniverse();
			break;
		}

		// Story line narration.
		if((*mi).Type() == AuCommPacket::NarrateTitle)
		{
			isCentered = true;
			pfont = &g_AuClient.FontTitle();
			float alpha = 1.f - (float)(*mi).MSecsOnDisplay()/(float)((s_TitleFadeTime+1)*1000);
			if(alpha > 0)
				color.setAlphaF(alpha);
			else
				color.setAlphaF(0);
		}

		// NOTE: Each message will return a sound effect to play only once. Subsequent ToneToPlay calls will return SOUND_None.
		//       That way each message does not make a sound in every frame that we render it, only the first frame that it is
		//       rendered.
		SOUND_EFFECT sound = (SOUND_EFFECT)(*mi).ToneToPlay();
		if(sound != SOUND_None)
		{
			auApp->PlayClientSound(sound, m_Fade);

			// Check point save the game when a title is displayed.
			if(g_Au.IsSceneSolo() && (*mi).Type() == AuCommPacket::NarrateTitle)
			{
				QString name = QString("%1.%2")
					.arg(SceneAtomName())
					.arg((*mi).Text());
				auApp->SaveCheckPoint(name);
			}
		}

		// Remember the time we displayed the message so it can begin to decay.
		(*mi).SetDisplayed();

		// Don't actually render anything if the message intensity is off. We do want to hear the sound effect and set the displayed
		// time so the message decays from the screen, even though it's not visible.
		if(!m_MessageIntensity)
			continue;

		float intFact = (float)m_MessageIntensity/3.f;
		color.setRedF(color.redF() * intFact);
		color.setGreenF(color.greenF() * intFact);
		color.setBlueF(color.blueF() * intFact);

		QColor shadColor(0,0,0);
		shadColor.setAlphaF(color.alphaF());

		QString text = (*mi).OnScreenMessage();
		if(isCentered)
		{
			// This text is centered, probably the title of a story block.
			// It doesn't affect the row pos because we put it in the center
			// of the screen. We assume that centered text does not word wrap.
			int cpos = width()/2;
			int vpos = height()/3;
			int wrapWidth = width() - left*2;

			// Draw the text in black for drop shadow.
			m_RenderInfo->re->RenderText(cpos, vpos, text, *pfont, shadColor, width(), height(), Qt::AlignCenter|Qt::TextWordWrap,
					wrapWidth/2);
			if(intFact > 0)
			{
				// Draw the colored text on top of the black text.
				// When the text is black, only one layer of text is drawn.
				m_RenderInfo->re->RenderText(cpos-1, vpos-2, text, *pfont, color, width(), height(),
						Qt::AlignCenter|Qt::TextWordWrap, wrapWidth/2);
			}
		}
		else
		{
			// Draw the text in black for drop shadow.
			int wrapWidth = width() - left;
			int rowHt = m_RenderInfo->re->RenderText(left, rowPos, text, *pfont, shadColor, width(), height(),
					Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, wrapWidth);
			if(intFact > 0)
			{
				// Draw the colored text on top of the black text.
				// When the text is black, only one layer of text is drawn.
				m_RenderInfo->re->RenderText(left-1, rowPos-2, text, *pfont, color, width(), height(),
						Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, wrapWidth);
				rowHt += 2;
			}
			rowPos += rowHt;
		}

		// Stop if we've gotten below the bottom of the screen.
		if(rowPos >= height())
			break;
	}
	m_MessageLock.unlock();
	m_RenderInfo->re->glDisable(GL_BLEND);
}

void AuDesignerView::SpotTargetOrTerrain(bool logOnly)
{
	//qDebug("AuDesignerView::SpotTargetOrTerrain %s", logOnly?"Log":"Current Channel");
	QString targetDesc = TargetDescription();			// Update target description.
	if(!targetDesc.isEmpty())
	{
		m_TargetLock.lockForRead();
		POLITICAL_STANCE targetStance = m_TargetStance;
		bool targetIsCivilian = m_TargetIsCivilian;
		m_TargetLock.unlock();
		emit SpotTargetOrTerrain(targetDesc, targetStance, targetIsCivilian, logOnly);
	}
}

AuDesignerView::INPUTMODE AuDesignerView::AssessInputMode() const
{
	//if(!isActiveWindow())
		//return UI;			// UI mode when this window is not the active window
	if(!m_MouseGrabbed && !underMouse())
		return UI;			// UI mode when this window is not under the mouse.
	if(m_ModalDlgCount)
		return UI;			// UI mode when a modal dialog is up.
	else if(m_bFreeMouse)
		return UI;			// Mouse freed for chatting/ui.
	else if(m_idSceneAtom.IsNull())
		return UI;			// No scene atom so operate in ui mode.
	else if(m_bFreeCamera)
		return UI;			// UI mode when camera is free.
	else if(m_HandInputTemporary)
		return UI;			// Input mode temporarily released for ui.
	return Character;
}

void AuDesignerView::SpawnRecall()
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

void AuDesignerView::SpawnTie()
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

void AuDesignerView::SquadCreate(int squadIndex)
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

void AuDesignerView::SquadSelect(int squadIndex, bool addToCurrentSelection)
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

void AuDesignerView::SquadIssueOrder(CITIZEN_ORDER order, const QString& orderText)
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
			m_OrderTime.restart();
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
		AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
		AuCommCitizenOrder msg(ordersTo, order, orderAtom, orderLoc.x, orderLoc.y, orderLoc.z, orderText,
				pFrame->OrdersChannel(), m_idSceneAtom, SceneAtomName(), SceneSector(), SceneGalaxy(), AuCommStdChannel::Universe,
				AuCommStdChannel::Scope(pFrame->OrdersChannel()));
		pFrame->SendChat(msg);
	}
}

void AuDesignerView::SquadIssuePosture(CITIZEN_POSTURE posture)
{
	AuIdList ordersTo;
	for(AuSelectedUnitArray::const_iterator ti = m_Selected.constBegin(); ti != m_Selected.constEnd(); ti++)
		ordersTo.append((*ti).Id());
	if(!ordersTo.isEmpty())
	{
		// The channel will adjust the range in my message to the channel range setting.
		AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
		AuCommCitizenPosture msg(ordersTo, posture,
				pFrame->OrdersChannel(), m_idSceneAtom, SceneAtomName(), SceneSector(), SceneGalaxy(), AuCommStdChannel::Universe,
				AuCommStdChannel::Scope(pFrame->OrdersChannel()));
		pFrame->SendChat(msg);
	}
}

void AuDesignerView::LocalLogMessage(const QString& text)
{
	// NOTE: ChatReceived locks the scene atom only when the message type is AuCommMessage::ChatAudio.
	AuCommThought msg(text, m_idSceneAtom, SceneAtomName());

	// Send the message to the frame so it ends up in the comm Thoughts channel.
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	pFrame->ChatReceived(msg);
}

void AuDesignerView::RenderTactical(DaPerson* sceneAtom, DaAtom* pEnvironment)
{
	// No tactical information is rendered when message intensity is off.
	if(!m_MessageIntensity)
		return;
	if(!m_RenderInfo->m_ShowHUD)
		return;
	if(sceneAtom->RemoteCameraId())
		return;

	// Multiply by inverse of subatom transformation (Translation, rotation, and scale.)
	AuMatrix4x4 tsub;
	sceneAtom->GetTransformRelativeToParent(tsub, pEnvironment);
	AuMatrix4x4 tmodel = m_tCamera.Inverse() * tsub.Inverse();

	// Highlight all selectable units in top-down view.
	if(m_RenderInfo->IsViewTopDown())
	{
		m_RenderInfo->re->glDepthMask(false);
		m_RenderInfo->re->glDisable(GL_DEPTH_TEST);

		m_RenderInfo->re->GLSL().LineShader().Select();
		m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(m_tProjection);

		QList<AuId> atomIds = m_SceneEther.AtomIds();	// List of all atom ids subordinate to root at all levels.
		for(QList<AuId>::const_iterator si = atomIds.cbegin(); si != atomIds.cend(); si++)
		{
			const DaUnit* unit = dynamic_cast<const DaUnit*>(m_SceneEther.FindLiveAtom(*si));
			if(!unit)
				continue;	// Only allow select of unit atoms.
			if(!unit->UnitCanBeSelected(sceneAtom, m_RenderInfo->IsViewTopDown()))
				continue;

			QColor color = PoliticalStanceColor(sceneAtom->EmpireId(), unit->EmpireId(), unit->IsCivilian());
			float r = color.redF() * (float)m_MessageIntensity/3.f;
			float g = color.greenF() * (float)m_MessageIntensity/3.f;
			float b = color.blueF() * (float)m_MessageIntensity/3.f;
			if(!m_Selected.Contains(unit->Id()))
			{
				// Unselected units are dimmer.
				r *= .6f;
				g *= .6f;
				b *= .6f;
			}
			m_RenderInfo->re->GLSL().LineShader().SetColor(r,g,b,1);

			// Found a unit in the scene. Where is it on the screen?
			// Get the reverse transformation back up to the parent.
			AuMatrix4x4 tsel;
			unit->GetTransformRelativeToParent(tsel, pEnvironment);
			tsub = tmodel * tsel;
			if(dynamic_cast<const DaCreature*>(unit))
				tsub.Scale(2, 2, 1);			// The larger box on people helps them to be visible from above.
			else
				tsub.Scale(1.1, 1.1, 1);		// Vehicles need a smaller margin to look nice.

			AuBox fb;
			unit->RoughExtents(fb);
			fb.Transform(tsub);
			
			const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(unit);
			if(vehicle && vehicle->CommodityId() == CID_SpaceRocket)
			{
				// Rocket stands on its tail.
				// Compile a list of vertex locations.
				AuPointArray verts;
				verts.append(fb.m_fCorners[1]);
				verts.append(fb.m_fCorners[2]);
				verts.append(fb.m_fCorners[6]);
				verts.append(fb.m_fCorners[5]);

				// Make a VAO of the vertex locations.
				AuGLVAO vao;
				vao.CreateForLines(*m_RenderInfo->re, verts, true, GL_STREAM_DRAW);
				if(vao.VAO())
				{
					// Display the VAO once and clean it up.
					vao.SelectGL();
					vao.DrawGL(GL_LINE_STRIP);
					vao.DeselectGL();
					vao.DeleteGL();
				}
			}
			else
			{
				// Compile a list of vertex locations.
				AuPointArray verts;
				verts.append(fb.m_fCorners[4]);
				verts.append(fb.m_fCorners[5]);
				verts.append(fb.m_fCorners[6]);
				verts.append(fb.m_fCorners[7]);

				// Make a VAO of the vertex locations.
				AuGLVAO vao;
				vao.CreateForLines(*m_RenderInfo->re, verts, true, GL_STREAM_DRAW);
				if(vao.VAO())
				{
					// Display the VAO once and clean it up.
					vao.SelectGL();
					vao.DrawGL(GL_LINE_STRIP);
					vao.DeselectGL();
					vao.DeleteGL();
				}
			}
		}

		m_RenderInfo->re->glEnable(GL_DEPTH_TEST);
		m_RenderInfo->re->glDepthMask(true);

		m_RenderInfo->re->GLSL().LineShader().Deselect();
	}

	// Highlight selected item for manipulation
	if(m_ItemManipulator && m_ItemManipulator->isVisible() && !m_ItemManipulator->ItemId().IsNull())
	{
		// Highlight selected item to manipulate.
		DaItem* selItem = dynamic_cast<DaItem*>(m_SceneEther.FindAtom(m_ItemManipulator->ItemId()));
		if(selItem)
		{
			QColor color(255,0,255);
			float r = color.redF() * (float)m_MessageIntensity/3.f;
			float g = color.greenF() * (float)m_MessageIntensity/3.f;
			float b = color.blueF() * (float)m_MessageIntensity/3.f;

			// Get the reverse transformation back up to the parent.
			AuMatrix4x4 tsel;
			selItem->GetTransformRelativeToParent(tsel, pEnvironment);
			tsub.Scale(1.1, 1.1, 1.1);
			tsub = tmodel * tsel;

			// Get the rough extents of this atom so we can highlight it with a line and its name.
			AuBox fb;
			selItem->RoughExtents(fb);
			fb.Transform(tsub);

			m_RenderInfo->re->GLSL().LineShader().Select();
			m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(m_tProjection);
			m_RenderInfo->re->GLSL().LineShader().SetColor(r,g,b,1);

			AuPoint center = (fb.m_fCorners[0] + fb.m_fCorners[6])/2;
			AuPoint top = (fb.m_fCorners[4] + fb.m_fCorners[6])/2;

			// Compile a list of vertex locations.
			AuPointArray verts;
			verts.append(top);
			verts.append(center);

			// Make a VAO of the vertex locations.
			AuGLVAO vao;
			vao.CreateForLines(*m_RenderInfo->re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_LINES);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			m_RenderInfo->re->GLSL().LineShader().Deselect();

			// Put the unit's name at the top of the line.
			m_RenderInfo->re->glDepthMask(false);
			m_RenderInfo->re->glDisable(GL_DEPTH_TEST);
			m_RenderInfo->re->glEnable(GL_BLEND);
			m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			QColor tcolor;
			tcolor.setRgbF(r,g,b,1);
			m_RenderInfo->re->RenderText(m_tProjection, AuMatrix4x4(), top, g_Au.CommodityName(selItem->CommodityId()),
					g_AuClient.FontAtom(), tcolor, height(), Qt::AlignBottom|Qt::AlignHCenter);
			m_RenderInfo->re->glDisable(GL_BLEND);
			m_RenderInfo->re->glEnable(GL_DEPTH_TEST);
			m_RenderInfo->re->glDepthMask(true);
		}
	}

	// Highlight the selected atom(s).
	if(!m_Selected.isEmpty())
	{
		for(AuSelectedUnitArray::const_iterator si = m_Selected.constBegin(); si != m_Selected.constEnd(); si++)
		{
			DaUnit* selUnit = dynamic_cast<DaUnit*>(m_SceneEther.FindAtom((*si).Id()));
			if(!selUnit)
				continue;	// Selected atom is no longer in the scene.

			QColor color = PoliticalStanceColor(sceneAtom->EmpireId(), selUnit->EmpireId(), selUnit->IsCivilian());
			float r = color.redF() * (float)m_MessageIntensity/3.f;
			float g = color.greenF() * (float)m_MessageIntensity/3.f;
			float b = color.blueF() * (float)m_MessageIntensity/3.f;

			// Get the reverse transformation back up to the parent.
			AuMatrix4x4 tsel;
			selUnit->GetTransformRelativeToParent(tsel, pEnvironment);
			tsub = tmodel * tsel;

			// Get the rough extents of this atom so we can highlight it with a line and its name.
			AuBox fb;
			selUnit->RoughExtents(fb);
			double heightOfExtents = fabs(fb.m_fCorners[4].z - fb.m_fCorners[0].z);
			double heightOfLine = heightOfExtents/2 + .5;	// A little above the head so it isn't IN the head
			fb.Transform(tsub);

			m_RenderInfo->re->GLSL().LineShader().Select();
			m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(m_tProjection);
			m_RenderInfo->re->GLSL().LineShader().SetColor(r,g,b,1);

			AuPoint center = (fb.m_fCorners[0] + fb.m_fCorners[6])/2;
			AuPoint upVec(tsub.ProjectVector(selUnit->GravityNormal() * heightOfLine));
			AuPoint top = center + upVec;

			// Compile a list of vertex locations.
			AuPointArray verts;
			verts.append(top);
			verts.append(center);

#if !defined(QT_NO_DEBUG)
			// Bottom
			verts.append(fb.m_fCorners[0]);
			verts.append(fb.m_fCorners[1]);

			verts.append(fb.m_fCorners[1]);
			verts.append(fb.m_fCorners[2]);

			verts.append(fb.m_fCorners[2]);
			verts.append(fb.m_fCorners[3]);

			verts.append(fb.m_fCorners[3]);
			verts.append(fb.m_fCorners[0]);

			// Top
			verts.append(fb.m_fCorners[4]);
			verts.append(fb.m_fCorners[5]);

			verts.append(fb.m_fCorners[5]);
			verts.append(fb.m_fCorners[6]);

			verts.append(fb.m_fCorners[6]);
			verts.append(fb.m_fCorners[7]);

			verts.append(fb.m_fCorners[7]);
			verts.append(fb.m_fCorners[4]);

			// Legs
			verts.append(fb.m_fCorners[0]);
			verts.append(fb.m_fCorners[4]);

			verts.append(fb.m_fCorners[1]);
			verts.append(fb.m_fCorners[5]);

			verts.append(fb.m_fCorners[2]);
			verts.append(fb.m_fCorners[6]);

			verts.append(fb.m_fCorners[3]);
			verts.append(fb.m_fCorners[7]);
#endif

			// Make a VAO of the vertex locations.
			AuGLVAO vao;
			vao.CreateForLines(*m_RenderInfo->re, verts, false, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_LINES);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			m_RenderInfo->re->GLSL().LineShader().Deselect();

			// Put the unit's name at the top of the line.
			m_RenderInfo->re->glDepthMask(false);
			m_RenderInfo->re->glDisable(GL_DEPTH_TEST);
			m_RenderInfo->re->glEnable(GL_BLEND);
			m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			QColor tcolor;
			tcolor.setRgbF(r,g,b,1);
			m_RenderInfo->re->RenderText(m_tProjection, AuMatrix4x4(), top, selUnit->Name(), g_AuClient.FontAtom(), tcolor, height(),
					Qt::AlignBottom|Qt::AlignHCenter);
			m_RenderInfo->re->glDisable(GL_BLEND);
			m_RenderInfo->re->glEnable(GL_DEPTH_TEST);
			m_RenderInfo->re->glDepthMask(true);
		}
	}
}

// Saves last known whereabouts string for current scene atom before exiting.
void AuDesignerView::SaveWhereabouts()
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

void AuDesignerView::PageAgent()
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
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

void AuDesignerView::ScreenShot()
{
	// Get path to Pictures folder from settings.
	QString picPath = g_Au.PictureDir();

	// Insure Pictures folder exists.
	QDir screensDir(picPath);
	if(!screensDir.exists())
		screensDir.mkpath(picPath);

	// Construct a file name.
	// No spaces makes it easy to copy them right to the web site. My shell script there doesn't like spaces in the names.
	// Windows, rsync and ssh don't like colons in the name.
	picPath += "/Hazeron." + QDateTime::currentDateTime().toString("MMM-d-yyyy.h-mm-ss-zzz") + ".png";

	// Save the screen shot.
	QImage screen = grabFramebuffer().convertToFormat(QImage::Format_RGB32);
	screen.save(picPath);
}

// This function captures the result returned by the server from SelectGame().
// Only the person who submitted the SelectGame will get this response.
void AuDesignerView::GameSelect(quint8 gameType, bool gameSelected)
{
	if(gameSelected)
	{
		// The game selection was accepted by the ship.
		// Eliminate previous game ui we still might have around.
		if(m_GameUI)
		{
			m_GameUI->close();
			m_GameUI->deleteLater();
			m_GameUI = 0;
		}

		// Display a modeless dialog to gather settings from the dealer for the current game.
		AuGame* game = AuGame::CreateGame(gameType);
		if(game)
		{
			// Set up the players in the game according to who's at the table.
			DaPerson* sceneAtom;
			LockSceneAtom(&sceneAtom);
			if(sceneAtom)
			{
				if(sceneAtom->IsInCapsule())
				{
					DaCapsule* cap = sceneAtom->FindParentCapsuleInEnv();
					if(cap)
						cap->GameSetPlayers(sceneAtom->StationId(), game, SceneAtomId());
				}

				// The game creates its settings ui; display it modelessly so players can discuss settings.
				m_GameUI = game->CreateSettingsUI(*sceneAtom, auApp->Frame());
				connect(m_GameUI, SIGNAL(Accepted(quint8, QByteArray)), auApp, SLOT(GameStart(quint8, QByteArray)));
			}
			else
				m_GameUI = 0;

			// Game object is no longer needed. The dialog does the rest.
			delete game;

			UnlockSceneAtom();

			if(m_GameUI)
				m_GameUI->show();
		}
	}
	else
	{
		QString smsg = tr("<html>The game was not started. "
				"A new game cannot be started while a game is active at the table."
				"</html>");
		QMessageBox mb(g_Au.ApplicationName(), smsg,
				QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, auApp->Frame());
		auApp->Frame()->ExecMessageBox(mb);
	}
}

void AuDesignerView::GameTurn(const AuId& personsTurn)
{
	// Don't do anything if it's not my turn.
	if(personsTurn != SceneAtomId())
		return;	// It is not my turn.

	if(m_GameUI && m_GameUI->isHidden())
	{
		// Get rid of the hidden game UI, whatever it is.
		m_GameUI->close();
		m_GameUI->deleteLater();
		m_GameUI = 0;
	}

	// Don't bring up another game UI within two seconds of the previous one going away.
	// The server sends us a GameTurn notification every two seconds. Sometimes when the user closes the dialog, a game turn
	// notification arrives a split second later. This causes another turn ui to pop up right away. This is confusing because
	// the game state is probably about to change and it won't be my turn any more anyway.
	if(m_GameUITime.elapsed()/d_MSecPerSec < 2)
		return;

	// It is my turn.
	// Create/update a game ui to get my turn info.
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		if(sceneAtom->IsInCapsule())
		{
			DaCapsule* cap = sceneAtom->FindParentCapsuleInEnv();
			if(cap)
			{
				// The game creates its turn ui. It returns 0 if it's not my turn. This way, when the capsule atom hasn't received
				// an update to the game data yet, my turn ui won't appear until it's ready.
				if(m_GameUI && !cap->GameTurnUIValid(sceneAtom->StationId(), SceneAtomId(), m_GameUI))
				{
					// Get rid of the incorrect game UI, whatever it is.
					m_GameUI->close();
					m_GameUI->deleteLater();
					m_GameUI = 0;
				}

				if(!m_GameUI)
					m_GameUI = cap->GameCreateTurnUI(sceneAtom->StationId(), SceneAtomId(), auApp->Frame());
			}
		}
	}
	UnlockSceneAtom();

	// Display the turn ui modelessly so players can interact while taking their turn.
	if(m_GameUI && m_GameUI->isHidden())
	{
		connect(m_GameUI, SIGNAL(Accepted(quint8, QByteArray)), SLOT(GameTurn(quint8, QByteArray)));
		connect(m_GameUI, SIGNAL(Quit()), SLOT(GameQuit()));
		m_GameUI->show();
	}
}

// The local player has quit the current game.
void AuDesignerView::GameQuit()
{
	// Start the game ui timer so we don't immediately display another game turn ui due to bad timing.
	m_GameUITime.start();
	auApp->GameQuit();
}

// The local player has finished their turn at a game table.
void AuDesignerView::GameTurn(quint8 gameType, QByteArray gameSettings)
{
	// Start the game ui timer so we don't immediately display another game turn ui due to bad timing.
	m_GameUITime.start();
	auApp->GameTurn(gameType, gameSettings);
}

void AuDesignerView::GameOver(quint8 gameType, const QByteArray& gameResult)
{
	// The game ended.
	// Get rid of the previous game UI.
	if(m_GameUI)
	{
		m_GameUI->close();
		m_GameUI->deleteLater();
		m_GameUI = 0;
	}

	// Display a modeless dialog to display the final game results.
	AuGame* game = AuGame::CreateGame(gameType);
	if(game)
	{
		// The game creates its finish ui; display it modelessly so players can discuss the results.
		// All players receive the finish message one time.
		m_GameUI = game->CreateFinishUI(gameResult, auApp->Frame());
		if(m_GameUI)
			m_GameUI->show();

		// Game object is no longer needed. The dialog does the rest.
		delete game;
	}
}

void AuDesignerView::ConfigureControls(const DiControlInputArray& aAllControls)
{
	// Update my controls with those passed to me.
	for(DiControlInputArray::const_iterator ci = aAllControls.constBegin(); ci != aAllControls.constEnd(); ci++)
	{
		// Locate the corresponding control in my controls array.
		for(DiControlInputArray::iterator myci = s_ViewControls.begin(); myci != s_ViewControls.end(); myci++)
		{
			if((*myci).GetId() == CI_Title)
				continue;	// Skip title entries.
			if((*myci).GetId() == (*ci).GetId())
			{
				// Found the corresponding control id. Copy in the new data.
				*myci = *ci;
				break;
			}
		}
	}
	SaveControls();
}

void AuDesignerView::ControlsChanged()
{
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);

	m_ControlLock.lock();

	if(m_pControlsUI)
		m_pControlsUI->clear();
	else
		m_pControlsUI = new DiControlInputArray;
	EnumControlInputs(*m_pControlsUI);

	if(sceneAtom)
	{
		if(m_pControlsAtom)
			m_pControlsAtom->clear();
		else
			m_pControlsAtom = new DiControlInputArray;
		sceneAtom->EnumControlInputs(*m_pControlsAtom);
	}

	m_ControlLock.unlock();

	UnlockSceneAtom();
}

void AuDesignerView::RestoreDefaultControls()
{
	// Some controls are reserved for programmers.
#if defined(QT_NO_DEBUG)
	const DgData& gd = g_Au.LockDataForRead();
	bool isProgrammer = gd.ClientPlayerIsProgrammer();
	g_Au.UnlockData();
#else
	bool isProgrammer = true;
#endif

	s_ViewControls.clear();
	for(int ic=0; ic<s_NumViewDefaults; ic++)
	{
		switch(s_ViewDefaults[ic].GetId())
		{
		default:
			break;
		case CI_FreeCamera:
			// Command available to programmer.
			if(!isProgrammer)
				continue;
			break;
		}

		s_ViewControls.append(s_ViewDefaults[ic]);
	}

	SaveControls();
}

void AuDesignerView::LoadControls() const
{
	// Some controls are reserved for programmers.
#if defined(QT_NO_DEBUG)
	const DgData& gd = g_Au.LockDataForRead();
	bool isProgrammer = gd.ClientPlayerIsProgrammer();
	g_Au.UnlockData();
#else
	bool isProgrammer = true;
#endif

	// Init with my default controls.
	s_ViewControls.clear();
	for(int ic=0; ic<s_NumViewDefaults; ic++)
	{
		switch(s_ViewDefaults[ic].GetId())
		{
		default:
			break;
		case CI_FreeCamera:
			// Command available to programmer.
			if(!isProgrammer)
				continue;
			break;
		}

		s_ViewControls.append(s_ViewDefaults[ic]);
	}

	// Load any overrides from the settings file.
	s_ViewControls.LoadSettings("Controls/Scene");

	/*
	for(DiControlInputArray::const_iterator ci = s_ViewControls.constBegin(); ci != s_ViewControls.constEnd(); ci++)
	{
		qDebug() << (*ci).Name() << "keyboard" << (*ci).Key() << (*ci).KeyModifier()
			<< "joystick" << (*ci).Joystick() << (*ci).JoystickAxisExclusive() << (*ci).JoystickIndex() << (*ci).JoystickSubIndex()
			<< "mouse" << (*ci).Mouse() << (*ci).MouseModifier() << (*ci).MouseWheel();
	}
	*/
}

void AuDesignerView::SaveControls() const
{
	// Store my controls in the settings file.
	s_ViewControls.SaveSettings("Controls/Scene");
}

// A scene atom should fill this array with control inputs it can accept.
void AuDesignerView::EnumControlInputs(DiControlInputArray& aAllControls) const
{
	// Load my static controls array if it hasn't been loaded yet. This data will only exist on the client since EnumControlInputs
	// is not called on the server.
	if(s_ViewControls.isEmpty())
		LoadControls();

	// Append my controls to the array. I must include titles because the settings UI calls this function to get started.
	for(DiControlInputArray::const_iterator ci = s_ViewControls.constBegin(); ci != s_ViewControls.constEnd(); ci++)
		aAllControls.append(*ci);
}

QString AuDesignerView::ControlBinding(CONTROLINPUT control) const
{
	if(s_ViewControls.isEmpty())
		LoadControls();

	if(control < 0 || control >= s_ViewControls.count())
		return "";

	for(DiControlInputArray::const_iterator ci = s_ViewControls.cbegin(); ci != s_ViewControls.cend(); ci++)
		if((*ci).GetId() == control)
		{
			Qt::Key key = (*ci).Key();
			if(key == Qt::Key_unknown)
				break;

			Qt::KeyboardModifiers keymod = (*ci).KeyModifier();
			return AuBindControls::ModifierName(keymod) + AuBindControls::KeyName(key);
		}

	return "";
}

QKeySequence AuDesignerView::ControlKeySequence(CONTROLINPUT control) const
{
	if(s_ViewControls.isEmpty())
		LoadControls();

	if(control < 0 || control >= s_ViewControls.count())
		return QKeySequence();

	for(DiControlInputArray::const_iterator ci = s_ViewControls.cbegin(); ci != s_ViewControls.cend(); ci++)
		if((*ci).GetId() == control)
		{
			Qt::Key key = (*ci).Key();
			if(key == Qt::Key_unknown)
				break;

			Qt::KeyboardModifiers keymod = (*ci).KeyModifier();
			return QKeySequence(key|keymod);
		}

	return QKeySequence();
}

// Handles controls that affect the game UI.
void AuDesignerView::ProcessControlInput(const DiControlInputArray& aActiveControls)
{
	if(aActiveControls.isEmpty())
		return;

	if(IsModalDlgUp())
	{
		//qDebug() << "AuDesignerView::ProcessControlInput modal dlg up" << aActiveControls.count();
		return;
	}

	for(DiControlInputArray::const_iterator ci = aActiveControls.constBegin(); ci != aActiveControls.constEnd(); ci++)
	{
		switch((*ci).GetId())
		{
		case CI_Title:
			// This one should never get activated. It is only used for titles in the settings UI.
			break;

		case CI_TopDownViewToggle:
			ToggleTopDownView();
			break;

		case CI_FirstThirdPersonViewToggle:
			if(m_RenderInfo->IsViewTopDown())
				ToggleTopDownView();
			else
				ToggleFirstThirdPersonView();
			break;

		case CI_TopDownNextInfo:
			// Increment up to the max, then wrap around to the beginning.
			if(m_RenderInfo->IsViewTopDown() && !IsInDesignEnvironment())
			{
				if(m_RenderInfo->TopViewInfo() < AuAtomRenderInfo::VI_CityNames)
					m_RenderInfo->SetTopViewInfo((AuAtomRenderInfo::TOP_VIEW_INFO)(m_RenderInfo->TopViewInfo() + 1));
				else
					m_RenderInfo->SetTopViewInfo(AuAtomRenderInfo::VI_None);
				m_OverlayText = m_RenderInfo->TopViewInfoName();
				m_OverlayTime.restart();
			}
			break;

		case CI_TopDownPrevInfo:
			// Decrement down to the beginning, then wrap around to the max.
			if(m_RenderInfo->IsViewTopDown() && !IsInDesignEnvironment())
			{
				if(m_RenderInfo->TopViewInfo() > AuAtomRenderInfo::VI_None)
					m_RenderInfo->SetTopViewInfo((AuAtomRenderInfo::TOP_VIEW_INFO)(m_RenderInfo->TopViewInfo() - 1));
				else
					m_RenderInfo->SetTopViewInfo(AuAtomRenderInfo::VI_CityNames);
				m_OverlayText = m_RenderInfo->TopViewInfoName();
				m_OverlayTime.restart();
			}
			break;

		case CI_ScreenShot:
			emit RequestScreenShot();	// Must be done by the gui thread.
			break;

		case CI_EndCommand:
			emit RequestEndCommand();
			DeselectAllUnits();
			StopCamera();
			break;

		case CI_FreeCamera:
			OnViewFreeflyingcamera();
			break;

		case CI_ScreenElementsToggle:
			m_RenderInfo->m_ShowHUD = !m_RenderInfo->m_ShowHUD;
			break;

		case CI_ViewDistanceDecrease:
			// Reduce view distance based on current view mode.
			switch(m_RenderInfo->ViewMode())
			{
			case AuAtomRenderInfo::VM_FirstPerson:
				m_RenderInfo->m_ViewDist1st -= .5f;
				if(m_RenderInfo->m_ViewDist1st < 0)
					m_RenderInfo->m_ViewDist1st = 0;
				g_Au.Settings().writeEntry("Camera/1stPersonDistance", m_RenderInfo->m_ViewDist1st);
				break;
			case AuAtomRenderInfo::VM_ThirdPerson:
				m_RenderInfo->m_ViewDist3rd -= .5f;
				if(m_RenderInfo->m_ViewDist3rd < 1)
					m_RenderInfo->m_ViewDist3rd = 1;
				g_Au.Settings().writeEntry("Camera/3rdPersonDistance", m_RenderInfo->m_ViewDist3rd);
				break;
			case AuAtomRenderInfo::VM_TopDown:
				m_RenderInfo->m_ViewAlt /= 2;
				if(m_RenderInfo->m_ViewAlt < 75)
					m_RenderInfo->m_ViewAlt = 2400;
				g_Au.Settings().writeEntry("Camera/TopDownAltitude", m_RenderInfo->m_ViewAlt);
				break;
			default:
				break;
			}
			break;

		case CI_ViewAngleDecrease:
			// Reduce view angle based on current view mode.
			switch(m_RenderInfo->ViewMode())
			{
			case AuAtomRenderInfo::VM_FirstPerson:
				m_RenderInfo->m_ViewAngle1st -= 1.f*DEG2RADf;
				if(m_RenderInfo->m_ViewAngle1st < 0)
					m_RenderInfo->m_ViewAngle1st = 0;
				g_Au.Settings().writeEntry("Camera/1stPersonAngle", m_RenderInfo->m_ViewAngle1st*RAD2DEGf);
				break;
			case AuAtomRenderInfo::VM_ThirdPerson:
				m_RenderInfo->m_ViewAngle3rd -= 1.f*DEG2RADf;
				if(m_RenderInfo->m_ViewAngle3rd < 0)
					m_RenderInfo->m_ViewAngle3rd = 0;
				g_Au.Settings().writeEntry("Camera/3rdPersonAngle", m_RenderInfo->m_ViewAngle3rd*RAD2DEGf);
				break;
			case AuAtomRenderInfo::VM_TopDown:
			default:
				break;
			}
			break;

		case CI_ViewDistanceIncrease:
			// Increase view distance based on current view mode.
			switch(m_RenderInfo->ViewMode())
			{
			case AuAtomRenderInfo::VM_FirstPerson:
				m_RenderInfo->m_ViewDist1st += .5f;
				if(m_RenderInfo->m_ViewDist1st > 10)
					m_RenderInfo->m_ViewDist1st = 10;
				g_Au.Settings().writeEntry("Camera/1stPersonDistance", m_RenderInfo->m_ViewDist1st);
				break;
			case AuAtomRenderInfo::VM_ThirdPerson:
				m_RenderInfo->m_ViewDist3rd += .5f;
				if(m_RenderInfo->m_ViewDist3rd > 20)
					m_RenderInfo->m_ViewDist3rd = 20;
				g_Au.Settings().writeEntry("Camera/3rdPersonDistance", m_RenderInfo->m_ViewDist3rd);
				break;
			case AuAtomRenderInfo::VM_TopDown:
				m_RenderInfo->m_ViewAlt *= 2;
				if(m_RenderInfo->m_ViewAlt > 2400)
					m_RenderInfo->m_ViewAlt = 75;
				g_Au.Settings().writeEntry("Camera/TopDownAltitude", m_RenderInfo->m_ViewAlt);
				break;
			default:
				break;
			}
			break;

		case CI_ViewAngleIncrease:
			// Increase view angle based on current view mode.
			switch(m_RenderInfo->ViewMode())
			{
			case AuAtomRenderInfo::VM_FirstPerson:
				m_RenderInfo->m_ViewAngle1st += 1.f*DEG2RADf;
				if(m_RenderInfo->m_ViewAngle1st > 45.f*DEG2RADf)
					m_RenderInfo->m_ViewAngle1st = 45.f*DEG2RADf;
				g_Au.Settings().writeEntry("Camera/1stPersonAngle", m_RenderInfo->m_ViewAngle1st*RAD2DEGf);
				break;
			case AuAtomRenderInfo::VM_ThirdPerson:
				m_RenderInfo->m_ViewAngle3rd += 1.f*DEG2RADf;
				if(m_RenderInfo->m_ViewAngle3rd > 60.f*DEG2RADf)
					m_RenderInfo->m_ViewAngle3rd = 60.f*DEG2RADf;
				g_Au.Settings().writeEntry("Camera/3rdPersonAngle", m_RenderInfo->m_ViewAngle3rd*RAD2DEGf);
				break;
			case AuAtomRenderInfo::VM_TopDown:
			default:
				break;
			}
			break;

		case CI_ConsoleUI:
			if(IsPersonAtGameTable())
			{
				// Display game table ui when not inside a design environment.
				if(!IsInDesignEnvironment())
					emit SelectGame();
			}
			break;

		case CI_InputModeHandTemporary:
			m_HandInputTemporary = true;
			break;
	
		case CI_InputModeToggle:
			m_bFreeMouse = !m_bFreeMouse;	// Free mouse from scene animator.
			if(m_bFreeMouse)
				emit ChatReleased();	// Give focus to the chat box.
			break;

		case CI_HudIntensityDecrease:
			g_AuClient.SetHUDIntensity(g_AuClient.HUDIntensity()-1);
			break;

		case CI_HudIntensityIncrease:
			g_AuClient.SetHUDIntensity(g_AuClient.HUDIntensity()+1);
			break;

		case CI_MessageIntensity:
			m_MessageIntensity++;
			if(m_MessageIntensity > 3)
				m_MessageIntensity = 0;
			break;

		case CI_SpotTargetComm:
			SpotTargetOrTerrain(false);
			break;

		case CI_SpotTargetLog:
			SpotTargetOrTerrain(true);
			break;

		// Squad creation.
		case CI_SquadCreate0:
			SquadCreate(0);
			break;
		case CI_SquadCreate1:
			SquadCreate(1);
			break;
		case CI_SquadCreate2:
			SquadCreate(2);
			break;
		case CI_SquadCreate3:
			SquadCreate(3);
			break;
		case CI_SquadCreate4:
			SquadCreate(4);
			break;
		case CI_SquadCreate5:
			SquadCreate(5);
			break;
		case CI_SquadCreate6:
			SquadCreate(6);
			break;
		case CI_SquadCreate7:
			SquadCreate(7);
			break;
		case CI_SquadCreate8:
			SquadCreate(8);
			break;
		case CI_SquadCreate9:
			SquadCreate(9);
			break;
		
		// Squad selection.
		case CI_SquadSelect0:
			SquadSelect(0, true);
			break;
		case CI_SquadSelect1:
			SquadSelect(1, true);
#if !defined(QT_NO_DEBUG)
			s_Animate = !s_Animate;
#endif
			break;
		case CI_SquadSelect2:
#if !defined(QT_NO_DEBUG)
			s_Create = !s_Create;
#endif
			SquadSelect(2, true);
			break;
		case CI_SquadSelect3:
			SquadSelect(3, true);
#if !defined(QT_NO_DEBUG)
			s_RenderComm = !s_RenderComm;
#endif
			break;
		case CI_SquadSelect4:
			SquadSelect(4, true);
#if !defined(QT_NO_DEBUG)
			s_RenderCursor = !s_RenderCursor;
#endif
			break;
		case CI_SquadSelect5:
			SquadSelect(5, true);
#if !defined(QT_NO_DEBUG)
			s_RenderEnv = !s_RenderEnv;
#endif
			break;
		case CI_SquadSelect6:
			SquadSelect(6, true);
#if !defined(QT_NO_DEBUG)
			s_RenderFade = !s_RenderFade;
#endif
			break;
		case CI_SquadSelect7:
			SquadSelect(7, true);
#if !defined(QT_NO_DEBUG)
			s_RenderScene = !s_RenderScene;
#endif
			break;
		case CI_SquadSelect8:
			SquadSelect(8, true);
#if !defined(QT_NO_DEBUG)
			s_RenderTactical = !s_RenderTactical;
#endif
			break;
		case CI_SquadSelect9:
			SquadSelect(9, true);
#if !defined(QT_NO_DEBUG)
			s_SightTarget = !s_SightTarget;
#endif
			break;

		// Squad orders.
		case CI_SquadOrderAttackTarget:
			TargetDescription();								// Update target information for the order.
			SquadIssueOrder(ORDER_AttackTarget);
			break;
		case CI_SquadOrderCloseDoor:
			SquadIssueOrder(ORDER_CloseDoor);
			break;
		case CI_SquadPostureDefendArea:
			SquadIssuePosture(POSTURE_DefendArea);
			break;
		case CI_SquadPostureDefendSelf:
			SquadIssuePosture(POSTURE_DefendSelf);
			break;
		case CI_SquadOrderFollowTarget:
			TargetDescription();								// Update target information for the order.
			SquadIssueOrder(ORDER_FollowTarget);
			break;
		case CI_SquadOrderGetItem:
			TargetDescription();								// Update target information for the order.
			SquadIssueOrder(ORDER_GetItem);
			break;
		case CI_SquadOrderComeHere:
			SquadIssueOrder(ORDER_ComeHere);
			break;
		case CI_SquadOrderGetIn:
			SquadIssueOrder(ORDER_GetIn);
			break;
		case CI_SquadOrderMountTarget:
			TargetDescription();								// Update target information for the order.
			SquadIssueOrder(ORDER_MountTarget);
			break;
		case CI_SquadPostureBerserk:
			SquadIssuePosture(POSTURE_Berserk);
			break;
		case CI_SquadPostureHoldFire:
			SquadIssuePosture(POSTURE_HoldFire);
			break;
		case CI_SquadOrderFollowMe:
			SquadIssueOrder(ORDER_FollowMe);
			break;
		case CI_SquadOrderBattleStation:
			SquadIssueOrder(ORDER_BattleStation);
			break;
		case CI_SquadOrderGetOut:
			SquadIssueOrder(ORDER_GetOut);
			break;
		case CI_SquadOrderOpenDoor:
			SquadIssueOrder(ORDER_OpenDoor);
			break;
		case CI_SquadOrderDropRider:
			SquadIssueOrder(ORDER_DropRider);
			break;
		case CI_SquadOrderGarrison:
			SquadIssueOrder(ORDER_Garrison);
			break;
		case CI_SquadOrderStay:
			SquadIssueOrder(ORDER_Stay);
			break;
		case CI_SquadOrderGoToTarget:
			TargetDescription();								// Update target information for the order.
			SquadIssueOrder(ORDER_GoToTarget);
			break;
		case CI_SquadOrderMountMe:
			SquadIssueOrder(ORDER_MountMe);
			break;
		case CI_SquadPostureOffensive:
			SquadIssuePosture(POSTURE_Offensive);
			break;
		case CI_SquadOrderReleaseCommand:
			SquadIssueOrder(ORDER_ReleaseCommand);
			break;
		case CI_SquadOrderEnterSurgery:
			SquadIssueOrder(ORDER_EnterSurgery);
			break;
		case CI_SquadOrderTakeCommand:
			SquadIssueOrder(ORDER_TakeCommand);
			break;

		// Designer
		case CI_DesignerGrab:
			// We must queue starting these designer commands because they are initiated in the control thread. That guy cannot call
			// StartCommand or EndCommand because it can't get the OpenGL context, and it can't create UI objects.
			emit DesignerGrabQueued();
			break;

		case CI_DesignerRotate:
			// We must queue starting these designer commands because they are initiated in the control thread. That guy cannot call
			// StartCommand or EndCommand because it can't get the OpenGL context, and it can't create UI objects.
			emit DesignerRotateQueued();
			break;

		case CI_DesignerStretch:
			// We must queue starting these designer commands because they are initiated in the control thread. That guy cannot call
			// StartCommand or EndCommand because it can't get the OpenGL context, and it can't create UI objects.
			emit DesignerStretchQueued();
			break;

		case CI_DesignerPushPull:
			DesignerPushPull(FromPixelToDesignPlane(m_MouseLast, true, 0));
			break;

		case CI_DragConstrainX:
			m_DragConstraint = (m_DragConstraint == 1) ? 0 : 1;
			break;

		case CI_DragConstrainY:
			m_DragConstraint = (m_DragConstraint == 2) ? 0 : 2;
			break;

		case CI_DragConstrainZ:
			m_DragConstraint = (m_DragConstraint == 4) ? 0 : 4;
			break;
		}
	}
}

// Process a coordinate entered into the AuDesignerBar edit control. It is sent to us still in its string form.
void AuDesignerView::CoordinateEntered(QString coord, int keymods)
{
	if(!IsInNewDesignEnvironment())
		return;
	// Designer does nothing in preview mode.
	if(IsDesignPreview())
		return;
	if(!Command() && !m_RenderInfo->DesignerHasPartSelection())
		return;							// Nothing to do without one of those two pieces of information.

	if(Command())
	{
		// Parse a coordinate from the string.
		AuPoint loc;
		loc.StringValue(coord, g_AuClient.DesignerUnitScale(), &m_DesignPlane);

		// Send the coordinate to the waiting command.
		CoordinateEntered(loc, true, keymods);
	}
	else
	{
		// No command waiting so use the vector to copy or move the selected elements.
		// Parse a vector from the string.
		// We will use a design plane that matches the current design plane's rotation but that has no origin offset. The resulting
		// vector produced from StringValue will be relative to the design plane rotation, unless the user specifies 'absolute', in
		// which case the vector will be relative to the native coordinate system. That works.
		AuMatrix4x4 vPlane;
		vPlane.Rotate(m_DesignPlane.GetRotation());
		AuPoint inputVec;
		inputVec.StringValue(coord, g_AuClient.DesignerUnitScale(), &vPlane);
		AuMatrix4x4 t;
		t.Translate(inputVec);
		if(DesignerObjectMode())
		{
			if(keymods & Qt::ControlModifier)
				auApp->DesignerCopyPartObjects(m_RenderInfo->DesignerPartObjectSelection(), t);
			else
				auApp->DesignerMovePartObjects(m_RenderInfo->DesignerPartObjectSelection(), t);
		}
		else
		{
			if(keymods & Qt::ControlModifier)
				auApp->DesignerCopyParts(m_RenderInfo->DesignerPartSelection(), t);
			else
				auApp->DesignerMoveParts(m_RenderInfo->DesignerPartSelection(), t);
		}
	}
}

// ALL coordinate input in response to new designer commands eventually gets to here.
void AuDesignerView::CoordinateEntered(AuPoint coord, bool fromKeyboard, int keymods)
{
	if(!IsInNewDesignEnvironment())
		return;
	// Designer does nothing in preview mode.
	if(IsDesignPreview())
		return;

	AuPoint loc(coord);
	if(Command())
		loc = Command()->CoordinateEntered(loc, fromKeyboard, keymods);

	if(g_AuClient.DesignerGridFollowsWork())
	{
		// Move the design plane to the location that was entered.
		AuMatrix4x4 newPlane;
		newPlane.Translate(loc);
		newPlane.Rotate(m_DesignPlane.GetRotation());
		SetDesignPlane(newPlane);
	}
}

void AuDesignerView::CommandPromptChanged(const QString& prompt)
{
	emit DesignerCommandPrompt(prompt);
}

bool AuDesignerView::GripLoc(const AuMatrix4x4& model, const AuPoint& loc, QPoint& gripLoc) const
{
	// Calculate the location of 'loc' on the screen.
	AuPoint screenLoc = model.Project(loc);
	if(screenLoc.z >= 0)
		return false;
	screenLoc = m_tProjection.Project(screenLoc);

	// Convert location to pixels.
	gripLoc.setX(width()/2 + round(screenLoc.x * width()/2));
	gripLoc.setY(height()/2 - round(screenLoc.y * height()/2));
	return true;
}

void AuDesignerView::DrawDiamond(const AuMatrix4x4& model, const AuPoint& loc, float r, float g, float b, bool filled)
{
	// Do nothing if the window is smashed.
	if(height() <= 0 || width() <= 0)
		return;

	// Calculate the location of 'loc' on the screen.
	AuPoint screenLoc = model.Project(loc);
	if(screenLoc.z >= 0)
		return;
	screenLoc = m_tProjection.Project(screenLoc);

	// Calculate pixel size of blip, corrected for the window's aspect ratio.
	double blipSizeY = 1./(double)height() * 12.;
	double blipSizeX = blipSizeY * AuGLEffects::WindowAspectRatio(rect());

	// Line shader is nice and bright, no textures or shading.
	m_RenderInfo->re->GLSL().LineShader().Select();
	m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(AuMatrix4x4());
	m_RenderInfo->re->GLSL().LineShader().SetColor(r,g,b,1.f);

	// Configure for drawing crispy little lines.
	m_RenderInfo->re->glDepthMask(GL_FALSE);
	m_RenderInfo->re->glDisable(GL_DEPTH_TEST);

	// Draw the diamond blip.
	if(filled)
	{
		// Compile a list of vertex locations.
		AuPointArray verts;
		verts.append(AuPoint(screenLoc.x, screenLoc.y, 0));
		verts.append(AuPoint(screenLoc.x, screenLoc.y+blipSizeY, 0));
		verts.append(AuPoint(screenLoc.x+blipSizeX, screenLoc.y, 0));

		verts.append(AuPoint(screenLoc.x, screenLoc.y, 0));
		verts.append(AuPoint(screenLoc.x, screenLoc.y-blipSizeY, 0));
		verts.append(AuPoint(screenLoc.x-blipSizeX, screenLoc.y, 0));

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForLines(*m_RenderInfo->re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			// Display the VAO once and clean it up.
			vao.SelectGL();
			vao.DrawGL(GL_TRIANGLES);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		// Draw upper left triangle of diamond bright white.
		m_RenderInfo->re->GLSL().LineShader().SetColor(1.f, 1.f, 1.f, 1.f);
		verts.clear();
		verts.append(AuPoint(screenLoc.x, screenLoc.y, 0));
		verts.append(AuPoint(screenLoc.x-blipSizeX, screenLoc.y, 0));
		verts.append(AuPoint(screenLoc.x, screenLoc.y+blipSizeY, 0));

		// Make a VAO of the vertex locations.
		vao.CreateForLines(*m_RenderInfo->re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			// Display the VAO once and clean it up.
			vao.SelectGL();
			vao.DrawGL(GL_TRIANGLES);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		// Draw lower right triangle of diamond in dim incoming color.
		m_RenderInfo->re->GLSL().LineShader().SetColor(r*.6f, g*.6f, b*.6f, 1.f);
		verts.clear();
		verts.append(AuPoint(screenLoc.x, screenLoc.y, 0));
		verts.append(AuPoint(screenLoc.x+blipSizeX, screenLoc.y, 0));
		verts.append(AuPoint(screenLoc.x, screenLoc.y-blipSizeY, 0));

		// Make a VAO of the vertex locations.
		vao.CreateForLines(*m_RenderInfo->re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			// Display the VAO once and clean it up.
			vao.SelectGL();
			vao.DrawGL(GL_TRIANGLES);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}
	else
	{
		// Not filled so draw diamond boundary.
		// Compile a list of vertex locations.
		AuPointArray verts;
		verts.append(AuPoint(screenLoc.x, screenLoc.y+blipSizeY, 0));
		verts.append(AuPoint(screenLoc.x+blipSizeX, screenLoc.y, 0));
		verts.append(AuPoint(screenLoc.x, screenLoc.y-blipSizeY, 0));
		verts.append(AuPoint(screenLoc.x-blipSizeX, screenLoc.y, 0));

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForLines(*m_RenderInfo->re, verts, true, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			// Display the VAO once and clean it up.
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	// Restore rendering states.
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);
	m_RenderInfo->re->glDepthMask(GL_TRUE);
	m_RenderInfo->re->GLSL().LineShader().Deselect();
}

void AuDesignerView::DrawGrip(const AuMatrix4x4& model, const AuPoint& loc, float r, float g, float b)
{
	// Do nothing if the window is smashed.
	if(height() <= 0 || width() <= 0)
		return;

	// Calculate the location of 'loc' on the screen.
	AuPoint screenLoc = model.Project(loc);
	if(screenLoc.z >= 0)
		return;
	screenLoc = m_tProjection.Project(screenLoc);

	// Calculate pixel size of blip, corrected for the window's aspect ratio.
	double blipSizeY = 1./(double)height() * 8.;
	double blipSizeX = blipSizeY * AuGLEffects::WindowAspectRatio(rect());

	// Line shader is nice and bright, no textures or shading.
	m_RenderInfo->re->GLSL().LineShader().Select();
	m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(AuMatrix4x4());
	m_RenderInfo->re->GLSL().LineShader().SetColor(r,g,b,1.f);

	// Configure for drawing crispy little lines.
	m_RenderInfo->re->glDepthMask(GL_FALSE);
	m_RenderInfo->re->glDisable(GL_DEPTH_TEST);

	// Compile a list of vertex locations.
	AuPointArray verts;
	verts.append(AuPoint(screenLoc.x-blipSizeX, screenLoc.y-blipSizeY, 0));
	verts.append(AuPoint(screenLoc.x-blipSizeX, screenLoc.y+blipSizeY, 0));
	verts.append(AuPoint(screenLoc.x+blipSizeX, screenLoc.y+blipSizeY, 0));
	verts.append(AuPoint(screenLoc.x+blipSizeX, screenLoc.y+blipSizeY, 0));
	verts.append(AuPoint(screenLoc.x+blipSizeX, screenLoc.y-blipSizeY, 0));

	// Make a VAO of the vertex locations.
	AuGLVAO vao;
	vao.CreateForLines(*m_RenderInfo->re, verts, true, GL_STREAM_DRAW);
	if(vao.VAO())
	{
		// Display the VAO once and clean it up.
		vao.SelectGL();
		vao.DrawGL(GL_TRIANGLES);
		vao.DeselectGL();
		vao.DeleteGL();
	}

	// Restore rendering states.
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);
	m_RenderInfo->re->glDepthMask(GL_TRUE);
	m_RenderInfo->re->GLSL().LineShader().Deselect();
}

// For designer.
void AuDesignerView::SetDesignPlane(const AuMatrix4x4& designPlane)
{
	m_DesignPlane = designPlane;
}

void AuDesignerView::GrabMouse()
{
	if(!m_MouseGrabbed)
	{
		grabMouse();
		m_MouseGrabbed = true;
		//qDebug() << "AuDesignerView::GrabMouse";
	}
}

// Returns true if the mouse was released, false if I was not grabbing the mouse when this was called.
bool AuDesignerView::ReleaseMouse()
{
	if(!m_MouseGrabbed)
		return false;

	releaseMouse();
	m_MouseGrabbed = false;
	//qDebug() << "AuDesignerView::ReleaseMouse";
	return true;
}

void AuDesignerView::ShowItemManipulator(const DaItem& item)
{
	//qDebug() << "AuDesignerView::ShowItemManipulator";
	if(!m_ItemManipulator)
	{
		AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
		m_ItemManipulator = new AuItemManipulator(pFrame);
	}
	m_ItemManipulator->SetItem(item);
	m_ItemManipulator->show();
}

void AuDesignerView::CreateViewButtons()
{
	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	g_Au.UnlockData();
	if(auApp->IsSingleUserDesignerMode())
	{
		// Buttons for spacecraft or building designer mode.
		class AuViewButton sceneClient[] =
		{
			AuViewButton(AuViewButton::VB_About),
			AuViewButton(AuViewButton::VB_Architect),
			AuViewButton(AuViewButton::VB_Programmer),
			AuViewButton(AuViewButton::VB_Agent),
			AuViewButton(AuViewButton::VB_HideWindows),
			AuViewButton(AuViewButton::VB_FullScreen),
			AuViewButton(AuViewButton::VB_Settings),
			AuViewButton(AuViewButton::VB_Exit,
					tr("<b>Exit Game</b> %2<br>"
						"Exit the game.")),
		};
		int buttons = sizeof(sceneClient)/sizeof(AuViewButton);
		for(int but=0; but<buttons; but++)
			m_ViewButtons.append(sceneClient[but]);
	}
	else if(auApp->IsSingleUserMode())
	{
		// Buttons for solo mode.
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
			AuViewButton(AuViewButton::VB_ConstructionBd),
			AuViewButton(AuViewButton::VB_Government),
			AuViewButton(AuViewButton::VB_ArenaStatus),
			AuViewButton(AuViewButton::VB_ArenaRecall),
			AuViewButton(AuViewButton::VB_ArenaExit),
			AuViewButton(AuViewButton::VB_Architect),
			AuViewButton(AuViewButton::VB_Programmer),
			AuViewButton(AuViewButton::VB_Agent),
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
						"<b>Cancel recall</b> by pressing the Recall Home button while waiting to recall."
						"</html>") : ""),
			AuViewButton(AuViewButton::VB_HideWindows),
			AuViewButton(AuViewButton::VB_FullScreen),
			AuViewButton(AuViewButton::VB_Settings),
			AuViewButton(AuViewButton::VB_Exit,
					tr("<b>Exit Game</b> %2<br>"
						"Exit the game.")),
		};
		int buttons = sizeof(sceneClient)/sizeof(AuViewButton);
		for(int but=0; but<buttons; but++)
			m_ViewButtons.append(sceneClient[but]);
	}
	else
	{
		// Buttons for scene client mode.
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
			AuViewButton(AuViewButton::VB_ConstructionBd),
			AuViewButton(AuViewButton::VB_Government),
			AuViewButton(AuViewButton::VB_ArenaStatus),
			AuViewButton(AuViewButton::VB_ArenaRecall),
			AuViewButton(AuViewButton::VB_ArenaExit),
			AuViewButton(AuViewButton::VB_Architect),
			AuViewButton(AuViewButton::VB_Programmer),
			AuViewButton(AuViewButton::VB_Agent),
			AuViewButton(AuViewButton::VB_Minion),
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
						"<b>Cancel recall</b> by pressing the Recall Home button while waiting to recall."
						"</html>") : ""),
			AuViewButton(AuViewButton::VB_HideWindows),
			AuViewButton(AuViewButton::VB_FullScreen),
			AuViewButton(AuViewButton::VB_Settings),
			AuViewButton(AuViewButton::VB_Exit,
					tr("<b>Exit Game</b> %2<br>"
						"Exit the game.")),
		};
		int buttons = sizeof(sceneClient)/sizeof(AuViewButton);
		for(int but=0; but<buttons; but++)
			m_ViewButtons.append(sceneClient[but]);
	}

	// Create a VAO that can be used for all the buttons.
	if(!m_ButtonVAO.VAO())
	{
		QList<AuTriTex> tris;
		tris << AuTriTex(AuPointTex(-1, -1, 0, 0,0),
				AuPointTex(-1, 1, 0, 0,1),
				AuPointTex(1, 1, 0, 1,1));
		tris << AuTriTex(AuPointTex(1, 1, 0, 1,1),
				AuPointTex(1, -1, 0, 1,0),
				AuPointTex(-1, -1, 0, 0,0));
		m_ButtonVAO.CreateForTexture(*m_RenderInfo->re, tris, GL_STATIC_DRAW);
	}

	// Create VAOs used for the sub button/menu items.
	if(!m_SubStartVAO.VAO())
	{
		// Texture coords for start is the left half of the circle texture.
		QList<AuTriTex> tris;
		tris << AuTriTex(AuPointTex(-1, -1, 0, 0,0),
				AuPointTex(-1, 1, 0, 0,1),
				AuPointTex(0, 1, 0, .5f,1));
		tris << AuTriTex(AuPointTex(0, 1, 0, .5f,1),
				AuPointTex(0, -1, 0, .5f,0),
				AuPointTex(-1, -1, 0, 0,0));
		m_SubStartVAO.CreateForTexture(*m_RenderInfo->re, tris, GL_STATIC_DRAW);
	}
	if(!m_SubMidVAO.VAO())
	{
		// Texture coord for middle is the center column of pixels of the circle texture.
		QList<AuTriTex> tris;
		tris << AuTriTex(AuPointTex(-1, -1, 0, .5f,0),
				AuPointTex(-1, 1, 0, .5f,1),
				AuPointTex(1, 1, 0, .5f,1));
		tris << AuTriTex(AuPointTex(1, 1, 0, .5f,1),
				AuPointTex(1, -1, 0, .5f,0),
				AuPointTex(-1, -1, 0, .5f,0));
		m_SubMidVAO.CreateForTexture(*m_RenderInfo->re, tris, GL_STATIC_DRAW);
	}
	if(!m_SubEndVAO.VAO())
	{
		// Texture coords for end is the right half of the circle texture.
		QList<AuTriTex> tris;
		tris << AuTriTex(AuPointTex(0, -1, 0, .5f,0),
				AuPointTex(0, 1, 0, .5f,1),
				AuPointTex(1, 1, 0, 1,1));
		tris << AuTriTex(AuPointTex(1, 1, 0, 1,1),
				AuPointTex(1, -1, 0, 1,0),
				AuPointTex(0, -1, 0, .5f,0));
		m_SubEndVAO.CreateForTexture(*m_RenderInfo->re, tris, GL_STATIC_DRAW);
	}
}

double AuDesignerView::ViewButtonRadius() const
{
	if(rect().height() <= 0)
		return 0;

	if(g_AuClient.ViewButtonSize() <= 0)
	{
		// Size is automatic.
		return 0.045;
	}
	else
	{
		double pxPerUnit = (double)rect().height()/2.;	// OpenGL screen height is 2 units, -1 to 1.
		return ((double)g_AuClient.ViewButtonSize()/2.) / pxPerUnit;
	}
}

// Returns the width of the widest sub item.
double AuDesignerView::ViewButtonSubLength() const
{
	if(rect().height() <= 0)
		return 0;

	/*
	// This is like a menu item, it must always be a fixed size.
	double pixelsLength = 100.;
	double pxPerUnit = (double)rect().height()/2.;	// OpenGL screen height is 2 units, -1 to 1.
	return pixelsLength / pxPerUnit;
	*/

	double widestSub = 0;
	for(QList<AuViewButton>::const_iterator bi = m_ViewButtons.cbegin(); bi != m_ViewButtons.cend(); bi++)
	{
		if(!(*bi).IsSub())
			break;
		double subWid = ViewButtonSubLength((*bi).Name());
		if(widestSub < subWid)
			widestSub = subWid;
	}
	return widestSub;
}

double AuDesignerView::ViewButtonSubLength(const QString& text) const
{
	if(rect().height() <= 0)
		return 0;

	// This is like a menu item, it must always be a fixed size.
	// Let's guess at the width based on the number of characters.
	double pixelsLength = text.count() * 12;
	double pxPerUnit = (double)rect().height()/2.;	// OpenGL screen height is 2 units, -1 to 1.
	return pixelsLength / pxPerUnit;
}

double AuDesignerView::ViewButtonSubRadius() const
{
	if(rect().height() <= 0)
		return 0;

	// This is like a menu item, it must always be a fixed size.
	double pixelsHeight = 16.;
	double pxPerUnit = (double)rect().height()/2.;	// OpenGL screen height is 2 units, -1 to 1.
	return pixelsHeight / pxPerUnit;
}

void AuDesignerView::RenderViewButtons()
{
	if(!m_RenderInfo->m_ShowHUD)
	{
		m_ViewButtonAt = AuViewButton::VB_Invalid;
		m_ViewButtonNear = AuViewButton::VB_Invalid;
		return;
	}
	if(rect().width() <= 0 || rect().height() <= 0)
	{
		m_ViewButtonAt = AuViewButton::VB_Invalid;
		m_ViewButtonNear = AuViewButton::VB_Invalid;
		return;
	}

	if(!m_ViewButtonTex || m_ViewButtonTexName != g_AuClient.ViewButtonImageName())
	{
		// This is the texture for the blank button face.
		m_ViewButtonTexName = g_AuClient.ViewButtonImageName();
		QString sid = QString("VB:%1").arg(m_ViewButtonTexName);
		QImage imgb = QImage(g_AuClient.ViewButtonImageName()).scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		m_ViewButtonTex = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, imgb, GL_LINEAR, GL_LINEAR, false, 1);
	}

	if(!m_ViewButtonGlow[0])
	{
		// This is the texture for highlighting buttons.
		for(int g=0; g<4; g++)
		{
			QString sid = QString("VBGlow%1").arg(g);
			QImage imgb = QImage(QString(":data/ButtonGlow%1.png").arg(g));
			m_ViewButtonGlow[g] =
				m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, imgb, GL_LINEAR, GL_LINEAR, false, 1);
		}
	}

	float alpha;
	if(!IsCharacterInputMode())
	{
		// Buttons need to be visible.
		if(m_InputModeChanged.elapsed() > 500)
			alpha = 1;
		else
			alpha = m_InputModeChanged.elapsed()/500.f;
	}
	else
	{
		// Buttons need to disappear.
		if(m_InputModeChanged.elapsed() > 1000)
			alpha = 0;
		else
			alpha = 1.f - m_InputModeChanged.elapsed()/1000.f;
	}

	// Modulate alpha using user's view button transparency setting.
	alpha *= g_AuClient.ViewButtonAlpha();
	if(alpha == 0)
	{
		m_ViewButtonAt = AuViewButton::VB_Invalid;
		m_ViewButtonNear = AuViewButton::VB_Invalid;
		return;
	}

	// Update hot button under the cursor.
	AuViewButton::VIEWBUTTON viewButtonAt = ViewButtonAt(m_MouseLast);
	if(viewButtonAt != m_ViewButtonAt)
	{
		// At button changed.
		m_ViewButtonAt = viewButtonAt;
		if(m_ViewButtonAt != AuViewButton::VB_Invalid)
			m_ViewButtonEntered.restart();	// Start tooltip timer.
		else
		{
			m_ViewButtonEntered = AuElapsedTimer();	// Kill tooltip timer.
			QToolTip::showText(mapToGlobal(m_MouseLast), "");	// Get rid of the tooltip.
		}
	}

	AuViewButton::VIEWBUTTON viewButtonNear = ViewButtonNear(m_MouseLast);
	if(viewButtonNear != m_ViewButtonNear)
	{
		// Near button changed.
		m_ViewButtonNear = viewButtonNear;

		// Remove previous sub buttons.
		QList<AuViewButton>::iterator bi = m_ViewButtons.begin();
		while(bi != m_ViewButtons.end())
		{
			if((*bi).IsSub())
				bi = m_ViewButtons.erase(bi);
			else
				bi++;
		}

		// Create new sub buttons, if any.
		switch(viewButtonNear)
		{
		default:
			break;

		case AuViewButton::VB_Cargo:
			{
				// NOTE: These are listed in reverse order because they are prepended to the list one by one.
				class AuViewButton subItems[] =
				{
					AuViewButton(AuViewButton::VB_CargoOrdnance),
					AuViewButton(AuViewButton::VB_CargoTransfer),
					AuViewButton(AuViewButton::VB_CargoHold),
					AuViewButton(AuViewButton::VB_CargoManifest),
				};
				int buttons = sizeof(subItems)/sizeof(AuViewButton);
				for(int but=0; but<buttons; but++)
				{
					AuViewButton vb(subItems[but]);
					vb.m_Parent = viewButtonNear;
					m_ViewButtons.prepend(vb);
				}
			}
			break;

		case AuViewButton::VB_Gear:
			{
				// NOTE: These are listed in reverse order because they are prepended to the list one by one.
				class AuViewButton subItems[] =
				{
					AuViewButton(AuViewButton::VB_GearHandCraft),
					AuViewButton(AuViewButton::VB_Gear),
				};
				int buttons = sizeof(subItems)/sizeof(AuViewButton);
				for(int but=0; but<buttons; but++)
				{
					AuViewButton vb(subItems[but]);
					vb.m_Parent = viewButtonNear;
					m_ViewButtons.prepend(vb);
				}
			}
			break;

		case AuViewButton::VB_Government:
			{
				// NOTE: These are listed in reverse order because they are prepended to the list one by one.
				class AuViewButton subItems[] =
				{
					AuViewButton(AuViewButton::VB_GovernTaxes),
					AuViewButton(AuViewButton::VB_GovernCities),
					AuViewButton(AuViewButton::VB_GovernCitizens),
					AuViewButton(AuViewButton::VB_GovernPatents),
					AuViewButton(AuViewButton::VB_GovernCommerce),
					AuViewButton(AuViewButton::VB_GovernDiplomacy),
					AuViewButton(AuViewButton::VB_GovernPolicy),
					AuViewButton(AuViewButton::VB_GovernAnnouncements),
				};
				int buttons = sizeof(subItems)/sizeof(AuViewButton);
				for(int but=0; but<buttons; but++)
				{
					AuViewButton vb(subItems[but]);
					vb.m_Parent = viewButtonNear;
					m_ViewButtons.prepend(vb);
				}
			}
			break;

		case AuViewButton::VB_Locator:
			{
				// NOTE: These are listed in reverse order because they are prepended to the list one by one.
				class AuViewButton subItems[] =
				{
					AuViewButton(AuViewButton::VB_LocatorGPS),
					AuViewButton(AuViewButton::VB_LocatorStarMap),
					AuViewButton(AuViewButton::VB_LocatorSystemScan),
					AuViewButton(AuViewButton::VB_LocatorWorldMap),
					AuViewButton(AuViewButton::VB_LocatorCompass),
				};
				int buttons = sizeof(subItems)/sizeof(AuViewButton);
				for(int but=0; but<buttons; but++)
				{
					AuViewButton vb(subItems[but]);
					vb.m_Parent = viewButtonNear;
					m_ViewButtons.prepend(vb);
				}
			}
			break;

		case AuViewButton::VB_Spacecraft:
			{
				// NOTE: These are listed in reverse order because they are prepended to the list one by one.
				class AuViewButton subItems[] =
				{
					AuViewButton(AuViewButton::VB_SpacecraftDesignExchange),
					AuViewButton(AuViewButton::VB_SpacecraftSpecs),
					AuViewButton(AuViewButton::VB_SpacecraftShipments),
					AuViewButton(AuViewButton::VB_SpacecraftDamage),
					AuViewButton(AuViewButton::VB_SpacecraftBounties),
					AuViewButton(AuViewButton::VB_SpacecraftAccount),
					AuViewButton(AuViewButton::VB_SpacecraftRoster),
					AuViewButton(AuViewButton::VB_SpacecraftPolicy),
					AuViewButton(AuViewButton::VB_SpacecraftMission),
				};
				int buttons = sizeof(subItems)/sizeof(AuViewButton);
				for(int but=0; but<buttons; but++)
				{
					AuViewButton vb(subItems[but]);
					vb.m_Parent = viewButtonNear;
					m_ViewButtons.prepend(vb);
				}
			}
			break;

		case AuViewButton::VB_Building:
			{
				// NOTE: These are listed in reverse order because they are prepended to the list one by one.
				class AuViewButton subItems[] =
				{
					AuViewButton(AuViewButton::VB_BdDesignExchange),
					AuViewButton(AuViewButton::VB_Building),
				};
				int buttons = sizeof(subItems)/sizeof(AuViewButton);
				for(int but=0; but<buttons; but++)
				{
					AuViewButton vb(subItems[but]);
					vb.m_Parent = viewButtonNear;
					m_ViewButtons.prepend(vb);
				}
			}
			break;

		case AuViewButton::VB_Bio:
			{
				// NOTE: These are listed in reverse order because they are prepended to the list one by one.
				class AuViewButton subItems[] =
				{
					AuViewButton(AuViewButton::VB_Avatar6),
					AuViewButton(AuViewButton::VB_Avatar5),
					AuViewButton(AuViewButton::VB_Avatar4),
					AuViewButton(AuViewButton::VB_Avatar3),
					AuViewButton(AuViewButton::VB_Avatar2),
					AuViewButton(AuViewButton::VB_Avatar1),
				};
				int buttons = sizeof(subItems)/sizeof(AuViewButton);
				for(int but=0; but<buttons; but++)
				{
					const DgData& gd = g_Au.LockDataForRead();
					AuId curId = gd.ClientUnitId();
					AuId avId = gd.ClientUnitId(5-but);
					QString avName = gd.ClientUnitName(avId);
					g_Au.UnlockData();
					if(avId.IsNull())
						continue;
					if(avName.isEmpty())
					{
						auApp->RequestAvatars();
						continue;
					}

					AuViewButton vb(subItems[but]);
					if(avId == curId)
					{
						vb.m_Enabled = false;	// Disabled the current one.
						vb.m_Tip = tr("<b>%1</b> <i>%2</i> is Current").arg(avName).arg(avId.StringId());
					}
					else
					{
						vb.m_Enabled = true;
						vb.m_Tip = tr("Switch to <b>%1</b> <i>%2</i>").arg(avName).arg(avId.StringId());
					}
					vb.m_Parent = viewButtonNear;
					vb.m_Text = avName;
					vb.m_Tip += " %2";
					m_ViewButtons.prepend(vb);
				}
			}
			break;

		case AuViewButton::VB_About:
			{
				// NOTE: These are listed in reverse order because they are prepended to the list one by one.
				if(g_Au.IsSceneSolo())
				{
					class AuViewButton subItems[] =
					{
						AuViewButton(AuViewButton::VB_SaveSolitaire),
						AuViewButton(AuViewButton::VB_Videos),
						AuViewButton(AuViewButton::VB_Forum),
						AuViewButton(AuViewButton::VB_Wiki),
						AuViewButton(AuViewButton::VB_HazeronHome),
					};
					int buttons = sizeof(subItems)/sizeof(AuViewButton);
					for(int but=0; but<buttons; but++)
					{
						AuViewButton vb(subItems[but]);
						vb.m_Parent = viewButtonNear;
						m_ViewButtons.prepend(vb);
					}
				}
				else if(auApp->IsSingleUserMode())
				{
					class AuViewButton subItems[] =
					{
						AuViewButton(AuViewButton::VB_Videos),
						AuViewButton(AuViewButton::VB_Forum),
						AuViewButton(AuViewButton::VB_Wiki),
						AuViewButton(AuViewButton::VB_HazeronHome),
					};
					int buttons = sizeof(subItems)/sizeof(AuViewButton);
					for(int but=0; but<buttons; but++)
					{
						AuViewButton vb(subItems[but]);
						vb.m_Parent = viewButtonNear;
						m_ViewButtons.prepend(vb);
					}
				}
				else
				{
					class AuViewButton subItems[] =
					{
						AuViewButton(AuViewButton::VB_WarDimensionStandings),
						AuViewButton(AuViewButton::VB_Avatars),
						AuViewButton(AuViewButton::VB_Videos),
						AuViewButton(AuViewButton::VB_Forum),
						AuViewButton(AuViewButton::VB_Wiki),
						AuViewButton(AuViewButton::VB_HazeronHome),
					};
					int buttons = sizeof(subItems)/sizeof(AuViewButton);
					for(int but=0; but<buttons; but++)
					{
						AuViewButton vb(subItems[but]);
						vb.m_Parent = viewButtonNear;
						m_ViewButtons.prepend(vb);
					}
				}
			}
			break;
		}
	}

	bool hasNewDossierDoc = false;
	quint32 sceneAtomEmpire = SceneAtomEmpire();
	AuId sceneAtomId = SceneAtomId();
	const DgData& gd = g_Au.LockDataForRead();
	bool canManageDossier = gd.EmpireCitizenCanManageDossier(sceneAtomEmpire, sceneAtomId);
	if(canManageDossier)
	{
		AuTime newestNew = gd.EmpireNewestNewDossierDoc(sceneAtomEmpire);
		if(!newestNew.IsNull())
			hasNewDossierDoc = true;
	}
	g_Au.UnlockData();

	// Position buttons.
	PositionViewButtons();

	AuMatrix4x4 p;
	double gldAspect = AuGLEffects::WindowAspectRatio(rect());
	p.Scale(gldAspect, 1, 1);

	AuPoint mouse;
	mouse.x = (double)m_MouseLast.x()/(double)rect().width() - .5;
	mouse.y = -((double)m_MouseLast.y()/(double)rect().height() - .5);
	mouse *= 2;

	m_RenderInfo->re->GLSL().TextureShader().Select();

	m_RenderInfo->re->glDisable(GL_DEPTH_TEST);
	m_RenderInfo->re->glEnable(GL_BLEND);
	//m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// Additive blending for extra brightness.
	m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Render all unhighlighted main buttons, skipping subs.
	m_ButtonVAO.SelectGL();
	bool hasEmpire = SceneAtomEmpire();
	double vbRadius = ViewButtonRadius();
	bool anySubs = false;
	for(QList<AuViewButton>::iterator bi = m_ViewButtons.begin(); bi != m_ViewButtons.end(); bi++)
	{
		if((*bi).IsSub())
		{
			anySubs = true;
			continue;
		}

		if(!(*bi).IsVisible())
			continue;
		if((*bi).IsHidden(SceneContext(), hasEmpire))
			continue;

		if((*bi).m_Id == m_ViewButtonNear ||
				((*bi).m_Id == m_ViewButtonPressed && m_ViewButtonPressed == m_ViewButtonAt))
			continue;

		AuMatrix4x4 mv;
		mv.Translate((*bi).m_ScreenPos.x/gldAspect, (*bi).m_ScreenPos.y, 0);

		// Background.
		{
			AuMatrix4x4 mvb(mv);
			mvb.Scale(vbRadius, vbRadius, 1);

			// Prepare to draw the button icon.
			m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(p * mvb);
			m_RenderInfo->re->GLSL().TextureShader().SetColor(1,1,1, alpha);
			m_RenderInfo->re->GLSL().TextureShader().SetTexture(m_ViewButtonTex);

			// Draw the button icon.
			m_ButtonVAO.DrawGL();

		}

		// Highlight some buttons in certain conditions.
		bool highlight = false;
		if((*bi).m_Id == AuViewButton::VB_Government)
		{
			// Highlighted when new messages are waiting.
			highlight = hasNewDossierDoc;
		}
		if(highlight)
		{
			AuMatrix4x4 mvg(mv);
			mvg.Scale(vbRadius*1.5, vbRadius*1.5, 1);

			// Prepare to draw the highlight splash.
			quint64 ms = AuTime::Now().MSecsSinceEpoch();
			ms /= 100;
			ms %= 4;
			m_RenderInfo->re->GLSL().TextureShader().SetTexture(m_ViewButtonGlow[ms]);
			m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(p * mvg);
			m_RenderInfo->re->GLSL().TextureShader().SetColor(1,1,1, 1);

			// Draw the button icon.
			m_ButtonVAO.DrawGL();
		}

		mv.Scale(.6 * vbRadius, .6 * vbRadius, 1);

		m_RenderInfo->re->GLSL().TextureShader().SetColor(1,1,1, alpha);
		m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(p * mv);
		m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Prepare to draw the button icon.
		if(!(*bi).m_Texture)
		{
			QString sid = (*bi).TextureName();
			(*bi).m_Texture = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, (*bi).Image(), GL_LINEAR, GL_LINEAR,
					false, 1);
		}

		m_RenderInfo->re->GLSL().TextureShader().SetTexture((*bi).m_Texture);

		// Draw the button icon.
		m_ButtonVAO.DrawGL();
	}
	m_ButtonVAO.DeselectGL();

	// Render all sub buttons, skipping mains.
	// Text is drawn in this pass, highlighted or not.
	double vbSubRadius = ViewButtonSubRadius();
	if(anySubs)
	{
		QFont font;
		font.setPointSize(20);
		double vbSubLength = ViewButtonSubLength();
		double vbSubSpacing = vbSubRadius / 10.;
		for(QList<AuViewButton>::iterator bi = m_ViewButtons.begin(); bi != m_ViewButtons.end(); bi++)
		{
			if( ! (*bi).IsSub())
				continue;

			if(!(*bi).IsVisible())
				continue;
			if((*bi).IsHidden(SceneContext(), hasEmpire))
				continue;

			float buttonAlpha = alpha;
			if(!(*bi).m_Enabled)
				buttonAlpha = alpha;
			else if(((*bi).m_Id == m_ViewButtonPressed && m_ViewButtonPressed == m_ViewButtonAt))
				buttonAlpha = 1;
			else if((*bi).m_Id == m_ViewButtonAt)
				buttonAlpha = 1;
			else
				buttonAlpha = alpha;

			AuMatrix4x4 mv;
			mv.Translate((*bi).m_ScreenPos.x/gldAspect, (*bi).m_ScreenPos.y, 0);

			// Background.
			{
				//double vbSubLength = ViewButtonSubLength((*bi).Name());

				// Prepare to draw the button icon.
				m_RenderInfo->re->GLSL().TextureShader().SetColor(1,1,1, buttonAlpha);
				m_RenderInfo->re->GLSL().TextureShader().SetTexture(m_ViewButtonTex);

				// Draw the button icon pieces to make a long background for the text.
				AuMatrix4x4 mvb(mv);
				mvb.Scale(vbSubRadius, vbSubRadius, 1);
				m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(p * mvb);
				m_SubStartVAO.SelectGL();
				m_SubStartVAO.DrawGL();
				m_SubStartVAO.DeselectGL();

				AuMatrix4x4 mve(mv);
				mve.Translate(vbSubLength,0,0);
				mve.Scale(vbSubRadius, vbSubRadius, 1);
				m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(p * mve);
				m_SubEndVAO.SelectGL();
				m_SubEndVAO.DrawGL();
				m_SubEndVAO.DeselectGL();

				AuMatrix4x4 mvm(mv);
				mvm.Translate(vbSubLength/2,0,0);
				mvm.Scale(vbSubLength/2., vbSubRadius, 1);
				m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(p * mvm);
				m_SubMidVAO.SelectGL();
				m_SubMidVAO.DrawGL();
				m_SubMidVAO.DeselectGL();
			}

			m_ButtonVAO.SelectGL();

			// Highlight some buttons in certain conditions.
			bool highlight = false;
			if((*bi).m_Id == AuViewButton::VB_GovernDiplomacy)
			{
				// Highlighted when new messages are waiting.
				highlight = hasNewDossierDoc;
			}
			if(highlight)
			{
				AuMatrix4x4 mvg(mv);
				mvg.Scale(vbSubRadius*1.5, vbSubRadius*1.5, 1);

				// Prepare to draw the highlight splash.
				quint64 ms = AuTime::Now().MSecsSinceEpoch();
				ms /= 100;
				ms %= 4;
				m_RenderInfo->re->GLSL().TextureShader().SetTexture(m_ViewButtonGlow[ms]);
				m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(p * mvg);
				m_RenderInfo->re->GLSL().TextureShader().SetColor(1,1,1, 1);

				// Draw the button icon.
				m_ButtonVAO.DrawGL();
			}

			mv.Scale(.5 * vbSubRadius, .5 * vbSubRadius, 1);

			m_RenderInfo->re->GLSL().TextureShader().SetColor(1,1,1, buttonAlpha);
			m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(p * mv);
			m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Prepare to draw the button icon.
			if(!(*bi).m_Texture)
			{
				QString sid = (*bi).TextureName();
				(*bi).m_Texture = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, (*bi).Image(), GL_LINEAR,
						GL_LINEAR, false, 1);
			}

			m_RenderInfo->re->GLSL().TextureShader().SetTexture((*bi).m_Texture);

			// Draw the button icon.
			m_ButtonVAO.DrawGL();
			m_ButtonVAO.DeselectGL();

			// Draw the text on the button.
			AuMatrix4x4 mvt;
			mvt.Translate((*bi).m_ScreenPos.x/gldAspect + vbSubRadius + vbSubSpacing, (*bi).m_ScreenPos.y, 0);

			if(!(*bi).m_Enabled)
			{
				m_RenderInfo->re->RenderText_Shader(p, mvt, (*bi).Name(), font, QColor(Qt::lightGray), rect().height(),
						Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);

				m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				m_RenderInfo->re->RenderText_Shader(p, mvt, (*bi).Name(), font, QColor(217,172,66), rect().height(),
						Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
				m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else if(((*bi).m_Id == m_ViewButtonPressed && m_ViewButtonPressed == m_ViewButtonAt))
			{
				m_RenderInfo->re->RenderText_Shader(p, mvt, (*bi).Name(), font, QColor(Qt::lightGray), rect().height(),
						Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);

				m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				m_RenderInfo->re->RenderText_Shader(p, mvt, (*bi).Name(), font, QColor(217,172,66), rect().height(),
						Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
				m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else if((*bi).m_Id == m_ViewButtonAt)
			{
				m_RenderInfo->re->RenderText_Shader(p, mvt, (*bi).Name(), font, QColor(85,170,255), rect().height(),
						Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
			}
			else
			{
				m_RenderInfo->re->RenderText_Shader(p, mvt, (*bi).Name(), font, QColor(Qt::lightGray), rect().height(),
						Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
			}
		}
	}

	// Render near and pressed buttons.
	// The near button is never a sub button.
	// The pressed button may be a sub button.
	if(m_ViewButtonNear != AuViewButton::VB_Invalid ||
			(m_ViewButtonPressed != AuViewButton::VB_Invalid && m_ViewButtonPressed == m_ViewButtonAt))
	{
		m_ButtonVAO.SelectGL();
		double vbPopRadius = vbRadius * 1.45;
		for(QList<AuViewButton>::iterator bi = m_ViewButtons.begin(); bi != m_ViewButtons.end(); bi++)
		{
			if(!(*bi).IsVisible())
				continue;
			if((*bi).IsHidden(SceneContext(), hasEmpire))
				continue;

			if( ! ((*bi).m_Id == m_ViewButtonNear ||
					((*bi).m_Id == m_ViewButtonPressed && m_ViewButtonPressed == m_ViewButtonAt)))
				continue;

			AuMatrix4x4 mv;
			mv.Translate((*bi).m_ScreenPos.x/gldAspect, (*bi).m_ScreenPos.y, 0);

			// Background.
			if(!(*bi).IsSub())
			{
				AuMatrix4x4 mvb(mv);
				mvb.Scale(vbPopRadius, vbPopRadius, 1);

				// Prepare to draw the button icon.
				m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(p * mvb);
				m_RenderInfo->re->GLSL().TextureShader().SetColor(1,1,1, .7f);
				m_RenderInfo->re->GLSL().TextureShader().SetTexture(m_ViewButtonTex);

				// Draw the button icon.
				m_ButtonVAO.DrawGL();
			}

			// Highlight some buttons in certain conditions.
			bool highlight = false;
			if((*bi).m_Id == AuViewButton::VB_Government)
			{
				// Highlighted when new messages are waiting.
				highlight = hasNewDossierDoc;
			}
			if(highlight)
			{
				AuMatrix4x4 mvg(mv);
				if(!(*bi).IsSub())
					mvg.Scale(vbPopRadius*1.5, vbPopRadius*1.5, 1);
				else
					mvg.Scale(vbSubRadius*1.5, vbSubRadius*1.5, 1);

				// Prepare to draw the highlight splash.
				quint64 ms = AuTime::Now().MSecsSinceEpoch();
				ms /= 100;
				ms %= 4;
				m_RenderInfo->re->GLSL().TextureShader().SetTexture(m_ViewButtonGlow[ms]);
				m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(p * mvg);
				m_RenderInfo->re->GLSL().TextureShader().SetColor(1,1,1, 1);

				// Draw the button icon.
				m_ButtonVAO.DrawGL();
			}

			if(!(*bi).IsSub())
				mv.Scale(.6 * vbPopRadius, .6 * vbPopRadius, 1);
			else
				mv.Scale(.6 * vbSubRadius, .6 * vbSubRadius, 1);

			m_RenderInfo->re->GLSL().TextureShader().SetColor(1,1,1, 1);
			m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(p * mv);
			m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Prepare to draw the button icon.
			if(!(*bi).m_Texture)
			{
				QString sid = (*bi).TextureName();
				(*bi).m_Texture = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, (*bi).Image(),
						GL_LINEAR, GL_LINEAR, false, 1);
			}

			m_RenderInfo->re->GLSL().TextureShader().SetTexture((*bi).m_Texture);

			// Draw the button icon.
			m_ButtonVAO.DrawGL();
			if((*bi).m_Id == m_ViewButtonPressed && m_ViewButtonPressed == m_ViewButtonAt)
			{
				m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// Additive blending for extra brightness.
				m_ButtonVAO.DrawGL();	// Extra pass makes it brighter when button pushed.
				m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
		}
		m_ButtonVAO.DeselectGL();
	}
	m_RenderInfo->re->glDisable(GL_BLEND);
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);

	m_RenderInfo->re->GLSL().TextureShader().Deselect();
}

void AuDesignerView::RenderWindowSize()
{
	if(!g_AuClient.ShowWindowSize())
		return;
	if(m_SizeFoVY != g_Au.FieldOfViewY())
	{
		m_SizeFoVY = g_Au.FieldOfViewY();
		m_SizeChanged.restart();
	}

	float alpha;
	if(m_SizeChanged.elapsed() > 2000)
		alpha = 0;
	else
		alpha = 1.f - m_SizeChanged.elapsed()/2000.f;

	// Modulate alpha using user's view button transparency setting.
	alpha *= g_AuClient.ViewButtonAlpha();
	if(alpha == 0)
		return;

	QColor textcolor(0,255,255);
	textcolor.setAlphaF(alpha);
	int cpos = width()/2;
	int vpos = height()/3;
	double aspect = AuGLEffects::WindowAspectRatio(rect());	// aspect 1 = 90, aspect .5 = 180
	QString text = QString("%1 x %2").arg(width()).arg(height());

	// Field of view angles.
	double fovx = m_SizeFoVY / aspect;
	double fovy;
	if(fovx <= PI)
	{
		fovx *= RAD2DEG;
		fovy = m_SizeFoVY * RAD2DEG;
	}
	else
	{
		fovx = 180;
		fovy = 180 * aspect;
	}
	QString text2 = QString("%1%3 x %2%3").arg(fovx, 0,'f',0).arg(fovy, 0,'f',0).arg(g_Au.DegreeSymbol());

	m_RenderInfo->re->glEnable(GL_BLEND);
	m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	m_RenderInfo->re->RenderText(cpos, vpos, text, g_AuClient.FontAtom(), textcolor, width(), height(),
			Qt::AlignHCenter|Qt::AlignBottom);
	m_RenderInfo->re->RenderText(cpos, vpos, text2, g_AuClient.FontAtom(), textcolor, width(), height(),
			Qt::AlignHCenter|Qt::AlignTop);
	m_RenderInfo->re->glDisable(GL_BLEND);
}

void AuDesignerView::SaveSolitaireCheckPoint()
{
	if(!g_Au.IsSceneSolo())
		return;
	auApp->SaveSolitaire();
	auApp->SaveCheckPoint(SceneAtomName());
}

void AuDesignerView::ViewButtonClicked(AuViewButton::VIEWBUTTON button)
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();

	QAction a(this);
	switch(button)
	{
#if defined(QT_NO_DEBUG)
	default:	// Omitted in debug build so I get a warning when an enum is added. Default case needed for safety in release.
#endif
	case AuViewButton::VB_Invalid:
	case AuViewButton::VB_UNUSED1:
	case AuViewButton::VB_UNUSED3:
	case AuViewButton::VB_UNUSED4:
	case AuViewButton::VB_UNUSED5:
	case AuViewButton::VB_UNUSED6:
	case AuViewButton::VB_UNUSED7:
	case AuViewButton::VB_UNUSED8:
	case AuViewButton::VB_COUNT:
		return;
	case AuViewButton::VB_SaveSolitaire:
		connect(&a, SIGNAL(triggered()), this, SLOT(SaveSolitaireCheckPoint()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Agent:
	case AuViewButton::VB_Architect:
	case AuViewButton::VB_Programmer:
	case AuViewButton::VB_Minion:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleAgentUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_ArenaExit:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ArenaExit()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_ArenaRecall:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ArenaRecall()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_ArenaStatus:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ArenaStatus()), Qt::QueuedConnection);
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
	case AuViewButton::VB_ConstructionBd:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleConstructionUi()), Qt::QueuedConnection);
		break;
	case AuViewButton::VB_Exit:
		connect(&a, SIGNAL(triggered()), this, SLOT(DesignerExit()), Qt::QueuedConnection);
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
	case AuViewButton::VB_SpacecraftSpecs:
		connect(&a, SIGNAL(triggered()), pFrame, SLOT(ToggleSpacecraftSpecsUi()), Qt::QueuedConnection);
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

bool AuDesignerView::ViewButtonIsEnabled(AuViewButton::VIEWBUTTON vb) const
{
	for(QList<AuViewButton>::const_iterator bi = m_ViewButtons.cbegin(); bi != m_ViewButtons.cend(); bi++)
		if((*bi).m_Id == vb)
			return (*bi).m_Enabled;
	return false;
}

AuViewButton::VIEWBUTTON AuDesignerView::ViewButtonAt(const QPoint& mousePos) const
{
	if(g_AuClient.ViewButtonTransparency() >= 100)
		return AuViewButton::VB_Invalid;	// All view buttons are invisible.

	if(rect().width() <= 0 || rect().height() <= 0)
		return AuViewButton::VB_Invalid;

	double gldAspect = AuGLEffects::WindowAspectRatio(rect());

	// Calculate position of mouse in the range -1 to 1 in each axis.
	AuPoint mouse;
	mouse.x = (double)mousePos.x()/(double)rect().width() - .5;		// This gets us into the range -.5 to .5.
	mouse.x /= gldAspect;
	mouse.y = -((double)mousePos.y()/(double)rect().height() - .5);	// This gets us into the range -.5 to .5.
	mouse *= 2;

	bool hasEmpire = SceneAtomEmpire();
	double vbSubLength = ViewButtonSubLength();
	double vbRadius = ViewButtonRadius();
	double vbSubRadius = ViewButtonSubRadius();
	for(QList<AuViewButton>::const_iterator bi = m_ViewButtons.cbegin(); bi != m_ViewButtons.cend(); bi++)
	{
		if(!(*bi).IsVisible())
			continue;
		if((*bi).IsHidden(SceneContext(), hasEmpire))
			continue;

		AuPoint spos((*bi).m_ScreenPos);
		spos.x /= gldAspect;

		double dist;
		if((*bi).IsSub())
		{
			// Sub items are segments.
			//double vbSubLength = ViewButtonSubLength((*bi).Name());
			AuPoint epos((*bi).m_ScreenPos);
			epos.x += vbSubLength;
			epos.x /= gldAspect;
			dist = mouse.DistToSegment_2D(spos, epos);
			if(dist <= vbSubRadius)
				return (*bi).m_Id;
		}
		else
		{
			// Main items are circles.
			dist = mouse.Distance_2D(spos);
			if(dist <= vbRadius)
				return (*bi).m_Id;
		}
	}

	return AuViewButton::VB_Invalid;
}

AuViewButton::VIEWBUTTON AuDesignerView::ViewButtonNear(const QPoint& mousePos) const
{
	if(g_AuClient.ViewButtonTransparency() >= 100)
		return AuViewButton::VB_Invalid;	// All view buttons are invisible.

	if(rect().width() <= 0 || rect().height() <= 0)
		return AuViewButton::VB_Invalid;

	double gldAspect = AuGLEffects::WindowAspectRatio(rect());

	// Calculate position of mouse in the range -1 to 1 in each axis.
	AuPoint mouse;
	mouse.x = (double)mousePos.x()/(double)rect().width() - .5;		// This gets us into the range -.5 to .5.
	mouse.x /= gldAspect;
	mouse.y = -((double)mousePos.y()/(double)rect().height() - .5);	// This gets us into the range -.5 to .5.
	mouse *= 2;

	// Find closest button within search radius.
	bool hasEmpire = SceneAtomEmpire();
	double vbRadius = ViewButtonRadius();
	double vbMargin = vbRadius / 2.;
	double vbSearchRadius = vbRadius + .15f;					// Search radius for main buttons.
	double vbSubLength = ViewButtonSubLength();
	double vbSubSpacing = ViewButtonSubRadius() / 10.;
	double vbSubSearchRadius = ViewButtonSubRadius() + vbSubSpacing + vbMargin;	// Search radius for sub buttons.
	AuViewButton::VIEWBUTTON vbClosest = AuViewButton::VB_Invalid;
	double closestDist = 0;
	bool nearSub = false;
	for(QList<AuViewButton>::const_iterator bi = m_ViewButtons.cbegin(); bi != m_ViewButtons.cend(); bi++)
	{
		if(!(*bi).IsVisible())
			continue;
		if((*bi).IsHidden(SceneContext(), hasEmpire))
			continue;

		// When near a sub, don't find mains except my own.
		if(nearSub && (!(*bi).IsSub() || vbClosest != (*bi).m_Id))
			continue;

		AuPoint spos((*bi).m_ScreenPos);
		spos.x /= gldAspect;

		double dist;
		if((*bi).IsSub())
		{
			// Sub items are segments.
			//double vbSubLength = ViewButtonSubLength((*bi).Name());
			AuPoint epos((*bi).m_ScreenPos);
			epos.x += vbSubLength;
			epos.x /= gldAspect;
			dist = mouse.DistToSegment_2D(spos, epos);
			if(dist > vbSubSearchRadius)
				continue;
		}
		else
		{
			// Main items are circles.
			dist = mouse.Distance_2D(spos);
			if(dist > vbSearchRadius)
				continue;
		}

		if(dist < closestDist || vbClosest == AuViewButton::VB_Invalid)
		{
			if((*bi).IsSub())
				vbClosest = (*bi).m_Parent;
			else
				vbClosest = (*bi).m_Id;
			closestDist = dist;
			if((*bi).IsSub())
				nearSub = true;
		}
	}

	return vbClosest;
}

void AuDesignerView::PositionViewButtons()
{
	double gldAspect = AuGLEffects::WindowAspectRatio(rect());

	// Calculate the position of the first main button at the top left of the screen.
	double vbRadius = ViewButtonRadius();
	double vbMargin = vbRadius / 2.;
	double vbSpacing = vbRadius / 10.;
	double vbHeight = vbRadius * 2. + vbSpacing;
	AuPoint bpos(-1. + ((vbRadius + vbMargin) * gldAspect), 1. - (vbRadius + vbMargin));

	// Position main buttons first.
	bool hasEmpire = SceneAtomEmpire();
	int visibleButtons = 0;
	int buttonsPerColumn = 0;
	bool anysubs = false;
	AuPoint mpos;
	for(QList<AuViewButton>::iterator bi = m_ViewButtons.begin(); bi != m_ViewButtons.end(); bi++)
	{
		if((*bi).IsHidden(SceneContext(), hasEmpire))
			continue;

		if((*bi).IsSub())
		{
			anysubs = true;
			continue;
		}

		visibleButtons++;

		(*bi).m_ScreenPos = bpos;
		if((*bi).m_Id == m_ViewButtonNear)
			mpos = bpos;

		// Walk down the column.
		bpos.y -= vbHeight;
		if(bpos.y < -1 + vbRadius + vbSpacing)
		{
			// Start a new column at the bottom.
			if(!buttonsPerColumn)
				buttonsPerColumn = visibleButtons;

			bpos.y = 1. - (vbRadius + vbMargin);
			bpos.x += vbHeight * gldAspect;
		}
	}

	if(visibleButtons)
	{
		if(!buttonsPerColumn)
			m_ViewButtonColumns = 1;
		else
			m_ViewButtonColumns = (visibleButtons + buttonsPerColumn - 1) / buttonsPerColumn;
	}
	else
		m_ViewButtonColumns = 0;

	if(!anysubs)
		return;

	// Calculate the position of the first sub button off to the right of its main button.
	double vbSubRadius = ViewButtonSubRadius();
	double vbSubSpacing = vbSubRadius / 10.;
	double vbSubHeight = vbSubRadius * 2. + vbSubSpacing;
	bpos = mpos;						// Center position of my main button.
	bpos.x += (vbRadius + vbMargin + vbSubRadius)*gldAspect;

	// Position sub buttons now that we know the location of the current main button.
	for(QList<AuViewButton>::iterator bi = m_ViewButtons.begin(); bi != m_ViewButtons.end(); bi++)
	{
		if((*bi).IsHidden(SceneContext(), hasEmpire))
			continue;

		// Subs are first in the list.
		if(!(*bi).IsSub())
			break;

		(*bi).m_ScreenPos = bpos;

		// Walk down the column.
		bpos.y -= vbSubHeight;
	}
}

void AuDesignerView::ToggleFullScreen()
{
	PauseRendering();
	emit ToggleScreenReady();
}

quint16 AuDesignerView::DesignerObjectModeTargetPart() const
{
	return m_RenderInfo->DesignerObjectModeTargetPart();
}

void AuDesignerView::ProcessDesignerMouseDoubleClick(QMouseEvent* /*e*/)
{
}

void AuDesignerView::ProcessDesignerMousePress(QMouseEvent* e)
{
	// Designer only cares about left button.
	if(e->button() != Qt::LeftButton)
		return;
	// Designer does nothing in preview mode.
	if(IsDesignPreview())
		return;

	if(DesignerObjectMode())
		m_VertexSelected = false;
	else
		m_AnchorSelected = false;

	m_RenderInfo->m_DragCopy = false;
	m_RenderInfo->m_DragT.LoadIdentity();

	/*
	// Check for grips of the design plane rotator.
	m_RotatorGrip = -1;
	m_RotatorT.LoadIdentity();
	if(g_AuClient.ShowDesignPlaneRotator())
	{
		// Design plane rotator is visible.
		// Check for mouse press in one of the rotator's grips.
		QRect rcmouse(e->pos().x()-5, e->pos().y()-5, 10,10);
		for(QMap<int, QPoint>::const_iterator ri = m_RotatorGrips.constBegin(); ri != m_RotatorGrips.constEnd(); ri++)
		{
			if(rcmouse.contains(*ri))
			{
				m_RotatorGrip = ri.key();
				return;
			}
		}
	}
	*/

	QString anchorDesc(m_AnchorDesc);

	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			if((e->modifiers() & (Qt::ControlModifier|Qt::ShiftModifier)) == (Qt::ControlModifier|Qt::ShiftModifier))
			{
				// Control and Shift key modifiers.
				if(m_AnchorLoc != QPoint(-1,-1))
				{
					// Determine selection rectangle coordinates from the pointer press location to the pointer anchor location.
					// This forms a pixel rectangle for selection, like dragging the cursor.  
					/*
					QPoint topLeft;
					if(m_AnchorLoc.x() < m_MousePress.x())
						topLeft.setX(m_AnchorLoc.x());
					else
						topLeft.setX(m_MousePress.x());
					if(m_AnchorLoc.y() < m_MousePress.y())
						topLeft.setY(m_AnchorLoc.y());
					else
						topLeft.setY(m_MousePress.y());

					QSize size;
					size.setWidth(labs(m_MousePress.x() - m_AnchorLoc.x()));
					size.setHeight(labs(m_MousePress.y() - m_AnchorLoc.y()));

					QRect pixelRect(topLeft, size);
					*/

					QPointF viewAnchor = PixelToViewport(this, m_AnchorLoc);
					QPointF viewMouse = PixelToViewport(this, m_MousePress);

					QPointF topLeft;
					if(viewAnchor.x() < viewMouse.x())
						topLeft.setX(viewAnchor.x());
					else
						topLeft.setX(viewMouse.x());
					if(viewAnchor.y() < viewMouse.y())
						topLeft.setY(viewAnchor.y());
					else
						topLeft.setY(viewMouse.y());

					QSizeF size;
					size.setWidth(fabs(viewMouse.x() - viewAnchor.x()));
					size.setHeight(fabs(viewMouse.y() - viewAnchor.y()));

					QRectF pixelRect(topLeft, size);

					if(DesignerObjectMode())
					{
						// Toggle the selection state of all the vertices in the rectangle.
						pde->TogglePartObjectSelection(m_tProjection, m_tCamera.Inverse(), sceneAtom,
								m_RenderInfo->DesignerSelection(), pixelRect);
					}
					else
					{
						// Toggle the selection state of all the parts in the rectangle.
						AuScSelection partsInRect;
						pde->FindPartsInArea(m_tProjection, m_tCamera.Inverse(), sceneAtom, pixelRect, partsInRect);
						for(AuScSelection::iterator ri = partsInRect.begin(); ri != partsInRect.end(); ri++)
							if(m_RenderInfo->DesignerSelection().contains(ri.key()))
								m_RenderInfo->DesignerSelection().remove(ri.key());		// Deselect the selected part.
							else
								m_RenderInfo->DesignerSelection().insert(ri.key(),*ri);	// Select the unselected part.
					}
				}
			}
			else if(e->modifiers() == Qt::ShiftModifier)
			{
				// Shift key modifier.
				if(m_AnchorLoc != QPoint(-1,-1))
				{
					// Determine selection rectangle coordinates from the pointer press location to the pointer anchor location.
					// This forms a pixel rectangle for selection, like dragging the cursor.  
					/*
					QPoint topLeft;
					if(m_AnchorLoc.x() < m_MousePress.x())
						topLeft.setX(m_AnchorLoc.x());
					else
						topLeft.setX(m_MousePress.x());
					if(m_AnchorLoc.y() < m_MousePress.y())
						topLeft.setY(m_AnchorLoc.y());
					else
						topLeft.setY(m_MousePress.y());

					QSize size;
					size.setWidth(labs(m_MousePress.x() - m_AnchorLoc.x()));
					size.setHeight(labs(m_MousePress.y() - m_AnchorLoc.y()));

					QRect pixelRect(topLeft, size);
					*/

					QPointF viewAnchor = PixelToViewport(this, m_AnchorLoc);
					QPointF viewMouse = PixelToViewport(this, m_MousePress);

					QPointF topLeft;
					if(viewAnchor.x() < viewMouse.x())
						topLeft.setX(viewAnchor.x());
					else
						topLeft.setX(viewMouse.x());
					if(viewAnchor.y() < viewMouse.y())
						topLeft.setY(viewAnchor.y());
					else
						topLeft.setY(viewMouse.y());

					QSizeF size;
					size.setWidth(fabs(viewMouse.x() - viewAnchor.x()));
					size.setHeight(fabs(viewMouse.y() - viewAnchor.y()));

					QRectF pixelRect(topLeft, size);

					if(DesignerObjectMode())
					{
						// No Control key so deselect vertices first.
						m_RenderInfo->DesignerClearPartObjectSelection();	// vertices, edges and faces

						// Select all the vertices in the rectangle.
						pde->FindPartObjectsInArea(m_tProjection, m_tCamera.Inverse(), sceneAtom, pixelRect,
								m_RenderInfo->DesignerSelection());
					}
					else
					{
						// No Control key so deselect first.
						m_RenderInfo->DesignerClearPartSelection();

						// Select all the elements in the rectangle.
						pde->FindPartsInArea(m_tProjection, m_tCamera.Inverse(), sceneAtom, pixelRect,
								m_RenderInfo->DesignerSelection());
					}
				}
			}
			else if(e->modifiers() == Qt::ControlModifier)
			{
				// Control key modifier only.
				// Locate the element at the pointer location, if any.
				m_AnchorLoc = e->pos();
				QPointF viewPos = PixelToViewport(this, e->pos());

				if(DesignerObjectMode())
				{
					m_RenderInfo->SetDragPart(pde->FindPartObjectAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos,
								m_RenderInfo->DesignerPartObjectSelection()));
					if(m_RenderInfo->DragPart())
					{
						// Pointer on an element.
						AuPoint dragRef = FromPixelToDesignPlane(e->pos(), false, 0);
						AuPoint vertLoc;
						if(pde->FindVertexAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos,
									m_RenderInfo->DesignerPartObjectSelection(), m_RenderInfo->DesignerObjectMode(),
									m_RenderInfo->DesignerObjectIsolate(), &vertLoc))
							dragRef = vertLoc;	// Reference existing vertex for accuracy.
						else if(pde->FindEdgeAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos,
									m_RenderInfo->DesignerPartObjectSelection(), m_RenderInfo->DesignerObjectMode(),
									m_RenderInfo->DesignerObjectIsolate(), &vertLoc))
							dragRef = vertLoc;	// Reference existing vertex for accuracy.
						m_DragConstraint = 0;
						m_RenderInfo->DragStart(dragRef, m_DesignPlane);

						// Prompt
						QString prompt = QObject::tr("<html>"
								"<span style=\"color: %2;\">Drag selection. "
								"</span>"
								"<span style=\"color: rgb(255, 255, 0);\">%3,%4,%5%1Axis. </span>"
								"</html>")
							.arg('=')
							.arg(palette().color(QPalette::WindowText).name())
							.arg(ControlBinding(CI_DragConstrainX))
							.arg(ControlBinding(CI_DragConstrainY))
							.arg(ControlBinding(CI_DragConstrainZ));
						CommandPromptChanged(prompt);

						if(pde->SelectPartObjectsAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos,
									m_RenderInfo->DesignerSelection()))
						{
							// Pointer on an unselected vertex selected it.
							m_VertexSelected = true;
						}
						else
						{
							// Pointer on a selected vertex.
							// Prepare for drag copy?
						}
					}
					else
					{
						// Pointer on whitespace.
					}
				}
				else
				{
					m_RenderInfo->SetAnchorPart(pde->FindPartAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos));
					if(m_RenderInfo->AnchorPart())
					{
						// Pointer on an element.
						AuPoint dragRef = FromPixelToDesignPlane(e->pos(), false, 0);
						AuPoint vertLoc;
						if(pde->FindVertexAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos,
									m_RenderInfo->DesignerPartObjectSelection(), m_RenderInfo->DesignerObjectMode(),
									m_RenderInfo->DesignerObjectIsolate(), &vertLoc))
							dragRef = vertLoc;	// Reference existing vertex for accuracy.
						else if(pde->FindEdgeAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos,
									m_RenderInfo->DesignerPartObjectSelection(), m_RenderInfo->DesignerObjectMode(),
									m_RenderInfo->DesignerObjectIsolate(), &vertLoc))
							dragRef = vertLoc;	// Reference existing vertex for accuracy.
						m_DragConstraint = 0;
						m_RenderInfo->DragStart(dragRef, m_DesignPlane);

						// Prompt
						QString prompt = QObject::tr("<html>"
								"<span style=\"color: %2;\">Drag selection. "
								"</span>"
								"<span style=\"color: rgb(255, 255, 0);\">%3,%4,%5%1Axis. </span>"
								"</html>")
							.arg('=')
							.arg(palette().color(QPalette::WindowText).name())
							.arg(ControlBinding(CI_DragConstrainX))
							.arg(ControlBinding(CI_DragConstrainY))
							.arg(ControlBinding(CI_DragConstrainZ));
						CommandPromptChanged(prompt);

						anchorDesc = pde->PartSelectedDesc(m_RenderInfo->AnchorPart());
						AuScSelection::iterator si = m_RenderInfo->DesignerSelection().find(m_RenderInfo->AnchorPart());
						if(si != m_RenderInfo->DesignerSelection().end())
						{
							// Pointer on a selected element.
							// Prepare for drag copy?
						}
						else
						{
							// Pointer on an unselected element.
							m_RenderInfo->DesignerSelection().insert(m_RenderInfo->AnchorPart(), AuScSelectInfo());
							m_AnchorSelected = true;
						}
					}
					else
					{
						// Pointer on whitespace.
						anchorDesc = "";
					}
				}
			}
			else
			{
				// No keyboard modifiers.
				// Locate the element at the pointer location, if any.
				m_AnchorLoc = e->pos();
				QPointF viewPos = PixelToViewport(this, e->pos());

				if(DesignerObjectMode())
				{
					m_RenderInfo->SetDragPart(pde->FindPartObjectAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos,
								m_RenderInfo->DesignerPartObjectSelection()));
					if(m_RenderInfo->DragPart())
					{
						// Pointer on an element.
						AuPoint dragRef = FromPixelToDesignPlane(e->pos(), false, 0);
						AuPoint vertLoc;
						if(pde->FindVertexAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos,
									m_RenderInfo->DesignerPartObjectSelection(), m_RenderInfo->DesignerObjectMode(),
									m_RenderInfo->DesignerObjectIsolate(), &vertLoc))
							dragRef = vertLoc;	// Reference existing vertex for accuracy.
						else if(pde->FindEdgeAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos,
									m_RenderInfo->DesignerPartObjectSelection(), m_RenderInfo->DesignerObjectMode(),
									m_RenderInfo->DesignerObjectIsolate(), &vertLoc))
							dragRef = vertLoc;	// Reference existing vertex for accuracy.
						m_DragConstraint = 0;
						m_RenderInfo->DragStart(dragRef, m_DesignPlane);

						// Prompt
						QString prompt = QObject::tr("<html>"
								"<span style=\"color: %2;\">Drag selection. "
								"</span>"
								"<span style=\"color: rgb(255, 255, 0);\">%3,%4,%5%1Axis. </span>"
								"</html>")
							.arg('=')
							.arg(palette().color(QPalette::WindowText).name())
							.arg(ControlBinding(CI_DragConstrainX))
							.arg(ControlBinding(CI_DragConstrainY))
							.arg(ControlBinding(CI_DragConstrainZ));
						CommandPromptChanged(prompt);

						if(pde->SelectPartObjectsAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos,
									m_RenderInfo->DesignerSelection()))
						{
							// A vertex was selected.
							// Deselect all vertices and reselect just the one, for typical unmodified click behavior.
							m_RenderInfo->DesignerClearPartObjectSelection();	// vertices, edges and faces
							pde->SelectPartObjectsAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos,
									m_RenderInfo->DesignerSelection());
							m_VertexSelected = true;
						}
						else
						{
							// A vertex was not selected.
							// Prepare for drag move?
						}
					}
					else
					{
						// Pointer on whitespace.
						m_RenderInfo->ClearDragPart();
						m_RenderInfo->DesignerClearPartObjectSelection();	// vertices, edges and faces
					}
				}
				else
				{
					m_RenderInfo->SetAnchorPart(pde->FindPartAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos));
					if(m_RenderInfo->AnchorPart())
					{
						// Pointer on an element.
						AuPoint dragRef = FromPixelToDesignPlane(e->pos(), false, 0);
						AuPoint vertLoc;
						if(pde->FindVertexAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos,
									m_RenderInfo->DesignerPartObjectSelection(), m_RenderInfo->DesignerObjectMode(),
									m_RenderInfo->DesignerObjectIsolate(), &vertLoc))
							dragRef = vertLoc;	// Reference existing vertex for accuracy.
						else if(pde->FindEdgeAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos,
									m_RenderInfo->DesignerPartObjectSelection(), m_RenderInfo->DesignerObjectMode(),
									m_RenderInfo->DesignerObjectIsolate(), &vertLoc))
							dragRef = vertLoc;	// Reference existing vertex for accuracy.
						m_DragConstraint = 0;
						m_RenderInfo->DragStart(dragRef, m_DesignPlane);

						// Prompt
						QString prompt = QObject::tr("<html>"
								"<span style=\"color: %2;\">Drag selection. "
								"</span>"
								"<span style=\"color: rgb(255, 255, 0);\">%3,%4,%5%1Axis. </span>"
								"</html>")
							.arg('=')
							.arg(palette().color(QPalette::WindowText).name())
							.arg(ControlBinding(CI_DragConstrainX))
							.arg(ControlBinding(CI_DragConstrainY))
							.arg(ControlBinding(CI_DragConstrainZ));
						CommandPromptChanged(prompt);

						anchorDesc = pde->PartSelectedDesc(m_RenderInfo->AnchorPart());
						AuScSelection::iterator si = m_RenderInfo->DesignerSelection().find(m_RenderInfo->AnchorPart());
						if(si != m_RenderInfo->DesignerSelection().end())
						{
							// Pointer on a selected element.
							// Prepare for drag move?
						}
						else
						{
							// Pointer on an unselected element.
							m_RenderInfo->DesignerClearPartSelection();
							m_RenderInfo->DesignerSelection().insert(m_RenderInfo->AnchorPart(), AuScSelectInfo());
							m_AnchorSelected = true;
						}
					}
					else
					{
						// Pointer on whitespace.
						anchorDesc = "";
						m_RenderInfo->ClearAnchorPart();
						m_RenderInfo->DesignerClearPartSelection();
					}
				}
			}

			// Notify of possible changes for control enable/disable by the frame.
			emit DesignerSelectionChanged();
		}
	}
	UnlockSceneAtom();

	if(anchorDesc != m_AnchorDesc)
	{
		m_AnchorDesc = anchorDesc;
		emit DesignerAnchorChanged(m_AnchorDesc);
	}
}

void AuDesignerView::ProcessDesignerMouseRelease(QMouseEvent* e)
{
	// Designer only cares about left button.
	if(e->button() != Qt::LeftButton)
		return;
	// Designer does nothing in preview mode.
	if(IsDesignPreview())
		return;

	// Check for grips of the design plane rotator.
	/*
	if(m_RotatorGrip >= 0)
	{
		// A design plane rotator grip was dragged. Figure out what to do.
		if(m_RotatorGrip == 0)
		{
			// Move the design plane.
			// We do not move the selected shapes with the rotator. It's necessary to move it independently of the selected shapes
			// to establish the pivot point when rotating.
			AuMatrix4x4 dT;
			dT.Translate(m_DesignPlane.Translation() + m_RenderInfo->m_DragT.Translation());
			dT.Rotate(m_DesignPlane.GetRotation());
			SetDesignPlane(dT);
		}
		else
		{
			// Rotate the selected shapes.
			if(!m_RotatorT.IsIdentity())
			{
				// Determine the transformation matrix to rotate around the design plane origin.
				if(e->modifiers() == Qt::ControlModifier)
					auApp->DesignerCopyParts(m_RenderInfo->m_DesignerSelection, m_RenderInfo->m_DragT,
					m_RenderInfo->DesignerObjectMode());
				else
					auApp->DesignerMoveParts(m_RenderInfo->m_DesignerSelection, m_RenderInfo->m_DragT,
					m_RenderInfo->DesignerObjectMode());

				// Rotate the design plane.
				AuMatrix4x4 dT = m_DesignPlane * m_RotatorT;
				SetDesignPlane(dT);
			}
		}
		m_RotatorGrip = -1;
	}
	else
	*/
	{
		// Move/copy ids.
		AuScSelection  moveIds;
		AuScSelection  copyIds;

		DaPerson* sceneAtom;
		LockSceneAtom(&sceneAtom);
		if(sceneAtom)
		{
			const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
			if(pde)
			{
				if((e->modifiers() & (Qt::ControlModifier|Qt::ShiftModifier)) == (Qt::ControlModifier|Qt::ShiftModifier))
				{
					// Control and Shift key modifiers.
				}
				else if(e->modifiers() == Qt::ShiftModifier)
				{
					// Shift key modifier.
				}
				else if(e->modifiers() == Qt::ControlModifier)
				{
					// Control key modifier only.
					if(m_MouseDragging)
					{
						if(DragSelectedParts())
						{
							// Dragging the selected elements with Control modifier results in copying them.
							if(DesignerObjectMode())
								copyIds = m_RenderInfo->DesignerPartObjectSelection();
							else
								copyIds = m_RenderInfo->DesignerPartSelection();
						}
						else
						{
							// Parts are not being dragged. Select the parts in the drag selection rectangle.
							// Determine selection rectangle coordinates from the pointer press location to the pointer anchor
							// location.
							// This forms a pixel rectangle for selection, like dragging the cursor.  
							/*
							QPoint topLeft;
							if(m_AnchorLoc.x() < e->pos().x())
								topLeft.setX(m_AnchorLoc.x());
							else
								topLeft.setX(e->pos().x());
							if(m_AnchorLoc.y() < e->pos().y())
								topLeft.setY(m_AnchorLoc.y());
							else
								topLeft.setY(e->pos().y());

							QSize size;
							size.setWidth(labs(e->pos().x() - m_AnchorLoc.x()));
							size.setHeight(labs(e->pos().y() - m_AnchorLoc.y()));

							QRect pixelRect(topLeft, size);
							*/

							QPointF viewAnchor = PixelToViewport(this, m_AnchorLoc);
							QPointF viewPos = PixelToViewport(this, e->pos());

							QPointF topLeft;
							if(viewAnchor.x() < viewPos.x())
								topLeft.setX(viewAnchor.x());
							else
								topLeft.setX(viewPos.x());
							if(viewAnchor.y() < viewPos.y())
								topLeft.setY(viewAnchor.y());
							else
								topLeft.setY(viewPos.y());

							QSizeF size;
							size.setWidth(fabs(viewPos.x() - viewAnchor.x()));
							size.setHeight(fabs(viewPos.y() - viewAnchor.y()));

							QRectF pixelRect(topLeft, size);

							if(DesignerObjectMode())
							{
								// Toggle the selection state of all the vertices in the rectangle.
								pde->TogglePartObjectSelection(m_tProjection, m_tCamera.Inverse(), sceneAtom,
										m_RenderInfo->DesignerSelection(), pixelRect);
							}
							else
							{
								// Toggle the selection state of all the elements in the rectangle.
								AuScSelection partsInRect;
								pde->FindPartsInArea(m_tProjection, m_tCamera.Inverse(), sceneAtom, pixelRect, partsInRect);
								for(AuScSelection::iterator ri = partsInRect.begin(); ri != partsInRect.end(); ri++)
									if(m_RenderInfo->DesignerSelection().contains(ri.key()))
										m_RenderInfo->DesignerSelection().remove(ri.key());		// Deselect the selected element.
									else
										m_RenderInfo->DesignerSelection().insert(ri.key(),*ri);	// Select the unselected element.
							}
							emit DesignerSelectionChanged();
						}
					}
					else
					{
						// User didn't drag anything.
						if(DesignerObjectMode())
						{
							if(!m_VertexSelected)
							{
								// We didn't just select a vertex so we are permitted to deselect it. Without this test we
								// would deselect it on the mouse up immediately following the mouse down that selected it.
								// Locate the part at the pointer location, if any.
								QPointF viewPos = PixelToViewport(this, e->pos());
								if(pde->DeselectPartObjectsAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos,
											m_RenderInfo->DesignerSelection()))
									emit DesignerSelectionChanged();
							}
						}
						else
						{
							if(!m_AnchorSelected)
							{
								// We didn't just select the anchor element so we are permitted to deselect it. Without this test we
								// would deselect it on the mouse up immediately following the mouse down that selected it.
								// Locate the part at the pointer location, if any.
								QPointF viewPos = PixelToViewport(this, e->pos());
								quint16 partId = pde->FindPartAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos);
								if(partId)
								{
									// Pointer released over part with Control modifier. Anchor element 0 tells us to deselect it.
									m_RenderInfo->DesignerSelection().remove(partId);
									emit DesignerSelectionChanged();
								}
							}
						}
					}
				}
				else
				{
					// No keyboard modifiers.
					if(m_MouseDragging)
					{
						if(DragSelectedParts())
						{
							// Dragging the selected elements with no modifiers results in moving them.
							if(DesignerObjectMode())
								moveIds = m_RenderInfo->DesignerPartObjectSelection();
							else
								moveIds = m_RenderInfo->DesignerPartSelection();
						}
						else
						{
							// Parts are not being dragged. Select the parts in the drag selection rectangle.
							// Determine selection rectangle coordinates from the pointer press location to the pointer anchor
							// location.
							// This forms a pixel rectangle for selection, like dragging the cursor.  
							/*
							QPoint topLeft;
							if(m_AnchorLoc.x() < e->pos().x())
								topLeft.setX(m_AnchorLoc.x());
							else
								topLeft.setX(e->pos().x());
							if(m_AnchorLoc.y() < e->pos().y())
								topLeft.setY(m_AnchorLoc.y());
							else
								topLeft.setY(e->pos().y());

							QSize size;
							size.setWidth(labs(e->pos().x() - m_AnchorLoc.x()));
							size.setHeight(labs(e->pos().y() - m_AnchorLoc.y()));

							QRect pixelRect(topLeft, size);
							*/

							QPointF viewAnchor = PixelToViewport(this, m_AnchorLoc);
							QPointF viewPos = PixelToViewport(this, e->pos());

							QPointF topLeft;
							if(viewAnchor.x() < viewPos.x())
								topLeft.setX(viewAnchor.x());
							else
								topLeft.setX(viewPos.x());
							if(viewAnchor.y() < viewPos.y())
								topLeft.setY(viewAnchor.y());
							else
								topLeft.setY(viewPos.y());

							QSizeF size;
							size.setWidth(fabs(viewPos.x() - viewAnchor.x()));
							size.setHeight(fabs(viewPos.y() - viewAnchor.y()));

							QRectF pixelRect(topLeft, size);

							if(DesignerObjectMode())
							{
								// Select all the vertices in the rectangle.
								m_RenderInfo->DesignerClearPartObjectSelection();	// vertices, edges and faces
								pde->FindPartObjectsInArea(m_tProjection, m_tCamera.Inverse(), sceneAtom, pixelRect,
										m_RenderInfo->DesignerSelection());
							}
							else
							{
								// Select all the meshes in the rectangle.
								m_RenderInfo->DesignerClearPartSelection();
								pde->FindPartsInArea(m_tProjection, m_tCamera.Inverse(), sceneAtom, pixelRect,
										m_RenderInfo->DesignerSelection());
							}
							emit DesignerSelectionChanged();
						}
					}
				}
			}
		}
		UnlockSceneAtom();

		if(DesignerObjectMode())
		{
			if(!moveIds.isEmpty())
				auApp->DesignerMovePartObjects(moveIds, m_RenderInfo->m_DragT);
			else if(!copyIds.isEmpty())
				auApp->DesignerCopyPartObjects(copyIds, m_RenderInfo->m_DragT);
		}
		else
		{
			if(!moveIds.isEmpty())
				auApp->DesignerMoveParts(moveIds, m_RenderInfo->m_DragT);
			else if(!copyIds.isEmpty())
				auApp->DesignerCopyParts(copyIds, m_RenderInfo->m_DragT);
		}
	}

	m_RenderInfo->DragEnd();
	CommandPromptChanged("");
}

void AuDesignerView::ProcessDesignerMouseMove(QMouseEvent* e)
{
	if(!m_MouseDragging)
		return;
	// Designer does nothing in preview mode.
	if(IsDesignPreview())
		return;

	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			/*
			if(m_RotatorGrip > 0)
			{
				// Rotate the selected shapes.
				// Determine the transformation matrix to rotate around the design plane origin.
				m_RotatorT.LoadIdentity();
				AuPoint dorg = m_DesignPlane.Translation();			// Design plane origin.
				if(m_RotatorGrip == 1)
				{
					// X axis rotating around the Y axis.
					// Determine the cursor location in the XZ plane of the design plane.
					AuMatrix4x4 tplane(m_DesignPlane);
					tplane.RotateX(PI/2.);
					AuPoint dragTo = FromPixelToPlane(e->pos(), tplane, false, &dorg);
					dragTo = tplane.UnProject(dragTo);			// In coordinates relative to the design plane.
					double rotang = dragTo.ArcTangent_2D();
					if(rotang != 0.)
						m_RotatorT.RotateY(-rotang);
				}
				else if(m_RotatorGrip == 2)
				{
					// X axis rotating around the Z axis.
					// Determine the cursor location in the XY plane of the design plane.
					AuPoint dragTo = FromPixelToPlane(e->pos(), m_DesignPlane, true, &dorg);
					dragTo = m_DesignPlane.UnProject(dragTo);			// In coordinates relative to the design plane.
					double rotang = dragTo.ArcTangent_2D();
					if(rotang != 0.)
						m_RotatorT.RotateZ(rotang);
				}
				else if(m_RotatorGrip == 3)
				{
					// Y axis rotating around the Z axis.
					// Determine the cursor location in the XY plane of the design plane.
					AuPoint dragTo = FromPixelToPlane(e->pos(), m_DesignPlane, true, &dorg);
					dragTo = m_DesignPlane.UnProject(dragTo);			// In coordinates relative to the design plane.
					double rotang = dragTo.ArcTangent_2D() - PI/2;
					if(rotang != 0.)
						m_RotatorT.RotateZ(rotang);
				}
				else if(m_RotatorGrip == 4)
				{
					// Y axis rotating around the X axis.
					// Determine the cursor location in the YZ plane of the design plane.
					AuMatrix4x4 tplane(m_DesignPlane);
					tplane.RotateX(PI/2.);
					tplane.RotateY(PI/2.);
					AuPoint dragTo = FromPixelToPlane(e->pos(), tplane, false, &dorg);
					dragTo = tplane.UnProject(dragTo);			// In coordinates relative to the design plane.
					double rotang = dragTo.ArcTangent_2D();
					if(rotang != 0.)
						m_RotatorT.RotateX(rotang);
				}
				else if(m_RotatorGrip == 5)
				{
					// Z axis rotating around the X axis.
					// Determine the cursor location in the YZ plane of the design plane.
					AuMatrix4x4 tplane(m_DesignPlane);
					tplane.RotateX(PI/2.);
					tplane.RotateY(PI/2.);
					AuPoint dragTo = FromPixelToPlane(e->pos(), tplane, false, &dorg);
					dragTo = tplane.UnProject(dragTo);			// In coordinates relative to the design plane.
					double rotang = dragTo.ArcTangent_2D() - PI/2;
					if(rotang != 0.)
						m_RotatorT.RotateX(rotang);
				}
				else// if(m_RotatorGrip == 6)
				{
					// Z axis rotating around the Y axis.
					// Determine the cursor location in the XZ plane of the design plane.
					AuMatrix4x4 tplane(m_DesignPlane);
					tplane.RotateX(PI/2.);
					AuPoint dragTo = FromPixelToPlane(e->pos(), tplane, false, &dorg);
					dragTo = tplane.UnProject(dragTo);			// In coordinates relative to the design plane.
					double rotang = dragTo.ArcTangent_2D() - PI/2;
					if(rotang != 0.)
						m_RotatorT.RotateY(-rotang);
				}

				// Create shape transformation that rotates around current design plane origin.
				AuMatrix4x4 dragT;
				dragT.Translate(dorg);
				dragT.Rotate(m_DesignPlane.GetRotation());
				dragT *= m_RotatorT;
				dragT.UnRotate(m_DesignPlane.GetRotation());
				dragT.Translate(-dorg);
				if(dragT != m_RenderInfo->m_DragT)
					m_RenderInfo->m_DragT = dragT;
			}
			else*/ if(m_MouseDragging)
			{
				// Moving/copying the selected parts or moving the design plane.
				// Determine drag vector in design plane.
				AuPoint dragStart;
				/*
				if(m_RotatorGrip == 0)
					dragStart = m_DesignPlane.Translation();
				else
				*/
					dragStart = FromPixelToDesignPlane(m_MousePress, false, 0);
				AuPoint dragEnd = FromPixelToDesignPlane(e->pos(), false, &dragStart);
				AuPoint dragVec = dragEnd - dragStart;

				QPointF viewPos = PixelToViewport(this, e->pos());

				// Attempt to snap to existing shapes.
				bool snapped = false;
				if(g_AuClient.DesignerSnapToVertices() &&
						pde->FindVertexAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos,
							m_RenderInfo->DesignerPartObjectSelection(), m_RenderInfo->DesignerObjectMode(),
							m_RenderInfo->DesignerObjectIsolate()))
					snapped = true;
				else if(g_AuClient.DesignerSnapToEdges() &&
						pde->FindEdgeAtPixel(m_tProjection, m_tCamera.Inverse(), sceneAtom, viewPos,
							m_RenderInfo->DesignerPartObjectSelection(), m_RenderInfo->DesignerObjectMode(),
							m_RenderInfo->DesignerObjectIsolate()))
					snapped = true;

				if(!snapped)
				{
					// Cursor did not snap to a shape.
					if(g_AuClient.DesignerSnapToGrid() || m_DragConstraint)
					{
						// Grid rounding and axis constraints must be applied in grid space.
						dragVec = m_DesignPlane.UnProjectVector(dragVec);	// Loc now in design plane coordinate space.

						if(g_AuClient.DesignerSnapToGrid())
						{
							// Apply grid rounding to the drag vector.
							double gridSpacing = g_AuClient.DesignerGridSpacingFt();
							if(gridSpacing > 0.)
							{
								// Round the point in the coordinate space of the design plane.
								dragVec.Round(gridSpacing);
							}
						}

						// Apply axis constraints.
						if(m_DragConstraint == 1)
						{
							dragVec.y = 0;
							dragVec.z = 0;
						}
						else if(m_DragConstraint == 2)
						{
							dragVec.x = 0;
							dragVec.z = 0;
						}
						else if(m_DragConstraint == 4)
						{
							dragVec.x = 0;
							dragVec.y = 0;
						}

						dragVec = m_DesignPlane.ProjectVector(dragVec);		// Loc now in design studio coordinates.
					}
				}

				m_RenderInfo->m_DragT.LoadIdentity();
				m_RenderInfo->m_DragCopy = e->modifiers() == Qt::ControlModifier;

				if(m_RenderInfo->IsDragging() && DragSelectedParts())
				{
					// Dragging selected parts. Update part drag info.
					if(!dragVec.IsZero())
						m_RenderInfo->m_DragT.Translate(dragVec);

					dragVec = DesignPlane().UnProjectVector(dragVec);		// Show drag vector relative to design plane.
					AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
					pFrame->DesignerBlipCoordinate(dragVec);
				}
			}
		}
	}
	UnlockSceneAtom();
}

void AuDesignerView::SetDesignerObjectIsolate(bool objectIsolate)
{
	if(!IsInNewDesignEnvironment())
		return;

	// Cannot be in face mode with no selection or with other architects' sealed parts selected.
	if(!DesignerObjectModeOk())
		m_RenderInfo->SetDesignerObjectIsolate(false);
	else
		m_RenderInfo->SetDesignerObjectIsolate(objectIsolate);
}

void AuDesignerView::SetDesignerObjectMode(bool objectMode)
{
	if(!IsInNewDesignEnvironment())
		return;

	// Cannot be in face mode with no selection or with other architects' sealed parts selected.
	if(!DesignerObjectModeOk())
		m_RenderInfo->SetDesignerObjectMode(false);
	else
		m_RenderInfo->SetDesignerObjectMode(objectMode);
}

AuMatrix4x4 AuDesignerView::DesignPlane() const
{
	if(Command())
		return Command()->DesignPlane();

	if(!m_RenderInfo->IsDragging())
		return m_DesignPlane;

	// Simple dragging.
	if(m_DragConstraint == 4)
	{
		// Dragging in Z requires us to rotate the drag plane.
		AuPoint org = m_RenderInfo->DragPlane().Translation();
		AuPoint rot = m_DesignPlane.GetRotation();
		AuMatrix4x4 dragPlane;
		dragPlane.Translate(org);
		dragPlane.Rotate(rot);
		dragPlane.RotateX(PI/2);
		return dragPlane;
	}
	else
		return m_RenderInfo->DragPlane();
}

void AuDesignerView::DesignerDragStart(const AuPoint& ref)
{
	m_RenderInfo->DragStart(ref, m_DesignPlane);
	m_RenderInfo->m_DragT.LoadIdentity();
	m_RenderInfo->m_DragCopy = false;
}

void AuDesignerView::DesignerDragging(const AuMatrix4x4& t, bool isCopy)
{
	m_RenderInfo->m_DragT = t;
	m_RenderInfo->m_DragCopy = isCopy;
}

void AuDesignerView::DesignerMove(const AuPoint& ref)
{
	if(!IsInNewDesignEnvironment())
		return;
	StartCommand(new DdScMove(ref, this));
}

void AuDesignerView::DesignerRotate(const AuPoint& ref)
{
	if(!IsInNewDesignEnvironment())
		return;
	StartCommand(new DdScRotate(ref, this));
}

void AuDesignerView::DesignerStretch(const AuPoint& ref, double stretchBase)
{
	if(!IsInNewDesignEnvironment())
		return;

	StartCommand(new DdScStretch(ref, stretchBase, this));
}

void AuDesignerView::DesignerSetPartSelection(AuScIdSet partIds)
{
	// Remove selected items that are not in 'partIds'.
	AuScSelection::iterator si = m_RenderInfo->DesignerSelection().begin();
	while(si != m_RenderInfo->DesignerSelection().end())
	{
		if(!partIds.m_Ids.contains(si.key()))
			si = m_RenderInfo->DesignerSelection().erase(si);
		else
			si++;
	}

	// Add selection in 'partIds' that is not already there.
	for(QSet<quint16>::const_iterator pi = partIds.m_Ids.cbegin(); pi != partIds.m_Ids.cend(); pi++)
		if(!m_RenderInfo->DesignerSelection().contains(*pi))
			m_RenderInfo->DesignerSelection().insert(*pi, AuScSelectInfo());

	// Face mode requires the anchor part to be in the selection. It's handy for part mode too.
	if(partIds.m_Ids.empty())
		m_RenderInfo->SetAnchorPart(0);
	else if(!m_RenderInfo->AnchorPartSelected())
		m_RenderInfo->SetAnchorPart(*partIds.m_Ids.begin());

	// Notify of possible changes for control enable/disable by the frame.
	emit DesignerSelectionChanged();

	QString anchorDesc(m_AnchorDesc);
	if(m_RenderInfo->AnchorPart())
	{
		DaPerson* sceneAtom;
		LockSceneAtom(&sceneAtom);
		if(sceneAtom)
		{
			const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
			if(pde)
				anchorDesc = pde->PartSelectedDesc(m_RenderInfo->AnchorPart());
		}
		UnlockSceneAtom();
	}
	else
		anchorDesc = "";

	if(anchorDesc != m_AnchorDesc)
	{
		m_AnchorDesc = anchorDesc;
		emit DesignerAnchorChanged(m_AnchorDesc);
	}

	// Face mode is not permitted if any of the selected parts is sealed and the current user does not have access.
	if(DesignerObjectMode() && HasSealedPartsSelected())
		auDesignerApp->DesignerFrame()->DesignerSetObjectMode(false);
}

void AuDesignerView::DesignerBerthCapt()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScBerth(AuScBerthCapt::TypeStatic(), this));
}

void AuDesignerView::DesignerBerthCrew()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScBerth(AuScBerthCrew::TypeStatic(), this));
}

void AuDesignerView::DesignerBerthOfficer()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScBerth(AuScBerthOfficer::TypeStatic(), this));
}

void AuDesignerView::DesignerBerthPassenger()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScBerth(AuScBerthPassenger::TypeStatic(), this));
}

void AuDesignerView::DesignerBerthTroop()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScBerth(AuScBerthTroop::TypeStatic(), this));
}

void AuDesignerView::DesignerConsoleCapt()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScConsole(AuScConsoleCommand::TypeStatic(), this));
}

void AuDesignerView::DesignerConsoleEngineer()
{
	if(!IsInDesignEnvironment())
		return;

	bool isStarship = false;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			const AuScDesign* d = dynamic_cast<const AuScDesign*>(pde->LockDesignForRead());
			if(d)
				isStarship = d->Structure() == SC_Starship;
			pde->UnlockDesign();
		}
	}
	UnlockSceneAtom();

	StartCommand(new DdScConsoleEngineer(isStarship, this));
}

void AuDesignerView::DesignerConsoleFireControl()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScConsoleFireControl(this));
}

void AuDesignerView::DesignerConsoleHelm()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScConsole(AuScConsoleHelm::TypeStatic(), this));
}

void AuDesignerView::DesignerConsoleMedic()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScConsoleMedic(this));
}

void AuDesignerView::DesignerConsoleNavigator()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScConsole(AuScConsoleNavigator::TypeStatic(), this));
}

void AuDesignerView::DesignerConsolePowerRelay()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScConsole(AuScConsolePowerRelay::TypeStatic(), this));
}

void AuDesignerView::DesignerConsoleSensor()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScConsole(AuScConsoleSensor::TypeStatic(), this));
}

void AuDesignerView::DesignerConsoleShield()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScConsole(AuScConsoleShield::TypeStatic(), this));
}

void AuDesignerView::DesignerConsoleTransporter()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScConsoleTransporter(this));
}

void AuDesignerView::DesignerDoorProperties()
{
	if(!IsInNewDesignEnvironment())
		return;
	StartCommand(new DdScDoorAccess(this));
}

void AuDesignerView::DesignerSiteProperties()
{
	if(!IsInNewDesignEnvironment())
		return;
	StartCommand(new DdSiteProperties(this));
}

void AuDesignerView::DesignerCopy()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	if(DesignerObjectMode())
	{
		if(!m_RenderInfo->DesignerHasPartObjectSelection())
			return;
	}
	else
	{
		if(!m_RenderInfo->DesignerHasPartSelection())
			return;
	}

	// Create a custom mime data object of the selected elements.
	QMimeData* mymime = 0;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
			mymime = pde->CreateMIME(sceneAtom->Id(), DesignerObjectMode(), m_RenderInfo->DesignerPartObjectSelection(),
					m_DesignPlane);
	}
	UnlockSceneAtom();
	if(!mymime)
		return;

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setMimeData(mymime);
}

void AuDesignerView::DesignerCut()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	if(DesignerObjectMode())
	{
		if(!m_RenderInfo->DesignerHasPartObjectSelection())
			return;
	}
	else
	{
		if(!m_RenderInfo->DesignerHasPartSelection())
			return;
	}

	DesignerCopy();					// Copy the selected elements to the clipboard.
	DesignerDelete();				// Delete the selected elements.
}

void AuDesignerView::DesignerCutFace()
{
	if(!IsInDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;
	StartCommand(new DdScCutFace(this));
}

void AuDesignerView::DesignerCuttingJig()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScCuttingJig(this));
}

void AuDesignerView::DesignerDecalJig()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScDecalJig(this));
}

void AuDesignerView::DesignerDelete()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	if(DesignerObjectMode())
	{
		auApp->DesignerDeletePartObjects(m_RenderInfo->DesignerPartObjectSelection());
		m_RenderInfo->DesignerClearPartObjectSelection();
	}
	else
	{
		auApp->DesignerDeleteParts(m_RenderInfo->DesignerPartSelection());
		m_RenderInfo->DesignerClearPartSelection();
	}
	emit DesignerSelectionChanged();
}

void AuDesignerView::DesignerDepart()
{
	// Try not to leave any commands running.
	EndCommands();
	if(!IsInDesignEnvironment())
		return;

	// Warn against losing the current design. Provide an option to cancel.
	if(!DesignerConfirmLoseChanges())
		return;

	m_DesignerFileName = "";

	// Restore title on main frame window so it does not mention our design file name.
	auApp->Frame()->setWindowTitle(g_Au.ApplicationName());

	auApp->DesignExit();
}

void AuDesignerView::DesignerDivideFaces()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;
	StartCommand(new DdScDivide(this));
}

void AuDesignerView::DesignerDoorJig()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScDoorJig(this));
}

void AuDesignerView::DesignerEmitterContrail()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScContrailEmitter(this));
}

void AuDesignerView::DesignerExit()
{
	// AuFrame::closeEvent warns against losing the current design and provides an option to cancel.
	auApp->closeAllWindows();
}

void AuDesignerView::DesignerFlatten()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;

	auApp->DesignerFlatten(m_RenderInfo->DesignerPartObjectSelection());
}

void AuDesignerView::DesignerFractureFaces()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;

	auApp->DesignerFractureFaces(m_RenderInfo->DesignerPartObjectSelection());
}

void AuDesignerView::DesignerFuseFaces()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;

	auApp->DesignerFuseFaces(m_RenderInfo->DesignerPartObjectSelection());
}

void AuDesignerView::DesignerGameTable()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScGameTable(this));
}

void AuDesignerView::DesignerGrid3pt()
{
	if(!IsInNewDesignEnvironment())
		return;
	StartCommand(new DdGrid3pt(this));
}

void AuDesignerView::DesignerGridMove()
{
	if(!IsInNewDesignEnvironment())
		return;
	StartCommand(new DdGridMove(this));
}

void AuDesignerView::DesignerGridXY()
{
	if(!IsInNewDesignEnvironment())
		return;

	const AuPoint zvec(0,0,1);

	AuPoint xvec(m_DesignPlane.ProjectVector(1,0,0));
	if(xvec.IsParallel(zvec))
		xvec = m_DesignPlane.ProjectVector(0,1,0);
	xvec.Unit();

	AuPoint yvec = zvec.Cross(xvec);
	yvec.Unit();

	xvec = yvec.Cross(zvec);
	xvec.Unit();

	AuMatrix4x4 newPlane;
	newPlane.Translate(m_DesignPlane.Translation());
	newPlane.RotateToPlane(xvec, yvec);
	SetDesignPlane(newPlane);
}

void AuDesignerView::DesignerGridXZ()
{
	if(!IsInNewDesignEnvironment())
		return;

	const AuPoint yvec(0,0,1);

	AuPoint xvec(m_DesignPlane.ProjectVector(1,0,0));
	if(xvec.IsParallel(yvec))
		xvec = m_DesignPlane.ProjectVector(0,1,0);
	xvec.Unit();

	AuPoint zvec = xvec.Cross(yvec);
	zvec.Unit();

	xvec = yvec.Cross(zvec);
	xvec.Unit();

	AuMatrix4x4 newPlane;
	newPlane.Translate(m_DesignPlane.Translation());
	newPlane.RotateToPlane(xvec, yvec);
	SetDesignPlane(newPlane);
}

void AuDesignerView::DesignerGridYZ()
{
	if(!IsInNewDesignEnvironment())
		return;

	const AuPoint xvec(0,0,1);

	AuPoint yvec(m_DesignPlane.ProjectVector(0,1,0));
	if(yvec.IsParallel(xvec))
		yvec = m_DesignPlane.ProjectVector(1,0,0);
	yvec.Unit();

	AuPoint zvec = xvec.Cross(yvec);
	zvec.Unit();

	yvec = zvec.Cross(xvec);
	yvec.Unit();

	AuMatrix4x4 newPlane;
	newPlane.Translate(m_DesignPlane.Translation());
	newPlane.RotateToPlane(xvec, yvec);
	SetDesignPlane(newPlane);
}

void AuDesignerView::DesignerRemoteCamera()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScRemoteCamera(this));
}

void AuDesignerView::DesignerLightBulb()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScLightBulb(this));
}

void AuDesignerView::DesignerHullExtrusion()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScExtrusion(AuScHull::TypeStatic(), this));
}

void AuDesignerView::DesignerHullSphereEqDivs()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScSphereUniform(AuScHull::TypeStatic(), this));
}

void AuDesignerView::DesignerHullSphereLatLong()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScSphereLatLong(AuScHull::TypeStatic(), this));
}

void AuDesignerView::DesignerHullSpindle()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScSpindle(AuScHull::TypeStatic(), this));
}

void AuDesignerView::DesignerHullTubing()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScTubing(AuScHull::TypeStatic(), this));
}

void AuDesignerView::DesignerImport3DS()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScImport3ds(this));
}

void AuDesignerView::DesignerImportSTL()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScImportSTL(this));
}

void AuDesignerView::DesignerExportSTL()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScExportSTL(this));
}

void AuDesignerView::DesignerInteriorExtrusion()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScExtrusion(AuScMesh::TypeStatic(), this));
}

void AuDesignerView::DesignerInteriorSphereEqDivs()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScSphereUniform(AuScMesh::TypeStatic(), this));
}

void AuDesignerView::DesignerInteriorSphereLatLong()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScSphereLatLong(AuScMesh::TypeStatic(), this));
}

void AuDesignerView::DesignerInteriorSpindle()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScSpindle(AuScMesh::TypeStatic(), this));
}

void AuDesignerView::DesignerInteriorTubing()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScTubing(AuScMesh::TypeStatic(), this));
}

void AuDesignerView::DesignerJigCut()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	if(DesignerObjectMode())
		return;
	if(m_RenderInfo->DesignerPartSelection().empty())
		return;

	StartCommand(new DdScJigCut(this));

	m_RenderInfo->DesignerClearPartSelection();
	emit DesignerSelectionChanged();
}

void AuDesignerView::DesignerJoinParts()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	if(DesignerObjectMode())
		return;
	if(!m_RenderInfo->AnchorPart())
		return;
	if(m_RenderInfo->DesignerPartSelection().size() < 2)
		return;

	auApp->DesignerJoinParts(m_RenderInfo->AnchorPart(), m_RenderInfo->DesignerPartSelection());
	m_RenderInfo->DesignerClearPartSelection();
	emit DesignerSelectionChanged();
}

void AuDesignerView::DesignerLandingGearExtrusion()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScExtrusion(AuScLandingGear::TypeStatic(), this));
}

void AuDesignerView::DesignerLandingGearSphereEqDivs()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScSphereUniform(AuScLandingGear::TypeStatic(), this));
}

void AuDesignerView::DesignerLandingGearSphereLatLong()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScSphereLatLong(AuScLandingGear::TypeStatic(), this));
}

void AuDesignerView::DesignerLandingGearSpindle()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScSpindle(AuScLandingGear::TypeStatic(), this));
}

void AuDesignerView::DesignerLandingGearTubing()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScTubing(AuScLandingGear::TypeStatic(), this));
}

void AuDesignerView::DesignerLandingSystem()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScLandingSystem(this));
}

void AuDesignerView::DesignerLaunchTube()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScLaunchTube(this));
}

void AuDesignerView::DesignerLaunchTubeJig()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScLaunchTubeJig(this));
}

void AuDesignerView::DesignerLoopCut()
{
	if(!IsInDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;
	StartCommand(new DdScLoopCut(this));
}

void AuDesignerView::DesignerMakeDecal()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScMakeDecal(this));
}

void AuDesignerView::DesignerMakeDoor()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScMakeDoor(this));
}

void AuDesignerView::DesignerMakeLightLens()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScMakeLightLens(this));
}

void AuDesignerView::DesignerMakeHull()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScMakeHull(this));
}

void AuDesignerView::DesignerMakeLandingGear()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScMakeLandingGear(this));
}

void AuDesignerView::DesignerMakeMesh()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScMakeMesh(this));
}

void AuDesignerView::DesignerMakeNavLightLens()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScMakeNavLightLens(this));
}

void AuDesignerView::DesignerMakeRoom()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScMakeRoom(this));
}

void AuDesignerView::DesignerMakeRotatingBeaconLens()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScMakeRotatingBeaconLens(this));
}

void AuDesignerView::DesignerMakeStrobeLens()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScMakeStrobeLens(this));
}

void AuDesignerView::DesignerMakeTurboLift()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScMakeTurboLift(this));
}

void AuDesignerView::DesignerMakeTurret()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScMakeTurret(this));
}

void AuDesignerView::DesignerMakeWindow()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScMakeWindow(this));
}

void AuDesignerView::DesignerMove()
{
	if(!IsInNewDesignEnvironment())
		return;
	AuPoint dragRef = FromPixelToDesignPlane(m_MouseLast, false, 0);
	DesignerMove(dragRef);
}

void AuDesignerView::DesignerMoveMenu()
{
	if(!IsInNewDesignEnvironment())
		return;
	DesignerMove(DesignPlane().Translation());
}

void AuDesignerView::DesignerNewSpacecraft()
{
	// Warn against losing the current design. Provide an option to cancel.
	if(!DesignerConfirmLoseChanges())
		return;

	// Start with no commands running.
	EndCommands();

	// Start with the same structure as last time.
	SPACECRAFT_STRUCTURE newStructure = (SPACECRAFT_STRUCTURE)g_Au.Settings().readNumEntry("/Designer/NewScStructure", SC_Starship);
	if(newStructure != SC_Starship &&
			newStructure != SC_Spaceship &&
			newStructure != SC_Station)
		newStructure = SC_Starship;

	AuScDesign designRec;
	designRec.SetStructure(newStructure);

	m_DesignerFileName = "";
	auApp->DesignerNew(designRec);

	// Update title on main frame window to show design file unsaved status.
	QString windowTitle = QString("%1 %2 New Spacecraft")
		.arg(g_Au.ApplicationName())
		.arg(g_Au.MiddleDotSymbol());
	auApp->Frame()->setWindowTitle(windowTitle);

	// Trigger enable/disable of menu controls.
	m_RenderInfo->DesignerClearPartSelection();
	auDesignerApp->DesignerFrame()->DesignerSetObjectMode(false);
	emit DesignerSelectionChanged();
}

void AuDesignerView::DesignerNewBuilding()
{
	// Warn against losing the current design. Provide an option to cancel.
	if(!DesignerConfirmLoseChanges())
		return;

	// Start with no commands running.
	EndCommands();

	// Start with the same structure as last time.
	BUILDING_STRUCTURE newStructure = (BUILDING_STRUCTURE)g_Au.Settings().readNumEntry("/Designer/NewBdStructure", BD_Building);
	if(newStructure <= BD_Invalid || newStructure >= BD_Count)
		newStructure = BD_Building;

	AuBdDesign designRec;
	designRec.SetStructure(newStructure);

	m_DesignerFileName = "";
	auApp->DesignerNew(designRec);

	// Update title on main frame window to show design file unsaved status.
	QString windowTitle = QString("%1 %2 New Building")
		.arg(g_Au.ApplicationName())
		.arg(g_Au.MiddleDotSymbol());
	auApp->Frame()->setWindowTitle(windowTitle);

	// Trigger enable/disable of menu controls.
	m_RenderInfo->DesignerClearPartSelection();
	auDesignerApp->DesignerFrame()->DesignerSetObjectMode(false);
	emit DesignerSelectionChanged();
}

void AuDesignerView::DesignerNewVehicle()
{
	//DHH Designer
}

void AuDesignerView::DesignerNewItem()
{
	// Warn against losing the current design. Provide an option to cancel.
	if(!DesignerConfirmLoseChanges())
		return;

	// Start with no commands running.
	EndCommands();

	// Start with the same structure as last time.
	COMMODITYID newCommodityId = (COMMODITYID)g_Au.Settings().readNumEntry("/Designer/NewItCommodity", CID_Invalid);
	if(newCommodityId < CID_Invalid || newCommodityId >= CID_Count)
		newCommodityId = CID_Invalid;

	AuItDesign designRec;
	designRec.SetCommodityId(newCommodityId);

	m_DesignerFileName = "";
	auApp->DesignerNew(designRec);

	// Update title on main frame window to show design file unsaved status.
	QString windowTitle = QString("%1 %2 New Item")
		.arg(g_Au.ApplicationName())
		.arg(g_Au.MiddleDotSymbol());
	auApp->Frame()->setWindowTitle(windowTitle);

	// Trigger enable/disable of menu controls.
	m_RenderInfo->DesignerClearPartSelection();
	auDesignerApp->DesignerFrame()->DesignerSetObjectMode(false);
	emit DesignerSelectionChanged();
}

void AuDesignerView::DesignerObjectExtrusion()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScExtrusion(AuScMesh::TypeStatic(), this));
}

void AuDesignerView::DesignerObjectFace()
{
	if(!IsInDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;
	StartCommand(new DdScFace(this));
}

void AuDesignerView::DesignerObjectQuad()
{
	if(!IsInDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;
	StartCommand(new DdScQuad(this));
}

void AuDesignerView::DesignerObjectSphereEqDivs()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScSphereUniform(AuScMesh::TypeStatic(), this));
}

void AuDesignerView::DesignerObjectSphereLatLong()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScSphereLatLong(AuScMesh::TypeStatic(), this));
}

void AuDesignerView::DesignerObjectSpindle()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScSpindle(AuScMesh::TypeStatic(), this));
}

void AuDesignerView::DesignerObjectTriangle()
{
	if(!IsInDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;
	StartCommand(new DdScTriangle(this));
}

void AuDesignerView::DesignerObjectTubing()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScTubing(AuScMesh::TypeStatic(), this));
}

void AuDesignerView::DesignerOpen()
{
	// Warn against losing the current design. Provide an option to cancel.
	if(!DesignerConfirmLoseChanges())
		return;

	// Get file name and open the file.
	QString designFileName(m_DesignerFileName);
	if(designFileName.isEmpty())
	{
		designFileName = g_Au.Settings().readEntry("/Designer/LastSoHFile", "");
		if(designFileName.isEmpty())
			designFileName = QDir::currentPath();
		else
		{
			QFileInfo lastFileInfo(designFileName);
			if(!lastFileInfo.exists())
				designFileName = lastFileInfo.absolutePath();	// Use last known file folder.
		}
	}

	QString loadFile = QFileDialog::getOpenFileName(this, tr("Hazeron Design File"), designFileName,
			"Hazeron Designs (*.SoH);;All Files (*.*)", 0,
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(!loadFile.isEmpty() && loadFile.right(4).compare(".SoH", Qt::CaseInsensitive) == 0)
	{
		// Store last known .SoH file folder.
		g_Au.Settings().writeEntry("/Designer/LastSoHFile", loadFile);

		AuDesign* design = AuDesign::LoadFromFile(loadFile, SOH_FILE_TYPE);
		if(design)
		{
			// Start with no commands running.
			EndCommands();

			bool sizeOk = auApp->DesignerLoadedFromFile(*design);
			delete design;
			if(!sizeOk)
			{
				QMessageBox mb(g_Au.ApplicationName(), tr("The blueprint data is too large. "
							"The file cannot be sent to the server. "),
						QMessageBox::Warning, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton,
						auApp->Frame());
				auApp->Frame()->ExecMessageBox(mb);
			}
			else
			{
				m_DesignerFileName = loadFile;

				// Update title on main frame window to show current design file name.
				QString windowTitle = QString("%1 %2 %3")
					.arg(g_Au.ApplicationName())
					.arg(g_Au.MiddleDotSymbol())
					.arg(m_DesignerFileName);
				auApp->Frame()->setWindowTitle(windowTitle);

				// Trigger enable/disable of menu controls.
				m_RenderInfo->DesignerClearPartSelection();
				auDesignerApp->DesignerFrame()->DesignerSetObjectMode(false);
				emit DesignerSelectionChanged();
			}
		}
	}
}

void AuDesignerView::DesignerPanelFTL()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;

	StartCommand(new DdScScreen(AuScPanelFTLDrive::TypeStatic(), this));
}

void AuDesignerView::DesignerPanelLifeSupport()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;

	StartCommand(new DdScScreen(AuScPanelLifeSupport::TypeStatic(), this));
}

void AuDesignerView::DesignerPanelManeuver()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;

	StartCommand(new DdScScreen(AuScPanelManeuverDrive::TypeStatic(), this));
}

void AuDesignerView::DesignerPanelCapacitor()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;

	StartCommand(new DdScScreen(AuScPanelCapacitor::TypeStatic(), this));
}

void AuDesignerView::DesignerPanelPowerPlant()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;

	StartCommand(new DdScScreen(AuScPanelPowerPlant::TypeStatic(), this));
}

void AuDesignerView::DesignerPanelSensor()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;

	StartCommand(new DdScScreen(AuScPanelSensor::TypeStatic(), this));
}

void AuDesignerView::DesignerPanelShield()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;

	StartCommand(new DdScScreen(AuScPanelShield::TypeStatic(), this));
}

void AuDesignerView::DesignerPanelWeapon()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;

	StartCommand(new DdScScreen(AuScPanelWeapon::TypeStatic(), this));
}

void AuDesignerView::DesignerParkSpacecraft()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScParkSpacecraft(this));
}

void AuDesignerView::DesignerParkGroundVehicle()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScParkGroundVehicle(this));
}

void AuDesignerView::DesignerParkRocket()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScParkRocket(this));
}

void AuDesignerView::DesignerParkSpaceVehicle()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScParkSpaceVehicle(this));
}

void AuDesignerView::DesignerParkWaterVehicle()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScParkWaterVehicle(this));
}

void AuDesignerView::DesignerPaste()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	QClipboard* clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	if(!mimeData)
		return;

	AuScParts pastedParts;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
			pde->ReadMIME(sceneAtom->Id(), sceneAtom->Name(), DesignerObjectMode(), *mimeData, pastedParts);
	}
	UnlockSceneAtom();

	if(!pastedParts.empty())
	{
		// Transform the pasted parts to the position of the design plane.
		pastedParts.Transform(m_DesignPlane);

		// Add the parts to the designer.
		if(DesignerObjectMode())
			auApp->DesignerAddPartObjects(DesignerObjectModeTargetPart(), pastedParts);
		else
			auApp->DesignerAddParts(pastedParts);

		// If sealed parts were added, we can't remain in face mode.
		if(DesignerObjectMode() && HasSealedPartsSelected())
			auDesignerApp->DesignerFrame()->DesignerSetObjectMode(false);
	}
	/*
	else if(mimeData->hasImage())
	{
		//DHH Designer paste image
		QPixmap pm = qvariant_cast<QPixmap>(mimeData->imageData());
	}
	*/
}

void AuDesignerView::DesignerPasteAt()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	// Check for valid mime data to paste.
	bool mimeOk = false;
	QClipboard* clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	if(mimeData)
	{
		DaPerson* sceneAtom;
		LockSceneAtom(&sceneAtom);
		if(sceneAtom)
		{
			const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
			if(pde)
				mimeOk = pde->CheckMIME(DesignerObjectMode(), *mimeData);
		}
		UnlockSceneAtom();
	}
	if(!mimeOk)
		return;

	StartCommand(new DdScPasteAt(this));
}

void AuDesignerView::DesignerPasteRadial()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	// Check for valid mime data to paste.
	bool mimeOk = false;
	QClipboard* clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	if(mimeData)
	{
		DaPerson* sceneAtom;
		LockSceneAtom(&sceneAtom);
		if(sceneAtom)
		{
			const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
			if(pde)
				mimeOk = pde->CheckMIME(DesignerObjectMode(), *mimeData);
		}
		UnlockSceneAtom();
	}
	if(!mimeOk)
		return;

	StartCommand(new DdScPasteRadial(this));
}

void AuDesignerView::DesignerPodBay()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScPodBay(this));
}

void AuDesignerView::DesignerProperties()
{
	switch(m_DesignMode)
	{
	case DaDesignBase::InvalidMode:
	case DaDesignBase::DESIGN_Unknown:
		return;

	case DaDesignBase::DESIGN_Spacecraft:
		{
			AuScDesignProps* mods = 0;

			// Get current values from design.
			DaPerson* person;
			LockSceneAtom(&person);
			if(person)
			{
				DaDesigner* env = person->FindParentDesignerInEnv();
				if(env)
				{
					const AuScDesign* designRec = dynamic_cast<const AuScDesign*>(env->LockDesignForRead());
					if(designRec)
						mods = new AuScDesignProps(*designRec, this);
					env->UnlockDesign();
				}
			}
			UnlockSceneAtom();

			// Show the dialog if we got the current values.
			if(mods)
			{
				if(mods->exec() == QDialog::Accepted)
				{
					auApp->DesignerSetProperties(mods->m_DesignRec);

					g_Au.Settings().writeEntry("/Designer/NewScStructure", mods->m_DesignRec.Structure());
				}

				mods->deleteLater();
			}
		}
		break;

	case DaDesignBase::DESIGN_Building:
		{
			AuBdDesignProps* mods = 0;

			// Get current values from design.
			DaPerson* person;
			LockSceneAtom(&person);
			if(person)
			{
				DaDesigner* env = person->FindParentDesignerInEnv();
				if(env)
				{
					const AuBdDesign* designRec = dynamic_cast<const AuBdDesign*>(env->LockDesignForRead());
					if(designRec)
						mods = new AuBdDesignProps(*designRec, this);
					env->UnlockDesign();
				}
			}
			UnlockSceneAtom();

			// Show the dialog if we got the current values.
			if(mods)
			{
				if(mods->exec() == QDialog::Accepted)
				{
					auApp->DesignerSetProperties(mods->m_DesignRec);

					g_Au.Settings().writeEntry("/Designer/NewBdStructure", mods->m_DesignRec.Structure());
				}

				mods->deleteLater();
			}
		}
		break;

	case DaDesignBase::DESIGN_Item:
		{
			AuItDesignProps* mods = 0;

			// Get current values from design.
			DaPerson* person;
			LockSceneAtom(&person);
			if(person)
			{
				DaDesigner* env = person->FindParentDesignerInEnv();
				if(env)
				{
					const AuItDesign* designRec = dynamic_cast<const AuItDesign*>(env->LockDesignForRead());
					if(designRec)
						mods = new AuItDesignProps(*designRec, this);
					env->UnlockDesign();
				}
			}
			UnlockSceneAtom();

			// Show the dialog if we got the current values.
			if(mods)
			{
				if(mods->exec() == QDialog::Accepted)
				{
					auApp->DesignerSetProperties(mods->m_DesignRec);

					g_Au.Settings().writeEntry("/Designer/NewItCommodity", mods->m_DesignRec.CommodityId());
				}

				mods->deleteLater();
			}
		}
		break;

	//case DaDesignBase::DESIGN_Vehicle:
		//break;
	}
}

void AuDesignerView::DesignerPunchFace()
{
	if(!IsInDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;
	StartCommand(new DdScPunchFace(this));
}

void AuDesignerView::DesignerPushPull()
{
	if(!IsInNewDesignEnvironment())
		return;

	DesignerPushPull(DesignPlane().Translation());
}

void AuDesignerView::DesignerPushPull(const AuPoint& ref)
{
	if(!IsInNewDesignEnvironment())
		return;
	StartCommand(new DdScPushPull(ref, this));
}

void AuDesignerView::DesignerRedo()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	auApp->DesignerRedo();
}

void AuDesignerView::DesignerReverseFaces()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;

	auApp->DesignerReverseFaces(m_RenderInfo->DesignerPartObjectSelection());
}

void AuDesignerView::DesignerRoomExtrusion()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScExtrusion(AuScRoom::TypeStatic(), this));
}

void AuDesignerView::DesignerRoomSphereEqDivs()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScSphereUniform(AuScRoom::TypeStatic(), this));
}

void AuDesignerView::DesignerRoomSphereLatLong()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScSphereLatLong(AuScRoom::TypeStatic(), this));
}

void AuDesignerView::DesignerRoomSpindle()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScSpindle(AuScRoom::TypeStatic(), this));
}

void AuDesignerView::DesignerRoomTubing()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScTubing(AuScRoom::TypeStatic(), this));
}

void AuDesignerView::DesignerRotate()
{
	if(!IsInNewDesignEnvironment())
		return;

	DesignerRotate(DesignPlane().Translation());
}

void AuDesignerView::DesignerSave()
{
	bool savedIt = false;

	// Attempt to resave the file using it's last known file name.
	if(!m_DesignerFileName.isEmpty())
	{
		// Grab a thumbnail image of the screen, to accompany the design.
		QImage thumb = grabFramebuffer().convertToFormat(QImage::Format_RGB16).scaled(256,256, Qt::KeepAspectRatio,
				Qt::SmoothTransformation);
		DaPerson* sceneAtom;
		LockSceneAtom(&sceneAtom);
		if(sceneAtom)
		{
			DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
			if(pde)
				savedIt = pde->SaveToFile(sceneAtom->Id(), m_DesignerFileName, &thumb);
		}
		UnlockSceneAtom();
	}

	if(savedIt)
	{
		// Update modified state in root data in single player mode.
		if(auApp->IsSingleUserDesignerMode())
		{
			AuEther& ether = g_Au.LockEtheriumDelete();
			DaPerson* person = dynamic_cast<DaPerson*>(ether.FindAtom(SceneAtomId()));
			if(person)
			{
				DaDesigner* pde = person->FindParentDesignerInEnv();
				if(pde)
					pde->SetDesignModified(false);
			}
			g_Au.UnlockEtheriumDelete();
		}
	}
	else
	{
		// If resave using last known name didn't work, call SaveAs instead.
		if(!savedIt)
			DesignerSaveAs();
	}
}

void AuDesignerView::DesignerSaveAs()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	// Get file name and save the file.
	QString designFileName(m_DesignerFileName);
	if(designFileName.isEmpty())
	{
		QString lastFile = g_Au.Settings().readEntry("/Designer/LastSoHFile", "");
		if(lastFile.isEmpty())
			lastFile = QDir::currentPath();
		QFileInfo lastFileInfo(lastFile);

		designFileName = lastFileInfo.absolutePath();	// Use last known ship file folder.
	}

	QString suffix(".SoH");
	QString filter = QString("Hazeron Designs (*%1);;All Files (*.*)").arg(suffix);

	QString saveFile = QFileDialog::getSaveFileName(this, tr("Hazeron Design File"), designFileName, filter, 0,
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(saveFile.isEmpty())
		return;

	// Insure the file has an appropriate suffix.
	if(saveFile.right(suffix.length()) != suffix)
		saveFile += suffix;

	// Store last known .SoH file folder.
	g_Au.Settings().writeEntry("/Designer/LastSoHFile", saveFile);

	// Grab a thumbnail image of the screen, to accompany the design.
	QImage thumb = grabFramebuffer().convertToFormat(QImage::Format_RGB16).scaled(256,256, Qt::KeepAspectRatio,
			Qt::SmoothTransformation);

	// Tell designer to save to a file.
	bool savedIt = false;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			savedIt = pde->SaveToFile(sceneAtom->Id(), saveFile, &thumb);
			m_DesignerFileName = saveFile;

			// Update title on main frame window to show current design file name.
			QString windowTitle = QString("%1 %2 %3")
				.arg(g_Au.ApplicationName())
				.arg(g_Au.MiddleDotSymbol())
				.arg(m_DesignerFileName);
			auApp->Frame()->setWindowTitle(windowTitle);
		}
	}
	UnlockSceneAtom();

	// Update modified state in root data in single player mode.
	if(savedIt && auApp->IsSingleUserDesignerMode())
	{
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaPerson* person = dynamic_cast<DaPerson*>(ether.FindAtom(SceneAtomId()));
		if(person)
		{
			DaDesigner* pde = person->FindParentDesignerInEnv();
			if(pde)
				pde->SetDesignModified(false);
		}
		g_Au.UnlockEtheriumDelete();
	}
}

void AuDesignerView::DesignerSelectAll()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			if(DesignerObjectMode())
			{
				pde->PartSelectionVertexIds(m_RenderInfo->DesignerSelection());	// Get vertex ids of the selected parts.
				pde->PartSelectionEdgeIds(m_RenderInfo->DesignerSelection());	// Get edge ids of the selected parts.
				pde->PartSelectionFaceIds(m_RenderInfo->DesignerSelection());	// Get face ids of the selected parts.
			}
			else
				pde->PartSelectionIds(m_RenderInfo->DesignerSelection());	// Get ids of all parts.
			emit DesignerSelectionChanged();
		}
	}
	UnlockSceneAtom();
}

void AuDesignerView::DesignerSelectContiguous()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;

	// Get selection options.
	float angleTol = g_AuClient.DesignerSelectionAngleTolerance();
	AuSelContiguousDlg dlg(angleTol, this);
	if(auApp->Frame()->ExecDialog(dlg) != QDialog::Accepted)
		return;
	angleTol = dlg.AngleTolerance();
	g_AuClient.DesignerSetSelectionAngleTolerance(angleTol);
	angleTol *= DEG2RADf;

	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde && pde->SelectContiguous(angleTol, m_RenderInfo->DesignerSelection()))
			emit DesignerSelectionChanged();
	}
	UnlockSceneAtom();
}

void AuDesignerView::DesignerSelectEdges()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			if(DesignerObjectMode())
				pde->PartSelectionEdgeIds(m_RenderInfo->DesignerSelection());	// Get edge ids of the selected parts.
			else
				pde->PartSelectionIds(m_RenderInfo->DesignerSelection());	// Get ids of all parts.
			emit DesignerSelectionChanged();
		}
	}
	UnlockSceneAtom();
}

void AuDesignerView::DesignerSelectedProperties()
{
	//DHH Designer
}

void AuDesignerView::DesignerSelectFaces()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			if(DesignerObjectMode())
				pde->PartSelectionFaceIds(m_RenderInfo->DesignerSelection());	// Get face ids of the selected parts.
			else
				pde->PartSelectionIds(m_RenderInfo->DesignerSelection());	// Get ids of all parts.
			emit DesignerSelectionChanged();
		}
	}
	UnlockSceneAtom();
}

void AuDesignerView::DesignerSelectInvert()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			if(DesignerObjectMode())
			{
				pde->InvertPartObjectSelection(m_RenderInfo->DesignerSelection());
			}
			else
			{
				AuScSelection allPartIds;
				pde->PartSelectionIds(allPartIds);
				for(AuScSelection::const_iterator api = allPartIds.constBegin(); api != allPartIds.constEnd(); api++)
					if(m_RenderInfo->DesignerSelection().contains(api.key()))
						m_RenderInfo->DesignerSelection().remove(api.key());
					else
						m_RenderInfo->DesignerSelection().insert(api.key(), *api);
			}
			emit DesignerSelectionChanged();
		}
	}
	UnlockSceneAtom();
}

void AuDesignerView::DesignerSelectNone()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	if(DesignerObjectMode())
		m_RenderInfo->DesignerClearPartObjectSelection();	// vertices, edges and faces
	else
		m_RenderInfo->DesignerClearPartSelection();
	emit DesignerSelectionChanged();
}

void AuDesignerView::DesignerSelectVertices()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			if(DesignerObjectMode())
				pde->PartSelectionVertexIds(m_RenderInfo->DesignerSelection());	// Get vertex ids of the selected parts.
			else
				pde->PartSelectionIds(m_RenderInfo->DesignerSelection());	// Get ids of all parts.
			emit DesignerSelectionChanged();
		}
	}
	UnlockSceneAtom();
}

void AuDesignerView::DesignerDeselectVertices()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;
	if(!DesignerObjectMode())
		return;

	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			m_RenderInfo->DesignerSelection().ClearVertices();
			emit DesignerSelectionChanged();
		}
	}
	UnlockSceneAtom();
}

void AuDesignerView::DesignerSketch()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;

	StartCommand(new DdScSketch(this));
}

void AuDesignerView::DesignerSmooth()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;

	auApp->DesignerSmooth(m_RenderInfo->DesignerPartObjectSelection());
}

void AuDesignerView::DesignerSplitFaces()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;

	auApp->DesignerSplitFaces(m_RenderInfo->DesignerPartObjectSelection());
}

void AuDesignerView::DesignerStretch()
{
	if(!IsInNewDesignEnvironment())
		return;

	// Calculate basis for stretch. Scale 1 is current cursor distance from grid origin.
	AuPoint dragRef = FromPixelToDesignPlane(m_MouseLast, true, 0);
	AuPoint startVec = m_DesignPlane.UnProject(dragRef);
	double stretchBase = startVec.Length();
	if(IsFuzzyZero(stretchBase))
		stretchBase = 1;

	DesignerStretch(DesignPlane().Translation(), stretchBase);
}

void AuDesignerView::DesignerStretchMenu()
{
	if(!IsInNewDesignEnvironment())
		return;

	// Calculate basis for stretch. Scale 1 is the bright grid lines, which are every 10 grid increments.
	double gridSpacing = g_AuClient.DesignerGridSpacingFt();
	double stretchBase = gridSpacing * 10;

	DesignerStretch(DesignPlane().Translation(), stretchBase);
}

void AuDesignerView::DesignerStationCapt()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScStation(AuScStationCommand::TypeStatic(), this));
}

void AuDesignerView::DesignerStationDesigner()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScStation(AuScStationDesigner::TypeStatic(), this));
}

void AuDesignerView::DesignerStationEngineer()
{
	if(!IsInDesignEnvironment())
		return;

	bool isStarship = false;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			const AuScDesign* d = dynamic_cast<const AuScDesign*>(pde->LockDesignForRead());
			if(d)
				isStarship = d->Structure() == SC_Starship;
			pde->UnlockDesign();
		}
	}
	UnlockSceneAtom();

	StartCommand(new DdScStationEngineer(isStarship, this));
}

void AuDesignerView::DesignerStationFireControl()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScStationFireControl(this));
}

void AuDesignerView::DesignerStationGunner()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScStationGunner(this));
}

void AuDesignerView::DesignerStationHelm()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScStation(AuScStationHelm::TypeStatic(), this));
}

void AuDesignerView::DesignerStationMedic()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScStationMedic(this));
}

void AuDesignerView::DesignerStationNavigator()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScStation(AuScStationNavigator::TypeStatic(), this));
}

void AuDesignerView::DesignerStationPowerRelay()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScStation(AuScStationPowerRelay::TypeStatic(), this));
}

void AuDesignerView::DesignerStationSensor()
{
	if(!IsInDesignEnvironment())
		return;

	StartCommand(new DdScStation(AuScStationSensor::TypeStatic(), this));
}

void AuDesignerView::DesignerStationShield()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScStation(AuScStationShield::TypeStatic(), this));
}

void AuDesignerView::DesignerStationTransporter()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScStationTransporter(this));
}

void AuDesignerView::DesignerStationTroop()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScTroopPost(this));
}

void AuDesignerView::DesignerStatusAlert()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;

	StartCommand(new DdScScreen(AuScStatusAlert::TypeStatic(), this));
}

void AuDesignerView::DesignerStatusLifeSupport()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;

	StartCommand(new DdScScreen(AuScStatusLifeSupport::TypeStatic(), this));
}

void AuDesignerView::DesignerStatusMission()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;

	StartCommand(new DdScScreen(AuScStatusMission::TypeStatic(), this));
}

void AuDesignerView::DesignerStatusWorldMap()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;

	StartCommand(new DdScScreen(AuScStatusWorldMap::TypeStatic(), this));
}

void AuDesignerView::DesignerTurretBase()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScTurretBase(this));
}

void AuDesignerView::DesignerTurretGun()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScTurretGun(this));
}

void AuDesignerView::DesignerUndo()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignMode() == DaDesignBase::DESIGN_Unknown)
		return;

	auApp->DesignerUndo();
}

void AuDesignerView::DesignerEmpireFlag()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScEmpireFlag(this));
}

void AuDesignerView::DesignerEmpireName()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScEmpireName(this));
}

void AuDesignerView::DesignerShipName()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScShipName(this));
}

void AuDesignerView::DesignerBerthName()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScBerthName(this));
}

void AuDesignerView::DesignerCaptName()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScCaptName(this));
}

void AuDesignerView::DesignerChangeDNA()
{
	if(!auApp->IsSingleUserDesignerMode())
		return;

	//DHH animalia .dna files too
	// Load last known file folder.
	QString lastFile = g_Au.Settings().readEntry("/Settings/LastDNAFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();
	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QFileDialog fileDialog(this, tr("Load Avatar File..."), lastFile);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	if(!g_Au.UseNativeFileDialog())
		fileDialog.setOptions(QFileDialog::DontUseNativeDialog);
	QStringList sFilters;
	sFilters.append("Avatar File (*.chr)");
	fileDialog.setNameFilters(sFilters);
	if(fileDialog.exec() != QDialog::Accepted)
		return;
	if(fileDialog.selectedFiles().isEmpty())
		return;

	// Store last known file folder.
	QString sFile = fileDialog.selectedFiles().first();
	g_Au.Settings().writeEntry("/Settings/LastDNAFile", sFile);

	QFile file(sFile);
	if(!file.open(QFile::ReadOnly))
		return;

	QDataStream ds(&file);
	ds.setVersion(QDataStream::Qt_4_1);

	DhDNA_ dna;

	// File Type Byte
	quint8 chrFileType;
	ds >> chrFileType;
	if(chrFileType == CHR_FILE_TYPE)
	{
		// Version Byte
		quint8 chrFileVersion;
		ds >> chrFileVersion;

		// Character Data
		QByteArray baDNA;
		ds >> baDNA;
		dna = DhDNA_(baDNA);
	}
	else
	{
		// Old 'Jacob' format file.
		file.seek(0);		// Restore file pos back to beginning.
		QByteArray baFileData = file.read(sizeof(DhDNA_));
		dna = DhDNA_(baFileData);

		// Update size values for DNA changes that doubled length and thickness values. D.H.H. 12/22/2010
		if(dna.CalfLength*2 > 31)
			dna.CalfLength = 31;
		else
			dna.CalfLength *= 2;

		if(dna.CalfThickness*2 > 31)
			dna.CalfThickness = 31;
		else
			dna.CalfThickness *= 2;

		if(dna.ForeArmLength*2 > 31)
			dna.ForeArmLength = 31;
		else
			dna.ForeArmLength *= 2;

		if(dna.ForeArmThickness*2 > 31)
			dna.ForeArmThickness = 31;
		else
			dna.ForeArmThickness *= 2;

		if(dna.FootLength*2 > 31)
			dna.FootLength = 31;
		else
			dna.FootLength *= 2;

		if(dna.HandLength*2 > 31)
			dna.HandLength = 31;
		else
			dna.HandLength *= 2;

		if(dna.OverallSize*2 > 31)
			dna.OverallSize = 31;
		else
			dna.OverallSize *= 2;

		if(dna.ThighLength*2 > 31)
			dna.ThighLength = 31;
		else
			dna.ThighLength *= 2;

		if(dna.ThighThickness*2 > 31)
			dna.ThighThickness = 31;
		else
			dna.ThighThickness *= 2;

		if(dna.TorsoSegmentLength*2 > 31)
			dna.TorsoSegmentLength = 31;
		else
			dna.TorsoSegmentLength *= 2;

		if(dna.TorsoThickness*2 > 31)
			dna.TorsoThickness = 31;
		else
			dna.TorsoThickness *= 2;

		if(dna.UpperArmLength*2 > 31)
			dna.UpperArmLength = 31;
		else
			dna.UpperArmLength *= 2;

		if(dna.UpperArmThickness*2 > 31)
			dna.UpperArmThickness = 31;
		else
			dna.UpperArmThickness *= 2;
	}
	file.close();

	// We have DNA.
	DhDNA dna3(dna);
	auApp->DesignerSetDNA(&dna3);
}

void AuDesignerView::DesignerWindowJig()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScWindowJig(this));
}

void AuDesignerView::DesignerBoundGround()
{
	double boxRad = GroundSpawnSizeLimit();
	g_AuClient.DesignerSetGridRadius(boxRad);
}

void AuDesignerView::DesignerBoundMax()
{
	double boxRad = AuScDesign::HullSizeLimit();
	g_AuClient.DesignerSetGridRadius(boxRad);
}

void AuDesignerView::DesignerBdHullMax()
{
	double boxRad = AuBdDesign::HullSizeLimitBase(DesignBdStructure());
	g_AuClient.DesignerSetGridRadius(boxRad);
}

void AuDesignerView::DesignerSiteMax()
{
	double boxRad = AuBdDesign::SiteSizeLimitBase(DesignBdStructure());
	g_AuClient.DesignerSetGridRadius(boxRad);
}

void AuDesignerView::DesignerTexturePalette()
{
	if(!IsInDesignEnvironment())
		return;
	StartCommand(new DdScTexturePalette(this));
}

void AuDesignerView::DesignerEffectIntensity()
{
	if(!IsInDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;
	StartCommand(new DdScEffectIntensity(this));
}

void AuDesignerView::DesignerGlow()
{
	if(!IsInDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;
	StartCommand(new DdScGlow(this));
}

void AuDesignerView::DesignerPaint()
{
	if(!IsInDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;
	StartCommand(new DdScPaint(this));
}

void AuDesignerView::DesignerPolish()
{
	if(!IsInDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;
	StartCommand(new DdScPolish(this));
}

void AuDesignerView::DesignerTextureCylindrical(int layer)
{
	if(!IsInDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;
	StartCommand(new DdScTextureCylindrical(layer, this));
}

void AuDesignerView::DesignerTextureProjected(int layer)
{
	if(!IsInDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;
	StartCommand(new DdScTextureProjected(layer, this));
}

void AuDesignerView::DesignerTextureSpherical(int layer)
{
	if(!IsInDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;
	StartCommand(new DdScTextureSpherical(layer, this));
}

void AuDesignerView::DesignerOpacity()
{
	if(!IsInDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;
	StartCommand(new DdScOpacity(this));
}

void AuDesignerView::DesignerSelfIllumination()
{
	if(!IsInDesignEnvironment())
		return;
	if(!DesignerObjectMode())
		return;
	StartCommand(new DdScSelfIllumination(this));
}

void AuDesignerView::DesignerCopySymmetrical()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScCopySymmetrical(this));
}

void AuDesignerView::DesignerSelectTextureVertex(DaDesigner* pde, quint16 partId, quint32 partVertex)
{
	if(pde->SelectPartTextureVertices(partId, partVertex, *m_RenderInfo))
		emit DesignerSelectionChanged();
}

void AuDesignerView::DesignerDeselectTextureVertex(DaDesigner* pde, quint16 partId, quint32 partVertex)
{
	if(pde->DeselectPartTextureVertices(partId, partVertex, *m_RenderInfo))
		emit DesignerSelectionChanged();
}

void AuDesignerView::DesignerBarrier()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScExtrusion(AuScBarrier::TypeStatic(), this));
}

void AuDesignerView::DesignerVoidHullExtrusion()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScExtrusion(AuScHullVoid::TypeStatic(), this));
}

void AuDesignerView::DesignerVoidHullSpindle()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScSpindle(AuScHullVoid::TypeStatic(), this));
}

void AuDesignerView::DesignerVoidRoomExtrusion()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScRoomVoidExtrusion(this));
}

void AuDesignerView::DesignerVoidRoomSpindle()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScRoomVoidSpindle(this));
}

void AuDesignerView::DesignerVoidEnvironmentExtrusion()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScEnvVoidExtrusion(this));
}

void AuDesignerView::DesignerVoidEnvironmentSpindle()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScEnvVoidSpindle(this));
}

void AuDesignerView::DesignerAssociate()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScVoidAssociate(this));
}

void AuDesignerView::DesignerDissociate()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;

	auApp->DesignerDissociateParts(DesignerPartSelection());
}

void AuDesignerView::DesignerSetRoomProperties()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScRoomProperties(this));
}

void AuDesignerView::DesignerSetVoidProperties()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScVoidProperties(this));
}

void AuDesignerView::DesignerLightBulbProps()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScLightBulbProps(this));
}

void AuDesignerView::DesignerLightLensProps()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScLightLensProps(this));
}

void AuDesignerView::DesignerLadderPath()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScLadderPath(this));
}

void AuDesignerView::DesignerWalkPath()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScWalkPath(this));
}

void AuDesignerView::DesignerBdLibraryDelete()
{
	// On line only.
	if(!IsInDesignEnvironment())
		return;

	// Get id and name of current design.
	quint32 designId = 0;
	QString designName;
	QString designReport;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			const AuDesign* d = pde->LockDesignForRead();
			if(d)
			{
				designId = d->Id();
				designName = d->Name();
				designReport = d->ReportAnalysis();

				m_DesignerFileName = "";

				// Update title on main frame window to show current design file name.
				QString windowTitle = QString("%1 %2 %3 %2 %4")
					.arg(g_Au.ApplicationName())
					.arg(g_Au.MiddleDotSymbol())
					.arg(designId)
					.arg(designName);
				auApp->Frame()->setWindowTitle(windowTitle);
			}
			pde->UnlockDesign();
		}
	}
	UnlockSceneAtom();

	// Show UI of existing designs, so they can choose one to open.
	AuBdLibDeleteDlg dlg(SceneAtomId(), this);
	dlg.SetDesign(designId, designName, designReport);
	auApp->Frame()->ExecDialog(dlg);
}

void AuDesignerView::DesignerScLibraryDelete()
{
	// On line only.
	if(!IsInDesignEnvironment())
		return;

	// Get id and name of current design.
	quint32 designId = 0;
	QString designName;
	QString designReport;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			const AuDesign* d = pde->LockDesignForRead();
			if(d)
			{
				designId = d->Id();
				designName = d->Name();
				designReport = d->ReportAnalysis();

				m_DesignerFileName = "";

				// Update title on main frame window to show current design file name.
				QString windowTitle = QString("%1 %2 %3 %2 %4")
					.arg(g_Au.ApplicationName())
					.arg(g_Au.MiddleDotSymbol())
					.arg(designId)
					.arg(designName);
				auApp->Frame()->setWindowTitle(windowTitle);
			}
			pde->UnlockDesign();
		}
	}
	UnlockSceneAtom();

	// Show UI of existing designs, so they can choose one to open.
	AuScLibDeleteDlg dlg(SceneAtomId(), this);
	dlg.SetDesign(designId, designName, designReport);
	auApp->Frame()->ExecDialog(dlg);
}

void AuDesignerView::DesignerItLibraryDelete()
{
	// On line only.
	if(!IsInDesignEnvironment())
		return;

	// Get id and name of current design.
	quint32 designId = 0;
	QString designName;
	QString designReport;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			const AuDesign* d = pde->LockDesignForRead();
			if(d)
			{
				designId = d->Id();
				designName = d->Name();
				designReport = d->ReportAnalysis();

				m_DesignerFileName = "";

				// Update title on main frame window to show current design file name.
				QString windowTitle = QString("%1 %2 %3 %2 %4")
					.arg(g_Au.ApplicationName())
					.arg(g_Au.MiddleDotSymbol())
					.arg(designId)
					.arg(designName);
				auApp->Frame()->setWindowTitle(windowTitle);
			}
			pde->UnlockDesign();
		}
	}
	UnlockSceneAtom();

	// Show UI of existing designs, so they can choose one to open.
	AuItLibDeleteDlg dlg(SceneAtomId(), this);
	dlg.SetDesign(designId, designName, designReport);
	auApp->Frame()->ExecDialog(dlg);
}

void AuDesignerView::DesignerScLibraryOpen()
{
	// On line only.
	if(!IsInDesignEnvironment())
		return;

	// Warn against losing the current design. Provide an option to cancel.
	if(!DesignerConfirmLoseChanges())
		return;

	// Get id and name of current design.
	quint32 designId = 0;
	QString designName;
	QString designReport;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			const AuDesign* d = pde->LockDesignForRead();
			if(d)
			{
				designId = d->Id();
				designName = d->Name();
				designReport = d->ReportAnalysis();

				m_DesignerFileName = "";

				// Update title on main frame window to show current design file name.
				QString windowTitle = QString("%1 %2 %3 %2 %4")
					.arg(g_Au.ApplicationName())
					.arg(g_Au.MiddleDotSymbol())
					.arg(designId)
					.arg(designName);
				auApp->Frame()->setWindowTitle(windowTitle);
			}
			pde->UnlockDesign();
		}
	}
	UnlockSceneAtom();

	// Show UI of existing designs, so they can choose one to open.
	AuDesignLibOpenDlg dlg(SceneAtomId(), this);
	dlg.SetDesign(designId, designName, designReport);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted && dlg.DesignId())
	{
		// Open the design in the library.
		auApp->DesignOpen(dlg.DesignId(), DaDesignBase::DESIGN_Spacecraft);
	}
}

void AuDesignerView::DesignerBdLibraryOpen()
{
	// On line only.
	if(!IsInDesignEnvironment())
		return;

	// Warn against losing the current design. Provide an option to cancel.
	if(!DesignerConfirmLoseChanges())
		return;

	// Get id and name of current design.
	quint32 designId = 0;
	QString designName;
	QString designReport;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			const AuDesign* d = pde->LockDesignForRead();
			if(d)
			{
				designId = d->Id();
				designName = d->Name();
				designReport = d->ReportAnalysis();

				m_DesignerFileName = "";

				// Update title on main frame window to show current design file name.
				QString windowTitle = QString("%1 %2 %3 %2 %4")
					.arg(g_Au.ApplicationName())
					.arg(g_Au.MiddleDotSymbol())
					.arg(designId)
					.arg(designName);
				auApp->Frame()->setWindowTitle(windowTitle);
			}
			pde->UnlockDesign();
		}
	}
	UnlockSceneAtom();

	// Show UI of existing designs, so they can choose one to open.
	AuBdLibOpenDlg dlg(SceneAtomId(), this);
	dlg.SetDesign(designId, designName, designReport);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted && dlg.DesignId())
	{
		// Open the design in the library.
		auApp->DesignOpen(dlg.DesignId(), DaDesignBase::DESIGN_Building);
	}
}

void AuDesignerView::DesignerItLibraryOpen()
{
	// On line only.
	if(!IsInDesignEnvironment())
		return;

	// Warn against losing the current design. Provide an option to cancel.
	if(!DesignerConfirmLoseChanges())
		return;

	// Get id and name of current design.
	quint32 designId = 0;
	QString designName;
	QString designReport;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			const AuDesign* d = pde->LockDesignForRead();
			if(d)
			{
				designId = d->Id();
				designName = d->Name();
				designReport = d->ReportAnalysis();

				m_DesignerFileName = "";

				// Update title on main frame window to show current design file name.
				QString windowTitle = QString("%1 %2 %3 %2 %4")
					.arg(g_Au.ApplicationName())
					.arg(g_Au.MiddleDotSymbol())
					.arg(designId)
					.arg(designName);
				auApp->Frame()->setWindowTitle(windowTitle);
			}
			pde->UnlockDesign();
		}
	}
	UnlockSceneAtom();

	// Show UI of existing designs, so they can choose one to open.
	AuItLibOpenDlg dlg(SceneAtomId(), this);
	dlg.SetDesign(designId, designName, designReport);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted && dlg.DesignId())
	{
		// Open the design in the library.
		auApp->DesignOpen(dlg.DesignId(), DaDesignBase::DESIGN_Item);
	}
}

void AuDesignerView::DesignerLibrarySave()
{
	// On line only.
	if(!IsInDesignEnvironment())
		return;

	// Grab a clone of the current design.
	AuDesign* design = 0;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			const AuDesign* d = pde->LockDesignForRead();
			if(d)
				design = d->Clone();
			pde->UnlockDesign();
		}
	}
	UnlockSceneAtom();

	if(!design)
		return;

	// Grab a thumbnail image of the screen, to accompany the design.
	QImage thumb = grabFramebuffer()
		.scaled(256,256, Qt::KeepAspectRatio, Qt::SmoothTransformation)
		.convertToFormat(QImage::Format_RGB16);
	//thumb.save("thumb.png");

	switch(m_DesignMode)
	{
	case DaDesignBase::InvalidMode:
	case DaDesignBase::DESIGN_Unknown:
		break;
	case DaDesignBase::DESIGN_Spacecraft:
		{
			// Show UI of existing designs, so they can choose a different name or to overwrite an existing design.
			AuDesignLibSaveDlg dlg(SceneAtomId(), thumb, this);
			dlg.SetDesign(design->Id(), design->Name(), design->ReportAnalysis());
			if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
			{
				// Configure design with results of UI selection.
				design->SetId(dlg.DesignId());							// 0 indicates "new" file.
				design->SetName(dlg.DesignName());
				if(!thumb.isNull())
					design->SetThumbnail(thumb);

				// Save the design into the library.
				bool sizeOk = auApp->DesignerLibrarySave(design);
				if(!sizeOk)
				{
					QMessageBox mb(g_Au.ApplicationName(), tr("The blueprint data is too large. "
								"The spacecraft design cannot be saved to the library. "),
							QMessageBox::Warning, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton,
							auApp->Frame());
					auApp->Frame()->ExecMessageBox(mb);
				}
				else
				{
					if(!m_DesignerFileName.isEmpty())
					{
						// Update title on main frame window to show current design file name.
						QString windowTitle = QString("%1 %2 %3")
							.arg(g_Au.ApplicationName())
							.arg(g_Au.MiddleDotSymbol())
							.arg(m_DesignerFileName);
						auApp->Frame()->setWindowTitle(windowTitle);
					}
					else
					{
						// Update title on main frame window to show current design file name.
						QString windowTitle = QString("%1 %2 %3 %2 %4")
							.arg(g_Au.ApplicationName())
							.arg(g_Au.MiddleDotSymbol())
							.arg(dlg.DesignId() ? QVariant(dlg.DesignId()).toString() : QObject::tr("New Design"))
							.arg(dlg.DesignName());
						auApp->Frame()->setWindowTitle(windowTitle);
					}
				}
			}
		}
		break;
	case DaDesignBase::DESIGN_Building:
		{
			// Show UI of existing designs, so they can choose a different name or to overwrite an existing design.
			AuBdLibSaveDlg dlg(SceneAtomId(), thumb, this);
			dlg.SetDesign(design->Id(), design->Name(), design->ReportAnalysis());
			if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
			{
				// Configure design with results of UI selection.
				design->SetId(dlg.DesignId());							// 0 indicates "new" file.
				design->SetName(dlg.DesignName());
				if(!thumb.isNull())
					design->SetThumbnail(thumb);

				// Save the design into the library.
				bool sizeOk = auApp->DesignerLibrarySave(design);
				if(!sizeOk)
				{
					QMessageBox mb(g_Au.ApplicationName(), tr("The blueprint data is too large. "
								"The building design cannot be saved to the library. "),
							QMessageBox::Warning, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton,
							auApp->Frame());
					auApp->Frame()->ExecMessageBox(mb);
				}
				else
				{
					if(!m_DesignerFileName.isEmpty())
					{
						// Update title on main frame window to show current design file name.
						QString windowTitle = QString("%1 %2 %3")
							.arg(g_Au.ApplicationName())
							.arg(g_Au.MiddleDotSymbol())
							.arg(m_DesignerFileName);
						auApp->Frame()->setWindowTitle(windowTitle);
					}
					else
					{
						// Update title on main frame window to show current design file name.
						QString windowTitle = QString("%1 %2 %3 %2 %4")
							.arg(g_Au.ApplicationName())
							.arg(g_Au.MiddleDotSymbol())
							.arg(dlg.DesignId() ? QVariant(dlg.DesignId()).toString() : QObject::tr("New Design"))
							.arg(dlg.DesignName());
						auApp->Frame()->setWindowTitle(windowTitle);
					}
				}
			}
		}
		break;
	case DaDesignBase::DESIGN_Item:
		{
			// Show UI of existing designs, so they can choose a different name or to overwrite an existing design.
			AuItLibSaveDlg dlg(SceneAtomId(), thumb, this);
			dlg.SetDesign(design->Id(), design->Name(), design->ReportAnalysis());
			if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
			{
				// Configure design with results of UI selection.
				design->SetId(dlg.DesignId());							// 0 indicates "new" file.
				design->SetName(dlg.DesignName());
				if(!thumb.isNull())
					design->SetThumbnail(thumb);

				// Save the design into the library.
				bool sizeOk = auApp->DesignerLibrarySave(design);
				if(!sizeOk)
				{
					QMessageBox mb(g_Au.ApplicationName(), tr("The blueprint data is too large. "
								"The item design cannot be saved to the library. "),
							QMessageBox::Warning, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton,
							auApp->Frame());
					auApp->Frame()->ExecMessageBox(mb);
				}
				else
				{
					if(!m_DesignerFileName.isEmpty())
					{
						// Update title on main frame window to show current design file name.
						QString windowTitle = QString("%1 %2 %3")
							.arg(g_Au.ApplicationName())
							.arg(g_Au.MiddleDotSymbol())
							.arg(m_DesignerFileName);
						auApp->Frame()->setWindowTitle(windowTitle);
					}
					else
					{
						// Update title on main frame window to show current design file name.
						QString windowTitle = QString("%1 %2 %3 %2 %4")
							.arg(g_Au.ApplicationName())
							.arg(g_Au.MiddleDotSymbol())
							.arg(dlg.DesignId() ? QVariant(dlg.DesignId()).toString() : QObject::tr("New Design"))
							.arg(dlg.DesignName());
						auApp->Frame()->setWindowTitle(windowTitle);
					}
				}
			}
		}
		break;
	}

	// Delete the clone.
	delete design;
}

void AuDesignerView::DesignerGearOpen()
{
	// On line only.
	if(!IsInDesignEnvironment())
		return;

	AuDesignerGearOpen dlg(this);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		quint32 designId = dlg.DesignId();
		AuGearPos pos = dlg.GearPos();
		auApp->DesignerGearOpen(designId, pos);
	}
}

void AuDesignerView::DesignerGearSaveBd()
{
	// On line only.
	if(!IsInDesignEnvironment())
		return;

	AuBdGearSave dlg(SceneAtomId(), this);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		if(!dlg.DesignId())
			return;

		const QHash<quint32, AuBdDesignListEntry>& designs = g_AuClient.LockBdDesignList();
		QHash<quint32, AuBdDesignListEntry>::const_iterator di = designs.find(dlg.DesignId());
		if(di == designs.cend() ||
				((*di).State() != AuDesign::Final && (*di).State() != AuDesign::FinalAssembly))
		{
			g_AuClient.UnlockBdDesignList();
			return;
		}

		AuBdDesignExchangeEntry libEntry((*di).Report(), (*di).State(), 0, dlg.CanCopy(), dlg.CanView(), dlg.UseOnce(),
				SceneAtomEmpire(), dlg.MinStance());
		g_AuClient.UnlockBdDesignList();

		auApp->BdExchangeSaveToGear(libEntry, dlg.MediaPos());
	}
}

void AuDesignerView::DesignerGearSaveSc()
{
	// On line only.
	if(!IsInDesignEnvironment())
		return;

	AuDesignerGearSave dlg(SceneAtomId(), this);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		if(!dlg.DesignId())
			return;

		const QHash<quint32, AuScDesignListEntry>& designs = g_AuClient.LockScDesignList();
		QHash<quint32, AuScDesignListEntry>::const_iterator di = designs.find(dlg.DesignId());
		if(di == designs.cend() ||
				((*di).State() != AuDesign::Final && (*di).State() != AuDesign::FinalAssembly))
		{
			g_AuClient.UnlockScDesignList();
			return;
		}

		AuScDesignExchangeEntry libEntry((*di).Report(), (*di).State(), 0, dlg.CanCopy(), dlg.CanView(), dlg.UseOnce(),
				SceneAtomEmpire(), dlg.MinStance());
		g_AuClient.UnlockScDesignList();

		auApp->ScExchangeSaveToGear(libEntry, dlg.MediaPos());
	}
}

void AuDesignerView::DesignerFinalize()
{
	if(!IsInDesignEnvironment())
		return;

	switch(m_DesignMode)
	{
	case DaDesignBase::InvalidMode:
	case DaDesignBase::DESIGN_Unknown:
		return;

	case DaDesignBase::DESIGN_Spacecraft:
		{
			// Grab a thumbnail image of the screen, to accompany the design.
			QImage thumb = grabFramebuffer().convertToFormat(QImage::Format_RGB16).scaled(256,256, Qt::KeepAspectRatio,
					Qt::SmoothTransformation);
			//thumb.save("thumb.png");

			// The dialog grabs a clone of the current design in its constructor. That is the one that is finalized, in case of 
			// changes while the dialog is up.
			AuScFinalizeDlg* dlg = 0;
			DaPerson* sceneAtom;
			LockSceneAtom(&sceneAtom);
			if(sceneAtom)
			{
				DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
				if(pde)
				{
					const AuDesign* d = pde->LockDesignForRead();
					if(d)
						dlg = new AuScFinalizeDlg(*d, thumb, sceneAtom->Id(), sceneAtom->Name(), this);
					pde->UnlockDesign();
				}
			}
			UnlockSceneAtom();
			if(!dlg)
				return;

			// Show the dialog.
			if(auApp->Frame()->ExecDialog(*dlg) == QDialog::Accepted)
			{
				bool sizeOk;
				if(dlg->IsAssembly())
					sizeOk = auApp->DesignerLibraryFinalizeAssembly(dlg->Design());
				else
					sizeOk = auApp->DesignerLibraryFinalizeDesign(dlg->Design(), dlg->DesignId());
				if(!sizeOk)
				{
					QMessageBox mb(g_Au.ApplicationName(), tr("The blueprint data is too large. "
								"The finalized spacecraft design cannot be sent to the server. "),
							QMessageBox::Warning, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton,
							auApp->Frame());
					auApp->Frame()->ExecMessageBox(mb);
				}
			}

			// Preserve last result.
			m_FinalizeReport = dlg->Report();
			delete dlg;
		}
		break;

	case DaDesignBase::DESIGN_Building:
		{
			// Grab a thumbnail image of the screen, to accompany the design.
			QImage thumb = grabFramebuffer().convertToFormat(QImage::Format_RGB16).scaled(256,256, Qt::KeepAspectRatio,
					Qt::SmoothTransformation);
			//thumb.save("thumb.png");

			// The dialog grabs a clone of the current design in its constructor. That is the one that is finalized, in case of 
			// changes while the dialog is up.
			AuBdFinalizeDlg* dlg = 0;
			DaPerson* sceneAtom;
			LockSceneAtom(&sceneAtom);
			if(sceneAtom)
			{
				DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
				if(pde)
				{
					const AuDesign* d = pde->LockDesignForRead();
					if(d)
						dlg = new AuBdFinalizeDlg(*d, thumb, sceneAtom->Id(), sceneAtom->Name(), this);
					pde->UnlockDesign();
				}
			}
			UnlockSceneAtom();
			if(!dlg)
				return;

			// Show the dialog.
			if(auApp->Frame()->ExecDialog(*dlg) == QDialog::Accepted)
			{
				bool sizeOk;
				if(dlg->IsAssembly())
					sizeOk = auApp->DesignerLibraryFinalizeAssembly(dlg->Design());
				else
					sizeOk = auApp->DesignerLibraryFinalizeDesign(dlg->Design(), dlg->DesignId());
				if(!sizeOk)
				{
					QMessageBox mb(g_Au.ApplicationName(), tr("The blueprint data is too large. "
								"The finalized building design cannot be sent to the server. "),
							QMessageBox::Warning, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton,
							auApp->Frame());
					auApp->Frame()->ExecMessageBox(mb);
				}
			}

			// Preserve last result.
			m_FinalizeReport = dlg->Report();
			delete dlg;
		}
		break;

	case DaDesignBase::DESIGN_Item:
		{
			// Grab a thumbnail image of the screen, to accompany the design.
			QImage thumb = grabFramebuffer().convertToFormat(QImage::Format_RGB16).scaled(64,64, Qt::KeepAspectRatio,
					Qt::SmoothTransformation);
			//thumb.save("thumb.png");

			// The dialog grabs a clone of the current design in its constructor. That is the one that is finalized, in case of 
			// changes while the dialog is up.
			AuItFinalizeDlg* dlg = 0;
			DaPerson* sceneAtom;
			LockSceneAtom(&sceneAtom);
			if(sceneAtom)
			{
				DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
				if(pde)
				{
					const AuDesign* d = pde->LockDesignForRead();
					if(d)
						dlg = new AuItFinalizeDlg(*d, thumb, sceneAtom->Id(), sceneAtom->Name(), this);
					pde->UnlockDesign();
				}
			}
			UnlockSceneAtom();
			if(!dlg)
				return;

			// Show the dialog.
			if(auApp->Frame()->ExecDialog(*dlg) == QDialog::Accepted)
			{
				bool sizeOk;
				if(dlg->IsAssembly())
					sizeOk = auApp->DesignerLibraryFinalizeAssembly(dlg->Design());
				else
					sizeOk = auApp->DesignerLibraryFinalizeDesign(dlg->Design(), dlg->DesignId());
				if(!sizeOk)
				{
					QMessageBox mb(g_Au.ApplicationName(), tr("The blueprint data is too large. "
								"The finalized item design cannot be sent to the server. "),
							QMessageBox::Warning, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton,
							auApp->Frame());
					auApp->Frame()->ExecMessageBox(mb);
				}
			}

			// Preserve last result.
			m_FinalizeReport = dlg->Report();
			delete dlg;
		}
		break;

	//case DaDesignBase::DESIGN_Vehicle:
		//break;
	}
}

void AuDesignerView::DesignerFinalizeReport()
{
	AuFinalizeReportDlg dlg(m_FinalizeReport);
	auApp->Frame()->ExecDialog(dlg);
}

bool AuDesignerView::DesignerObjectModeOk()
{
	// Designer does nothing in preview mode.
	if(IsDesignPreview())
		return false;
	if(!m_RenderInfo->DesignerHasPartSelection())
		return false;
	//if(!m_RenderInfo->AnchorPartSelected())
		//return false;

	// Cannot enter face mode if any of the selected parts is sealed by an architect other than the current one.
	return !HasSealedPartsSelected();
}

bool AuDesignerView::HasSealedPartsSelected()
{
	// Insure none of the selected parts is sealed by an architect other than the current one.
	bool hasSealedParts = false;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
		{
			const AuDesign* d = pde->LockDesignForRead();
			if(d && d->HasSealedParts(sceneAtom->Id(), m_RenderInfo->DesignerPartObjectSelection()))
				hasSealedParts = true;
			pde->UnlockDesign();
		}
	}
	UnlockSceneAtom();
	return hasSealedParts;
}

bool AuDesignerView::DesignerIsViewOnly()
{
	// Insure none of the selected parts is sealed by an architect other than the current one.
	bool viewOnly = true;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
			viewOnly = pde->IsDesignViewOnly();
	}
	UnlockSceneAtom();
	return viewOnly;
}

void AuDesignerView::DesignerGoToSelection()
{
	if(!IsInDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;

	auApp->DesignerGoToSelection(DesignerPartSelection());
}

void AuDesignerView::DesignerAlign()
{
	if(!IsInNewDesignEnvironment())
		return;
	StartCommand(new DdScAlign(this));
}

void AuDesignerView::DesignerAlignLiftStop()
{
	if(!IsInNewDesignEnvironment())
		return;
	StartCommand(new DdScAlignLiftStop(this));
}

void AuDesignerView::DesignerMoveDoorSwitches()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScMoveDoorSwitches(this));
}

void AuDesignerView::OperateTurboLift(const AuId& shipId, const AuId& personId, quint16 liftId)
{
	QStringList stopNames;
	QString currentStop;
	DaPerson* person;
	LockSceneAtom(&person);
	if(person && person->Is(personId))
	{
		DaAtom* envAtom = person->FindEnvironmentAtom();
		if(envAtom)
		{
			const DaCapsule* capsule = dynamic_cast<const DaCapsule*>(envAtom->FindIdRecursive(shipId));
			if(capsule)
				currentStop = capsule->TurboLiftStop(liftId, stopNames);
		}
	}
	UnlockSceneAtom();

	if(stopNames.isEmpty())
		return;

	QMap<QString, QString> sortedStops;
	for(QStringList::const_iterator si = stopNames.cbegin(); si != stopNames.cend(); si++)
		sortedStops.insert((*si).toUpper(), *si);

	// Show a pop-up menu of lift stops.
	QMenu menu(this);
	for(QMap<QString, QString>::const_iterator si = sortedStops.cbegin(); si != sortedStops.cend(); si++)
	{
		if(*si == currentStop)
			menu.addAction(QIcon(":data/ScChangeStop.png"), *si);
		else
			menu.addAction(QIcon(":data/ScLiftStop.png"), *si);
	}

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;

	QString stopName = action->text();
	auApp->CallTurboLift(shipId, liftId, stopName);
}

void AuDesignerView::DesignerTransporterPadLocs()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScTransporterPadLocs(this));
}

void AuDesignerView::DesignerWeaponEmitLoc()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdScWeaponEmitLoc(this));
}

void AuDesignerView::AvatarOnLine(AuId id)
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
	auApp->PlayClientSound(SOUND_BuddyOnLine, m_Fade);
}

void AuDesignerView::DesignerFence()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdBdFence(this));
}

void AuDesignerView::DesignerShrub()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdBdShrub(this));
}

void AuDesignerView::DesignerSite()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdBdSite(this));
}

void AuDesignerView::DesignerBuildZone()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdBdBuildZone(this));
}

void AuDesignerView::DesignerStationCitizen()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdBdCitizenPost(this));
}

void AuDesignerView::DesignerStationLivestock()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdBdLivestockPost(this));
}

void AuDesignerView::DesignerTree()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(DesignerObjectMode())
		return;
	StartCommand(new DdBdTree(this));
}

void AuDesignerView::PlanBd(INDUSTRYID industryId, const AuBdReport& report, COMMODITYID constructionMaterial, quint16 mpid,
		bool enableWorker)
{
	AuKnownPatents patents;	// All patents possessed by the person and their empire.
	// Get patents held in the person's personal gear.
	quint32 empireId = 0;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		sceneAtom->ListPatents(patents);
		empireId = sceneAtom->EmpireId();
	}
	UnlockSceneAtom();

	// Get empire patents.
	if(empireId)
	{
		const DgData& gd = g_Au.LockDataForRead();
		DgEmpireHash::const_iterator ei = gd.m_gEmpires.find(empireId);
		if(ei != gd.m_gEmpires.cend())
			(*ei).second.ListIndustryPatents(patents, industryId, gd);
		g_Au.UnlockData();
	}

	StartCommand(new DdPlanBdBuilding(industryId, report, constructionMaterial, mpid, enableWorker, patents, this));
}

void AuDesignerView::PlanBdFence(int fenceStyle, quint32 customDesignId, COMMODITYID constructionMaterial)
{
	StartCommand(new DdPlanBdFence(fenceStyle, customDesignId, constructionMaterial, this));
}

void AuDesignerView::PlanBdConstructionProcess(const AuBdReport& report)
{
	if(!Command())
		return;
	Command()->SetConstructionProcess(report);
}

void AuDesignerView::PlanBdMaterial(COMMODITYID constructionMaterial)
{
	if(!Command())
		return;
	Command()->SetMaterial(constructionMaterial);
}

void AuDesignerView::PlanGeoRoadAsphalt()
{
	if(IsInDesignEnvironment())
		return;
	StartCommand(new DdPlanGeoRoad(AuGeoRoadAsphalt::GeoType(), this));
}

void AuDesignerView::PlanGeoRoadConcrete()
{
	if(IsInDesignEnvironment())
		return;
	StartCommand(new DdPlanGeoRoad(AuGeoRoadConcrete::GeoType(), this));
}

void AuDesignerView::PlanGeoRoadDirt()
{
	if(IsInDesignEnvironment())
		return;
	StartCommand(new DdPlanGeoRoad(AuGeoRoadDirt::GeoType(), this));
}

void AuDesignerView::PlanGeoRoadGrade()
{
	if(IsInDesignEnvironment())
		return;
	StartCommand(new DdPlanGeoRoad(AuGeoRoadGrade::GeoType(), this));
}

void AuDesignerView::PlanGeoLandscapeClearing()
{
	if(IsInDesignEnvironment())
		return;
	StartCommand(new DdPlanGeoLandscape(0, this));
}

void AuDesignerView::PlanGeoLandscapeDefoliate()
{
	if(IsInDesignEnvironment())
		return;
	StartCommand(new DdPlanGeoLandscape(1, this));
}

void AuDesignerView::PlanGeoLandscapeIrrigate()
{
	if(IsInDesignEnvironment())
		return;
	StartCommand(new DdPlanGeoLandscape(2, this));
}

void AuDesignerView::SyncScene()
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

void AuDesignerView::DesignerPreviewSettings()
{
	bool hasPower = true;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesigner* de = sceneAtom->FindParentDesignerInEnv();
		if(de)
			hasPower = de->PreviewHasPower();
	}
	UnlockSceneAtom();

	switch(m_DesignMode)
	{
	case DaDesignBase::InvalidMode:
	case DaDesignBase::DESIGN_Unknown:
		break;

	case DaDesignBase::DESIGN_Spacecraft:
		{
			AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
			AuScPreview dlg(pFrame);
			connect(&dlg, SIGNAL(TimeOfDayChanged(float)), SLOT(DesignerSetPreviewTimeOfDay(float)));
			auApp->Frame()->ExecDialog(dlg);
		}
		break;

	case DaDesignBase::DESIGN_Building:
		{
			AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
			AuBdPreview dlg(hasPower, pFrame);
			connect(&dlg, SIGNAL(HasPowerChanged(bool)), SLOT(DesignerSetPreviewHasPower(bool)));
			connect(&dlg, SIGNAL(TimeOfDayChanged(float)), SLOT(DesignerSetPreviewTimeOfDay(float)));
			auApp->Frame()->ExecDialog(dlg);
		}
		break;

	case DaDesignBase::DESIGN_Item:
		break;

	//case DaDesignBase::DESIGN_Vehicle:
		//break;
	}
}

void AuDesignerView::DesignerSetPreviewTimeOfDay(float angle)
{
	AuId setWorld;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesigner* de = sceneAtom->FindParentDesignerInEnv();
		if(de && de->IsPreview())
		{
			DaWorldLL* world = dynamic_cast<DaWorldLL*>(sceneAtom->FindParentPlanetInEnv());
			if(world && world->InDesignerPreview())
			{
				world->SetRotationAngleOffset(angle);
				setWorld = world->Id();
			}
		}
	}
	UnlockSceneAtom();

	if(!setWorld.IsNull())
	{
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaWorldSphere* world = dynamic_cast<DaWorldSphere*>(ether.FindAtom(setWorld));
		if(world)
			world->SetRotationAngleOffset(angle);
		g_Au.UnlockEtheriumDelete();
	}
}

void AuDesignerView::DesignerSetPreviewHasPower(bool hasPower)
{
	AuId setWorld;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesigner* de = sceneAtom->FindParentDesignerInEnv();
		if(de && de->IsPreview())
			de->SetPreviewHasPower(hasPower);
	}
	UnlockSceneAtom();
}

void AuDesignerView::DesignerPreviewBd()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(!IsDesignPreview())
		return;
	if(DesignMode() != DaDesignBase::DESIGN_Building)
		return;

	// Hitting this while a command is running kills it.
	if(Command())
		EndCommands();
	else
	{
		if(DesignBdStructure() == BD_Fence)
			StartCommand(new DdPreviewFence(this));
		else
			StartCommand(new DdBdPreview(this));
	}
}

void AuDesignerView::DesignerPreviewPeople()
{
	if(!IsInNewDesignEnvironment())
		return;
	if(!IsDesignPreview())
		return;
	// Get the species of the scene atom.
	AuSpecies species;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
		species = sceneAtom->Species();
	UnlockSceneAtom();
	auApp->DesignerPreviewPeople(species);
}

void AuDesignerView::RenderControlButtons()
{
	bool renderButtons = true;
	if(!g_AuClient.ViewActionButtons())
		renderButtons = false;
	else if(!m_RenderInfo->m_ShowHUD)
		renderButtons = false;
	else if(rect().width() <= 0 || rect().height() <= 0)
		renderButtons = false;
	else if(IsInDesignEnvironment() && !m_IsDesignPreview)
		renderButtons = false;
	else if(m_RenderInfo->IsViewTopDown())
		renderButtons = false;

	// Modulate alpha using user's view button transparency setting.
	float alpha = 1;
	alpha *= g_AuClient.ViewButtonAlpha();
	if(alpha == 0)
		renderButtons = false;

	if(!renderButtons)
	{
		m_ControlButtonAt = AuControlButton();
		return;
	}

	// Position buttons.
	PositionControlButtons();

	// Update hot button under the cursor.
	AuControlButton controlButtonAt = ControlButtonAt(m_MouseLast);
	if(controlButtonAt != m_ControlButtonAt)
	{
		// Control button changed.
		m_ControlButtonAt = controlButtonAt;
	}

	AuMatrix4x4 p;
	double gldAspect = AuGLEffects::WindowAspectRatio(rect());
	p.Scale(gldAspect, 1, 1);

	// Calculate the position of the first main button at the top right of the screen.
	double vbRadius = ViewButtonSubRadius();
	double vbLength = ControlButtonLength();
	double vbSpacing = vbRadius / 10.;

	m_RenderInfo->re->glDisable(GL_DEPTH_TEST);
	m_RenderInfo->re->glEnable(GL_BLEND);
	m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Backdrop to buttons.
	{
		m_RenderInfo->re->GLSL().LineShader().Select();
		int idx=0;
		for(QList<AuControlButton>::const_iterator ci = m_ControlButtons.cbegin(); ci != m_ControlButtons.cend(); ci++, idx++)
		{
			float buttonAlpha;
			if((*ci == m_ControlButtonPressed && m_ControlButtonPressed == m_ControlButtonAt))
				buttonAlpha = 1;
			else if(*ci == m_ControlButtonAt)
				buttonAlpha = 1;
			else
				buttonAlpha = alpha;

			// Prepare to draw the button icon.
			m_RenderInfo->re->GLSL().LineShader().SetColor(0,0,0, buttonAlpha);

			AuMatrix4x4 mvm;
			mvm.Translate((*ci).m_ScreenPos.x/gldAspect + vbLength/2, (*ci).m_ScreenPos.y, 0);
			mvm.Scale(vbLength/2., vbRadius, 1);
			m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(p * mvm);
			m_SubMidVAO.SelectGL();
			m_SubMidVAO.DrawGL();
			m_SubMidVAO.DeselectGL();
		}
		m_RenderInfo->re->GLSL().LineShader().Deselect();
	}

	// Icon and text on buttons.
	{
		QFont font;
		font.setPointSize(20);
		AuPoint tloc(0,0,0);
		for(QList<AuControlButton>::const_iterator ci = m_ControlButtons.cbegin(); ci != m_ControlButtons.cend(); ci++)
		{
			float buttonAlpha;
			if((*ci == m_ControlButtonPressed && m_ControlButtonPressed == m_ControlButtonAt))
				buttonAlpha = 1;
			else if(*ci == m_ControlButtonAt)
				buttonAlpha = 1;
			else
				buttonAlpha = alpha;

			// Draw the icon on the button.
			AuMatrix4x4 mvt;
			mvt.Translate((*ci).m_ScreenPos.x/gldAspect + vbSpacing + vbRadius, (*ci).m_ScreenPos.y, -1);
			if((*ci).m_Texture)
			{
				AuMatrix4x4 mvi(mvt);
				if(*ci == m_ControlButtonAt)
					mvi.Scale(vbRadius*.75, vbRadius*.75, 1);
				else
					mvi.Scale(vbRadius*.5, vbRadius*.5, 1);
				m_RenderInfo->re->GLSL().TextureShader().Select();
				m_RenderInfo->re->GLSL().TextureShader().SetProjectionModelView(p * mvi);
				m_RenderInfo->re->GLSL().TextureShader().SetColor(1,1,1, buttonAlpha);
				m_RenderInfo->re->GLSL().TextureShader().SetTexture((*ci).m_Texture);
				m_ButtonVAO.SelectGL();
				m_ButtonVAO.DrawGL();
				m_ButtonVAO.DeselectGL();
				m_RenderInfo->re->GLSL().TextureShader().Deselect();
			}
			mvt.Translate(vbRadius + vbSpacing, 0, 0);

			// Draw the text on the button.
			if((*ci == m_ControlButtonPressed && m_ControlButtonPressed == m_ControlButtonAt))
			{
				m_RenderInfo->re->RenderText(p, mvt, tloc, (*ci).m_Text, font, QColor(Qt::lightGray), rect().height(),
						Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);

				m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				m_RenderInfo->re->RenderText(p, mvt, tloc, (*ci).m_Text, font, QColor(217,172,66), rect().height(),
						Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
				m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else if(*ci == m_ControlButtonAt)
			{
				m_RenderInfo->re->RenderText(p, mvt, tloc, (*ci).m_Text, font, QColor(85,170,255), rect().height(),
						Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
			}
			else
			{
				m_RenderInfo->re->RenderText(p, mvt, tloc, (*ci).m_Text, font, QColor(Qt::lightGray), rect().height(),
						Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
			}
		}
	}
	m_RenderInfo->re->glDisable(GL_BLEND);
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);
}

void AuDesignerView::PositionControlButtons()
{
	if(!g_AuClient.ViewActionButtons())
		return;
	m_ControlButtons.clear();
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		QPoint pxFind(rect().center());
		qint16 idPanel;
		AuPoint panelLoc;
		AuPoint hitLoc;
		DaAtom* atom = FindControlAt(sceneAtom, pxFind, idPanel, panelLoc, hitLoc);
		if(atom)
		{
			bool okToControlAtom = true;
			const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(atom);
			if(idPanel != CID_Invalid && llworld)
			{
				// Impose a limit to how much can be foraged from the same area in a certain time period.
				// Ideally, this would result in a slower rate of foraging than a mine or logging camp.
				// The panel id is the COMMODITYID foraged.
				AuPoint globeLoc;
				sceneAtom->GetLocationRelativeToParent(globeLoc, llworld);
				AuForageHistory forageAttempt = AuForageHistory((COMMODITYID)idPanel, atom->Id(), globeLoc);
				
				// Check for any recent forage activity that prevents this attempt.
				// To manage the list, we will throw away and ignore items that have grown too old.
				QList<AuForageHistory>::iterator fi = m_ForageHistory.begin();
				while(fi != m_ForageHistory.end())
				{
					if((*fi).IsExpired())
					{
						// Discard the forage history item that has grown too old.
						fi = m_ForageHistory.erase(fi);
					}
					else
					{
						// The forage history item has not expired. See if it blocks the current forage attempt.
						bool foragePrevented = (*fi).PreventsForageOnGlobe(forageAttempt);
						if(foragePrevented)
							okToControlAtom = false;
						fi++;
					}
				}
			}
			if(okToControlAtom)
				atom->EnumViewPanelControls(idPanel, m_ControlButtons, *m_RenderInfo);
		}
		// Parent atom may have controls available just by standing there.
		sceneAtom->EnumViewControls(m_ControlButtons, *m_RenderInfo);
	}
	UnlockSceneAtom();

	// Calculate the position of the first main button at the top right of the screen.
	double vbRadius = ViewButtonSubRadius();
	double vbLength = ControlButtonLength();
	double vbSpacing = vbRadius / 10.;
	double vbMargin = vbRadius / 2.;
	double vbHeight = vbRadius * 2. + vbSpacing;
	double gldAspect = AuGLEffects::WindowAspectRatio(rect());
	double hht = (vbHeight * m_ControlButtons.size()) / 2;
	AuPoint bpos(1. - ((vbLength + vbMargin) * gldAspect), hht);	// u/l corner of first button
	for(QList<AuControlButton>::iterator bi = m_ControlButtons.begin(); bi != m_ControlButtons.end(); bi++)
	{
		(*bi).m_ScreenPos = bpos;
		bpos.y -= vbHeight;				// Walk down the screen.
	}
}

// Returns the width of the widest sub item.
double AuDesignerView::ControlButtonLength() const
{
	if(rect().height() <= 0)
		return 0;
	double widestSub = 0;
	for(QList<AuControlButton>::const_iterator bi = m_ControlButtons.cbegin(); bi != m_ControlButtons.cend(); bi++)
	{
		double subWid = ControlButtonTextLength((*bi).m_Text);
		if(widestSub < subWid)
			widestSub = subWid;
	}
	double vbRadius = ViewButtonSubRadius();
	double vbSpacing = vbRadius / 10.;
	return widestSub + vbRadius*2 + vbSpacing;
}

double AuDesignerView::ControlButtonTextLength(const QString& text) const
{
	if(rect().height() <= 0)
		return 0;

	// This is like a menu item, it must always be a fixed size.
	// Let's guess at the width based on the number of characters.
	double pixelsLength = text.count() * 11;
	double pxPerUnit = (double)rect().height()/2.;	// OpenGL screen height is 2 units, -1 to 1.
	return pixelsLength / pxPerUnit;
}

AuControlButton AuDesignerView::ControlButtonAt(const QPoint& mousePos) const
{
	if(!g_AuClient.ViewActionButtons())
		return AuControlButton();
	if(rect().width() <= 0 || rect().height() <= 0)
		return AuControlButton();

	double gldAspect = AuGLEffects::WindowAspectRatio(rect());

	// Calculate position of mouse in the range -1 to 1 in each axis.
	AuPoint mouse;
	mouse.x = (double)mousePos.x()/(double)rect().width() - .5;		// This gets us into the range -.5 to .5.
	mouse.x /= gldAspect;
	mouse.y = -((double)mousePos.y()/(double)rect().height() - .5);	// This gets us into the range -.5 to .5.
	mouse *= 2;

	double vbSubLength = ControlButtonLength();
	double vbSubRadius = ViewButtonSubRadius();
	for(QList<AuControlButton>::const_iterator bi = m_ControlButtons.cbegin(); bi != m_ControlButtons.cend(); bi++)
	{
		AuPoint spos((*bi).m_ScreenPos);
		spos.x /= gldAspect;

		double dist;
		AuPoint epos((*bi).m_ScreenPos);
		epos.x += vbSubLength;
		epos.x /= gldAspect;
		dist = mouse.DistToSegment_2D(spos, epos);
		if(dist <= vbSubRadius)
			return (*bi);
	}
	return AuControlButton();
}

void AuDesignerView::ControlButtonClicked(const AuControlButton& button)
{
	// Atom control timer keeps us from sending control updates too frequently.
	m_ControlLock.lock();
	if(m_AtomControlTime.isValid() && m_AtomControlTime.elapsed() < d_AtomControlTime)
	{
		m_ControlLock.unlock();
		return;
	}
	int msElapsed;
	if(m_AtomControlTime.isValid())
		msElapsed = m_AtomControlTime.elapsed();
	else
		msElapsed = 0;
	m_AtomControlTime.restart();
	m_ControlLock.unlock();

	m_TargetLock.lockForRead();
	AuId targetId(m_TargetId);
	int targetLevel = m_TargetLevel;
	AuPoint targetLoc(m_TargetLoc);
	unsigned int targetPart = m_TargetPart;
	bool targetShield = m_TargetShield;
	m_TargetLock.unlock();

	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuGearPos gearPos = pFrame->GetGearSelectionPos();

	// Insure they don't forage too much.
	DaAtom* atom = 0;
	qint8 chValue = 0;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaAtom* env = sceneAtom->FindEnvironmentAtom();
		if(env)
		{
			atom = env->FindIdRecursive(button.m_AtomId);
			if(atom)
			{
				const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(atom);
				if(button.m_AtomPanel != CID_Invalid && llworld)
				{
					// Impose a limit to how much can be foraged from the same area in a certain time period.
					// Ideally, this would result in a slower rate of foraging than a mine or logging camp.
					// The panel id is the COMMODITYID foraged.
					AuPoint globeLoc;
					sceneAtom->GetLocationRelativeToParent(globeLoc, llworld);
					AuForageHistory forageAttempt = AuForageHistory((COMMODITYID)button.m_AtomPanel, atom->Id(), globeLoc);
					
					// Check for any recent forage activity that prevents this attempt.
					// To manage the list, we will throw away and ignore items that have grown too old.
					QList<AuForageHistory>::iterator fi = m_ForageHistory.begin();
					while(fi != m_ForageHistory.end())
					{
						if((*fi).IsExpired())
						{
							// Discard the forage history item that has grown too old.
							fi = m_ForageHistory.erase(fi);
						}
						else
						{
							// The forage history item has not expired. See if it blocks the current forage attempt.
							bool foragePrevented = (*fi).PreventsForageOnGlobe(forageAttempt);
							if(foragePrevented)
							{
								// The terrain cannot be foraged again. More time must pass. Tell the user why.
								QString text;
								COMMODITYID cid = forageAttempt.CommodityId();
								switch(cid)
								{
								default:
									text = tr("All the readily accessible %1 has been gleaned from this area.")
										.arg(g_Au.CommodityName(cid));
									break;

								// Rock Resources
								case CID_Water:
									text = tr("The bottle is filling with %1 slowly.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Bolite:
								case CID_Ice:
									text = tr("The frozen mass of %1 cracked but a piece did not break away.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Coal:
								case CID_Stone:
									text = tr("Searching the boulders and rubble did not reveal another usable chunk of %1.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_CrudeOil:
								case CID_Viathol:
								case CID_Polytaride:
									text = tr("The pool of %1 is now empty. Oozing fissures have been exposed.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Crystals:
								case CID_Eludium:
									text = tr("Splinters and fragments crumble as you search. "
											"A good shard of %1 could not be found.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Gems:
								case CID_Lumenite:
									text = tr("Clear bright %1 could not be found among many tantalizing bits and pieces.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_GoldOre:
								case CID_Minerals:
								case CID_Ore:
								case CID_Radioactives:
								case CID_Vulcanite:
								case CID_Adamantite:
									text = tr("The rich vein of %1 could not be followed into the substrate.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Phlogiston:
									text = tr("The roaring %1 vent has subsided. "
											"The pressure is now too low to easily collect more from the seeping cracks.")
										.arg(g_Au.CommodityName(cid));
									break;

								// Plant Resources
								case CID_Beans:					// Shrub
								case CID_Grain:					// Shrub
								case CID_Grapes:				// Shrub
								case CID_Hops:					// Shrub
								case CID_Vegetable:				// Shrub
									text = tr("The remaining clusters of %1 are not ripe.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Fiber:					// Shrub or Tree
									text = tr("The branches are bare of %1.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Fruit:					// Shrub or Tree
								case CID_Herbs:					// Shrub or Tree
								case CID_Nuts:					// Shrub or Tree
								case CID_Spices:				// Shrub or Tree
									text = tr("The branches have been picked clean of ripe %1.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Hay:					// Shrub
									text = tr("The %1 needs to grow before it can be harvested again.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Wood:					// Tree
									text = tr("The branch creaked but a %1 did not break away.")
										.arg(g_Au.CommodityName(cid));
									break;
								case CID_Seed:					// Ground cover
									text = tr("The tiny plants are bare of seeds.");
									break;
								}

								LocalLogMessage(text);
								atom = 0;
								break;
							}
							fi++;
						}
					}

					// Remember successful forage attempts.
					if(atom)
						m_ForageHistory.append(forageAttempt);
				}
				else
				{
					const DaItem* item = dynamic_cast<const DaItem*>(atom);
					if(item)
					{
						// Watch for armed bombs and static display items.
						QString empireName;
						const DgData& gd = g_Au.LockDataForRead();
						bool isAgent = gd.ClientPlayerIsAgent();		// Show design tips to agents.
						g_Au.UnlockData();
						bool isControllable = item->IsStaticDisplayControllableBy(m_idSceneAtom, m_SceneAtomEmpire);
						if(isControllable || (item->IsStaticDisplay() && isAgent))
						{
							// Display a modeless dialog to manipulate the item.
							atom = 0;
							QString whatItem = item->SpotDescription(SceneAtomEmpire());
							if(isControllable)
							{
								if(!item->Inscription().isEmpty())
									LocalLogMessage(tr("%1 displayed by %2, selected for manipulation. "
												"The inscription says, \"%3\"")
											.arg(whatItem)
											.arg(item->OwnerName())
											.arg(item->Inscription()));
								else
									LocalLogMessage(tr("%1 displayed by %2, selected for manipulation.")
											.arg(whatItem)
											.arg(item->OwnerName()));
							}
							else
							{
								if(!item->Inscription().isEmpty())
									LocalLogMessage(tr("%1 displayed by %2, selected for manipulation by agent. "
												"The inscription says, \"%3\"")
											.arg(whatItem)
											.arg(item->OwnerName())
											.arg(item->Inscription()));
								else
									LocalLogMessage(tr("%1 displayed by %2, selected for manipulation by agent.")
											.arg(whatItem)
											.arg(item->OwnerName()));
							}
							ShowItemManipulator(*item);
						}
						else if(item->IsStaticDisplay())
						{
							atom = 0;
							QString whatItem = item->SpotDescription(SceneAtomEmpire());
							if(!item->Inscription().isEmpty())
								LocalLogMessage(tr("%1 displayed by %2. The inscription says, \"%3\"")
										.arg(whatItem)
										.arg(item->OwnerName())
										.arg(item->Inscription()));
							else
								LocalLogMessage(tr("%1 displayed by %2.")
										.arg(whatItem)
										.arg(item->OwnerName()));
						}
						else if(item->IsArmedDetonatorBomb() && !item->IsArmedBy(m_idSceneAtom))
						{
							AuId bombId(item->Id());
							UnlockSceneAtom();
							atom = 0;
							AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
							AuDefuseBomb defuse(bombId, pFrame);
							if(auApp->Frame()->ExecDialog(defuse) == QDialog::Accepted)
							{
								chValue = defuse.CutWireIndex();					// Attempt to defuse the bomb.
								LockSceneAtom(&sceneAtom);
								atom = m_SceneEther.FindAtom(bombId);			// Restore our pointer to the bomb atom.
							}
							else
								LockSceneAtom(&sceneAtom);
						}
					}
				}
			}
		}
	}
	if(atom)
	{
		DiControlInput control(button.m_AtomPanel, button.m_AtomPanelControlId);
		if(button.NeedsTarget())
			control.SetTarget(targetId, targetPart, targetShield, targetLoc, targetLevel);
		DiControlInputArray aActiveControls;
		aActiveControls.append(control);
		auApp->ProcessControlInput(button.m_AtomId, aActiveControls, gearPos, chValue, msElapsed);
	}
	UnlockSceneAtom();
}

void AuDesignerView::SetSelectedBd(const AuId& idBd)
{
	m_RenderInfo->SetSelectedBd(idBd);
}

// returns a DaDesignBase::DESIGNMODE
int AuDesignerView::DesignMode() const
{
	m_SceneInfoLock.lockForRead();
	int v = m_DesignMode;
	m_SceneInfoLock.unlock();
	return v;
}

// Returns true if in a DaDesignBase instance.
bool AuDesignerView::IsInDesignEnvironment() const
{
	return DesignMode() != DaDesignBase::InvalidMode;
}

// Returns true if in a DaDesigner instance.
bool AuDesignerView::IsInNewDesignEnvironment() const
{
	DaDesignBase::DESIGNMODE v = (DaDesignBase::DESIGNMODE)DesignMode();
	return v == DaDesignBase::DESIGN_Unknown ||
		v == DaDesignBase::DESIGN_Spacecraft ||
		v == DaDesignBase::DESIGN_Building ||
		v == DaDesignBase::DESIGN_Vehicle ||
		v == DaDesignBase::DESIGN_Item;
}

void AuDesignerView::DesignerVisibleFaces(AuScFaceList& faces)
{
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
			pde->VisibleFaces(faces);
	}
	UnlockSceneAtom();
}

BUILDING_STRUCTURE AuDesignerView::DesignBdStructure()
{
	BUILDING_STRUCTURE bd = BD_Invalid;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
			bd = pde->BuildingStructure();
	}
	UnlockSceneAtom();
	return bd;
}

void AuDesignerView::DesignerPaintInUse(QList<QColor>& paintInUse)
{
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
			pde->PaintInUse(paintInUse);
	}
	UnlockSceneAtom();
}

void AuDesignerView::DesignerViewPositionLoad1()
{
	DesignerViewPositionLoad(1);
}

void AuDesignerView::DesignerViewPositionLoad2()
{
	DesignerViewPositionLoad(2);
}

void AuDesignerView::DesignerViewPositionLoad3()
{
	DesignerViewPositionLoad(3);
}

void AuDesignerView::DesignerViewPositionLoad4()
{
	DesignerViewPositionLoad(4);
}

void AuDesignerView::DesignerViewPositionLoad5()
{
	DesignerViewPositionLoad(5);
}

void AuDesignerView::DesignerViewPositionLoad6()
{
	DesignerViewPositionLoad(6);
}

void AuDesignerView::DesignerViewPositionLoad7()
{
	DesignerViewPositionLoad(8);
}

void AuDesignerView::DesignerViewPositionLoad8()
{
	DesignerViewPositionLoad(8);
}

void AuDesignerView::DesignerViewPositionLoad9()
{
	DesignerViewPositionLoad(9);
}

void AuDesignerView::DesignerViewPositionSave1()
{
	DesignerViewPositionSave(1);
}

void AuDesignerView::DesignerViewPositionSave2()
{
	DesignerViewPositionSave(2);
}

void AuDesignerView::DesignerViewPositionSave3()
{
	DesignerViewPositionSave(3);
}

void AuDesignerView::DesignerViewPositionSave4()
{
	DesignerViewPositionSave(4);
}

void AuDesignerView::DesignerViewPositionSave5()
{
	DesignerViewPositionSave(5);
}

void AuDesignerView::DesignerViewPositionSave6()
{
	DesignerViewPositionSave(6);
}

void AuDesignerView::DesignerViewPositionSave7()
{
	DesignerViewPositionSave(7);
}

void AuDesignerView::DesignerViewPositionSave8()
{
	DesignerViewPositionSave(8);
}

void AuDesignerView::DesignerViewPositionSave9()
{
	DesignerViewPositionSave(9);
}

void AuDesignerView::DesignerViewPositionSave(int number)
{
	if(!IsInNewDesignEnvironment())
		return;
	auApp->DesignerViewPositionSave(SceneAtomId(), number);
}

void AuDesignerView::DesignerViewPositionLoad(int number)
{
	if(!IsInNewDesignEnvironment())
		return;
	auApp->DesignerViewPositionLoad(SceneAtomId(), number);
}

void AuDesignerView::DesignerViewPositionGrid()
{
	if(!IsInNewDesignEnvironment())
		return;
	AuPose pose(AuPose::Stand);
	float headTilt = 0;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		headTilt = sceneAtom->CurrentHeadTilt();
		pose = sceneAtom->CurrentGeneralPose();
	}
	UnlockSceneAtom();
	AuDesignPosition pos(m_DesignPlane, headTilt, pose);
	auApp->DesignerViewPositionGrid(SceneAtomId(), pos);
}

void AuDesignerView::DesignerViewPositionOrigin()
{
	if(!IsInNewDesignEnvironment())
		return;
	auApp->DesignerViewPositionOrigin(SceneAtomId());
}

QSet<quint8> AuDesignerView::DesignerViewPositions()
{
	QSet<quint8> restorableViews;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
			restorableViews = pde->ViewPositions();
	}
	UnlockSceneAtom();
	return restorableViews;
}

void AuDesignerView::DesignerGridPositionLoad1()
{
	DesignerGridPositionLoad(1);
}

void AuDesignerView::DesignerGridPositionLoad2()
{
	DesignerGridPositionLoad(2);
}

void AuDesignerView::DesignerGridPositionLoad3()
{
	DesignerGridPositionLoad(3);
}

void AuDesignerView::DesignerGridPositionLoad4()
{
	DesignerGridPositionLoad(4);
}

void AuDesignerView::DesignerGridPositionLoad5()
{
	DesignerGridPositionLoad(5);
}

void AuDesignerView::DesignerGridPositionLoad6()
{
	DesignerGridPositionLoad(6);
}

void AuDesignerView::DesignerGridPositionLoad7()
{
	DesignerGridPositionLoad(8);
}

void AuDesignerView::DesignerGridPositionLoad8()
{
	DesignerGridPositionLoad(8);
}

void AuDesignerView::DesignerGridPositionLoad9()
{
	DesignerGridPositionLoad(9);
}

void AuDesignerView::DesignerGridPositionSave1()
{
	DesignerGridPositionSave(1);
}

void AuDesignerView::DesignerGridPositionSave2()
{
	DesignerGridPositionSave(2);
}

void AuDesignerView::DesignerGridPositionSave3()
{
	DesignerGridPositionSave(3);
}

void AuDesignerView::DesignerGridPositionSave4()
{
	DesignerGridPositionSave(4);
}

void AuDesignerView::DesignerGridPositionSave5()
{
	DesignerGridPositionSave(5);
}

void AuDesignerView::DesignerGridPositionSave6()
{
	DesignerGridPositionSave(6);
}

void AuDesignerView::DesignerGridPositionSave7()
{
	DesignerGridPositionSave(7);
}

void AuDesignerView::DesignerGridPositionSave8()
{
	DesignerGridPositionSave(8);
}

void AuDesignerView::DesignerGridPositionSave9()
{
	DesignerGridPositionSave(9);
}

void AuDesignerView::DesignerGridPositionSave(int number)
{
	if(!IsInNewDesignEnvironment())
		return;
	auApp->DesignerGridPositionSave(number, m_DesignPlane);
}

void AuDesignerView::DesignerGridPositionLoad(int number)
{
	if(!IsInNewDesignEnvironment())
		return;
	// Design grid positioning is maintained in the view.
	//auApp->DesignerGridPositionLoad(SceneAtomId(), number);
	int result = 2;
	AuMatrix4x4 gridM;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
			result = pde->GridPosition(number, gridM);
	}
	UnlockSceneAtom();

	QString resultMsg;
	switch(result)
	{
	case 0: // ok
		SetDesignPlane(gridM);
		resultMsg = tr("Grid moved to blueprint grid position %1.").arg(number);
		break;
	case 1:
		resultMsg = tr("%1 is not a valid grid position number.").arg(number);
		break;
	case 2:
		resultMsg = tr("This blueprint has no grid position %1 to restore.").arg(number);
		break;
	case 3:
		resultMsg = tr("Grid position %1 was not restored because the position data is invalid.").arg(number);
		break;
	}
	if(!resultMsg.isEmpty())
		LocalLogMessage(resultMsg);
}

void AuDesignerView::DesignerGridPositionOrigin()
{
	if(!IsInNewDesignEnvironment())
		return;
	//auApp->DesignerGridPositionOrigin(SceneAtomId());
	SetDesignPlane(AuMatrix4x4());
	QString resultMsg = tr("Grid moved to the blueprint origin.");
	LocalLogMessage(resultMsg);
}

QSet<quint8> AuDesignerView::DesignerGridPositions()
{
	QSet<quint8> restorableViews;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
		if(pde)
			restorableViews = pde->GridPositions();
	}
	UnlockSceneAtom();
	return restorableViews;
}

bool AuDesignerView::FindClosestFencePost(AuPoint& postLoc, AuId& platformId)
{
	bool foundPost = false;
	DaPerson* sceneAtom;
	LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		AuPoint fLoc;
		DaBd* platform;
		DaWorldLL* world = FindTerrainLocationAtPixel(sceneAtom, m_MouseLast, fLoc, &platform);
		if(world)
		{
			if(platform)
				platformId = platform->Id();	// Find posts on a platform.
			else
				platformId = AuId(0);			// Find posts on the ground.
			foundPost = world->FindClosestFencePost(postLoc, fLoc, platformId);
		}
	}
	UnlockSceneAtom();
	return foundPost;
}

