//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignertexturewrap.cpp,v 1.14 2022/04/11 20:17:09 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audesignertexturewrap.h"
//
#include "audesign.h"
#include "audesignerapp.h"
#include "audesignerframe.h"
#include "audesignerview.h"
#include "auscaddstate.h"
#include "auscaddstopdlg.h"
#include "auscchangestopdlg.h"
#include "auscdeletestate.h"
#include "auscdeletestopdlg.h"
#include "ausceffectunwrapdlg.h"
#include "auscparts.h"
#include "ausctextureunwrapdlg.h"
#include "autoolbutton.h"
#include "dadesigner.h"
#include "daperson.h"
#include "dgdata.h"
#include <QHeaderView>
#include <QIcon>
#include <QMenu>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>

#define d_SearchTol						4						// Pixels

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuDesignerTextureWrapDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleDesignTextureWrapUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

AuDesignerTextureWrapDock::AuDesignerTextureWrapDock(QWidget* parent) :
	AuDockWidget(":data/ScTexPalette.png", tr("<b>Texture Wrapper</b>"), tr("Texture Wrapper"), parent)
{
	m_Wrapper = new AuDesignerTextureWrap(this);
	setWidget(m_Wrapper);

	int dockSides = g_Au.Settings().readNumEntry("Dock/DesignerTextureWrapAreas", (int)allowedAreas());
	setAllowedAreas((Qt::DockWidgetAreas)dockSides);
}

void AuDesignerTextureWrapDock::SetAllowedAreas(Qt::DockWidgetAreas areas)
{
	setAllowedAreas(areas);
	g_Au.Settings().writeEntry("Dock/DesignerTextureWrapAreas", (int)allowedAreas());
}

void AuDesignerTextureWrapDock::DesignerSelectionChanged()
{
	m_Wrapper->DesignerSelectionChanged();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDesignerTextureWrapView::AuDesignerTextureWrapView(QWidget* parent) : QWidget(parent)
{
	m_Angle = 0;
	m_ButtonsPressed = 0;
	m_Constrain = 0;
	m_Effect = EFF_Count;
	m_InputMode = INPUT_Select;
	m_InputPerformed = false;
	m_Layer = 0;
	m_PanSet = false;
	m_Repeat = false;
	m_Scale = 1;
	m_TexIndex = SCTEXI_Count;
	m_UndoSeq = 0;

	setAttribute(Qt::WA_OpaquePaintEvent, true);
	//setCursor(Qt::CrossCursor);
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);
}

void AuDesignerTextureWrapView::keyPressEvent(QKeyEvent* e)
{
	QWidget::keyPressEvent(e);

	switch((Qt::Key)e->key())
	{
	case Qt::Key_Escape:
		CancelInput();
		break;

	case Qt::Key_A:
		SelectAll();
		e->accept();
		break;

	case Qt::Key_G:
		Grab();
		e->accept();
		break;

	case Qt::Key_R:
		Rotate();
		e->accept();
		break;

	case Qt::Key_S:
		Stretch();
		e->accept();
		break;

	case Qt::Key_X:
		ConstrainX();
		e->accept();
		break;

	case Qt::Key_Y:
		ConstrainY();
		e->accept();
		break;

	default:
		break;
	}
}

void AuDesignerTextureWrapView::keyReleaseEvent(QKeyEvent* e)
{
	QWidget::keyReleaseEvent(e);
}

void AuDesignerTextureWrapView::mouseMoveEvent(QMouseEvent* e)
{
	QWidget::mouseMoveEvent(e);
	if(m_ButtonsPressed & Qt::LeftButton)
	{
		if(m_InputMode == INPUT_Select)
		{
			// Calculate the drag vector in texture space.
			QPointF drag = PixelToTextureCoordinate(e->pos()) - PixelToTextureCoordinate(m_PressPos);
			m_PressPos = e->pos();

			// Move the texture coordinates on the selected faces.
			emit TextureMove(drag, m_UndoSeq);

			update();
		}
	}
	else if(m_ButtonsPressed & Qt::RightButton)
	{
		// Pan the display.
		QPointF panVec(e->pos() - m_PressPos);
		m_Pan += panVec;
		m_InputPos += panVec;
		m_StretchPos += panVec;
		m_PressPos = e->pos();
		update();
	}
	else
	{
		switch(m_InputMode)
		{
		case INPUT_Select:
			break;

		case INPUT_SelectBox:
			// Selecting a box with no buttons pushed? Something is out of sync.
			m_InputMode = INPUT_Select;
			break;

		case INPUT_Grab:
			{
				// Calculate the drag vector in texture space.
				QPointF drag = PixelToTextureCoordinate(e->pos()) - PixelToTextureCoordinate(m_InputPos);
				m_InputPos = e->pos();

				// Apply constraints.
				if(m_Constrain == 1)
					drag.setY(0);
				else if(m_Constrain == 2)
					drag.setX(0);

				// Move the texture coordinates on the selected faces.
				emit TextureMove(drag, m_UndoSeq);
				m_InputPerformed = true;						// This can be undone if they hit Esc.
				update();
			}
			break;

		case INPUT_Rotate:
			{
				// Calculate the drag vector to determine the angle.
				AuPoint drag(e->pos() - m_InputPos);
				double angle = drag.ZRotation();
				double angleChange = m_Angle - angle;			// Sign reversed due to the Y reversal.
				m_Angle = angle;

				// Input position is the pivot point around which everything rotates.
				QPointF pivot = PixelToTextureCoordinate(m_InputPos);

				// Rotate the texture coordinates on the selected faces.
				emit TextureRotate(pivot, angleChange, m_UndoSeq);
				m_InputPerformed = true;						// This can be undone if they hit Esc.
				update();
			}
			break;

		case INPUT_Stretch:
			{
				// Calculate the drag vector to determine the angle.
				AuPoint oldVec(m_StretchPos - m_InputPos);
				AuPoint nuVec(e->pos() - m_InputPos);
				m_StretchPos = e->pos();

				double oldLen = oldVec.Length_2D();
				if(oldLen == 0)
					break;

				// Calculate scaling based on distance moved.
				double nuLen = nuVec.Length_2D();
				double scale = nuLen / oldLen;
				double scaleX = scale;
				double scaleY = scale;

				// Apply constraints.
				if(m_Constrain == 1)
					scaleY = 1;
				else if(m_Constrain == 2)
					scaleX = 1;

				// Input position is the base point from which everything stretches.
				QPointF base = PixelToTextureCoordinate(m_InputPos);

				// Stretch the texture coordinates on the selected faces.
				emit TextureStretch(base, scaleX, scaleY, m_UndoSeq);
				m_InputPerformed = true;						// This can be undone if they hit Esc.
				update();
			}
			break;
		}
	}
}

void AuDesignerTextureWrapView::mouseDoubleClickEvent(QMouseEvent* e)
{
	QWidget::mouseDoubleClickEvent(e);

	m_Scale = 1;
	m_PanSet = false;
	update();
}

void AuDesignerTextureWrapView::mousePressEvent(QMouseEvent* e)
{
	QWidget::mousePressEvent(e);
	m_ButtonsPressed = e->buttons();
	m_PressPos = e->pos();
	if(m_ButtonsPressed & Qt::LeftButton)
	{
		if(m_InputMode == INPUT_Select)
		{
			// In select mode.
			if(IsVertexAt(e->pos()))
			{
				// Select or deselect a vertex.
				if(e->modifiers() & Qt::ControlModifier)
				{
					ToggleVerticesAt(e->pos());
					update();
				}
				else
				{
					if(!IsSelectedVertexAt(e->pos()))
					{
						auDesignerApp->DesignerFrame()->DesignerView()->DesignerDeselectVertices();
						SelectVerticesAt(e->pos());
					}
				}
			}
			else
			{
				if(!(e->modifiers() & Qt::ControlModifier))
					auDesignerApp->DesignerFrame()->DesignerView()->DesignerDeselectVertices();
				m_InputMode = INPUT_SelectBox;	// Start a box selection.
			}
		}
		else
			m_InputMode = INPUT_Select;		// Left click returns to select mode from other input modes.
	}
}

