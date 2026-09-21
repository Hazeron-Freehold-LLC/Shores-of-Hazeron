//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdexchangefilter.h,v 1.14 2022/02/19 17:50:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdExchangeFilter_H
#define AuBdExchangeFilter_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"
#include "dbindustry.h"

class AuBdReport;
class AuCheckBoxValue64;
class QCheckBox;
class QLabel;
class QPushButton;
class QRadioButton;
class QScrollArea;
class QSpinBox;

class AUCLIENT_DLL AuBdExchangeFilter : public AuDialog
{
	Q_OBJECT;
	QSet<AuId>							m_PubExclude;			// Ids of excluded publishers.

	// Widgets
	QLabel*								lbIntend;
	QLabel*								lbPub;
	QLabel*								lbRating;
	QLabel*								lbTer;
	QRadioButton*						m_Advanced;
	QRadioButton*						m_Beginner;
	QCheckBox*							m_cbIntended;
	QCheckBox*							m_cbLand;
	QCheckBox*							m_cbRated;
	QCheckBox*							m_cbSeaBottom;
	QPushButton*						m_DeselectAll;
	QSpinBox*							m_MinRating;
	QList<AuCheckBoxValue64*>			m_PubChecks;
	QScrollArea*						m_PubScroller;
	QPushButton*						m_SelectAll;

public:
	AuBdExchangeFilter(QWidget* parent=0);
	virtual ~AuBdExchangeFilter();

	// Returns true if the return values are an advanced filter.
	// Returns false if the return values are a basic filter.
	static bool LoadFilter(QSet<AuId>& excludePub, bool& terLand, bool& terSeaBottom, bool& intendedIndustry, bool& mustBeRated,
			int& minRating);

	// Returns true if 'rpt' passes the filter, false if 'rpt' fails the filter.
	static bool TestFilter(const QSet<AuId>& excludePub, bool terLand, bool terSeaBottom, bool intendedIndustry,
			INDUSTRYID industryId, ORBITZONE ozone, bool gasGiant, bool breathableAtmo, bool enclosurePresent, bool mustBeRated,
			int minRating, const AuBdReport& rpt, float rating);

public slots:
	void AdvancedClicked();
	void BeginnerClicked();
	void DeselectAllPressed();
	void OkPressed();
	void RatedChanged(int state);
	void SelectAllPressed();
	virtual void setVisible(bool visible);

protected:
	QWidget* CreatePublisherChecks(const QSet<AuId>& pubExclude);
	void EnableControls();
	static bool LoadFilterSettings(QSet<AuId>& excludePub, bool& terLand, bool& terSeaBottom, bool& intendedIndustry,
			bool& mustBeRated, int& minRating);
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuBdExchangeFilter_H
