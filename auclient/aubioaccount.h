//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubioaccount.h,v 1.5 2022/02/19 17:50:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBioAccount_H
#define AuBioAccount_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "aujoboffer.h"
#include <QPaintEvent>

class AuOpenStoryView;
class AuPushButtonValue;
class QLabel;
class QPushButton;
class QTextEdit;
class QTimer;
class QTreeView;

class AUCLIENT_DLL AuBioAccount : public QWidget
{
	Q_OBJECT;
	QTimer*								m_Timer;

	// Widgets
	QLabel*								lbIcon;
	QLabel*								lbName;
	QLabel*								lbStatus;

public:
	AuBioAccount(QWidget *parent=0);
	~AuBioAccount();

protected:
	virtual void showEvent(QShowEvent* event);

protected slots:
	void UpdateContents();
};

#endif // AuBioAccount_H
