//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucompasswidget.h,v 1.2 2022/02/19 18:30:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCompassWidget_H
#define AuCompassWidget_H

#include "auglobal.h"
#include <QWidget>

class AUGLOBAL_DLL AuCompassWidget : public QWidget
{
	Q_OBJECT;
private:
	QImage								m_Compass;
	QImage								m_CompassShadow;
	double								m_Heading;				// In radians

public:
	AuCompassWidget(QWidget* parent=0);
	virtual ~AuCompassWidget();

	inline void SetHeading(double heading)
	{
		if(m_Heading == heading)
			return;
		m_Heading = heading;
		update();
	}

	virtual QSize sizeHint() const;

protected:
	virtual void paintEvent(QPaintEvent* event);
};

#endif // AuCompassWidget_H
