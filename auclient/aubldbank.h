//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldbank.h,v 1.6 2022/02/19 17:51:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBldBank_H
#define AuBldBank_H

#include "auclientlib.h"
#include "auid.h"
#include <QWidget>

class QLineEdit;
class QTimer;

class AUCLIENT_DLL AuBldBank : public QWidget
{
	Q_OBJECT;
private:
	AuId								m_CapitolOpId;			// Id of building when operated from a capitol building.
	QTimer*								m_timer;

	// Widgets
	QLineEdit*							m_PlayerLoansEdit;
	QLineEdit*							m_PlayerOwedEdit;

public:
	AuBldBank(QWidget* parent=0);
	virtual ~AuBldBank();

	// Clear information needed when this building is operated from a capitol building.
	void ClearCapitolOp()
	{
		m_CapitolOpId = AuId(0);
	}

	// Set information needed when this building is operated from a capitol building.
	void SetCapitolOp(const AuId& capitolOpId)
	{
		m_CapitolOpId = capitolOpId;
	}

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void showEvent(QShowEvent* event);

protected slots:
	void ClaimRewards();
	void GADepositFunds();
	void GARequestBalance();
	void GAWithdrawFunds();
	void PayDebt();
	void RequestLoan();
	void TimerUpdate();
};

#endif // AuBldBank_H
