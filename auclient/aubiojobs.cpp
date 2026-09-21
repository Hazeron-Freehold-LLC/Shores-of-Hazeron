//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubiojobs.cpp,v 1.7 2022/04/08 16:23:20 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubiojobs.h"
//
#include "auapp.h"
#include "aucompileroutput.h"
#include "auglobal.h"
#include "aujobsquerydialog.h"
#include "auframe.h"
#include "auopenstoryview.h"
#include "autoolbutton.h"
#include "auview.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dgdata.h"
#include <QFileDialog>
#include <QGridLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QTimer>

AuBioJobs::AuBioJobs(QWidget *parent) : QWidget(parent)
{
	m_timer = 0;

	m_SelectedJob = 0;

	m_OpenStoryModel.insertColumns(0,1);
	m_OpenStoryModel.setHeaderData(0, Qt::Horizontal, tr("Stories and Jobs"));

	// Create the widgets.

	m_OpenJobs = new AuOpenStoryView(this);
	m_OpenJobs->setAlternatingRowColors(true);
	m_OpenJobs->setAllColumnsShowFocus(true);
	m_OpenJobs->setModel(&m_OpenStoryModel);
	m_OpenJobs->setRootIsDecorated(false);
	m_OpenJobs->setSortingEnabled(true);
	m_OpenJobs->sortByColumn(0, Qt::AscendingOrder);
	m_OpenJobs->setToolTip(tr("<html><b>Stories and Jobs</b><br>"
				"List of stories and jobs that are currently in progress.</html>"));
	connect(m_OpenJobs, SIGNAL(ItemSelected(const QModelIndex&)), SLOT(OpenStorySelected(const QModelIndex&)));

	m_StoryHint = new AuIconButton(QIcon(":data/StoryHint.png"), this);
	m_StoryHint->setToolTip(tr("<html><b>Hint</b><br>"
				"Requests a hint about how to proceed in the selected job. "
				"This button is disabled if the selected job does not provide hints."
				"</html>"));
	connect(m_StoryHint, SIGNAL(clicked()), SLOT(StoryHint()));

	AuIconButton* pbclose = new AuIconButton(QIcon(":data/StoryClose.png"), this);
	pbclose->setToolTip(tr("<html><b>Drop...</b>"
				"<p>"
				"Drops the selected job. "
				"The selected job is removed from the list of active jobs."
				"</p>"
				"<p>"
				"Most stories will restart when dropped. "
				"The amount of time that must pass depends on the story. "
				"</p>"
				"</html>"));
	connect(pbclose, SIGNAL(clicked()), SLOT(CloseStory()));

	AuIconButton* resetStory = 0;
	AuIconButton* startStory = 0;
	AuIconButton* stopStory = 0;
	if(g_Au.IsSceneSolo())
	{
		resetStory = new AuIconButton(QIcon(":data/StoryReset.png"), this);
		resetStory->setToolTip(tr("<html><b>Reset Story...</b>"
					"<p>"
					"Resets a story as if you never started it. "
					"</p>"
					"</html>"));
		connect(resetStory, SIGNAL(clicked()), SLOT(ResetStory()));

		startStory = new AuIconButton(QIcon(":data/StoryOpen.png"), this);
		startStory->setToolTip(tr("<html><b>Load Custom Story...</b>"
					"<p>"
					"Loads a custom story script from a file. "
					"Compiles the script and reports any errors. "
					"</p>"
					"<p>"
					"Attaches the script to the avatar as a custom story, if it compiles successfully. "
					"An associated story job will start on the avatar, when any autostart condition is true. "
					"</p>"
					"</html>"));
		connect(startStory, SIGNAL(clicked()), SLOT(SubmitStory()));

		stopStory = new AuIconButton(QIcon(":data/StoryClear.png"), this);
		stopStory->setToolTip(tr("<html><b>Stop Custom Story...</b>"
					"<p>"
					"Clears the custom story script from the avatar. "
					"Any associated story job is canceled. "
					"</p>"
					"</html>"));
		connect(stopStory, SIGNAL(clicked()), SLOT(StopStory()));
	}

	const DgData& gd = g_Au.LockDataForRead();
	bool isProgrammer = gd.ClientPlayerIsProgrammer();
	g_Au.UnlockData();
	AuIconButton* pbquery;
	if(isProgrammer)
	{
		pbquery = new AuIconButton(QIcon(":data/Jobs.png"), this);
		pbquery->setToolTip(tr("<html><b>Request Jobs</b><br>"
					"<b>Agent only.</b>"
					"</html>"));
		connect(pbquery, SIGNAL(clicked()), SLOT(RequestWork()));
	}
	else
	{
		pbquery = 0;
	}

	m_JobDesc = new QTextEdit(this);
	m_JobDesc->setReadOnly(true);

	// Layout the widgets.
	QVBoxLayout* buttonlay = new QVBoxLayout;
	buttonlay->addWidget(m_StoryHint);
	buttonlay->addStretch();
	buttonlay->addWidget(pbclose);
	if(g_Au.IsSceneSolo())
	{
		buttonlay->addWidget(resetStory);
		buttonlay->addStretch();
		buttonlay->addWidget(startStory);
		buttonlay->addWidget(stopStory);
	}
	if(pbquery)
	{
		buttonlay->addStretch();
		buttonlay->addWidget(pbquery);
	}
	buttonlay->setSpacing(2);
	buttonlay->setMargin(0);

	QVBoxLayout* jobslay = new QVBoxLayout;
	jobslay->addWidget(m_OpenJobs, 2);
	jobslay->addWidget(m_JobDesc, 3);
	jobslay->setSpacing(2);
	jobslay->setMargin(0);

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addLayout(buttonlay);
	lay->addLayout(jobslay);
	lay->setMargin(4);
	lay->setSpacing(2);
	setLayout(lay);
}

