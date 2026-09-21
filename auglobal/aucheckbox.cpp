//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucheckbox.cpp,v 1.3 2022/02/19 18:32:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucheckbox.h"
//

AuCheckBoxValue::AuCheckBoxValue(const QString& text, int value, QWidget* parent) : QCheckBox(text, parent)
{
	m_Value = value;
	connect(this, SIGNAL(stateChanged(int)), SLOT(BaseStateChanged(int)));
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCheckBoxValue64::AuCheckBoxValue64(const QString& text, qint64 value, QWidget* parent) : QCheckBox(text, parent)
{
	m_Value = value;
	connect(this, SIGNAL(stateChanged(int)), SLOT(BaseStateChanged(int)));
}

