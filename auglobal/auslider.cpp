//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auslider.cpp,v 1.2 2022/02/19 18:33:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auslider.h"
//

AuSliderValue::AuSliderValue(Qt::Orientation orientation, int value, QWidget* parent) : QSlider(orientation, parent)
{
	m_Value = value;
	connect(this, SIGNAL(valueChanged(int)), SLOT(BaseValueChanged(int)));
}

