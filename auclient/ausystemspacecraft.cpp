//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausystemspacecraft.cpp,v 1.6 2022/02/19 18:17:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausystemspacecraft.h"
//
#include "auclientglobal.h"
#include "austarmapdata.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>

AuSystemSpacecraft::AuSystemSpacecraft(quint32 empireId, const AuId& sectorId, const AuId& systemId, const QString& systemName,
		QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/Spacecraft.png"));

	QLabel* lb;
	if(systemName.isEmpty())
	{
		setAuDlgTitle(tr("Deep Space Spacecraft Report"));
		lb = new QLabel(tr("<html>"
					"<p>"
					"%1"
					"</p>"
					"<p>"
					"Deep Space"
					"</p>"
					"<p>"
					"<small>Spacecraft status reports may be up to 10 minutes old.</small>"
					"</p>"
					"</html>")
				.arg(QDateTime::currentDateTime().toLocalTime().toString()), this);
	}
	else
	{
		setAuDlgTitle(tr("System Spacecraft Report"));
		lb = new QLabel(tr("<html>"
					"<p>"
					"%1"
					"</p>"
					"<p>"
					"%2"
					"</p>"
					"<p>"
					"<small>Spacecraft status reports may be up to 10 minutes old.</small>"
					"</p>"
					"</html>")
				.arg(QDateTime::currentDateTime().toLocalTime().toString())
				.arg(systemName), this);
	}
	lb->setWordWrap(true);

	QScrollArea* scroll = new QScrollArea;
	{
		QGridLayout* slay = new QGridLayout;

		const AuStarMapData& starMap = g_AuClient.LockStarMapForRead();
		unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI = starMap.m_Sectors.find(sectorId);
		if(sectI != starMap.m_Sectors.cend())
		{
			// Found the star map sector.
			unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator sysI = (*sectI).second.m_Systems.find(systemId);
			if(sysI != (*sectI).second.m_Systems.cend())
			{
				// Found the star map solar system.
				// Sort alphabetically by 'empire structure name'.
				QMap<QString, unordered_map<AuId, AuStarMapSpacecraft, AuIdHash>::const_iterator> sorted;
				for(unordered_map<AuId, AuStarMapSpacecraft, AuIdHash>::const_iterator scI = (*sysI).second.m_Spacecraft.cbegin();
						scI != (*sysI).second.m_Spacecraft.cend(); scI++)
				{
					QString ename;
					const DgData& gd = g_Au.LockDataForRead();
					gd.EmpireName((*scI).second.EmpireId(), ename);
					g_Au.UnlockData();

					QString fc;
					if((*scI).second.IsCompany())
						fc = "B";	// Sort company after fleet.
					else
						fc = "A";	// Sort fleet first.

					QString ss;
					if((*scI).second.IsSpaceStation())
						ss = "C";	// Sort stations last
					else if((*scI).second.HasWarpDrive() || (*scI).second.HasWormholeDrive())
						ss = "A";	// Sort starships first.
					else
						ss = "B";	// Sort spaceships in the middle.

					QString sortName = QString("%1 %2%3 %4")
						.arg(ename)
						.arg(fc)
						.arg(ss)
						.arg((*scI).second.Name()).toUpper();
					sorted.insert(sortName, scI);
				}

				int row=0;
				for(QMap<QString, unordered_map<AuId, AuStarMapSpacecraft, AuIdHash>::const_iterator>::const_iterator si =
						sorted.cbegin(); si != sorted.cend(); si++)
				{
					unordered_map<AuId, AuStarMapSpacecraft, AuIdHash>::const_iterator scI = *si;
					QString ename;
					QImage imFlag;
					const DgData& gd = g_Au.LockDataForRead();
					gd.EmpireName((*scI).second.EmpireId(), ename);
					gd.EmpireFlag((*scI).second.EmpireId(), imFlag);
					g_Au.UnlockData();

					QString fc;
					if((*scI).second.IsCompany())
						fc = tr("Company");
					else
						fc = tr("Fleet");

					QString ss;
					if((*scI).second.IsSpaceStation())
						ss = SpacecraftStructureName(SC_Station);
					else if((*scI).second.HasWarpDrive() || (*scI).second.HasWormholeDrive())
						ss = SpacecraftStructureName(SC_Starship);
					else
						ss = SpacecraftStructureName(SC_Spaceship);

					QLabel* lbflag = new QLabel(this);
					lbflag->setPixmap(QPixmap::fromImage(imFlag));
					lbflag->setAlignment(Qt::AlignTop|Qt::AlignHCenter);

					POLITICAL_STANCE stance = PoliticalStance(empireId, (*scI).second.EmpireId());
					QLabel* lbico = new QLabel(this);
					if((*scI).second.IsSpaceStation())
					{
						switch(stance)
						{
						case POL_Friend:
							lbico->setPixmap(QPixmap(":data/clFStation.png").scaled(32,32,
										Qt::KeepAspectRatio, Qt::SmoothTransformation));
							break;
						case POL_Neutral:
							lbico->setPixmap(QPixmap(":data/clNStation.png").scaled(32,32,
										Qt::KeepAspectRatio, Qt::SmoothTransformation));
							break;
						case POL_Enemy:
							lbico->setPixmap(QPixmap(":data/clEStation.png").scaled(32,32,
										Qt::KeepAspectRatio, Qt::SmoothTransformation));
							break;
						case POL_Empire:
						case POL_Vassal:
							lbico->setPixmap(QPixmap(":data/clMStation.png").scaled(32,32,
										Qt::KeepAspectRatio, Qt::SmoothTransformation));
							break;
						}
					}
					else if((*scI).second.HasFTLDrive())
					{
						switch(stance)
						{
						case POL_Friend:
							lbico->setPixmap(QPixmap(":data/clFStarship.png").scaled(32,32,
										Qt::KeepAspectRatio, Qt::SmoothTransformation));
							break;
						case POL_Neutral:
							lbico->setPixmap(QPixmap(":data/clNStarship.png").scaled(32,32,
										Qt::KeepAspectRatio, Qt::SmoothTransformation));
							break;
						case POL_Enemy:
							lbico->setPixmap(QPixmap(":data/clEStarship.png").scaled(32,32,
										Qt::KeepAspectRatio, Qt::SmoothTransformation));
							break;
						case POL_Empire:
						case POL_Vassal:
							lbico->setPixmap(QPixmap(":data/clMStarship.png").scaled(32,32,
										Qt::KeepAspectRatio, Qt::SmoothTransformation));
							break;
						}
					}
					else
					{
						switch(stance)
						{
						case POL_Friend:
							lbico->setPixmap(QPixmap(":data/clFShip.png").scaled(32,32,
										Qt::KeepAspectRatio, Qt::SmoothTransformation));
							break;
						case POL_Neutral:
							lbico->setPixmap(QPixmap(":data/clNShip.png").scaled(32,32,
										Qt::KeepAspectRatio, Qt::SmoothTransformation));
							break;
						case POL_Enemy:
							lbico->setPixmap(QPixmap(":data/clEShip.png").scaled(32,32,
										Qt::KeepAspectRatio, Qt::SmoothTransformation));
							break;
						case POL_Empire:
						case POL_Vassal:
							lbico->setPixmap(QPixmap(":data/clMShip.png").scaled(32,32,
										Qt::KeepAspectRatio, Qt::SmoothTransformation));
							break;
						}
					}
					lbico->setAlignment(Qt::AlignTop|Qt::AlignHCenter);

					QString estruct;
					switch(stance)
					{
					case POL_Friend:
						estruct = tr("<span style=\"color: rgb(64, 255, 64);\">%1 %2 %3</span>").arg(ename).arg(fc).arg(ss);
						break;
					case POL_Neutral:
						estruct = tr("<span style=\"color: rgb(192, 192, 192);\">%1 %2 %3</span>").arg(ename).arg(fc).arg(ss);
						break;
					case POL_Enemy:
						estruct = tr("<span style=\"color: rgb(255, 32, 32);\">%1 %2 %3</span>").arg(ename).arg(fc).arg(ss);
						break;
					case POL_Empire:
					case POL_Vassal:
						estruct = tr("<span style=\"color: rgb(96, 96, 255);\">%1 %2 %3</span>").arg(ename).arg(fc).arg(ss);
						break;
					}

					QString alert;
					switch((*scI).second.AlertCondition())
					{
					case ConditionGreen:
						alert = tr("<span style=\"color: rgb(0, 255, 0);\">Green</span>");
						break;
					case ConditionYellow:
						alert = tr("<span style=\"color: rgb(255, 255, 0);\">Yellow</span>");
						break;
					case ConditionRed:
						alert = tr("<span style=\"color: rgb(255, 0, 0);\">Red</span>");
						break;
					}

					QString health;
					float h = (*scI).second.Health() * 100 / 255.f;	// Health was scaled to the range of 0 to 255.
					if(h <= 10.f)
						health = tr("<span style=\"color: rgb(255, 51, 51);\">%1%</span>").arg(h, 0,'f',1);
					else if(h <= 50.f)
						health = tr("<span style=\"color: rgb(51, 178, 51);\">%1%</span>").arg(h, 0,'f',1);
					else
						health = tr("<span style=\"color: rgb(51, 255, 51);\">%1%</span>").arg(h, 0,'f',1);

					QString state;
					if((*scI).second.IsMoving())
					{
						if((*scI).second.IsAboardSpacecraft())
							state = tr("Moving Aboard %1").arg((*scI).second.AboardName());
						else if((*scI).second.IsAtBuilding())
							state = tr("Moving in %1").arg((*scI).second.AboardName());
						else if(!(*scI).second.IsInSystem())
							state = tr("Moving in Deep Space");
						else if((*scI).second.IsInAtmosphere())
							state = tr("Moving in Atmosphere");
						else if((*scI).second.IsOrbiting())
							state = tr("Moving Near a World");
						else
							state = tr("Moving in System");
					}
					else
					{
						if((*scI).second.IsAboardSpacecraft())
							state = tr("Stopped Aboard %1").arg((*scI).second.AboardName());
						else if((*scI).second.IsAtBuilding())
							state = tr("Stopped in %1").arg((*scI).second.AboardName());
						else if(!(*scI).second.IsInSystem())
							state = tr("Stopped in Deep Space");
						else if((*scI).second.IsInAtmosphere())
							state = tr("Stopped in Atmosphere");
						else if((*scI).second.IsOrbiting())
							state = tr("Stopped Near a World");
						else
							state = tr("Stopped in System");
					}

					QString ftl;
					if((*scI).second.HasWarpDrive() && (*scI).second.HasWormholeDrive())
					{
						if((*scI).second.IsWarpFTLHot())
							ftl = tr("Warp Drive Hot");
						else if((*scI).second.HasWarpDrive())
							ftl = tr("Warp Drive");

						if((*scI).second.IsWormholeFTLHot())
							ftl += tr(", Wormhole Drive Hot");
						else if((*scI).second.HasWormholeDrive())
							ftl += tr(", Wormhole Drive");
					}
					else if((*scI).second.HasWarpDrive())
					{
						if((*scI).second.IsWarpFTLHot())
							ftl = tr("Warp Drive Hot");
						else if((*scI).second.HasWarpDrive())
							ftl = tr("Warp Drive");
					}
					else if((*scI).second.HasWormholeDrive())
					{
						if((*scI).second.IsWormholeFTLHot())
							ftl = tr("Wormhole Drive Hot");
						else if((*scI).second.HasWormholeDrive())
							ftl = tr("Wormhole Drive");
					}
					else
						ftl = tr("None");

					QString sensors;
					if((*scI).second.HasSensors())
						sensors = tr("Yes");
					else
						sensors = tr("No");

					QString command;
					if((*scI).second.IsNew())
						command = tr("New Spacecraft");
					else if((*scI).second.HasCaptain() && (*scI).second.HasOfficer())
						command = tr("Captain and Officer");
					else if((*scI).second.HasCaptain())
						command = tr("Captain");
					else if((*scI).second.HasOfficer())
						command = tr("Officer");
					else
						command = tr("None");

					QLabel* lbname = new QLabel(QString("<html>"
								"%1<br>"
								"<big><b>%2</b></big><br>"
								"<small>Alert Condition</small> %3<br>"
								"<small>Health</small> %4<br>"
								"<small>Command</small> %8<br>"
								"<small>Status</small> %5<br>"
								"<small>Sensors</small> %6<br>"
								"<small>FTL</small> %7<br>"
								"</html>")
							.arg(estruct)
							.arg((*scI).second.Name())
							.arg(alert)
							.arg(health)
							.arg(state)
							.arg(sensors)
							.arg(ftl)
							.arg(command), this);
					lbname->setWordWrap(true);

					slay->addWidget(lbflag, row,0);
					slay->addWidget(lbico, row+1,0);
					slay->addWidget(lbname, row,1, 2,1);
					row+=2;
				}
				slay->setColumnStretch(1,1);
				slay->setRowStretch(row,1);
			}
		}
		g_AuClient.UnlockStarMap();

		QWidget* wf = new QWidget();
		wf->setLayout(slay);
		scroll->setWidget(wf);
		scroll->setWidgetResizable(true);
	}

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->addWidget(scroll);
	setAuDlgLayout(lay);
}

AuSystemSpacecraft::~AuSystemSpacecraft()
{
}

void AuSystemSpacecraft::setVisible(bool visible)
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

QSize AuSystemSpacecraft::sizeHint() const
{
	return QSize(350,400);
}

void AuSystemSpacecraft::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SystemSpacecraft/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SystemSpacecraft/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SystemSpacecraft/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SystemSpacecraft/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuSystemSpacecraft::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SystemSpacecraft/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SystemSpacecraft/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SystemSpacecraft/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SystemSpacecraft/SizeY",sz.height());
}
