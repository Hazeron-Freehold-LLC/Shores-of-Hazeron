//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucityreportdlg.h,v 1.8 2022/05/05 19:33:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCityReportDlg_H
#define AuCityReportDlg_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"

class AuCityReport;

class AUCLIENT_DLL AuCityReportDlg : public AuDialog
{
	Q_OBJECT;
private:
	AuId								m_CityId;
	
	// Widgets
	QToolButton*						m_NextReport;

public:
	AuCityReportDlg(const AuId& cityId, const AuCityReport& report, QWidget* parent=0);
	virtual ~AuCityReportDlg();

	virtual QSize sizeHint() const;

public slots:
	void ReportCityLast();
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuCityReportDlg_H
