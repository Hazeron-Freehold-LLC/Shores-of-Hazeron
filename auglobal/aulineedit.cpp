//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulineedit.cpp,v 1.2 2022/02/19 18:33:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aulineedit.h"
//
#include <QKeyEvent>

AuLineEdit::AuLineEdit(QWidget* parent) : QLineEdit(parent)
{
}

AuLineEdit::AuLineEdit(const QString& contains, QWidget* parent) : QLineEdit(contains, parent)
{
}

void AuLineEdit::keyPressEvent(QKeyEvent* ev)
{
	int key = ev->key();

	QLineEdit::keyPressEvent(ev);

	switch(key)
	{
	case Qt::Key_Up:
		emit UpArrowPressed();
		break;
	case Qt::Key_Down:
		emit DownArrowPressed();
		break;
	}
}

void AuLineEdit::keyReleaseEvent(QKeyEvent* ev)
{
	int key = ev->key();

	QLineEdit::keyReleaseEvent(ev);

	switch(key)
	{
	case Qt::Key_Up:
		emit UpArrowReleased();
		break;
	case Qt::Key_Down:
		emit DownArrowReleased();
		break;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuLineEditValue::AuLineEditValue(int value, QWidget* parent) : QLineEdit(parent)
{
	m_Value = value;
	connect(this, SIGNAL(editingFinished()), SLOT(BaseEditingFinished()));
}

AuLineEditValue::AuLineEditValue(const QString& contains, int value, QWidget* parent) : QLineEdit(contains, parent)
{
	m_Value = value;
	connect(this, SIGNAL(editingFinished()), SLOT(BaseEditingFinished()));
}

