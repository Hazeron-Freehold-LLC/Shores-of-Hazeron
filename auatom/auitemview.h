//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitemview.h,v 1.6 2022/02/19 17:31:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuItemView_H
#define AuItemView_H

#include "aumatrix4x4.h"
#include "aupoint.h"
#include "dbcommodity.h"
#include <QOpenGLWidget>

class AuAtomRenderInfo;
class AuGLModelCache;
class AuGLModelMaker;
class AuItem;
class AuMatrix4x4;
class QWheelEvent;

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuItemView : public QOpenGLWidget
{
	Q_OBJECT;
private:
	AuPoint								m_AmbientUp;
	float								m_BusyAngle;
	GLuint								m_BusyTex;
	AuGLVAO								m_BusyVAO;
	QFont								m_Font;
    AuMatrix4x4							m_Model;				// Model matrix.
	AuGLModelCache*						m_ModelCache;
	Qt::MouseButtons					m_MouseButtons;			// Buttons during last mouse press event.
	QPoint								m_MouseLast;
	AuGLModelMaker*						m_MM;					// Model maker.
	AuAtomRenderInfo*					m_RenderInfo;			// Values needed by atoms when rendering them.

	// Things to render.
	AuItem*								m_Item;

public:	// Constructors
	AuItemView(QWidget *parent=0);

	~AuItemView();

public:
	virtual void initializeGL();

	virtual void paintGL();

	AuPoint PointAtPixel(const QPoint& pixel) const;

	void SetupLights(const AuMatrix4x4& model);

public slots:
	void ResetView();

	void ViewItem(const AuItem& item);

protected:
	void DeleteViewObjects();

	void ShowBusyIndicator();

	virtual void mouseDoubleClickEvent(QMouseEvent* e);

	virtual void mouseMoveEvent(QMouseEvent* e);

	virtual void mousePressEvent(QMouseEvent* e);

	virtual void wheelEvent(QWheelEvent* e);
};

#endif // AuItemView_H
