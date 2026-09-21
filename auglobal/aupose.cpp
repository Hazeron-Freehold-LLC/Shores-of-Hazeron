//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupose.cpp,v 1.20 2022/04/14 17:01:11 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aupose.h"
//
#include "aumath.h"
#include "auelapsedtimer.h"
#include <QDebug>

QDebug operator<<(QDebug dbg, const AuPose& c)
{
	dbg.nospace() << c.Name();
	return dbg.space();
}

float AuPose::Rhythm(float bodyHz, float cycleOffset, const AuElapsedTimer& when) const
{
	float speed = 0.f;
	switch(m_Pose)
	{
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
	case ConsoleSurgeryUnit:
	case Command_Sit:
	case Command_SitStationary:
	case Engine_Sit:
	case Engine_SitStationary:
	case FireControl_Sit:
	case FireControl_SitStationary:
	case Helm_Sit:
	case Helm_SitStationary:
	case Hug:
	case Medic_Sit:
	case Medic_SitStationary:
	case Nav_Sit:
	case Nav_SitStationary:
	case Patient_Sit:
	case Patient_SitStationary:
	case Power_Sit:
	case Power_SitStationary:
	case Sensor_Sit:
	case Sensor_SitStationary:
	case Shield_Sit:
	case Shield_SitStationary:
	case Transporter_Sit:
	case Transporter_SitStationary:
	case Turret_Sit:
	case Turret_SitStationary:
	case Turret_SitSwivel3D:
	case Command_Stand:
	case Command_StandStationary:
	case Engine_Stand:
	case Engine_StandStationary:
	case FireControl_Stand:
	case FireControl_StandStationary:
	case Helm_Stand:
	case Helm_StandStationary:
	case Medic_Stand:
	case Medic_StandStationary:
	case Nav_Stand:
	case Nav_StandStationary:
	case Patient_Stand:
	case Patient_StandStationary:
	case Power_Stand:
	case Power_StandStationary:
	case Sensor_Stand:
	case Sensor_StandStationary:
	case Shield_Stand:
	case Shield_StandStationary:
	case Transporter_Stand:
	case Transporter_StandStationary:
	case Turret_Stand:
	case Turret_StandStationary:
	case ConsoleTransporter:
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
	case GameTableSeat0_0:
	case GameTableSeat1_0:
	case GameTableSeat2_0:
	case GameTableSeat3_0:
	case GameTableSeat0_45:
	case GameTableSeat1_45:
	case GameTableSeat2_45:
	case GameTableSeat3_45:
	case Glide:
	case Ladder:
	case Point:
	case Prone:
	case SitOnGround:
	case Spawn:
	case Stand:
	case Straddle:
	case Upright:
	case Post_Sit:
	case Post_Stand:
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
		return 0;
	case Beckon:
	case Bite:
	case Block:
	case Eat:
	case LadderClimb:
	case Psionic:
	case StandWalk:
	case Wave:
		speed = 1;
		break;
	case CrouchCreep:
	case Dig:
	case MateOnBottom:
	case MateOnTop:
	case ReceptiveMate:
	case TreadWater:
		speed = .5f;
		break;
	case SitOnGroundCreep:
		speed = .3f;
		break;
	case Fly:
	case Hover:
		speed = 2.5f;
		break;
	case FlyFast:
		speed = 3.f;
		break;
	case StandRun:
	case Wag:
		speed = 2.f;
		break;
	case Jump:
	case Kick:
	case Lash:
	case Stomp:
	case Strike:
	case Throw:
		speed = 1;
		break;
	case ProneCreep:
		speed = .35f;
		break;
	case Swim:
		speed = .75f;
		break;
	case SwimFast:
		speed = .85f;
		break;
	case WritheFaceDown:
	case WritheFaceLeft:
	case WritheFaceRight:
	case WritheFaceUp:
	case Patient_Prone:
		speed = .1f;
		break;
	}

	float time = when.MinCycle() * 60.f;		// 0..60 seconds
	float cycles = time * bodyHz * speed;				// number of cycles
	float rhythm = sinf(cycles * PIf*2 + cycleOffset);
	return rhythm;
}