void AuDesignerTextureWrapView::mouseReleaseEvent(QMouseEvent* e)
{
	QWidget::mouseReleaseEvent(e);

	if(m_ButtonsPressed)
	{
		if(m_InputMode == INPUT_SelectBox)
		{
			m_InputMode = INPUT_Select;

			QPoint curPos = QWidget::mapFromGlobal(QCursor::pos());
			if(e->modifiers() & Qt::ControlModifier)
				ToggleVerticesInRect(QRect(m_PressPos, curPos));
			else
				SelectVerticesInRect(QRect(m_PressPos, curPos));
		}

		m_ButtonsPressed = 0;
		update();
	}
}

void AuDesignerTextureWrapView::wheelEvent(QWheelEvent* e)
{
	QWidget::wheelEvent(e);

	// Calculate current mouse pos in the texture image.
	float x = ((float)e->pos().x() - m_Pan.x() + m_TexSize.width()/2) / m_Scale;
	float y = ((float)e->pos().y() - m_Pan.y() + m_TexSize.height()/2) / m_Scale;

#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
	int zDelta = e->angleDelta().y();
#else
	int zDelta = e->delta();
#endif
	if(zDelta > 0)
	{
		bool bfast = false;
		if(e->modifiers() & Qt::ControlModifier)
			bfast = true;

		double dist = 1.1;
		if(bfast)
			dist *= 1.5;

		m_Scale *= dist;
	}
	else if(zDelta < 0)
	{
		bool bfast = false;
		if(e->modifiers() & Qt::ControlModifier)
			bfast = true;

		double dist = 1.1;
		if(bfast)
			dist *= 1.5;

		m_Scale /= dist;
	}

	// Calculate change in pan so pixel under mouse doesn't move.
	float px = (float)e->pos().x() + m_TexSize.width()/2 - x * m_Scale;
	float py = (float)e->pos().y() + m_TexSize.height()/2 - y * m_Scale;
	m_Pan = QPointF(px, py);;

	update();
}

void AuDesignerTextureWrapView::SetLayer(int layer)
{
	if(layer < 0 || layer > 1)
		return;
	m_Layer = layer;
	update();
}

int AuDesignerTextureWrapView::Layer() const
{
	return m_Layer;
}

void AuDesignerTextureWrapView::paintEvent(QPaintEvent* event)
{
	//qDebug() << "AuDesignerTextureWrapView::paintEvent";
	Q_UNUSED(event);
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	if(!pFrame)
		return;
	AuDesignerView* view = pFrame->DesignerView();
	if(!view)
		return;
	if(width() == 0 || height() == 0)
		return;

	if(!m_PanSet)
	{
		m_Pan.setX(width()/2);
		m_Pan.setY(height()/2);
		m_PanSet = true;
	}

	QPainter p(this);
	p.fillRect(rect(), QColor(0,0,0));

	QImage imgTile;
	int layer = Layer();
	if(layer == 0)
	{
		// Base layer
		if(m_TexIndex < SCTEXI_Count)
		{
			AuDesignTex texturePalette[SCTEXI_Count];
			COMMODITYID hullModule = CID_Invalid;
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				DaDesigner* pde = person->FindParentDesignerInEnv();
				if(pde)
				{
					pde->TexturePalette(texturePalette);
					const AuDesign* design = pde->LockDesignForRead();
					if(design)
						hullModule = design->HullModule();
					pde->UnlockDesign();
				}
			}
			view->UnlockSceneAtom();
			if(!texturePalette[m_TexIndex].Image().isNull())
				imgTile = texturePalette[m_TexIndex].Image();
			else if(m_TexIndex == SCTEXI_Material)
			{
				switch(hullModule)
				{
				default:
				case CID_WoodHullModule:
					imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullWood()));
					break;
				case CID_LumberHullModule:
					imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullLumber()));
					break;
				case CID_StoneHullModule:
					imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullStone()));
					break;
				case CID_MetalHullModule:
					imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullMetal()));
					break;
				case CID_MagmiumHullModule:
					imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullMagmium()));
					break;
				case CID_VulciumHullModule:
					imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullVulcium()));
					break;
				case CID_AdamantineHullModule:
					imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullAdamantine()));
					break;
				}
			}
			else
				imgTile.load(AuTextureLibItem::LibraryTextureFile(texturePalette[m_TexIndex].TexLibId()));
		}
	}
	else if(layer == 1)
	{
		// Effect layer.
		if(m_Effect != EFF_None && m_Effect < EFF_Count)
			imgTile.load(AuTextureLibItem::EffectTextureItem(m_Effect).FileName());
	}
	else
		return;

	m_TexSize = imgTile.size();
	if(!imgTile.isNull())
	{
		QPointF targLoc(m_Pan.x() - imgTile.width()/2, m_Pan.y() - imgTile.height()/2);
		QSizeF targSize(m_TexSize.width() * m_Scale, m_TexSize.height() * m_Scale);
		QRectF targRect(targLoc, targSize);
		p.setRenderHint(QPainter::SmoothPixmapTransform, true);
		p.drawImage(targRect, imgTile);

		// Frame it with repeats.
		if(m_Repeat)
		{
			QRectF brect = targRect.translated(-targSize.width(), -targSize.height());
			p.drawImage(brect, imgTile);
			brect = targRect.translated(0, -targSize.height());
			p.drawImage(brect, imgTile);
			brect = targRect.translated(targSize.width(), -targSize.height());
			p.drawImage(brect, imgTile);
			brect = targRect.translated(-targSize.width(), 0);
			p.drawImage(brect, imgTile);
			brect = targRect.translated(targSize.width(), 0);
			p.drawImage(brect, imgTile);
			brect = targRect.translated(-targSize.width(), targSize.height());
			p.drawImage(brect, imgTile);
			brect = targRect.translated(0, targSize.height());
			p.drawImage(brect, imgTile);
			brect = targRect.translated(targSize.width(), targSize.height());
			p.drawImage(brect, imgTile);
		}

		// Paint edges of faces on top of texture, at texture coordinates.
		QColor curCol(g_Au.DesignerLineColor());
		p.setPen(curCol);

		DaPerson* person;
		view->LockSceneAtom(&person);
		if(person)
		{
			DaDesigner* pde = person->FindParentDesignerInEnv();
			if(pde)
			{
				const AuScParts* parts = pde->LockPartsForRead();
				if(parts)
				{
					AuScSelection sel = view->DesignerPartObjectSelection();
					for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
					{
						AuScParts::const_iterator pi = parts->find(si.key());
						if(pi == parts->cend())
							continue;

						// Part is selected. We only show selected faces.
						AuScFaceList faces;
						(*pi)->Faces(faces);

						int findex = 0;
						int vindex = 0;
						for(AuScFaceList::const_iterator fi = faces.cbegin(); fi != faces.cend();
								findex++, vindex += (*fi)->VertexCount(), fi++)
						{
							if(!(*si).m_Faces.contains(findex))
								continue;	// Skip unselected face.

							// Show selected face.
							AuSegmentArray edges;
							(*fi)->EdgeSegments(edges);
							for(AuSegmentArray::const_iterator ei = edges.cbegin(); ei != edges.cend(); ei++)
							{
								int vsIdx = (*fi)->VertexAt((*ei).m_fLoc1);
								if(vsIdx < 0)
									continue;
								int veIdx = (*fi)->VertexAt((*ei).m_fLoc2);
								if(veIdx < 0)
									continue;

								// Calculate pixel locations of the end point texture coords.
								AuScPoint vs = (*fi)->Vertex(vsIdx);
								AuScPoint ve = (*fi)->Vertex(veIdx);
								QPoint pxs;
								QPoint pxe;
								if(layer == 0)
								{
									pxs = TextureCoordToPixel(vs.TexCoord()).toPoint();
									pxe = TextureCoordToPixel(ve.TexCoord()).toPoint();
								}
								else
								{
									pxs = TextureCoordToPixel(vs.EffectCoord()).toPoint();
									pxe = TextureCoordToPixel(ve.EffectCoord()).toPoint();
								}

								// Color according to selection.
								bool sel1 = (*si).m_Vertices.contains(vindex + vsIdx);
								bool sel2 = (*si).m_Vertices.contains(vindex + veIdx);
								if(sel1 && sel2)
								{
									// Both ends selected.
									if(curCol != g_Au.DesignerSelColor())
									{
										curCol = g_Au.DesignerSelColor();
										p.setPen(curCol);
									}

									// Draw a pixel line in the texture.
									p.drawLine(pxs, pxe);
									p.drawRect(pxs.x()-1, pxs.y()-1, 2, 2);
									p.drawRect(pxe.x()-1, pxe.y()-1, 2, 2);
								}
								else if(sel1)
								{
									// End 1 selected.
									QPoint pxm = (pxs + pxe)/2;
									if(curCol == g_Au.DesignerSelColor())
									{
										// Draw selected end first.
										p.drawLine(pxs, pxm);
										p.drawRect(pxs.x()-1, pxs.y()-1, 2, 2);

										curCol = g_Au.DesignerLineColor();
										p.setPen(curCol);

										p.drawLine(pxm, pxe);
										p.drawRect(pxe.x()-1, pxe.y()-1, 2, 2);
									}
									else
									{
										// Draw unselected end first.
										p.drawLine(pxm, pxe);
										p.drawRect(pxe.x()-1, pxe.y()-1, 2, 2);

										curCol = g_Au.DesignerSelColor();
										p.setPen(curCol);

										p.drawLine(pxs, pxm);
										p.drawRect(pxs.x()-1, pxs.y()-1, 2, 2);
									}

								}
								else if(sel2)
								{
									// End 2 selected.
									QPoint pxm = (pxs + pxe)/2;
									if(curCol == g_Au.DesignerSelColor())
									{
										// Draw selected end first.
										p.drawLine(pxm, pxe);
										p.drawRect(pxe.x()-1, pxe.y()-1, 2, 2);

										curCol = g_Au.DesignerLineColor();
										p.setPen(curCol);

										p.drawLine(pxs, pxm);
										p.drawRect(pxs.x()-1, pxs.y()-1, 2, 2);
									}
									else
									{
										// Draw unselected end first.
										p.drawLine(pxs, pxm);
										p.drawRect(pxs.x()-1, pxs.y()-1, 2, 2);

										curCol = g_Au.DesignerSelColor();
										p.setPen(curCol);

										p.drawLine(pxm, pxe);
										p.drawRect(pxe.x()-1, pxe.y()-1, 2, 2);
									}
								}
								else
								{
									// Neither end selected.
									if(curCol != g_Au.DesignerLineColor())
									{
										curCol = g_Au.DesignerLineColor();
										p.setPen(curCol);
									}

									// Draw a pixel line in the texture.
									p.drawLine(pxs, pxe);
									p.drawRect(pxs.x()-1, pxs.y()-1, 2, 2);
									p.drawRect(pxe.x()-1, pxe.y()-1, 2, 2);
								}
							}
						}
					}
				}
				pde->UnlockParts();
			}
		}
		view->UnlockSceneAtom();
	}

	if(m_InputMode == INPUT_SelectBox)
	{
		// Overlay the box selection rectangle onto everything.
		QPoint curPos = QWidget::mapFromGlobal(QCursor::pos());
		p.setPen(g_Au.DesignerGridZColor());
		p.drawRect(QRect(m_PressPos, curPos));
	}
	//qDebug() << "AuDesignerTextureWrapView::paintEvent done";
}

