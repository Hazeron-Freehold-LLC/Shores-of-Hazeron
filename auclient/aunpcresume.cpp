//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aunpcresume.cpp,v 1.4 2022/02/19 18:16:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aunpcresume.h"
//
#include "auglobal.h"
#include "dabd.h"
#include "daperson.h"
#include "dasentient.h"
#include "davessel.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QTextBrowser>
#include <QVBoxLayout>

AuNpcResume::AuNpcResume(const DaSentient& sent, QWidget* parent) : AuDialog(parent)
{
	QString rankIcon(sent.RankIcon());
	setAuDlgTitle(sent.Name());
	setAuDlgIcon(QIcon(rankIcon));

	QString typeIcon(TTIcon(sent.TransporterTarget()));
	QString resume = tr("<html>"
			"<table><tbody>"
			"<tr>"
			"<td>"
			"<img height=\"32\" src=\"qrc%5\">"
			"</td>"
			"<td>"
			"Name <big><b>%1</b></big>"
			"<br>"
			"Gender <b>%2</b>"
			"<br>"
			"Species <b>%3</b>"
			"<br>"
			"Born <b>%4</b>")
		.arg(sent.AssignedName())
		.arg(GenderName(sent.Gender()))
		.arg(sent.SpeciesName())
		.arg(sent.CreatedTime().UTC().toLocalTime().toString())
		.arg(typeIcon);

	if(sent.IsMarried())
	{
		if(sent.SpouseName().isEmpty())
			resume += tr("<br>"
					"Marital Status <b>Married</b>");
		else
			resume += tr("<br>"
					"Marital Status <b>Married</b>"
					"<br>"
					"Spouse <b>%1</b>")
				.arg(sent.SpouseName());
	}
	else
		resume += tr("<br>"
				"Marital Status <b>Single</b>");

	resume += QString("</td>"
			"</tr>"
			"<tr>"
			"<td>"
			"</td>"
			"<td>");

	// Status
	{
		QString status;
		if(sent.CurrentHitPoints() < sent.MaxHitPoints())
			status = tr("%1/%2hp").arg(sent.CurrentHitPoints()).arg(sent.MaxHitPoints());
		else
			status = tr("%1hp").arg(sent.CurrentHitPoints());
		if(sent.CurrentHitPoints() <= 0)
			status = tr("Dying") + " " + status;
		else if(sent.CurrentHitPoints() < sent.MaxHitPoints())
			status = tr("Wounded") + " " + status;
		else
			status = tr("Unhurt") + " " + status;

		// Determine combat armor factors based on current armor.
		float baseAbsorb, baseBlock, auxAbsorb, auxBlock;
		sent.CombatArmorEffectiveness(WM_Contact, WE_Generic, false, AuPoint(0,0), AuPoint(0,0), baseAbsorb,
				baseBlock, auxAbsorb, auxBlock);
		float absorbK = baseAbsorb + auxAbsorb;
		float blockK = baseBlock + auxBlock;

		resume += tr("Health <b>%1</b>"
				"<br>"
				"Armor <b>%2</b>")
			.arg(status)
			.arg(g_Au.FormatArmorFactors(absorbK, blockK));
	}

	if(rankIcon.isEmpty() || rankIcon == typeIcon)
		resume += QString("<br></td>"
				"</tr>"
				"<tr>"
				"<td>"
				"</td>"
				"<td>");
	else
		resume += QString("<br></td>"
				"</tr>"
				"<tr>"
				"<td>"
				"<img height=\"32\" src=\"qrc%1\">"
				"</td>"
				"<td>")
			.arg(rankIcon);

	// Job
	{
		QString rankAbbr = sent.RankAbbr();
		QString rankName;
		if(!rankAbbr.isEmpty() && rankAbbr != sent.RankName())
			rankName = tr("%1 (%2)").arg(sent.RankName()).arg(rankAbbr);
		else
			rankName = tr("%1").arg(sent.RankName());

		QString empireName;
		const DgData& gd = g_Au.LockDataForRead();
		gd.EmpireName(sent.EmpireId(), empireName);	// Name of empire of allegiance.
		g_Au.UnlockData();
		if(empireName.isEmpty())
			empireName = tr("None");

		quint32 goalxp = sent.PromotionGoal();
		quint32 totalxp = sent.TotalExperiencePoints();
		if(goalxp)
		{
			resume += tr("Rank <b>%3</b>"
					"<br>"
					"Empire <b>%1</b>"
					"<br>"
					"Job <b>%2</b>")
				.arg(empireName)
				.arg(NpcRoleName(sent.NpcRole()))
				.arg(rankName);

			quint32 jobxp = sent.JobExperiencePoints();
			resume += tr("<br>"
					"Next Promotion <b>%L4xp</b>"
					"<br>"
					"Job Experience <b>%L5xp</b>"
					"<br>"
					"Total Experience <b>%L6xp</b>")
				.arg(goalxp)
				.arg(jobxp)
				.arg(totalxp);
		}
		else
		{
			resume += tr("Title <b>%3</b>"
					"<br>"
					"Empire <b>%1</b>"
					"<br>"
					"Role <b>%2</b>"
					"<br>"
					"Total Experience <b>%L4xp</b>")
				.arg(empireName)
				.arg(NpcRoleName(sent.NpcRole()))
				.arg(rankName)
				.arg(totalxp);
		}

		const AuSkillSet& skills = sent.LockSkillsForRead();
		if(skills.Count())
		{
			resume += tr("<br>"
					"<br><big>Skills</big>"
					"<br>"
					"<small>Bold = Job Experience</small>");
			resume += "<table><tbody>";

			QMap<QString, quint32> sortedSkills;
			QMap<QString, QString> skillIcons;
			QSet<QString> jobSkills;
			for(QHash<AuSkill::SKILL, AuSkill>::const_iterator si = skills.Skills().cbegin(); si != skills.Skills().cend(); si++)
			{
				bool isJobSkill = false;
				switch(sent.NpcRole())
				{
				case ROLE_Citizen:
				case ROLE_IndigenousPerson:
				case ROLE_IndigenousElder:
				case ROLE_IndigenousHealer:
				case ROLE_IndigenousStargazer:
				case ROLE_Worker:
				case ROLE_Count:
				case ROLE_Passenger:
					break;
				case ROLE_Crew:
					isJobSkill = (*si).IsCrewExperience();
					break;
				case ROLE_Troop:
					isJobSkill = (*si).IsTroopExperience();
					break;
				case ROLE_Officer:
					isJobSkill = (*si).IsCommandExperience();
					break;
				}
				sortedSkills.insert((*si).Name(), (*si).Experience());
				skillIcons.insert((*si).Name(), (*si).IconName());
				if(isJobSkill)
					jobSkills.insert((*si).Name());
			}

			for(QMap<QString, quint32>::const_iterator si = sortedSkills.cbegin(); si != sortedSkills.cend(); si++)
			{
				bool isJobSkill = jobSkills.contains(si.key());
				if(isJobSkill)
					resume += QString("<tr valign=\"top\">"
							"<td><img height=\"16\" src=\"qrc%1\"> <b>%2</b></td><td align=\"right\"><b>%L3xp</b></td>"
							"</tr>")
						.arg(skillIcons[si.key()])
						.arg(si.key())
						.arg(*si);
				else
					resume += QString("<tr valign=\"top\">"
							"<td><img height=\"16\" src=\"qrc%1\"> %2</td><td align=\"right\">%L3xp</td>"
							"</tr>")
						.arg(skillIcons[si.key()])
						.arg(si.key())
						.arg(*si);
			}
			resume += "</tbody></table>";
		}
		sent.UnlockSkills();
	}

	// Location
	{
		QString location;
		if(sent.IsAboardVehicle())
		{
			location = tr("<br>Aboard <b>%1</b>").arg(sent.Parent()->Name());
		}
		else
		{
			switch(sent.CurrentGeneralPose().Pose())
			{
			case AuPose::Init:
			case AuPose::Beckon:
			case AuPose::Bite:
			case AuPose::Block:
			case AuPose::Eat:
			case AuPose::Hug:
			case AuPose::Kick:
			case AuPose::Lash:
			case AuPose::Point:
			case AuPose::Psionic:
			case AuPose::Stomp:
			case AuPose::Strike:
			case AuPose::Throw:
			case AuPose::Upright:
			case AuPose::Wag:
			case AuPose::Wave:
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
				// These are not "general" poses.
				break;
			case AuPose::Straddle:
			case AuPose::Spawn:
			case AuPose::EyesOnly:
				// These poses don't occur.
				break;
			case AuPose::GameTableSeat0_0:
			case AuPose::GameTableSeat0_45:
				location = tr("<br>Position <b>Game Table Chair 1</b>");
				break;
			case AuPose::GameTableSeat1_0:
			case AuPose::GameTableSeat1_45:
				location = tr("<br>Position <b>Game Table Chair 2</b>");
				break;
			case AuPose::GameTableSeat2_0:
			case AuPose::GameTableSeat2_45:
				location = tr("<br>Position <b>Game Table Chair 3</b>");
				break;
			case AuPose::GameTableSeat3_0:
			case AuPose::GameTableSeat3_45:
				location = tr("<br>Position <b>Game Table Chair 4</b>");
				break;
			case AuPose::ConsoleCommand:
			case AuPose::Command_Sit:
			case AuPose::Command_SitStationary:
			case AuPose::Command_Stand:
			case AuPose::Command_StandStationary:
				location = tr("<br>Position <b>Command Station</b>");
				break;
			case AuPose::ConsoleNav:
			case AuPose::Nav_Sit:
			case AuPose::Nav_SitStationary:
			case AuPose::Nav_Stand:
			case AuPose::Nav_StandStationary:
				location = tr("<br>Position <b>Navigator Station</b>");
				break;
			case AuPose::ConsoleEngine:
			case AuPose::Engine_Sit:
			case AuPose::Engine_SitStationary:
			case AuPose::Engine_Stand:
			case AuPose::Engine_StandStationary:
				location = tr("<br>Position <b>Engineer Station</b>");
				break;
			case AuPose::ConsoleFireControl:
			case AuPose::FireControl_Sit:
			case AuPose::FireControl_SitStationary:
			case AuPose::FireControl_Stand:
			case AuPose::FireControl_StandStationary:
				location = tr("<br>Position <b>Fire Control Station</b>");
				break;
			case AuPose::ConsoleSurgery:
			case AuPose::Medic_Sit:
			case AuPose::Medic_SitStationary:
			case AuPose::Medic_Stand:
			case AuPose::Medic_StandStationary:
				location = tr("<br>Position <b>Medical Station</b>");
				break;
			case AuPose::ConsolePilot:
			case AuPose::Helm_Sit:
			case AuPose::Helm_SitStationary:
			case AuPose::Helm_Stand:
			case AuPose::Helm_StandStationary:
				location = tr("<br>Position <b>Helm</b>");
				break;
			case AuPose::ConsolePower:
			case AuPose::Power_Sit:
			case AuPose::Power_SitStationary:
			case AuPose::Power_Stand:
			case AuPose::Power_StandStationary:
				location = tr("<br>Position <b>Power Relay Station</b>");
				break;
			case AuPose::ConsoleSensor:
			case AuPose::Sensor_Sit:
			case AuPose::Sensor_SitStationary:
			case AuPose::Sensor_Stand:
			case AuPose::Sensor_StandStationary:
				location = tr("<br>Position <b>Sensor Station</b>");
				break;
			case AuPose::ConsoleShield:
			case AuPose::Shield_Sit:
			case AuPose::Shield_SitStationary:
			case AuPose::Shield_Stand:
			case AuPose::Shield_StandStationary:
				location = tr("<br>Position <b>Shield Station</b>");
				break;
			case AuPose::ConsoleSurgeryUnit:
			case AuPose::Patient_Sit:
			case AuPose::Patient_SitStationary:
			case AuPose::Patient_Stand:
			case AuPose::Patient_StandStationary:
			case AuPose::Patient_Prone:
				location = tr("<br>Position <b>Surgery Unit</b>");
				break;
			case AuPose::ConsoleTransporter:
			case AuPose::Transporter_Sit:
			case AuPose::Transporter_SitStationary:
			case AuPose::Transporter_Stand:
			case AuPose::Transporter_StandStationary:
				location = tr("<br>Position <b>Transporter Station</b>");
				break;
			case AuPose::Turret_Sit:
			case AuPose::Turret_SitStationary:
			case AuPose::Turret_SitSwivel3D:
			case AuPose::Turret_Stand:
			case AuPose::Turret_StandStationary:
				location = tr("<br>Position <b>Turret Station</b>");
				break;
			case AuPose::Post_Sit:
			case AuPose::Post_Stand:
				location = tr("<br>Position <b>Guard Post</b>");
				break;
			case AuPose::ConsoleTurret0:
				location = tr("<br>Position <b>Turret Starboard</b>");
				break;
			case AuPose::ConsoleTurret45:
				location = tr("<br>Position <b>Turret Starboard Fore</b>");
				break;
			case AuPose::ConsoleTurret90:
				location = tr("<br>Position <b>Turret Fore</b>");
				break;
			case AuPose::ConsoleTurret135:
				location = tr("<br>Position <b>Turret Port Fore</b>");
				break;
			case AuPose::ConsoleTurret180:
				location = tr("<br>Position <b>Turret Port</b>");
				break;
			case AuPose::ConsoleTurret225:
				location = tr("<br>Position <b>Turret Port Quarter</b>");
				break;
			case AuPose::ConsoleTurret270:
				location = tr("<br>Position <b>Turret Aft</b>");
				break;
			case AuPose::ConsoleTurret315:
				location = tr("<br>Position <b>Turret Starboard Quarter</b>");
				break;
			case AuPose::ConsoleTurretBottom:
				location = tr("<br>Position <b>Turret Below</b>");
				break;
			case AuPose::ConsoleTurretTop:
				location = tr("<br>Position <b>Turret Aloft</b>");
				break;
			case AuPose::Crouch:
				location = tr("<br>Position <b>Crouching</b>");
				break;
			case AuPose::Prone:
				location = tr("<br>Position <b>Lying Prone</b>");
				break;
			case AuPose::SitOnGround:
				location = tr("<br>Position <b>Sitting Down</b>");
				break;
			case AuPose::Stand:
				location = tr("<br>Position <b>Standing</b>");
				break;
			case AuPose::TreadWater:
				location = tr("<br>Position <b>Treading Water</b>");
				break;
			case AuPose::Hover:
				location = tr("<br>Position <b>Hovering</b>");
				break;
			case AuPose::Ladder:
				location = tr("<br>Position <b>Ladder</b>");
				break;
			case AuPose::CrouchCreep:
				location = tr("<br>Position <b>Creeping Crouched</b>");
				break;
			case AuPose::SitOnGroundCreep:
				location = tr("<br>Position <b>Creeping while Sitting</b>");
				break;
			case AuPose::Dig:
				location = tr("<br>Position <b>Digging</b>");
				break;
			case AuPose::Fly:
				location = tr("<br>Position <b>Flying</b>");
				break;
			case AuPose::FlyFast:
				location = tr("<br>Position <b>Flying Fast</b>");
				break;
			case AuPose::Glide:
				location = tr("<br>Position <b>Gliding</b>");
				break;
			case AuPose::Jump:
				location = tr("<br>Position <b>Jumping</b>");
				break;
			case AuPose::LadderClimb:
				location = tr("<br>Position <b>Climing Ladder</b>");
				break;
			case AuPose::ProneCreep:
				location = tr("<br>Position <b>Creeping Prone</b>");
				break;
			case AuPose::StandRun:
				location = tr("<br>Position <b>Running</b>");
				break;
			case AuPose::StandWalk:
				location = tr("<br>Position <b>Walking</b>");
				break;
			case AuPose::Swim:
				location = tr("<br>Position <b>Swimming</b>");
				break;
			case AuPose::SwimFast:
				location = tr("<br>Position <b>Swimming Fast</b>");
				break;
			case AuPose::DeadFaceDown:
				location = tr("<br>Position <b>Dead Facing Down</b>");
				break;
			case AuPose::DeadFaceLeft:
				location = tr("<br>Position <b>Dead Facing Left</b>");
				break;
			case AuPose::DeadFaceRight:
				location = tr("<br>Position <b>Dead Facing Right</b>");
				break;
			case AuPose::DeadFaceUp:
				location = tr("<br>Position <b>Dead Facing Up</b>");
				break;
			case AuPose::WritheFaceDown:
				location = tr("<br>Position <b>Dying Facing Down</b>");
				break;
			case AuPose::WritheFaceLeft:
				location = tr("<br>Position <b>Dying Facing Left</b>");
				break;
			case AuPose::WritheFaceRight:
				location = tr("<br>Position <b>Dying Facing Right</b>");
				break;
			case AuPose::WritheFaceUp:
				location = tr("<br>Position <b>Dying Facing Up</b>");
				break;
			case AuPose::ReceptiveMate:
				location = tr("<br>Position <b>Waiting to Have Sex</b>");
				break;
			case AuPose::MateOnBottom:
				location = tr("<br>Position <b>Having Sex</b>");
				break;
			case AuPose::MateOnTop:
				location = tr("<br>Position <b>Having Sex</b>");
				break;
			case AuPose::BerthBottomFaceDown:
				location = tr("<br>Position <b>Bottom Bunk, Facing Down</b>");
				break;
			case AuPose::BerthBottomFaceLeft:
				location = tr("<br>Position <b>Bottom Bunk, Facing Left</b>");
				break;
			case AuPose::BerthBottomFaceRight:
				location = tr("<br>Position <b>Bottom Bunk, Facing Right</b>");
				break;
			case AuPose::BerthBottomFaceUp:
				location = tr("<br>Position <b>Bottom Bunk, Facing Up</b>");
				break;
			case AuPose::BerthTopFaceDown:
				location = tr("<br>Position <b>Top Bunk, Facing Down</b>");
				break;
			case AuPose::BerthTopFaceLeft:
				location = tr("<br>Position <b>Top Bunk, Facing Left</b>");
				break;
			case AuPose::BerthTopFaceRight:
				location = tr("<br>Position <b>Top Bunk, Facing Right</b>");
				break;
			case AuPose::BerthTopFaceUp:
				location = tr("<br>Position <b>Top Bunk, Facing Up</b>");
				break;
			}
		}

		if(sent.IsInSpacecraft())
		{
			const DaVessel* ship = sent.FindParentVesselInEnv();
			if(ship)
			{
				location += tr("<br>Aboard <b>%1").arg(ship->NameWithEmpire());
				if(ship->IsDesignDataCacheValid())
				{
					ROOMTYPE room = sent.ShipRoomType();
					if(room != ROOM_None)
					{
						QString desc(" " + ship->LocationDesc(sent));
						location += desc;
					}
				}
				location += "</b>";
			}
		}

		const DaWorld* planet = sent.FindParentPlanetInEnv();
		if(planet)
		{
			location += tr("<br>World <b>%1</b>").arg(planet->Name());

			AuMatrix4x4 tp;
			sent.GetTransformRelativeToParent(tp, planet);
			AuPoint planetLoc = tp.Project(AuPoint(0,0));

			QString strLongLat;
			const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
			if(llworld)
			{
				AuGlobeLL ll;
				if(llworld->LLOfLoc(ll, planetLoc))
					strLongLat = ll.ToString();
			}
			location += tr("<br>Coordinates <b>%1</b>").arg(strLongLat);

			if(!sent.BdId().IsNull() && sent.InIndustry() != IID_None)
			{
				// City name
				const DaBd* bd = planet->FindBd(sent.BdId());
				if(bd)
				{
					if(!bd->CityName().isEmpty())
					{
						location += tr("<br>Building <b>%1 %2</b>").arg(bd->CityName()).arg(bd->Name());
					}
					else
					{
						location += tr("<br>Building <b>%1</b>").arg(bd->Name());
					}
				}
			}
		}

		if(!location.isEmpty())
			resume += QString("<br><br><big>Location</big>"
					"%1<br>").arg(location);
	}

	// Home world
	if(!sent.HomeWorldName().isEmpty())
	{
		QString bodyIcon(BodyTypeIcon(sent.HomeWorldBodyType()));
		if(bodyIcon.isEmpty())
			bodyIcon = ":data/City.png";

		resume += QString("</td>"
				"</tr>"
				"<tr>"
				"<td>"
				"<img height=\"32\" src=\"qrc%1\">"
				"</td>"
				"<td>"
				"<big>Home</big><br>")
			.arg(bodyIcon);

		if(!bodyIcon.isEmpty())
			resume += tr("World <b>%1 %2</b>")
				.arg(BodyTypeName(sent.HomeWorldBodyType()))
				.arg(sent.HomeWorldName());
		else
			resume += tr("World <b>%1</b>")
				.arg(sent.HomeWorldName());

		if(!sent.HomeSolarSystemName().isEmpty())
			resume += tr("<br>"
					"Solar System <b>%1</b>")
				.arg(sent.HomeSolarSystemName());

		if(!sent.HomeSectorName().isEmpty())
			resume += tr("<br>"
					"Sector <b>%1</b>")
				.arg(sent.HomeSectorName());

		if(sent.HomeGalaxy() != GALAXY_Count)
			resume += tr("<br>"
					"Galaxy <b>%1</b>")
				.arg(GalaxyName(sent.HomeGalaxy()));
	}

	resume += QString("</td>"
			"</tr>"
			"</tbody></table>");

	resume += QString("</html>");

	QTextBrowser* tb = new QTextBrowser(this);
	tb->setHtml(resume);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(tb);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuNpcResume::~AuNpcResume()
{
}

void AuNpcResume::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuNpcResume::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/NpcResume/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/NpcResume/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/NpcResume/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/NpcResume/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuNpcResume::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/NpcResume/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/NpcResume/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/NpcResume/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/NpcResume/SizeY",sz.height());
}

QSize AuNpcResume::sizeHint() const
{
	return QSize(500,600);
}
