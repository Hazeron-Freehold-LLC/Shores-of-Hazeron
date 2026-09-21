//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auindustrydlg.h,v 1.6 2022/02/19 17:57:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuIndustryDlg_H
#define AuIndustryDlg_H

#include "auclientlib.h"
#include "audialog.h"

class QComboBox;
class QTextBrowser;

class AUCLIENT_DLL AuIndustryDlg : public AuDialog
{
	Q_OBJECT;
	// Widgets
	QComboBox*							m_IndustriesCombo;
	QTextBrowser*						m_IndustryInfo;

public:
	AuIndustryDlg(QWidget* parent=0);
	virtual ~AuIndustryDlg();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void IndustryChanged(int index);
};

#endif // AuIndustryDlg_H
