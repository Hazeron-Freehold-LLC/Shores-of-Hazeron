//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autabcolumn.cpp,v 1.18 2022/04/11 20:17:34 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "autabcolumn.h"
//
#include "austylesheet.h"
#include <QDebug>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>

#define SCROLL_OVERHANG					6						// Number of pixels to show of first item when scrolled away.

QSize AuTabItem::SizeHint(int tabHeight) const
{
	// Start with our known tab height. A little width allows for the semi-circle at the edge of the tab.
	QSize hint(tabHeight,tabHeight);
	
	// Allow space for the icon, if any.
	if(!m_Icon.isNull())
		hint.setWidth(hint.width() + tabHeight);				// Allow a square space for the icon.

	//DHH Take a wag at text width without doing too much work.
	if(!m_Label.isEmpty())
		hint.setWidth(hint.width() + m_Label.length()*tabHeight/2);
	return hint;
}

void AuTabItem::Paint(QPainter& p, int tabHeight, int tabWidth, bool isCurrentTab, bool isMouseOverControl)
{
	// Draw tab frame.
	QPixmap pxleft;
	QPixmap pxright;
	if(isCurrentTab)
	{
		// Load bright tab images for left and right sides of current tab.
		pxleft.load(":data/TabLeftCurrent.png");	// 32 pixels wide
		pxright.load(":data/TabRightCurrent.png");	// 64 pixels wide
	}
	else
	{
		// Load dim tab images for left and right sides of tab.
		pxleft.load(":data/TabLeft.png");	// 32 pixels wide
		pxright.load(":data/TabRight.png");	// 64 pixels wide
	}

	// Draw images at left and right sides.
	QRect rc(0,0, pxleft.width(), tabHeight+1);
	p.drawPixmap(rc, pxleft);
	if(tabWidth > pxleft.width())
	{
		QRect rc(pxleft.width(),0, tabWidth - pxleft.width(), tabHeight+1);
		p.drawPixmap(rc, pxright);
	}

	// Draw close button.
	switch(m_ShowControl)
	{
	case 0:
		break;
	case 1:
		if(isMouseOverControl)
		{
			QPixmap pxclose(":data/TabClose.png");
			p.drawPixmap(tabHeight/2 - pxclose.width()/2, tabHeight/2 - pxclose.height()/2, pxclose);
		}
		break;
	case 2:
		{
			QPixmap pxclose(":data/TabWindow.png");
			p.drawPixmap(tabHeight/2 - pxclose.width()/2, tabHeight/2 - pxclose.height()/2, pxclose);
		}
		break;
	}

	int lpos = tabHeight;

	// Draw icon, if any.
	if(!m_Icon.isNull())
	{
		int iconSize = tabHeight - 4;
		p.drawPixmap(lpos,3, iconSize,iconSize, m_Icon.pixmap(iconSize,iconSize));
		lpos += iconSize + 3;
	}

	// Draw label text, if any.
	if(!m_Label.isEmpty())
	{
		p.setPen(QPen(m_TextColor, 0.f));
		p.drawText(lpos,1, tabWidth-lpos-1,tabHeight-2, Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine, m_Label);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuTabColumn::AuTabColumn(QWidget* parent) : QWidget(parent)
{
	m_CurrentTabIndex = 0;
	m_DragTabIndex = -1;
	m_DragTabsEnabled = false;
	m_FirstVisibleTab = 0;
	m_MouseOverControlIndex = -1;
	m_TabHeight = AuStyleSheet::TabHeight();

	setFocusPolicy(Qt::ClickFocus);
	setMouseTracking(true);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
}

AuTabColumn::~AuTabColumn()
{
}

int AuTabColumn::AddTab(const QIcon& icon)
{
	// Add a tab and update the geometry to fit accordingly.
	m_Tabs.append(AuTabItem(icon));
	QWidget::updateGeometry();

	// Set current index to first entry when it is the only entry.
	if(Count() == 1)
		SetCurrentIndex(0);

	// Return the index of the nu tab.
	return Count()-1;
}

int AuTabColumn::AddTab(const QIcon& icon, const QString& label)
{
	// Add a tab and update the geometry to fit accordingly.
	m_Tabs.append(AuTabItem(icon, label));
	QWidget::updateGeometry();

	// Set current index to first entry when it is the only entry.
	if(Count() == 1)
		SetCurrentIndex(0);

	// Return the index of the nu tab.
	return Count()-1;
}

int AuTabColumn::AddTab(const QString& label)
{
	// Add a tab and update the geometry to fit accordingly.
	m_Tabs.append(AuTabItem(label));
	QWidget::updateGeometry();

	// Set current index to first entry when it is the only entry.
	if(Count() == 1)
		SetCurrentIndex(0);

	// Return the index of the nu tab.
	return Count()-1;
}

int AuTabColumn::InsertTab(int index, const QIcon& icon)
{
	// If the index is outside the valid range, simply add the tab and return its index.
	if(index >= Count())
		return AddTab(icon);
	else if(index < 0)
		index = 0;

	// Insert a tab and update the geometry to fit accordingly.
	m_Tabs.insert(index, AuTabItem(icon));
	QWidget::updateGeometry();

	// Push current and first visible indices up if we inserted before or at their positions.
	if(m_FirstVisibleTab >= index)
		m_FirstVisibleTab++;
	if(CurrentIndex() >= index)
		SetCurrentIndex(CurrentIndex()+1);

	// Return the index of the nu tab.
	return index;
}

int AuTabColumn::InsertTab(int index, const QIcon& icon, const QString& label)
{
	// If the index is outside the valid range, simply add the tab and return its index.
	if(index >= Count())
		return AddTab(icon, label);
	else if(index < 0)
		index = 0;

	// Insert a tab and update the geometry to fit accordingly.
	m_Tabs.insert(index, AuTabItem(icon, label));
	QWidget::updateGeometry();

	// Push current and first visible indices up if we inserted before or at their positions.
	if(m_FirstVisibleTab >= index)
		m_FirstVisibleTab++;
	if(CurrentIndex() >= index)
		SetCurrentIndex(CurrentIndex()+1);

	// Return the index of the nu tab.
	return index;
}

int AuTabColumn::InsertTab(int index, const QString& label)
{
	// If the index is outside the valid range, simply add the tab and return its index.
	if(index >= Count())
		return AddTab(label);
	else if(index < 0)
		index = 0;

	// Insert a tab and update the geometry to fit accordingly.
	m_Tabs.insert(index, AuTabItem(label));
	QWidget::updateGeometry();

	// Push current and first visible indices up if we inserted before or at their positions.
	if(m_FirstVisibleTab >= index)
		m_FirstVisibleTab++;
	if(CurrentIndex() >= index)
		SetCurrentIndex(CurrentIndex()+1);

	// Return the index of the nu tab.
	return index;
}

void AuTabColumn::RemoveTab(int tabIndex)
{
	// Do nothing if the tab index is not valid.
	if(tabIndex >= 0 && tabIndex < Count())
	{
		// Remove a tab and update the geometry to fit the remaining tabs accordingly.
		m_Tabs.removeAt(tabIndex);
		QWidget::updateGeometry();

		// Pull current and first visible indices down if we removed before or at their positions.
		if(m_FirstVisibleTab > 0 && m_FirstVisibleTab >= tabIndex)
			m_FirstVisibleTab--;
		if(CurrentIndex() > 0 && CurrentIndex() >= tabIndex)
			SetCurrentIndex(CurrentIndex()-1);
		update();
	}
}

QSize AuTabColumn::sizeHint() const
{
	// Collect size hint for a column of tabs.
	QSize hint;
	for(QList<AuTabItem>::const_iterator ti = m_Tabs.constBegin(); ti != m_Tabs.constEnd(); ti++)
	{
		QSize itemHint = (*ti).SizeHint(m_TabHeight);

		// Width is that of the widest tab.
		if(hint.width() < itemHint.width())
			hint.setWidth(itemHint.width());

		// Height is the sum of all the tab heights.
		hint.setHeight(hint.height() + itemHint.height());
	}
	return hint;
}

void AuTabColumn::paintEvent(QPaintEvent* /*pe*/)
{
	QPainter p(this);

	// Fill background to black.
	p.fillRect(rect(), QColor(0,0,0));

	// When items are scrolled off the top of the widget, leave a little overhand to make it apparent.
	if(m_FirstVisibleTab > 0)
		p.translate(QPoint(0,SCROLL_OVERHANG-m_TabHeight*m_FirstVisibleTab));

	// Draw all but the current item.
	int tabIndex = 0;
	for(QList<AuTabItem>::iterator ti = m_Tabs.begin(); ti != m_Tabs.end(); ti++)
	{
		if(tabIndex != CurrentIndex())
		{
			// Draw a tab that is not the current tab.
			bool showControlIndicator = (m_MouseOverControlIndex == tabIndex && m_DragTabIndex < 0);
			(*ti).Paint(p, m_TabHeight, width(), false, showControlIndicator);
		}

		// Move down by one tab height.
		p.translate(QPoint(0,m_TabHeight));
		tabIndex++;
	}

	// Draw the current item last.
	// This way its image overlaps all the others and it pops above them.
	if(CurrentIndex() >= 0 && CurrentIndex() < Count())
	{
		// Position transform to draw the current tab.
		// When items are scrolled off the top of the widget, there is a little overhand to make it apparent.
		p.resetTransform();
		if(m_FirstVisibleTab > 0)
			p.translate(QPoint(0,SCROLL_OVERHANG-m_TabHeight*(m_FirstVisibleTab - CurrentIndex())));
		else
			p.translate(QPoint(0,m_TabHeight*CurrentIndex()));

		// Draw the current tab.
		bool showControlIndicator = (m_MouseOverControlIndex == CurrentIndex() && m_DragTabIndex < 0);
		m_Tabs[CurrentIndex()].Paint(p, m_TabHeight, width(), true, showControlIndicator);
	}
}

void AuTabColumn::SetTabShowControl(int tabIndex, int showControl)
{
	// Don't do anything if the tab index is invalid.
	if(tabIndex >= 0 && tabIndex < Count())
		if(m_Tabs[tabIndex].SetShowControl(showControl))			// Change the show close state of the tab.
			update();											// Show close change requires repaint.
}

void AuTabColumn::SetTabTextColor(int tabIndex, const QColor& color)
{
	// Don't do anything if the tab index is invalid.
	if(tabIndex >= 0 && tabIndex < Count())
		if(m_Tabs[tabIndex].SetTextColor(color))				// Change the text color of the tab.
			update();											// Color change requires repaint.
}

void AuTabColumn::SetTabToolTip(int tabIndex, const QString& toolTip)
{
	// Don't do anything if the tab index is invalid.
	if(tabIndex >= 0 && tabIndex < Count())
		m_Tabs[tabIndex].SetToolTip(toolTip);					// Change the tool tip of the tab.
}

int AuTabColumn::TabAt(const QPoint& pos) const
{
	if(m_FirstVisibleTab > 0)
	{
		// When items are scrolled off the top of the widget, there is a little overhand to make it apparent.
		if(pos.y() < SCROLL_OVERHANG)
			return m_FirstVisibleTab - 1;						// The half-hidden item revealed by the overhang.
		else
			return m_FirstVisibleTab + (pos.y()-SCROLL_OVERHANG) / m_TabHeight;	// The top full visible item or below.
	}
	else
		return pos.y() / m_TabHeight;							// No overhang so calc is easy.
}

void AuTabColumn::mouseMoveEvent(QMouseEvent *e)
{
	QWidget::mouseMoveEvent(e);

	// Determine if the mouse is over any of the close buttons.
	int mouseOverControlIndex = m_MouseOverControlIndex;
	int tabIndex = TabAt(e->pos());
	if(e->pos().x() > 0 && e->pos().x() < m_TabHeight)
		mouseOverControlIndex = tabIndex;
	else
		mouseOverControlIndex = -1;
	if(mouseOverControlIndex != m_MouseOverControlIndex)
	{
		// The state of the mouse being over a close button changed so repaint is required.
		m_MouseOverControlIndex = mouseOverControlIndex;
		update();
	}

	if(tabIndex < 0 || tabIndex >= Count())
	{
		// No tab under cursor.
		if(m_DragTabIndex >= 0 && m_DragTabIndex < Count() && tabIndex >= Count())
		{
			// Move the tab that was dragged to the end.
			int oldIndex = m_DragTabIndex;
			AuTabItem takeItem = m_Tabs.takeAt(oldIndex);
			m_Tabs.append(takeItem);
			m_DragTabIndex = Count()-1;

			// The current tab index is now the dragging tab index.
			SetCurrentIndex(m_DragTabIndex);

			// Notify the world that a tab moved.
			emit TabMoved(oldIndex, m_DragTabIndex);
		}

		// Update tool tip.
		if(toolTip() != "")
			setToolTip("");
	}
	else
	{
		// A tab is under the cursor.
		if(m_DragTabIndex >= 0 && m_DragTabIndex < Count() && m_DragTabIndex != tabIndex)
		{
			// Move the tab that was dragged.
			int oldIndex = m_DragTabIndex;
			AuTabItem takeItem = m_Tabs.takeAt(oldIndex);
			if(m_DragTabIndex > tabIndex)
				m_DragTabIndex = tabIndex;						// Target item index unaffected by removal of the drag item.
			else
				m_DragTabIndex = tabIndex;					// Target item index dropped by removal of the drag item.
			m_Tabs.insert(m_DragTabIndex, takeItem);

			// The current tab index is now the dragging tab index.
			SetCurrentIndex(m_DragTabIndex);

			// Notify the world that a tab moved.
			emit TabMoved(oldIndex, m_DragTabIndex);
		}

		// Get tool tip from tab under cursor.
		QString tabToolTip = m_Tabs[tabIndex].ToolTip();
		if(toolTip() != tabToolTip)
			setToolTip(tabToolTip);
	}

	// Set the grabbing hand cursor when a drag operation has been initiated.
	if(m_DragTabIndex >= 0 && m_DragTabIndex < Count())
		setCursor(Qt::ClosedHandCursor);
	else
		unsetCursor();
}

void AuTabColumn::mousePressEvent(QMouseEvent *e)
{
	QWidget::mousePressEvent(e);

	if(m_MouseOverControlIndex >= 0 && m_MouseOverControlIndex < Count())
	{
		// Mouse was pressed on a close button.
		switch(TabShowControl(m_MouseOverControlIndex))
		{
		case 0:
			break;
		case 1:
			emit TabCloseRequested(m_MouseOverControlIndex);
			break;
		case 2:
			emit TabWindowRequested(m_MouseOverControlIndex);
			break;
		}
	}
	//else
	{
		// Determine if a tab was clicked.
		int tabIndex = TabAt(e->pos());
		if(m_DragTabsEnabled && tabIndex != m_FirstVisibleTab - 1 && tabIndex >= 0 && tabIndex < Count())
		{
			// Initiate drag operation when tab is valid and it's not the half-hidden one at the top of a scrolled list.
			m_DragTabIndex = tabIndex;
			setCursor(Qt::ClosedHandCursor);
		}
		else
		{
			// Do not drag the tab that was clicked.
			m_DragTabIndex = -1;
			unsetCursor();
		}

		// Select the clicked tab index as current.
		SetCurrentIndex(tabIndex);
	}
}

void AuTabColumn::mouseReleaseEvent(QMouseEvent *e)
{
	QWidget::mouseReleaseEvent(e);

	if(m_DragTabIndex != -1)
	{
		// Clear the drag tab index and restore the cursor.
		m_DragTabIndex = -1;
		unsetCursor();
	}
}

void AuTabColumn::mouseDoubleClickEvent(QMouseEvent *e)
{
	QWidget::mouseDoubleClickEvent(e);

	bool overControlIndicator = (m_MouseOverControlIndex == m_CurrentTabIndex && m_DragTabIndex < 0);
	if(!overControlIndicator)
		emit TabDoubleClicked(m_CurrentTabIndex);
}

void AuTabColumn::wheelEvent(QWheelEvent *e)
{
	QWidget::wheelEvent(e);

	int topIndex = m_FirstVisibleTab;
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
	if(e->angleDelta().y() < 0)
		topIndex++;												// Scroll the list up.
	else if(e->angleDelta().y() > 0)
		topIndex--;												// Scroll the list down.
#else
	if(e->delta() < 0)
		topIndex++;												// Scroll the list up.
	else if(e->delta() > 0)
		topIndex--;												// Scroll the list down.
#endif

	if(topIndex != m_FirstVisibleTab && topIndex >= 0 && topIndex < Count())
	{
		// Scroll position changed and the nu top item position is valid. Repaint is required.
		m_FirstVisibleTab = topIndex;
		update();
	}
}

void AuTabColumn::keyPressEvent(QKeyEvent *e)
{
	QWidget::keyPressEvent(e);

	int tabIndex = CurrentIndex();
	switch(e->key())
	{
	case Qt::Key_Up:
	case Qt::Key_Left:
		// Select the previous tab in sequence.
		tabIndex--;
		break;
	case Qt::Key_Right:
	case Qt::Key_Down:
		// Select the next tab in sequence.
		tabIndex++;
		break;
	}

	// Selected tab index changed and the nu index is valid.
	SetCurrentIndex(tabIndex);
}

int AuTabColumn::TabShowControl(int tabIndex) const
{
	if(tabIndex >= 0 && tabIndex < Count())
		return m_Tabs[tabIndex].ShowControl();
	return false;
}

QString AuTabColumn::TabText(int tabIndex) const
{
	if(tabIndex >= 0 && tabIndex < Count())
		return m_Tabs[tabIndex].Label();
	return "";
}

QColor AuTabColumn::TabTextColor(int tabIndex) const
{
	if(tabIndex >= 0 && tabIndex < Count())
		return m_Tabs[tabIndex].TextColor();
	return QColor(255,255,255);
}

void AuTabColumn::SetCurrentIndex(int tabIndex)
{
	// Insure the nu tab index is valid.
	if(!Count())
		tabIndex = 0;											// No tabs to make current.
	else if(tabIndex < 0)
		tabIndex = 0;											// Less than the first so select the first.
	else if(tabIndex >= Count())
		tabIndex = Count()-1;									// Greater than the last so select the last.

	// Do nothing if the tab index isn't changing.
	if(tabIndex == m_CurrentTabIndex)
		return;

	// Change the index of the current tab.
	m_CurrentTabIndex = tabIndex;
	emit CurrentChanged(tabIndex);								// Notify the world the current tab changed.

	// Make sure the current tab is now visible.
	if(m_FirstVisibleTab > m_CurrentTabIndex)
		m_FirstVisibleTab = m_CurrentTabIndex;					// First visible was beyond current so scroll down to current.
	else
	{
		// End visibility is based on the number of whole tabs that are visible.
		int visibleTabs = height() / m_TabHeight;				// Number of whole visible tabs, could be zero.
		if(visibleTabs <= 1)
			m_FirstVisibleTab = m_CurrentTabIndex;				// Scroll up to current when one or less full tabs is visible.
		else
		{
			// Calculate first visible tab so that the current tab is the last full visible tab.
			int lastVisibleTab = m_FirstVisibleTab + (visibleTabs - 1);
			if(lastVisibleTab < m_CurrentTabIndex)
				m_FirstVisibleTab = m_CurrentTabIndex - (visibleTabs - 1);
		}
	}

	// Trigger a repaint.
	update();
}
