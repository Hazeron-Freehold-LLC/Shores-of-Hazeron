//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulocatormap.h,v 1.13 2022/04/15 17:00:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuLocatorMap_H
#define AuLocatorMap_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "aufogofwar.h"
#include "auglobal.h"
#include "auglobell.h"
#include "auid.h"
#include "aupoint.h"
#include <QWidget>

class QShowEvent;
class QSplitter;
class QTableWidget;
class QTimer;

class AUCLIENT_DLL AuLocatorMapWidget : public QWidget
{
	Q_OBJECT;
private:
	bool								m_CanBeamOver;
	bool								m_CanBeamVehicle;
	double								m_CenterLat;			// In map units
	double								m_CenterLong;			// In map units
	QPoint								m_CompassLoc;
	int									m_CompassSize;
	QRect								m_CompMagnifier;
	QCursor								m_Cursor;				// Crosshair cursor
	AuFogOfWarData						m_FogOfWarData;
	QImage								m_FogOfWarImg;			// Fog of war overlay
	bool								m_FollowPerson;			// true while map lat/long follows the person.
	bool								m_IsCrew;				// true if current person can issue crew orders
	bool								m_IsNewbieEmperor;		// True if current person is a newbie emperor.
	bool								m_IsRingSection;		// true if the map is showing a ring section
	int									m_LeftButtonMode;		// 0=none, 1=pan, 2=zoom, 3=resize compass
	bool								m_SunlightEnable;		// true to show sunlight overlay.
	QImage								m_SunlightMap;			// Sunlight overlay
	AuElapsedTimer						m_SunlightTime;			// For update to the sunlight overlay.
	QRect								m_Magnifier;
	float								m_MagnifierScale;		// Scale when magnifier started being dragged.
	QPoint								m_MouseDownPt;
	AuId								m_PlanetId;				// Id of planet we are at if it has a surface image.
	QImage								m_PlanetImage;			// Surface image of m_PlanetId
	int									m_PlanetLatDivs;		// Non-zero when at a planet with a surface image.
	int									m_PlanetLonDivs;		// Non-zero when at a planet with a surface image.
	QString								m_PlanetName;
	float								m_Scale;
	AuId								m_SystemId;				// solar system id that contains the planet
	QTimer*								m_timer;

public:
	AuLocatorMapWidget(QWidget* parent=0);
	virtual ~AuLocatorMapWidget();

	const QPoint& CompassLoc() const
	{
		return m_CompassLoc;
	}

	inline bool IsAtPlanet() const
	{
		return !m_PlanetId.IsNull();
	}

	AuGlobeLLF MapLL(const QPoint& mapPixel);

	float MapScale() const
	{
		return m_Scale;
	}

	void SetCompassLoc(const QPoint& compassLoc)
	{
		m_CompassLoc = compassLoc;
	}

	void SetCompassSize(int size)
	{
		if(size < 4)
			m_CompassSize = 4;
		else if(size > 500)
			m_CompassSize = 500;
		else
			m_CompassSize = size;
	}

	void SetMapScale(float mapScale)
	{
		m_Scale = mapScale;
	}

	virtual QSize sizeHint() const;

	void TeleportTo(const AuGlobeLLF& ll, bool rideAlso);

signals:
	void CompassSizeRestored(int size);

public slots:
	void BeamOver(const AuGlobeLLF& ll);
	void CompassSizeChanged(int size);
	void LookAt(const QPoint& pos);
	void LookAtLL(const AuGlobeLLF& ll);
	virtual void setVisible(bool visible);

protected:
	virtual void mouseDoubleClickEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void paintEvent(QPaintEvent* event);
	AuGlobeLLF PlanetLongLat(const QPoint& pos) const;
	QPoint PlanetLongLatPos(const AuGlobeLLF& ll) const;
	virtual void showEvent(QShowEvent* event);
	virtual void wheelEvent(QWheelEvent* e);

protected slots:
	void UpdateContents();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuLocatorMap : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	AuLocatorMapWidget*					m_Map;
	QTableWidget*						m_Places;
	QSplitter*							m_Splitter;

public:
	AuLocatorMap(QWidget* parent=0);
	virtual ~AuLocatorMap();

public slots:
	void PlaceSelectionChanged();
	void SplitterMoved(int pos, int index);

protected:
	virtual void showEvent(QShowEvent* e);

private:
	void AddPlace(const QString& iconName, const QString& placeName, const AuGlobeLLF& placeLL);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuLocatorWorldMapDock : public AuDockWidget
{
public:
	AuLocatorMap*						m_Content;

public:
	AuLocatorWorldMapDock(QWidget* parent) : AuDockWidget(":data/Map.png", tr("<b>World Map</b>"), tr("World Map"), parent)
	{
		m_Content = new AuLocatorMap(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/LocatorWorldMapAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/LocatorWorldMapAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuLocatorMap_H
