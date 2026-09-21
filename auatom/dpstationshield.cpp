//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationshield.cpp,v 1.25 2022/04/14 20:47:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpstationshield.h"
//
#include "auatomrenderinfo.h"
#include "auglobal.h"
#include "ausound.h"
#include "daperson.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "dipanelcontrol.h"
#include "augleffects.h"
#include <qpainter.h>
#include <qsqlquery.h>
#include <QPixmap>

#define d_Balance						SOUND_Bleep4
#define d_Power							SOUND_Bleep7
#define d_Shield						SOUND_Bleep1

// Table of control inputs.
DiControlInputArray s_ShieldConsoleControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_ShieldConsoleDefaults[] =
{
	DiControlInput(QObject::tr("Power On/Off"),
				DpStationShield::CI_PowerOnOff, Qt::Key_P,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Balance Power Distribution"),
				DpStationShield::CI_Reset, Qt::Key_O,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	DiControlInput(QObject::tr("Front Full Power"),
				DpStationShield::CI_FrontAll, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Front Decrease Power"),
				DpStationShield::CI_FrontDec, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Front Increase Power"),
				DpStationShield::CI_FrontInc, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Front Zero Power"),
				DpStationShield::CI_FrontInc, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	DiControlInput(QObject::tr("Back Full Power"),
				DpStationShield::CI_BackAll, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Back Decrease Power"),
				DpStationShield::CI_BackDec, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Back Increase Power"),
				DpStationShield::CI_BackInc, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Back Zero Power"),
				DpStationShield::CI_BackInc, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	DiControlInput(QObject::tr("Left Full Power"),
				DpStationShield::CI_LeftAll, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Left Decrease Power"),
				DpStationShield::CI_LeftDec, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Left Increase Power"),
				DpStationShield::CI_LeftInc, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Left Zero Power"),
				DpStationShield::CI_LeftInc, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	DiControlInput(QObject::tr("Right Full Power"),
				DpStationShield::CI_RightAll, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Right Decrease Power"),
				DpStationShield::CI_RightDec, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Right Increase Power"),
				DpStationShield::CI_RightInc, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Right Zero Power"),
				DpStationShield::CI_RightInc, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	DiControlInput(QObject::tr("Top Full Power"),
				DpStationShield::CI_TopAll, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Top Decrease Power"),
				DpStationShield::CI_TopDec, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Top Increase Power"),
				DpStationShield::CI_TopInc, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Top Zero Power"),
				DpStationShield::CI_TopInc, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	DiControlInput(QObject::tr("Bottom Full Power"),
				DpStationShield::CI_BottomAll, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Bottom Decrease Power"),
				DpStationShield::CI_BottomDec, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Bottom Increase Power"),
				DpStationShield::CI_BottomInc, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Bottom Zero Power"),
				DpStationShield::CI_BottomInc, Qt::Key_unknown,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
};
int s_NumShieldConsoleDefaults = sizeof(s_ShieldConsoleDefaults)/sizeof(DiControlInput);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DpStationShield::DpStationShield() : DpStation()
{
	m_LastShieldsPower = DaVessel::SS_Invalid;
	m_LastShieldsUnits = -1;
	for(int side=0; side<6; side++)
	{
		m_LastShields[side] = -1;
		m_Shields[side] = 0;
	}
	m_ShieldUnits = 0;
	m_ScreenTex = 0;
}

DpStationShield::~DpStationShield()
{

}

DpStationShield::DpStationShield(const DpStationShield& c) : DpStation(c)
{
	m_LastShieldsPower = c.m_LastShieldsPower;
	m_LastShieldsUnits = c.m_LastShieldsUnits;
	for(int side=0; side<6; side++)
	{
		m_LastShields[side] = c.m_LastShields[side];
		m_Shields[side] = c.m_Shields[side];
	}
	m_ShieldUnits = c.m_ShieldUnits;
	m_ScreenTex = 0;
}

void DpStationShield::Tx(QDataStream& out) const
{
	DpStation::Tx(out);
	for(int side=0; side<6; side++)
		out << m_Shields[side];
	out << m_ShieldUnits;
}

void DpStationShield::Rx(QDataStream& in, DaCapsule& capsule)
{
	DpStation::Rx(in, capsule);
	for(int side=0; side<6; side++)
		in >> m_Shields[side];
	in >> m_ShieldUnits;
}

// Refresh data from valid counterpart.
void DpStationShield::Refresh(const DpStation* pCon, const DaAtom& parent)
{
	DpStation::Refresh(pCon, parent);		// Refresh base class.

	const DpStationShield* pA = (const DpStationShield*)pCon;
	for(int side=0; side<6; side++)
		m_Shields[side] = pA->m_Shields[side];
	m_ShieldUnits = pA->m_ShieldUnits;
}

bool DpStationShield::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person)
{
	Q_UNUSED(last);
	Q_UNUSED(now);
	// Monitor shield systems on server.
	DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
	if(ship)
	{
		if(ship->IsDesignDataCacheValid())
		{
			for(int side=0; side<6; side++)
				m_Shields[side] = (float)(ship->ShieldCapacitor((DaVessel::SHIELD)side)/ship->ShieldUnits());
			m_ShieldUnits = ship->ShieldUnitsWorking();	// Number of shield units actually functioning.

			// Get the blip that is locked-on on the ship's sensors.
			return true;
		}
	}
	return false;
}

// Called between rendering scene frames that contains this object.
void DpStationShield::AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
		const AuPoint& location, const DaCreature& sceneAtom)
{
	// Base class animates the lower console pane.
	DpStation::AnimateDeviceObjects(re, ship, last, location, sceneAtom);

	// Get the texture because we're going to make several changes to it.
	QString name;
	name = QString("HCon%1").arg(ship.Id().GetID());
	AuGLTexture *pTex = re.FindTexture(name);
	if(!pTex)
		return;

	bool distChange = false;	// Set true if we need to redraw the power distribution bars.

	if(ship.ShieldState() != m_LastShieldsPower)
	{
		m_LastShieldsPower = ship.ShieldState();
		distChange = true;		// Power state changed.

		QImage bmbut;
		QImage bmlit;
		switch(m_LastShieldsPower)
		{
		case DaVessel::SS_Up:
			bmbut.load(":/data/ec_PowerOn.bmp");
			bmlit.load(":/data/ec_StatusLightGreen.bmp");
			break;
		case DaVessel::SS_Down:
			bmbut.load(":/data/ec_PowerPark.bmp");
			bmlit.load(":/data/ec_StatusLightPurple.bmp");
			break;
		case DaVessel::SS_Off:
			bmbut.load(":/data/ec_PowerOff.bmp");
			bmlit.load(":/data/ec_StatusLightOff.bmp");
			break;
		case DaVessel::SS_Invalid:
			break;
		}
		pTex->ReplaceSubImage(re, bmbut, 4,108);
		pTex->ReplaceSubImage(re, bmlit, 5,91);
	}

	// Power distribution by side.
	if(!distChange)
	{
		// Check to see if we need to redraw the power distribution bars.
		if(m_ShieldUnits != m_LastShieldsUnits)
			distChange = true;
		else
			for(int side=0; side<6 && !distChange; side++)
				if(m_Shields[side] != m_LastShields[side])
					distChange = true;
	}
	if(distChange)
	{
		// Power distribution bars need to be redrawn.
		// These coords are relative to the bottom left, with positive Y going up the texture.
		QPoint barLoc[6] =
		{
			QPoint(234,75),		// Front
			QPoint(6,17),		// Back
			QPoint(24,67),		// Left
			QPoint(216,25),		// Right
			QPoint(170,86),		// Top
			QPoint(70,5),		// Bottom
		};
		QPoint panelLoc[6] =
		{
			QPoint(193,74),		// Front
			QPoint(26,18),		// Back
			QPoint(44,66),		// Left
			QPoint(172,25),		// Right
			QPoint(91,79),		// Top
			QPoint(90,24),		// Bottom
		};
		const char* panelBase[6] =
		{
			":/data/txShieldFront",
			":/data/txShieldBack",
			":/data/txShieldLeft",
			":/data/txShieldRight",
			":/data/txShieldTop",
			":/data/txShieldBottom",
		};
		const float shieldFrac = m_ShieldUnits/ship.ShieldUnits();	// Fraction of total shield units functioning.
		const int barHt = 37;
		QPixmap bmbar(5,barHt);
		QImage imPanel;
		QString panelName;
		QBrush brlit(QColor(195,241,247));
		QColor coldim(61,75,77);
		for(int side=0; side<6; side++)
		{
			// Calculate number of pixels in the bar used by the shield allocation ratio.
			m_LastShields[side] = m_Shields[side];

			// Paint the bar.
			int shieldStr = 0;
			bmbar.fill(coldim);
			switch(ship.ShieldState())
			{
			case DaVessel::SS_Up:
				{
					// Shields are on and up.
					int shieldPix = (int)ceil(m_Shields[side] * shieldFrac * barHt);	// Pixels of the bar used by the lit part.
					shieldStr = (int)ceil(m_Shields[side] * shieldFrac * 10);	// Shield strength in the range 0..10

					// Show power allocation portion of bar.
					QPainter p(&bmbar);
					p.fillRect(0,barHt - shieldPix, 5,shieldPix, brlit);
					p.end();
				}
				break;
			case DaVessel::SS_Down:
				{
					// Shields are on but down.
					int shieldPix = (int)ceil(m_Shields[side] * shieldFrac * barHt);	// Pixels of the bar used by the lit part.
					shieldStr = 0;

					// Show power allocation portion of bar.
					QPainter p(&bmbar);
					p.fillRect(0,barHt - shieldPix, 5,shieldPix, brlit);
					p.end();
				}
				break;
			case DaVessel::SS_Off:
			case DaVessel::SS_Invalid:
				break;
			}

			// Put the bar into the console texture.
			QImage imbar = bmbar.toImage();
			pTex->ReplaceSubImage(re, imbar, barLoc[side].x(), barLoc[side].y());

			// Show shield strength indicator panel.
			if(shieldStr >= 0 && shieldStr <= 10)
			{
				// Panel name. e.g. ':/data/txShieldFront0.png'
				panelName = QString("%1%2.png").arg(panelBase[side]).arg(shieldStr);
				imPanel.load(panelName);
				pTex->ReplaceSubImage(re, imPanel, panelLoc[side].x(), panelLoc[side].y());
			}
		}

		m_LastShieldsUnits = m_ShieldUnits;
	}
}

void DpStationShield::ConfigureControls(const DiControlInputArray &aAllControls)
{
	// Update my controls with those passed to me.
	for(DiControlInputArray::const_iterator ci = aAllControls.constBegin(); ci != aAllControls.constEnd(); ci++)
	{
		// Locate the corresponding control in my controls array.
		for(DiControlInputArray::iterator myci = s_ShieldConsoleControls.begin(); myci != s_ShieldConsoleControls.end(); myci++)
		{
			if((*myci).GetId() == (*ci).GetId())
			{
				// Found the corresponding control id. Copy in the nu data.
				*myci = *ci;
				break;
			}
		}
	}
	SaveControls();
}

void DpStationShield::RestoreDefaultControls()
{
	s_ShieldConsoleControls.clear();
	for(int ic=0; ic<s_NumShieldConsoleDefaults; ic++)
		s_ShieldConsoleControls.append(s_ShieldConsoleDefaults[ic]);
	SaveControls();
}

void DpStationShield::LoadControls() const
{
	// Init with my default controls.
	s_ShieldConsoleControls.clear();
	for(int ic=0; ic<s_NumShieldConsoleDefaults; ic++)
		s_ShieldConsoleControls.append(s_ShieldConsoleDefaults[ic]);

	// Load any overrides from the settings file.
	s_ShieldConsoleControls.LoadSettings("Controls/Shield Console");
}

void DpStationShield::SaveControls() const
{
	// Store my controls in the settings file.
	s_ShieldConsoleControls.SaveSettings("Controls/Shield Console");
}

// A scene atom should fill this array with control inputs it can accept.
void DpStationShield::EnumControlInputs(DiControlInputArray& aAllControls) const
{
	// Load my static controls array if it hasn't been loaded yet. This data will only exist on the client since EnumControlInputs
	// is not called on the server.
	if(s_ShieldConsoleControls.isEmpty())
		LoadControls();

	// Append my controls to the array.
	for(DiControlInputArray::const_iterator ci = s_ShieldConsoleControls.constBegin();
			ci != s_ShieldConsoleControls.constEnd(); ci++)
		aAllControls.append(*ci);
}

// This is how all control inputs are passed to a spacecraft console.
// Only control inputs that have been returned by EnumControlInputs will be passed.
// Only control inputs that have been activated will be passed.
void DpStationShield::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaVessel& ship,
		CONTROL_CONTEXT cc, const AuPoint& /*location*/)
{
	if(ship.IsNew())
		return;	// This console cannot be used when the ship is "nu".

	for(int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch (ci.GetId())
		{
		case CI_PowerOnOff:
			if(cc != CONTROL_Client)
			{
				switch(ship.ShieldState())
				{
				case DaVessel::SS_Up:
				case DaVessel::SS_Invalid:
					ship.SetShieldState(DaVessel::SS_Down);
					break;
				case DaVessel::SS_Down:
					ship.SetShieldState(DaVessel::SS_Off);
					break;
				case DaVessel::SS_Off:
					ship.SetShieldState(DaVessel::SS_Up);
					break;
				}
			}
			if(cc != CONTROL_Server)
			{
				float bp = .8f + ship.ShieldState()/10.f;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Power, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Reset:
			if(cc != CONTROL_Client)
				ship.ShieldBalance();
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Balance, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_FrontAll:
			if(cc != CONTROL_Client)
				ship.ShieldFull(DaVessel::SHIELD_Front);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_FrontDec:
			if(cc != CONTROL_Client)
				ship.ShieldDecrease(DaVessel::SHIELD_Front);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_FrontInc:
			if(cc != CONTROL_Client)
				ship.ShieldIncrease(DaVessel::SHIELD_Front);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_FrontNone:
			if(cc != CONTROL_Client)
				ship.ShieldOff(DaVessel::SHIELD_Front);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.75f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_BackAll:
			if(cc != CONTROL_Client)
				ship.ShieldFull(DaVessel::SHIELD_Back);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_BackDec:
			if(cc != CONTROL_Client)
				ship.ShieldDecrease(DaVessel::SHIELD_Back);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_BackInc:
			if(cc != CONTROL_Client)
				ship.ShieldIncrease(DaVessel::SHIELD_Back);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_BackNone:
			if(cc != CONTROL_Client)
				ship.ShieldOff(DaVessel::SHIELD_Back);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.75f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_LeftAll:
			if(cc != CONTROL_Client)
				ship.ShieldFull(DaVessel::SHIELD_Left);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_LeftDec:
			if(cc != CONTROL_Client)
				ship.ShieldDecrease(DaVessel::SHIELD_Left);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_LeftInc:
			if(cc != CONTROL_Client)
				ship.ShieldIncrease(DaVessel::SHIELD_Left);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_LeftNone:
			if(cc != CONTROL_Client)
				ship.ShieldOff(DaVessel::SHIELD_Left);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.75f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_RightAll:
			if(cc != CONTROL_Client)
				ship.ShieldFull(DaVessel::SHIELD_Right);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_RightDec:
			if(cc != CONTROL_Client)
				ship.ShieldDecrease(DaVessel::SHIELD_Right);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_RightInc:
			if(cc != CONTROL_Client)
				ship.ShieldIncrease(DaVessel::SHIELD_Right);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_RightNone:
			if(cc != CONTROL_Client)
				ship.ShieldOff(DaVessel::SHIELD_Right);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.75f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_TopAll:
			if(cc != CONTROL_Client)
				ship.ShieldFull(DaVessel::SHIELD_Top);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_TopDec:
			if(cc != CONTROL_Client)
				ship.ShieldDecrease(DaVessel::SHIELD_Top);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_TopInc:
			if(cc != CONTROL_Client)
				ship.ShieldIncrease(DaVessel::SHIELD_Top);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_TopNone:
			if(cc != CONTROL_Client)
				ship.ShieldOff(DaVessel::SHIELD_Top);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.75f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_BottomAll:
			if(cc != CONTROL_Client)
				ship.ShieldFull(DaVessel::SHIELD_Bottom);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_BottomDec:
			if(cc != CONTROL_Client)
				ship.ShieldDecrease(DaVessel::SHIELD_Bottom);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_BottomInc:
			if(cc != CONTROL_Client)
				ship.ShieldIncrease(DaVessel::SHIELD_Bottom);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_BottomNone:
			if(cc != CONTROL_Client)
				ship.ShieldOff(DaVessel::SHIELD_Bottom);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shield, 1,.75f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		}
	}
}

// This is how mouse control inputs activate controls in the control input array.
// Controls in aActiveControls are activated accordingly.
void DpStationShield::SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
		bool bContinuous)
{
	if(idPanel)
		return;
	double panWid, panHt;
	PanelDimensions(panWid, panHt);
	int ix = (int)(fPanelLoc.x * 256 / panWid);	// X pixel in texture map.
	int iy = (int)(fPanelLoc.y * 128 / panHt);
	QPoint pxHit(ix, iy);

	// Note that the top/bottom is reversed in the rectangles. This works with the iy
	// we've calculated that is also measured from the bottom, not the top as usual in GDI.
	DiPanelControl acontrols[] =
	{
		DiPanelControl(CI_PowerOnOff, QRect(4,108, 16,16)),
		DiPanelControl(CI_Reset, QRect(236,4, 16,16)),
		DiPanelControl(CI_FrontAll, QRect(243,104, 9,9)),
		DiPanelControl(CI_FrontDec, QRect(243,84, 9,9)),
		DiPanelControl(CI_FrontInc, QRect(243,94, 9,9)),
		DiPanelControl(CI_FrontNone, QRect(243,74, 9,9)),
		DiPanelControl(CI_BackAll, QRect(15,46, 9,9)),
		DiPanelControl(CI_BackDec, QRect(15,26, 9,9)),
		DiPanelControl(CI_BackInc, QRect(15,36, 9,9)),
		DiPanelControl(CI_BackNone, QRect(15,16, 9,9)),
		DiPanelControl(CI_LeftAll, QRect(33,96, 9,9)),
		DiPanelControl(CI_LeftDec, QRect(33,76, 9,9)),
		DiPanelControl(CI_LeftInc, QRect(33,86, 9,9)),
		DiPanelControl(CI_LeftNone, QRect(33,66, 9,9)),
		DiPanelControl(CI_RightAll, QRect(225,54, 9,9)),
		DiPanelControl(CI_RightDec, QRect(225,34, 9,9)),
		DiPanelControl(CI_RightInc, QRect(225,44, 9,9)),
		DiPanelControl(CI_RightNone, QRect(225,24, 9,9)),
		DiPanelControl(CI_TopAll, QRect(179,115, 9,9)),
		DiPanelControl(CI_TopDec, QRect(179,95, 9,9)),
		DiPanelControl(CI_TopInc, QRect(179,105, 9,9)),
		DiPanelControl(CI_TopNone, QRect(179,85, 9,9)),
		DiPanelControl(CI_BottomAll, QRect(79,34, 9,9)),
		DiPanelControl(CI_BottomDec, QRect(79,14, 9,9)),
		DiPanelControl(CI_BottomInc, QRect(79,24, 9,9)),
		DiPanelControl(CI_BottomNone, QRect(79,4, 9,9)),
	};
	unsigned int icontrols = sizeof(acontrols)/sizeof(DiPanelControl);

	aActiveControls.SampleControls(idPanel, pxHit, bContinuous, acontrols, icontrols);
}

void DpStationShield::Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
		const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari)
{
	DpStation::Render(projection, farProjection, tModel, sceneAtom, seatedCreature, ari);
	if(seatedCreature && seatedCreature->Is(sceneAtom.Id()))
	{
		const DaVessel* ship = sceneAtom.FindParentVesselInEnv();
		if(ship)
		{
			// Draw the lock-on reticle.
			if(!ship->LockedBlip().IsNull())
				RenderSensorLock(ship->LockedBlip(), ship->LockedStance(), ship->LockedIsCivilian(), ship->LockedSysLoc(), tModel,
						sceneAtom, ari);
		}
	}
}

void DpStationShield::CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding)
{
	DpStation::CreateDeviceObjects(ari, parent, inBuilding);
	if(!m_ScreenTex)
	{
		if(inBuilding)
			m_ScreenTex = CreateBuildingDisplayTexture(*ari.re, parent.Id());
		else
			m_ScreenTex = CreateSpacecraftDisplayTexture(*ari.re, parent.Id());
	}
}

//static
GLuint DpStationShield::CreateSpacecraftDisplayTexture(AuGLEffects& re, const AuId& parentId)
{
	// Identify it uniquely by ship so all shield consoles on the same vessel show the same data.
	QString name = QString("HCon%1").arg(parentId.GetID());
	return re.CreateTextureFromBitmap(parentId.GetID(), name, ":data/txConsoleShield.bmp");
}

//static
GLuint DpStationShield::CreateBuildingDisplayTexture(AuGLEffects& re, const AuId& parentId)
{
	QString name = QString("HCon%1").arg(parentId.GetID());
	return re.CreateTextureFromBitmap(parentId.GetID(), name, ":data/BldConsoleShield.bmp");
}

void DpStationShield::DeleteDeviceObjects(AuGLEffects& re)
{
	DpStation::DeleteDeviceObjects(re);
	m_ScreenTex = 0;					// DaAtom deletes all textures used by the owner's atom id.
}

quint64 DpStationShield::MemoryUsage() const
{
	quint64 mem = DpStation::MemoryUsage();
	mem += sizeof(m_Shields);
	mem += sizeof(m_ShieldUnits);
	mem += sizeof(m_LastShieldsPower);
	mem += sizeof(m_LastShieldsUnits);
	mem += sizeof(m_LastShields);
	mem += sizeof(m_ScreenTex);
	return mem;
}
