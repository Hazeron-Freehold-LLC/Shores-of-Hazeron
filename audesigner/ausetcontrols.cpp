//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetcontrols.cpp,v 1.11 2022/02/19 18:46:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausetcontrols.h"
//
#include "auclientglobal.h"
#include "audesignerapp.h"
#include "audesignerframe.h"
#include "audesignerview.h"
#include "aujoystick.h"
#include "aupushbutton.h"
#include "daperson.h"
#include "davehicle.h"
#include "dpconsoleempty.h"
#include "dpconsoleengineer.h"
#include "dpconsolehelm.h"
#include "dpconsolenav.h"
#include "dpconsolepower.h"
#include "dpconsolesensor.h"
#include "dpconsoleshield.h"
#include "dpconsolesurgery.h"
#include "dpconsoletransporter.h"
#include "dpconsoleweapon.h"
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>

AuSetControls::AuSetControls(QWidget *parent) : QWidget(parent)
{
	// Control Bindings
	QLabel* lbcontrols = new QLabel(tr("<b>Control Bindings</b>"), this);
	lbcontrols->setAlignment(Qt::AlignCenter);

	QPushButton* pbChar = new QPushButton("Avatar Controls");
	pbChar->setToolTip(tr("<html>Configures controls that affect your avatar.</html>"));
	connect(pbChar, SIGNAL(clicked()), SLOT(CharacterControls()));

	QPushButton* pbScene = new QPushButton("Scene Controls");
	pbScene->setToolTip(tr("<html>Configures controls that affect the 3D scene window.</html>"));
	connect(pbScene, SIGNAL(clicked()), SLOT(SceneControls()));

	QPushButton* pbWindows = new QPushButton("Window Controls");
	pbWindows->setToolTip(tr("<html>Configures controls that affect the main program window.</html>"));
	connect(pbWindows, SIGNAL(clicked()), SLOT(WindowControls()));

	QLabel* lbvehicles = new QLabel(tr("Vehicles"), this);

	QPushButton* pbDirtBike = new QPushButton("Motor Bike Controls");
	pbDirtBike->setToolTip(tr("<html>Configures controls that affect the motor bike.</html>"));
	connect(pbDirtBike, SIGNAL(clicked()), SLOT(DirtBikeControls()));

	QPushButton* pbHelicopter = new QPushButton("Helicopter Controls");
	pbHelicopter->setToolTip(tr("<html>Configures controls that affect the helicopter.</html>"));
	connect(pbHelicopter, SIGNAL(clicked()), SLOT(HelicopterControls()));

	QPushButton* pbSailBoat = new QPushButton("Sail Boat Controls");
	pbSailBoat->setToolTip(tr("<html>Configures controls that affect the sail boat.</html>"));
	connect(pbSailBoat, SIGNAL(clicked()), SLOT(SailBoatControls()));

	QPushButton* pbSpaceFighter = new QPushButton("Space Fighter Controls");
	pbSpaceFighter->setToolTip(tr("<html>Configures controls that affect the space fighter.</html>"));
	connect(pbSpaceFighter, SIGNAL(clicked()), SLOT(SpaceFighterControls()));

	QPushButton* pbSpaceRocket = new QPushButton("Space Rocket Controls");
	pbSpaceRocket->setToolTip(tr("<html>Configures controls that affect the space rocket.</html>"));
	connect(pbSpaceRocket, SIGNAL(clicked()), SLOT(SpaceRocketControls()));

	QPushButton* pbSubmarine = new QPushButton("Submarine Controls");
	pbSubmarine->setToolTip(tr("<html>Configures controls that affect the submarine.</html>"));
	connect(pbSubmarine, SIGNAL(clicked()), SLOT(SubmarineControls()));

	QPushButton* pbSUV = new QPushButton("SUV Controls");
	pbSUV->setToolTip(tr("<html>Configures controls that affect the SUV.</html>"));
	connect(pbSUV, SIGNAL(clicked()), SLOT(SUVControls()));

	QLabel* lbcon = new QLabel(tr("Control Stations"), this);

	QPushButton* pbEng = new QPushButton("Engineer Station");
	pbEng->setToolTip(tr("<html>Configures controls that affect the engineer station.</html>"));
	connect(pbEng, SIGNAL(clicked()), SLOT(EngineerControls()));

	QPushButton* pbCommand = new QPushButton("Command Station");
	pbCommand->setToolTip(tr("<html>Configures controls that affect the Captain's chair and other command stations.</html>"));
	connect(pbCommand, SIGNAL(clicked()), SLOT(CommandControls()));

	QPushButton* pbHelm = new QPushButton("Helm Station");
	pbHelm->setToolTip(tr("<html>Configures controls that affect the helm station.</html>"));
	connect(pbHelm, SIGNAL(clicked()), SLOT(HelmControls()));

	QPushButton* pbNav = new QPushButton("Navigator Station");
	pbNav->setToolTip(tr("<html>Configures controls that affect the navigation station.</html>"));
	connect(pbNav, SIGNAL(clicked()), SLOT(NavigatorControls()));

	QPushButton* pbPower = new QPushButton("Power Relay Station");
	pbPower->setToolTip(tr("<html>Configures controls that affect the power relay station.</html>"));
	connect(pbPower, SIGNAL(clicked()), SLOT(PowerRelayControls()));

	QPushButton* pbSensor = new QPushButton("Sensor Station");
	pbSensor->setToolTip(tr("<html>Configures controls that affect the sensor station.</html>"));
	connect(pbSensor, SIGNAL(clicked()), SLOT(SensorControls()));

	QPushButton* pbShield = new QPushButton("Shield Station");
	pbShield->setToolTip(tr("<html>Configures controls that affect the shield station.</html>"));
	connect(pbShield, SIGNAL(clicked()), SLOT(ShieldControls()));

	QPushButton* pbSurgery = new QPushButton("Surgery Station");
	pbSurgery->setToolTip(tr("<html>Configures controls that affect the surgery station.</html>"));
	connect(pbSurgery, SIGNAL(clicked()), SLOT(SurgeryControls()));

	QPushButton* pbTrans = new QPushButton("Transporter Station");
	pbTrans->setToolTip(tr("<html>Configures controls that affect the transporter station.</html>"));
	connect(pbTrans, SIGNAL(clicked()), SLOT(TransporterControls()));

	QPushButton* pbFireControl = new QPushButton("Fire Control Station");
	pbFireControl->setToolTip(tr("<html>Configures controls that affect the fire control station.</html>"));
	connect(pbFireControl, SIGNAL(clicked()), SLOT(WeaponControls()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbcontrols, row,0, 1,2);
	row++;
	lay->addWidget(pbChar, row,0);
	lay->addWidget(pbScene, row,1);
	row++;
	lay->addWidget(pbWindows, row,1);
	row++;
	lay->addWidget(lbvehicles, row,0, 1,2);
	row++;
	lay->addWidget(pbHelicopter, row,0);
	lay->addWidget(pbDirtBike, row,1);
	row++;
	lay->addWidget(pbSailBoat, row,0);
	lay->addWidget(pbSpaceFighter, row,1);
	row++;
	lay->addWidget(pbSpaceRocket, row,0);
	lay->addWidget(pbSubmarine, row,1);
	row++;
	lay->addWidget(pbSUV, row,0);
	row++;
	lay->addWidget(lbcon, row,0, 1,2);
	row++;
	lay->addWidget(pbCommand, row,0);
	lay->addWidget(pbEng, row,1);
	row++;
	lay->addWidget(pbFireControl, row,0);
	lay->addWidget(pbHelm, row,1);
	row++;
	lay->addWidget(pbNav, row,0);
	lay->addWidget(pbPower, row,1);
	row++;
	lay->addWidget(pbSensor, row,0);
	lay->addWidget(pbShield, row,1);
	row++;
	lay->addWidget(pbSurgery, row,0);
	lay->addWidget(pbTrans, row,1);
	row++;

	// Mouse
	QLabel* lbmouse = new QLabel(tr("<b>Mouse</b>"));
	lbmouse->setAlignment(Qt::AlignCenter);
	lay->addWidget(lbmouse, row,0, 1,2);
	row++;

	QLabel* lbmouselook = new QLabel(tr("Mouse Look Speed"));
	lay->addWidget(lbmouselook, row,0, 1,2);
	row++;

	QSlider* mouseSlider = new QSlider(Qt::Horizontal, this);
	mouseSlider->setRange(1, 500);
	mouseSlider->setSingleStep(1);
	mouseSlider->setPageStep(5);
	mouseSlider->setValue(g_AuClient.MouseLookSensitivity());
	mouseSlider->setToolTip(tr("<html>Adjusts the speed of looking around using the mouse.</html>"));
	connect(mouseSlider, SIGNAL(valueChanged(int)), SLOT(SetMouseLookSensitivity(int)));
	lay->addWidget(mouseSlider, row,0, 1,2);
	row++;

	QLabel* lbmousesmoothing = new QLabel(tr("Mouse Look Smoothing"));
	lay->addWidget(lbmousesmoothing, row,0, 1,2);
	row++;

	QSlider* mouseSmoothingSlider = new QSlider(Qt::Horizontal, this);
	mouseSmoothingSlider->setRange(0, 500);
	mouseSmoothingSlider->setSingleStep(1);
	mouseSmoothingSlider->setPageStep(5);
	mouseSmoothingSlider->setValue(g_AuClient.ControlAverageSpanMs());
	mouseSmoothingSlider->setToolTip(tr("<html>Adjusts the amount of smoothing applied when looking around using the mouse. "
				"</html>"));
	connect(mouseSmoothingSlider, SIGNAL(valueChanged(int)), SLOT(SetMouseLookSmoothing(int)));
	lay->addWidget(mouseSmoothingSlider, row,0, 1,2);
	row++;

	QCheckBox* mouseGrab = new QCheckBox(tr("Grab Mouse"));
	mouseGrab->setCheckState(g_AuClient.MouseGrab()?Qt::Checked:Qt::Unchecked);
	mouseGrab->setToolTip(tr("<html>When checked, the mouse pointer is grabbed while using the mouse to look around. "
				"This prevents the mouse from escaping the window when moving quickly.<br>"
				"When unchecked, the mouse pointer is not grabbed during mouse look. "
				"The mouse may escape the window when moving quickly.<br>"
#if defined(Q_OS_MACOS)
				"<b>Mouse grab is not recommended on macOS. "
				"It causes problems with the visibility of the cursor.</b>"
#endif
				"</html>"));
	connect(mouseGrab, SIGNAL(stateChanged(int)), SLOT(SetMouseGrab(int)));
	lay->addWidget(mouseGrab, row,0, 1,2);
	row++;

	// Optional joystick info.
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	int joyCount = pFrame->Joystick()->Count();
	if(joyCount > 6)
		joyCount = 6;	// We only support up to six joysticks.
	if(pFrame->Joystick() && joyCount)
	{
		QLabel* lbjoy;
		if(joyCount > 1)
			lbjoy = new QLabel(tr("<b>Joysticks</b>"), this);
		else
			lbjoy = new QLabel(tr("<b>Joystick</b>"), this);
		lbjoy->setAlignment(Qt::AlignCenter);
		lay->addWidget(lbjoy, row,0, 1,2);
		row++;

		for(int joyIndex=0; joyIndex < joyCount; joyIndex++)
		{
			AuPushButtonValue* pbjoy = new AuPushButtonValue(tr("%1").arg(pFrame->Joystick()->Name(joyIndex)), joyIndex);
			pbjoy->setToolTip(tr("<html>Displays information about a joystick and enables/disables input "
						"from the joystick.</html>"));
			lay->addWidget(pbjoy, row++,0, 1,2);
			connect(pbjoy, SIGNAL(ClickedValue(int)), SLOT(ConfigureJoystick(int)));
		}
	}

	lay->setRowStretch(row, 1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuSetControls::~AuSetControls()
{
}

void AuSetControls::CharacterControls()
{
	DaPerson person;
	person.UpdateEnvironment();	// So it will know it's not in a vehicle or spacecraft.
	DiControlInputArray controls;
	person.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder("Avatar", controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			person.RestoreDefaultControls();
		else
			person.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::SceneControls()
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuDesignerView* view = pFrame->DesignerView();
	if(view)
	{
		DiControlInputArray controls;
		view->EnumControlInputs(controls);
		AuBindControls controlBinder("Scene", controls, pFrame, true);

		if(pFrame->ExecDialog(controlBinder) == QDialog::Accepted)
		{
			if(controlBinder.IsRestoringDefaults())
				view->RestoreDefaultControls();
			else
				view->ConfigureControls(controlBinder.Controls());
			emit ControlsChanged();
		}
	}
	else
	{
		// We must be at the login screen. Create a temporary view object to make settings.
		DiControlInputArray controls;
		AuDesignerView view;
		view.EnumControlInputs(controls);
		AuBindControls controlBinder("Scene", controls, pFrame, true);

		if(pFrame->ExecDialog(controlBinder) == QDialog::Accepted)
		{
			if(controlBinder.IsRestoringDefaults())
				view.RestoreDefaultControls();
			else
				view.ConfigureControls(controlBinder.Controls());
			emit ControlsChanged();
		}
	}
}

void AuSetControls::WindowControls()
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	DiControlInputArray controls;
	pFrame->EnumControlInputs(controls);
	AuBindControls controlBinder("Window", controls, pFrame, true, false, false);

	if(pFrame->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			pFrame->RestoreDefaultControls();
		else
			pFrame->ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::EngineerControls()
{
	DiControlInputArray controls;
	DpConsoleEngineer console(0);
	console.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder("Engineer Station", controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			console.RestoreDefaultControls();
		else
			console.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::CommandControls()
{
	DiControlInputArray controls;
	DpConsoleEmpty console;
	console.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder("Command Station", controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			console.RestoreDefaultControls();
		else
			console.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::HelmControls()
{
	DiControlInputArray controls;
	DpConsoleHelm console;
	console.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder("Helm Station", controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			console.RestoreDefaultControls();
		else
			console.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::NavigatorControls()
{
	DiControlInputArray controls;
	DpConsoleNav console;
	console.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder("Navigator Station", controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			console.RestoreDefaultControls();
		else
			console.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::PowerRelayControls()
{
	DiControlInputArray controls;
	DpConsolePower console;
	console.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder("Power Relay Station", controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			console.RestoreDefaultControls();
		else
			console.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::SensorControls()
{
	DiControlInputArray controls;
	DpConsoleSensor console;
	console.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder("Sensor Station", controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			console.RestoreDefaultControls();
		else
			console.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::ShieldControls()
{
	DiControlInputArray controls;
	DpConsoleShield console;
	console.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder("Shield Station", controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			console.RestoreDefaultControls();
		else
			console.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::SurgeryControls()
{
	DiControlInputArray controls;
	DpConsoleSurgery console(0);
	console.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder("Surgery Station", controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			console.RestoreDefaultControls();
		else
			console.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::TransporterControls()
{
	DiControlInputArray controls;
	DpConsoleTransporter console(0);
	console.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder("Transporter Station", controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			console.RestoreDefaultControls();
		else
			console.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::WeaponControls()
{
	DiControlInputArray controls;
	DpConsoleWeapon console(0);
	console.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder("Fire Control Station", controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			console.RestoreDefaultControls();
		else
			console.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::DirtBikeControls()
{
	DaVehicle vehicle(CID_DirtBike, 255);
	DiControlInputArray controls;
	vehicle.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder(vehicle.Name(), controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			vehicle.RestoreDefaultControls();
		else
			vehicle.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::SailBoatControls()
{
	DaVehicle vehicle(CID_SailBoat, 255);
	DiControlInputArray controls;
	vehicle.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder(vehicle.Name(), controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			vehicle.RestoreDefaultControls();
		else
			vehicle.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::SpaceRocketControls()
{
	DaVehicle vehicle(CID_SpaceRocket, 255);
	DiControlInputArray controls;
	vehicle.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder(vehicle.Name(), controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			vehicle.RestoreDefaultControls();
		else
			vehicle.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::SpaceFighterControls()
{
	DaVehicle vehicle(CID_SpaceFighter, 255);
	DiControlInputArray controls;
	vehicle.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder(vehicle.Name(), controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			vehicle.RestoreDefaultControls();
		else
			vehicle.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::SubmarineControls()
{
	DaVehicle vehicle(CID_Submarine, 255);
	DiControlInputArray controls;
	vehicle.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder(vehicle.Name(), controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			vehicle.RestoreDefaultControls();
		else
			vehicle.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::HelicopterControls()
{
	DaVehicle vehicle(CID_Helicopter, 255);
	DiControlInputArray controls;
	vehicle.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder(vehicle.Name(), controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			vehicle.RestoreDefaultControls();
		else
			vehicle.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::SUVControls()
{
	DaVehicle vehicle(CID_SUV, 255);
	DiControlInputArray controls;
	vehicle.EnumControlInputs(controls);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindControls controlBinder(vehicle.Name(), controls, pFrame);

	if(auApp->Frame()->ExecDialog(controlBinder) == QDialog::Accepted)
	{
		if(controlBinder.IsRestoringDefaults())
			vehicle.RestoreDefaultControls();
		else
			vehicle.ConfigureControls(controlBinder.Controls());
		emit ControlsChanged();
	}
}

void AuSetControls::ConfigureJoystick(int joyIndex)
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuConfigureJoystick joy(joyIndex, pFrame);
	auApp->Frame()->ExecDialog(joy);
}

void AuSetControls::SetMouseLookSensitivity(int mls)
{
	g_AuClient.SetMouseLookSensitivity(mls);
}

void AuSetControls::SetMouseLookSmoothing(int mls)
{
	g_AuClient.SetControlAverageSpanMs(mls);
}

void AuSetControls::SetMouseGrab(int grab)
{
	g_AuClient.SetMouseGrab(grab);
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBindControls::AuBindControls(const QString& name, const DiControlInputArray& controls, QWidget* parent, bool doTitles,
		bool mouseOk, bool joystickOk) : AuDialog(parent)
{
	m_Controls = controls;
	m_RestoreDefaults = false;

	setAuDlgIcon(QIcon(":data/Settings.png"));
	setAuDlgTitle(QString("<html><b>%1 Controls</b></html>").arg(name));

	QWidget* buttons = new QWidget;

	QLabel* lbctlname = new QLabel(tr("<html><b>Control</b><br>"
				"* <i>Denotes Tip</i></html>"));

	QLabel* lbctlkey = new QLabel(tr("<html><b>Keyboard</b></html>"));

	QLabel* lbctlmouse;
	QLabel* lbctlwheel = new QLabel(tr("<html><b>Wheel</b></html>"));
	if(mouseOk)
	{
		lbctlmouse = new QLabel(tr("<html><b>Mouse</b></html>"));
		lbctlwheel = new QLabel(tr("<html><b>Wheel</b></html>"));
	}
	else
	{
		lbctlmouse = 0;
		lbctlwheel = 0;
	}

	QLabel* lbctljoy;
	if(joystickOk)
		lbctljoy = new QLabel(tr("<html><b>Joystick</b></html>"));
	else
		lbctljoy = 0;

	QGridLayout* slay = new QGridLayout(buttons);
	int row=0;

	// Header line.
	int col=0;
	slay->addWidget(lbctlname, row,col++);
	slay->setColumnMinimumWidth(col++, 4);
	slay->addWidget(lbctlkey, row,col++);
	slay->setColumnMinimumWidth(col++, 4);
	if(lbctlmouse)
	{
		slay->addWidget(lbctlmouse, row,col++);
		slay->setColumnMinimumWidth(col++, 4);
	}
	if(lbctlwheel)
	{
		slay->addWidget(lbctlwheel, row,col++);
		slay->setColumnMinimumWidth(col++, 4);
	}
	if(lbctljoy)
		slay->addWidget(lbctljoy, row,col++);
	row++;

	// Control Bindings
	for(int ic=0; ic<controls.count(); ic++)
	{
		col=0;
		if(doTitles && controls[ic].GetId() == 0)
		{
			// This is a title entry.
			if(controls[ic].Description().isEmpty())
				lbctlname = new QLabel(tr("<html><br>%1").arg(controls[ic].Name()));
			else
			{
				lbctlname = new QLabel(tr("<br>%1 *").arg(controls[ic].Name()));
				lbctlname->setToolTip(tr("<html><b>%1</b><br>%2</html>")
						.arg(controls[ic].Name())
						.arg(controls[ic].Description()));
			}

			slay->addWidget(lbctlname, row,col, 1,7);
			row++;
			continue;
		}

		if(controls[ic].Description().isEmpty())
			lbctlname = new QLabel(controls[ic].Name());
		else
		{
			lbctlname = new QLabel(tr("%1 *").arg(controls[ic].Name()));
			lbctlname->setToolTip(tr("<html><b>%1</b><br>%2</html>")
					.arg(controls[ic].Name())
					.arg(controls[ic].Description()));
		}

		slay->addWidget(lbctlname, row,col);
		col+=2;

		if(!controls[ic].JoystickAxisExclusive())
		{
			// The control is not exclusive to a joystick axis. Allow mapping of keys, mice, and wheel.
			QString keyname;
			Qt::KeyboardModifiers keymod = controls[ic].KeyModifier();
			Qt::Key key = controls[ic].Key();
			if(key != Qt::Key_unknown)
			{
				keyname = ModifierName(keymod);
				keyname += KeyName(key);
			}
			AuPushButtonValue* pbkey = new AuPushButtonValue(keyname, ic);
			pbkey->setObjectName(QString("pbkey%1").arg(ic));
			pbkey->setToolTip(tr("<html>Configures the key that will activate the <i>%1</i> control.</html>")
					.arg(controls[ic].Name()));
			connect(pbkey, SIGNAL(ClickedValue(int)), SLOT(BindKey(int)));
			slay->addWidget(pbkey, row,col);
			col+=2;

			if(mouseOk)
			{
				QString mousename;
				Qt::KeyboardModifiers mousemod = controls[ic].MouseModifier();
				DiControlInput::MOUSETRIGGER mouse = controls[ic].Mouse();
				if(mouse != DiControlInput::MT_None)
				{
					mousename = ModifierName(mousemod);

					switch(mouse)
					{
					case DiControlInput::MT_None:
						break;
					case DiControlInput::MT_Left:
						mousename += tr("Move Left");
						break;
					case DiControlInput::MT_Right:
						mousename += tr("Move Right");
						break;
					case DiControlInput::MT_Up:
						mousename += tr("Move Up");
						break;
					case DiControlInput::MT_Down:
						mousename += tr("Move Down");
						break;
					case DiControlInput::MT_ButtonLeft:
						mousename += tr("Left Button");
						break;
					case DiControlInput::MT_ButtonMiddle:
						mousename += tr("Middle Button");
						break;
					case DiControlInput::MT_ButtonRight:
						mousename += tr("Right Button");
						break;
					}
				}
				AuPushButtonValue* pbmouse = new AuPushButtonValue(mousename, ic);
				pbmouse->setObjectName(QString("pbmouse%1").arg(ic));
				pbmouse->setToolTip(tr("<html>Configures the mouse action that will activate the <i>%1</i> control.</html>")
						.arg(controls[ic].Name()));
				connect(pbmouse, SIGNAL(ClickedValue(int)), SLOT(BindMouse(int)));
				slay->addWidget(pbmouse, row,col);
				col+=2;

				QString wheelname;
				DiControlInput::WHEELTRIGGER wheel = controls[ic].MouseWheel();
				if(wheel != DiControlInput::WT_None)
				{
					switch(wheel)
					{
					case DiControlInput::WT_None:
						break;
					case DiControlInput::WT_AwayFromUser:
						wheelname = tr("Away From User");
						break;
					case DiControlInput::WT_TowardUser:
						wheelname = tr("Toward User");
						break;
					}
				}
				AuPushButtonValue* pbwheel = new AuPushButtonValue(wheelname, ic);
				pbwheel->setObjectName(QString("pbwheel%1").arg(ic));
				pbwheel->setToolTip(tr("<html>Configures the wheel action that will activate the <i>%1</i> control.</html>")
						.arg(controls[ic].Name()));
				connect(pbwheel, SIGNAL(ClickedValue(int)), SLOT(BindWheel(int)));
				slay->addWidget(pbwheel, row,col);
				col+=2;
			}
			else
				col += 4; 	// Skip to joystick column.
		}
		else
			col += 6; 	// Skip to joystick column.

		if(joystickOk)
		{
			QString joyAction;
			DiControlInput::JOYTRIGGER joy = controls[ic].Joystick();
			switch(joy)
			{
			case DiControlInput::JT_None:
				break;
			case DiControlInput::JT_Axis:
				joyAction = tr("Joystick %1 Axis %2")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_AxisInvert:
				joyAction = tr("Joystick %1 Axis %2 Inverted")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_AxisNegative:
				joyAction = tr("Joystick %1 Axis %2 Negative")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_AxisPositive:
				joyAction = tr("Joystick %1 Axis %2 Positive")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_Button:
				joyAction = tr("Joystick %1 Button %2")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_BallXNegative:
				joyAction = tr("Joystick %1 Ball %2 X Negative")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_BallXPositive:
				joyAction = tr("Joystick %1 Ball %2 X Positive")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_BallYNegative:
				joyAction = tr("Joystick %1 Ball %2 Y Negative")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_BallYPositive:
				joyAction = tr("Joystick %1 Ball %2 Y Positive")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_HatCentered:
				joyAction = tr("Joystick %1 Hat %2 Centered")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_HatDown:
				joyAction = tr("Joystick %1 Hat %2 Down")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_HatLeft:
				joyAction = tr("Joystick %1 Hat %2 Left")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_HatLeftDown:
				joyAction = tr("Joystick %1 Hat %2 Left Down")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_HatLeftUp:
				joyAction = tr("Joystick %1 Hat %2 Left Up")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_HatRight:
				joyAction = tr("Joystick %1 Hat %2 Right")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_HatRightDown:
				joyAction = tr("Joystick %1 Hat %2 Right Down")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_HatRightUp:
				joyAction = tr("Joystick %1 Hat %2 Right Up")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			case DiControlInput::JT_HatUp:
				joyAction = tr("Joystick %1 Hat %2 Up")
					.arg(controls[ic].JoystickIndex()+1)
					.arg(controls[ic].JoystickSubIndex()+1);
				break;
			}
			AuPushButtonValue* pbjoy = new AuPushButtonValue(joyAction, ic);
			pbjoy->setObjectName(QString("pbjoy%1").arg(ic));
			pbjoy->setToolTip(tr("<html>Configures the joystick action that will activate the <i>%1</i> control.</html>")
					.arg(controls[ic].Name()));
			connect(pbjoy, SIGNAL(ClickedValue(int)), SLOT(BindJoystick(int)));
			slay->addWidget(pbjoy, row,col);
			col+=2;
		}

		row++;
	}
	slay->setMargin(2);
	slay->setSpacing(2);
	buttons->setLayout(slay);

	QScrollArea* scroller = new QScrollArea;
	scroller->setWidget(buttons);

	QGridLayout* lay = new QGridLayout;
	lay->addWidget(scroller, row,0, 1,2);
	row++;

	QPushButton* pbrestore = new QPushButton("Restore Defaults");
	pbrestore->setToolTip(tr("<html>Returns the control bindings to their default settings.</html>"));
	connect(pbrestore, SIGNAL(clicked()), SLOT(RestoreDefaults()));
	lay->addWidget(pbrestore, row,0);

	QPushButton* pbok = new QPushButton("Save Changes");
	pbok->setToolTip(tr("<html>Saves your changes to the control bindings.</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(SaveChanges()));
	lay->addWidget(pbok, row,1);

	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuBindControls::~AuBindControls()
{
}

void AuBindControls::RestoreDefaults()
{
	m_RestoreDefaults = true;
	accept();
}

void AuBindControls::SaveChanges()
{
	m_RestoreDefaults = false;
	accept();
}

//static
QString AuBindControls::KeyName(Qt::Key key)
{
	return AuKeyFilter::KeyName(key);
}

//static
QString AuBindControls::ModifierName(Qt::KeyboardModifiers modifiers)
{
	return AuKeyFilter::ModifierName(modifiers);
}

void AuBindControls::BindKey(int ctrlIdx)
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindKey bindKey(m_Controls[ctrlIdx].Name(), m_Controls[ctrlIdx].Key(), m_Controls[ctrlIdx].KeyModifier(), pFrame);
	if(auApp->Frame()->ExecDialog(bindKey) == QDialog::Accepted)
	{
		// Update the controls array.
		m_Controls[ctrlIdx].SetKey(bindKey.Key(), bindKey.Modifier());

		// Update the button text.
		QString keyname;
		if(m_Controls[ctrlIdx].Key() != Qt::Key_unknown)
		{
			keyname = ModifierName(m_Controls[ctrlIdx].KeyModifier());
			keyname += KeyName(m_Controls[ctrlIdx].Key());
		}
		QPushButton* pbkey = findChild<QPushButton*>(QString("pbkey%1").arg(ctrlIdx));
		pbkey->setText(keyname);
	}
}

void AuBindControls::BindMouse(int ctrlIdx)
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindMouse bindMouse(m_Controls[ctrlIdx].Name(), m_Controls[ctrlIdx].Mouse(), m_Controls[ctrlIdx].MouseModifier(), pFrame);
	if(auApp->Frame()->ExecDialog(bindMouse) == QDialog::Accepted)
	{
		// Update the controls array.
		m_Controls[ctrlIdx].SetMouse(bindMouse.Mouse(), bindMouse.Modifier());

		// Update the button text.
		QString mousename;
		if(m_Controls[ctrlIdx].Mouse() != DiControlInput::MT_None)
		{
			mousename = ModifierName(m_Controls[ctrlIdx].MouseModifier());

			switch(m_Controls[ctrlIdx].Mouse())
			{
			case DiControlInput::MT_None:
				break;
			case DiControlInput::MT_Left:
				mousename += tr("Move Left");
				break;
			case DiControlInput::MT_Right:
				mousename += tr("Move Right");
				break;
			case DiControlInput::MT_Up:
				mousename += tr("Move Up");
				break;
			case DiControlInput::MT_Down:
				mousename += tr("Move Down");
				break;
			case DiControlInput::MT_ButtonLeft:
				mousename += tr("Left Button");
				break;
			case DiControlInput::MT_ButtonMiddle:
				mousename += tr("Middle Button");
				break;
			case DiControlInput::MT_ButtonRight:
				mousename += tr("Right Button");
				break;
			}
		}
		QPushButton* pbmouse = findChild<QPushButton*>(QString("pbmouse%1").arg(ctrlIdx));
		pbmouse->setText(mousename);
	}
}

void AuBindControls::BindWheel(int ctrlIdx)
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindWheel bindWheel(m_Controls[ctrlIdx].Name(), m_Controls[ctrlIdx].MouseWheel(), pFrame);
	if(auApp->Frame()->ExecDialog(bindWheel) == QDialog::Accepted)
	{
		// Update the controls array.
		m_Controls[ctrlIdx].SetWheel(bindWheel.Wheel());

		// Update the button text.
		QString wheelname;
		if(m_Controls[ctrlIdx].MouseWheel() != DiControlInput::WT_None)
		{
			switch(m_Controls[ctrlIdx].MouseWheel())
			{
			case DiControlInput::WT_None:
				break;
			case DiControlInput::WT_AwayFromUser:
				wheelname = tr("Away From User");
				break;
			case DiControlInput::WT_TowardUser:
				wheelname = tr("Toward User");
				break;
			}
		}
		QPushButton* pbwheel = findChild<QPushButton*>(QString("pbwheel%1").arg(ctrlIdx));
		pbwheel->setText(wheelname);
	}
}

void AuBindControls::BindJoystick(int ctrlIdx)
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuBindJoystick bindJoy(m_Controls[ctrlIdx].Name(), m_Controls[ctrlIdx].Joystick(), m_Controls[ctrlIdx].JoystickIndex(),
			m_Controls[ctrlIdx].JoystickSubIndex(), m_Controls[ctrlIdx].JoystickAxisExclusive(), pFrame);
	if(auApp->Frame()->ExecDialog(bindJoy) == QDialog::Accepted)
	{
		// Update the controls array.
		m_Controls[ctrlIdx].SetJoystick(bindJoy.JoyAction(), bindJoy.JoyIndex(), bindJoy.JoySubIndex());

		// Update the button text.
		QString joyAction;
		switch(m_Controls[ctrlIdx].Joystick())
		{
		case DiControlInput::JT_None:
			break;
		case DiControlInput::JT_Axis:
			joyAction = tr("Joystick %1 Axis %2")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_AxisInvert:
			joyAction = tr("Joystick %1 Axis %2 Inverted")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_AxisNegative:
			joyAction = tr("Joystick %1 Axis %2 Negative")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_AxisPositive:
			joyAction = tr("Joystick %1 Axis %2 Positive")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_Button:
			joyAction = tr("Joystick %1 Button %2")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_BallXNegative:
			joyAction = tr("Joystick %1 Ball %2 X Negative")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_BallXPositive:
			joyAction = tr("Joystick %1 Ball %2 X Positive")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_BallYNegative:
			joyAction = tr("Joystick %1 Ball %2 Y Negative")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_BallYPositive:
			joyAction = tr("Joystick %1 Ball %2 Y Positive")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_HatCentered:
			joyAction = tr("Joystick %1 Hat %2 Centered")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_HatDown:
			joyAction = tr("Joystick %1 Hat %2 Down")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_HatLeft:
			joyAction = tr("Joystick %1 Hat %2 Left")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_HatLeftDown:
			joyAction = tr("Joystick %1 Hat %2 Left Down")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_HatLeftUp:
			joyAction = tr("Joystick %1 Hat %2 Left Up")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_HatRight:
			joyAction = tr("Joystick %1 Hat %2 Right")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_HatRightDown:
			joyAction = tr("Joystick %1 Hat %2 Right Down")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_HatRightUp:
			joyAction = tr("Joystick %1 Hat %2 Right Up")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		case DiControlInput::JT_HatUp:
			joyAction = tr("Joystick %1 Hat %2 Up")
				.arg(m_Controls[ctrlIdx].JoystickIndex()+1)
				.arg(m_Controls[ctrlIdx].JoystickSubIndex()+1);
			break;
		}
		QPushButton* pbjoy = findChild<QPushButton*>(QString("pbjoy%1").arg(ctrlIdx));
		pbjoy->setText(joyAction);
	}
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBindKey::AuBindKey(const QString& action, Qt::Key key, Qt::KeyboardModifiers keyMod, QWidget* parent) : AuDialog(parent)
{
	m_GrabInput = false;
	m_Key = key;
	m_KeyMod = keyMod;
	setAuDlgIcon(QIcon(":data/Settings.png"));
	setAuDlgTitle(tr("<b>%1</b>").arg(action));

	QGridLayout* lay = new QGridLayout;
	int row=0;

	QLabel* lbmods = new QLabel(tr("Modifier"), this);
	lay->addWidget(lbmods, row++,0);

	QCheckBox* rbmod1 = new QCheckBox(tr("Alt"));
	rbmod1->setObjectName("rbmod1");
	lay->addWidget(rbmod1, row++,0);
	if(keyMod & Qt::AltModifier)
		rbmod1->setChecked(true);

	QCheckBox* rbmod2 = new QCheckBox(tr("Ctrl"));
	rbmod2->setObjectName("rbmod2");
	lay->addWidget(rbmod2, row++,0);
	if(keyMod & Qt::ControlModifier)
		rbmod2->setChecked(true);

	QCheckBox* rbmod3 = new QCheckBox(tr("Meta"));
	rbmod3->setObjectName("rbmod3");
	lay->addWidget(rbmod3, row++,0);
	if(keyMod & Qt::MetaModifier)
		rbmod3->setChecked(true);

	QCheckBox* rbmod4 = new QCheckBox(tr("Shift"));
	rbmod4->setObjectName("rbmod4");
	lay->addWidget(rbmod4, row++,0);
	if(keyMod & Qt::ShiftModifier)
		rbmod4->setChecked(true);

	QLabel* lbkey = new QLabel(tr("Key"), this);
	lay->addWidget(lbkey, row++,0);

	QComboBox* cbkey = new QComboBox(this);
	cbkey->setObjectName("cbkey");
	cbkey->setToolTip(tr("<html>Selects a key that will activate the control.</html>"));
	lay->addWidget(cbkey, row++,0);

	// Add characters
	for(Qt::Key addkey = Qt::Key_Exclam; addkey <= Qt::Key_QuoteLeft; addkey = (Qt::Key)(addkey +1))
		cbkey->addItem(QString(" %1").arg(QChar(addkey)), addkey);
	for(Qt::Key addkey = Qt::Key_BraceLeft; addkey <= Qt::Key_AsciiTilde; addkey = (Qt::Key)(addkey +1))
		cbkey->addItem(QString(" %1").arg(QChar(addkey)), addkey);

	// Add special keys.
	cbkey->addItem("Escape", Qt::Key_Escape);
	cbkey->addItem("Tab", Qt::Key_Tab);
	cbkey->addItem("Backtab", Qt::Key_Backtab);
	cbkey->addItem("Backspace", Qt::Key_Backspace);
	cbkey->addItem("Return", Qt::Key_Return);
	cbkey->addItem("Enter", Qt::Key_Enter);
	cbkey->addItem("Insert", Qt::Key_Insert);
	cbkey->addItem("Delete", Qt::Key_Delete);
	cbkey->addItem("Pause", Qt::Key_Pause);
	cbkey->addItem("Print", Qt::Key_Print);
	cbkey->addItem("SysReq", Qt::Key_SysReq);
	cbkey->addItem("Clear", Qt::Key_Clear);
	cbkey->addItem("Home", Qt::Key_Home);
	cbkey->addItem("End", Qt::Key_End);
	cbkey->addItem("Left Arrow", Qt::Key_Left);
	cbkey->addItem("Up Arrow", Qt::Key_Up);
	cbkey->addItem("Right Arrow", Qt::Key_Right);
	cbkey->addItem("Down Arrow", Qt::Key_Down);
	cbkey->addItem("Page Up", Qt::Key_PageUp);
	cbkey->addItem("Page Down", Qt::Key_PageDown);
	cbkey->addItem("Shift", Qt::Key_Shift);
	cbkey->addItem("Control", Qt::Key_Control);
	cbkey->addItem("Meta", Qt::Key_Meta);
	cbkey->addItem("Alt", Qt::Key_Alt);
	cbkey->addItem("AltGr", Qt::Key_AltGr);
	cbkey->addItem("Caps Lock", Qt::Key_CapsLock);
	cbkey->addItem("Num Lock", Qt::Key_NumLock);
	cbkey->addItem("Scroll Lock", Qt::Key_ScrollLock);
	cbkey->addItem("F1", Qt::Key_F1);
	cbkey->addItem("F2", Qt::Key_F2);
	cbkey->addItem("F3", Qt::Key_F3);
	cbkey->addItem("F4", Qt::Key_F4);
	cbkey->addItem("F5", Qt::Key_F5);
	cbkey->addItem("F6", Qt::Key_F6);
	cbkey->addItem("F7", Qt::Key_F7);
	cbkey->addItem("F8", Qt::Key_F8);
	cbkey->addItem("F9", Qt::Key_F9);
	cbkey->addItem("F10", Qt::Key_F10);
	cbkey->addItem("F11", Qt::Key_F11);
	cbkey->addItem("F12", Qt::Key_F12);
	cbkey->addItem("Menu", Qt::Key_Menu);
	cbkey->addItem("Space", Qt::Key_Space);

	int icb = cbkey->findData(key);
	if(icb < 0)
	{
		// Current key mapping was not found. Add it as a new item. Cyrillic keyboard?
		cbkey->addItem(QString("Key 0x%1").arg(key, 0, 16), key);
		icb = cbkey->findData(key);
	}
	cbkey->setCurrentIndex(icb);

	QPushButton* pbgrab = new QPushButton("Grab Input");
	pbgrab->setObjectName("pbgrab");
	pbgrab->setToolTip(tr("<html>Monitors the keyboard for input that will be used for this control.<br>"
				"Press <b>Grab Input</b> and then press the key you wish to bind to the control. To include a modifier, hold "
				"Alt, Ctrl, Meta, or Shift while pressing a key.</html>"));
	connect(pbgrab, SIGNAL(clicked()), SLOT(GrabInput()));
	lay->addWidget(pbgrab, row++,0);

	QPushButton* pbunbind = new QPushButton("Unbind Key");
	pbunbind->setToolTip(tr("<html>Removes the key binding from the control. "
				"The control will no longer be activated by a key.</html>"));
	connect(pbunbind, SIGNAL(clicked()), SLOT(Unbind()));
	lay->addWidget(pbunbind, row++,0);

	QPushButton* pbok = new QPushButton("Ok");
	pbok->setToolTip(tr("<html>Binds the selected key to the control.</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(OkPressed()));
	lay->addWidget(pbok, row++,0);

	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuBindKey::~AuBindKey()
{
}

void AuBindKey::GrabInput()
{
	m_GrabInput = true;
	QPushButton* pbgrab = findChild<QPushButton*>("pbgrab");
	pbgrab->setDown(true);
}

void AuBindKey::keyReleaseEvent(QKeyEvent* e)
{
	if(m_GrabInput)
	{
		m_GrabInput = false;
		QPushButton* pbgrab = findChild<QPushButton*>("pbgrab");
		pbgrab->setDown(false);
		int key = e->key();

		QComboBox* cbkey = findChild<QComboBox*>("cbkey");
		int icb = cbkey->findData(key);
		if(icb < 0)
		{
			// Key mapping was not found. Add it as a new item. Cyrillic keyboard?
			cbkey->addItem(QString("Key 0x%1").arg(key, 0, 16), key);
			icb = cbkey->findData(key);
		}
		cbkey->setCurrentIndex(icb);

		// Set the modifier
		if(e->modifiers() & Qt::AltModifier)
		{
			QCheckBox* rbmod1 = findChild<QCheckBox*>("rbmod1");
			rbmod1->setChecked(true);
		}
		if(e->modifiers() & Qt::ControlModifier)
		{
			QCheckBox* rbmod2 = findChild<QCheckBox*>("rbmod2");
			rbmod2->setChecked(true);
		}
		if(e->modifiers() & Qt::MetaModifier)
		{
			QCheckBox* rbmod3 = findChild<QCheckBox*>("rbmod3");
			rbmod3->setChecked(true);
		}
		if(e->modifiers() & Qt::ShiftModifier)
		{
			QCheckBox* rbmod4 = findChild<QCheckBox*>("rbmod4");
			rbmod4->setChecked(true);
		}
	}
	else
		AuDialog::keyPressEvent(e);
}

void AuBindKey::OkPressed()
{
	QCheckBox* rbmod1 = findChild<QCheckBox*>("rbmod1");
	QCheckBox* rbmod2 = findChild<QCheckBox*>("rbmod2");
	QCheckBox* rbmod3 = findChild<QCheckBox*>("rbmod3");
	QCheckBox* rbmod4 = findChild<QCheckBox*>("rbmod4");

	m_KeyMod = Qt::NoModifier;
	if(rbmod1->isChecked())
		m_KeyMod = (Qt::KeyboardModifiers)(m_KeyMod|Qt::AltModifier);
	if(rbmod2->isChecked())
		m_KeyMod = (Qt::KeyboardModifiers)(m_KeyMod|Qt::ControlModifier);
	if(rbmod3->isChecked())
		m_KeyMod = (Qt::KeyboardModifiers)(m_KeyMod|Qt::MetaModifier);
	if(rbmod4->isChecked())
		m_KeyMod = (Qt::KeyboardModifiers)(m_KeyMod|Qt::ShiftModifier);

	QComboBox* cbkey = findChild<QComboBox*>("cbkey");
	m_Key = (Qt::Key)cbkey->itemData(cbkey->currentIndex()).toInt();

	accept();
}

void AuBindKey::Unbind()
{
	m_Key = Qt::Key_unknown;
	m_KeyMod = Qt::NoModifier;

	accept();
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBindMouse::AuBindMouse(const QString& action, DiControlInput::MOUSETRIGGER mouse, Qt::KeyboardModifiers mouseMod, QWidget* parent) :
	AuDialog(parent)
{
	m_Mouse = mouse;
	m_MouseMod = mouseMod;
	setAuDlgIcon(QIcon(":data/Settings.png"));
	setAuDlgTitle(tr("<b>%1</b>").arg(action));

	QGridLayout* lay = new QGridLayout;
	int row=0;

	QLabel* lbmods = new QLabel(tr("Modifier"), this);
	lay->addWidget(lbmods, row++,0);

	QCheckBox* rbmod1 = new QCheckBox(tr("Alt"));
	rbmod1->setObjectName("rbmod1");
	lay->addWidget(rbmod1, row++,0);
	if(mouseMod & Qt::AltModifier)
		rbmod1->setChecked(true);

	QCheckBox* rbmod2 = new QCheckBox(tr("Ctrl"));
	rbmod2->setObjectName("rbmod2");
	lay->addWidget(rbmod2, row++,0);
	if(mouseMod & Qt::ControlModifier)
		rbmod2->setChecked(true);

	QCheckBox* rbmod3 = new QCheckBox(tr("Meta"));
	rbmod3->setObjectName("rbmod3");
	lay->addWidget(rbmod3, row++,0);
	if(mouseMod & Qt::MetaModifier)
		rbmod3->setChecked(true);

	QCheckBox* rbmod4 = new QCheckBox(tr("Shift"));
	rbmod4->setObjectName("rbmod4");
	lay->addWidget(rbmod4, row++,0);
	if(mouseMod & Qt::ShiftModifier)
		rbmod4->setChecked(true);

	QLabel* lbkey = new QLabel(tr("Mouse"), this);
	lay->addWidget(lbkey, row++,0);

	QComboBox* cbmouse = new QComboBox(this);
	cbmouse->setObjectName("cbmouse");
	cbmouse->setToolTip(tr("<html>Selects a mouse action that will activate the control.</html>"));
	lay->addWidget(cbmouse, row++,0);

	cbmouse->addItem("Move Left", DiControlInput::MT_Left);
	cbmouse->addItem("Move Right", DiControlInput::MT_Right);
	cbmouse->addItem("Move Up", DiControlInput::MT_Up);
	cbmouse->addItem("Move Down", DiControlInput::MT_Down);
	cbmouse->addItem("Left Button", DiControlInput::MT_ButtonLeft);
	cbmouse->addItem("Middle Button", DiControlInput::MT_ButtonMiddle);
	cbmouse->addItem("Right Button", DiControlInput::MT_ButtonRight);

	int icb = cbmouse->findData(mouse);
	cbmouse->setCurrentIndex(icb);

	QPushButton* pbunbind = new QPushButton("Unbind Mouse");
	pbunbind->setToolTip(tr("<html>Removes the mouse binding from the control. "
				"The control will no longer be activated by a mouse action.</html>"));
	connect(pbunbind, SIGNAL(clicked()), SLOT(Unbind()));
	lay->addWidget(pbunbind, row++,0);

	QPushButton* pbok = new QPushButton("Ok");
	pbok->setToolTip(tr("<html>Binds the selected mouse action to the control.</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(OkPressed()));
	lay->addWidget(pbok, row++,0);

	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuBindMouse::~AuBindMouse()
{
}

void AuBindMouse::OkPressed()
{
	QCheckBox* rbmod1 = findChild<QCheckBox*>("rbmod1");
	QCheckBox* rbmod2 = findChild<QCheckBox*>("rbmod2");
	QCheckBox* rbmod3 = findChild<QCheckBox*>("rbmod3");
	QCheckBox* rbmod4 = findChild<QCheckBox*>("rbmod4");
	m_MouseMod = Qt::NoModifier;
	if(rbmod1->isChecked())
		m_MouseMod = (Qt::KeyboardModifiers)(m_MouseMod|Qt::AltModifier);
	if(rbmod2->isChecked())
		m_MouseMod = (Qt::KeyboardModifiers)(m_MouseMod|Qt::ControlModifier);
	if(rbmod3->isChecked())
		m_MouseMod = (Qt::KeyboardModifiers)(m_MouseMod|Qt::MetaModifier);
	if(rbmod4->isChecked())
		m_MouseMod = (Qt::KeyboardModifiers)(m_MouseMod|Qt::ShiftModifier);

	QComboBox* cbmouse = findChild<QComboBox*>("cbmouse");
	m_Mouse = (DiControlInput::MOUSETRIGGER)cbmouse->itemData(cbmouse->currentIndex()).toInt();

	accept();
}

void AuBindMouse::Unbind()
{
	m_Mouse = DiControlInput::MT_None;
	m_MouseMod = Qt::NoModifier;

	accept();
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBindWheel::AuBindWheel(const QString& action, DiControlInput::WHEELTRIGGER mouse, QWidget* parent) : AuDialog(parent)
{
	m_Wheel = mouse;
	setAuDlgIcon(QIcon(":data/Settings.png"));
	setAuDlgTitle(tr("<b>%1</b>").arg(action));

	QGridLayout* lay = new QGridLayout;
	int row=0;

	QLabel* lbkey = new QLabel(tr("Wheel"), this);
	lay->addWidget(lbkey, row++,0);

	QComboBox* cbwheel = new QComboBox(this);
	cbwheel->setObjectName("cbwheel");
	cbwheel->setToolTip(tr("<html>Selects a wheel action that will activate the control.</html>"));
	lay->addWidget(cbwheel, row++,0);

	cbwheel->addItem("Away From User", DiControlInput::WT_AwayFromUser);
	cbwheel->addItem("Toward User", DiControlInput::WT_TowardUser);

	int icb = cbwheel->findData(mouse);
	cbwheel->setCurrentIndex(icb);

	QPushButton* pbunbind = new QPushButton("Unbind Wheel");
	pbunbind->setToolTip(tr("<html>Removes the wheel binding from the control. "
				"The control will no longer be activated by a wheel action.</html>"));
	connect(pbunbind, SIGNAL(clicked()), SLOT(Unbind()));
	lay->addWidget(pbunbind, row++,0);

	QPushButton* pbok = new QPushButton("Ok");
	pbok->setToolTip(tr("<html>Binds the selected wheel action to the control.</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(OkPressed()));
	lay->addWidget(pbok, row++,0);

	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuBindWheel::~AuBindWheel()
{
}

void AuBindWheel::OkPressed()
{
	QComboBox* cbwheel = findChild<QComboBox*>("cbwheel");
	m_Wheel = (DiControlInput::WHEELTRIGGER)cbwheel->itemData(cbwheel->currentIndex()).toInt();

	accept();
}

void AuBindWheel::Unbind()
{
	m_Wheel = DiControlInput::WT_None;

	accept();
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBindJoystick::AuBindJoystick(const QString& action, DiControlInput::JOYTRIGGER joy, int joyIndex, int joySubIndex,
		bool joyAxisExclusive, QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/Settings.png"));
	setAuDlgTitle(tr("<b>%1</b>").arg(action));

	m_GrabInput = false;
	m_Joy = joy;
	m_JoyIndex = joyIndex;
	m_JoySubIndex = joySubIndex;
	m_JoyAxisExclusive = joyAxisExclusive;
	m_timer = 0;
	m_HatJoyIndex = -1;	// >= 0 to enable autoselect of centered hat.
	m_HatJoySubIndex = 0;

	QGridLayout* lay = new QGridLayout;
	int row=0;

	if(joyAxisExclusive)
	{
		QLabel* lbaxisex = new QLabel(tr("The full range of a joystick axis is required for the control input."), this);
		lay->addWidget(lbaxisex, row++,0);
	}

	QString joyAction;
	switch(joy)
	{
	case DiControlInput::JT_None:
		break;
	case DiControlInput::JT_Axis:
		joyAction = tr("Joystick %1 Axis %2")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_AxisInvert:
		joyAction = tr("Joystick %1 Axis %2 Inverted")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_AxisNegative:
		joyAction = tr("Joystick %1 Axis %2 Negative")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_AxisPositive:
		joyAction = tr("Joystick %1 Axis %2 Positive")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_Button:
		joyAction = tr("Joystick %1 Button %2")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_BallXNegative:
		joyAction = tr("Joystick %1 Ball %2 X Negative")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_BallXPositive:
		joyAction = tr("Joystick %1 Ball %2 X Positive")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_BallYNegative:
		joyAction = tr("Joystick %1 Ball %2 Y Negative")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_BallYPositive:
		joyAction = tr("Joystick %1 Ball %2 Y Positive")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_HatCentered:
		joyAction = tr("Joystick %1 Hat %2 Centered")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_HatDown:
		joyAction = tr("Joystick %1 Hat %2 Down")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_HatLeft:
		joyAction = tr("Joystick %1 Hat %2 Left")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_HatLeftDown:
		joyAction = tr("Joystick %1 Hat %2 Left Down")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_HatLeftUp:
		joyAction = tr("Joystick %1 Hat %2 Left Up")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_HatRight:
		joyAction = tr("Joystick %1 Hat %2 Right")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_HatRightDown:
		joyAction = tr("Joystick %1 Hat %2 Right Down")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_HatRightUp:
		joyAction = tr("Joystick %1 Hat %2 Right Up")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	case DiControlInput::JT_HatUp:
		joyAction = tr("Joystick %1 Hat %2 Up")
			.arg(joyIndex+1)
			.arg(joySubIndex+1);
		break;
	}
	QLabel* lbjoyact = new QLabel(joyAction, this);
	lbjoyact->setObjectName("lbjoyact");
	lbjoyact->setAlignment(Qt::AlignCenter);
	lay->addWidget(lbjoyact, row++,0);

	if(joyAxisExclusive)
	{
		QCheckBox* ckinvert = new QCheckBox(tr("Invert Axis"), this);
		ckinvert->setObjectName("ckinvert");
		ckinvert->setCheckState((joy == DiControlInput::JT_AxisInvert)?Qt::Checked:Qt::Unchecked);
		ckinvert->setToolTip(tr("<html>The positive/negative range of the axis can be reversed. "
					"This is often useful for single-axis throttle controls so the position doesn't seem backward.</html>"));
		connect(ckinvert, SIGNAL(stateChanged(int)), SLOT(InvertAxis(int)));
		lay->addWidget(ckinvert, row++,0);
	}
	else
	{
		// Check to see if any of the joysticks have hats.
		AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
		for(int joyIndex=0; joyIndex<pFrame->Joystick()->Count(); joyIndex++)
		{
			// Skip joysticks that are disabled.
			if(!g_Au.IsJoystickEnabled(joyIndex))
				continue;

			if(pFrame->Joystick()->Hats(joyIndex))
			{
				// Found a joystick with a hat.
				QLabel* lbhatwait = new QLabel(tr("A hat was detected.<br>"
							"To use the center position of a hat, press <b>Grab Input</b> and<br>"
							"wait without activating any joystick. Every five seconds the next<br>"
							"centered hat among those available will be selected."), this);
				lay->addWidget(lbhatwait, row++,0);
				m_HatJoyIndex = 0;	// So we know we have at least one hat.
				break;
			}
		}
	}

	QPushButton* pbgrab = new QPushButton("Grab Input");
	pbgrab->setObjectName("pbgrab");
	if(joyAxisExclusive)
		pbgrab->setToolTip(tr("<html>Monitors all joysticks for the axis that will be used for the control.<br>"
					"Set all joystick axes to their neutral positions before pressing <b>Grab Input</b>.<br>"
					"Press <b>Grab Input</b> and then deflect a joystick axis you wish to bind to the control.</html>"));
	else if(m_HatJoyIndex == 0)	// A hat was detected.
		pbgrab->setToolTip(tr("<html>Monitors all joysticks for input that will be used for the control.<br>"
					"Set all joystick axes to their neutral positions before pressing <b>Grab Input</b>.<br>"
					"Press <b>Grab Input</b> and then activate a joystick you wish to bind to the control. To activate a "
					"joystick, deflect a stick on one of its axes, press a button, roll a ball, or deflect a hat.<br>"
					"To use the center position of a hat, wait five seconds without activating any joystick and the hat will be "
					"selected automatically. When multiple joysticks with hats are enabled, the center position of each "
					"joystick is selected in turn every five seconds.</html>"));
	else
		pbgrab->setToolTip(tr("<html>Monitors all joysticks for input that will be used for the control.<br>"
					"Set all joystick axes to their neutral positions before pressing <b>Grab Input</b>.<br>"
					"Press <b>Grab Input</b> and then activate a joystick you wish to bind to the control. To activate a "
					"joystick, deflect a stick on one of its axes, press a button, or roll a ball.</html>"));
	connect(pbgrab, SIGNAL(clicked()), SLOT(GrabInput()));
	lay->addWidget(pbgrab, row++,0);

	QPushButton* pbunbind = new QPushButton("Unbind Joystick");
	pbunbind->setToolTip(tr("<html>Removes the joystick binding from the control. "
				"The control will no longer be activated by a joystick action.</html>"));
	connect(pbunbind, SIGNAL(clicked()), SLOT(Unbind()));
	lay->addWidget(pbunbind, row++,0);

	QPushButton* pbok = new QPushButton("Ok");
	pbok->setToolTip(tr("<html>Binds the selected joystick action to the control.</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(OkPressed()));
	lay->addWidget(pbok, row++,0);

	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuBindJoystick::~AuBindJoystick()
{
}

void AuBindJoystick::GrabInput()
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	if(!pFrame->Joystick() || pFrame->Joystick()->Count() == 0)
	{
		QString smsg;
		smsg = tr("No joysticks have been detected. Plug in a joystick to use this\n"
				"feature. %1 may need to be restarted\n"
				"after doing so.").arg(g_Au.ApplicationName());
		QMessageBox mb(g_Au.ApplicationName(), smsg,
				QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
		auApp->Frame()->ExecMessageBox(mb);
		return;
	}

	// Start a timer to wait for hat center recognition.
	if(m_HatJoyIndex >= 0)
	{
		for(m_HatJoyIndex=0; m_HatJoyIndex<pFrame->Joystick()->Count(); m_HatJoyIndex++)
		{
			// Skip joysticks that are disabled.
			if(!g_Au.IsJoystickEnabled(m_HatJoyIndex))
				continue;

			if(pFrame->Joystick()->Hats(m_HatJoyIndex))
			{
				// Found a joystick with a hat.
				m_HatWait.start();
				break;
			}
		}
	}
	m_HatJoySubIndex = 0;

	m_GrabInput = true;
	QPushButton* pbgrab = findChild<QPushButton*>("pbgrab");
	pbgrab->setDown(true);
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(PollJoystick()));
	}
	if(!m_timer->isActive())
		m_timer->start(100);

	QLabel* lbjoyact = findChild<QLabel*>("lbjoyact");
	lbjoyact->setText("");
	m_Joy = DiControlInput::JT_None;
}

void AuBindJoystick::PollJoystick()
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	if(!pFrame->Joystick())
		return;
	pFrame->Joystick()->Poll();

	QString joyAction;
	for(int joyIndex=0; joyIndex<pFrame->Joystick()->Count(); joyIndex++)
	{
		// Skip joysticks that are disabled.
		if(!g_Au.IsJoystickEnabled(joyIndex))
			continue;

		// Look for an axis that is deflected.
		for(int joySubIndex=0; joySubIndex<pFrame->Joystick()->Axes(joyIndex); joySubIndex++)
		{
			int value = pFrame->Joystick()->AxisValue(joyIndex, joySubIndex);
			if(abs(value) > 16000)	// Watch for at least 1/2 full deflection.
			{
				// Found a deflected axis.
				// Stop grabbing input.
				m_GrabInput = false;
				if(m_timer && m_timer->isActive())
					m_timer->stop();

				// Describe the result.
				if(m_JoyAxisExclusive)
				{
					QCheckBox* ckinvert = findChild<QCheckBox*>("ckinvert");
					if(ckinvert->checkState() == Qt::Unchecked)
					{
						joyAction = tr("Joystick %1 Axis %2")
							.arg(joyIndex+1)
							.arg(joySubIndex+1);
						m_Joy = DiControlInput::JT_Axis;
					}
					else
					{
						joyAction = tr("Joystick %1 Axis %2 Inverted")
							.arg(joyIndex+1)
							.arg(joySubIndex+1);
						m_Joy = DiControlInput::JT_AxisInvert;
					}
				}
				else if(value > 0)
				{
					joyAction = tr("Joystick %1 Axis %2 Positive")
						.arg(joyIndex+1)
						.arg(joySubIndex+1);
					m_Joy = DiControlInput::JT_AxisPositive;
				}
				else
				{
					joyAction = tr("Joystick %1 Axis %2 Negative")
						.arg(joyIndex+1)
						.arg(joySubIndex+1);
					m_Joy = DiControlInput::JT_AxisNegative;
				}
				m_JoyIndex = joyIndex;
				m_JoySubIndex = joySubIndex;

				// Update controls
				QLabel* lbjoyact = findChild<QLabel*>("lbjoyact");
				lbjoyact->setText(joyAction);
				QPushButton* pbgrab = findChild<QPushButton*>("pbgrab");
				pbgrab->setDown(false);

				// We're done. Stop looking for input.
				return;
			}
		}

		// Skip all other checks if this input needs exclusive use of a joystick axis.
		if(m_JoyAxisExclusive)
			continue;

		// Look for a button that is pressed.
		for(int joySubIndex=0; joySubIndex<pFrame->Joystick()->Buttons(joyIndex); joySubIndex++)
		{
			if(pFrame->Joystick()->ButtonState(joyIndex, joySubIndex))
			{
				// Found a depressed button.
				// Stop grabbing input.
				m_GrabInput = false;
				if(m_timer && m_timer->isActive())
					m_timer->stop();

				// Describe the result.
				joyAction = tr("Joystick %1 Button %2")
					.arg(joyIndex+1)
					.arg(joySubIndex+1);
				m_Joy = DiControlInput::JT_Button;
				m_JoyIndex = joyIndex;
				m_JoySubIndex = joySubIndex;

				// Update controls
				QLabel* lbjoyact = findChild<QLabel*>("lbjoyact");
				lbjoyact->setText(joyAction);
				QPushButton* pbgrab = findChild<QPushButton*>("pbgrab");
				pbgrab->setDown(false);

				// We're done. Stop looking for input.
				return;
			}
		}

		// Look for a ball that has moved.
		for(int joySubIndex=0; joySubIndex<pFrame->Joystick()->Balls(joyIndex); joySubIndex++)
		{
			int dx=0;
			int dy=0;
			pFrame->Joystick()->BallState(joyIndex, joySubIndex, dx,dy);
			if(abs(dx) > 10 || abs(dy) > 10)
			{
				// Found a ball that moved.
				// Stop grabbing input.
				m_GrabInput = false;
				if(m_timer && m_timer->isActive())
					m_timer->stop();

				// Describe the result.
				if(abs(dx) > abs(dy))
				{
					// X axis moved more than Y axis.
					if(dx > 0)
					{
						joyAction = tr("Joystick %1 Ball %2 X Positive")
							.arg(joyIndex+1)
							.arg(joySubIndex+1);
						m_Joy = DiControlInput::JT_BallXPositive;
					}
					else
					{
						joyAction = tr("Joystick %1 Ball %2 X Negative")
							.arg(joyIndex+1)
							.arg(joySubIndex+1);
						m_Joy = DiControlInput::JT_BallXNegative;
					}
				}
				else
				{
					// Y axis moved more than X axis.
					if(dy > 0)
					{
						joyAction = tr("Joystick %1 Ball %2 Y Positive")
							.arg(joyIndex+1)
							.arg(joySubIndex+1);
						m_Joy = DiControlInput::JT_BallYPositive;
					}
					else
					{
						joyAction = tr("Joystick %1 Ball %2 Y Negative")
							.arg(joyIndex+1)
							.arg(joySubIndex+1);
						m_Joy = DiControlInput::JT_BallYNegative;
					}
				}
				m_JoyIndex = joyIndex;
				m_JoySubIndex = joySubIndex;

				// Update controls
				QLabel* lbjoyact = findChild<QLabel*>("lbjoyact");
				lbjoyact->setText(joyAction);
				QPushButton* pbgrab = findChild<QPushButton*>("pbgrab");
				pbgrab->setDown(false);

				// We're done. Stop looking for input.
				return;
			}
		}

		// Look for a hat that is deflected.
		for(int joySubIndex=0; joySubIndex<pFrame->Joystick()->Hats(joyIndex); joySubIndex++)
		{
			quint8 hat = pFrame->Joystick()->HatState(joyIndex, joySubIndex);
			if(hat != SDL_HAT_CENTERED)
			{
				// Found a deflected hat.
				// Describe the result.
				switch(hat)
				{
				default:
				case SDL_HAT_CENTERED:
					continue;			// Skip these inputs.
				case SDL_HAT_UP:
					joyAction = tr("Joystick %1 Hat %2 Up")
						.arg(joyIndex+1)
						.arg(joySubIndex+1);
					m_Joy = DiControlInput::JT_HatUp;
					break;
				case SDL_HAT_RIGHT:
					joyAction = tr("Joystick %1 Hat %2 Right")
						.arg(joyIndex+1)
						.arg(joySubIndex+1);
					m_Joy = DiControlInput::JT_HatRight;
					break;
				case SDL_HAT_DOWN:
					joyAction = tr("Joystick %1 Hat %2 Down")
						.arg(joyIndex+1)
						.arg(joySubIndex+1);
					m_Joy = DiControlInput::JT_HatDown;
					break;
				case SDL_HAT_LEFT:
					joyAction = tr("Joystick %1 Hat %2 Left")
						.arg(joyIndex+1)
						.arg(joySubIndex+1);
					m_Joy = DiControlInput::JT_HatLeft;
					break;
				case SDL_HAT_RIGHTUP:
					joyAction = tr("Joystick %1 Hat %2 Right Up")
						.arg(joyIndex+1)
						.arg(joySubIndex+1);
					m_Joy = DiControlInput::JT_HatRightUp;
					break;
				case SDL_HAT_RIGHTDOWN:
					joyAction = tr("Joystick %1 Hat %2 Right Down")
						.arg(joyIndex+1)
						.arg(joySubIndex+1);
					m_Joy = DiControlInput::JT_HatRightDown;
					break;
				case SDL_HAT_LEFTUP:
					joyAction = tr("Joystick %1 Hat %2 Left Up")
						.arg(joyIndex+1)
						.arg(joySubIndex+1);
					m_Joy = DiControlInput::JT_HatLeftUp;
					break;
				case SDL_HAT_LEFTDOWN:
					joyAction = tr("Joystick %1 Hat %2 Left Down")
						.arg(joyIndex+1)
						.arg(joySubIndex+1);
					m_Joy = DiControlInput::JT_HatLeftDown;
					break;
				}
				m_JoyIndex = joyIndex;
				m_JoySubIndex = joySubIndex;

				// Stop grabbing input.
				m_GrabInput = false;
				if(m_timer && m_timer->isActive())
					m_timer->stop();

				// Update controls
				QLabel* lbjoyact = findChild<QLabel*>("lbjoyact");
				lbjoyact->setText(joyAction);
				QPushButton* pbgrab = findChild<QPushButton*>("pbgrab");
				pbgrab->setDown(false);

				// We're done. Stop looking for input.
				return;
			}
		}
	}

	// We get here if no joystick control is activated yet.
	// Each five seconds of elapsed time we will select a centered hat.
	if(m_HatJoyIndex >= 0 && m_HatWait.elapsed() >= 5000)
	{
		// At least five seconds have passed.
		m_Joy = DiControlInput::JT_HatCentered;
		m_JoyIndex = m_HatJoyIndex;
		m_JoySubIndex = m_HatJoySubIndex;

		// Describe the result and update the control but don't stop grabbing so we will cycle to the next hat in 5 more seconds.
		joyAction = tr("Joystick %1 Hat %2 Centered")
			.arg(m_JoyIndex+1)
			.arg(m_JoySubIndex+1);
		QLabel* lbjoyact = findChild<QLabel*>("lbjoyact");
		lbjoyact->setText(joyAction);

		// Cycle to the next hat for the next 5 second interval.
		m_HatJoySubIndex++;
		if(m_HatJoySubIndex >= pFrame->Joystick()->Hats(m_HatJoyIndex))
		{
			// Skip to the next joystick that has a hat.
			for(; m_HatJoyIndex<pFrame->Joystick()->Count(); m_HatJoyIndex++)
			{
				// Skip joysticks that are disabled.
				if(!g_Au.IsJoystickEnabled(m_HatJoyIndex))
					continue;

				if(pFrame->Joystick()->Hats(m_HatJoyIndex))
					break;	// Found a joystick with a hat.
			}
			if(m_HatJoyIndex >= pFrame->Joystick()->Count())
			{
				// Did not find another joystick with a hats. Restart from the beginning.
				for(m_HatJoyIndex=0; m_HatJoyIndex<pFrame->Joystick()->Count(); m_HatJoyIndex++)
				{
					// Skip joysticks that are disabled.
					if(!g_Au.IsJoystickEnabled(m_HatJoyIndex))
						continue;

					if(pFrame->Joystick()->Hats(m_HatJoyIndex))
						break;	// Found a joystick with a hat.
				}
			}
			m_HatJoySubIndex = 0;
		}

		// Restart the hat wait timer.
		m_HatWait.restart();
	}
}

void AuBindJoystick::OkPressed()
{
	accept();
}

void AuBindJoystick::Unbind()
{
	m_Joy = DiControlInput::JT_None;

	accept();
}

void AuBindJoystick::InvertAxis(int value)
{
	QString joyAction;
	if(value == Qt::Unchecked)
	{
		joyAction = tr("Joystick %1 Axis %2")
			.arg(m_JoyIndex+1)
			.arg(m_JoySubIndex+1);
		m_Joy = DiControlInput::JT_Axis;
	}
	else
	{
		joyAction = tr("Joystick %1 Axis %2 Inverted")
			.arg(m_JoyIndex+1)
			.arg(m_JoySubIndex+1);
		m_Joy = DiControlInput::JT_AxisInvert;
	}
	QLabel* lbjoyact = findChild<QLabel*>("lbjoyact");
	lbjoyact->setText(joyAction);
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuConfigureJoystick::AuConfigureJoystick(int joyIndex, QWidget* parent) : AuDialog(parent)
{
	m_JoyIndex = joyIndex;

	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	setAuDlgIcon(QIcon(":data/Settings.png"));
	setAuDlgTitle(tr("<b>%1</b>").arg(pFrame->Joystick()->Name(joyIndex)));

	QGridLayout* lay = new QGridLayout;
	int row=0;

	QCheckBox* ckjoy = new QCheckBox(tr("Enable Joystick %1, %2").arg(joyIndex+1).arg(pFrame->Joystick()->Name(joyIndex)));
	ckjoy->setCheckState(g_Au.IsJoystickEnabled(joyIndex)?Qt::Checked:Qt::Unchecked);
	ckjoy->setToolTip(tr("<html>When a joystick is disabled, all input from that joystick will be ignored.</html>"));
	connect(ckjoy, SIGNAL(stateChanged(int)), SLOT(SetEnableJoystick(int)));
	lay->addWidget(ckjoy, row++,0);

	int axes = pFrame->Joystick()->Axes(joyIndex);
	if(axes)
	{
		QLabel* lbstick;
		if(axes == 1)
			lbstick = new QLabel(tr("%1 Axis").arg(axes));
		else
			lbstick = new QLabel(tr("%1 Axes").arg(axes));
		lay->addWidget(lbstick, row++,0);
	}

	int balls = pFrame->Joystick()->Balls(joyIndex);
	if(balls)
	{
		QLabel* lbballs = new QLabel(tr("%1 Balls").arg(balls));
		lay->addWidget(lbballs, row++,0);
	}

	int hats = pFrame->Joystick()->Hats(joyIndex);
	if(hats)
	{
		QLabel* lbhats = new QLabel(tr("%1 Hats").arg(hats));
		lay->addWidget(lbhats, row++,0);
	}

	int buttons = pFrame->Joystick()->Buttons(joyIndex);
	if(buttons)
	{
		QLabel* lbbuttons = new QLabel(tr("%1 Buttons").arg(buttons));
		lay->addWidget(lbbuttons, row++,0);
	}

	lay->setRowStretch(row, 1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuConfigureJoystick::~AuConfigureJoystick()
{
}

void AuConfigureJoystick::SetEnableJoystick(int value)
{
	g_Au.SetEnableJoystick(m_JoyIndex, value ? true : false);
}
