//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubankpay.h,v 1.6 2022/02/19 17:50:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBankPay_H
#define AuBankPay_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"

class QLineEdit;

class AUCLIENT_DLL AuBankPay : public AuDialog
{
	Q_OBJECT;
private:
	QLineEdit*							m_Amount;				// Amount entered.
	AuId								m_CapitolOpId;			// Id of bank when operated from a capitol building.

public:
	AuBankPay(const AuId& opBd, QWidget* parent=0);
	virtual ~AuBankPay();

	virtual QSize sizeHint() const;

protected slots:
	void MakePayment();
};

#endif // AuBankPay_H
