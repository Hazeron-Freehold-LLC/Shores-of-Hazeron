//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupatentsrecord.h,v 1.6 2022/02/19 17:59:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuPatentsRecord_H
#define AuPatentsRecord_H

#include "auclientlib.h"
#include "audialog.h"
#include "auknownpatents.h"

class QListWidget;

class AUCLIENT_DLL AuPatentsRecord : public AuDialog
{
	Q_OBJECT;
	// Widgets
	QLabel*								m_CountLabel;
	QListWidget*						m_PatentList;

public:
	AuPatentsRecord(QWidget* parent=0);
	virtual ~AuPatentsRecord();

	// Fills 'patents' with the selected patents.
	void Patents(AuKnownPatents& patents) const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListPatents();
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void PatentSelectionChanged();
};

#endif // AuPatentsRecord_H
