//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubioskills.h,v 1.3 2022/02/19 17:51:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBioSkills_H
#define AuBioSkills_H

#include "auclientlib.h"
#include "audockwidget.h"
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

class AUCLIENT_DLL AuBioSkillsTable : public QTableWidget
{
	Q_OBJECT;
	AuSkillSet							m_Contents;
	QTableWidgetItem*					m_MousePressItem;		// Item under the mouse press.

public:
	AuBioSkillsTable(QWidget* parent=0);
	virtual ~AuBioSkillsTable()
	{
	}

	void UpdateContents(const AuSkillSet& skills);

signals:
	void ItemRightClicked(QTableWidgetItem*);

protected:
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuBioSkills : public QWidget
{
	Q_OBJECT;
private:
	QTimer*								m_timer;

	// Widgets
	AuBioSkillsTable*					m_Skills;

public:
	AuBioSkills(QWidget *parent=0);
	~AuBioSkills();

	virtual QSize sizeHint() const;

protected:
	virtual void showEvent(QShowEvent* event);

protected slots:
	void UpdateContents();
};

#endif // AuBioSkills_H
