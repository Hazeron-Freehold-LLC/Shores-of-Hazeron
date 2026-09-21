//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdshrubdlg.h,v 1.2 2022/02/19 18:21:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdShrubDlg_H
#define AuBdShrubDlg_H

#include "audialog.h"
#include "dbcommodity.h"

class QComboBox;

class AuBdShrubDlg : public AuDialog
{
	Q_OBJECT;
	QComboBox*							m_ResourceChoices;

public:
	AuBdShrubDlg(QWidget* parent=0);
	virtual ~AuBdShrubDlg();

	COMMODITYID PlantCID() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListResources();
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoOk();
};

#endif // AuBdShrubDlg_H
