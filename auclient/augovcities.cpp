//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovcities.cpp,v 1.13 2022/05/04 21:29:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augovcities.h"
//
#include "auapp.h"
#include "auchooseempire.h"
#include "aucitieswidget.h"
#include "aucity.h"
#include "audomain.h"
#include "auglobal.h"
#include "auframe.h"
#include "aurenameplanet.h"
#include "aurenamesector.h"
#include "aurenamesystem.h"
#include "auview.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSplitter>
#include <QTextBrowser>

void AuGovCitiesDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleGovernCitiesUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGovCities::AuGovCities(QWidget* parent) : QWidget(parent)
{
	AuFrame* frame = auApp->Frame();

	m_QuitClaim = new QPushButton(tr("Quit Claim to World"));
	m_QuitClaim->setIcon(QIcon(":data/SurrenderFlag.png"));
	m_QuitClaim->setToolTip(tr("<html><b>Quit Claim to World</b><br>"
				"Quits your empire's claim to the current world.<br>"
				"<i>Tips</i><br>"
				"%1 A claim cannot be quit from a world with a military headquarters."
				"<br>"
				"%1 A claim can be quit by anyone in the chain of command of the world."
				"<br>"
				"%1 A claim can be quit by the avatar who staked the claim, if no capitol has been built."
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_QuitClaim, SIGNAL(clicked(bool)), SLOT(QuitClaim()));

	QPushButton* pbreq = new QPushButton(tr("Refresh Cities"));
	pbreq->setToolTip(tr("<html><b>Refresh Cities</b><br>"
				"Requests fresh city information about your empire from the server. "
				"This may be helpful to see recent changes to your empire's cities.</html>"));
	connect(pbreq, SIGNAL(clicked(bool)), SLOT(RequestEmpireData()));

	QLabel* lbcities = new QLabel(tr("Empire Cities"));
	lbcities->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	lbcities->setToolTip(tr("<html><p><b>Empire Cities</b><br>"
				"All cities of the empire are listed. "
				"</p>"
				"<p>"
				"Right-click selected cities for options. "
				"<u>You can only surrender cities founded by you, unless the founder is not a citizen of the empire.</u>"
				"</p>"
				"</html>"));

	m_Cities = new AuCitiesWidget(this);
	m_Cities->setToolTip(tr("<html><p><b>Empire Cities</b><br>"
				"All cities of the empire are listed. "
				"</p>"
				"<p>"
				"Right-click selected cities for options. "
				"<u>You can only surrender cities founded by you, unless the founder is not a citizen of the empire.</u>"
				"</p>"
				"</html>"));
	connect(m_Cities, SIGNAL(CityRightClicked(AuId)), SLOT(CityRightClicked(AuId)));
	connect(m_Cities, SIGNAL(itemSelectionChanged()), SLOT(CitySelected()));

	renPlanetButton = new QPushButton(tr("Rename Current World"));
	renPlanetButton->setToolTip(tr("<html><b>Rename World</b><br>"
				"Changes the name of the current world.<br>"
				"<i>Tips</i><br>"
				"%1 A world can be renamed by the avatar who claims it and "
				"by anyone in the worlds' chain of command, who has the authority to change place names."
				"<br>"
				"%1 Your empire must claim a world to have naming rights. "
				"</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(renPlanetButton, SIGNAL(clicked(bool)), SLOT(RenamePlanet()));

	renSystemButton = new QPushButton(tr("Rename Current Solar System"));
	renSystemButton->setToolTip(tr("<html><b>Rename Solar System</b><br>"
				"Changes the name of the current solar system and all of its planets.<br>"
				"<i>Tips</i><br>"
				"%1 A solar system can only be renamed by someone in its chain of command.<br>"
				"%1 Your empire must have at least one city in the solar system or the system must have no worlds that can host "
					"cities, such as gas giants.<br>"
				"%1 All flags in all cities must be controlled by your empire.<br>"
				"%1 No cities can be occupied cities.<br>"
				"%1 Solar system name change is propagated to all planets in the solar system."
				"</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(renSystemButton, SIGNAL(clicked(bool)), SLOT(RenameSystem()));

	const DgData& gd = g_Au.LockDataForRead();
	m_SceneAtomId = gd.ClientUnitId();
	g_Au.UnlockData();

	renSectorButton = new QPushButton(tr("Rename Current Sector"));
	renSectorButton->setToolTip(tr("<html><b>Rename Sector</b><br>"
				"Changes the name of the current sector.<br>"
				"<i>Tips</i><br>"
				"%1 A sector can only be renamed by someone in its chain of command.<br>"
				"%1 Your empire must have at least one city in the sector.<br>"
				"%1 Your empire must control all cities in the sector."
				"</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(renSectorButton, SIGNAL(clicked(bool)), SLOT(RenameSector()));

	m_Facilities = new QTextBrowser(this);

	m_Splitter = new QSplitter;
	m_Splitter->setOrientation(Qt::Vertical);
	m_Splitter->addWidget(m_Cities);
	m_Splitter->addWidget(m_Facilities);
	m_Splitter->setCollapsible(0, false);
	m_Splitter->setCollapsible(1, false);
	m_Splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(renPlanetButton, row,4, 1,4);
	row++;
	lay->addWidget(renSystemButton, row,4, 1,4);
	row++;
	lay->addWidget(m_QuitClaim, row,0, 1,3);
	lay->addWidget(renSectorButton, row,4, 1,4);
	row++;
	lay->addWidget(lbcities, row,0, 1,4);
	lay->addWidget(pbreq, row,5, 1,3);
	row++;
	lay->addWidget(m_Splitter, row,0, 1,8);
	row++;
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	connect(this, SIGNAL(ShowCitiesOnStarMap(AuIdList)),
			frame, SLOT(ShowCitiesOnStarMap(AuIdList)));
	connect(this, SIGNAL(SurrenderCities(AuIdList, QStringList, quint32)),
			frame, SLOT(SurrenderCities(AuIdList, QStringList, quint32)));

	QByteArray splitterstate;
	g_Au.Settings().readEntry("Widget/GovCities/Splitter", splitterstate);
	m_Splitter->restoreState(splitterstate);
}

void AuGovCities::UpdateEmpireRecord()
{
	// If this widget is not visible, just clear the empire id to 0.
	// setVisible calls us to really update the page when we become visible.
	if(!isVisible())
	{
		//qDebug("AuGovCities::UpdateEmpireRecord page is not visible. Postponing update until page becomes visible.");
		m_EmpireId = 0;
		return;
	}

	//qDebug("AuGovCities::UpdateEmpireRecord page is visible. Update contents.");
	// Get new empire record data
	AuView* pView = auApp->Frame()->View();
	m_EmpireId = pView->SceneAtomEmpire();
	m_SceneAtomId = pView->SceneAtomId();

	m_Cities->ListCities(m_EmpireId);

	EnableControls();
}

void AuGovCities::EnableControls()
{
	const DgData& gd = g_Au.LockDataForRead();
	bool canNamePlaces = gd.EmpireCitizenCanSetPlaceNames(m_EmpireId, m_SceneAtomId);
	g_Au.UnlockData();

	//renPlanetButton->setEnabled(canNamePlaces);
	renSystemButton->setEnabled(canNamePlaces);
	renSectorButton->setEnabled(canNamePlaces);
}

void AuGovCities::RequestEmpireData()
{
	if(m_EmpireId)
		auApp->RequestEmpire(m_EmpireId);
}

void AuGovCities::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	//if(!m_EmpireId)
		UpdateEmpireRecord();
}

void AuGovCities::RenamePlanet()
{
	//qDebug() << "AuGovCities::RenamePlanet";
	AuRenamePlanet renameDlg(this);
	renameDlg.exec();
}

void AuGovCities::RenameSystem()
{
	//qDebug() << "AuGovCities::RenameSystem";
	AuRenameSystem renameDlg(this);
	renameDlg.exec();
}

void AuGovCities::RenameSector()
{
	//qDebug() << "AuGovCities::RenameSector";
	AuRenameSector renameDlg(this);
	renameDlg.exec();
}

void AuGovCities::CityRightClicked(AuId /*cityId*/)
{
	AuIdList cityIds;
	int govOrOwn = m_Cities->SelectedCities(cityIds, m_SceneAtomId);

	QMenu menu(this);

	QAction* reportCityLast = menu.addAction(QIcon(":data/MailCityStatus.png"), tr("Recent Report..."));
	reportCityLast->setEnabled(govOrOwn);

	QAction* reportCityHistory = menu.addAction(QIcon(":data/MailCityStatus.png"), tr("History Report..."));
	reportCityHistory->setEnabled(govOrOwn);

	menu.addSeparator();

	QAction* mailCityLast = menu.addAction(QIcon(":data/ToggleMail.png"), tr("Recent Report by Mail..."));
	mailCityLast->setEnabled(govOrOwn);

	menu.addSeparator();

	QAction* showStarMap = menu.addAction(QIcon(":data/RangeGalaxy.png"), tr("Show on Star Map..."));

	menu.addSeparator();

	QAction* surrender = menu.addAction(QIcon(":data/SurrenderFlag.png"), tr("Surrender..."));
	surrender->setEnabled(govOrOwn == 2);

	QAction* domainAdd = menu.addAction(QIcon(":data/AvatarDomain.png"), tr("Add to Land..."));

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;

	if(action == reportCityLast)
		ReportCityLast();
	else if(action == reportCityHistory)
		ReportCityHistory();
	else if(action == mailCityLast)
		MailCityLast();
	else if(action == surrender)
		SurrenderSelectedCities();
	else if(action == showStarMap)
		ShowSelectedCitiesOnStarMap();
	else if(action == domainAdd)
		DomainAdd();
}

void AuGovCities::QuitClaim()
{
	auApp->GovQuitClaim();
}

void AuGovCities::MailCityLast()
{
	AuIdList cityIds;
	if(!m_Cities->SelectedCities(cityIds, m_SceneAtomId))
		return;
	auApp->ReportCitiesByMail(cityIds);
}

void AuGovCities::ReportCityHistory()
{
	AuIdList cityIds;
	if(!m_Cities->SelectedCities(cityIds, m_SceneAtomId))
		return;
	if(cityIds.isEmpty())
		return;
	if(cityIds.size() == 1)
		auApp->ReportCityHistory(cityIds.first());
	else
		auApp->ReportCitiesHistoryByMail(cityIds);
}

void AuGovCities::ReportCityLast()
{
	AuIdList cityIds;
	if(!m_Cities->SelectedCities(cityIds, m_SceneAtomId))
		return;
	if(cityIds.isEmpty())
		return;
	if(cityIds.size() == 1)
		auApp->ReportCityLast(cityIds.first());
	else
		auApp->ReportCitiesLastByMail(cityIds);
}

void AuGovCities::ShowSelectedCitiesOnStarMap()
{
	AuIdList cityIds;
	QStringList cityNames;
	m_Cities->SelectedCities(cityIds, cityNames, m_SceneAtomId);
	if(cityIds.isEmpty())
		return;
	if(cityIds.count() != cityNames.count())
		return;

	emit ShowCitiesOnStarMap(cityIds);
}

void AuGovCities::SurrenderSelectedCities()
{
	AuIdList cityIds;
	QStringList cityNames;
	if(m_Cities->SelectedCities(cityIds, cityNames, m_SceneAtomId) != 2)
		return;	// not founder of all cities
	if(cityIds.isEmpty())
		return;
	if(cityIds.count() != cityNames.count())
		return;

	QString chooseMsg = tr("<html><p>"
			"Surrender %2 selected cities."
			"</p>"
			"<p>"
			"You have authority to surrender the cities because you are the founder or the founder is not a citizen of the empire. "
			"</p>"
			"<p>"
			"Choose an empire to receive the surrendered cities. "
			"</p>"
			"%1 <b>A message will be sent on the Diplomacy channel that can only be heard by your empire and the other empire. "
			"A governor of the other empire, with the right to accept city surrender, must be on line to accept surrender.</b><br>"
			"%1 Pirate empires always accept surrendered cities.<br>"
			"%1 Military base surrenders all cities on the world.<br> "
			"%1 Military base surrenders your claim to the world.<br>"
			"%1 City surrender fails if the world has a military base.<br>"
			"%1 City surrenders your claim to the world, "
			"if no cities belong to your empire after the surrender is complete.<br>"
			"%1 Allegiance of troops at bases, new vehicles and new spacecraft is changed to the other empire."
			"</html>").arg(g_Au.MiddleDotSymbol()).arg(cityIds.count());

	// Get the id of the empire to surrender the city to.
	AuChooseEmpire empiresDlg(chooseMsg, this);
	if(empiresDlg.exec() == QDialog::Accepted)
	{
		quint32 toEmpire = empiresDlg.EmpireId();
		if(toEmpire != m_EmpireId)
			emit SurrenderCities(cityIds, cityNames, toEmpire);
	}
}

void AuGovCities::CitySelected()
{
	QString rpt;
	AuIdList cityIds;
	m_Cities->SelectedCities(cityIds, m_SceneAtomId);
	if(cityIds.size() == 1)
	{
		const DgData& gd = g_Au.LockDataForRead();
		DgEmpireHash::const_iterator ei = gd.m_gEmpires.find(m_EmpireId);
		if(ei != gd.m_gEmpires.cend())
		{
			AuCityHash::const_iterator ci = (*ei).second.m_Cities->find(cityIds.first());
			if(ci != (*ei).second.m_Cities->cend())
			{
				QMap<QString, AuCityIndustryIID> facil;
				for(AuCityIndustryHash::const_iterator ii = (*ci).second.Facilities().cbegin();
						ii != (*ci).second.Facilities().cend(); ii++)
				{
					// Don't show zeros. Shouldn't exist any more now that it's a hash and not an array.
					if((*ii).Levels() == 0)
						continue;
					const DgData& gd = g_Au.LockDataForRead();
					QString ind = gd.IndustryName(ii.key());
					g_Au.UnlockData();
					facil.insert(ind, AuCityIndustryIID(*ii, ii.key()));
				}
				if(!facil.isEmpty())
				{
					rpt += QObject::tr("<b>FACILITIES</b><div style=\"margin-left: 10px;\">"
						"<table width=\"100%\" border=\"0\" cellpadding=\"2\" "
						"cellspacing=\"0\" style=\"background-color: rgb(7, 62, 94);\"><tbody>");
					rpt += QObject::tr("<tr><td><b>Name</b></td><td><b>Levels</b></td>"
							"<td></td>"
							"<td><b>Name</b></td><td><b>Levels</b></td></tr>");
					for(QMap<QString, AuCityIndustryIID>::const_iterator mit = facil.cbegin(); mit != facil.cend(); mit++)
					{
						QString sFacility = mit.key();
						int levelsTotal = (*mit).Levels();
						int levelsInUse = (*mit).LevelsInUse();
						QString levels;
						if(DbIndustry::DoesManufacture((*mit).IndustryId()))
							levels = QString("%L1, %L2 in use").arg(levelsTotal).arg(levelsInUse);
						else
							levels = QString("%L1").arg(levelsTotal);
						INDUSTRYID iid0 = (*mit).IndustryId();
						mit++;
						if(mit != facil.cend())
						{
							// We have two columns to list in this row, could be the last row but it doesn't matter.
							int levelsTotal1 = (*mit).Levels();
							int levelsInUse1 = (*mit).LevelsInUse();
							QString levels1;
							if(DbIndustry::DoesManufacture((*mit).IndustryId()))
								levels1 = QString("%L1, %L2 in use").arg(levelsTotal1).arg(levelsInUse1);
							else
								levels1 = QString("%L1").arg(levelsTotal1);

							rpt += QObject::tr("<tr>"
									"<td valign=\"top\"><img height=\"32\" align=\"left\" src=\"qrc%7\"> %1</td>"
									"<td valign=\"top\">%3</td>"
									"<td valign=\"top\"></td>"
									"<td valign=\"top\"><img height=\"32\" align=\"left\" src=\"qrc%8\"> %4</td>"
									"<td valign=\"top\">%6</td>"
									"</tr>")
								.arg(sFacility)
								.arg(levels)
								.arg(mit.key())
								.arg(levels1)
								.arg(DbIndustry::BitmapName(iid0))
								.arg(DbIndustry::BitmapName((*mit).IndustryId()));
						}
						else
						{
							// We only have one column to list in this last row.
							rpt += QObject::tr("<tr>"
									"<td valign=\"top\"><img height=\"32\" align=\"left\" src=\"qrc%4\"> %1</td>"
									"<td valign=\"top\">%3</td>"
									"</tr>")
								.arg(sFacility)
								.arg(levels)
								.arg(DbIndustry::BitmapName(iid0));
							break;		// Break out because we know we're done
						}
					}
					rpt += "</tbody></table><br></div>";
				}
			}
		}
		g_Au.UnlockData();
	}
	m_Facilities->setHtml(rpt);
}

void AuGovCities::SplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/GovCities/Splitter", m_Splitter->saveState());
}

void AuGovCities::DomainAdd()
{
	AuIdList systemIds;
	QStringList systemNames;
	m_Cities->SelectedCitySystems(systemIds, systemNames);
	if(systemIds.isEmpty())
		return;
	if(systemIds.count() != systemNames.count())
		return;

	QString chooseMsg = tr("<html><p>"
			"Add %2 solar systems to your land."
			"</p>"
			"<p>"
			"An avatar's land can contain up to %1 solar systems. "
			"Buildings on your land are immune to abandonment decay for 30 days after this avatar was last on line. "
			"</p>"
			"</html>").arg(AuDomain::LimitPerAvatar()).arg(systemIds.count());

	AuIdList::const_iterator ii = systemIds.cbegin();
	QStringList::const_iterator ni = systemNames.cbegin();
	while(ii != systemIds.cend() && ni != systemNames.cend())
	{
		auApp->LandAddSystem(*ii, *ni);
		ii++;
		ni++;
	}
}

QSize AuGovCities::sizeHint() const
{
	return QSize(900,800);
}
