//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovannouncements.h,v 1.8 2022/05/04 21:25:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGovAnnouncements_H
#define AuGovAnnouncements_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include <QWidget>

class QLabel;
class QPushButton;
class QSplitter;
class QTextBrowser;
class QTextEdit;

class AUCLIENT_DLL AuGovAnnouncements : public QWidget
{
	Q_OBJECT;
private:
	bool								m_DataValid;			// true if data in controls is valid

	// Widgets
	QTextBrowser*						m_Announcements;
	QPushButton*						m_ApplyChanges;
	QPushButton*						m_Clear;
	QTextEdit*							m_Editor;
	QLabel*								m_EmpireFlag;
	QLabel*								m_EmpireName;
	QSplitter*							m_Splitter;

public:
	AuGovAnnouncements(QWidget* parent=0);
	void UpdateEmpireRecord();

	virtual QSize sizeHint() const;

protected slots:
	void ApplyChanges();
	void ClearAnnouncements();
	void EditorTextChanged();

protected:
	virtual void showEvent(QShowEvent* event);

signals:
	void AnnouncementsChanged(QString announcements);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuGovAnnouncementsDock : public AuDockWidget
{
public:
	AuGovAnnouncements*					m_Content;

public:
	AuGovAnnouncementsDock(QWidget* parent) : AuDockWidget(":data/Speaker.png", tr("<b>Announcements</b>"),
			tr("Govern Announcements"), parent)
	{
		m_Content = new AuGovAnnouncements(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/GovernAnnouncementsAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/GovernAnnouncementsAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuGovAnnouncements_H
