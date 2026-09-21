//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscdeletestate.h,v 1.2 2022/02/19 18:22:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScDeleteState_H
#define AuScDeleteState_H

#include "audialog.h"

class QComboBox;

class AuScDeleteState : public AuDialog
{
	Q_OBJECT;
	// Widgets
	QComboBox*							m_States;

public:
	AuScDeleteState(const QMap<quint8, QString>& deletableStates, QWidget* parent=0);
	virtual ~AuScDeleteState();

	quint8 State() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuScDeleteState_H
