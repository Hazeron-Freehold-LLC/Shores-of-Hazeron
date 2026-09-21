//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auhistopair.h,v 1.7 2022/02/19 18:40:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuHistoPair_H
#define AuHistoPair_H

#include "auid.h"
#include "dtmgrclient.h"
#include <QWidget>

class AuStatusAtoms;
class AuStatusHistogram;

class AuHistoPair : public QWidget
{
	Q_OBJECT;
private:
	const QList<AuStatusReport>*	m_Status;

	// Widgets
	AuStatusAtoms*			m_StatusAtoms;
	AuStatusHistogram*		m_StatusHistoAvg;
	AuStatusHistogram*		m_StatusHistoCycle;
	AuStatusHistogram*		m_StatusHistoMax;

public:
	AuHistoPair(const QList<AuStatusReport>* status, QWidget* parent=0);
	virtual ~AuHistoPair();

signals:
	void SystemRightClicked(AuId systemId);
	void TMinus(int ms);

public slots:
	void SetTMinus(int ms);

protected slots:
	void TimerUpdate();
};

#endif // AuHistoPair_H