QPointF AuDesignerTextureWrapView::TextureCoordToPixel(const QPointF& v) const
{
	// Calculate pixel location in texture coordinates.
	float px = v.x() * m_TexSize.width() * m_Scale - m_TexSize.width()/2 + m_Pan.x();
	float py = -v.y() * m_TexSize.height() * m_Scale - m_TexSize.height()/2 + m_Pan.y();
	return QPointF(px,py);
}

QPointF AuDesignerTextureWrapView::PixelToTextureCoordinate(const QPoint& pixel) const
{
	// Calculate texture coordate at the pixel location.
	float tx = (pixel.x() + m_TexSize.width()/2 - m_Pan.x()) / (m_TexSize.width() * m_Scale);
	float ty = -(pixel.y() + m_TexSize.height()/2 - m_Pan.y()) / (m_TexSize.height() * m_Scale);

	/*
	// Verify result.
	QPointF pf = TextureCoordToPixel(QPointF(tx,ty));
	QPoint p((int)pf.x(), (int)pf.y());
	if(p != pixel)
		qDebug() << "AuDesignerTextureWrapView::PixelToTextureCoordinate pixel" << pixel << "p" << p;
		*/
	return QPointF(tx,ty);
}

QPointF AuDesignerTextureWrapView::PixelToTextureCoordinate(const QPointF& pixel) const
{
	// Calculate texture coordate at the pixel location.
	float tx = (pixel.x() + m_TexSize.width()/2 - m_Pan.x()) / (m_TexSize.width() * m_Scale);
	float ty = -(pixel.y() + m_TexSize.height()/2 - m_Pan.y()) / (m_TexSize.height() * m_Scale);

	/*
	// Verify result.
	QPointF p = TextureCoordToPixel(QPointF(tx,ty));
	if(p != pixel)
		qDebug() << "AuDesignerTextureWrapView::PixelToTextureCoordinate pixel" << pixel << "p" << p;
		*/
	return QPointF(tx,ty);
}

void AuDesignerTextureWrapView::SelectVerticesAt(const QPoint& pixel)
{
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();
	if(!view)
		return;

	int layer = Layer();
	if(layer < 0 || layer > 1)
		return;

	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			const AuScParts* parts = pde->LockPartsForRead();
			if(parts)
			{
				AuScSelection sel = view->DesignerPartObjectSelection();
				for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
				{
					AuScParts::const_iterator pi = parts->find(si.key());
					if(pi == parts->cend())
						continue;

					// Part is selected. We only show selected faces.
					AuScFaceList faces;
					(*pi)->Faces(faces);

					int findex = 0;
					int vindex = 0;
					for(AuScFaceList::const_iterator fi = faces.cbegin(); fi != faces.cend();
							findex++, vindex += (*fi)->VertexCount(), fi++)
					{
						if(!(*si).m_Faces.contains(findex))
							continue;	// Skip unselected face.

						// Check vertices of selected face.
						AuSegmentArray edges;
						(*fi)->EdgeSegments(edges);
						for(AuSegmentArray::const_iterator ei = edges.cbegin(); ei != edges.cend(); ei++)
						{
							int vsIdx = (*fi)->VertexAt((*ei).m_fLoc1);
							if(vsIdx < 0)
								continue;
							int veIdx = (*fi)->VertexAt((*ei).m_fLoc2);
							if(veIdx < 0)
								continue;

							// Calculate pixel locations of the end point texture coords.
							AuScPoint vs = (*fi)->Vertex(vsIdx);
							AuScPoint ve = (*fi)->Vertex(veIdx);
							QPoint pxs;
							QPoint pxe;
							if(layer == 0)
							{
								pxs = TextureCoordToPixel(vs.TexCoord()).toPoint();
								pxe = TextureCoordToPixel(ve.TexCoord()).toPoint();
							}
							else
							{
								pxs = TextureCoordToPixel(vs.EffectCoord()).toPoint();
								pxe = TextureCoordToPixel(ve.EffectCoord()).toPoint();
							}

							QPoint svec = pxs - pixel;
							if(abs(svec.x()) <= d_SearchTol && abs(svec.y()) <= d_SearchTol)
								view->DesignerSelectTextureVertex(pde, si.key(), vindex + vsIdx);

							QPoint evec = pxe - pixel;
							if(abs(evec.x()) <= d_SearchTol && abs(evec.y()) <= d_SearchTol)
								view->DesignerSelectTextureVertex(pde, si.key(), vindex + veIdx);
						}
					}
				}
			}
			pde->UnlockParts();
		}
	}
	view->UnlockSceneAtom();
}

