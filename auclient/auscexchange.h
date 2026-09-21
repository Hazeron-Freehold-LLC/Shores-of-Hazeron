//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscexchange.h,v 1.10 2022/05/04 21:14:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScExchange_H
#define AuScExchange_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auexchangerating.h"
#include "auglobal.h"
#include "auid.h"
#include "auroster.h"
#include <QWidget>

class AuPictureWidget;
class AuScExchangeWidget;
class QLabel;
class QLineEdit;
class QPushButton;
class QSplitter;
class QTextBrowser;

class AUCLIENT_DLL AuScExchange : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	QPushButton*						m_Buy;
	QLineEdit*							m_DesignId;
	QLineEdit*							m_DesignName;
	QTextBrowser*						m_DesignReport;
	AuPictureWidget*					m_DesignThumb;
	AuScExchangeWidget*					m_Exchange;
	QPushButton*						m_Publish;
	QPushButton*						m_Rate;
	QPushButton*						m_Ratings;
	QPushButton*						m_RequestDesigns;
	QSplitter*							m_Splitter;
	QSplitter*							m_Splitter2;
	QTextBrowser*						m_Terms;
	QPushButton*						m_Unpublish;

public:
	AuScExchange(QWidget* parent);
	virtual ~AuScExchange();

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
	void RequestDesigns();
	void SplitterMoved(int pos, int index);
	void Splitter2Moved(int pos, int index);
	void UnpublishDesign();
	void ViewRatings();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuScExchangeDock : public AuDockWidget
{
public:
	AuScExchange*				m_Content;

public:
	AuScExchangeDock(QWidget* parent) : AuDockWidget(":data/Exchange.png",
			tr("<b>Spacecraft Blueprint Exchange</b>"), tr("Spacecraft Blueprint Exchange"), parent)
	{
		m_Content = new AuScExchange(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/ScExchangeAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/ScExchangeAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuScExchange_H
