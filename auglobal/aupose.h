//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupose.h,v 1.23 2022/04/14 16:10:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuPose_H
#define AuPose_H

#include "augloballib.h"
#include "aupoint.h"

class AuElapsedTimer;

class AuPose
{
public:
	// Poses that can be assigned to parts of a body.
	// Not all poses are appropriate for all body parts.
	// Console specific poses enable us to determine which console to construct with this body when setting the pose.
	// NOTE: If you add a pose value, update the AuPose::Name function.
	enum POSE
	{
		Init = 0,				// Identifies a body that needs initialization.
		Beckon,					// arm
		Block,					// arm
		ConsoleNav,				// all
		MateOnBottom,
		ConsoleEngine,			// all
		ConsoleFireControl,		// all
		ConsoleSurgery,			// all
		ConsolePilot,			// all
		ConsolePower,			// all
		ConsoleSensor,			// all
		ConsoleShield,			// all
		ConsoleTransporter,		// all
		ConsoleSurgeryUnit,		// all
		ConsoleTurret0,			// all, straight right
		ConsoleTurretTop,		// all
		Crouch,					// arm, leg, torso
		CrouchCreep,			// arm, leg, torso
		DeadFaceDown,			// all
		DeadFaceLeft,			// all
		DeadFaceRight,			// all
		DeadFaceUp,				// all
		Dig,					// arm, front leg, torso
		Eat,					// arm, head, upper torso
		Fly,					// torso, wing
		FlyFast,				// torso, wing
		SitOnGroundCreep,
		Glide,					// torso, wing
		Jump,					// arm, leg, torso
		Kick,					// leg, torso
		Lash,					// tail
		Point,					// arm
		Prone,					// arm, leg, torso
		ProneCreep,				// arm, leg
		Psionic,				// arm, head
		SitOnGround,			// arm, leg
		Stand,					// arm, leg, torso (handfeet on ground)
		StandRun,				// arm, leg, torso
		StandWalk,				// arm, leg, torso
		Stomp,					// leg
		Straddle,				// leg, lower torso
		Strike,					// arm, head, tail, upper torso (lunge)
		Swim,					// arm, leg, torso tail, wings
		SwimFast,				// arm, leg, torso, tail, wings
		Throw,					// arm
		TreadWater,				// arm, leg, torso, tail, wings
		Upright,				// head, tail, upper torso (hands not on ground)
		Wag,					// tail
		Wave,					// arm, tail
		WritheFaceDown,			// all
		WritheFaceLeft,			// all
		WritheFaceRight,		// all
		WritheFaceUp,			// all
		Hover,					// torso, wing
		Spawn,					// awaiting respawn
		EyesOnly,				// HeadOnly body not rendered. e.g. vehicle cockpit
		Bite,
		ConsoleTurretBottom,	// all
		ConsoleTurret45,		// all, ahead-right
		ConsoleTurret90,		// all, straight ahead
		ConsoleTurret135,		// all, ahead-left
		ConsoleTurret180,		// all, straight left
		ConsoleTurret225,		// all, back-left
		ConsoleTurret270,		// all, straight back
		ConsoleTurret315,		// all, back-right
		ConsoleCommand,			// all, back-right
		GameTableSeat0_0,		// seat at unrotated table
		GameTableSeat1_0,		// seat at unrotated table
		GameTableSeat2_0,		// seat at unrotated table
		GameTableSeat3_0,		// seat at unrotated table
		GameTableSeat0_45,		// seat at 45 degree rotated table
		GameTableSeat1_45,		// seat at 45 degree rotated table
		GameTableSeat2_45,		// seat at 45 degree rotated table
		GameTableSeat3_45,		// seat at 45 degree rotated table
		Ladder,					// stationary on a ladder
		LadderClimb,			// actively climing up/down a ladder
		BerthBottomFaceDown,	// all
		BerthBottomFaceLeft,	// all
		BerthBottomFaceRight,	// all
		BerthBottomFaceUp,		// all
		BerthTopFaceDown,		// all
		BerthTopFaceLeft,		// all
		BerthTopFaceRight,		// all
		BerthTopFaceUp,			// all
		Command_Sit,			// all
		Command_SitStationary,	// all
		Command_Stand,			// all
		Command_StandStationary,	// all
		Engine_Sit,				// all
		Engine_SitStationary,	// all
		Engine_Stand,			// all
		Engine_StandStationary,	// all
		FireControl_Sit,		// all
		FireControl_SitStationary,	// all
		FireControl_Stand,		// all
		FireControl_StandStationary,	// all
		Helm_Sit,				// all
		Helm_SitStationary,		// all
		Helm_Stand,				// all
		Helm_StandStationary,	// all
		Medic_Sit,				// all
		Medic_SitStationary,	// all
		Medic_Stand,			// all
		Medic_StandStationary,	// all
		Nav_Sit,				// all
		Nav_SitStationary,		// all
		Nav_Stand,				// all
		Nav_StandStationary,	// all
		Patient_Prone,			// all
		Patient_Sit,			// all
		Patient_SitStationary,	// all
		Patient_Stand,			// all
		Patient_StandStationary,	// all
		Power_Sit,				// all
		Power_SitStationary,	// all
		Power_Stand,			// all
		Power_StandStationary,	// all
		Sensor_Sit,				// all
		Sensor_SitStationary,	// all
		Sensor_Stand,			// all
		Sensor_StandStationary,	// all
		Shield_Sit,				// all
		Shield_SitStationary,	// all
		Shield_Stand,			// all
		Shield_StandStationary,	// all
		Transporter_Sit,		// all
		Transporter_SitStationary,	// all
		Transporter_Stand,		// all
		Transporter_StandStationary,	// all
		Turret_Sit,				// all
		Turret_SitStationary,	// all
		Turret_SitSwivel3D,		// all
		Turret_Stand,			// all
		Turret_StandStationary,	// all
		Post_Sit,
		Post_Stand,
		ReceptiveMate,
		MateOnTop,
		Hug,
		MouthPursed,
		MouthClosed,
		MouthOpen,
		MouthAgape,
		MouthOpenWide,
		MouthYawn,
		MouthBite,
		MouthLaugh,				// :D animated
		MouthYap,
		MouthWhisper,
		MouthTalk,
		MouthJabber,
		MouthYell,
		MouthSmile,				// :)
		MouthGrin,				// :D
		MouthSad,				// D:
		MouthFrown,				// :(
		MouthCry,				// D: animated
		MouthPerplexed,			// :S
		MouthGrim,				// :/
		MouthSardonic,			// :P
	};

private:
	POSE								m_Pose;

public:
	inline AuPose(POSE pose)
	{
		m_Pose = pose;
	}