void AuDesignerTextureWrapView::ToggleVerticesAt(const QPoint& pixel)
{
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();
	if(!view)
		return;

	int layer = Layer();
	if(layer < 0 || layer > 1)
		return;

	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			const AuScParts* parts = pde->LockPartsForRead();
			if(parts)
			{
				QMultiHash<quint16, quint32> selVerts;
				QMultiHash<quint16, quint32> deselVerts;

				AuScSelection sel = view->DesignerPartObjectSelection();
				for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
				{
					AuScParts::const_iterator pi = parts->find(si.key());
					if(pi == parts->cend())
						continue;

					// Part is selected. We only show selected faces.
					AuScFaceList faces;
					(*pi)->Faces(faces);

					int findex = 0;
					int vindex = 0;
					for(AuScFaceList::const_iterator fi = faces.cbegin(); fi != faces.cend();
							findex++, vindex += (*fi)->VertexCount(), fi++)
					{
						if(!(*si).m_Faces.contains(findex))
							continue;	// Skip unselected face.

						// Check vertices of selected face.
						AuSegmentArray edges;
						(*fi)->EdgeSegments(edges);
						for(AuSegmentArray::const_iterator ei = edges.cbegin(); ei != edges.cend(); ei++)
						{
							int vsIdx = (*fi)->VertexAt((*ei).m_fLoc1);
							if(vsIdx < 0)
								continue;
							int veIdx = (*fi)->VertexAt((*ei).m_fLoc2);
							if(veIdx < 0)
								continue;

							// Calculate pixel locations of the end point texture coords.
							AuScPoint vs = (*fi)->Vertex(vsIdx);
							AuScPoint ve = (*fi)->Vertex(veIdx);
							QPoint pxs;
							QPoint pxe;
							if(layer == 0)
							{
								pxs = TextureCoordToPixel(vs.TexCoord()).toPoint();
								pxe = TextureCoordToPixel(ve.TexCoord()).toPoint();
							}
							else
							{
								pxs = TextureCoordToPixel(vs.EffectCoord()).toPoint();
								pxe = TextureCoordToPixel(ve.EffectCoord()).toPoint();
							}

							QPoint svec = pxs - pixel;
							if(abs(svec.x()) <= d_SearchTol && abs(svec.y()) <= d_SearchTol)
							{
								if((*si).m_Vertices.contains(vindex + vsIdx))
									deselVerts.insert(si.key(), vindex + vsIdx);
								else
									selVerts.insert(si.key(), vindex + vsIdx);
							}

							QPoint evec = pxe - pixel;
							if(abs(evec.x()) <= d_SearchTol && abs(evec.y()) <= d_SearchTol)
							{
								if((*si).m_Vertices.contains(vindex + veIdx))
									deselVerts.insert(si.key(), vindex + veIdx);
								else
									selVerts.insert(si.key(), vindex + veIdx);
							}
						}
					}
				}

				for(QMultiHash<quint16, quint32>::const_iterator svi = deselVerts.cbegin(); svi != deselVerts.cend(); svi++)
					view->DesignerDeselectTextureVertex(pde, svi.key(), *svi);

				for(QMultiHash<quint16, quint32>::const_iterator svi = selVerts.cbegin(); svi != selVerts.cend(); svi++)
					view->DesignerSelectTextureVertex(pde, svi.key(), *svi);
			}
			pde->UnlockParts();
		}
	}
	view->UnlockSceneAtom();
}

void AuDesignerTextureWrapView::SelectVerticesInRect(const QRect& rect)
{
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();
	if(!view)
		return;

	int layer = Layer();
	if(layer < 0 || layer > 1)
		return;

	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			const AuScParts* parts = pde->LockPartsForRead();
			if(parts)
			{
				AuScSelection sel = view->DesignerPartObjectSelection();
				for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
				{
					AuScParts::const_iterator pi = parts->find(si.key());
					if(pi == parts->cend())
						continue;

					// Part is selected. We only show selected faces.
					AuScFaceList faces;
					(*pi)->Faces(faces);

					int findex = 0;
					int vindex = 0;
					for(AuScFaceList::const_iterator fi = faces.cbegin(); fi != faces.cend();
							findex++, vindex += (*fi)->VertexCount(), fi++)
					{
						if(!(*si).m_Faces.contains(findex))
							continue;	// Skip unselected face.

						// Check vertices of selected face.
						AuSegmentArray edges;
						(*fi)->EdgeSegments(edges);
						for(AuSegmentArray::const_iterator ei = edges.cbegin(); ei != edges.cend(); ei++)
						{
							int vsIdx = (*fi)->VertexAt((*ei).m_fLoc1);
							if(vsIdx < 0)
								continue;
							int veIdx = (*fi)->VertexAt((*ei).m_fLoc2);
							if(veIdx < 0)
								continue;

							// Calculate pixel locations of the end point texture coords.
							AuScPoint vs = (*fi)->Vertex(vsIdx);
							AuScPoint ve = (*fi)->Vertex(veIdx);
							QPoint pxs;
							QPoint pxe;
							if(layer == 0)
							{
								pxs = TextureCoordToPixel(vs.TexCoord()).toPoint();
								pxe = TextureCoordToPixel(ve.TexCoord()).toPoint();
							}
							else
							{
								pxs = TextureCoordToPixel(vs.EffectCoord()).toPoint();
								pxe = TextureCoordToPixel(ve.EffectCoord()).toPoint();
							}

							if(rect.contains(pxs))
								view->DesignerSelectTextureVertex(pde, si.key(), vindex + vsIdx);

							if(rect.contains(pxe))
								view->DesignerSelectTextureVertex(pde, si.key(), vindex + veIdx);
						}
					}
				}
			}
			pde->UnlockParts();
		}
	}
	view->UnlockSceneAtom();
}

void AuDesignerTextureWrapView::ToggleVerticesInRect(const QRect& rect)
{
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();
	if(!view)
		return;

	int layer = Layer();
	if(layer < 0 || layer > 1)
		return;

	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			const AuScParts* parts = pde->LockPartsForRead();
			if(parts)
			{
				QMultiHash<quint16, quint32> selVerts;
				QMultiHash<quint16, quint32> deselVerts;

				AuScSelection sel = view->DesignerPartObjectSelection();
				for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
				{
					AuScParts::const_iterator pi = parts->find(si.key());
					if(pi == parts->cend())
						continue;

					// Part is selected. We only show selected faces.
					AuScFaceList faces;
					(*pi)->Faces(faces);

					int findex = 0;
					int vindex = 0;
					for(AuScFaceList::const_iterator fi = faces.cbegin(); fi != faces.cend();
							findex++, vindex += (*fi)->VertexCount(), fi++)
					{
						if(!(*si).m_Faces.contains(findex))
							continue;	// Skip unselected face.

						// Check vertices of selected face.
						AuSegmentArray edges;
						(*fi)->EdgeSegments(edges);
						for(AuSegmentArray::const_iterator ei = edges.cbegin(); ei != edges.cend(); ei++)
						{
							int vsIdx = (*fi)->VertexAt((*ei).m_fLoc1);
							if(vsIdx < 0)
								continue;
							int veIdx = (*fi)->VertexAt((*ei).m_fLoc2);
							if(veIdx < 0)
								continue;

							// Calculate pixel locations of the end point texture coords.
							AuScPoint vs = (*fi)->Vertex(vsIdx);
							AuScPoint ve = (*fi)->Vertex(veIdx);
							QPoint pxs;
							QPoint pxe;
							if(layer == 0)
							{
								pxs = TextureCoordToPixel(vs.TexCoord()).toPoint();
								pxe = TextureCoordToPixel(ve.TexCoord()).toPoint();
							}
							else
							{
								pxs = TextureCoordToPixel(vs.EffectCoord()).toPoint();
								pxe = TextureCoordToPixel(ve.EffectCoord()).toPoint();
							}

							if(rect.contains(pxs))
							{
								if((*si).m_Vertices.contains(vindex + vsIdx))
									deselVerts.insert(si.key(), vindex + vsIdx);
								else
									selVerts.insert(si.key(), vindex + vsIdx);
							}

							if(rect.contains(pxe))
							{
								if((*si).m_Vertices.contains(vindex + veIdx))
									deselVerts.insert(si.key(), vindex + veIdx);
								else
									selVerts.insert(si.key(), vindex + veIdx);
							}
						}
					}
				}

				for(QMultiHash<quint16, quint32>::const_iterator svi = deselVerts.cbegin(); svi != deselVerts.cend(); svi++)
					view->DesignerDeselectTextureVertex(pde, svi.key(), *svi);

				for(QMultiHash<quint16, quint32>::const_iterator svi = selVerts.cbegin(); svi != selVerts.cend(); svi++)
					view->DesignerSelectTextureVertex(pde, svi.key(), *svi);
			}
			pde->UnlockParts();
		}
	}
	view->UnlockSceneAtom();
}

