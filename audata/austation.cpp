//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austation.cpp,v 1.19 2022/02/19 18:20:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austation.h"
//
#include "auglobal.h"

//static
QString AuStation::NameInBuilding(TYPE type)
{
	if(type >= TYPE_Count)
		return "";
	QString names[TYPE_Count] =
	{
		QObject::tr("Command Station"),	// Console_Command
		QObject::tr("Engineer Station"),	// Console_Engine
		QObject::tr("Fire Control Station"),	// Console_FireControl
		QObject::tr("Helm"),	// Console_Helm
		QObject::tr("Doctor's Station"),	// Console_Medic
		QObject::tr("Telescope Station"),	// Console_Navigation
		QObject::tr("Surgery Unit"),	// Console_Patient
		QObject::tr("Power Relay Station"),	// Console_Power
		QObject::tr("Radar Station"),	// Console_Sensor
		QObject::tr("Shield Station"),	// Console_Shield
		QObject::tr("Transporter Station"),	// Console_Transporter
		QObject::tr("Command Station"),	// Integrated_Command
		QObject::tr("Engineer Station"),	// Integrated_Engine
		QObject::tr("Fire Control Station"),	// Integrated_FireControl
		QObject::tr("Helm"),	// Integrated_Helm
		QObject::tr("Doctor's Station"),	// Integrated_Medic
		QObject::tr("Telescope Station"),	// Integrated_Navigation
		QObject::tr("Surgery Unit"),	// Integrated_Patient
		QObject::tr("Power Relay Station"),	// Integrated_Power
		QObject::tr("Radar Station"),	// Integrated_Sensor
		QObject::tr("Shield Station"),	// Integrated_Shield
		QObject::tr("Transporter Station"),	// Integrated_Transporter
		QObject::tr("Turret"),	// Turret_Integrated
		QObject::tr("Turret (Floor)"),	// Turret_Bottom
		QObject::tr("Turret (Ceiling)"),	// Turret_Top
		QObject::tr("Turret (Wall)"),	// Turret_0
		QObject::tr("Turret (Wall)"),	// Turret_45
		QObject::tr("Turret (Wall)"),	// Turret_90
		QObject::tr("Turret (Wall)"),	// Turret_135
		QObject::tr("Turret (Wall)"),	// Turret_180
		QObject::tr("Turret (Wall)"),	// Turret_225
		QObject::tr("Turret (Wall)"),	// Turret_270
		QObject::tr("Turret (Wall)"),	// Turret_315
		QObject::tr("Game Table"),	// Table_Game
		QObject::tr("Game Table"),	// Table_Game_0
		QObject::tr("Game Table"),	// Table_Game_45
		QObject::tr("Bed"),	// Berth_Captain
		QObject::tr("Bed"),	// Berth_Crew
		QObject::tr("Bed"),	// Berth_Officer
		QObject::tr("Bed"),	// Berth_Passenger
		QObject::tr("Bed"),	// Berth_Troop
		QObject::tr("Bed"),	// Berth_1_0
		QObject::tr("Bed"),	// Berth_1_45
		QObject::tr("Bed"),	// Berth_1_90
		QObject::tr("Bed"),	// Berth_1_135
		QObject::tr("Bed"),	// Berth_1_180
		QObject::tr("Bed"),	// Berth_1_225
		QObject::tr("Bed"),	// Berth_1_270
		QObject::tr("Bed"),	// Berth_1_315
		QObject::tr("Bunk Bed"),	// Berth_2_0
		QObject::tr("Bunk Bed"),	// Berth_2_45
		QObject::tr("Bunk Bed"),	// Berth_2_90
		QObject::tr("Bunk Bed"),	// Berth_2_135
		QObject::tr("Bunk Bed"),	// Berth_2_180
		QObject::tr("Bunk Bed"),	// Berth_2_225
		QObject::tr("Bunk Bed"),	// Berth_2_270
		QObject::tr("Bunk Bed"),	// Berth_2_315
		QObject::tr("Troop Station"),	// Troop
		QObject::tr("Citizen Post"),	// Citizen
		QObject::tr("Livestock Post"),	// Livestock
		QObject::tr("Designer Station"),	// Integrated_Designer
	};
	return names[type];
}