	inline AuPose()
	{
		m_Pose = Init;
	}

	inline AuPose(const AuPose& c)
	{
		m_Pose = c.m_Pose;
	}

public:
	inline const AuPose& operator=(const AuPose& c)
	{
		m_Pose = c.m_Pose;
		return *this;
	}

	inline bool operator==(const AuPose& pose) const
	{
		return m_Pose == pose.m_Pose;
	}

	inline bool operator!=(const AuPose& pose) const
	{
		return !operator==(pose);
	}

	inline bool operator==(POSE pose) const
	{
		return m_Pose == pose;
	}

	inline bool operator!=(POSE pose) const
	{
		return !operator==(pose);
	}

	inline void operator>>(QDataStream& out) const
	{
		out << (quint8)m_Pose;
	}

	inline void operator<<(QDataStream& in)
	{
		quint8 ui8;
		in >> ui8;
		m_Pose = (POSE)ui8;
	}

public:
	inline unsigned int ConsoleSeatIndex() const
	{
		switch(m_Pose)
		{
		case Init:
		case Beckon:
		case Block:
			break;
		case ConsoleCommand:
		case ConsoleNav:
		case ConsoleEngine:
		case ConsoleFireControl:
		case ConsoleSurgery:
		case ConsolePilot:
		case ConsolePower:
		case ConsoleSensor:
		case ConsoleShield:
		case ConsoleTransporter:
		case ConsoleTurretTop:
		case ConsoleTurretBottom:
		case ConsoleTurret0:
		case ConsoleTurret45:
		case ConsoleTurret90:
		case ConsoleTurret135:
		case ConsoleTurret180:
		case ConsoleTurret225:
		case ConsoleTurret270:
		case ConsoleTurret315:
		case ConsoleSurgeryUnit:
		case Command_Sit:
		case Command_SitStationary:
		case Command_Stand:
		case Command_StandStationary:
		case Engine_Sit:
		case Engine_SitStationary:
		case Engine_Stand:
		case Engine_StandStationary:
		case FireControl_Sit:
		case FireControl_SitStationary:
		case FireControl_Stand:
		case FireControl_StandStationary:
		case Helm_Sit:
		case Helm_SitStationary:
		case Helm_Stand:
		case Helm_StandStationary:
		case Medic_Sit:
		case Medic_SitStationary:
		case Medic_Stand:
		case Medic_StandStationary:
		case Nav_Sit:
		case Nav_SitStationary:
		case Nav_Stand:
		case Nav_StandStationary:
		case Post_Sit:
		case Post_Stand:
		case Power_Sit:
		case Power_SitStationary:
		case Power_Stand:
		case Power_StandStationary:
		case Sensor_Sit:
		case Sensor_SitStationary:
		case Sensor_Stand:
		case Sensor_StandStationary:
		case Shield_Sit:
		case Shield_SitStationary:
		case Shield_Stand:
		case Shield_StandStationary:
		case Transporter_Sit:
		case Transporter_SitStationary:
		case Transporter_Stand:
		case Transporter_StandStationary:
		case Turret_Sit:
		case Turret_SitStationary:
		case Turret_SitSwivel3D:
		case Turret_Stand:
		case Turret_StandStationary:
			return 0;
		case Patient_Prone:
		case Patient_Sit:
		case Patient_SitStationary:
		case Patient_Stand:
		case Patient_StandStationary:
			return 1;
		case Crouch:
		case CrouchCreep:
		case DeadFaceDown:
		case DeadFaceLeft:
		case DeadFaceRight:
		case DeadFaceUp:
		case Dig:
		case Eat:
		case Fly:
		case FlyFast:
		case Glide:
		case Jump:
		case Kick:
		case Lash:
		case Point:
		case Prone:
		case ProneCreep:
		case Psionic:
		case SitOnGround:
		case SitOnGroundCreep:
		case Stand:
		case StandRun:
		case StandWalk:
		case Stomp:
		case Straddle:
		case Strike:
		case Swim:
		case SwimFast:
		case Throw:
		case TreadWater:
		case Upright:
		case Wag:
		case Wave:
		case WritheFaceDown:
		case WritheFaceLeft:
		case WritheFaceRight:
		case WritheFaceUp:
		case ReceptiveMate:
		case MateOnBottom:
		case MateOnTop:
		case Hover:
		case Hug:
		case Spawn:
		case EyesOnly:
		case Bite:
			break;
		case MouthPursed:
		case MouthClosed:
		case MouthOpen:
		case MouthAgape:
		case MouthOpenWide:
		case MouthYawn:
		case MouthBite:
		case MouthLaugh:
		case MouthYap:
		case MouthWhisper:
		case MouthTalk:
		case MouthJabber:
		case MouthYell:
		case MouthSmile:
		case MouthGrin:
		case MouthSad:
		case MouthFrown:
		case MouthCry:
		case MouthPerplexed:
		case MouthGrim:
		case MouthSardonic:
			break;
		case GameTableSeat0_0:
			return 0;
		case GameTableSeat1_0:
			return 1;
		case GameTableSeat2_0:
			return 2;
		case GameTableSeat3_0:
			return 3;
		case GameTableSeat0_45:
			return 0;
		case GameTableSeat1_45:
			return 1;
		case GameTableSeat2_45:
			return 2;
		case GameTableSeat3_45:
			return 3;
		case Ladder:
		case LadderClimb:
			return 0;
		case BerthBottomFaceDown:
		case BerthBottomFaceLeft:
		case BerthBottomFaceRight:
		case BerthBottomFaceUp:
			return 0;
		case BerthTopFaceDown:
		case BerthTopFaceLeft:
		case BerthTopFaceRight:
		case BerthTopFaceUp:
			return 1;
		}
		return 0;
	}

	inline bool HeadOnly() const
	{
		return m_Pose == EyesOnly;
	}