int AuPose::StaminaTrend() const
{
	switch(m_Pose)
	{
	case Init:
	case Spawn:
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
		return 0;	// Not a "general" pose.

	case FlyFast:
	case MateOnTop:
	case StandRun:
	case SwimFast:
		// Fast movement uses lots of stamina.
		return -4;
	case MateOnBottom:
		return -3;

	case Dig:
		return -2;

	case ProneCreep:
		return -1;

	case CrouchCreep:
	case DeadFaceDown:
	case DeadFaceLeft:
	case DeadFaceRight:
	case DeadFaceUp:
	case Fly:
	case Jump:			// Jump energy taken when jump initiated.
	case WritheFaceDown:
	case WritheFaceLeft:
	case WritheFaceRight:
	case WritheFaceUp:
	case SitOnGroundCreep:
		return 0;

	// Normal speed movement regains a minimal amount of stamina.
	case Glide:
	case StandWalk:
		return 4;
	case Hover:
	case LadderClimb:
	case Swim:
		return 3;
	case Straddle:
		return 5;

	// Remaining motionless regains stamina faster than walking.
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
	case EyesOnly:
	case SitOnGround:
	case ReceptiveMate:
	case Command_Sit:
	case Command_SitStationary:
	case Engine_Sit:
	case Engine_SitStationary:
	case FireControl_Sit:
	case FireControl_SitStationary:
	case Helm_Sit:
	case Helm_SitStationary:
	case Medic_Sit:
	case Medic_SitStationary:
	case Nav_Sit:
	case Nav_SitStationary:
	case Patient_Sit:
	case Patient_SitStationary:
	case Post_Sit:
	case Power_Sit:
	case Power_SitStationary:
	case Sensor_Sit:
	case Sensor_SitStationary:
	case Shield_Sit:
	case Shield_SitStationary:
	case Transporter_Sit:
	case Transporter_SitStationary:
	case Turret_Sit:
	case Turret_SitStationary:
	case Turret_SitSwivel3D:
		return 9;
	case Stand:
	case Command_Stand:
	case Command_StandStationary:
	case Engine_Stand:
	case Engine_StandStationary:
	case FireControl_Stand:
	case FireControl_StandStationary:
	case Helm_Stand:
	case Helm_StandStationary:
	case Medic_Stand:
	case Medic_StandStationary:
	case Nav_Stand:
	case Nav_StandStationary:
	case Patient_Stand:
	case Patient_StandStationary:
	case Post_Stand:
	case Power_Stand:
	case Power_StandStationary:
	case Sensor_Stand:
	case Sensor_StandStationary:
	case Shield_Stand:
	case Shield_StandStationary:
	case Transporter_Stand:
	case Transporter_StandStationary:
	case Turret_Stand:
	case Turret_StandStationary:
		return 8;
	case Crouch:
	case Ladder:
		return 7;
	case TreadWater:
		return 6;

	// Lying prone regians stamina the fastest.
	case Prone:
	case Patient_Prone:
	case BerthBottomFaceDown:
	case BerthBottomFaceLeft:
	case BerthBottomFaceRight:
	case BerthBottomFaceUp:
	case BerthTopFaceDown:
	case BerthTopFaceLeft:
	case BerthTopFaceRight:
	case BerthTopFaceUp:
		return 10;
	}
	return 0;
}

// Returns noise level due to pose, in the range 0 to 1.
float AuPose::NoiseLevel() const
{
	switch(m_Pose)
	{
	case Init:
	case Spawn:
	case Beckon:
	case Bite:
	case Block:
	case Eat:
	case EyesOnly:
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
		return 0;	// Not a "general" pose.
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
	case Ladder:
	case Straddle:
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
	case ReceptiveMate:
		return .1f;
	case Crouch:
	case DeadFaceDown:
	case DeadFaceLeft:
	case DeadFaceRight:
	case DeadFaceUp:
	case Prone:
	case SitOnGround:
	case Stand:
		return 0;
	case CrouchCreep:
	case Glide:
	case TreadWater:
		return .5f;
	case SitOnGroundCreep:
		return .3f;
	case Dig:
	case FlyFast:
	case Hover:
	case MateOnBottom:
	case MateOnTop:
	case StandWalk:
	case Swim:
		return 1;
	case Fly:
		return .75f;
	case Jump:
	case LadderClimb:
	case StandRun:
	case SwimFast:
		return 1.25f;
	case ProneCreep:
		return .25f;
	case WritheFaceDown:
	case WritheFaceLeft:
	case WritheFaceRight:
	case WritheFaceUp:
	case Patient_Prone:
		return .05f;
	}
	return 1;
}

