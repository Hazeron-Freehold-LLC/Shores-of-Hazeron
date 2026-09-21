//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovcities.h,v 1.9 2022/05/04 21:28:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGovCities_H
#define AuGovCities_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "auid.h"
#include <QStandardItemModel>
#include <QWidget>

class AuCitiesWidget;
class QPushButton;
class QSplitter;
class QTextBrowser;

class AUCLIENT_DLL AuGovCities : public QWidget
{
	Q_OBJECT;
private:
	quint32								m_EmpireId;
	AuId								m_SceneAtomId;

	// Widgets
	AuCitiesWidget*						m_Cities;
	QTextBrowser*						m_Facilities;
	QPushButton*						m_QuitClaim;
	QSplitter*							m_Splitter;
	QPushButton*						renPlanetButton;
	QPushButton*						renSectorButton;
	QPushButton*						renSystemButton;

public:
	AuGovCities(QWidget* parent=0);

	virtual QSize sizeHint() const;

	void UpdateEmpireRecord();

signals:
	void ShowCitiesOnStarMap(AuIdList cityIds);
	void SurrenderCities(AuIdList cityIds, QStringList cityNames, quint32 toEmpire);

protected slots:
	void CityRightClicked(AuId cityId);
	void CitySelected();
	void DomainAdd();
	void EnableControls();
	void MailCityLast();
	void QuitClaim();
	void RenamePlanet();
	void RenameSector();
	void RenameSystem();
	void ReportCityHistory();
	void ReportCityLast();
	void RequestEmpireData();
	void ShowSelectedCitiesOnStarMap();
	void SplitterMoved(int pos, int index);
	void SurrenderSelectedCities();

protected:
	virtual void showEvent(QShowEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuGovCitiesDock : public AuDockWidget
{
public:
	AuGovCities*						m_Content;

public:
	AuGovCitiesDock(QWidget* parent) : AuDockWidget(":data/City.png", tr("<b>Cities and Military Bases</b>"),
			tr("Govern Cities and Bases"), parent)
	{
		m_Content = new AuGovCities(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/GovernCitiesAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/GovernCitiesAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuGovCities_H
