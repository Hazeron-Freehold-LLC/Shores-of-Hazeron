//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubioskills.cpp,v 1.4 2022/02/19 18:08:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubioskills.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include <QHeaderView>
#include <QTimer>
#include <QVBoxLayout>

AuBioSkillsTable::AuBioSkillsTable(QWidget* parent) : QTableWidget(0,2, parent)
{
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setShowGrid(false);

	QStringList headerLabels;
	headerLabels << tr("Skill");
	headerLabels << tr("Level");
	setHorizontalHeaderLabels(headerLabels);

	setSortingEnabled(true);
	sortByColumn(0, Qt::AscendingOrder);

	QHeaderView* hhdr = horizontalHeader();
	hhdr->setSectionResizeMode(0, QHeaderView::Stretch);
	hhdr->setSectionResizeMode(1, QHeaderView::Interactive);

	verticalHeader()->hide();
	verticalHeader()->setDefaultSectionSize(18);
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
}

void AuBioSkillsTable::mousePressEvent(QMouseEvent* event)
{
	QTableWidget::mousePressEvent(event);
	m_MousePressItem = itemAt(event->pos());
}

void AuBioSkillsTable::mouseReleaseEvent(QMouseEvent* event)
{
	QTableWidget::mouseReleaseEvent(event);

	if(event->button() == Qt::RightButton)
	{
		QTableWidgetItem* item = itemAt(event->pos());
		if(item && item == m_MousePressItem)
			emit ItemRightClicked(item);
	}
}

void AuBioSkillsTable::UpdateContents(const AuSkillSet& skills)
{
	// Refresh the contents of the table widget.
	if(m_Contents == skills)
		return;
	m_Contents = skills;
	clearContents();
	while(rowCount())
		removeRow(0);

	setSortingEnabled(false);
	for(QHash<AuSkill::SKILL, AuSkill>::const_iterator si = skills.Skills().cbegin(); si != skills.Skills().cend(); si++)
	{
		insertRow(0);

		QIcon plico((*si).IconName());

		// Skill Name
		int col=0;
		QTableWidgetItem* twname = new QTableWidgetItem(plico, (*si).Name());
		twname->setData(Qt::UserRole, (*si).Skill());
		setItem(0,col++, twname);

		// Experience
		QString xp = QString("%L1").arg((*si).Experience());
		QTableWidgetItem* twxp = new QTableWidgetItem(xp);
		twxp->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
		twxp->setData(Qt::UserRole, (*si).Skill());
		setItem(0,col++, twxp);
	}
	setSortingEnabled(true);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBioSkills::AuBioSkills(QWidget *parent) : QWidget(parent)
{
	m_timer = 0;

	m_Skills = new AuBioSkillsTable(this);

	// Layout the widgets.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_Skills);
	setLayout(lay);
}

AuBioSkills::~AuBioSkills()
{
}

QSize AuBioSkills::sizeHint() const
{
	return QSize(400,300);
}

void AuBioSkills::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(1000);
}

void AuBioSkills::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	// Get the scene atom so we can show their information.
	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(!person)
	{
		view->UnlockSceneAtom();
		return;
	}

	// Skills
	AuSkillSet skillSet;
	const AuSkillSet& skills = person->LockSkillsForRead();
	skillSet = skills;
	person->UnlockSkills();

	view->UnlockSceneAtom();

	m_Skills->blockSignals(true);
	m_Skills->UpdateContents(skillSet);
	m_Skills->blockSignals(false);
}

