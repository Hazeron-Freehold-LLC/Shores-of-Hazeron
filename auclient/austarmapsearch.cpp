//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austarmapsearch.cpp,v 1.7 2022/02/19 18:17:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austarmapsearch.h"
//
#include "auclientglobal.h"
#include "dasolarsystem.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

AuStarMapSearch::AuStarMapSearch(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Star Map Name Search"));
	setAuDlgIcon(QIcon(":data/MsgChristen.png"));

	QString lastSearch = g_Au.Settings().readEntry("Widget/StarMapSearch/LastSearch","");
	bool lastCase = g_Au.Settings().readBoolEntry("Widget/StarMapSearch/LastCase",true);
	bool lastCities = g_Au.Settings().readBoolEntry("Widget/StarMapSearch/LastCities",true);
	bool lastCityEmpires = g_Au.Settings().readBoolEntry("Widget/StarMapSearch/LastCityEmpires",true);
	bool lastRegEx = g_Au.Settings().readBoolEntry("Widget/StarMapSearch/LastRegEx",false);
	bool lastSpacecraft = g_Au.Settings().readBoolEntry("Widget/StarMapSearch/LastSpacecraft",true);
	bool lastSpacecraftEmpires = g_Au.Settings().readBoolEntry("Widget/StarMapSearch/LastSpacecraftEmpires",true);
	bool lastSystems = g_Au.Settings().readBoolEntry("Widget/StarMapSearch/LastSystems",true);
	bool lastSystemCat = g_Au.Settings().readBoolEntry("Widget/StarMapSearch/LastSystemCatalog",true);
	bool lastSectors = g_Au.Settings().readBoolEntry("Widget/StarMapSearch/LastSectors",true);

	m_Text = new QLineEdit(lastSearch, this);
	m_Text->setToolTip(tr("<html>"
				"Enter text to find. "
				"</html>"));

	m_Case = new QCheckBox(tr("Case Sensitive"), this);
	m_Case->setChecked(lastCase);
	m_Case->setToolTip(tr("<html>"
				"When checked, comparisons are case sensitive. "
				"</html>"));
	connect(m_Case, SIGNAL(stateChanged(int)), SLOT(CaseChanged(int)));

	m_RegEx = new QCheckBox(tr("Regular Expression Pattern Matching"), this);
	m_RegEx->setChecked(lastRegEx);
	m_RegEx->setToolTip(tr("<html>"
				"When checked, regular expression pattern matching is used, "
				"</html>"));
	connect(m_RegEx, SIGNAL(stateChanged(int)), SLOT(RegExChanged(int)));

	m_Cities = new QCheckBox(tr("City Names"), this);
	m_Cities->setChecked(lastCities);
	m_Cities->setToolTip(tr("<html>"
				"When checked, the names of cities are checked. "
				"</html>"));
	connect(m_Cities, SIGNAL(stateChanged(int)), SLOT(CitiesChanged(int)));

	m_CityEmpires = new QCheckBox(tr("City Empire Names"), this);
	m_CityEmpires->setChecked(lastCityEmpires);
	m_CityEmpires->setToolTip(tr("<html>"
				"When checked, the empire names of cities are checked. "
				"</html>"));
	connect(m_CityEmpires, SIGNAL(stateChanged(int)), SLOT(CityEmpiresChanged(int)));

	m_Spacecraft = new QCheckBox(tr("Spacecraft Names"), this);
	m_Spacecraft->setChecked(lastSpacecraft);
	m_Spacecraft->setToolTip(tr("<html>"
				"When checked, the names of spacecraft are checked. "
				"</html>"));
	connect(m_Spacecraft, SIGNAL(stateChanged(int)), SLOT(SpacecraftChanged(int)));

	m_SpacecraftEmpires = new QCheckBox(tr("Spacecraft Empire Names"), this);
	m_SpacecraftEmpires->setChecked(lastSpacecraftEmpires);
	m_SpacecraftEmpires->setToolTip(tr("<html>"
				"When checked, the empire names of spacecraft are checked. "
				"</html>"));
	connect(m_SpacecraftEmpires, SIGNAL(stateChanged(int)), SLOT(SpacecraftEmpiresChanged(int)));

	m_Systems = new QCheckBox(tr("Solar System Names"), this);
	m_Systems->setChecked(lastSystems);
	m_Systems->setToolTip(tr("<html>"
				"When checked, names of solar systems are checked. "
				"</html>"));
	connect(m_Systems, SIGNAL(stateChanged(int)), SLOT(SystemsChanged(int)));

	m_SystemCat = new QCheckBox(tr("Solar System Catalog Names"), this);
	m_SystemCat->setChecked(lastSystemCat);
	m_SystemCat->setToolTip(tr("<html>"
				"When checked, catalog names of solar systems are checked. "
				"</html>"));
	connect(m_SystemCat, SIGNAL(stateChanged(int)), SLOT(SystemCatChanged(int)));

	m_Sectors = new QCheckBox(tr("Sector Names"), this);
	m_Sectors->setChecked(lastSectors);
	connect(m_Sectors, SIGNAL(stateChanged(int)), SLOT(SectorsChanged(int)));

	QPushButton* pbok = new QPushButton(QIcon(":data/Magnifier.png"), tr("Search Now"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Conducts a search of the current star map. "
				"Updates the list with places that match. "
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(SearchNow()));

	m_Result = new QListWidget(this);
	m_Result->setToolTip(tr("<html>"
				"Places that match the search are listed. "
				"Click a place to center the star map on its location. "
				"</html>"));
	connect(m_Result, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(SearchResultClicked(QListWidgetItem*)));

	QLabel* lbwhat = new QLabel(tr("Find What"), this);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(m_Text, row,0, 1,2);
	row++;
	lay->addWidget(m_Case, row,1, 1,1);
	row++;
	lay->addWidget(m_RegEx, row,1, 1,1);
	row++;
	lay->addWidget(lbwhat, row,0, 1,2);
	row++;
	lay->addWidget(m_Cities, row,1, 1,1);
	row++;
	lay->addWidget(m_CityEmpires, row,1, 1,1);
	row++;
	lay->addWidget(m_Spacecraft, row,1, 1,1);
	row++;
	lay->addWidget(m_SpacecraftEmpires, row,1, 1,1);
	row++;
	lay->addWidget(m_Systems, row,1, 1,1);
	row++;
	lay->addWidget(m_SystemCat, row,1, 1,1);
	row++;
	lay->addWidget(m_Sectors, row,1, 1,1);
	row++;
	lay->addLayout(blay, row,0, 1,2);
	row++;
	lay->addWidget(m_Result, row,0, 1,2);
	row++;
	lay->setColumnMinimumWidth(0, 20);
	setAuDlgLayout(lay);
}

AuStarMapSearch::~AuStarMapSearch()
{
}

void AuStarMapSearch::setVisible(bool visible)
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

void AuStarMapSearch::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/StarMapSearch/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/StarMapSearch/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/StarMapSearch/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/StarMapSearch/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuStarMapSearch::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/StarMapSearch/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/StarMapSearch/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/StarMapSearch/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/StarMapSearch/SizeY",sz.height());
}