QString AuPose::Name() const
{
	QString names[] =
	{
		QObject::tr("Init"),
		QObject::tr("Beckon"),
		QObject::tr("Block"),
		QObject::tr("ConsoleNav"),
		QObject::tr("MateOnBottom"),
		QObject::tr("ConsoleEngine"),
		QObject::tr("ConsoleFireControl"),
		QObject::tr("ConsoleSurgery"),
		QObject::tr("ConsolePilot"),
		QObject::tr("ConsolePower"),
		QObject::tr("ConsoleSensor"),
		QObject::tr("ConsoleShield"),
		QObject::tr("ConsoleTransporter"),
		QObject::tr("ConsoleSurgeryUnit"),
		QObject::tr("ConsoleTurret0"),
		QObject::tr("ConsoleTurretTop"),
		QObject::tr("Crouch"),
		QObject::tr("CrouchCreep"),
		QObject::tr("DeadFaceDown"),
		QObject::tr("DeadFaceLeft"),
		QObject::tr("DeadFaceRight"),
		QObject::tr("DeadFaceUp"),
		QObject::tr("Dig"),
		QObject::tr("Eat"),
		QObject::tr("Fly"),
		QObject::tr("FlyFast"),
		QObject::tr("SitOnGroundCreep"),
		QObject::tr("Glide"),
		QObject::tr("Jump"),
		QObject::tr("Kick"),
		QObject::tr("Lash"),
		QObject::tr("Point"),
		QObject::tr("Prone"),
		QObject::tr("ProneCreep"),
		QObject::tr("Psionic"),
		QObject::tr("SitOnGround"),
		QObject::tr("Stand"),
		QObject::tr("StandRun"),
		QObject::tr("StandWalk"),
		QObject::tr("Stomp"),
		QObject::tr("Straddle"),
		QObject::tr("Strike"),
		QObject::tr("Swim"),
		QObject::tr("SwimFast"),
		QObject::tr("Throw"),
		QObject::tr("TreadWater"),
		QObject::tr("Upright"),
		QObject::tr("Wag"),
		QObject::tr("Wave"),
		QObject::tr("WritheFaceDown"),
		QObject::tr("WritheFaceLeft"),
		QObject::tr("WritheFaceRight"),
		QObject::tr("WritheFaceUp"),
		QObject::tr("Hover"),
		QObject::tr("Spawn"),
		QObject::tr("EyesOnly"),
		QObject::tr("Bite"),
		QObject::tr("ConsoleTurretBottom"),
		QObject::tr("ConsoleTurret45"),
		QObject::tr("ConsoleTurret90"),
		QObject::tr("ConsoleTurret135"),
		QObject::tr("ConsoleTurret180"),
		QObject::tr("ConsoleTurret225"),
		QObject::tr("ConsoleTurret270"),
		QObject::tr("ConsoleTurret315"),
		QObject::tr("ConsoleCommand"),
		QObject::tr("GameTableSeat0_0"),
		QObject::tr("GameTableSeat1_0"),
		QObject::tr("GameTableSeat2_0"),
		QObject::tr("GameTableSeat3_0"),
		QObject::tr("GameTableSeat0_45"),
		QObject::tr("GameTableSeat1_45"),
		QObject::tr("GameTableSeat2_45"),
		QObject::tr("GameTableSeat3_45"),
		QObject::tr("Ladder"),
		QObject::tr("LadderClimb"),
		QObject::tr("BerthBottomFaceDown"),
		QObject::tr("BerthBottomFaceLeft"),
		QObject::tr("BerthBottomFaceRight"),
		QObject::tr("BerthBottomFaceUp"),
		QObject::tr("BerthTopFaceDown"),
		QObject::tr("BerthTopFaceLeft"),
		QObject::tr("BerthTopFaceRight"),
		QObject::tr("BerthTopFaceUp"),
		QObject::tr("Command_Sit"),
		QObject::tr("Command_SitStationary"),
		QObject::tr("Command_Stand"),
		QObject::tr("Command_StandStationary"),
		QObject::tr("Engine_Sit"),
		QObject::tr("Engine_SitStationary"),
		QObject::tr("Engine_Stand"),
		QObject::tr("Engine_StandStationary"),
		QObject::tr("FireControl_Sit"),
		QObject::tr("FireControl_SitStationary"),
		QObject::tr("FireControl_Stand"),
		QObject::tr("FireControl_StandStationary"),
		QObject::tr("Helm_Sit"),
		QObject::tr("Helm_SitStationary"),
		QObject::tr("Helm_Stand"),
		QObject::tr("Helm_StandStationary"),
		QObject::tr("Medic_Sit"),
		QObject::tr("Medic_SitStationary"),
		QObject::tr("Medic_Stand"),
		QObject::tr("Medic_StandStationary"),
		QObject::tr("Nav_Sit"),
		QObject::tr("Nav_SitStationary"),
		QObject::tr("Nav_Stand"),
		QObject::tr("Nav_StandStationary"),
		QObject::tr("Patient_Prone"),
		QObject::tr("Patient_Sit"),
		QObject::tr("Patient_SitStationary"),
		QObject::tr("Patient_Stand"),
		QObject::tr("Patient_StandStationary"),
		QObject::tr("Power_Sit"),
		QObject::tr("Power_SitStationary"),
		QObject::tr("Power_Stand"),
		QObject::tr("Power_StandStationary"),
		QObject::tr("Sensor_Sit"),
		QObject::tr("Sensor_SitStationary"),
		QObject::tr("Sensor_Stand"),
		QObject::tr("Sensor_StandStationary"),
		QObject::tr("Shield_Sit"),
		QObject::tr("Shield_SitStationary"),
		QObject::tr("Shield_Stand"),
		QObject::tr("Shield_StandStationary"),
		QObject::tr("Transporter_Sit"),
		QObject::tr("Transporter_SitStationary"),
		QObject::tr("Transporter_Stand"),
		QObject::tr("Transporter_StandStationary"),
		QObject::tr("Turret_Sit"),
		QObject::tr("Turret_SitStationary"),
		QObject::tr("Turret_SitSwivel3D"),
		QObject::tr("Turret_Stand"),
		QObject::tr("Turret_StandStationary"),
		QObject::tr("Post_Sit"),
		QObject::tr("Post_Stand"),
		QObject::tr("ReceptiveMate"),
		QObject::tr("MateOnTop"),
		QObject::tr("Hug"),
		QObject::tr("Mouth Pursed"),	// Mouth
		QObject::tr("Mouth Closed"),	// Mouth
		QObject::tr("Mouth Open"),		// Mouth
		QObject::tr("Mouth Agape"),		// Mouth
		QObject::tr("Mouth Open Wide"),	// Mouth
		QObject::tr("Yawn"),			// Mouth
		QObject::tr("Bite"),			// Mouth
		QObject::tr("Laugh"),			// Mouth
		QObject::tr("Yap"),				// Mouth
		QObject::tr("Whisper"),			// Mouth
		QObject::tr("Talk"),			// Mouth
		QObject::tr("Jabber"),			// Mouth
		QObject::tr("Yell"),			// Mouth
		QObject::tr("Smile"),			// Mouth
		QObject::tr("Grin"),			// Mouth
		QObject::tr("Sad"),				// Mouth
		QObject::tr("Frown"),			// Mouth
		QObject::tr("Cry"),				// Mouth
		QObject::tr("Perplexed"),		// Mouth
		QObject::tr("Grim"),			// Mouth
		QObject::tr("Sardonic"),		// Mouth
	};
	int nameCount = sizeof(names) / sizeof(QString);
	if(m_Pose < 0 || m_Pose >= nameCount)
		return QObject::tr("Bad Pose %1").arg(m_Pose);
	return names[m_Pose];
}

