//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommissionspacecraft.h,v 1.6 2022/02/19 17:54:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCommissionSpacecraft_H
#define AuCommissionSpacecraft_H

#include "auclientlib.h"
#include "audialog.h"
#include "augear.h"

class QComboBox;
class QLabel;

class AUCLIENT_DLL AuCommissionSpacecraft : public AuDialog
{
	Q_OBJECT;
private:
	AuGearPos							m_BuyDesign;			// Design selected to buy, if accepted.
	quint32								m_DesignId;

	// Widgets
	QComboBox*							m_BuyWhat;

public:
	AuCommissionSpacecraft(QWidget* parent=0);
	virtual ~AuCommissionSpacecraft();

	inline quint32 DesignId() const
	{
		return m_DesignId;
	}

	inline AuGearPos GearPos() const
	{
		return m_BuyDesign;
	}

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListDesigns();
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void BuyOne();
};

#endif // AuCommissionSpacecraft_H
