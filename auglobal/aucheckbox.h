//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucheckbox.h,v 1.4 2022/02/19 18:30:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCheckBox_H
#define AuCheckBox_H

#include "augloballib.h"
#include <QCheckBox>

class AUGLOBAL_DLL AuCheckBoxValue : public QCheckBox
{
	Q_OBJECT;
public:
	AuCheckBoxValue(const QString& text, int value, QWidget* parent=0);

	inline int Value() const
	{
		return m_Value;
	}

signals:
	void StateChanged(int state, int value);

protected slots:
	void BaseStateChanged(int state)
	{
		// Emit my version of the state changed signal that passes the value.
		emit StateChanged(state, m_Value);
	}

private:
	int		m_Value;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuCheckBoxValue64 : public QCheckBox
{
	Q_OBJECT;
public:
	AuCheckBoxValue64(const QString& text, qint64 value, QWidget* parent=0);

	inline qint64 Value() const
	{
		return m_Value;
	}

signals:
	void StateChanged(int state, qint64 value);

protected slots:
	void BaseStateChanged(int state)
	{
		// Emit my version of the state changed signal that passes the value.
		emit StateChanged(state, m_Value);
	}

private:
	qint64		m_Value;
};

#endif // AuCheckBox_H
