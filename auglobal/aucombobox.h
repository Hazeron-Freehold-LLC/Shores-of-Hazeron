//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucombobox.h,v 1.3 2022/02/19 18:30:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuComboBox_H
#define AuComboBox_H

#include "augloballib.h"
#include <QComboBox>

class QSpinBox;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuComboBoxValue : public QComboBox
{
	Q_OBJECT;
public:
	AuComboBoxValue(int value, QWidget* parent=0) : QComboBox(parent)
	{
		m_Value = value;
		connect(this, SIGNAL(currentIndexChanged(int)), SLOT(BaseIndexChanged(int)));
	}

signals:
	void CurrentIndexChanged(int index, int value);

protected slots:
	void BaseIndexChanged(int index)
	{
		// Emit my version of the index changed signal that passes the value.
		emit CurrentIndexChanged(index, m_Value);
	}

private:
	int		m_Value;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuComboNumberValue : public QWidget
{
	Q_OBJECT;
public:
	AuComboNumberValue(int value, QWidget* parent);

	QComboBox* ComboBox();

	QSpinBox* SpinBox();

signals:
	void CurrentIndexChanged(int index, int value);
	void CurrentNumberChanged(int number, int value);

protected slots:
	void BaseIndexChanged(int index)
	{
		// Emit my version of the index changed signal that passes the value.
		emit CurrentIndexChanged(index, m_Value);
	}

	void BaseNumberChanged(int value)
	{
		// Emit my version of the number changed signal that passes the value.
		emit CurrentNumberChanged(value, m_Value);
	}

private:
	int		m_Value;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuComboBox_H
