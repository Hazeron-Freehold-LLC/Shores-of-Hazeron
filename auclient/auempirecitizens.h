//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auempirecitizens.h,v 1.10 2022/05/04 21:27:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef	AuEmpireCitizens_H
#define AuEmpireCitizens_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "dgempire.h"
#include <QDialog>
#include <QImage>

class QGridLayout;
class QPushButton;
class QTableWidget;
class QTreeWidget;
class QTreeWidgetItem;

class AUCLIENT_DLL AuEmpireCitizens : public QWidget
{
	Q_OBJECT;
public:
	DgEmpire		m_Empire;
	AuId			m_idPerson;
	AuId			m_idSector;
	QString			m_sPersonName;
	quint32			m_idPersonEmpire;
	bool			m_bPersonHasDebt;
	bool			m_EmpireMailOk;

	// Widgets
    QGridLayout*	gridLayout;
    QTreeWidget*	m_CitizensTree;
    QPushButton*	pbSendEmpireMail;
    QPushButton*	pbSendGovernmentMail;
	QPushButton*	pbSendMail;
    QPushButton*	pbAssignPosition;
    QPushButton*	pbChangeBoss;
    QPushButton*	pbChangePosition;
    QPushButton*	pbLeaveEmpire;
    QPushButton*	pbResignPosition;
    QPushButton*	pbRevokePosition;
    QPushButton*	pbViewPosition;
    QPushButton*	pbKick;

public:
	AuEmpireCitizens(QWidget* parent=0);

	virtual QSize sizeHint() const;

	void UpdateEmpireRecord();

public slots:
	void CitizenLeaveEmpire();
	void Dismiss();
	void EnableControls();
	void AssignPosition();
	void ChangeBoss();
	void ChangePosition();
	void ResignPosition();
	void RevokePosition();
	void SendMail();
	void SendEmpireMail();
	void SendGovernmentMail();
	void ViewPosition();

protected:
	void AddSubordinates(QTreeWidgetItem* boss, QSet<AuId>& alreadyListed);
	void FillCitizensTree(QTreeWidget* tree);
	virtual void showEvent(QShowEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuGovCitizensDock : public AuDockWidget
{
public:
	AuEmpireCitizens*					m_Content;

public:
	AuGovCitizensDock(QWidget* parent) : AuDockWidget(":data/CommCrew.png", tr("<b>Citizens</b>"), tr("Govern Citizens"), parent)
	{
		m_Content = new AuEmpireCitizens(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/GovernCitizensAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/GovernCitizensAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuEmpireCitizens_H
