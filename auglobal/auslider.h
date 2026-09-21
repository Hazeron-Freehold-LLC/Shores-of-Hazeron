//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auslider.h,v 1.3 2022/02/19 18:31:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSlider_H
#define AuSlider_H

#include "augloballib.h"
#include <QSlider>

class AUGLOBAL_DLL AuSliderValue : public QSlider
{
	Q_OBJECT;
public:
	AuSliderValue(Qt::Orientation orientation, int value, QWidget* parent=0);

	// Returns the identifier value, not the value of the slider.
	inline int Value() const
	{
		return m_Value;
	}

signals:
	void ValueChanged(int sliderValue, int value);

protected slots:
	void BaseValueChanged(int sliderValue)
	{
		// Emit my version of the value changed signal that passes the value associated with the widget.
		emit ValueChanged(sliderValue, m_Value);
	}

private:
	int		m_Value;					// The widget value passed in the constructor, not the value of the slider.
};

#endif // AuSlider_H
