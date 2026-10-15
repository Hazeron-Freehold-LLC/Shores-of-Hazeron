//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autabcolumn.h,v 1.9 2022/02/19 18:31:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuTabColumn_H
#define AuTabColumn_H

#include "augloballib.h"
#include <QIcon>
#include <QWidget>

class QPainter;

class AUGLOBAL_DLL AuTabItem
{
private:
	QIcon								m_Icon;					// Image on the tab. May be null.
	QString								m_Label;				// Text on the tab. May be blank.
	int									m_ShowControl;			// 0=none, 1=close indicator, 2=window.
	QColor								m_TextColor;			// Color of label text.
	QString								m_ToolTip;				// Tool tip for this label.

public:
	AuTabItem()
	{
		m_ShowControl = 0;
		m_TextColor = QColor(255,255,255);
	}

	AuTabItem(const AuTabItem& c)
	{
		m_Icon = c.m_Icon;
		m_Label = c.m_Label;
		m_ShowControl = c.m_ShowControl;
		m_TextColor = c.m_TextColor;
		m_ToolTip = c.m_ToolTip;
	}

	AuTabItem(const QIcon& icon)
	{
		m_Icon = icon;
		m_ShowControl = 0;
		m_TextColor = QColor(255,255,255);
	}

	AuTabItem(const QIcon& icon, const QString& label)
	{
		m_Icon = icon;
		m_Label = label;
		m_ShowControl = 0;
		m_TextColor = QColor(255,255,255);
	}

	AuTabItem(const QString& label)
	{
		m_Label = label;
		m_ShowControl = 0;
		m_TextColor = QColor(255,255,255);
	}

	inline const AuTabItem& operator=(const AuTabItem& c)
	{
		m_Icon = c.m_Icon;
		m_Label = c.m_Label;
		m_ShowControl = c.m_ShowControl;
		m_TextColor = c.m_TextColor;
		m_ToolTip = c.m_ToolTip;
		return *this;
	}

	inline const QString& Label() const
	{
		return m_Label;
	}

	void Paint(QPainter& p, int tabHeight, int tabWidth, bool isCurrentTab, bool isMouseOverControl);

	inline bool SetTextColor(const QColor& color)
	{
		if(m_TextColor == color)
			return false;

		m_TextColor = color;
		return true;
	}

	// 0=none, 1=close control, 2=window control
	inline bool SetShowControl(int showControl)
	{
		if(m_ShowControl == showControl)
			return false;

		m_ShowControl = showControl;
		return true;
	}

	inline void SetToolTip(const QString& toolTip)
	{
		m_ToolTip = toolTip;
	}

	inline int ShowControl() const
	{
		return m_ShowControl;
	}

	QSize SizeHint(int tabHeight) const;

	inline const QColor& TextColor() const
	{
		return m_TextColor;
	}

	inline const QString& ToolTip() const
	{
		return m_ToolTip;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuTabColumn : public QWidget
{
	Q_OBJECT;
private:
	int									m_CurrentTabIndex;		// Index of current tab.
	int									m_DragTabIndex;			// Index of tab that is being dragged, or -1 if none.
	bool								m_DragTabsEnabled;		// True if ok to drag tabs to change order.
	int									m_FirstVisibleTab;		// Tab at the top of the visible list.
	int									m_MouseOverControlIndex;	// Index of tab that the mouse is over the control button.
	int									m_TabHeight;			// Height of each tab including 1 pixel margin top&bottom.
	QList<AuTabItem>					m_Tabs;					// Tab items.

public:
	AuTabColumn(QWidget* parent=0);
	virtual ~AuTabColumn();

	int AddTab(const QIcon& icon);

	int AddTab(const QIcon& icon, const QString& label);

	int AddTab(const QString& label);

	inline int Count() const
	{
		return m_Tabs.count();
	}

	inline int CurrentIndex() const
	{
		return m_CurrentTabIndex;
	}

	inline void SetDragTabsEnabled(bool draggingEnabled)
	{
		m_DragTabsEnabled = draggingEnabled;
	}

	int InsertTab(int index, const QIcon& icon);

	int InsertTab(int index, const QIcon& icon, const QString& label);

	int InsertTab(int index, const QString& label);

	virtual void paintEvent(QPaintEvent* pe);

	inline void SetTabHeight(int tabHeight)
	{
		Q_ASSERT(tabHeight > 2);
		m_TabHeight = tabHeight;
	}

	void SetTabShowControl(int tabIndex, int showControl);

	void SetTabTextColor(int tabIndex, const QColor& color);

	void SetTabToolTip(int tabIndex, const QString& toolTip);

	virtual QSize sizeHint() const;

	int TabAt(const QPoint& pos) const;

	int TabShowControl(int tabIndex) const;

	QString TabText(int tabIndex) const;

	QColor TabTextColor(int tabIndex) const;

public slots:
	void SetCurrentIndex(int tabIndex);
	void RemoveTab(int tabIndex);

protected:
	virtual void keyPressEvent(QKeyEvent *e);
	virtual void mouseDoubleClickEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void wheelEvent(QWheelEvent *e);

signals:
	void CurrentChanged(int currentTabIndex);
	void TabCloseRequested(int tabIndex);
	void TabDoubleClicked(int tabIndex);
	void TabMoved(int fromIndex, int toIndex);
	void TabWindowRequested(int tabIndex);
};

#endif // AuTabColumn_H