// Returns period of a mouth pose, in milliseconds.
// Other poses could be added.
unsigned int AuPose::Period() const
{
	switch(m_Pose)
	{
	default:
		break;
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
		break;
	case MouthYawn:
		return 8000;
	case MouthBite:
		return 2000;
	case MouthLaugh:
	case MouthCry:
		return 200;
	case MouthYap:
		return 400;
	case MouthWhisper:
	case MouthYell:
		return 30000;
	case MouthTalk:
		return 20000;
	case MouthJabber:
		return 15000;
	}
	return 0;
}

AuPoint AuPose::JumpVector(float jumpForce) const
{
	switch(m_Pose)
	{
	case Init:
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
		break;	// Invalid "general" pose to set.
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
	case DeadFaceDown:
	case DeadFaceLeft:
	case DeadFaceRight:
	case DeadFaceUp:
	case Dig:
	case EyesOnly:
	case Fly:
	case FlyFast:
	case Glide:
	case Hover:
	case Hug:
	case Jump:
	case Post_Sit:
	case Post_Stand:
	case Prone:
	case ProneCreep:
	case SitOnGround:
	case SitOnGroundCreep:
	case Spawn:
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
		// Cannot jump from these poses.
		break;
	case Crouch:
		// Jump straight up.
		// Strength is increased by 10% due to crouched start.
		return AuPoint(0,0,jumpForce*1.1);
	case CrouchCreep:
		// Jump forward at a 45 degree angle.
		// Strength is increased by 10% due to crouched start.
		return AuPoint(0,0,jumpForce*1.1).RotateX(-45.*DEG2RAD);
	case Ladder:
	case LadderClimb:
		// Jump backward weakly off a ladder.
		return AuPoint(0,0,jumpForce * .8f).RotateX(30.*DEG2RAD);
	case Stand:
		// Jump straight up.
		return AuPoint(0,0,jumpForce);
	case StandRun:
		// Jump forward at a 45 degree angle with more force.
		// Strength increased by 20% due to running start.
		return AuPoint(0,0,jumpForce*1.2).RotateX(-45.*DEG2RAD);
	case StandWalk:
		// Jump forward at a 45 degree angle.
		return AuPoint(0,0,jumpForce).RotateX(-45.*DEG2RAD);
	}
	return AuPoint(0,0);
}

