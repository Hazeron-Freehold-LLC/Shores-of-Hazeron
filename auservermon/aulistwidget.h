//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulistwidget.h,v 1.4 2022/02/19 18:40:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuListWidget_H
#define AuListWidget_H

#include <QListWidget>
#include <QListWidgetItem>

class QMouseEvent;

class AuListWidget : public QListWidget
{
	Q_OBJECT;
private:
	Qt::MouseButton		m_LastButtonDown;
	Qt::MouseButton		m_LastButtonUp;

public:
	AuListWidget(QWidget* parent=0);
	virtual ~AuListWidget();

	Qt::MouseButton LastButtonDown() const
	{
		return m_LastButtonDown;
	}

	Qt::MouseButton LastButtonUp() const
	{
		return m_LastButtonUp;
	}

protected:
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuListWidgetItem : public QListWidgetItem
{
public:
	inline AuListWidgetItem(const QIcon& icon, const QString& text, QListWidget* parent = 0, int type = Type) :
		QListWidgetItem(icon, text, parent, type)
	{
	}

	inline void SetSortOrder(int sortOrder)
	{
		setData(Qt::UserRole+100, sortOrder);
	}

	virtual bool operator<(const QListWidgetItem& other) const
	{
		return data(Qt::UserRole+100).toInt() < other.data(Qt::UserRole+100).toInt();
	}
};

#endif // AuListWidget_H