	inline bool IsBerth() const
	{
		switch(m_Pose)
		{
		case BerthBottomFaceDown:
		case BerthBottomFaceLeft:
		case BerthBottomFaceRight:
		case BerthBottomFaceUp:
		case BerthTopFaceDown:
		case BerthTopFaceLeft:
		case BerthTopFaceRight:
		case BerthTopFaceUp:
			return true;
		default:
			return false;
		}
	}

	inline bool IsBiting() const
	{
		return m_Pose == Bite || m_Pose == MouthBite;
	}

	inline bool IsCommandStation() const
	{
		return m_Pose == ConsoleCommand ||
			m_Pose == Command_Sit ||
			m_Pose == Command_SitStationary ||
			m_Pose == Command_Stand ||
			m_Pose == Command_StandStationary;
	}

	// True if in a console pose that requires a DpConsole.
	// Like IsLockedPose except these poses require a DpConsole.
	inline bool IsConsole() const
	{
		switch(m_Pose)
		{
		case Init:
		case Spawn:
		case Beckon:
		case Bite:
		case Block:
		case Crouch:
		case CrouchCreep:
		case DeadFaceDown:
		case DeadFaceLeft:
		case DeadFaceRight:
		case DeadFaceUp:
		case Dig:
		case Eat:
		case EyesOnly:
		case Fly:
		case FlyFast:
		case Glide:
		case Hover:
		case Hug:
		case Jump:
		case Kick:
		case Ladder:
		case LadderClimb:
		case Lash:
		case Point:
		case Post_Sit:
		case Post_Stand:
		case Prone:
		case ProneCreep:
		case Psionic:
		case SitOnGround:
		case SitOnGroundCreep:
		case Stand:
		case StandRun:
		case StandWalk:
		case Stomp:
		case Straddle:
		case Strike:
		case Swim:
		case SwimFast:
		case Throw:
		case TreadWater:
		case Upright:
		case Wag:
		case Wave:
		case WritheFaceDown:
		case WritheFaceLeft:
		case WritheFaceRight:
		case WritheFaceUp:
		case ReceptiveMate:
		case MateOnBottom:
		case MateOnTop:
		case BerthBottomFaceDown:
		case BerthBottomFaceLeft:
		case BerthBottomFaceRight:
		case BerthBottomFaceUp:
		case BerthTopFaceDown:
		case BerthTopFaceLeft:
		case BerthTopFaceRight:
		case BerthTopFaceUp:
		case ConsoleSurgeryUnit:
		case ConsoleTurret0:
		case ConsoleTurret45:
		case ConsoleTurret90:
		case ConsoleTurret135:
		case ConsoleTurret180:
		case ConsoleTurret225:
		case ConsoleTurret270:
		case ConsoleTurret315:
		case ConsoleTurretBottom:
		case ConsoleTurretTop:
		case Patient_Sit:
		case Patient_SitStationary:
		case Patient_Stand:
		case Patient_StandStationary:
		case Patient_Prone:
		case Turret_Sit:
		case Turret_SitStationary:
		case Turret_SitSwivel3D:
		case Turret_Stand:
		case Turret_StandStationary:
			return false;
		case MouthPursed:
		case MouthClosed:
		case MouthOpen:
		case MouthAgape:
		case MouthOpenWide:
		case MouthYawn:
		case MouthBite:
		case MouthLaugh:
		case MouthYap:
		case MouthWhisper:
		case MouthTalk:
		case MouthJabber:
		case MouthYell:
		case MouthSmile:
		case MouthGrin:
		case MouthSad:
		case MouthFrown:
		case MouthCry:
		case MouthPerplexed:
		case MouthGrim:
		case MouthSardonic:
			return false;
		case GameTableSeat0_0:
		case GameTableSeat1_0:
		case GameTableSeat2_0:
		case GameTableSeat3_0:
		case GameTableSeat0_45:
		case GameTableSeat1_45:
		case GameTableSeat2_45:
		case GameTableSeat3_45:
		case ConsoleCommand:
		case ConsoleNav:
		case ConsoleEngine:
		case ConsoleFireControl:
		case ConsoleSurgery:
		case ConsolePilot:
		case ConsolePower:
		case ConsoleSensor:
		case ConsoleShield:
		case ConsoleTransporter:
		case Command_Sit:
		case Command_SitStationary:
		case Command_Stand:
		case Command_StandStationary:
		case Engine_Sit:
		case Engine_SitStationary:
		case Engine_Stand:
		case Engine_StandStationary:
		case FireControl_Sit:
		case FireControl_SitStationary:
		case FireControl_Stand:
		case FireControl_StandStationary:
		case Helm_Sit:
		case Helm_SitStationary:
		case Helm_Stand:
		case Helm_StandStationary:
		case Medic_Sit:
		case Medic_SitStationary:
		case Medic_Stand:
		case Medic_StandStationary:
		case Nav_Sit:
		case Nav_SitStationary:
		case Nav_Stand:
		case Nav_StandStationary:
		case Power_Sit:
		case Power_SitStationary:
		case Power_Stand:
		case Power_StandStationary:
		case Sensor_Sit:
		case Sensor_SitStationary:
		case Sensor_Stand:
		case Sensor_StandStationary:
		case Shield_Sit:
		case Shield_SitStationary:
		case Shield_Stand:
		case Shield_StandStationary:
		case Transporter_Sit:
		case Transporter_SitStationary:
		case Transporter_Stand:
		case Transporter_StandStationary:
			return true;
		}
		return false;
	}

	inline bool IsCrouched() const
	{
		if(m_Pose == Crouch || m_Pose == CrouchCreep)
			return true;
		return false;
	}

	inline bool IsDead() const
	{
		return m_Pose == DeadFaceDown || m_Pose == DeadFaceLeft ||
			m_Pose == DeadFaceRight || m_Pose == DeadFaceUp;
	}

	inline bool IsDying() const
	{
		return m_Pose == WritheFaceDown || m_Pose == WritheFaceLeft ||
			m_Pose == WritheFaceRight || m_Pose == WritheFaceUp;
	}

	inline bool IsEating() const
	{
		return m_Pose == Eat;
	}

