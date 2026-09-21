//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augawithdraw.h,v 1.6 2022/02/19 17:56:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGAWithdraw_H
#define AuGAWithdraw_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"

class QLineEdit;

class AUCLIENT_DLL AuGAWithdraw : public AuDialog
{
	Q_OBJECT;
private:
	QLineEdit*							m_Amount;				// Amount entered.
	AuId								m_CapitolOpId;			// Id of bank when operated from a capitol building.

public:
	AuGAWithdraw(const AuId& opBd, QWidget* parent=0);
	virtual ~AuGAWithdraw();

protected slots:
	void WithdrawFunds();
};

#endif // AuGAWithdraw_H