AuBioJobs::~AuBioJobs()
{
	if(m_SelectedJob)
	{
		delete m_SelectedJob;
		m_SelectedJob = 0;
	}
}

QSize AuBioJobs::sizeHint() const
{
	return QSize(400,300);
}

void AuBioJobs::showEvent(QShowEvent* event)
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

void AuBioJobs::UpdateContents()
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

	// Found the person in the local root data.
	bool anyAdded = false;

	// Get a list of the titleBriefs that are already in the tree control, that is, the title and brief concatenated.
	QStringList titleBriefs;
	for(int ir=0; ir<m_OpenStoryModel.rowCount(); ir++)
	{
		QModelIndex itemIdx = m_OpenStoryModel.index(ir,0);
		QString titleBrief = itemIdx.data(Qt::UserRole+1).toString();		// data is title+brief
		titleBriefs.append(titleBrief);
		//qDebug("AuJobsDialog::TimerUpdate title %s", (const char*)title.toAscii());
	}
	bool wasEmpty = titleBriefs.isEmpty();
	//qDebug("AuJobsDialog::TimerUpdate %d titles in model, person "
			//"open titles %d authors %d",
			//titleBriefs.count(), person->m_OpenTitles.count(),
			//person->m_OpenAuthors.count());

	// List titles the person has open that are not already in the tree.
	// titles that belong in the tree will be removed from the 'titleBriefs' list so we know which to remove when we're done.
	const AuJobList& jobs = person->LockJobs();
	for(AuJobList::const_iterator ji = jobs.constBegin(); ji != jobs.constEnd(); ji++)
	{
		//qDebug("AuJobsDialog::TimerUpdate open title %s", (const char*)title.toAscii());
		int tbi = titleBriefs.indexOf((*ji)->Title()+(*ji)->Brief());	// titleBrief index
		if(tbi >= 0)
		{
			// titles listed in the control already contains the open title.
			// remove it from the titleBriefs list so we know which titles don't belong in the control when we're done.
			titleBriefs.removeAt(tbi);
		}
		else
		{
			m_OpenStoryModel.insertRow(0);

			// Pack the job into a byte array so it can be stored on the job item.
			QByteArray bajob;
			QDataStream out(&bajob, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)(*ji)->Type();
			out << *(*ji);

			QModelIndex itemIdx = m_OpenStoryModel.index(0,0);
			m_OpenStoryModel.setData(itemIdx, (*ji)->Title());
			m_OpenStoryModel.setData(itemIdx, (*ji)->Title()+(*ji)->Brief(), Qt::UserRole+1);
			m_OpenStoryModel.setData(itemIdx, bajob, Qt::UserRole);			// Job stored on the list item.
			m_OpenStoryModel.item(0,0)->setIcon(QIcon((*ji)->IconName()));
			m_OpenStoryModel.item(0,0)->setToolTip((*ji)->Title());

			anyAdded = true;
		}
	}
	person->UnlockJobs();

	//qDebug("AuJobsDialog::TimerUpdate %d titles to remove", titleBriefs.count());

	// Now any titles remaining in the 'titleBriefs' list must be removed from the tree control.
	for(QStringList::const_iterator ti = titleBriefs.constBegin(); ti != titleBriefs.constEnd(); ti++)
	{
		for(int ir=0; ir<m_OpenStoryModel.rowCount(); ir++)
		{
			QModelIndex itemIdx = m_OpenStoryModel.index(ir,0);
			QString titleBrief = itemIdx.data(Qt::UserRole+1).toString();	// data is title+brief
			if(titleBrief == *ti)
			{
				// Remove the row from the control's data model.
				m_OpenStoryModel.removeRow(ir);
				break;
			}
		}
	}

	if(anyAdded && wasEmpty)
	{
		m_OpenJobs->resizeColumnToContents(0);
		m_OpenJobs->sortByColumn(0, Qt::AscendingOrder);
	}

	view->UnlockSceneAtom();
}