	inline bool IsEngineStation() const
	{
		return m_Pose == ConsoleEngine ||
			m_Pose == Engine_Sit ||
			m_Pose == Engine_SitStationary ||
			m_Pose == Engine_Stand ||
			m_Pose == Engine_StandStationary;
	}

	inline bool IsFastMoving() const
	{
		return m_Pose == StandRun ||
			m_Pose == FlyFast ||
			m_Pose == SwimFast;
	}

	inline bool IsFireControlStation() const
	{
		return m_Pose == ConsoleFireControl ||
			m_Pose == FireControl_Sit ||
			m_Pose == FireControl_SitStationary ||
			m_Pose == FireControl_Stand ||
			m_Pose == FireControl_StandStationary;
	}

	inline bool IsFlying() const
	{
		if(m_Pose == Fly ||
				m_Pose == FlyFast ||
				m_Pose == Glide ||
				m_Pose == Hover)
			return true;
		return false;
	}

	inline bool IsGameTable() const
	{
		switch(m_Pose)
		{
		case GameTableSeat0_0:
		case GameTableSeat1_0:
		case GameTableSeat2_0:
		case GameTableSeat3_0:
		case GameTableSeat0_45:
		case GameTableSeat1_45:
		case GameTableSeat2_45:
		case GameTableSeat3_45:
			return true;
		default:
			return false;
		}
	}

	inline bool IsGeneral() const
	{
		switch(m_Pose)
		{
		case Beckon:
		case Bite:
		case Block:
		case Eat:
		case Hug:
		case Kick:
		case Lash:
		case Point:
		case Psionic:
		case Stomp:
		case Strike:
		case Throw:
		case Upright:
		case Wag:
		case Wave:
		case MouthPursed:
		case MouthClosed:
		case MouthOpen:
		case MouthAgape:
		case MouthOpenWide:
		case MouthYawn:
		case MouthBite:
		case MouthLaugh:
		case MouthYap:
		case MouthWhisper:
		case MouthTalk:
		case MouthJabber:
		case MouthYell:
		case MouthSmile:
		case MouthGrin:
		case MouthSad:
		case MouthFrown:
		case MouthCry:
		case MouthPerplexed:
		case MouthGrim:
		case MouthSardonic:
			break;						// Invalid "general" pose to set.

		case Init:
		case BerthBottomFaceDown:
		case BerthBottomFaceLeft:
		case BerthBottomFaceRight:
		case BerthBottomFaceUp:
		case BerthTopFaceDown:
		case BerthTopFaceLeft:
		case BerthTopFaceRight:
		case BerthTopFaceUp:
		case ConsoleCommand:
		case ConsoleNav:
		case ConsoleEngine:
		case ConsoleFireControl:
		case ConsoleSurgery:
		case ConsolePilot:
		case ConsolePower:
		case ConsoleSensor:
		case ConsoleShield:
		case ConsoleTransporter:
		case ConsoleSurgeryUnit:
		case ConsoleTurret0:
		case ConsoleTurret45:
		case ConsoleTurret90:
		case ConsoleTurret135:
		case ConsoleTurret180:
		case ConsoleTurret225:
		case ConsoleTurret270:
		case ConsoleTurret315:
		case ConsoleTurretBottom:
		case ConsoleTurretTop:
		case Crouch:
		case CrouchCreep:
		case DeadFaceDown:
		case DeadFaceLeft:
		case DeadFaceRight:
		case DeadFaceUp:
		case Dig:
		case EyesOnly:
		case Fly:
		case FlyFast:
		case GameTableSeat0_0:
		case GameTableSeat1_0:
		case GameTableSeat2_0:
		case GameTableSeat3_0:
		case GameTableSeat0_45:
		case GameTableSeat1_45:
		case GameTableSeat2_45:
		case GameTableSeat3_45:
		case Glide:
		case Hover:
		case Jump:
		case Ladder:
		case LadderClimb:
		case Post_Sit:
		case Post_Stand:
		case Prone:
		case ProneCreep:
		case SitOnGround:
		case SitOnGroundCreep:
		case Spawn:
		case Stand:
		case StandRun:
		case StandWalk:
		case Straddle:
		case Swim:
		case SwimFast:
		case TreadWater:
		case WritheFaceDown:
		case WritheFaceLeft:
		case WritheFaceRight:
		case WritheFaceUp:
		case ReceptiveMate:
		case MateOnBottom:
		case MateOnTop:
		case Command_Sit:
		case Command_SitStationary:
		case Command_Stand:
		case Command_StandStationary:
		case Engine_Sit:
		case Engine_SitStationary:
		case Engine_Stand:
		case Engine_StandStationary:
		case FireControl_Sit:
		case FireControl_SitStationary:
		case FireControl_Stand:
		case FireControl_StandStationary:
		case Helm_Sit:
		case Helm_SitStationary:
		case Helm_Stand:
		case Helm_StandStationary:
		case Medic_Sit:
		case Medic_SitStationary:
		case Medic_Stand:
		case Medic_StandStationary:
		case Nav_Sit:
		case Nav_SitStationary:
		case Nav_Stand:
		case Nav_StandStationary:
		case Patient_Sit:
		case Patient_SitStationary:
		case Patient_Stand:
		case Patient_StandStationary:
		case Patient_Prone:
		case Power_Sit:
		case Power_SitStationary:
		case Power_Stand:
		case Power_StandStationary:
		case Sensor_Sit:
		case Sensor_SitStationary:
		case Sensor_Stand:
		case Sensor_StandStationary:
		case Shield_Sit:
		case Shield_SitStationary:
		case Shield_Stand:
		case Shield_StandStationary:
		case Transporter_Sit:
		case Transporter_SitStationary:
		case Transporter_Stand:
		case Transporter_StandStationary:
		case Turret_Sit:
		case Turret_SitStationary:
		case Turret_SitSwivel3D:
		case Turret_Stand:
		case Turret_StandStationary:
			return true;
		}
		return false;
	}

	inline bool IsHelmStation() const
	{
		return m_Pose == ConsolePilot ||
			m_Pose == Helm_Sit ||
			m_Pose == Helm_SitStationary ||
			m_Pose == Helm_Stand ||
			m_Pose == Helm_StandStationary;
	}

