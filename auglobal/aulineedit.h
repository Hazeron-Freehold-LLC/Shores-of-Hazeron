//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulineedit.h,v 1.3 2022/02/19 18:31:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuLineEdit_H
#define AuLineEdit_H

#include "augloballib.h"
#include <QLineEdit>

class QHideEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QShowEvent;
class QTimer;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuLineEdit : public QLineEdit
{
	Q_OBJECT;
public:
	AuLineEdit(QWidget* parent=0);
	AuLineEdit(const QString& contains, QWidget* parent=0);

protected:
	virtual void keyPressEvent(QKeyEvent* ev);
	virtual void keyReleaseEvent(QKeyEvent* ev);

signals:
	void DownArrowPressed();
	void DownArrowReleased();
	void UpArrowPressed();
	void UpArrowReleased();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuLineEditValue : public QLineEdit
{
	Q_OBJECT;
private:
	int									m_Value;

public:
	AuLineEditValue(int value, QWidget* parent=0);
	AuLineEditValue(const QString& contains, int value, QWidget* parent=0);

signals:
	void EditingFinished(int value);

public slots:
	void BaseEditingFinished()
	{
		// Emit my version of the clicked signal that passes the value.
		emit EditingFinished(m_Value);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuLineEdit_H
