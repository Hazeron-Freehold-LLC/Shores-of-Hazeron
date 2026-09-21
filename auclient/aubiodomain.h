//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubiodomain.h,v 1.6 2022/02/19 17:50:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBioDomain_H
#define AuBioDomain_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "audomain.h"
#include "auglobal.h"
#include "aujoboffer.h"
#include "auskill.h"
#include <QPaintEvent>
#include <QTableWidget>

class AuOpenStoryView;
class AuPushButtonValue;
class QLabel;
class QPushButton;
class QTextEdit;
class QTimer;
class QTreeView;

class AUCLIENT_DLL AuBioDomainTable : public QTableWidget
{
	Q_OBJECT;
	AuDomain							m_Contents;
	QTableWidgetItem*					m_MousePressItem;		// Item under the mouse press.

public:
	AuBioDomainTable(QWidget* parent=0);
	virtual ~AuBioDomainTable()
	{
	}

	void UpdateContents(const AuDomain& domain);

signals:
	void ItemRightClicked(QTableWidgetItem*);

protected:
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuBioDomain : public QWidget
{
	Q_OBJECT;
private:
	QTimer*								m_timer;

	// Widgets
	QLabel*								m_Counts;
	QLabel*								m_HomeCitySys;
	QLabel*								m_HomeCitySysIcon;
	AuBioDomainTable*					m_Domain;

public:
	AuBioDomain(QWidget *parent=0);
	~AuBioDomain();

	virtual QSize sizeHint() const;

protected:
	virtual void showEvent(QShowEvent* event);

protected slots:
	void SystemRightClicked(QTableWidgetItem* item);
	void UpdateContents();
};

#endif // AuBioDomain_H
