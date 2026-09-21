//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dttarget.h,v 1.4 2022/02/19 18:25:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtTarget_H
#define DtTarget_H

#include <QSemaphore>
#include <QThread>

class AuMission;
class AuDesignerView;

class DtTarget : public QThread
{
	Q_OBJECT;
private:
	QSemaphore							m_semAlive;				// Used to know when I'm supposed to die.

public:
	DtTarget(AuDesignerView* parent=0);
	~DtTarget();

	bool Alive() const
	{
		return !m_semAlive.available();
	}

	void Kill()
	{
		if(Alive())
			m_semAlive.release(1);
	}

protected:
	virtual void run();
};

#endif // DtTarget_H
