//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovpatents.h,v 1.8 2022/05/04 21:27:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGovPatents_H
#define AuGovPatents_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include <QStandardItemModel>
#include <QWidget>

class QListWidget;
class QToolButton;

class AUCLIENT_DLL AuGovPatents : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	QListWidget*						m_PatentList;
	QToolButton*						m_PatentsGet;
	QToolButton*						m_PatentsRecord;

public:
	AuGovPatents(QWidget* parent=0);

	virtual QSize sizeHint() const;

	void UpdateEmpireRecord();

protected:
	void ListPatents();
	virtual void showEvent(QShowEvent* event);

protected slots:
	void GetPatent();
	void RecordPatent();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuGovPatentsDock : public AuDockWidget
{
public:
	AuGovPatents*						m_Content;

public:
	AuGovPatentsDock(QWidget* parent) : AuDockWidget(":data/c_Patent.png", tr("<b>Patents</b>"), tr("Govern Patents"), parent)
	{
		m_Content = new AuGovPatents(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/GovernPatentsAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/GovernPatentsAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuGovPatents_H
