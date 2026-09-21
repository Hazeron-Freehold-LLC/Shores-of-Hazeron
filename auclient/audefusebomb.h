//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audefusebomb.h,v 1.8 2022/04/15 17:02:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDefuseBomb_H
#define AuDefuseBomb_H

#include "auclientlib.h"
#include "audialog.h"
#include "auelapsedtimer.h"
#include "auid.h"

class QMouseEvent;
class QPaintEvent;

class AUCLIENT_DLL AuDefuseBombWire
{
public:
	enum WIRECOLOR
	{
		Black,
		Green,
		Purple,
		Red,
	};

	enum WIREEFFECT
	{
		Defuse,
		Detonate,
		Nothing,
		Whizzer
	};

	enum WIREORIENT
	{
		Normal,
		FlipX,
		FlipY,
		FlipXY,
	};

public:
	WIRECOLOR							m_Color;
	WIREEFFECT							m_Effect;
	bool								m_IsCut;
	WIREORIENT							m_Orientation;

public:
	AuDefuseBombWire()
	{
		m_Color = Black;
		m_IsCut = false;
		m_Orientation = Normal;
	}
};

class AUCLIENT_DLL AuDefuseBombWidget : public QWidget
{
	Q_OBJECT;
private:
	qint8								m_ActiveWire;
	AuId								m_BombId;
	int									m_NumSec;
	int									m_NumVal;
	AuElapsedTimer						m_TimeUp;
	int									m_WhizzerMs;			// Whizzer ms elapsed.
	AuElapsedTimer						m_WhizzerTime;
	AuDefuseBombWire					m_Wires[4];				// One wire per row.

public:
	AuDefuseBombWidget(const AuId& bombId, QWidget* parent=0);
	virtual ~AuDefuseBombWidget();

	inline bool AnyWireCut() const
	{
		return m_Wires[0].m_IsCut ||
			m_Wires[1].m_IsCut ||
			m_Wires[2].m_IsCut ||
			m_Wires[3].m_IsCut;
	}

public slots:
	virtual void setVisible(bool visible);

protected:
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void paintEvent(QPaintEvent* pe);
	virtual QSize sizeHint() const;

signals:
	void WireCut(qint8 wireIndex);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuDefuseBomb : public AuDialog
{
	Q_OBJECT;
private:
	qint8								m_CutWireIndex;

	// Widgets
	AuDefuseBombWidget*					m_DefuseWidget;

public:
	AuDefuseBomb(const AuId& bombId, QWidget* parent=0);
	virtual ~AuDefuseBomb();

	inline qint8 CutWireIndex() const
	{
		return m_CutWireIndex;
	}

protected slots:
	void WireCut(qint8 wireIndex);
};

#endif // AuDefuseBomb_H
