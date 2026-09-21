//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupostbounty.h,v 1.6 2022/02/19 17:59:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuPostBounty_H
#define AuPostBounty_H

#include "auclientlib.h"
#include <audialog.h>

class QLineEdit;
class QPushButton;

class AUCLIENT_DLL AuPostBounty : public AuDialog
{
	Q_OBJECT;
private:
	double								m_Account;				// Money currently in the poster's personal account.
	QString								m_Reason;
	double								m_Reward;
	QString								m_VictimName;

	// Widgets
	QPushButton*						m_Ok;
	QLineEdit*							m_ReasonEdit;
	QLineEdit*							m_RewardEdit;

public:
	// mode: 0 = posting to comm message sender, 1 posting to spotted atom
	AuPostBounty(const QString& victimName, double account, int mode, QWidget* parent=0);
	virtual ~AuPostBounty();

	inline const QString& Reason() const
	{
		return m_Reason;
	}

	inline double Reward() const
	{
		return m_Reward;
	}

protected slots:
	void EnableControls();
	void ReasonChanged(const QString& reason);
	void RewardChanged(const QString& reward);
};

#endif // AuPostBounty_H
