//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdstorage.h,v 1.3 2022/02/19 18:21:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdStorage_H
#define AuBdStorage_H

#include "audialog.h"
#include "dbindustry.h"

class QComboBox;
class QLabel;

class AuBdStorage : public AuDialog
{
	Q_OBJECT;
	INDUSTRYID							m_IID;

	QComboBox*							m_StoreCombo;

public:
	AuBdStorage(INDUSTRYID iid, QWidget* parent=0);
	virtual ~AuBdStorage();

	COMMODITYID StoreCommodity() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuBdStorage_H
