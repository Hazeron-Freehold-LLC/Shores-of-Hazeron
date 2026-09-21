//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dttrash.h,v 1.4 2022/02/19 17:38:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtTrash_H
#define DtTrash_H

#include <QThread>
#include <QSemaphore>

class DaAtom;

class DtTrash : public QThread
{
	Q_OBJECT;
public:
	DtTrash(QObject* parent=0);
	~DtTrash();

	inline bool Alive() const
	{
		return !m_semAlive.available();
	}

	inline void Kill()
	{
		m_semAlive.release(1);
	}

protected:
	virtual void run();

private:
	QSemaphore		m_semAlive;			// Used to know when I'm supposed to die.
};

#endif // DtTrash_H
