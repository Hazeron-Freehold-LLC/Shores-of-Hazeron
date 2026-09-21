//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aukeyfilter.h,v 1.7 2022/02/19 17:57:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuKeyFilter_H
#define AuKeyFilter_H

#include "auclientlib.h"
#include <QObject>
#include <QTime>

class QKeyEvent;

// The key filter monitors the state of the Ctrl key.
class AUCLIENT_DLL AuKeyFilter : public QObject
{
	Q_OBJECT;
	Q_DISABLE_COPY(AuKeyFilter);
public:
	AuKeyFilter(QObject* parent=0);

	static QString KeyName(Qt::Key key);

	static QString ModifierName(Qt::KeyboardModifiers modifiers);

signals:
	void KeyPressed(QKeyEvent* e);
	void KeyReleased(QKeyEvent* e);

protected:
	bool eventFilter(QObject* obj, QEvent* event);
};

#endif // AuKeyFilter_H
