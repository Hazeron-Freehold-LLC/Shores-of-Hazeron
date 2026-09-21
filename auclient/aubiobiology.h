//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubiobiology.h,v 1.13 2022/04/20 16:46:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBioBiology_H
#define AuBioBiology_H

#include "auclientlib.h"
#include "audockwidget.h"

class QCheckBox;
class QLabel;
class QPushButton;
class QTimer;

class QTableWidgetItem;

class AUCLIENT_DLL AuBioBiology : public QWidget
{
	Q_OBJECT;
private:
	QTimer*								m_timer;

	// Widgets
	QCheckBox*							ckautoeat;
	QLabel*								lbborn;
	QLabel*								lbdesc;
	QLabel*								lbgender;
	QLabel*								lbhp;
	QLabel*								lbhandedness;
	QLabel*								lbhunger;
	QLabel*								lblineage;
	QLabel*								lbstam;
	QLabel*								lbweight;
	QLabel*								m_ImmunizationIcon;
	QLabel*								m_Immunization;
	QPushButton*						pbnamebaby;
	QPushButton*						m_UpgradeDNA;

public:
	AuBioBiology(QWidget *parent=0);
	~AuBioBiology();

	virtual QSize sizeHint() const;

protected:
	virtual void showEvent(QShowEvent* event);

protected slots:
	void AutoEatChanged(int state);
	void NameTheBaby();
	void UpdateContents();
	void UpgradeDNA();
};

#endif // AuBioBiology_H