bool AuPose::IsLocked() const
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
		return true;
	}
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuVisage::AuVisage(AuVisage::VISAGE v)
{
	AuVisage expressions[] =
	{
		AuVisage(AuPose::Init, 0),	// Invalid
		AuVisage(AuPose::MouthAgape, 0),
		AuVisage(AuPose::MouthClosed, 0),
		AuVisage(AuPose::MouthCry, 2000),
		AuVisage(AuPose::MouthFrown, 0),
		AuVisage(AuPose::MouthGrim, 0),
		AuVisage(AuPose::MouthGrin, 0),
		AuVisage(AuPose::MouthLaugh, 1000),
		AuVisage(AuPose::MouthLaugh, 2000),
		AuVisage(AuPose::MouthLaugh, 4000),
		AuVisage(AuPose::MouthOpen, 0),
		AuVisage(AuPose::MouthOpenWide, 0),
		AuVisage(AuPose::MouthPerplexed, 0),
		AuVisage(AuPose::MouthPursed, 0),
		AuVisage(AuPose::MouthSad, 0),
		AuVisage(AuPose::MouthSardonic, 0),
		AuVisage(AuPose::MouthSmile, 0),
		AuVisage(AuPose::MouthYawn, 8000),
	};
	m_DurMs = expressions[v].DurationMs();
	m_MouthPose = expressions[v].MouthPose();
}

QString AuVisage::Name() const
{
	if(!m_MouthPose.IsMouth())
		return QObject::tr("Invalid");
	else if(m_MouthPose.IsMouthStationary())
		return m_MouthPose.Name();
	return QObject::tr("%1 %2 sec").arg(m_MouthPose.Name()).arg(m_DurMs/1000.f);
}

