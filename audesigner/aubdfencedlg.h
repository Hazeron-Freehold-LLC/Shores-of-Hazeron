//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdfencedlg.h,v 1.3 2022/02/19 18:21:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdFenceDlg_H
#define AuBdFenceDlg_H

#include "audialog.h"

class QComboBox;

class AuBdFenceDlg : public AuDialog
{
	Q_OBJECT;
	QComboBox*							m_StyleChoices;

public:
	AuBdFenceDlg(QWidget* parent=0);
	virtual ~AuBdFenceDlg();

	// Only non-zero if style is custom.
	quint32 DesignId() const;

	// Return value is an AuScFence::STYLE.
	int Style() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListStyles();
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoOk();
};

#endif // AuBdFenceDlg_H
