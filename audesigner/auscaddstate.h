//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscaddstate.h,v 1.2 2022/02/19 18:22:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScAddState_H
#define AuScAddState_H

#include "audialog.h"

class QComboBox;

class AuScAddState : public AuDialog
{
	Q_OBJECT;
	// Widgets
	QComboBox*							m_States;

public:
	AuScAddState(const QMap<quint8, QString>& addableStates, QWidget* parent=0);
	virtual ~AuScAddState();

	quint8 State() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuScAddState_H
