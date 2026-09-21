//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovannouncements.cpp,v 1.5 2022/05/04 21:25:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augovannouncements.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auview.h"
#include "auglobal.h"
#include "daperson.h"
#include "dgdata.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QTextBrowser>
#include <QTextEdit>
#include <QVBoxLayout>

void AuGovAnnouncementsDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleGovernAnnouncementsUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGovAnnouncements::AuGovAnnouncements(QWidget* parent) : QWidget(parent)
{
	m_DataValid = false;

	m_Announcements = new QTextBrowser();
	m_Announcements->setReadOnly(true);
	m_Announcements->setOpenExternalLinks(true);

	m_EmpireFlag = new QLabel(this);
	m_EmpireFlag->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);

	m_EmpireName = new QLabel(this);
	m_EmpireName->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);

	m_Editor = new QTextEdit();
	m_Editor->setToolTip(tr("<html><b>Editor</b><br>"
				"Edit the announcements in this window. "
				"The formatted result is shown in the other window. "
				"A limited subset of HTML formatting is supported. "
				"This editor control is only visible if you are the emperor or vice emperor."
				"</html>"));
	connect(m_Editor, SIGNAL(textChanged()), SLOT(EditorTextChanged()));

	m_ApplyChanges = new QPushButton(tr("Publish Changes"));
	connect(m_ApplyChanges, SIGNAL(clicked()), SLOT(ApplyChanges()));
	m_ApplyChanges->setToolTip(tr("<html>"
				"<p>Publishes announcements to the citizens of the empire.</p>"
				"<p>Only the emperor or vice-emperor can publish announcements.</p>"
				"</html>"));

	m_Clear = new QPushButton(tr("Clear Announcements"));
	connect(m_Clear, SIGNAL(clicked()), SLOT(ClearAnnouncements()));
	m_Clear->setToolTip(tr("<html>"
				"<p>Clears the current announcements.</p>"
				"<p>Only the emperor or vice-emperor can clear announcements.</p>"
				"</html>"));

	QHBoxLayout* butlay = new QHBoxLayout;
	butlay->addWidget(m_Clear);
	butlay->addWidget(m_ApplyChanges);
	butlay->setSpacing(2);
	butlay->setMargin(0);

	QVBoxLayout* editorlay = new QVBoxLayout;
	editorlay->addWidget(m_Editor);
	editorlay->addLayout(butlay);
	editorlay->setSpacing(2);
	editorlay->setMargin(0);

	QWidget* editorwidget = new QWidget;
	editorwidget->setLayout(editorlay);

	// Assemble the widgets into a splitter.
	m_Splitter = new QSplitter;
	m_Splitter->setOrientation(Qt::Vertical);
	m_Splitter->addWidget(m_Announcements);
	m_Splitter->addWidget(editorwidget);
	m_Splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	QVBoxLayout* titlay = new QVBoxLayout;
	titlay->addWidget(m_EmpireFlag);
	titlay->addWidget(m_EmpireName);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(titlay);
	lay->addWidget(m_Splitter);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	connect(this, SIGNAL(AnnouncementsChanged(QString)), auApp, SLOT(GovSetAnnouncements(QString)));
}

void AuGovAnnouncements::UpdateEmpireRecord()
{
	// If this widget is not visible, mark the data as invalid.
	// setVisible calls us to really update the page when we become visible.
	if(!isVisible())
	{
		//qDebug("AuGovAnnouncements::UpdateEmpireRecord page is not visible. Postponing update until page becomes visible.");
		m_DataValid = false;	// Our trigger to update contents.
		return;
	}

	//qDebug("AuGovAnnouncements::UpdateEmpireRecord page is visible. Update contents.");
	// Get new empire record data
	DgEmpire emp;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
		emp.m_id = person->EmpireId();
	pView->UnlockSceneAtom();

	const DgData& gd = g_Au.LockDataForRead();
	gd.Empire(emp);
	bool canChange = gd.EmpireCitizenCanSetAnnouncements(person->EmpireId(), person->Id());
	g_Au.UnlockData();
	m_DataValid = true;

	m_EmpireFlag->setPixmap(QPixmap::fromImage(emp.m_flag));

	m_EmpireName->setText(tr("<html><big><big><b>%1</b></big></big></html>").arg(emp.Name()));

	QString announce = emp.Announcements();
	if(announce.isEmpty())
		announce = tr("<big><b>Announcements</b></big><br>\n<small><i>%2</i></small>\n<p>No announcements at this time.</p>")
			.arg(emp.CitizenNameWithTitle(emp.EmperorId()));

	m_Announcements->setHtml(announce);

	if(canChange)
	{
		m_ApplyChanges->show();
		m_Clear->show();
		m_Editor->show();
		m_Editor->setPlainText(announce);
	}
	else
	{
		m_ApplyChanges->hide();
		m_Clear->hide();
		m_Editor->hide();

		QList<int> sizes = m_Splitter->sizes();
		sizes[0] += sizes[1];
		sizes[1] = 0;
		m_Splitter->setSizes(sizes);
	}
}

void AuGovAnnouncements::ApplyChanges()
{
	QString announcements = m_Editor->toPlainText();
	emit AnnouncementsChanged(announcements);
}

void AuGovAnnouncements::ClearAnnouncements()
{
	emit AnnouncementsChanged("");
}

void AuGovAnnouncements::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	if(!m_DataValid)
		UpdateEmpireRecord();
}

void AuGovAnnouncements::EditorTextChanged()
{
	QString text = m_Editor->toPlainText();
	m_Announcements->setHtml(text);
}

QSize AuGovAnnouncements::sizeHint() const
{
	return QSize(500,800);
}
