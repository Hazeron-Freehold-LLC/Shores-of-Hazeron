//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AuSendFunds.h,v 1.7 2022/02/19 18:00:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSendFunds_H
#define AuSendFunds_H

#include "auclientlib.h"
#include <audialog.h>
#include <auid.h>

class QLineEdit;
class QPushButton;

class AUCLIENT_DLL AuSendFunds : public AuDialog
{
	Q_OBJECT;
	double								m_AccountBalance;
	AuId								m_ToId;
	QString								m_ToName;

	// Widgets
	QLineEdit*							m_AmountEdit;
	QPushButton*						m_SendButton;

public:
	AuSendFunds(const QString& titleText, const QString& toName, const AuId& toId, QWidget* parent=0);
	virtual ~AuSendFunds();

protected slots:
	void AmountChanged(const QString& amt);
	void SendFunds();
};

#endif // AuSendFunds_H