	inline bool IsIntegratedStation() const
	{
		switch(m_Pose)
		{
		default:
			return false;
		case Command_Sit:
		case Command_SitStationary:
		case Command_Stand:
		case Command_StandStationary:
		case Engine_Sit:
		case Engine_SitStationary:
		case Engine_Stand:
		case Engine_StandStationary:
		case FireControl_Sit:
		case FireControl_SitStationary:
		case FireControl_Stand:
		case FireControl_StandStationary:
		case Helm_Sit:
		case Helm_SitStationary:
		case Helm_Stand:
		case Helm_StandStationary:
		case Medic_Sit:
		case Medic_SitStationary:
		case Medic_Stand:
		case Medic_StandStationary:
		case Nav_Sit:
		case Nav_SitStationary:
		case Nav_Stand:
		case Nav_StandStationary:
		case Patient_Prone:
		case Patient_Sit:
		case Patient_SitStationary:
		case Patient_Stand:
		case Patient_StandStationary:
		case Power_Sit:
		case Power_SitStationary:
		case Power_Stand:
		case Power_StandStationary:
		case Sensor_Sit:
		case Sensor_SitStationary:
		case Sensor_Stand:
		case Sensor_StandStationary:
		case Shield_Sit:
		case Shield_SitStationary:
		case Shield_Stand:
		case Shield_StandStationary:
		case Transporter_Sit:
		case Transporter_SitStationary:
		case Transporter_Stand:
		case Transporter_StandStationary:
		case Turret_Sit:
		case Turret_SitStationary:
		case Turret_SitSwivel3D:
		case Turret_Stand:
		case Turret_StandStationary:
			return true;
		}
	}

	inline bool IsJumping() const
	{
		return m_Pose == Jump;
	}

	inline bool IsLadder() const
	{
		return m_Pose == Ladder ||
			m_Pose == LadderClimb;
	}

	// True if in a console-like pose that may or may not require a DpConsole.
	// Like IsConsolePose except no requirement for a DpConsole.
	bool IsLocked() const;

	inline bool IsMating() const
	{
		return m_Pose == MateOnTop || m_Pose == MateOnBottom;
	}

	inline bool IsMatingOrWaiting() const
	{
		return m_Pose == ReceptiveMate || m_Pose == MateOnTop || m_Pose == MateOnBottom;
	}

	inline bool IsMedicStation() const
	{
		return m_Pose == ConsoleSurgery ||
			m_Pose == Medic_Sit ||
			m_Pose == Medic_SitStationary ||
			m_Pose == Medic_Stand ||
			m_Pose == Medic_StandStationary;
	}

	inline bool IsMouth() const
	{
		switch(m_Pose)
		{
		case MouthPursed:
		case MouthClosed:
		case MouthOpen:
		case MouthAgape:
		case MouthOpenWide:
		case MouthYawn:
		case MouthBite:
		case MouthLaugh:
		case MouthYap:
		case MouthWhisper:
		case MouthTalk:
		case MouthJabber:
		case MouthYell:
		case MouthSmile:
		case MouthGrin:
		case MouthSad:
		case MouthFrown:
		case MouthCry:
		case MouthPerplexed:
		case MouthGrim:
		case MouthSardonic:
			return true;
		default:
			break;
		}
		return false;
	}

	// True for inanimate mouth poses.
	inline bool IsMouthStationary() const
	{
		switch(m_Pose)
		{
		case MouthPursed:
		case MouthClosed:
		case MouthOpen:
		case MouthAgape:
		case MouthOpenWide:
		case MouthSmile:
		case MouthGrin:
		case MouthSad:
		case MouthFrown:
		case MouthPerplexed:
		case MouthGrim:
		case MouthSardonic:
			return true;
		case MouthYawn:
		case MouthBite:
		case MouthLaugh:
		case MouthYap:
		case MouthWhisper:
		case MouthTalk:
		case MouthJabber:
		case MouthYell:
		case MouthCry:
		default:
			break;
		}
		return false;
	}

	inline bool IsMoving() const
	{
		switch(m_Pose)
		{
		case Beckon:
		case Bite:
		case Block:
		case Eat:
		case Kick:
		case Lash:
		case Point:
		case Psionic:
		case Stomp:
		case Strike:
		case Throw:
		case Upright:
		case Wag:
		case Wave:
		case MouthPursed:
		case MouthClosed:
		case MouthOpen:
		case MouthAgape:
		case MouthOpenWide:
		case MouthYawn:
		case MouthBite:
		case MouthLaugh:
		case MouthYap:
		case MouthWhisper:
		case MouthTalk:
		case MouthJabber:
		case MouthYell:
		case MouthSmile:
		case MouthGrin:
		case MouthSad:
		case MouthFrown:
		case MouthCry:
		case MouthPerplexed:
		case MouthGrim:
		case MouthSardonic:
			break;
			Q_ASSERT(false);	// Invalid lower torso pose.
			break;
		case Init:
		case BerthBottomFaceDown:
		case BerthBottomFaceLeft:
		case BerthBottomFaceRight:
		case BerthBottomFaceUp:
		case BerthTopFaceDown:
		case BerthTopFaceLeft:
		case BerthTopFaceRight:
		case BerthTopFaceUp:
		case GameTableSeat0_0:
		case GameTableSeat1_0:
		case GameTableSeat2_0:
		case GameTableSeat3_0:
		case GameTableSeat0_45:
		case GameTableSeat1_45:
		case GameTableSeat2_45:
		case GameTableSeat3_45:
		case ConsoleCommand:
		case ConsoleNav:
		case ConsoleEngine:
		case ConsoleFireControl:
		case ConsoleSurgery:
		case ConsolePilot:
		case ConsolePower:
		case ConsoleSensor:
		case ConsoleShield:
		case ConsoleTransporter:
		case ConsoleSurgeryUnit:
		case ConsoleTurret0:
		case ConsoleTurret45:
		case ConsoleTurret90:
		case ConsoleTurret135:
		case ConsoleTurret180:
		case ConsoleTurret225:
		case ConsoleTurret270:
		case ConsoleTurret315:
		case ConsoleTurretBottom:
		case ConsoleTurretTop:
		case Crouch:
		case DeadFaceDown:
		case DeadFaceLeft:
		case DeadFaceRight:
		case DeadFaceUp:
		case EyesOnly:
		case Hover:
		case Ladder:
		case Post_Sit:
		case Post_Stand:
		case Prone:
		case SitOnGround:
		case Spawn:
		case Stand:
		case Straddle:
		case TreadWater:
		case WritheFaceDown:
		case WritheFaceLeft:
		case WritheFaceRight:
		case WritheFaceUp:
		case ReceptiveMate:
		case MateOnBottom:
		case MateOnTop:
		case Command_Sit:
		case Command_SitStationary:
		case Command_Stand:
		case Command_StandStationary:
		case Engine_Sit:
		case Engine_SitStationary:
		case Engine_Stand:
		case Engine_StandStationary:
		case FireControl_Sit:
		case FireControl_SitStationary:
		case FireControl_Stand:
		case FireControl_StandStationary:
		case Helm_Sit:
		case Helm_SitStationary:
		case Helm_Stand:
		case Helm_StandStationary:
		case Hug:
		case Medic_Sit:
		case Medic_SitStationary:
		case Medic_Stand:
		case Medic_StandStationary:
		case Nav_Sit:
		case Nav_SitStationary:
		case Nav_Stand:
		case Nav_StandStationary:
		case Patient_Sit:
		case Patient_SitStationary:
		case Patient_Stand:
		case Patient_StandStationary:
		case Patient_Prone:
		case Power_Sit:
		case Power_SitStationary:
		case Power_Stand:
		case Power_StandStationary:
		case Sensor_Sit:
		case Sensor_SitStationary:
		case Sensor_Stand:
		case Sensor_StandStationary:
		case Shield_Sit:
		case Shield_SitStationary:
		case Shield_Stand:
		case Shield_StandStationary:
		case Transporter_Sit:
		case Transporter_SitStationary:
		case Transporter_Stand:
		case Transporter_StandStationary:
		case Turret_Sit:
		case Turret_SitStationary:
		case Turret_SitSwivel3D:
		case Turret_Stand:
		case Turret_StandStationary:
			break;	// Stationary
		case CrouchCreep:
		case Dig:
		case Fly:
		case FlyFast:
		case Glide:
		case Jump:
		case LadderClimb:
		case SitOnGroundCreep:
		case StandWalk:
		case StandRun:
		case ProneCreep:
		case Swim:
		case SwimFast:
			return true;
		}
		return false;
	}

