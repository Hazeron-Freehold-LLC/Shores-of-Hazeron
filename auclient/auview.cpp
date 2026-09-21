//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auview.cpp,v 1.9 2022/02/19 18:17:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auview.h"
//
#include "auselectedunit.h"
#include "dabd.h"
#include "dadesignbase.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "daworldll.h"
#include "dgdata.h"

static AuMatrix4x4						s_Matrix4x4;
static AuScSelection					s_ScSelection;
static AuSelectedUnitArray				s_SelectedUnits;

void AuView::DeselectAllUnits()
{
}

QString AuView::DesignAnalysis()
{
	return QString();
}

const AuSelectedUnitArray& AuView::SelectedUnits() const
{
	return s_SelectedUnits;
}

bool AuView::SelectUnit(const AuId& id, const QString& name, int rank, const QString& rankAbbr, const QString& rankName,
		NPC_ROLE npcRole, bool selectIt)
{
	Q_UNUSED(id);
	Q_UNUSED(name);
	Q_UNUSED(rank);
	Q_UNUSED(rankAbbr);
	Q_UNUSED(rankName);
	Q_UNUSED(npcRole);
	Q_UNUSED(selectIt);
	return false;
}

void AuView::SetModalDlgUp(bool isUp)
{
	Q_UNUSED(isUp);
}

void AuView::SetSelectedBd(const AuId& idBd)
{
	Q_UNUSED(idBd);
}

AuId AuView::SpottedAtom() const
{
	return AuId();
}

AuPoint AuView::SpottedLocation() const
{
	return AuPoint();
}

const AuId& AuView::TargetId() const
{
	return AuId::NullId();
}

void AuView::ProcessControls()
{
}

const AuDNABase* AuView::TargetDNA() const
{
	return 0;
}