//static
QString AuStation::NameInSpacecraft(TYPE type)
{
	if(type >= TYPE_Count)
		return "";
	QString names[TYPE_Count] =
	{
		QObject::tr("Command Station"),	// Console_Command
		QObject::tr("Engineer Station"),	// Console_Engine
		QObject::tr("Fire Control Station"),	// Console_FireControl
		QObject::tr("Helm"),	// Console_Helm
		QObject::tr("Medic Station"),	// Console_Medic
		QObject::tr("Navigator Station"),	// Console_Navigation
		QObject::tr("Surgery Unit"),	// Console_Patient
		QObject::tr("Power Relay Station"),	// Console_Power
		QObject::tr("Sensor Station"),	// Console_Sensor
		QObject::tr("Shield Station"),	// Console_Shield
		QObject::tr("Transporter Station"),	// Console_Transporter
		QObject::tr("Command Station"),	// Integrated_Command
		QObject::tr("Engineer Station"),	// Integrated_Engine
		QObject::tr("Fire Control Station"),	// Integrated_FireControl
		QObject::tr("Helm"),	// Integrated_Helm
		QObject::tr("Medic Station"),	// Integrated_Medic
		QObject::tr("Navigator Station"),	// Integrated_Navigation
		QObject::tr("Surgery Unit"),	// Integrated_Patient
		QObject::tr("Power Relay Station"),	// Integrated_Power
		QObject::tr("Sensor Station"),	// Integrated_Sensor
		QObject::tr("Shield Station"),	// Integrated_Shield
		QObject::tr("Transporter Station"),	// Integrated_Transporter
		QObject::tr("Turret"),	// Turret_Integrated
		QObject::tr("Turret (Bottom)"),	// Turret_Bottom
		QObject::tr("Turret (Top)"),	// Turret_Top
		QObject::tr("Turret (Side)"),	// Turret_0
		QObject::tr("Turret (Side)"),	// Turret_45
		QObject::tr("Turret (Side)"),	// Turret_90
		QObject::tr("Turret (Side)"),	// Turret_135
		QObject::tr("Turret (Side)"),	// Turret_180
		QObject::tr("Turret (Side)"),	// Turret_225
		QObject::tr("Turret (Side)"),	// Turret_270
		QObject::tr("Turret (Side)"),	// Turret_315
		QObject::tr("Game Table"),	// Table_Game
		QObject::tr("Game Table"),	// Table_Game_0
		QObject::tr("Game Table"),	// Table_Game_45
		QObject::tr("Captain's Berth"),	// Berth_Captain
		QObject::tr("Crew Berth"),	// Berth_Crew
		QObject::tr("Officer Berth"),	// Berth_Officer
		QObject::tr("Passenger Berth"),	// Berth_Passenger
		QObject::tr("Troop Berth"),	// Berth_Troop
		QObject::tr("Single Berth"),	// Berth_1_0
		QObject::tr("Single Berth"),	// Berth_1_45
		QObject::tr("Single Berth"),	// Berth_1_90
		QObject::tr("Single Berth"),	// Berth_1_135
		QObject::tr("Single Berth"),	// Berth_1_180
		QObject::tr("Single Berth"),	// Berth_1_225
		QObject::tr("Single Berth"),	// Berth_1_270
		QObject::tr("Single Berth"),	// Berth_1_315
		QObject::tr("Double Berth"),	// Berth_2_0
		QObject::tr("Double Berth"),	// Berth_2_45
		QObject::tr("Double Berth"),	// Berth_2_90
		QObject::tr("Double Berth"),	// Berth_2_135
		QObject::tr("Double Berth"),	// Berth_2_180
		QObject::tr("Double Berth"),	// Berth_2_225
		QObject::tr("Double Berth"),	// Berth_2_270
		QObject::tr("Double Berth"),	// Berth_2_315
		QObject::tr("Troop Station"),	// Troop
		QObject::tr("Citizen Post"),	// Citizen
		QObject::tr("Livestock Post"),	// Livestock
		QObject::tr("Designer Station"),	// Integrated_Designer
	};
	return names[type];
}

