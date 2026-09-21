//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldrepairshop.h,v 1.6 2022/02/19 17:52:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBldRepairShop_H
#define AuBldRepairShop_H

#include "auclientlib.h"
#include <QWidget>

class QLabel;
class QPushButton;

class AUCLIENT_DLL AuBldRepairShop : public QWidget
{
	Q_OBJECT;
private:
	QLabel*								m_Info;
	QPushButton*						m_Recover;

public:
	AuBldRepairShop(QWidget* parent=0);
	virtual ~AuBldRepairShop();

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void showEvent(QShowEvent* event);

protected slots:
	void Recover();
};

#endif // AuBldRepairShop_H
