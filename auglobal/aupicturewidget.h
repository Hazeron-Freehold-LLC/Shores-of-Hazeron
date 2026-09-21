//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupicturewidget.h,v 1.4 2022/02/19 18:31:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuPictureWidget_H
#define AuPictureWidget_H

#include "auglobal.h"
#include <QWidget>

class AUGLOBAL_DLL AuPictureWidget : public QWidget
{
	Q_OBJECT;
private:
	bool								m_Auto;					// stretches image to fill the space. Ignores pan and zoom.
	QColor								m_BackgroundColor;
	QImage								m_Image;
	int									m_LeftButtonMode;		// 0=none, 1=pan, 2=zoom, 3=resize compass
	QPointF								m_MiddlePixel;			// Pixel in m_Image that is centered in the widget.
	QPoint								m_MouseDownPt;
	float								m_Scale;

public:
	AuPictureWidget(bool autoScale, QWidget* parent=0);
	virtual ~AuPictureWidget();

	inline float Scale() const
	{
		return m_Scale;
	}

	void SetBackgroundColor(const QColor& bgcol);

	void SetImage(const QImage& image);

	virtual QSize sizeHint() const;

public slots:
	virtual void setVisible(bool visible);
	void SetScale(float scale);

protected:
	virtual void mouseDoubleClickEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void paintEvent(QPaintEvent* event);
	virtual void showEvent(QShowEvent* event);
	virtual void wheelEvent(QWheelEvent* e);

signals:
	void ScaleChanged(float scale);
};

#endif // AuPictureWidget_H