	inline bool IsNavStation() const
	{
		return m_Pose == ConsoleNav ||
			m_Pose == Nav_Sit ||
			m_Pose == Nav_SitStationary ||
			m_Pose == Nav_Stand ||
			m_Pose == Nav_StandStationary;
	}

	inline bool IsPatientProne() const
	{
		return m_Pose == Patient_Prone;
	}

	inline bool IsPatientStation() const
	{
		return m_Pose == ConsoleSurgeryUnit ||
			m_Pose == Patient_Sit ||
			m_Pose == Patient_SitStationary ||
			m_Pose == Patient_Stand ||
			m_Pose == Patient_StandStationary ||
			m_Pose == Patient_Prone;
	}

	inline bool IsPost() const
	{
		return m_Pose == Post_Sit ||
			m_Pose == Post_Stand;
	}

	inline bool IsPowerStation() const
	{
		return m_Pose == ConsolePower ||
			m_Pose == Power_Sit ||
			m_Pose == Power_SitStationary ||
			m_Pose == Power_Stand ||
			m_Pose == Power_StandStationary;
	}

	inline bool IsProne() const
	{
		switch(m_Pose)
		{
		default:
			return false;
		case BerthBottomFaceDown:
		case BerthBottomFaceLeft:
		case BerthBottomFaceRight:
		case BerthBottomFaceUp:
		case BerthTopFaceDown:
		case BerthTopFaceLeft:
		case BerthTopFaceRight:
		case BerthTopFaceUp:
		case DeadFaceDown:
		case DeadFaceLeft:
		case DeadFaceRight:
		case DeadFaceUp:
		case Prone:
		case ProneCreep:
		case Swim:
		case SwimFast:
		case WritheFaceDown:
		case WritheFaceLeft:
		case WritheFaceRight:
		case WritheFaceUp:
		case ReceptiveMate:
		case MateOnBottom:
		case MateOnTop:
		case Patient_Prone:
			return true;
		}
	}

	inline bool IsSensorStation() const
	{
		return m_Pose == ConsoleSensor ||
			m_Pose == Sensor_Sit ||
			m_Pose == Sensor_SitStationary ||
			m_Pose == Sensor_Stand ||
			m_Pose == Sensor_StandStationary;
	}

	inline bool IsSittingOnGround() const
	{
		if(m_Pose == SitOnGround || m_Pose == SitOnGroundCreep)
			return true;
		return false;
	}

	inline bool IsShieldStation() const
	{
		return m_Pose == ConsoleShield ||
			m_Pose == Shield_Sit ||
			m_Pose == Shield_SitStationary ||
			m_Pose == Shield_Stand ||
			m_Pose == Shield_StandStationary;
	}

	inline bool IsSwimming() const
	{
		if(m_Pose == Swim ||
				m_Pose == SwimFast ||
				m_Pose == TreadWater)
			return true;
		return false;
	}

	inline bool IsTransporterStation() const
	{
		return m_Pose == ConsoleTransporter ||
			m_Pose == Transporter_Sit ||
			m_Pose == Transporter_SitStationary ||
			m_Pose == Transporter_Stand ||
			m_Pose == Transporter_StandStationary;
	}

