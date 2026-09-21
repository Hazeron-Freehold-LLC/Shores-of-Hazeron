//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubiobounties.h,v 1.7 2022/02/19 17:50:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBioBounties_H
#define AuBioBounties_H

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

class AUCLIENT_DLL AuBioBounties : public QWidget
{
	Q_OBJECT;
private:
	double								m_AccountBalance;
	QStandardItemModel					m_BountyModel;
	double								m_InsuranceClaim;
	QTimer*								m_timer;

	// Widgets
	QLabel*								lbbounties;
	QTreeView*							m_BountyTree;
	QPushButton*						m_PayInsurer;

public:
	AuBioBounties(QWidget *parent=0);
	~AuBioBounties();

	virtual QSize sizeHint() const;

protected:
	virtual void showEvent(QShowEvent* event);

protected slots:
	void PayInsurer();
	void UpdateContents();
};

#endif // AuBioBounties_H