void AuBioJobs::CloseStory()
{
	//qDebug("CloseStory");
	if(m_SelectedJob)
	{
		if(m_SelectedJob->Type() == AuJob::Story)
		{
			QString smsg = tr("<html>"
					"<p>"
					"Most stories will restart at the beginning when dropped. "
					"The amount of time that must pass before restarting depends on the story. "
					"</p>"
					"<p>"
					"Are you sure you want to drop the story?"
					"</p>"
					"</html>");
			QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Question, QMessageBox::Yes, QMessageBox::Cancel,
					QMessageBox::NoButton, this);
			int result = auApp->Frame()->ExecMessageBox(mb);
			if(result != QMessageBox::Yes)
				return;
		}

		// Put the close request into a job offer object so it can be emitted as a signal.
		AuJobOffer jobOffer("", 0, AuPoint(0,0), GALAXY_Count, m_SelectedJob);
		emit CloseStoryRequested(jobOffer);
	}
}

void AuBioJobs::OpenStorySelected(const QModelIndex& sel)
{
	if(m_SelectedJob)
	{
		delete m_SelectedJob;
		m_SelectedJob = 0;
	}

	//qDebug("OpenStorySelected row %d", sel.row());
	if(sel.isValid())
	{
		QModelIndex col0 = sel.sibling(sel.row(), 0);

		// Get a copy of the job offer from the selected item.
		QByteArray bajob = col0.data(Qt::UserRole).toByteArray();
		QDataStream in(bajob);
		in.setVersion(QDataStream::Qt_4_1);
		quint8 jobType;
		in >> jobType;
		m_SelectedJob = AuJob::CreateJob((AuJob::TYPE)jobType);
		if(m_SelectedJob)
			in >> *m_SelectedJob;

		const AuJobStory* jobStory = dynamic_cast<const AuJobStory*>(m_SelectedJob);
		if(jobStory)
			m_StoryHint->setEnabled(true);
		else
			m_StoryHint->setEnabled(false);

		if(m_SelectedJob)
		{
			QString jobDesc = tr("<html><b>%1</b><br>%2</html>")
				.arg(m_SelectedJob->Title())
				.arg(m_SelectedJob->Brief());
			m_JobDesc->setText(jobDesc);
		}
		else
			m_JobDesc->setText("");
	}
	else
		m_JobDesc->setText("");
}

