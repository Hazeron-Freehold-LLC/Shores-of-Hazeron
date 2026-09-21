//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AuMsgDialog.h,v 1.6 2022/02/19 17:58:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMsgDialog_H
#define AuMsgDialog_H

#include "auclientlib.h"
#include <audialog.h>
#include <QWidget>

class AUCLIENT_DLL AuMsgDialog : public AuDialog
{
	Q_OBJECT;
	int									m_Mode;
	int									m_Step;

	// Widgets
	QLabel*								m_IconLabel;
	QLabel*								m_MsgLabel;
	
public:
	// Constructor / Destructor
	AuMsgDialog(QWidget* parent=0, QString sMsg="", bool bTitle = false);
	~AuMsgDialog();

	void SetMessage(int mode, QString sMsg);
	void StepNext();
};

#endif //AuMsgDialog_H
