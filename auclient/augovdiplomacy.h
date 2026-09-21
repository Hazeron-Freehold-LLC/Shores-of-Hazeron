//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovdiplomacy.h,v 1.11 2022/05/04 21:26:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGovDiplomacy_H
#define AuGovDiplomacy_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "auid.h"
#include "dgempire.h"
#include <QStandardItemModel>
#include <QTableWidget>
#include <QWidget>

class AuHueSelector;
class AuContrailSample;
class AuDossierWidget;
class DaPerson;
class QCheckBox;
class QComboBox;
class QPushButton;
class QSlider;
class QSplitter;
class QTextBrowser;

class AUCLIENT_DLL AuDossierWidget : public QTableWidget
{
	Q_OBJECT;
public:	// Constructors
	AuDossierWidget(QWidget* parent=Q_NULLPTR);

signals:
	void New();
	void Open();
	void Pend();
	void Close();
	void Delete();
	void DoubleClick();

protected:
	virtual void mouseDoubleClickEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuGovDiplomacy : public QWidget
{
	Q_OBJECT;
	DgEmpire			m_Empire;

	// Widgets
	QComboBox*			cbDefStance;
	QPushButton*						m_AddNote;
	QPushButton*						m_OpenDoc;
	QPushButton*						m_CloseDoc;
	QPushButton*						m_DeleteDoc;
	QPushButton*						m_DispatchCourier;
	QTextBrowser*						m_Dossier;
	QSplitter*							m_DosSplitter;
	QPushButton*						m_NewDoc;
	QPushButton*						m_PendDoc;
	QSplitter*							m_Splitter;
	QPushButton*						m_WarCriteria;
	QPushButton*		pbally;
	QPushButton*		pbenemy;
	QPushButton*		pbfriend;
	QPushButton*		pbneutral;
	QPushButton*		pbvassal;
	AuDossierWidget*					tableDossier;
	QTableWidget*		tableEmpires;

public:
	AuGovDiplomacy(QWidget* parent=0);

	void EmpireDataComplete();

	virtual QSize sizeHint() const;

	void UpdateEmpireRecord();

signals:
	void DossierAddNote(quint32 dosEmpire, QString docNote);
	void DossierDispatchCourier(quint32 toEmpire, QString message);
	void DossierDocState(quint32 dosEmpire, AuTime docTime, quint8 docType, quint16 docIndex, quint8 docState);
	void EmpireDataCompleteSignal();
	void SetStance(quint32 towardEmpire, quint8 stance, quint8 vassalTributePercent);
	void SetWarCriteria(AuWarCriteria crit);

protected slots:
	void AddNote();
	void CloseDoc();
	void DeleteDoc();
	void DispatchCourier();
	void DossierSelectionChanged();
	void DoubleClickDoc();
	void EmpireSelectionChanged();
	void EnableControls();
	void OpenDoc();
	void PendDoc();
	void NewDoc();
	void RequestEmpireData();
	void SetAlly();
	void SetDoubleClickStatus(int index);
	void SetEnemy();
	void SetFriend();
	void SetNeutral();
	void SetVassal();
	void SplitterMoved(int pos, int index);
	void UpdateDocument();
	void UpdateDossier();
	void WarCriteria();

protected:
	virtual void showEvent(QShowEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuGovDiplomacyDock : public AuDockWidget
{
public:
	AuGovDiplomacy*						m_Content;

public:
	AuGovDiplomacyDock(QWidget* parent) : AuDockWidget(":data/CommDiplomacy.png", tr("<b>Diplomacy</b>"), tr("Govern Diplomacy"),
			parent)
	{
		m_Content = new AuGovDiplomacy(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/GovernDiplomacyAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/GovernDiplomacyAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuGovDiplomacy_H