bool AuDesignerTextureWrapView::IsSelectedVertexAt(const QPoint& pixel) const
{
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();
	if(!view)
		return false;

	int layer = Layer();
	if(layer < 0 || layer > 1)
		return false;

	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			const AuScParts* parts = pde->LockPartsForRead();
			if(parts)
			{
				AuScSelection sel = view->DesignerPartObjectSelection();
				for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
				{
					AuScParts::const_iterator pi = parts->find(si.key());
					if(pi == parts->cend())
						continue;

					// Part is selected. We only show selected faces.
					AuScFaceList faces;
					(*pi)->Faces(faces);

					int findex = 0;
					int vindex = 0;
					for(AuScFaceList::const_iterator fi = faces.cbegin(); fi != faces.cend();
							findex++, vindex += (*fi)->VertexCount(), fi++)
					{
						if(!(*si).m_Faces.contains(findex))
							continue;	// Skip unselected face.

						// Check vertices of selected face.
						AuSegmentArray edges;
						(*fi)->EdgeSegments(edges);
						for(AuSegmentArray::const_iterator ei = edges.cbegin(); ei != edges.cend(); ei++)
						{
							int vsIdx = (*fi)->VertexAt((*ei).m_fLoc1);
							if(vsIdx < 0)
								continue;
							int veIdx = (*fi)->VertexAt((*ei).m_fLoc2);
							if(veIdx < 0)
								continue;

							if((*si).m_Vertices.contains(vindex + vsIdx))
							{
								// Calculate pixel location of the texture coords.
								AuScPoint vs = (*fi)->Vertex(vsIdx);
								QPoint pxs;
								if(layer == 0)
									pxs = TextureCoordToPixel(vs.TexCoord()).toPoint();
								else
									pxs = TextureCoordToPixel(vs.EffectCoord()).toPoint();
								QPoint vec = pxs - pixel;
								if(abs(vec.x()) <= d_SearchTol && abs(vec.y()) <= d_SearchTol)
								{
									pde->UnlockParts();
									view->UnlockSceneAtom();
									return true;
								}
							}

							if((*si).m_Vertices.contains(vindex + veIdx))
							{
								// Calculate pixel location of the texture coords.
								AuScPoint ve = (*fi)->Vertex(veIdx);
								QPoint pxe;
								if(layer == 0)
									pxe = TextureCoordToPixel(ve.TexCoord()).toPoint();
								else
									pxe = TextureCoordToPixel(ve.EffectCoord()).toPoint();
								QPoint vec = pxe - pixel;
								if(abs(vec.x()) <= d_SearchTol && abs(vec.y()) <= d_SearchTol)
								{
									pde->UnlockParts();
									view->UnlockSceneAtom();
									return true;
								}
							}
						}
					}
				}
			}
			pde->UnlockParts();
		}
	}
	view->UnlockSceneAtom();
	return false;
}

bool AuDesignerTextureWrapView::IsVertexAt(const QPoint& pixel) const
{
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();
	if(!view)
		return false;

	int layer = Layer();
	if(layer < 0 || layer > 1)
		return false;

	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			const AuScParts* parts = pde->LockPartsForRead();
			if(parts)
			{
				AuScSelection sel = view->DesignerPartObjectSelection();
				for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
				{
					AuScParts::const_iterator pi = parts->find(si.key());
					if(pi == parts->cend())
						continue;

					// Part is selected. We only show selected faces.
					AuScFaceList faces;
					(*pi)->Faces(faces);

					int findex = 0;
					int vindex = 0;
					for(AuScFaceList::const_iterator fi = faces.cbegin(); fi != faces.cend();
							findex++, vindex += (*fi)->VertexCount(), fi++)
					{
						if(!(*si).m_Faces.contains(findex))
							continue;	// Skip unselected face.

						// Check vertices of selected face.
						AuSegmentArray edges;
						(*fi)->EdgeSegments(edges);
						for(AuSegmentArray::const_iterator ei = edges.cbegin(); ei != edges.cend(); ei++)
						{
							int vsIdx = (*fi)->VertexAt((*ei).m_fLoc1);
							if(vsIdx < 0)
								continue;
							int veIdx = (*fi)->VertexAt((*ei).m_fLoc2);
							if(veIdx < 0)
								continue;

							// Calculate pixel location of the texture coords.
							AuScPoint vs = (*fi)->Vertex(vsIdx);
							QPoint pxs;
							if(layer == 0)
								pxs = TextureCoordToPixel(vs.TexCoord()).toPoint();
							else
								pxs = TextureCoordToPixel(vs.EffectCoord()).toPoint();
							QPoint svec = pxs - pixel;
							if(abs(svec.x()) <= d_SearchTol && abs(svec.y()) <= d_SearchTol)
							{
								pde->UnlockParts();
								view->UnlockSceneAtom();
								return true;
							}

							// Calculate pixel location of the texture coords.
							AuScPoint ve = (*fi)->Vertex(veIdx);
							QPoint pxe;
							if(layer == 0)
								pxe = TextureCoordToPixel(ve.TexCoord()).toPoint();
							else
								pxe = TextureCoordToPixel(ve.EffectCoord()).toPoint();
							QPoint evec = pxe - pixel;
							if(abs(evec.x()) <= d_SearchTol && abs(evec.y()) <= d_SearchTol)
							{
								pde->UnlockParts();
								view->UnlockSceneAtom();
								return true;
							}
						}
					}
				}
			}
			pde->UnlockParts();
		}
	}
	view->UnlockSceneAtom();
	return false;
}

void AuDesignerTextureWrapView::SetTextureIndex(SC_TEXTURE_INDEX texI)
{
	if(texI >= 0 && texI < SCTEXI_Count)
	{
		m_TexIndex = texI;
		m_Effect = EFF_Count;
	}
	else
		m_TexIndex = SCTEXI_Count;
	update();
}

void AuDesignerTextureWrapView::SetEffect(SURFACE_EFFECT texI)
{
	if(texI >= 0 && texI < EFF_Count)
	{
		m_TexIndex = SCTEXI_Count;
		m_Effect = texI;
	}
	else
		m_Effect = EFF_Count;
	update();
}

void AuDesignerTextureWrapView::SetScale(float scale)
{
	m_Scale = scale;
	update();
}

void AuDesignerTextureWrapView::SetRepeat(bool repeat)
{
	m_Repeat = repeat;
	update();
}

void AuDesignerTextureWrapView::DesignerSelectionChanged()
{
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();
	if(!view)
		return;

	int layer = Layer();
	if(layer < 0 || layer > 1)
		layer = 0;

	qint8 curIdx = -1;

	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			const AuDesign* design = pde->LockDesignForRead();
			if(design)
			{
				if(layer == 0)
					curIdx = design->TextureIndex(view->DesignerPartObjectSelection());	// Returns SCTEXI_Count if they differ.
				else
					curIdx = design->Effect(view->DesignerPartObjectSelection());	// Returns EFF_Count if they differ.
			}
			pde->UnlockDesign();
		}
	}
	view->UnlockSceneAtom();

	if(layer == 0)
		SetTextureIndex((SC_TEXTURE_INDEX)curIdx);
	else
		SetEffect((SURFACE_EFFECT)curIdx);
}

void AuDesignerTextureWrapView::SelectAll()
{
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();
	if(!view)
		return;

	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			const AuScParts* parts = pde->LockPartsForRead();
			if(parts)
			{
				AuScSelection sel = view->DesignerPartObjectSelection();
				for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
				{
					AuScParts::const_iterator pi = parts->find(si.key());
					if(pi == parts->cend())
						continue;

					// Part is selected. We only show selected faces.
					AuScFaceList faces;
					(*pi)->Faces(faces);

					int findex = 0;
					int vindex = 0;
					for(AuScFaceList::const_iterator fi = faces.cbegin(); fi != faces.cend();
							findex++, vindex += (*fi)->VertexCount(), fi++)
					{
						if(!(*si).m_Faces.contains(findex))
							continue;	// Skip unselected face.

						// Check vertices of selected face.
						AuSegmentArray edges;
						(*fi)->EdgeSegments(edges);
						for(AuSegmentArray::const_iterator ei = edges.cbegin(); ei != edges.cend(); ei++)
						{
							int vsIdx = (*fi)->VertexAt((*ei).m_fLoc1);
							if(vsIdx < 0)
								continue;
							int veIdx = (*fi)->VertexAt((*ei).m_fLoc2);
							if(veIdx < 0)
								continue;

							view->DesignerSelectTextureVertex(pde, si.key(), vindex + vsIdx);

							view->DesignerSelectTextureVertex(pde, si.key(), vindex + veIdx);
						}
					}
				}
			}
			pde->UnlockParts();
		}
	}
	view->UnlockSceneAtom();
}

