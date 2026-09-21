//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdexchange.h,v 1.9 2022/05/04 21:14:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdExchange_H
#define AuBdExchange_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auexchangerating.h"
#include "auglobal.h"
#include "auid.h"
#include "auroster.h"
#include <QWidget>

class AuPictureWidget;
class AuBdExchangeWidget;
class QLabel;
class QLineEdit;
class QPushButton;
class QSplitter;
class QTextBrowser;

class AUCLIENT_DLL AuBdExchange : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	QPushButton*						m_Buy;
	QLineEdit*							m_DesignId;
	QLineEdit*							m_DesignName;
	QTextBrowser*						m_DesignReport;
	AuPictureWidget*					m_DesignThumb;
	AuBdExchangeWidget*					m_Exchange;
	QPushButton*						m_Publish;
	QPushButton*						m_Rate;
	QPushButton*						m_Ratings;
	QPushButton*						m_RequestDesigns;
	QSplitter*							m_Splitter;
	QSplitter*							m_Splitter2;
	QTextBrowser*						m_Terms;
	QPushButton*						m_Unpublish;

public:
	AuBdExchange(QWidget* parent);
	virtual ~AuBdExchange();

	// Returns the currently selected design id, or 0 when none is selected.
	quint32 DesignId() const;

	// Returns the currently selected design name.
	QString DesignName() const;

	// Sets the states of the selected design id and design name boxes.
	void SetDesign(quint32 id, const QString& name, const QString& report);

	virtual QSize sizeHint() const;

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void showEvent(QShowEvent* event);

protected slots:
	void BuyDesign();
	void DesignSelectionChanged();
	void ImageScaleChanged(float scale);
	void PublishDesign();
	void RateDesign();
	void RefreshThumb();
	void SplitterMoved(int pos, int index);
	void Splitter2Moved(int pos, int index);
	void UnpublishDesign();
	void ViewRatings();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuBdExchangeDock : public AuDockWidget
{
public:
	AuBdExchange*				m_Content;

public:
	AuBdExchangeDock(QWidget* parent) : AuDockWidget(":data/Exchange.png",
			tr("<b>Building Blueprint Exchange</b>"), tr("Building Blueprint Exchange"), parent)
	{
		m_Content = new AuBdExchange(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/BdExchangeAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/BdExchangeAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuBdExchange_H
