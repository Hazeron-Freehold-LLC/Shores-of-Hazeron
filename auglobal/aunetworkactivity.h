//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aunetworkactivity.h,v 1.3 2022/02/19 18:31:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuNetworkActivity_H
#define AuNetworkActivity_H

#include "augloballib.h"
#include <QPushButton>

class QHideEvent;
class QPaintEvent;
class QShowEvent;
class QTimer;

class AUGLOBAL_DLL AuNetworkActivity : public QPushButton
{
	Q_OBJECT;
private:
	int									m_Net1Down;
	int									m_Net1Up;
	int									m_Net2Down;
	int									m_Net2Up;
	int									m_Net3Down;
	int									m_Net3Up;
	int									m_Net4Down;
	int									m_Net4Up;
	QImage								m_NetImageDown;
	QImage								m_NetImageUp;
	QTimer*								m_Timer;

public:
	AuNetworkActivity(QWidget* parent=0);

	virtual QSize sizeHint() const;

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void paintEvent(QPaintEvent* ev);
    virtual void showEvent(QShowEvent* event);
};

#endif // AuNetworkActivity_H
