//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auswitchavatar.h,v 1.6 2022/02/19 18:02:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSwitchAvatar_H
#define AuSwitchAvatar_H

#include "auclientlib.h"
#include <audialog.h>
#include <auid.h>

class AuPushButtonValue;

class AUCLIENT_DLL AuSwitchAvatar : public AuDialog
{
	Q_OBJECT;
private:
	bool								m_MailAlso;				// true to switch current mail recipient also
	AuId								m_SwitchTo;
	QTimer*								m_Timer;

	// Widgets
	AuPushButtonValue*					m_Avatars[6];

public:
	AuSwitchAvatar(QWidget* parent=0);
	virtual ~AuSwitchAvatar();

	inline bool SwitchMailAlso() const
	{
		return m_MailAlso;
	}

	inline const AuId& SwitchToId() const
	{
		return m_SwitchTo;
	}

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void showEvent(QShowEvent* event);

protected slots:
	void SwitchToAvatar(int index);
	void SwitchMailAlso(int checkBoxState);
	void UpdateContents();
};

#endif // AuSwitchAvatar_H