void AuStarMapSearch::CaseChanged(int state)
{
	g_Au.Settings().writeEntry("Widget/StarMapSearch/LastCase", state == Qt::Checked);
}

void AuStarMapSearch::RegExChanged(int state)
{
	g_Au.Settings().writeEntry("Widget/StarMapSearch/LastRegEx", state == Qt::Checked);
}

void AuStarMapSearch::CitiesChanged(int state)
{
	g_Au.Settings().writeEntry("Widget/StarMapSearch/LastCities", state == Qt::Checked);
}

void AuStarMapSearch::CityEmpiresChanged(int state)
{
	g_Au.Settings().writeEntry("Widget/StarMapSearch/LastCityEmpires", state == Qt::Checked);
}

void AuStarMapSearch::SpacecraftChanged(int state)
{
	g_Au.Settings().writeEntry("Widget/StarMapSearch/LastSpacecraft", state == Qt::Checked);
}

void AuStarMapSearch::SpacecraftEmpiresChanged(int state)
{
	g_Au.Settings().writeEntry("Widget/StarMapSearch/LastSpacecraftEmpires", state == Qt::Checked);
}

void AuStarMapSearch::SystemCatChanged(int state)
{
	g_Au.Settings().writeEntry("Widget/StarMapSearch/LastSystemCatalog", state == Qt::Checked);
}

void AuStarMapSearch::SystemsChanged(int state)
{
	g_Au.Settings().writeEntry("Widget/StarMapSearch/LastSystems", state == Qt::Checked);
}

