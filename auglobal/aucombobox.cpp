//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucombobox.cpp,v 1.2 2022/02/19 18:32:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucombobox.h"
//
#include <QGridLayout>
#include <QSpinBox>

AuComboNumberValue::AuComboNumberValue(int value, QWidget* parent) : QWidget(parent)
{
	m_Value = value;

	QComboBox* cb = new QComboBox;
	cb->setObjectName("cb");
	connect(cb, SIGNAL(currentIndexChanged(int)), SLOT(BaseIndexChanged(int)));

	QSpinBox* sb = new QSpinBox;
	sb->setAlignment(Qt::AlignRight);
	sb->setObjectName("sb");
	connect(sb, SIGNAL(valueChanged(int)), SLOT(BaseNumberChanged(int)));

	QGridLayout* hlay = new QGridLayout(this);
	hlay->addWidget(cb, 0,0);
	hlay->setColumnStretch(0, 1);
	hlay->addWidget(sb, 0,1);
	hlay->setMargin(0);
	hlay->setSpacing(2);
	setLayout(hlay);
}

QComboBox* AuComboNumberValue::ComboBox()
{
	return findChild<QComboBox*>("cb");
}

QSpinBox* AuComboNumberValue::SpinBox()
{
	return findChild<QSpinBox*>("sb");
}