double AuView::GetLocationInfo(QString& worldName, BODY_TYPE& bodyType, QString& slonglat, QString& scity, QString& sdev,
		INDUSTRYID& iid, QString& shipname, QString& shipinfo, SPACECRAFT_STRUCTURE& shipStruct, QString& sysNameDesc,
		QString& secDesc, QString& secName, QString& oat, QString& wx, QString& wind, QString& sysPos, QString& secPos,
		QString& galPos, QString& galName)
{
	// Lock the root and find the scene atom.
	DaPerson* person;
	LockSceneAtom(&person);
	if(!person)
	{
		iid = IID_None;
		shipStruct = SC_Count;
		bodyType = BODY_TYPE_Count;
		UnlockSceneAtom();
		return 0;
	}

	// I really expect to have a parent solar system.
	const DaSolarSystem* system = person->FindParentSolarSystemInEnv();
	if(system && !system->IsEmptySystem())
	{
		sysNameDesc = system->Name() + " " + system->Description();
		AuPoint sysLoc = person->EnvLocation() / FeetPerAu();
		sysPos = QString("%1, %2, %3").arg(sysLoc.x, 0,'f',4).arg(sysLoc.y, 0,'f',4).arg(sysLoc.z, 0,'f',4);
	}

	double speed, speedV, speedF, speedL, altitude, pitch, roll, heading, longitude, latitude;
	int modeActual = person->FlightInstruments(AuPoint(0,0), 0, speed, speedF, speedL, speedV, altitude, pitch, roll, heading,
			longitude, latitude);

	AuGlobeLLF globeLL(longitude, latitude);
	slonglat = globeLL.ToString();

	//  'modeActual' specifies relativity returned: 0=none, 1=planet, 2=system, 3=sector
	iid = IID_None;
	bodyType = BODY_TYPE_Count;
	const DaWorld* planet = person->FindParentPlanetInEnv();
	switch(modeActual)
	{
	case 0:
		// Not likely.
		// This would only result if I have no parent planet, star, solar system or sector.
		worldName = "";
		slonglat = "";
		break;
	case 1:
		if(!planet)
		{
			worldName = "";
			slonglat = "";
			break;
		}

		// Relative to planet
		bodyType = planet->BodyType();
		worldName = planet->Name();
		if(person->FindParentDesignBaseInEnv())
			break;

		if(planet->EmpireId())
		{
			QString empireName;
			const DgData& gd = g_Au.LockDataForRead();
			gd.EmpireName(planet->EmpireId(), empireName);
			g_Au.UnlockData();
			worldName = QObject::tr("%1 claimed by %2").arg(worldName).arg(empireName);
		}

		iid = person->InIndustry();
		if(person->IsAtBd() || person->IsInBd())
		{
			const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
			if(llworld)
			{
				const DaBd* bd = dynamic_cast<const DaBd*>(llworld->FindIdRecursive(person->BdId()));
				if(bd)
				{
					scity = bd->CityName();
					QString cityEmpire;
					{
						const DgData& gd = g_Au.LockDataForRead();
						gd.EmpireName(bd->EmpireId(), cityEmpire);
						g_Au.UnlockData();
					}

					if(bd->IsMilitary())
					{
						if(scity.isEmpty())
							scity = QObject::tr("%1 base remnant").arg(cityEmpire);
						else
							scity = QObject::tr("%1 base %2").arg(cityEmpire).arg(scity);
					}
					else
					{
						if(scity.isEmpty())
							scity = QObject::tr("%1 unincorporated").arg(cityEmpire);
						else
							scity = QObject::tr("%1 city %2").arg(cityEmpire).arg(scity);
					}

					if(bd->IsUnderConstruction())
						sdev = QObject::tr("%1 Under Construction").arg(bd->Name());
					else
						sdev = bd->Name();
					if(person->IsInsideBd())
						sdev += QObject::tr(", %1").arg(person->BdRoomName());
				}
			}
		}
		else if(!person->InBdCity().IsNull())
		{
			// Must be on a geo road.
			const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
			if(llworld)
			{
				const DaBd* bd = dynamic_cast<const DaBd*>(llworld->FindIdRecursive(person->InBdCity()));
				if(bd)
				{
					scity = bd->CityName();
					QString cityEmpire;
					{
						const DgData& gd = g_Au.LockDataForRead();
						gd.EmpireName(bd->EmpireId(), cityEmpire);
						g_Au.UnlockData();
					}
					if(scity.isEmpty())
						scity = cityEmpire;
					else if(bd->IsMilitary())
						scity = QObject::tr("%1 military %2").arg(cityEmpire).arg(scity);
					else
						scity = QObject::tr("%1 city %2").arg(cityEmpire).arg(scity);

					sdev = QObject::tr("Road");
				}
			}
		}
		break;
	case 2:
		// Relative to system
		bodyType = BODY_SolarSystemSpace;
		worldName = BodyTypeName(bodyType);
		slonglat = "";
		break;
	case 3:
		// Relative to sector
		bodyType = BODY_SectorSpace;
		worldName = BodyTypeName(bodyType);
		slonglat = "";
		break;
	case 4:
		// Relative to star
		{
			const DaStar* star = person->FindParentStarInEnv();
			if(star)
			{
				bodyType = star->BodyType();
				worldName = star->Name();
			}
		}
		break;
	}

	if(person->IsInSpacecraft())
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			shipname = ship->NameWithEmpire();
			shipinfo = ship->LocationDesc(*person);
			shipStruct = ship->Structure();
		}
		else
		{
			shipname = "";
			shipinfo = "";
			shipStruct = SC_Count;
		}
	}
	else
	{
		shipname = "";
		shipinfo = "";
		shipStruct = SC_Count;
	}

	// Sector
	const DaSector* sector = person->FindParentSector();
	if(sector)
	{
		secName = sector->Name();
		secDesc = sector->Description();

		AuMatrix4x4 tsec;
		person->GetTransformRelativeToParent(tsec, sector);
		AuPoint secLoc = tsec.Translation();
		secPos = QString("%1, %2, %3").arg(secLoc.x, 0,'f',4).arg(secLoc.y, 0,'f',4).arg(secLoc.z, 0,'f',4);
	}
	else
	{
		secName = "";
		secDesc = "";
	}

	// Galaxy
	const DaGalaxy* galaxy = person->FindParentGalaxy();
	if(galaxy)
	{
		galName = galaxy->Name();

		AuMatrix4x4 tgal;
		person->GetTransformRelativeToParent(tgal, galaxy);
		AuPoint galLoc = tgal.Translation();
		galPos = QString("%1, %2, %3").arg(galLoc.x, 0,'f',4).arg(galLoc.y, 0,'f',4).arg(galLoc.z, 0,'f',4);
	}

	// Weather information.
	oat = QString("%1" + g_Au.DegreeSymbol()).arg(person->OutsideAirTemp(), 0, 'f', 0);
	float humidity = person->RelativeHumidity();
	if(std::isnan(humidity))
	{
		wx = QString(QObject::tr("%1C, Humidity Undetermined"))
			.arg(oat);
	}
	else
	{
		humidity *= 100;
		if(humidity > 100)
			humidity = 100;
		wx = QString(QObject::tr("%1C, %2% Humidity"))
			.arg(oat)
			.arg(humidity, 0, 'f', 0);
	}
	if(planet && person->IsInAtmosphere())
	{
		QPointF windF;
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
		if(llworld)
			windF = llworld->WindAt(globeLL);
		AuPoint windV(windF.x(), windF.y());
		double speed = round(windV.Length_2D());	// wind speed in MPH
		speed *= KmPerMile();						// wind speed in KPH
		//float pressureInMillibars = DaWorld::AirDensityToBar(person->AirDensity()) * 1000;
		float pressureInMillibars = person->OutsideAirPressure() * 1000;
		if(speed <= 1)
		{
			wind = QObject::tr("Wind Calm, %L1mbars")
				.arg(pressureInMillibars, 0,'f',0);
		}
		else
		{
			double angle = round(windV.ArcTangent_2D() * RAD2DEG);
			angle = 270. - angle;
			while(angle >= 360)
				angle -= 360;
			int angidx = (int)((angle + 11.25) / 22.5);
			QString windirs[17] =
			{
				QObject::tr("N"),
				QObject::tr("NNE"),
				QObject::tr("NE"),
				QObject::tr("ENE"),
				QObject::tr("E"),
				QObject::tr("ESE"),
				QObject::tr("SE"),
				QObject::tr("SSE"),
				QObject::tr("S"),
				QObject::tr("SSW"),
				QObject::tr("SW"),
				QObject::tr("WSW"),
				QObject::tr("W"),
				QObject::tr("WNW"),
				QObject::tr("NW"),
				QObject::tr("N"),
				QObject::tr("ENE"),
			};
			QString sdir = windirs[angidx];
			wind = QObject::tr("Wind %1 at %L2kph, %L3mbars")
				.arg(sdir)
				.arg(speed, 0,'f',0)
				.arg(pressureInMillibars, 0,'f',0);
		}
	}
	else
		wind = "";

	if(person->InArena())
	{
		worldName = "";
		secDesc = "";
		secName = "";
		secPos = "";
		galPos = "";
		galName = "";
	}

	// We have all the data so unlock the scene before performing potentially slow UI draw operation.
	UnlockSceneAtom();
	return heading;
}