void AuStarMapSearch::SectorsChanged(int state)
{
	g_Au.Settings().writeEntry("Widget/StarMapSearch/LastSectors", state == Qt::Checked);
}

void AuStarMapSearch::SearchNow()
{
	QString searchText = m_Text->text();
	//qDebug() << "AuStarMapSearch::SearchNow" << searchText;
	g_Au.Settings().writeEntry("Widget/StarMapSearch/LastSearch", searchText);

	// Remove current search contents.
	while(m_Result->count())
		delete m_Result->takeItem(0);

	Qt::CaseSensitivity cs;
	if(m_Case->isChecked())
		cs = Qt::CaseSensitive;
	else
		cs = Qt::CaseInsensitive;
	bool cities = m_Cities->isChecked();
	bool cityEmpires = m_CityEmpires->isChecked();
	bool regex = m_RegEx->isChecked();
	bool spacecraft = m_Spacecraft->isChecked();
	bool spacecraftEmpires = m_SpacecraftEmpires->isChecked();
	bool systemCat = m_SystemCat->isChecked();
	bool systems = m_Systems->isChecked();
	bool sectors = m_Sectors->isChecked();

	// Get my empire data record.
	DgEmpire personEmpire;
	personEmpire.m_id = g_AuClient.m_StarMapEmpire;
	//qDebug() << "starmap empire" << g_AuClient.m_StarMapEmpire;
	const DgData& gd = g_Au.LockDataForRead();
	if(!gd.Empire(personEmpire))
		personEmpire.ChangeEmpireName(personEmpire.Name(), tr("Empire"));
	g_Au.UnlockData();
	QIcon cityIcon(":data/City.png");
	QHash<quint32, QIcon> empireFlagIcons;

	// Search the star map, adding found items as we go.
	const AuStarMapData& starMap = g_AuClient.LockStarMapForRead();
	for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI = starMap.m_Sectors.begin();
			sectI != starMap.m_Sectors.end(); sectI++)
	{
		if(sectors)
		{
			QString sectorName((*sectI).second.m_Name);
			if(sectorName == "Sector")
			{
				// Convert from parsec coord to sector coord.
				AuPoint fLoc = (*sectI).second.m_Loc/10.0;
				sectorName += QString(" (%1, %2, %3)").arg(fLoc.x).arg(fLoc.y).arg(fLoc.z);
			}
			if((!regex && sectorName.contains(searchText, cs)) ||
					(regex && sectorName.contains(QRegExp(searchText, cs))))
			{
				// Found a matching sector name.
				QListWidgetItem* item = new QListWidgetItem(QIcon(":data/RangeSector.png"), sectorName, m_Result);
				item->setData(Qt::UserRole, QVariant::fromValue<AuId>((*sectI).first));
			}
		}

		// Process solar systems in the sector.
		for(unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator sysI = (*sectI).second.m_Systems.begin();
				sysI != (*sectI).second.m_Systems.end(); sysI++)
		{
			bool foundSys = false;
			if(systems)
			{
				QString systemName = (*sysI).second.m_Name;
				if((!regex && systemName.contains(searchText, cs)) ||
						(regex && systemName.contains(QRegExp(searchText, cs))))
				{
					// Found a matching system name.
					QListWidgetItem* item = new QListWidgetItem(QIcon(":data/RangeSystem.png"), systemName, m_Result);
					item->setData(Qt::UserRole, QVariant::fromValue<AuId>((*sysI).first));
					foundSys = true;
				}
			}
			if(systemCat && !foundSys)
			{
				AuPoint systemLoc = (*sectI).second.m_Loc + (*sysI).second.m_Loc;
				QString catname = DaSolarSystem::CatalogName(systemLoc);
				if((!regex && catname.contains(searchText, cs)) ||
						(regex && catname.contains(QRegExp(searchText, cs))))
				{
					// Found a matching system name.
					QListWidgetItem* item = new QListWidgetItem(QIcon(":data/RangeSystem.png"), (*sysI).second.m_Name, m_Result);
					item->setData(Qt::UserRole, QVariant::fromValue<AuId>((*sysI).first));
				}
			}

			if((cities || cityEmpires) && (*sysI).second.m_Cities.size())
			{
				// Process cities.
				for(unordered_map<AuId, AuStarMapCity, AuIdHash>::const_iterator cityI = (*sysI).second.m_Cities.begin();
						cityI != (*sysI).second.m_Cities.end(); cityI++)
				{
					bool cityOk = false;
					if(cityEmpires)
					{
						// What do we know about the empire who owns the city.
						QString empireName;
						if(personEmpire.m_id == (*cityI).second.m_Empire)
						{
							// It is our empire.
							empireName = personEmpire.Name();
						}
						else
						{
							// It is not our empire.
							if(personEmpire.IsEmpireKnown((*cityI).second.m_Empire))
								empireName = personEmpire.NameOfKnownEmpire((*cityI).second.m_Empire);
							else
								empireName = tr("Unknown Empire");
						}

						if(regex)
						{
							if(empireName.contains(QRegExp(searchText, cs)))
								cityOk = true;
						}
						else
						{
							if(empireName.contains(searchText, cs))
								cityOk = true;
						}
					}

					if(cities && !cityOk)
					{
						if(regex)
						{
							if((*cityI).second.m_Name.contains(QRegExp(searchText, cs)))
								cityOk = true;
						}
						else
						{
							if((*cityI).second.m_Name.contains(searchText, cs))
								cityOk = true;
						}
					}

					if(!cityOk)
						continue;

					// Found a matching city or empire name.
					QHash<quint32, QIcon>::const_iterator ii = empireFlagIcons.constFind((*cityI).second.m_Empire);
					if(ii == empireFlagIcons.cend())
					{
						// Flag for that empire is not in our cache yet.
						DgEmpire empire;
						empire.m_id = (*cityI).second.m_Empire;
						const DgData& gd = g_Au.LockDataForRead();
						bool hasEmpire = gd.Empire(empire);
						g_Au.UnlockData();
						if(hasEmpire)
						{
							// We have the empire record locally so we can make an icon from its flag.
							QPixmap pxflag;
							pxflag.convertFromImage(empire.m_flag);
							ii = empireFlagIcons.insert((*cityI).second.m_Empire, QIcon(pxflag));
						}
						else
						{
							// We do not have the empire record so use the generic city icon instead of a flag.
							ii = empireFlagIcons.insert((*cityI).second.m_Empire, cityIcon);
						}
					}
					QListWidgetItem* item = new QListWidgetItem(*ii, (*cityI).second.m_Name, m_Result);
					item->setData(Qt::UserRole, QVariant::fromValue<AuId>((*cityI).first));
				}
			}

			if((spacecraft || spacecraftEmpires) && (*sysI).second.m_Spacecraft.size())
			{
				for(unordered_map<AuId, AuStarMapSpacecraft, AuIdHash>::const_iterator si = (*sysI).second.m_Spacecraft.begin();
						si != (*sysI).second.m_Spacecraft.end(); si++)
				{
					bool shipOk = false;
					if(spacecraftEmpires)
					{
						// What do we know about the empire who owns the spacecraft.
						QString empireName;
						if(personEmpire.m_id == (*si).second.m_Empire)
						{
							// It is our empire.
							empireName = personEmpire.Name();
						}
						else
						{
							// It is not our empire.
							if(personEmpire.IsEmpireKnown((*si).second.m_Empire))
								empireName = personEmpire.NameOfKnownEmpire((*si).second.m_Empire);
							else
								empireName = tr("Unknown Empire");
						}

						if(regex)
						{
							if(empireName.contains(QRegExp(searchText, cs)))
								shipOk = true;
						}
						else
						{
							if(empireName.contains(searchText, cs))
								shipOk = true;
						}
					}

					if(spacecraft && !shipOk)
					{
						if(regex)
						{
							if((*si).second.Name().contains(QRegExp(searchText, cs)))
								shipOk = true;
						}
						else
						{
							if((*si).second.Name().contains(searchText, cs))
								shipOk = true;
						}
					}

					if(!shipOk)
						continue;

					// Found a matching spacecraft or empire name.
					QListWidgetItem* item = new QListWidgetItem(QIcon(SpacecraftStructureIcon((*si).second.Structure())),
							(*si).second.Name(), m_Result);
					item->setData(Qt::UserRole, QVariant::fromValue<AuId>((*si).first));
				}
			}
		}
	}
	g_AuClient.UnlockStarMap();
}

void AuStarMapSearch::SearchResultClicked(QListWidgetItem* item)
{
	AuId placeId = item->data(Qt::UserRole).value<AuId>();
	emit PlaceClicked(placeId);
}