void AuBioJobs::StoryHint()
{
	const AuJobStory* jobStory = dynamic_cast<const AuJobStory*>(m_SelectedJob);
	if(jobStory)
		emit StoryHintRequested(jobStory->StoryInstance());
}

void AuBioJobs::RequestWork()
{
	AuView* view = auApp->Frame()->View();
	if(!view)
		return;

	AuId personId;
	AuPoint sectorLoc;
	GALAXY galaxy = GALAXY_Count;
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		personId = person->Id();
		const DaSector* sector = person->FindParentSector();
		if(sector)
			sectorLoc = sector->Location();
		galaxy = person->Galaxy();
	}
	view->UnlockSceneAtom();

	AuFrame* pFrame = auApp->Frame();
	AuJobsQueryDialog jobsQuery(personId, sectorLoc, galaxy, pFrame);
	auApp->Frame()->ExecDialog(jobsQuery);
}

void AuBioJobs::ResetStory()
{
	// Get default avatar id and name from the scene atom's mount or the scene atom.
	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();
	AuId avatarId = view->SceneAtomId();
	if(avatarId.IsNull())
		return;

	// Get the default script id to reset.
	quint32 scriptId = g_Au.Settings().readNumEntry("Widget/BioJobs/ResetStory", 0);

	// Query the user for the script id.
	QStringList stories;
	stories << "1. Veil of Targoss";
	stories << "2. Space Program";
	stories << "3. Getting Started";
	stories << "4. Rocket Training";
	stories << "5. Caroler.";
	stories << "6. Relic";
	stories << "7. Dangerous Exchange";
	stories << "8. Beacon";
	stories << "9. Reach for the Stars";
	stories << "10. Undertow";
	int scriptIndex = scriptId - 1;
	bool ok = false;
	QString script = QInputDialog::getItem(this, tr("Reset Story"),
			tr("<html>Select the story to reset. "
				"The avatar will no longer have any record of starting or finishing the story.</html>"), stories,
			scriptIndex, false, &ok);
	// Bail if they canceled.
	if(!ok)
		return;

	// Bail if they gave us a bogus id.
	scriptId = stories.indexOf(script) + 1;
	if(!scriptId)
		return;

	// Save the new default script id to reset.
	g_Au.Settings().writeEntry("Widget/BioJobs/ResetStory", (int)scriptId);

	auApp->StoryReset(avatarId, scriptId);
}

void AuBioJobs::SubmitStory()
{
	// Get avatar id and name.
	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();
	AuId avatarId = view->SceneAtomId();
	QString avatarName = view->SceneAtomName();
	if(avatarId.IsNull())
		return;

	// Load last known file folder.
	QString lastFile = g_Au.Settings().readEntry("Widget/BioJobs/ScriptFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();
	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QString file = QFileDialog::getOpenFileName(this, QObject::tr("Select Script File"), lastFile, "", 0,
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(file.isEmpty())
		return;

	// Store last known file folder.
	g_Au.Settings().writeEntry("Widget/BioJobs/ScriptFile", file);

	AuScript script;	// Construct with 0 script id.
	script.Load(file);	// Load and compile the file.

	AuCompilerOutput dlg(file, script);
	if(auApp->Frame()->ExecDialog(dlg) != QDialog::Accepted || script.CompileResult() != 0)
		return;

	// Attach the story script to the avatar.
	auApp->StorySubmitScript(avatarId, script);
}

void AuBioJobs::StopStory()
{
	QString smsg = tr("<html>"
			"<p>"
			"Are you sure you want to stop the custom story?"
			"</p>"
			"</html>");
	QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Question, QMessageBox::Yes, QMessageBox::Cancel,
			QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Yes)
		return;

	// Get avatar id and name.
	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();
	AuId avatarId = view->SceneAtomId();
	QString avatarName = view->SceneAtomName();
	if(avatarId.IsNull())
		return;

	auApp->StoryCancelSubmission(avatarId);
}