//static
AuStation::OCCUPANT_POSITION AuStation::OccPosForPose(const AuPose& pose)
{
	switch(pose.Pose())
	{
	case AuPose::Init:
	case AuPose::Beckon:
	case AuPose::Block:
		break;
	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceDown:
	case AuPose::BerthTopFaceLeft:
	case AuPose::BerthTopFaceRight:
	case AuPose::BerthTopFaceUp:
	case AuPose::Patient_Prone:
		return AuStation::Occupant_Prone;
	case AuPose::ConsoleNav:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleTurretTop:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleCommand:
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_45:
	case AuPose::Command_Sit:
	case AuPose::Engine_Sit:
	case AuPose::FireControl_Sit:
	case AuPose::Helm_Sit:
	case AuPose::Medic_Sit:
	case AuPose::Nav_Sit:
	case AuPose::Patient_Sit:
	case AuPose::Post_Sit:
	case AuPose::Power_Sit:
	case AuPose::Sensor_Sit:
	case AuPose::Shield_Sit:
	case AuPose::Transporter_Sit:
	case AuPose::Turret_Sit:
		return AuStation::Occupant_Sit;
	case AuPose::Turret_SitSwivel3D:
		return AuStation::Occupant_SitSwivel3D;
	case AuPose::Command_SitStationary:
	case AuPose::Engine_SitStationary:
	case AuPose::FireControl_SitStationary:
	case AuPose::Helm_SitStationary:
	case AuPose::Medic_SitStationary:
	case AuPose::Nav_SitStationary:
	case AuPose::Patient_SitStationary:
	case AuPose::Power_SitStationary:
	case AuPose::Sensor_SitStationary:
	case AuPose::Shield_SitStationary:
	case AuPose::Transporter_SitStationary:
	case AuPose::Turret_SitStationary:
		return AuStation::Occupant_SitStationary;
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::Command_Stand:
	case AuPose::Engine_Stand:
	case AuPose::FireControl_Stand:
	case AuPose::Helm_Stand:
	case AuPose::Medic_Stand:
	case AuPose::Nav_Stand:
	case AuPose::Patient_Stand:
	case AuPose::Post_Stand:
	case AuPose::Power_Stand:
	case AuPose::Sensor_Stand:
	case AuPose::Shield_Stand:
	case AuPose::Transporter_Stand:
	case AuPose::Turret_Stand:
		return AuStation::Occupant_Stand;
	case AuPose::Ladder:
	case AuPose::LadderClimb:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_StandStationary:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_StandStationary:
		return AuStation::Occupant_StandStationary;
	case AuPose::Crouch:
	case AuPose::CrouchCreep:
	case AuPose::DeadFaceDown:
	case AuPose::DeadFaceLeft:
	case AuPose::DeadFaceRight:
	case AuPose::DeadFaceUp:
	case AuPose::Dig:
	case AuPose::Eat:
	case AuPose::Fly:
	case AuPose::FlyFast:
	case AuPose::Glide:
	case AuPose::Hug:
	case AuPose::Jump:
	case AuPose::Kick:
	case AuPose::Lash:
	case AuPose::Point:
	case AuPose::Prone:
	case AuPose::ProneCreep:
	case AuPose::Psionic:
	case AuPose::SitOnGround:
	case AuPose::SitOnGroundCreep:
	case AuPose::Stand:
	case AuPose::StandRun:
	case AuPose::StandWalk:
	case AuPose::Stomp:
	case AuPose::Straddle:
	case AuPose::Strike:
	case AuPose::Swim:
	case AuPose::SwimFast:
	case AuPose::Throw:
	case AuPose::TreadWater:
	case AuPose::Upright:
	case AuPose::Wag:
	case AuPose::Wave:
	case AuPose::WritheFaceDown:
	case AuPose::WritheFaceLeft:
	case AuPose::WritheFaceRight:
	case AuPose::WritheFaceUp:
	case AuPose::ReceptiveMate:
	case AuPose::MateOnBottom:
	case AuPose::MateOnTop:
	case AuPose::Hover:
	case AuPose::Spawn:
	case AuPose::EyesOnly:
	case AuPose::Bite:
		break;
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
		break;
	}
	return AuStation::Occupant_Stand;
}

