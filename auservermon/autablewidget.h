//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autablewidget.h,v 1.3 2022/02/19 18:40:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuTableWidget_H
#define AuTableWidget_H

#include <QTableWidget>

class QMouseEvent;
class QSettings;

class AuTableWidget : public QTableWidget
{
	Q_OBJECT;
private:
	Qt::MouseButton		m_LastButtonDown;
	Qt::MouseButton		m_LastButtonUp;

public:
	AuTableWidget(QWidget* parent=0);
	virtual ~AuTableWidget();

	Qt::MouseButton LastButtonDown() const
	{
		return m_LastButtonDown;
	}

	Qt::MouseButton LastButtonUp() const
	{
		return m_LastButtonUp;
	}

	void RestoreColumnPositions(const QString& key, QSettings& settings);

	void SaveColumnPositions(const QString& key, QSettings& settings) const;

protected:
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
};

#endif // AuTableWidget_H
