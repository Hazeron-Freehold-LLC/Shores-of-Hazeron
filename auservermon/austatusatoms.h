//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austatusatoms.h,v 1.3 2022/02/19 18:40:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStatusAtoms_H
#define AuStatusAtoms_H

#include "auid.h"
#include "dtmgrclient.h"
#include <QWidget>

class AuStatusAtoms : public QWidget
{
	Q_OBJECT;
private:
	int								m_Mode;		// 0=averages, 1=maximums
	const QList<AuStatusReport>*	m_Status;
	AuId							m_SystemId;	// If cursor is over a system's bars.
	int								m_TMinus;

public:
	AuStatusAtoms(const QList<AuStatusReport>* status, QWidget* parent=0);
	virtual ~AuStatusAtoms();

	virtual void paintEvent(QPaintEvent* pe);
	virtual QSize sizeHint() const;
	
public slots:
	void SetTMinus(int ms);

protected:
	virtual void mouseReleaseEvent(QMouseEvent* event);

signals:
	void SystemRightClicked(AuId systemId);
};

#endif // AuStatusAtoms_H
