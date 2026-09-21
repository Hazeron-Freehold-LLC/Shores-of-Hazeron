//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubuyspacecraft.h,v 1.6 2022/02/19 17:52:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBuySpacecraft_H
#define AuBuySpacecraft_H

#include "auclientlib.h"
#include "audialog.h"
#include "augear.h"

class QComboBox;
class QLabel;

class AUCLIENT_DLL AuBuySpacecraft : public AuDialog
{
	Q_OBJECT;
private:
	AuGearPos							m_BuyDesign;
	quint32								m_DesignId;
	quint8								m_MaxQ;

	// Widgets
	QComboBox*							m_BuyWhat;
	QLabel*								m_Price;

public:
	AuBuySpacecraft(quint8 maxQ, QWidget* parent=0);
	virtual ~AuBuySpacecraft();

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
	void BuySelectionChanged(int index);
};

#endif // AuBuySpacecraft_H