QPointF AuDesignerTextureWrapView::InputBase() const
{
	//qDebug() << "AuDesignerTextureWrapView::paintEvent";
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();
	if(!view)
		return QPointF(0,0);
	if(width() == 0 || height() == 0)
		return QPointF(0,0);

	int layer = Layer();
	if(layer < 0 || layer > 1)
		return QPointF(0,0);

	QPoint min, max;	// Extents of pixel locations of selected vertices.
	bool minmaxset = false;
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			const AuScParts* parts = pde->LockPartsForRead();
			if(parts)
			{
				AuScSelection sel = view->DesignerPartObjectSelection();
				for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
				{
					AuScParts::const_iterator pi = parts->find(si.key());
					if(pi == parts->cend())
						continue;

					// Part is selected. We only show selected faces.
					AuScFaceList faces;
					(*pi)->Faces(faces);

					int findex = 0;
					int vindex = 0;
					for(AuScFaceList::const_iterator fi = faces.cbegin(); fi != faces.cend();
							findex++, vindex += (*fi)->VertexCount(), fi++)
					{
						if(!(*si).m_Faces.contains(findex))
							continue;	// Skip unselected face.

						// Show selected face.
						AuSegmentArray edges;
						(*fi)->EdgeSegments(edges);
						for(AuSegmentArray::const_iterator ei = edges.cbegin(); ei != edges.cend(); ei++)
						{
							int vsIdx = (*fi)->VertexAt((*ei).m_fLoc1);
							if(vsIdx < 0)
								continue;
							int veIdx = (*fi)->VertexAt((*ei).m_fLoc2);
							if(veIdx < 0)
								continue;

							// Calculate pixel locations of the end point texture coords.
							AuScPoint vs = (*fi)->Vertex(vsIdx);
							AuScPoint ve = (*fi)->Vertex(veIdx);
							QPoint pxs;
							QPoint pxe;
							if(layer == 0)
							{
								pxs = TextureCoordToPixel(vs.TexCoord()).toPoint();
								pxe = TextureCoordToPixel(ve.TexCoord()).toPoint();
							}
							else
							{
								pxs = TextureCoordToPixel(vs.EffectCoord()).toPoint();
								pxe = TextureCoordToPixel(ve.EffectCoord()).toPoint();
							}

							// Accumulate extents of vertices.
							if(!minmaxset)
							{
								min = pxs;
								max = pxs;
								minmaxset = true;
							}
							else
							{
								min.setX(MIN(min.x(), pxs.x()));
								min.setY(MIN(min.y(), pxs.y()));
								max.setX(MAX(max.x(), pxs.x()));
								max.setY(MAX(max.y(), pxs.y()));
							}

							min.setX(MIN(min.x(), pxe.x()));
							min.setY(MIN(min.y(), pxe.y()));
							max.setX(MAX(max.x(), pxe.x()));
							max.setY(MAX(max.y(), pxe.y()));
						}
					}
				}
			}
			pde->UnlockParts();
		}
	}
	view->UnlockSceneAtom();

	return QPointF(min + max)/2.f;
}

void AuDesignerTextureWrapView::CancelInput()
{
	if(m_InputPerformed)
	{
		// Input did something that we can undo at this point.
		auDesignerApp->DesignerFrame()->DesignerView()->DesignerUndo();
		m_InputPerformed = false;
	}

	m_InputMode = INPUT_Select;
}

void AuDesignerTextureWrapView::Grab()
{
	ClearConstraints();					// Start with no constraints.
	m_InputMode = INPUT_Grab;
	m_InputPerformed = false;			// Until we do something that can be undone.
	m_InputPos = QWidget::mapFromGlobal(QCursor::pos());
	m_UndoSeq++;						// New sequence for undo group.
}

void AuDesignerTextureWrapView::Rotate()
{
	ClearConstraints();					// Start with no constraints.
	m_InputMode = INPUT_Rotate;
	m_InputPerformed = false;			// Until we do something that can be undone.
	m_InputPos = InputBase();

	// Init angle based on current cursor position.
	QPoint curPos = QWidget::mapFromGlobal(QCursor::pos());
	AuPoint drag(curPos - m_InputPos);
	m_Angle = drag.ZRotation();
	m_UndoSeq++;						// New sequence for undo group.
}

void AuDesignerTextureWrapView::Stretch()
{
	ClearConstraints();					// Start with no constraints.
	m_InputMode = INPUT_Stretch;
	m_InputPerformed = false;			// Until we do something that can be undone.
	m_InputPos = InputBase();
	m_StretchPos = QWidget::mapFromGlobal(QCursor::pos());
	m_UndoSeq++;						// New sequence for undo group.
}

void AuDesignerTextureWrapView::ConstrainX()
{
	if(m_Constrain == 1)
		m_Constrain = 0;
	else
		m_Constrain = 1;
}

void AuDesignerTextureWrapView::ConstrainY()
{
	if(m_Constrain == 2)
		m_Constrain = 0;
	else
		m_Constrain = 2;
}