	// True if a troop in the 'pose' is manning a station.
	inline bool IsTroopStation() const
	{
		switch(m_Pose)
		{
		case ConsoleTurret0:
		case ConsoleTurret45:
		case ConsoleTurret90:
		case ConsoleTurret135:
		case ConsoleTurret180:
		case ConsoleTurret225:
		case ConsoleTurret270:
		case ConsoleTurret315:
		case ConsoleTurretBottom:
		case ConsoleTurretTop:
		case ConsoleFireControl:
		case ConsoleShield:
		case Turret_Sit:
		case Turret_SitStationary:
		case Turret_SitSwivel3D:
		case Turret_Stand:
		case Turret_StandStationary:
			// These are poses a troop might be in while manning a station.
			return true;
		case Init:
		case Spawn:
		case Beckon:
		case Bite:
		case Block:
		case Crouch:
		case CrouchCreep:
		case DeadFaceDown:
		case DeadFaceLeft:
		case DeadFaceRight:
		case DeadFaceUp:
		case Dig:
		case Eat:
		case EyesOnly:
		case Fly:
		case FlyFast:
		case Glide:
		case Hover:
		case Hug:
		case Jump:
		case Kick:
		case Ladder:
		case LadderClimb:
		case Lash:
		case Point:
		case Post_Sit:
		case Post_Stand:
		case Prone:
		case ProneCreep:
		case Psionic:
		case SitOnGround:
		case SitOnGroundCreep:
		case Stand:
		case StandRun:
		case StandWalk:
		case Stomp:
		case Straddle:
		case Strike:
		case Swim:
		case SwimFast:
		case Throw:
		case TreadWater:
		case Upright:
		case Wag:
		case Wave:
		case WritheFaceDown:
		case WritheFaceLeft:
		case WritheFaceRight:
		case WritheFaceUp:
		case ReceptiveMate:
		case MateOnBottom:
		case MateOnTop:
		case BerthBottomFaceDown:
		case BerthBottomFaceLeft:
		case BerthBottomFaceRight:
		case BerthBottomFaceUp:
		case BerthTopFaceDown:
		case BerthTopFaceLeft:
		case BerthTopFaceRight:
		case BerthTopFaceUp:
		case ConsoleCommand:
		case ConsoleEngine:
		case ConsoleNav:
		case ConsolePilot:
		case ConsolePower:
		case ConsoleSensor:
		case ConsoleSurgery:
		case ConsoleSurgeryUnit:
		case ConsoleTransporter:
		case GameTableSeat0_0:
		case GameTableSeat1_0:
		case GameTableSeat2_0:
		case GameTableSeat3_0:
		case GameTableSeat0_45:
		case GameTableSeat1_45:
		case GameTableSeat2_45:
		case GameTableSeat3_45:
		case Command_Sit:
		case Command_SitStationary:
		case Command_Stand:
		case Command_StandStationary:
		case Engine_Sit:
		case Engine_SitStationary:
		case Engine_Stand:
		case Engine_StandStationary:
		case FireControl_Sit:
		case FireControl_SitStationary:
		case FireControl_Stand:
		case FireControl_StandStationary:
		case Helm_Sit:
		case Helm_SitStationary:
		case Helm_Stand:
		case Helm_StandStationary:
		case Medic_Sit:
		case Medic_SitStationary:
		case Medic_Stand:
		case Medic_StandStationary:
		case Nav_Sit:
		case Nav_SitStationary:
		case Nav_Stand:
		case Nav_StandStationary:
		case Patient_Sit:
		case Patient_SitStationary:
		case Patient_Stand:
		case Patient_StandStationary:
		case Patient_Prone:
		case Power_Sit:
		case Power_SitStationary:
		case Power_Stand:
		case Power_StandStationary:
		case Sensor_Sit:
		case Sensor_SitStationary:
		case Sensor_Stand:
		case Sensor_StandStationary:
		case Shield_Sit:
		case Shield_SitStationary:
		case Shield_Stand:
		case Shield_StandStationary:
		case Transporter_Sit:
		case Transporter_SitStationary:
		case Transporter_Stand:
		case Transporter_StandStationary:
			return false;
		case MouthPursed:
		case MouthClosed:
		case MouthOpen:
		case MouthAgape:
		case MouthOpenWide:
		case MouthYawn:
		case MouthBite:
		case MouthLaugh:
		case MouthYap:
		case MouthWhisper:
		case MouthTalk:
		case MouthJabber:
		case MouthYell:
		case MouthSmile:
		case MouthGrin:
		case MouthSad:
		case MouthFrown:
		case MouthCry:
		case MouthPerplexed:
		case MouthGrim:
		case MouthSardonic:
			break;
		}
		return false;
	}

