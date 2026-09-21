//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augadeposit.h,v 1.6 2022/02/19 17:56:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGADeposit_H
#define AuGADeposit_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"

class QLineEdit;

class AUCLIENT_DLL AuGADeposit : public AuDialog
{
	Q_OBJECT;
private:
	QLineEdit*							m_Amount;				// Amount entered.
	AuId								m_CapitolOpId;			// Id of bank when operated from a capitol building.

public:
	AuGADeposit(const AuId& opBd, QWidget* parent=0);
	virtual ~AuGADeposit();

protected slots:
	void DepositFunds();
};

#endif // AuGADeposit_H