void AuDesignerTextureWrapView::ClearConstraints()
{
	m_Constrain = 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDesignerTextureWrap::AuDesignerTextureWrap(QWidget* parent) : QWidget(parent)
{
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();
	Q_ASSERT(view);

	m_timer = 0;

	m_TextureLayer = new QComboBox(this);
	m_TextureLayer->setToolTip(tr("<html>"
				"<p>"
				"<b>Layer</b><br>"
				"Changes the layer for applying textures. "
				"</p>"
				"<p>"
				"The base layer is visible on all faces. "
				"A texture should be applied to the base layer to give faces their primary appearance. "
				"</p>"
				"<p>"
				"The effects layer may be used to overlay animated effects onto the base texture. "
				"This gives an animated appearance to engines, weapons and transporter pads, when they are functioning. "
				"</p>"
				"</html"));
	m_TextureLayer->addItem(tr("Base"), 0);
	m_TextureLayer->addItem(tr("Effects"), 1);
	connect(m_TextureLayer, SIGNAL(currentIndexChanged(int)), SLOT(TextureLayerIndexChanged(int)));

	m_TextureEffects = new QComboBox(this);
	m_TextureEffects->setToolTip(tr("<html>"
				"<b>Effect Texture</b><br>"
				"Changes the effect texture applied to the selected faces. "
				"Lists the effect textures available to the design. "
				"<br>"
				"%1 FTL Drive animates when the FTL drive is active."
				"<br>"
				"%1 Life Support animates when the life support system is functioning."
				"<br>"
				"%1 Maneuver Drive animates when the maneuver drive is on, dimmer when holding."
				"<br>"
				"%1 Power Plant animates when the power plant is on."
				"<br>"
				"%1 Sensor animates when the sensor is on."
				"<br>"
				"%1 Shield animates when the shield is on."
				"<br>"
				"%1 Transporter Pad animates when a transporter system is on, brighter when active."
				"<br>"
				"%1 Weapon animates when a weapon system is firing."
				"</html").arg(g_Au.MiddleDotSymbol()));
	connect(m_TextureEffects, SIGNAL(currentIndexChanged(int)), SLOT(TextureEffectChanged(int)));
	m_TextureEffects->hide();

	m_TexturePalette = new QComboBox(this);
	m_TexturePalette->setToolTip(tr("<html>"
				"<b>Base Texture</b><br>"
				"Changes the base texture applied to the selected faces. "
				"Lists the textures in the design's texture palette. "
				"<br>"
				"<b>Face mode must be enabled to apply textures to faces.</b> "
				"</html"));
	connect(m_TexturePalette, SIGNAL(currentIndexChanged(int)), SLOT(TexturePaletteIndexChanged(int)));

	m_TextureWrapper = new AuDesignerTextureWrapView(this);
	connect(m_TextureWrapper, SIGNAL(TextureMove(QPointF, quint8)), SLOT(TextureMove(QPointF, quint8)));
	connect(m_TextureWrapper, SIGNAL(TextureRotate(QPointF, double, quint8)), SLOT(TextureRotate(QPointF, double, quint8)));
	connect(m_TextureWrapper, SIGNAL(TextureStretch(QPointF, double, double, quint8)),
			SLOT(TextureStretch(QPointF, double, double, quint8)));

	m_Unwrap = new AuIconButton(QIcon(":data/ScUnwrapVert.png"), this);
	m_Unwrap->setToolTip(tr("<html>"
				"<b>Unwrap...</b><br>"
				"Unwraps selected faces onto the texture. "
				"A window presents options. "
				"<br>"
				"<b>Face mode must be enabled to apply textures to faces.</b> "
				"</html"));
	connect(m_Unwrap, SIGNAL(clicked()), SLOT(Unwrap()));

	m_ProjectCylinder = new AuIconButton(QIcon(":data/ScUnwrapCylinder.png"), this);
	connect(m_ProjectCylinder, SIGNAL(clicked()), SLOT(TextureCylindrical()));
	m_ProjectCylinder->setToolTip(tr("<html>"
				"<b>Cylindrical Projection</b><br>"
				"Maps the texture to the selected faces using a cylindrical projection. "
				"Position of the grid plane affects the result. "
				"<br>"
				"<b>Face mode must be enabled to apply textures to faces.</b> "
				"</html"));

	m_ProjectPlane = new AuIconButton(QIcon(":data/ScUnwrapProject.png"), this);
	connect(m_ProjectPlane, SIGNAL(clicked()), SLOT(TextureProjected()));
	m_ProjectPlane->setToolTip(tr("<html>"
				"<b>Plane Projection</b><br>"
				"Maps the texture to the selected faces using a plane projection. "
				"Position of the grid plane affects the result. "
				"<br>"
				"<b>Face mode must be enabled to apply textures to faces.</b> "
				"</html"));

	m_ProjectSphere = new AuIconButton(QIcon(":data/ScUnwrapSphere.png"), this);
	connect(m_ProjectSphere, SIGNAL(clicked()), SLOT(TextureSpherical()));
	m_ProjectSphere->setToolTip(tr("<html>"
				"<b>Spherical Projection</b><br>"
				"Maps the texture to the selected faces using a spherical projection. "
				"Position of the grid plane affects the result. "
				"<br>"
				"<b>Face mode must be enabled to apply textures to faces.</b> "
				"</html"));

	m_TexRepeat = new AuIconButton(QIcon(":data/ScRepeat.png"), this);
	connect(m_TexRepeat, SIGNAL(clicked()), SLOT(ToggleRepeat()));
	m_TexRepeat->setToolTip(tr("<html>"
				"<b>Repeat</b><br>"
				"Shows a repeated pattern of the texture to aid in positioning. "
				"<u>All textures repeat</u>, regardless of this setting. "
				"</html"));

	m_TexPalette = new AuIconButton(QIcon(":data/ScTexPalette.png"), this);
	connect(m_TexPalette, SIGNAL(clicked()), view, SLOT(DesignerTexturePalette()));
	m_TexPalette->setToolTip(tr("<html>"
				"<b>Texture Palette Properties...</b><br>"
				"Configures the texture palette that is used by the design. "
				"</html"));

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addWidget(m_TextureLayer);
	blay->addStretch();
	blay->addWidget(m_Unwrap);
	blay->addWidget(m_ProjectPlane);
	blay->addWidget(m_ProjectCylinder);
	blay->addWidget(m_ProjectSphere);
	blay->addStretch();
	blay->addWidget(m_TexRepeat);
	blay->addStretch();
	blay->addWidget(m_TextureEffects);
	blay->addWidget(m_TexturePalette);
	blay->addWidget(m_TexPalette);

	QVBoxLayout* lay = new QVBoxLayout();
	lay->addLayout(blay);
	lay->addWidget(m_TextureWrapper);
	lay->setMargin(4);
	lay->setSpacing(2);
	setLayout(lay);
}

AuDesignerTextureWrap::~AuDesignerTextureWrap()
{
}

void AuDesignerTextureWrap::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(200);

	RestoreSettings();
	UpdateContents();
}

void AuDesignerTextureWrap::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_timer && m_timer->isActive())
		m_timer->stop();

	SaveSettings();
}

void AuDesignerTextureWrap::UpdateContents()
{
	AuDesignerView* pView = auDesignerApp->DesignerFrame()->DesignerView();
	if(!pView)
		return;

	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	if(!m_TextureEffects->count())
		ListEffects(-1);

	if(!m_TexturePalette->count())
		ListTextures(pView, -1);
}

void AuDesignerTextureWrap::ListTextures(AuView* view, int curIdx)
{
	COMMODITYID hullModule = CID_Invalid;
	AuDesignTex palette[SCTEXI_Count];
	{
		DaPerson* person;
		view->LockSceneAtom(&person);
		if(person)
		{
			DaDesigner* pde = person->FindParentDesignerInEnv();
			if(pde)
			{
				pde->TexturePalette(palette);
				const AuDesign* design = pde->LockDesignForRead();
				if(design)
					hullModule = design->HullModule();
				pde->UnlockDesign();
			}
		}
		view->UnlockSceneAtom();
	}

	m_TexturePalette->blockSignals(true);
	m_TexturePalette->clear();
	int iconSize = 32;
	for(int texIdx=0; texIdx<SCTEXI_Count; texIdx++)
	{
		QPixmap imgTile;
		QString name;
		if(!palette[texIdx].Image().isNull())
		{
			name = palette[texIdx].ImageName();
			imgTile = QPixmap::fromImage(palette[texIdx].Image());
		}
		else if(texIdx == SCTEXI_Material)
		{
			name = QObject::tr("Material");
			switch(hullModule)
			{
			default:
			case CID_WoodHullModule:
				imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullWood()));
				break;
			case CID_LumberHullModule:
				imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullLumber()));
				break;
			case CID_StoneHullModule:
				imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullStone()));
				break;
			case CID_MetalHullModule:
				imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullMetal()));
				break;
			case CID_MagmiumHullModule:
				imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullMagmium()));
				break;
			case CID_VulciumHullModule:
				imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullVulcium()));
				break;
			case CID_AdamantineHullModule:
				imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullAdamantine()));
				break;
			}
		}
		else
		{
			name = AuTextureLibItem::LibraryTextureName(palette[texIdx].TexLibId());
			imgTile.load(AuTextureLibItem::LibraryTextureFile(palette[texIdx].TexLibId()));
		}

		QString itemName = QString("%1. %2").arg(texIdx+1).arg(name);

		if(!imgTile.isNull())
		{
			QPixmap imgSmall = imgTile.scaled(iconSize,iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			m_TexturePalette->addItem(QIcon(imgSmall), itemName, texIdx);
		}
		else
			m_TexturePalette->addItem(itemName, texIdx);
	}

	if(curIdx >= 0)
		m_TexturePalette->setCurrentIndex(m_TexturePalette->findData(curIdx));

	m_TexturePalette->blockSignals(false);
}

