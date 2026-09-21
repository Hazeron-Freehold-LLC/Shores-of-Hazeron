//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovcommerce.h,v 1.7 2022/02/19 17:56:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGovCommerce_H
#define AuGovCommerce_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include <dgempire.h>
#include <QStandardItemModel>
#include <QWidget>

class AuCommodityFilter;
class QPushButton;
class QTreeView;

class AUCLIENT_DLL AuGovCommerce : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	QTreeView*							m_Tree;
	QPushButton*						pbany;
	QPushButton*						pbapply;
	QPushButton*						pbfriend;
	QPushButton*						pbneutral;
	QPushButton*						pbnoex;

public:
	AuGovCommerce(QWidget* parent=0);

	void UpdateEmpireRecord();

signals:
	void CommerceChanged(QByteArray commerce);

protected slots:
	void ApplyChanges();
	void DoNotExport();
	void ExportToAnyone();
	void ExportToFriends();
	void ExportToNeutrals();
	void FilterChanged(COMMODITYFILTER);

protected:
	void ListCommerce();
	void SetSelectedItems(EXPORT_POLICY com);

private:
	COMMODITYFILTER		m_Filter;
	AuCommodityFilter*	m_FilterWidget;
	QStandardItemModel	m_Model;

protected:
	virtual void showEvent(QShowEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuGovCommerceDock : public AuDockWidget
{
public:
	AuGovCommerce*						m_Content;

public:
	AuGovCommerceDock(QWidget* parent) : AuDockWidget(":data/GovCommerce.png", tr("<b>Commerce</b>"), tr("Govern Commerce"), parent)
	{
		m_Content = new AuGovCommerce(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/GovernCommerceAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/GovernCommerceAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuGovCommerce_H
