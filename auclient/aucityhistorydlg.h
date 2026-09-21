//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucityhistorydlg.h,v 1.6 2022/02/19 17:53:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCityHistoryDlg_H
#define AuCityHistoryDlg_H

#include "aucityreport.h"
#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"

class QTextBrowser;
class QTimer;
class QToolButton;

class AUCLIENT_DLL AuCityHistoryDlg : public AuDialog
{
	Q_OBJECT;
private:
	AuId								m_CityId;
	AuCityReportHistory					m_ReportHistory;
	
	// Widgets
	QToolButton*						m_NextReport;
	QTextBrowser*						m_ReportWidget;

public:
	AuCityHistoryDlg(const AuId& cityId, const AuCityReportHistory& report, QWidget* parent=0);
	virtual ~AuCityHistoryDlg();

public slots:
	void ReportCityLast();
	virtual void setVisible(bool visible);
	void SelectReport(int reportIndex);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuCityHistoryDlg_H