void AuDesignerTextureWrap::ListEffects(int curIdx)
{
	QMap<int, SURFACE_EFFECT> sortedEffects;
	for(int eff=EFF_None; eff<EFF_Count; eff++)
	{
		SURFACE_EFFECT effect = (SURFACE_EFFECT)eff;
		int effSort = SurfaceEffectSort(effect);
		if(effSort == 0)
			continue;
		sortedEffects.insert(effSort, effect);
	}
	if(sortedEffects.empty())
		return;

	m_TextureEffects->blockSignals(true);
	m_TextureEffects->clear();
	int iconSize = 32;
	for(QMap<int, SURFACE_EFFECT>::const_iterator ei = sortedEffects.cbegin(); ei != sortedEffects.cend(); ei++)
	{
		QString effectName = SurfaceEffectName(*ei);
		QPixmap imgTile(AuTextureLibItem::EffectTextureItem(*ei).FileName());
		if(!imgTile.isNull())
		{
			QPixmap imgSmall = imgTile.scaled(iconSize,iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			m_TextureEffects->addItem(QIcon(imgSmall), effectName, *ei);
		}
		else
			m_TextureEffects->addItem(effectName, *ei);
	}

	if(curIdx >= 0)
		m_TextureEffects->setCurrentIndex(m_TextureEffects->findData(curIdx));
	m_TextureEffects->blockSignals(false);
}

void AuDesignerTextureWrap::EnableControls()
{
}

void AuDesignerTextureWrap::RestoreSettings()
{
}

void AuDesignerTextureWrap::SaveSettings()
{
}

void AuDesignerTextureWrap::ToggleRepeat()
{
	m_TextureWrapper->SetRepeat(!m_TextureWrapper->IsRepeating());
	m_TexRepeat->setDown(m_TextureWrapper->IsRepeating());
}

void AuDesignerTextureWrap::DesignerSelectionChanged()
{
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();
	if(!view)
		return;

	int layer = m_TextureLayer->itemData(m_TextureLayer->currentIndex()).toInt();
	if(layer == 0)
	{
		// Base
		if(!m_TexturePalette->count())
			return;

		qint8 curIdx = -1;

		DaPerson* person;
		view->LockSceneAtom(&person);
		if(person)
		{
			DaDesigner* pde = person->FindParentDesignerInEnv();
			if(pde)
			{
				const AuDesign* design = pde->LockDesignForRead();
				if(design)
					curIdx = design->TextureIndex(view->DesignerPartObjectSelection());	// Returns SCTEXI_Count if they differ.
				pde->UnlockDesign();
			}
		}
		view->UnlockSceneAtom();

		m_TexturePalette->blockSignals(true);
		m_TexturePalette->setCurrentIndex(m_TexturePalette->findData(curIdx));
		m_TexturePalette->blockSignals(false);
		m_TextureWrapper->DesignerSelectionChanged();
	}
	else if(layer == 1)
	{
		// Effects
		if(!m_TextureEffects->count())
			return;

		qint8 curIdx = -1;

		DaPerson* person;
		view->LockSceneAtom(&person);
		if(person)
		{
			DaDesigner* pde = person->FindParentDesignerInEnv();
			if(pde)
			{
				const AuDesign* design = pde->LockDesignForRead();
				if(design)
					curIdx = design->Effect(view->DesignerPartObjectSelection());	// Returns EFF_Count if they differ.
				pde->UnlockDesign();
			}
		}
		view->UnlockSceneAtom();

		m_TextureEffects->blockSignals(true);
		m_TextureEffects->setCurrentIndex(m_TextureEffects->findData(curIdx));
		m_TextureEffects->blockSignals(false);
		m_TextureWrapper->DesignerSelectionChanged();
	}
}

void AuDesignerTextureWrap::TexturePaletteIndexChanged(int index)
{
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();

	SC_TEXTURE_INDEX texIdx = (SC_TEXTURE_INDEX)m_TexturePalette->itemData(index).toUInt();
	m_TextureWrapper->SetTextureIndex(texIdx);
	auApp->DesignerPartsTextureFaces(view->DesignerPartObjectSelection(), texIdx);
}

void AuDesignerTextureWrap::TextureMove(QPointF vec, quint8 undoSeq)
{
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();

	int layer = m_TextureLayer->itemData(m_TextureLayer->currentIndex()).toInt();
	if(layer == 1)
		auApp->DesignerPartsEffectMove(view->DesignerPartObjectSelection(), vec, undoSeq);
	else
		auApp->DesignerPartsTextureMove(view->DesignerPartObjectSelection(), vec, undoSeq);
}

void AuDesignerTextureWrap::TextureRotate(QPointF pivot, double angle, quint8 undoSeq)
{
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();

	int layer = m_TextureLayer->itemData(m_TextureLayer->currentIndex()).toInt();
	if(layer == 1)
		auApp->DesignerPartsEffectRotate(view->DesignerPartObjectSelection(), pivot, angle, undoSeq);
	else
		auApp->DesignerPartsTextureRotate(view->DesignerPartObjectSelection(), pivot, angle, undoSeq);
}

void AuDesignerTextureWrap::TextureStretch(QPointF base, double scaleX, double scaleY, quint8 undoSeq)
{
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();

	int layer = m_TextureLayer->itemData(m_TextureLayer->currentIndex()).toInt();
	if(layer == 1)
		auApp->DesignerPartsEffectStretch(view->DesignerPartObjectSelection(), base, scaleX, scaleY, undoSeq);
	else
		auApp->DesignerPartsTextureStretch(view->DesignerPartObjectSelection(), base, scaleX, scaleY, undoSeq);
}

void AuDesignerTextureWrap::Unwrap()
{
	AuDesignerFrame* frame = auDesignerApp->DesignerFrame();
	AuDesignerView* view = frame->DesignerView();

	int layer = m_TextureLayer->itemData(m_TextureLayer->currentIndex()).toInt();
	if(layer == 0)
	{
		// Base
		COMMODITYID hullModule = CID_Invalid;
		AuDesignTex palette[SCTEXI_Count];
		{
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				DaDesigner* pde = person->FindParentDesignerInEnv();
				if(pde)
				{
					pde->TexturePalette(palette);
					const AuDesign* design = pde->LockDesignForRead();
					if(design)
						hullModule = design->HullModule();
					pde->UnlockDesign();
				}
			}
			view->UnlockSceneAtom();
		}

		SC_TEXTURE_INDEX texIdx = (SC_TEXTURE_INDEX)m_TexturePalette->itemData(m_TexturePalette->currentIndex()).toUInt();
		AuScTextureUnwrapDlg dlg(texIdx, hullModule, palette, this);
		if(frame->ExecDialog(dlg) != QDialog::Accepted)
			return;

		texIdx = dlg.TextureIndex();
		double texSize = dlg.TextureSize();
		AuScFaceList::UNWRAP_METHOD unwrap = dlg.UnwrapMethod();
		float angleTol = dlg.AngleTolerance();
		bool gridAlign = dlg.GridAlign();
		int alignAxis = dlg.AlignAxis();

		auApp->DesignerPartsTextureUnwrap(view->DesignerPartObjectSelection(), texIdx, texSize, unwrap, angleTol,
				gridAlign, alignAxis, view->DesignPlaneMatrix());
	}
	else
	{
		// Effects
		SURFACE_EFFECT effect = (SURFACE_EFFECT)m_TextureEffects->itemData(m_TextureEffects->currentIndex()).toUInt();
		AuScEffectUnwrapDlg dlg(effect, this);
		if(frame->ExecDialog(dlg) != QDialog::Accepted)
			return;

		effect = dlg.Effect();
		double texSize = dlg.TextureSize();
		AuScFaceList::UNWRAP_METHOD unwrap = dlg.UnwrapMethod();
		float angleTol = dlg.AngleTolerance();
		bool gridAlign = dlg.GridAlign();
		int alignAxis = dlg.AlignAxis();

		auApp->DesignerPartsEffectUnwrap(view->DesignerPartObjectSelection(), effect, texSize, unwrap, angleTol,
				gridAlign, alignAxis, view->DesignPlaneMatrix());
	}
}

void AuDesignerTextureWrap::TextureEffectChanged(int index)
{
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();

	SURFACE_EFFECT effect = (SURFACE_EFFECT)m_TextureEffects->itemData(index).toUInt();
	m_TextureWrapper->SetEffect(effect);
	auApp->DesignerPartsEffectFaces(view->DesignerPartObjectSelection(), effect);
}

void AuDesignerTextureWrap::TextureLayerIndexChanged(int index)
{
	m_TextureWrapper->SetLayer(index);
	if(index == 1)
	{
		m_TexturePalette->setVisible(false);
		m_TexPalette->setVisible(false);
		m_TextureEffects->setVisible(true);
	}
	else
	{
		m_TextureEffects->setVisible(false);
		m_TexturePalette->setVisible(true);
		m_TexPalette->setVisible(true);
	}
}

void AuDesignerTextureWrap::TextureCylindrical()
{
	AuDesignerView* view = auDesignerApp->DesignerFrame()->DesignerView();
	int layer = m_TextureLayer->itemData(m_TextureLayer->currentIndex()).toInt();
	view->DesignerTextureCylindrical(layer);

}

void AuDesignerTextureWrap::TextureProjected()
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	if(!pFrame)
		return;
	AuDesignerView* view = pFrame->DesignerView();
	if(!view)
		return;
	int layer = m_TextureLayer->itemData(m_TextureLayer->currentIndex()).toInt();
	view->DesignerTextureProjected(layer);
}

void AuDesignerTextureWrap::TextureSpherical()
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	if(!pFrame)
		return;
	AuDesignerView* view = pFrame->DesignerView();
	if(!view)
		return;
	int layer = m_TextureLayer->itemData(m_TextureLayer->currentIndex()).toInt();
	view->DesignerTextureSpherical(layer);
}
