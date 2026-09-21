//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubankloan.h,v 1.6 2022/02/19 17:50:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBankLoan_H
#define AuBankLoan_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"

class QLineEdit;

class AUCLIENT_DLL AuBankLoan : public AuDialog
{
	Q_OBJECT;
private:
	QLineEdit*							m_Amount;				// Amount entered.
	AuId								m_CapitolOpId;			// Id of bank when operated from a capitol building.

public:
	AuBankLoan(const AuId& opBd, QWidget* parent=0);
	virtual ~AuBankLoan();

	virtual QSize sizeHint() const;

protected slots:
	void BorrowFunds();
};

#endif // AuBankLoan_H
