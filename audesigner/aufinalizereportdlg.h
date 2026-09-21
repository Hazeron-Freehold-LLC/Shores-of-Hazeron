//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aufinalizereportdlg.h,v 1.2 2022/02/19 18:21:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuFinalizeReportDlg_H
#define AuFinalizeReportDlg_H

#include "audialog.h"

class AuDesign;
class QTextBrowser;

class AuFinalizeReportDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QTextBrowser*						m_Verification;

public:
	AuFinalizeReportDlg(const QString& report, QWidget* parent=0);
	virtual ~AuFinalizeReportDlg();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuFinalizeReportDlg_H
