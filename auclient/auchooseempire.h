//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auchooseempire.h,v 1.6 2022/02/19 17:53:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuChooseEmpire_H
#define AuChooseEmpire_H

#include "auclientlib.h"
#include <audialog.h>

class QComboBox;

class AUCLIENT_DLL AuChooseEmpire : public AuDialog
{
	Q_OBJECT;
private:
	quint32					m_EmpireId;	// The id of the selected empire.

	// Widgets
	QComboBox*				m_Empires;

public:
	AuChooseEmpire(const QString& caption, QWidget* parent=0);
	virtual ~AuChooseEmpire();

	inline quint32 EmpireId() const
	{
		return m_EmpireId;
	}

protected slots:
	void EmpireSelected(int index);
};

#endif // AuChooseEmpire_H