	inline bool IsTurret() const
	{
		switch(m_Pose)
		{
		case ConsoleTurret0:
		case ConsoleTurret45:
		case ConsoleTurret90:
		case ConsoleTurret135:
		case ConsoleTurret180:
		case ConsoleTurret225:
		case ConsoleTurret270:
		case ConsoleTurret315:
		case ConsoleTurretBottom:
		case ConsoleTurretTop:
		case Turret_Sit:
		case Turret_SitStationary:
		case Turret_SitSwivel3D:
		case Turret_Stand:
		case Turret_StandStationary:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsTurretIntegrated() const
	{
		switch(m_Pose)
		{
		case Turret_Sit:
		case Turret_SitStationary:
		case Turret_SitSwivel3D:
		case Turret_Stand:
		case Turret_StandStationary:
			return true;
		default:
			break;
		}
		return false;
	}

	// Returns period of a mouth pose, in milliseconds.
	// Other poses could be added.
	unsigned int Period() const;

	AuPoint JumpVector(float jumpForce) const;

	QString Name() const;

	// Returns noise level due to pose, in the range 0 to 1.
	float NoiseLevel() const;

	inline POSE Pose() const
	{
		return m_Pose;
	}

	// Returns a number from -1 to 1 that represents the current moment in the body's rhythm cycle, which is based on its speed.
	// 'bodyHz' is the body's movement speed in cycles per second.
	// 'cycleOffset' is the number of seconds to offset the rhythm cycle.
	float Rhythm(float bodyHz, float cycleOffset, const AuElapsedTimer& when) const;

	// Returns stamina trend based on body pose in stamina points per second.
	// Positive represents a rejuvenation of stamina.
	// Negative represents a drain of stamina.
	int StaminaTrend() const;

	// stationary version of a pose
	inline AuPose Stationary() const
	{
		switch(m_Pose)
		{
		case Init:
		case Beckon:
		case Bite:
		case Block:
		case BerthBottomFaceDown:
		case BerthBottomFaceLeft:
		case BerthBottomFaceRight:
		case BerthBottomFaceUp:
		case BerthTopFaceDown:
		case BerthTopFaceLeft:
		case BerthTopFaceRight:
		case BerthTopFaceUp:
		case GameTableSeat0_0:
		case GameTableSeat1_0:
		case GameTableSeat2_0:
		case GameTableSeat3_0:
		case GameTableSeat0_45:
		case GameTableSeat1_45:
		case GameTableSeat2_45:
		case GameTableSeat3_45:
		case ConsoleCommand:
		case ConsoleNav:
		case ConsoleEngine:
		case ConsoleFireControl:
		case ConsoleSurgery:
		case ConsolePilot:
		case ConsolePower:
		case ConsoleSensor:
		case ConsoleShield:
		case ConsoleTransporter:
		case ConsoleSurgeryUnit:
		case ConsoleTurret0:
		case ConsoleTurret45:
		case ConsoleTurret90:
		case ConsoleTurret135:
		case ConsoleTurret180:
		case ConsoleTurret225:
		case ConsoleTurret270:
		case ConsoleTurret315:
		case ConsoleTurretBottom:
		case ConsoleTurretTop:
		case Crouch:
		case DeadFaceDown:
		case DeadFaceLeft:
		case DeadFaceRight:
		case DeadFaceUp:
		case Eat:
		case EyesOnly:
		case Glide:
		case Hover:
		case Hug:
		case Kick:
		case Lash:
		case Ladder:
		case Point:
		case Post_Sit:
		case Post_Stand:
		case Prone:
		case Psionic:
		case SitOnGround:
		case Spawn:
		case Stand:
		case Stomp:
		case Straddle:
		case Strike:
		case Throw:
		case TreadWater:
		case Upright:
		case Wag:
		case Wave:
		case WritheFaceDown:
		case WritheFaceLeft:
		case WritheFaceRight:
		case WritheFaceUp:
		case ReceptiveMate:
		case MateOnBottom:
		case MateOnTop:
		case Command_Sit:
		case Command_SitStationary:
		case Command_Stand:
		case Command_StandStationary:
		case Engine_Sit:
		case Engine_SitStationary:
		case Engine_Stand:
		case Engine_StandStationary:
		case FireControl_Sit:
		case FireControl_SitStationary:
		case FireControl_Stand:
		case FireControl_StandStationary:
		case Helm_Sit:
		case Helm_SitStationary:
		case Helm_Stand:
		case Helm_StandStationary:
		case Medic_Sit:
		case Medic_SitStationary:
		case Medic_Stand:
		case Medic_StandStationary:
		case Nav_Sit:
		case Nav_SitStationary:
		case Nav_Stand:
		case Nav_StandStationary:
		case Patient_Sit:
		case Patient_SitStationary:
		case Patient_Stand:
		case Patient_StandStationary:
		case Patient_Prone:
		case Power_Sit:
		case Power_SitStationary:
		case Power_Stand:
		case Power_StandStationary:
		case Sensor_Sit:
		case Sensor_SitStationary:
		case Sensor_Stand:
		case Sensor_StandStationary:
		case Shield_Sit:
		case Shield_SitStationary:
		case Shield_Stand:
		case Shield_StandStationary:
		case Transporter_Sit:
		case Transporter_SitStationary:
		case Transporter_Stand:
		case Transporter_StandStationary:
		case Turret_Sit:
		case Turret_SitStationary:
		case Turret_SitSwivel3D:
		case Turret_Stand:
		case Turret_StandStationary:
		case MouthPursed:
		case MouthClosed:
		case MouthOpen:
		case MouthAgape:
		case MouthOpenWide:
		case MouthYawn:
		case MouthBite:
		case MouthLaugh:
		case MouthYap:
		case MouthWhisper:
		case MouthTalk:
		case MouthJabber:
		case MouthYell:
		case MouthSmile:
		case MouthGrin:
		case MouthSad:
		case MouthFrown:
		case MouthCry:
		case MouthPerplexed:
		case MouthGrim:
		case MouthSardonic:
			// These poses don't really have a corresponding "stationary" pose
			// or they ARE stationary poses.
			break;
		case CrouchCreep:
			return AuPose(Crouch);
		case SitOnGroundCreep:
			return AuPose(SitOnGround);
		case Dig:
			return AuPose(Stand);
		case Fly:
		case FlyFast:
			//if(dna.ArmType == DhDNA::ARM_WingDragonFly ||
					//dna.ArmType == DhDNA::ARM_WingFairy)
				return AuPose(Hover);
			//else
				//return AuPose(Glide);
		case Jump:
			return AuPose(Stand);
		case LadderClimb:
			return AuPose(Ladder);
		case ProneCreep:
			return AuPose(Prone);
		case StandRun:
		case StandWalk:
			return AuPose(Stand);
		case Swim:
		case SwimFast:
			return AuPose(TreadWater);
		}
		return *this;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuPose& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuPose& c)
{
	c << in;
	return in;
}

QDebug operator<<(QDebug dbg, const AuPose& c);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuVisage
{
public:
	enum VISAGE
	{
		Invalid,
		Agape,
		Closed,
		Cry2Sec,
		Frown,
		Grim,
		Grin,
		Laugh1Sec,
		Laugh2Sec,
		Laugh4Sec,
		Open,
		OpenWide,
		Perplexed,
		Pursed,
		Sad,
		Sardonic,
		Smile,
		Yawn,
		FACE_Count
	};

private:
	unsigned int						m_DurMs;				// Duration of the facial expression, in milliseconds.
	AuPose								m_MouthPose;			// Mouth pose for facial expression.

public:
	AuVisage(VISAGE v);

	inline AuVisage(const AuPose& mouthPose, unsigned int durMs)
	{
		m_DurMs = durMs;
		m_MouthPose = mouthPose;
	}

	inline AuVisage()
	{
		m_DurMs = 1000;
		m_MouthPose = AuPose::Init;								// Default construction is not valid.
	}

public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		out << m_DurMs;
		out << m_MouthPose;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_DurMs;
		in >> m_MouthPose;
	}

public:
	inline unsigned int DurationMs() const
	{
		return m_DurMs;
	}

	inline bool IsValid() const
	{
		return MouthPose().IsMouth();
	}

	inline const AuPose& MouthPose() const
	{
		return m_MouthPose;
	}

	QString Name() const;

	inline void SetDurationMs(unsigned int durMs)
	{
		m_DurMs = durMs;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuVisage& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuVisage& c)
{
	c << in;
	return in;
}

#endif // AuPose_H
