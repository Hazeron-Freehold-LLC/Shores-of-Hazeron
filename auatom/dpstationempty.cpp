//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationempty.cpp,v 1.22 2022/02/19 17:45:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpstationempty.h"
//
#include "auatomrenderinfo.h"
#include "ausound.h"
#include "daatomlist.h"
#include "daperson.h"
#include "davessel.h"
#include "dicontrolinput.h"

#define d_RemoteCameraSwitch			SOUND_Bleep1
#define d_RemoteCameraNext				SOUND_Bleep2
#define d_RemoteCameraPrevious			SOUND_Bleep3
#define d_CloseDoors					SOUND_Bleep4
#define d_Headlight						SOUND_Bleep7

// Table of control inputs.
DiControlInputArray s_CommandConsoleControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_CommandConsoleDefaults[] =
{
	// Lights
	DiControlInput(QObject::tr("Headlight On/Off"),
				DpStationEmpty::CI_HeadlightOnOff, Qt::Key_L, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Door Remote
	DiControlInput(QObject::tr("Close All Hull Doors"),
				DpStationEmpty::CI_CloseHullDoors, Qt::Key_B, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Remote Door Switch"),
				DpStationEmpty::CI_DoorRemote, Qt::Key_M, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),

	// Remote Cameras
	DiControlInput(QObject::tr("Remote Camera On/Off"),
				DpStationEmpty::CI_RemoteCameraOnOff, Qt::Key_Home, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Remote Camera Next"),
				DpStationEmpty::CI_RemoteCameraNext, Qt::Key_PageUp, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Remote Camera Previous"),
				DpStationEmpty::CI_RemoteCameraPrevious, Qt::Key_PageDown, Qt::NoModifier,
				DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
};
int s_NumCommandConsoleDefaults = sizeof(s_CommandConsoleDefaults)/sizeof(DiControlInput);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DpStationEmpty::DpStationEmpty()
{
	m_RemoteCameraId = 0;
	m_RemoteCameraSwitch = false;
	m_ScreenFile = 0;
	m_ScreenTex = 0;
}

DpStationEmpty::~DpStationEmpty()
{
}

DpStationEmpty::DpStationEmpty(const DpStationEmpty& c) : DpStation(c)
{
	m_RemoteCameraId = c.m_RemoteCameraId;
	m_RemoteCameraSwitch = c.m_RemoteCameraSwitch;
	m_ScreenName = c.m_ScreenName;
	m_ScreenFile = c.m_ScreenFile;
	m_ScreenTex = 0;
}

void DpStationEmpty::CreateDeviceObjectsEmpty(AuAtomRenderInfo& ari, const DaAtom& parent, const QString& screenName,
		const char* screenFile, bool inBuilding)
{
	Q_ASSERT(!screenName.isEmpty());
	m_ScreenName = screenName;
	m_ScreenFile = screenFile;
	DpStation::CreateDeviceObjects(ari, parent, inBuilding);
	if(!m_ScreenTex && !m_ScreenName.isEmpty() && m_ScreenFile)
		m_ScreenTex = ari.re->CreateTextureFromBitmap(parent.Id().GetID(), m_ScreenName, m_ScreenFile);
}

void DpStationEmpty::DeleteDeviceObjects(AuGLEffects& re)
{
	DpStation::DeleteDeviceObjects(re);
	m_ScreenName = QString();
	m_ScreenFile = 0;
	m_ScreenTex = 0;					// DaAtom deletes all textures used by the owner's atom id.
}

void DpStationEmpty::ConfigureControls(const DiControlInputArray &aAllControls)
{
	// Update my controls with those passed to me.
	for(DiControlInputArray::const_iterator ci = aAllControls.cbegin(); ci != aAllControls.cend(); ci++)
	{
		// Locate the corresponding control in my controls array.
		for(DiControlInputArray::iterator myci = s_CommandConsoleControls.begin(); myci != s_CommandConsoleControls.end(); myci++)
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

void DpStationEmpty::RestoreDefaultControls()
{
	s_CommandConsoleControls.clear();
	for(int ic=0; ic<s_NumCommandConsoleDefaults; ic++)
		s_CommandConsoleControls.append(s_CommandConsoleDefaults[ic]);
	SaveControls();
}

void DpStationEmpty::LoadControls() const
{
	// Init with my default controls.
	s_CommandConsoleControls.clear();
	for(int ic=0; ic<s_NumCommandConsoleDefaults; ic++)
		s_CommandConsoleControls.append(s_CommandConsoleDefaults[ic]);

	// Load any overrides from the settings file.
	s_CommandConsoleControls.LoadSettings("Controls/Command Console");
}

void DpStationEmpty::SaveControls() const
{
	// Store my controls in the settings file.
	s_CommandConsoleControls.SaveSettings("Controls/Command Console");
}

// A scene atom should fill this array with control inputs it can accept.
void DpStationEmpty::EnumControlInputs(DiControlInputArray &aAllControls) const
{
	// Load my static controls array if it hasn't been loaded yet. This data will only exist on the client since EnumControlInputs
	// is not called on the server.
	if(s_CommandConsoleControls.isEmpty())
		LoadControls();

	// Append my controls to the array.
	for(DiControlInputArray::const_iterator ci = s_CommandConsoleControls.cbegin(); ci != s_CommandConsoleControls.cend();
			ci++)
		aAllControls.append(*ci);
}

// This is how all control inputs are passed to an atom.
// Only control inputs that have been returned by EnumControlInputs will be // passed.
// Only control inputs that have been activated will be passed.
void DpStationEmpty::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaVessel& ship,
		CONTROL_CONTEXT cc, const AuPoint& /*location*/)
{
	// None of the actions of this console can be performed on a "nu" ship.
	if(ship.IsNew())
		return;

	for(int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch(ci.GetId())
		{
		case CI_RemoteCameraOnOff:
			if(cc != CONTROL_Server)
			{
				if(RemoteCameraSwitch(ship))
				{
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_RemoteCameraSwitch, 1, 1);
					sceneAtom->AddEffect(buttonClick);
				}
			}
			break;
		case CI_RemoteCameraNext:
			if(cc != CONTROL_Server)
			{
				if(RemoteCameraNext(ship))
				{
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_RemoteCameraNext, 1, 1);
					sceneAtom->AddEffect(buttonClick);
				}
			}
			break;
		case CI_RemoteCameraPrevious:
			if(cc != CONTROL_Server)
			{
				if(RemoteCameraPrevious(ship))
				{
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_RemoteCameraPrevious, 1, 1);
					sceneAtom->AddEffect(buttonClick);
				}
			}
			break;

		case CI_CloseHullDoors:
			if(cc != CONTROL_Client)
				ship.CloseAllHullDoors();
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_CloseDoors, 1, 1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_DoorRemote:
			if(cc != CONTROL_Client)
			{
				if(ship.IsInsideSpacecraft())
				{
					// This spacecraft is inside a spacecraft.
					// Hull doors are not locked to people inside the hull.
					DaCapsule* inship = ship.FindParentCapsuleInEnv();
					if(!inship)
						break;

					// Toggle the state of the large hull doors in the room I'm in that is nearest to me.
					unsigned int doorIndex = inship->RemoteDoorIndex(false, ship.Location(), ship.ShipRoomId());
					inship->ToggleHullDoor(doorIndex);
				}
				else
				{
					// This vehicle is not inside a spacecraft.
					DaAtom* system = ship.FindEnvironmentAtom();
					if(!system)
						break;	// Not likely.

					// Determine my location relative to the solar system.
					AuPoint mysysloc = ship.EnvLocation();
					DaAtomList ships;
					system->FindCapsulesRecursive(ships);
					if(ships.isEmpty())
						break;	// No ships in the system.

					// Find the closest non-enemy ship to this vehicle.
					// Hull doors always act locked to enemy units.
					DaCapsule* closeship = 0;
					double closedist = 0;
					AuMatrix4x4 tcloseship;
					for(DaAtomList::const_iterator si = ships.begin(); si != ships.end(); si++)
					{
						// Skip myself.
						if((*si)->Is(ship.Id()))
							continue;

						POLITICAL_STANCE stance = ((DaCapsule*)(*si))->PoliticalStanceMakeContact(&ship);
						if(stance == POL_Enemy)
							continue;	// Enemy ships will not open their doors.

						const AuMatrix4x4& tship = (*si)->EnvTransform();
						AuPoint shipsysloc = tship.Project(AuPoint(0,0));
						if(!closeship)
						{
							closeship = (DaCapsule*)(*si);
							tcloseship = tship;
							closedist = shipsysloc.Distance(mysysloc);
						}
						else
						{
							double dist = shipsysloc.Distance(mysysloc);
							if(dist < closedist)
							{
								closeship = (DaCapsule*)(*si);
								tcloseship = tship;
								closedist = dist;
							}
						}
					}
					if(!closeship)
						break;	// No non-enemy ship found.
					if(closedist > 1500)
						break;	// Too far away.

					// Convert my system location to be relative to the spacecraft.
					AuPoint myshiploc = tcloseship.UnProject(mysysloc);

					// Locked hull doors can only be operated by members of the crew. That is officers, crew, or troops.
					unsigned int doorIndex = closeship->RemoteDoorIndex(false, myshiploc);
					bool hullDoorLocked = closeship->HullDoorLocked(doorIndex);
					if(hullDoorLocked && !closeship->IsCrewOrTroop(sceneAtom->Id()) && !closeship->InChainOfCommand(sceneAtom))
						break;	// The door is locked to the person.

					// Tell the ship to open the bay door closest to my ship location.
					closeship->ToggleHullDoor(doorIndex);
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_CloseDoors, 1, .9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_HeadlightOnOff:
			if(cc != CONTROL_Client)
			{
				ship.HeadlightToggle(!ship.IsHeadlightOn());
				ship.SetModified();
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Headlight, 1,
						ship.IsHeadlightOn()?.90f:1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		}
	}
}

void DpStationEmpty::Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
		const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari)
{
	DpStation::Render(projection, farProjection, tModel, sceneAtom, seatedCreature, ari);
	if(seatedCreature && seatedCreature->Is(sceneAtom.Id()))
	{
		const DaVessel* ship = sceneAtom.FindParentVesselInEnv();
		if(ship)
		{
			// Draw the next waypoint.
			if(ship->HasWaypoint())
			{
				DeWaypointRoute route;
				route.append(*ship->LockWaypoint(0));
				ship->UnlockWaypoint();
				RenderCourse(route, farProjection, tModel, sceneAtom, ari, false, true);
			}

			// Draw the lock-on reticle.
			if(!ship->LockedBlip().IsNull())
				RenderSensorLock(ship->LockedBlip(), ship->LockedStance(), ship->LockedIsCivilian(), ship->LockedSysLoc(), tModel,
						sceneAtom, ari);
		}
	}
}

quint64 DpStationEmpty::MemoryUsage() const
{
	quint64 mem = DpStation::MemoryUsage();
	mem += sizeof(m_RemoteCameraId);
	mem += sizeof(m_RemoteCameraSwitch);
	mem += StringMemoryUsage(m_ScreenName);
	mem += StringMemoryUsage(QString(m_ScreenFile));
	mem += sizeof(m_ScreenTex);
	return mem;
}

quint16 DpStationEmpty::RemoteCameraId() const
{
	if(!m_RemoteCameraSwitch)
		return 0;
	return m_RemoteCameraId;
}

bool DpStationEmpty::RemoteCameraNext(const DaCapsule& cap)
{
	quint16 camId = cap.RemoteCameraNext(m_RemoteCameraId);
	if(!camId)
		return false;
	if(m_RemoteCameraId == camId)
		return false;
	m_RemoteCameraId = camId;
	g_Au.Settings().writeEntry("Settings/RemoteCamera", m_RemoteCameraId);	// Store on client last selected camera id.
	return true;
}

bool DpStationEmpty::RemoteCameraPrevious(const DaCapsule& cap)
{
	quint16 camId = cap.RemoteCameraPrevious(m_RemoteCameraId);
	if(!camId)
		return false;
	if(m_RemoteCameraId == camId)
		return false;
	m_RemoteCameraId = camId;
	g_Au.Settings().writeEntry("Settings/RemoteCamera", m_RemoteCameraId);	// Store on client last selected camera id.
	return true;
}

bool DpStationEmpty::RemoteCameraSwitch(const DaCapsule& cap)
{
	if(m_RemoteCameraSwitch)
	{
		// Camera switch is on. Turn camera off.
		m_RemoteCameraSwitch = false;
	}
	else
	{
		// Camera switch is not on.
		if(!m_RemoteCameraId)
		{
			// First time through?
			m_RemoteCameraId = g_Au.Settings().readNumEntry("Settings/RemoteCamera", 0);	// Restore last selected camera id.
			if(!cap.RemoteCameraVerify(m_RemoteCameraId))
				m_RemoteCameraId = cap.RemoteCameraNext(0);
			if(!m_RemoteCameraId)
				return false;		// No camera to switch on.
		}
		// Turn camera on.
		m_RemoteCameraSwitch = true;
	}
	return true;
}

// Refresh data from counterpart.
void DpStationEmpty::Refresh(const DpStation* pCon, const DaAtom& parent)
{
	DpStation::Refresh(pCon, parent);		// Refresh base class.

	const DpStationEmpty* pA = (const DpStationEmpty*)pCon;
	//qDebug() << "DpStationEmpty::Refresh camId" << m_RemoteCameraId << pA->m_RemoteCameraId
		//<< "switch" << m_RemoteCameraSwitch << pA->m_RemoteCameraSwitch;
	m_RemoteCameraId = pA->m_RemoteCameraId;
	m_RemoteCameraSwitch = pA->m_RemoteCameraSwitch;
}

