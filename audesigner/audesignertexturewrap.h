//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignertexturewrap.h,v 1.2 2022/02/19 18:21:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDesignerTextureWrap_H
#define AuDesignerTextureWrap_H

#include "audockwidget.h"
#include "auglsl.h"
#include "auscface.h"
#include <QWidget>

class AuIconButton;
class AuView;
class QComboBox;
class QKeyEvent;
class QMouseEvent;
class QTimer;
class QWheelEvent;

class AuDesignerTextureWrapView : public QWidget
{
	enum INPUTMODE
	{
		INPUT_Select,
		INPUT_SelectBox,
		INPUT_Grab,
		INPUT_Rotate,
		INPUT_Stretch
	};

	Q_OBJECT;
	double								m_Angle;
	int									m_ButtonsPressed;
	int									m_Constrain;			// 0=none, 1=x, 2=y
	SURFACE_EFFECT						m_Effect;
	INPUTMODE							m_InputMode;
	bool								m_InputPerformed;		// true if input did something that should be undone if canceled
	QPointF								m_InputPos;
	int									m_Layer;				// 0=base, 1=effect
	QPointF								m_Pan;
	bool								m_PanSet;				// true after pan initialized
	QPoint								m_PressPos;
	bool								m_Repeat;
	QPointF								m_StretchPos;
	SC_TEXTURE_INDEX					m_TexIndex;
	QSize								m_TexSize;
	float								m_Scale;				// Texture pixels per screen pixel.
	quint8								m_UndoSeq;

public:	// Constructors
	AuDesignerTextureWrapView(QWidget* parent=Q_NULLPTR);

public:	// Methods
	void DesignerSelectionChanged();

	inline bool IsRepeating() const
	{
		return m_Repeat;
	}

	void SetEffect(SURFACE_EFFECT texI);

	void SetLayer(int layer);

	void SetRepeat(bool repeat);

	void SetScale(float scale);

	void SetTextureIndex(SC_TEXTURE_INDEX texI);

protected:
	void CancelInput();

	void ClearConstraints();

	void ConstrainX();

	void ConstrainY();

	void Grab();

	// Returns the base point for rotate and stretch operations.
	QPointF InputBase() const;

	bool IsSelectedVertexAt(const QPoint& pixel) const;

	bool IsVertexAt(const QPoint& pixel) const;

	virtual void keyPressEvent(QKeyEvent* e);

	virtual void keyReleaseEvent(QKeyEvent* e);

	// 0=base, 1=effect
	int Layer() const;

	virtual void mouseMoveEvent(QMouseEvent* e);

	virtual void mousePressEvent(QMouseEvent* e);

	virtual void mouseReleaseEvent(QMouseEvent* e);

	virtual void mouseDoubleClickEvent(QMouseEvent* e);

	virtual void paintEvent(QPaintEvent* event);

	// This works for both the base and the effect texture coords.
	QPointF PixelToTextureCoordinate(const QPoint& pixel) const;

	// This works for both the base and the effect texture coords.
	QPointF PixelToTextureCoordinate(const QPointF& pixel) const;

	void Rotate();

	void SelectAll();

	void SelectVerticesAt(const QPoint& pixel);

	void SelectVerticesInRect(const QRect& rect);

	void Stretch();

	// Returns screen pixel corresponding to texture coordinates in 'v'.
	// This works for both the base and the effect texture coords.
	QPointF TextureCoordToPixel(const QPointF& v) const;

	void ToggleVerticesAt(const QPoint& pixel);

	void ToggleVerticesInRect(const QRect& rect);

	// Returns vertex location that is nearest the pixel, within search tolerance.
	AuPoint VertexAt(const QPoint& pixel);

	virtual void wheelEvent(QWheelEvent* e);

signals:
	void TextureMove(QPointF vec, quint8 undoSeq);
	void TextureRotate(QPointF pivot, double angle, quint8 undoSeq);
	void TextureStretch(QPointF base, double scaleX, double scaleY, quint8 undoSeq);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDesignerTextureWrap : public QWidget
{
	Q_OBJECT;
	QTimer*								m_timer;				// UpdateContents timer

	// Widgets
	AuIconButton*								m_TexPalette;
	AuIconButton*								m_TexRepeat;
	QComboBox*									m_TextureEffects;
	QComboBox*									m_TextureLayer;
	QComboBox*									m_TexturePalette;
	AuDesignerTextureWrapView*					m_TextureWrapper;
	AuIconButton*								m_ProjectCylinder;
	AuIconButton*								m_ProjectPlane;
	AuIconButton*								m_ProjectSphere;
	AuIconButton*								m_Unwrap;

public:
	AuDesignerTextureWrap(QWidget* parent=0);
	virtual ~AuDesignerTextureWrap();

	void DesignerSelectionChanged();

protected:
	virtual void hideEvent(QHideEvent* event);
	void ListEffects(int curIdx);
	void ListTextures(AuView* view, int curIdx);
	void RestoreSettings();
	void SaveSettings();
	virtual void showEvent(QShowEvent* event);

public slots:
	void EnableControls();

protected slots:
	void TextureCylindrical();
	void TextureEffectChanged(int index);
	void TextureLayerIndexChanged(int index);
	void TextureMove(QPointF vec, quint8 undoSeq);
	void TexturePaletteIndexChanged(int index);
	void TextureProjected();
	void TextureRotate(QPointF pivot, double angle, quint8 undoSeq);
	void TextureSpherical();
	void TextureStretch(QPointF base, double scaleX, double scaleY, quint8 undoSeq);
	void ToggleRepeat();
	void Unwrap();
	void UpdateContents();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDesignerTextureWrapDock : public AuDockWidget
{
	Q_OBJECT;
public:
	AuDesignerTextureWrap*					m_Wrapper;

public:
	AuDesignerTextureWrapDock(QWidget* parent);

	void EnableControls()
	{
		m_Wrapper->EnableControls();
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas);

	virtual void UpdateTitleShortcut();

public slots:
	void DesignerSelectionChanged();
};

#endif // AuDesignerTextureWrap_H