//static
AuPose AuStation::OpPose(TYPE stationType, OCCUPANT_POSITION occPos, int seatIndex)
{
	switch(stationType)
	{
	case Console_Command:
		return AuPose::ConsoleCommand;
	case Console_Engine:
		return AuPose::ConsoleEngine;
	case Console_FireControl:
		return AuPose::ConsoleFireControl;
	case Console_Helm:
		return AuPose::ConsolePilot;
	case Console_Medic:
		return AuPose::ConsoleSurgery;
	case Console_Navigation:
		return AuPose::ConsoleNav;
	case Console_Patient:
		return AuPose::ConsoleSurgeryUnit;
	case Console_Power:
		return AuPose::ConsolePower;
	case Console_Sensor:
		return AuPose::ConsoleSensor;
	case Console_Shield:
		return AuPose::ConsoleShield;
	case Console_Transporter:
		return AuPose::ConsoleTransporter;
	case Integrated_Command:
		switch(occPos)
		{
		case Occupant_Prone:
		case Occupant_Sit:
		case Occupant_SitSwivel3D:
			return AuPose::Command_Sit;
		case Occupant_SitStationary:
			return AuPose::Command_SitStationary;
		case Occupant_Stand:
			return AuPose::Command_Stand;
		case Occupant_StandStationary:
			return AuPose::Command_StandStationary;
		}
		break;
	case Integrated_Engine:
		switch(occPos)
		{
		case Occupant_Prone:
		case Occupant_Sit:
		case Occupant_SitSwivel3D:
			return AuPose::Engine_Sit;
		case Occupant_SitStationary:
			return AuPose::Engine_SitStationary;
		case Occupant_Stand:
			return AuPose::Engine_Stand;
		case Occupant_StandStationary:
			return AuPose::Engine_StandStationary;
		}
		break;
	case Integrated_FireControl:
		switch(occPos)
		{
		case Occupant_Prone:
		case Occupant_Sit:
		case Occupant_SitSwivel3D:
			return AuPose::FireControl_Sit;
		case Occupant_SitStationary:
			return AuPose::FireControl_SitStationary;
		case Occupant_Stand:
			return AuPose::FireControl_Stand;
		case Occupant_StandStationary:
			return AuPose::FireControl_StandStationary;
		}
		break;
	case Integrated_Helm:
		switch(occPos)
		{
		case Occupant_Prone:
		case Occupant_Sit:
		case Occupant_SitSwivel3D:
			return AuPose::Helm_Sit;
		case Occupant_SitStationary:
			return AuPose::Helm_SitStationary;
		case Occupant_Stand:
			return AuPose::Helm_Stand;
		case Occupant_StandStationary:
			return AuPose::Helm_StandStationary;
		}
		break;
	case Integrated_Medic:
		switch(occPos)
		{
		case Occupant_Prone:
		case Occupant_Sit:
		case Occupant_SitSwivel3D:
			return AuPose::Medic_Sit;
		case Occupant_SitStationary:
			return AuPose::Medic_SitStationary;
		case Occupant_Stand:
			return AuPose::Medic_Stand;
		case Occupant_StandStationary:
			return AuPose::Medic_StandStationary;
		}
		break;
	case Integrated_Navigation:
		switch(occPos)
		{
		case Occupant_Prone:
		case Occupant_Sit:
		case Occupant_SitSwivel3D:
			return AuPose::Nav_Sit;
		case Occupant_SitStationary:
			return AuPose::Nav_SitStationary;
		case Occupant_Stand:
			return AuPose::Nav_Stand;
		case Occupant_StandStationary:
			return AuPose::Nav_StandStationary;
		}
		break;
	case Integrated_Patient:
		switch(occPos)
		{
		case Occupant_Prone:
			return AuPose::Patient_Prone;
		case Occupant_Sit:
		case Occupant_SitSwivel3D:
			return AuPose::Patient_Sit;
		case Occupant_SitStationary:
			return AuPose::Patient_SitStationary;
		case Occupant_Stand:
			return AuPose::Patient_Stand;
		case Occupant_StandStationary:
			return AuPose::Patient_StandStationary;
		}
		break;
	case Integrated_Power:
		switch(occPos)
		{
		case Occupant_Prone:
		case Occupant_Sit:
		case Occupant_SitSwivel3D:
			return AuPose::Power_Sit;
		case Occupant_SitStationary:
			return AuPose::Power_SitStationary;
		case Occupant_Stand:
			return AuPose::Power_Stand;
		case Occupant_StandStationary:
			return AuPose::Power_StandStationary;
		}
		break;
	case Integrated_Sensor:
		switch(occPos)
		{
		case Occupant_Prone:
		case Occupant_Sit:
		case Occupant_SitSwivel3D:
			return AuPose::Sensor_Sit;
		case Occupant_SitStationary:
			return AuPose::Sensor_SitStationary;
		case Occupant_Stand:
			return AuPose::Sensor_Stand;
		case Occupant_StandStationary:
			return AuPose::Sensor_StandStationary;
		}
		break;
	case Integrated_Shield:
		switch(occPos)
		{
		case Occupant_Prone:
		case Occupant_Sit:
		case Occupant_SitSwivel3D:
			return AuPose::Shield_Sit;
		case Occupant_SitStationary:
			return AuPose::Shield_SitStationary;
		case Occupant_Stand:
			return AuPose::Shield_Stand;
		case Occupant_StandStationary:
			return AuPose::Shield_StandStationary;
		}
		break;
	case Integrated_Transporter:
		switch(occPos)
		{
		case Occupant_Prone:
		case Occupant_Sit:
		case Occupant_SitSwivel3D:
			return AuPose::Transporter_Sit;
		case Occupant_SitStationary:
			return AuPose::Transporter_SitStationary;
		case Occupant_Stand:
			return AuPose::Transporter_Stand;
		case Occupant_StandStationary:
			return AuPose::Transporter_StandStationary;
		}
		break;
	case Turret_Integrated:
		switch(occPos)
		{
		case Occupant_Prone:
		case Occupant_Sit:
			return AuPose::Turret_Sit;
		case Occupant_SitStationary:
			return AuPose::Turret_SitStationary;
		case Occupant_SitSwivel3D:
			return AuPose::Turret_SitSwivel3D;
		case Occupant_Stand:
			return AuPose::Turret_Stand;
		case Occupant_StandStationary:
			return AuPose::Turret_StandStationary;
		}
		break;
	case Turret_Bottom:
		return AuPose::ConsoleTurretBottom;
	case Turret_Top:
		return AuPose::ConsoleTurretTop;
	case Turret_0:
		return AuPose::ConsoleTurret0;
	case Turret_45:
		return AuPose::ConsoleTurret45;
	case Turret_90:
		return AuPose::ConsoleTurret90;
	case Turret_135:
		return AuPose::ConsoleTurret135;
	case Turret_180:
		return AuPose::ConsoleTurret180;
	case Turret_225:
		return AuPose::ConsoleTurret225;
	case Turret_270:
		return AuPose::ConsoleTurret270;
	case Turret_315:
		return AuPose::ConsoleTurret315;
	case Table_Game:
	case Table_Game_0:
		switch(seatIndex)
		{
		case 0:
			return AuPose::GameTableSeat0_0;
		case 1:
			return AuPose::GameTableSeat1_0;
		case 2:
			return AuPose::GameTableSeat2_0;
		case 3:
			return AuPose::GameTableSeat3_0;
		}
		break;
	case Table_Game_45:
		switch(seatIndex)
		{
		case 0:
			return AuPose::GameTableSeat0_45;
		case 1:
			return AuPose::GameTableSeat1_45;
		case 2:
			return AuPose::GameTableSeat2_45;
		case 3:
			return AuPose::GameTableSeat3_45;
		}
		break;
	case Berth_Captain:
	case Berth_Crew:
	case Berth_Officer:
	case Berth_Passenger:
	case Berth_Troop:
	case Berth_1_0:
	case Berth_1_45:
	case Berth_1_90:
	case Berth_1_135:
	case Berth_1_180:
	case Berth_1_225:
	case Berth_1_270:
	case Berth_1_315:
	case Berth_2_0:
	case Berth_2_45:
	case Berth_2_90:
	case Berth_2_135:
	case Berth_2_180:
	case Berth_2_225:
	case Berth_2_270:
	case Berth_2_315:
		if(seatIndex == 0)
		{
			// Bottom berth.
			// Select a berth pose.
			AuPose sleepPose[4] =
			{
				AuPose(AuPose::BerthBottomFaceDown),
				AuPose(AuPose::BerthBottomFaceLeft),
				AuPose(AuPose::BerthBottomFaceRight),
				AuPose(AuPose::BerthBottomFaceUp)
			};
			int poseIndex = g_Au.Dice().Roll(1,4,-1);
			return sleepPose[poseIndex];
		}
		else
		{
			// Top berth.
			// Select a berth pose.
			AuPose sleepPose[4] =
			{
				AuPose(AuPose::BerthTopFaceDown),
				AuPose(AuPose::BerthTopFaceLeft),
				AuPose(AuPose::BerthTopFaceRight),
				AuPose(AuPose::BerthTopFaceUp)
			};
			int poseIndex = g_Au.Dice().Roll(1,4,-1);
			return sleepPose[poseIndex];
		}
		break;
	case Citizen:
	case Livestock:
	case Troop:
		switch(occPos)
		{
		case Occupant_Prone:
		case Occupant_Sit:
		case Occupant_SitStationary:
		case Occupant_SitSwivel3D:
			return AuPose::Post_Sit;
		case Occupant_Stand:
		case Occupant_StandStationary:
			return AuPose::Post_Stand;
		}
		break;
	case Integrated_Designer:
	case TYPE_Count:
		break;
	}
	return AuPose::Stand;	// oops
}
