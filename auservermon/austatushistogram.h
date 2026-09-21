//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austatushistogram.h,v 1.6 2022/02/19 18:40:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStatusHistogram_H
#define AuStatusHistogram_H

#include "dtmgrclient.h"
#include <QWidget>

class AuStatusHistogram : public QWidget
{
	Q_OBJECT;
private:
	int								m_Mode;		// 0=averages, 1=maximums, 2=cycles
	const QList<AuStatusReport>*	m_Status;
	int								m_TMinus;

public:
	AuStatusHistogram(const QList<AuStatusReport>* status, int mode, QWidget* parent=0);
	virtual ~AuStatusHistogram();

	virtual void paintEvent(QPaintEvent* pe);
	virtual QSize sizeHint() const;
	
signals:
	void TMinus(int ms);

public slots:
	void SetTMinus(int ms);

protected:
	void Plot(QPainter& p, int curX, int margin, int value);
};

#endif // AuStatusHistogram_H
