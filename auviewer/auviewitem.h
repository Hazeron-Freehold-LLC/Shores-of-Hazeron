//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auviewitem.h,v 1.4 2022/02/19 18:42:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuViewItem_H
#define AuViewItem_H

#include <audialog.h>
#include "dbcommodity.h"

class QComboBox;

class AuViewItem : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QComboBox*							m_Commodity;

public:
	AuViewItem(QWidget* parent=0);
	virtual ~AuViewItem();

	COMMODITYID CommodityId() const;
};

#endif // AuViewItem_H
