//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdtreedlg.h,v 1.2 2022/02/19 18:21:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdTreeDlg_H
#define AuBdTreeDlg_H

#include "audialog.h"
#include "dbcommodity.h"

class QComboBox;

class AuBdTreeDlg : public AuDialog
{
	Q_OBJECT;
	QComboBox*							m_ResourceChoices;

public:
	AuBdTreeDlg(QWidget* parent=0);
	virtual ~AuBdTreeDlg();

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

#endif // AuBdTreeDlg_H
