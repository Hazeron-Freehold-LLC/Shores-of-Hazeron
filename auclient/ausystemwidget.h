//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausystemwidget.h,v 1.21 2022/04/14 15:23:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSystemWidget_H
#define AuSystemWidget_H

#include "auclientlib.h"
#include "auid.h"
#include "aupoint.h"
#include "ausensorblip.h"
#include "dbcommodity.h"
#include <QWidget>

class DaAtom;
class DaSolarSystem;
class DaStar;
class QTimer;

class AUCLIENT_DLL AuSystemWidget : public QWidget
{
	Q_OBJECT;
private:
	QHash<DaAsteroid*, AuPoint>			m_Asteroids;			// key is clone, value is env location
	QHash<AuId, QRect>					m_Bodies;
	QCursor								m_Cursor;				// Crosshair cursor
	AuPoint								m_fPan;
	AuPoint								m_fRot;
	AuElapsedTimer						m_LastTick;
	int									m_LeftButtonMode;
	quint8								m_MinQToShow;
	QPoint								m_MouseDownPt;
	int									m_RightButtonMode;
	QHash<AuId, QRect>					m_Rings;
	int									m_ScanEx;				// 2=scanned, 1=explored, 0=observed
	AuId								m_Selected;
	DaSolarSystem*						m_System;
	QTimer*								m_timer;
	AuSensorBlip::TYPE					m_ViewerBlip;
	AuPoint								m_ViewerLoc;
	POLITICAL_STANCE					m_ViewerStance;
	float								m_Zoom;

public:
	AuSystemWidget(QWidget* parent=0);
	virtual ~AuSystemWidget();

	inline void SetSelected(const AuId& selected)
	{
		m_Selected = selected;
		update();
	}

	void SetSystem(DaSolarSystem* system, int scanEx);

	void SetViewerLoc(AuSensorBlip::TYPE viewerBlip, POLITICAL_STANCE viewerStance, const AuPoint& viewerLoc,
			const DaAtomList& asteroids);

signals:
	void Attack(AuId roidId, QString roidName);
	void BodyClicked(AuId bodyId);
	void Follow(AuId roidId, QString roidName);
	void GoLand(AuId id, QString catalogName);
	void GoOrbit(AuId id, QString catalogName);
	void GoToCoord(AuPoint systemLoc);

public slots:
	void SetMinQToShow(int minQ);
	void ViewReset();

protected:
	AuId BodyAt(const QPoint& px) const;
	void ClearAsteroids();
	void DrawAsteroids(QPainter& p, const AuMatrix4x4& mvp, double pixelsPerOrbit, const DaAtom*& selected, QRect& rcSel);
	void DrawCircle(QPainter& p, const AuMatrix4x4& mvp, const AuPoint& center, double radius, const AuMatrix4x4& model, int sides);
	void DrawSolarSystemDisplay(QPainter& p);
	void DrawStarAndPlanets(QPainter& p, const AuMatrix4x4& mvp, DaStar *pstar, double pixelsPerOrbit, bool bOrbit[6],
			const DaAtom*& selected, QRect& rcSel);
	virtual void keyPressEvent(QKeyEvent *e);

	// Create orthographic model view projection.
	// Returns pixels per orbit.
	double ModelViewProjection(AuMatrix4x4& mvp) const;

	virtual void mouseDoubleClickEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void paintEvent(QPaintEvent* event);
	virtual void showEvent(QShowEvent* event);
	void ShowSelected(QPainter& p, const QString& itemName, const QRect& itemRect,
		const QMultiMap<quint8, COMMODITYID>& atmRes, const QMultiMap<quint8, COMMODITYID>& hydRes,
		const QMultiMap<quint8, COMMODITYID>& geoRes, const QMultiMap<quint8, COMMODITYID>& bioRes);
	virtual void wheelEvent(QWheelEvent* e);

protected slots:
	void Update();
};

#endif // AuSystemWidget_H
