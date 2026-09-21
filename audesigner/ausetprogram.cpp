//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetprogram.cpp,v 1.28 2022/03/14 16:36:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausetprogram.h"
//
#include "auapp.h"
#include "auabout.h"
#include "aucheckbox.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "ausystemaccesscode.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>

AuSetProgram::AuSetProgram(QWidget *parent) : QWidget(parent)
{
	m_IconFrame = 0;
	m_timer = 0;
	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	g_Au.UnlockData();
#if !defined(QT_NO_DEBUG)
	isAgent = true;	// So I can get switch codes before logging in, when running the debug version.
#endif

	QPushButton* pbAbout = new QPushButton(QIcon(auApp->Icon32()), tr("About %1...").arg(g_Au.ApplicationName()), this);
	connect(pbAbout, SIGNAL(clicked()), SLOT(AboutSoH()));

	m_WorkerThreadsIcon = new QLabel(this);
	m_WorkerThreadsIcon->setToolTip(tr("<html>"
				"This indicator appears at the top right corner of the scene window whenever a worker thread is busy."
				"</html>"));

	QLabel* lbworkerthreads = new QLabel(tr("Worker Threads"));

	QSlider* slworkerthreads = new QSlider(Qt::Horizontal, this);
	slworkerthreads->setRange(0, QThread::idealThreadCount());
	slworkerthreads->setSingleStep(1);
	slworkerthreads->setPageStep(1);
	slworkerthreads->setTickPosition(QSlider::TicksBelow);
	slworkerthreads->setValue(g_Au.WorkerThreadCountSetting());
	connect(slworkerthreads, SIGNAL(valueChanged(int)), SLOT(SetWorkerThreads(int)));
	slworkerthreads->setToolTip(tr("<html>"
				"<p>"
				"Threads allow a program to run more than one process at a time. "
				"</p>"
				"<p>"
				"Each thread uses a CPU core when it is running. "
				"Threads share the CPU with the main GUI thread and the network threads. "
				"Other programs running on the computer share the same CPU cores. "
				"</p>"
				"<p>"
				"<b>Changes to this setting, after entering the game, require the program to be restarted.</b> "
				"</p>"
				"</html>"));

	QString threads;
	if(g_Au.WorkerThreadCountSetting())
		threads = tr("%1 threads").arg(g_Au.WorkerThreadCountSetting());
	else
	{
		int idealThreadCount = QThread::idealThreadCount();
		if(idealThreadCount > 2)
			idealThreadCount -= 2;
		else
			idealThreadCount = 1;
		if(idealThreadCount > 1)
			threads = tr("Auto (%1 threads)").arg(idealThreadCount);
		else
			threads = tr("Auto (1 thread)");
	}
	m_WorkerThreadsLabel = new QLabel(threads, this);

	threads = tr("%L1 Atom Managers (2 threads each)").arg(g_Au.AtomManagerCount());
	m_AtomManagersLabel = new QLabel(threads, this);
	m_AtomManagersLabel->setToolTip(tr("<html>"
				"Atom managers perform atomic tasks in the game universe. "
				"This includes movement due to physics and all other activities performed by the atoms of the universe. "
				"</html>"));
	m_AtomManagersLabel->setVisible(g_Au.AtomManagerCount());

	threads = tr("%L1 Worker Thread Pool").arg(g_Au.WorkerThreadPoolCount());
	m_WorkerPoolLabel = new QLabel(threads, this);
	m_WorkerPoolLabel->setToolTip(tr("<html>"
				"Worker threads make 3D models for the render scene. "
				"</html>"));

	QLabel* picFolderLabel = new QLabel(tr("Screen Grab Folder"), this);

	m_PictureDirButton = new QPushButton(g_Au.PictureDir(), this);
	m_PictureDirButton->setToolTip(tr("<html>Chooses a folder location to store screen grabs. "
				"Screen grabs are pictures of the window contents that you capture. "
				"The default key to grab screen shots is Alt+P. "
				"See Scene Controls for the current key binding.</html>"));
	connect(m_PictureDirButton, SIGNAL(clicked()), SLOT(SetPictureDir()));


	QLabel* tempFolderLabel = new QLabel(tr("Temporary Files Folder"), this);

	m_CacheDirButton = new QPushButton(g_Au.CacheApp(), this);
	m_CacheDirButton->setToolTip(tr("<html>Chooses a folder location to store temporary cache files.</html>"));
	connect(m_CacheDirButton, SIGNAL(clicked()), SLOT(SetCacheDir()));

	QPushButton* clearAllFiles = new QPushButton("Clear Cached Files...", this);
	clearAllFiles->setToolTip(tr("<html>Clears all cached files.<p>"
				"Cached files save a lot of time and reduce significant load from your computer. "
				"It is rarely necessary to delete cached files.<p>"
				"Suggested reasons to clear all cached files:<br>"
				"%1 You will not be playing <i>%2</i> on this computer any more.<br>"
				"%1 You want to reclaim disk space after playing <i>%2</i> for a long time.<br>"
				"%1 <i>%2</i> is acting weird and you don't know what else to do."
				"</html>").arg(g_Au.MiddleDotSymbol()).arg(g_Au.ApplicationName()));
	connect(clearAllFiles, SIGNAL(clicked()), SLOT(ClearAllFiles()));

	QLabel* gameSaveFolderLabel = 0;
	m_GameSaveDirButton = 0;
	QPushButton* clearSolo = 0;
	if(auApp->IsHazeronStarship())
	{
		gameSaveFolderLabel = new QLabel(tr("Game Save Folder"), this);

		m_GameSaveDirButton = new QPushButton(g_Au.GameSaveDir(), this);
		m_GameSaveDirButton->setToolTip(tr("<html>Chooses a folder location to store saved game files. "
					"This includes the game in progress and all checkpoints.</html>"));
		connect(m_GameSaveDirButton, SIGNAL(clicked()), SLOT(SetGameSaveDir()));

		if(auApp->IsHazeronStarship())
		{
			clearSolo = new QPushButton("Clear Saved Games...", this);
			clearSolo->setToolTip(tr("<html> "
						"Clears all saved game files, including the game in progress. "
						"</html>"));
			connect(clearSolo, SIGNAL(clicked()), SLOT(ClearSavedGames()));
		}
	}

	QPushButton* clearLogs = new QPushButton(tr("Clear Comm Channel Transcripts..."));
	clearLogs->setToolTip(tr("<html>Deletes all communication channel transcript files that have accumulated in the temporary "
				"files folder."
				"</html>"));
	connect(clearLogs, SIGNAL(clicked()), SLOT(ClearLogs()));

	QPushButton* clearNetCache = new QPushButton("Clear Internet Cache...", this);
	clearNetCache->setToolTip(tr("<html>The network cache holds files downloaded from the Internet. "
				"For example, the images downloaded for picture frames. "
				"</html>"));
	connect(clearNetCache, SIGNAL(clicked()), SLOT(ClearNetCache()));

	// Debug switches
	QGroupBox* gbBits = new QGroupBox(tr("Switches"));
	{
		QLabel* lbwarn = new QLabel(this);
		lbwarn->setPixmap(QPixmap(":data/DangerSwitchRoom.png"));

		QGridLayout* slay = new QGridLayout;
		slay->setSpacing(2);
		slay->setMargin(12);

		// Build grid of check boxes.
		{
			QString randomTips[] =
			{
				tr("I'm feeling lucky."),
				tr("Whiiiiizzzzzz.........CLICK!"),
				tr("Execute random opcode."),
				tr("Checkmate"),
				tr("Come to papa!"),
				tr("Whoa! Not so fast."),
				tr("Click me!"),
				tr("Black"),
				tr("Brown"),
				tr("Red"),
				tr("Orange"),
				tr("Yellow"),
				tr("Green"),
				tr("Blue"),
				tr("Violet"),
				tr("Grey"),
				tr("White"),
				tr("Silver"),
				tr("Gold"),
				tr("If you smell something burning, turn this check box off."),
				tr("Shield face before checking this box."),
				tr("What if this check box won't turn back off again?"),
				tr("Wear a lead codpiece before checking this box."),
				tr("Ignite"),
				tr("Detonate"),
				tr("Self Destruct"),
				tr("Call Haxus"),
				tr("Initiate"),
				tr("Activate"),
				tr("Deactivate"),
				tr("Disable"),
				tr("Enable"),
				tr("Charge"),
				tr("Discharge"),
				tr("Engage"),
				tr("Disengage"),
				tr("Bypass"),
				tr("Interconnect"),
				tr("Short Circuit"),
				tr("Turbo"),
				tr("Probe"),
				tr("Analyze"),
				tr("Logic Analyzer"),
				tr("RAM Scope"),
				tr("Stealth Mode"),
				tr("Assimilate"),
				tr("Disintegrate"),
				tr("Corrode"),
				tr("Dissolve"),
				tr("Burn"),
				tr("Freeze"),
				tr("What does this button do?"),
				tr("Call 911"),
				tr("Order Pizza"),
			};
			const int randomTipCount = sizeof(randomTips)/sizeof(QString);

			QSet<int> usedTips;
			int bit=31;
			for(int iy=0; iy<4; iy++)
			{
				for(int ix=0; ix<8; ix++)
				{
					AuCheckBoxValue* cbit = new AuCheckBoxValue(tr("b%1").arg(bit), bit, this);
					m_Switches[bit] = cbit;

					// Select a random tool tip.
					QString toolTip;
					{
						int randomTip;
						do
						{
							randomTip = g_Au.Dice().Roll(1, randomTipCount, -1);
						}
						while(usedTips.contains(randomTip));
						usedTips.insert(randomTip);
						toolTip = randomTips[randomTip];
					}

					// Set the initial state of the check box before we connnect the signal.
					// Update BitChanged to respond to changes to these check boxes.
					switch(bit)
					{
					default:
						break;

					case 0:
						cbit->setCheckState((g_Au.m_LogMessages&LOG_ConnectDisconnect)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("When checked, a lot of information is logged into the soh.txt file, "
									"in the temporary files folder.<br>"
									"Creation of the log will likely slow game performance considerably.");
						break;

					case 1:
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00008000)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Log Atom Arrivals.");
						break;
					
					case 4:	// Show paths
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00020000)?Qt::Checked:Qt::Unchecked);
						if(isAgent)
							toolTip += QString("Show paths.<br>"
									"Access Code is <b>%1</b>. The code changes with each epochal day.")
								.arg(AuTime::Now().CurrentDaysSinceEpoch() * (bit + 1) % 999);
						break;			// No tool tip for this mystery switch.

					case 5:	// AuEther thorough change parent
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00040000)?Qt::Checked:Qt::Unchecked);
						if(isAgent)
							toolTip += QString("Etherium thorough change parent.<br>"
									"Access Code is <b>%1</b>. The code changes with each epochal day.")
								.arg(AuTime::Now().CurrentDaysSinceEpoch() * (bit + 1) % 999);
						break;

					case 6:	// AuEther thorough trash
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00080000)?Qt::Checked:Qt::Unchecked);
						if(isAgent)
							toolTip += QString("Etherium thorough trash.<br>"
									"Access Code is <b>%1</b>. The code changes with each epochal day.")
								.arg(AuTime::Now().CurrentDaysSinceEpoch() * (bit + 1) % 999);
						break;

					case 7:	// AuEther thorough remove
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00100000)?Qt::Checked:Qt::Unchecked);
						if(isAgent)
							toolTip += QString("Etherium thorough remove.<br>"
									"Access Code is <b>%1</b>. The code changes with each epochal day.")
								.arg(AuTime::Now().CurrentDaysSinceEpoch() * (bit + 1) % 999);
						break;

					case 8:				// Frame time on screen.
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00000002)?Qt::Checked:Qt::Unchecked);
						break;			// No tool tip for this mystery switch.

					case 9:				// Frame rate on screen.
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00000001)?Qt::Checked:Qt::Unchecked);
						break;			// No tool tip for this mystery switch.

					case 10:
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x01000000)?Qt::Checked:Qt::Unchecked);
						break;			// No tool tip for this mystery switch.

					case 11:
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x02000000)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Avatar");
						break;			// No tool tip for this mystery switch.

					case 12:
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x04000000)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Sight");
						break;			// No tool tip for this mystery switch.

					case 13:
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x08000000)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Items");
						break;			// No tool tip for this mystery switch.

					case 14:
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x10000000)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Rocks");
						break;			// No tool tip for this mystery switch.

					case 15:
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x20000000)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Geo");
						break;			// No tool tip for this mystery switch.

					case 16:			// Render cities
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00000004)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Cities");
						break;			// No tool tip for this mystery switch.

					case 17:			// Render spacecraft.
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00000010)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Spacecraft");
						break;			// No tool tip for this mystery switch.

					case 18:			// Render vehicles.
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00000020)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Vehicles");
						break;			// No tool tip for this mystery switch.

					case 19:
						// Render at worlds.
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00000040)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("At Worlds");
						break;			// No tool tip for this mystery switch.

					case 20:
						// Render near worlds.
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00000008)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Near Worlds");
						break;			// No tool tip for this mystery switch.

					case 21:
						// Render far worlds.
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00000080)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Far Worlds");
						break;			// No tool tip for this mystery switch.

					case 22:
						// Render atmosphere.
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00000100)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Atmosphere");
						break;			// No tool tip for this mystery switch.

					case 23:
						// Render flora.
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00000200)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Flora");
						break;			// No tool tip for this mystery switch.

					case 24:
						// Render seas.
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00000400)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Sea");
						break;			// No tool tip for this mystery switch.

					case 25:
						// Create world device objects
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00000800)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Create World OpenGL Objects");
						break;			// No tool tip for this mystery switch.

					case 26:
						// Render worlds
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00001000)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Render Worlds");
						break;			// No tool tip for this mystery switch.

					case 27:
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00002000)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Buildings");
						break;

					case 28:
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00004000)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("Building Design");
						break;

					case 29:
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x00010000)?Qt::Checked:Qt::Unchecked);
						break;

					case 30:
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x40000000)?Qt::Checked:Qt::Unchecked);
						break;

					case 31:
						cbit->setCheckState((g_Au.m_DebugSwitches & 0x80000000)?Qt::Checked:Qt::Unchecked);
						toolTip = tr("When checked, OpenGL state <i>GL_TEXTURE_CUBE_MAP_SEAMLESS</i> is disabled when drawing the "
								"skybox. "
								"The effect of this change is immediate; client restart is not required.");
						break;
					}

					cbit->setToolTip(QString("<html><b>b%1</b><br>%2</html>")
							.arg(bit)
							.arg(toolTip));

					connect(cbit, SIGNAL(StateChanged(int, int)), SLOT(BitChanged(int, int)));

					slay->addWidget(cbit, iy,ix);

					bit--;
				}
			}
		}
		slay->addWidget(lbwarn, 0,8, 4,1);
		gbBits->setLayout(slay);
	}

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(pbAbout, row,2);
	row++;
	lay->addWidget(m_WorkerThreadsIcon, row,0);
	lay->addWidget(lbworkerthreads, row,1);
	lay->addWidget(slworkerthreads, row,2);
	lay->addWidget(m_WorkerThreadsLabel, row,3);
	row++;
	lay->addWidget(m_AtomManagersLabel, row,2, 1,2);
	row++;
	lay->addWidget(m_WorkerPoolLabel, row,2, 1,2);
	row++;
	if(gameSaveFolderLabel && m_GameSaveDirButton && clearSolo)
	{
		lay->addWidget(gameSaveFolderLabel, row,0, 1,2);
		lay->addWidget(m_GameSaveDirButton, row,2);
		lay->addWidget(clearSolo, row,3);
		row++;
	}
	lay->addWidget(picFolderLabel, row,0, 1,2);
	lay->addWidget(m_PictureDirButton, row,2);
	row++;
	lay->addWidget(tempFolderLabel, row,0, 1,2);
	lay->addWidget(m_CacheDirButton, row,2);
	lay->addWidget(clearAllFiles, row,3);
	row++;
	lay->addWidget(clearLogs, row,2);
	row++;
	lay->addWidget(clearNetCache, row,2);
	row++;
	lay->addWidget(gbBits, row,2);
	row++;
	lay->setColumnMinimumWidth(3, 120);
	lay->setRowStretch(row, 2);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuSetProgram::~AuSetProgram()
{
}

void AuSetProgram::SetWorkerThreads(int value)
{
	QString threads;
	if(value)
		threads = tr("%1 threads").arg(value);
	else
	{
		int idealThreadCount = QThread::idealThreadCount();
		if(idealThreadCount > 2)
			idealThreadCount -= 2;
		else
			idealThreadCount = 1;
		if(idealThreadCount > 1)
			threads = tr("Auto (%1 threads)").arg(idealThreadCount);
		else
			threads = tr("Auto (1 thread)");
	}
	m_WorkerThreadsLabel->setText(threads);

	g_Au.SetWorkerThreadCount(value);

	threads = tr("%L1 Atom Managers (2 threads each)").arg(g_Au.AtomManagerCount());
	m_AtomManagersLabel->setText(threads);
	m_AtomManagersLabel->setVisible(g_Au.AtomManagerCount());

	threads = tr("%L1 Worker Thread Pool").arg(g_Au.WorkerThreadPoolCount());
	m_WorkerPoolLabel->setText(threads);
}

void AuSetProgram::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(250);
}

void AuSetProgram::UpdateContents()
{
	//qDebug() << "AuSetProgram::UpdateContents" << m_IconFrame;
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	// There are four animation frames in the icon.
	m_IconFrame = (m_IconFrame + 3) % 4;

	// Extract the current icon frame from the strip of four images.
	QPixmap busyIcon(32, 32);
	busyIcon.fill(Qt::black);
	QPixmap busyStrip(":data/Busy.png");
	QPainter p(&busyIcon);
	p.drawPixmap(QRect(0,0,32,32), busyStrip, QRect(m_IconFrame*32,0,32,32));
	p.end();

	m_WorkerThreadsIcon->setPixmap(busyIcon);
}

void AuSetProgram::SetGameSaveDir()
{
	QString gameSaveDir = QFileDialog::getExistingDirectory(this, tr("Choose Folder for Game Save Files"), g_Au.GameSaveDir(),
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(!gameSaveDir.isEmpty())
	{
		g_Au.SetGameSaveDir(gameSaveDir);
		m_GameSaveDirButton->setText(gameSaveDir);
	}
}

void AuSetProgram::SetCacheDir()
{
	QString cacheDir = QFileDialog::getExistingDirectory(this, tr("Choose Folder for Temporary Files"), g_Au.CacheApp(),
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(!cacheDir.isEmpty())
	{
		g_Au.SetCacheApp(cacheDir);
		m_CacheDirButton->setText(cacheDir);
	}
}

void AuSetProgram::SetPictureDir()
{
	QString picDir = QFileDialog::getExistingDirectory(this, tr("Choose Folder for Screen Grabs"), g_Au.PictureDir(),
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(!picDir.isEmpty())
	{
		g_Au.SetPictureDir(picDir);
		m_PictureDirButton->setText(picDir);
	}
}

void AuSetProgram::ClearLogs()
{
	QString smsg = tr("<html>Confirm deletion of all <i>Comm Channel Transcript</i> files from the temporary files folder.<br><br>"
			"<b>%1</b></html>").arg(g_Au.CacheApp());
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);

	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	// Clear the cache files.
	QDir tempDir(g_Au.CacheApp());
	QStringList nameFilters;
	nameFilters << "Transcript.*.txt";
	QStringList delFiles = tempDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);

	QDir soloDir(g_Au.CacheApp() + "/Solo");
	delFiles = soloDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);
}

void AuSetProgram::ClearPathFiles()
{
	QString smsg = tr("<html>All <i>Spacecraft Path Map</i> files will be deleted from the temporary files folder.<br><br>"
			"<b>%1</b></html>").arg(g_Au.CacheGame());
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);

	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	// Clear the cache files.
	QDir tempDir(g_Au.CacheGame());
	QStringList nameFilters;
	nameFilters << "SP[0-9]*";
	QStringList delFiles = tempDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);

	QDir soloDir(g_Au.CacheGame() + "/Solo");
	delFiles = soloDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);
}

void AuSetProgram::ClearFloraFiles()
{
	QString smsg = tr("<html>All <i>Plant Texture Palette</i> files will be deleted from the temporary files folder.<br><br>"
			"<b>%1</b></html>").arg(g_Au.CacheGame());
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Information, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);

	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	// Clear the cache files.
	QDir tempDir(g_Au.CacheGame());
	QStringList nameFilters;
	nameFilters << "F[0-9]*Z[0-9]*";		// Flora
	nameFilters << "FG[0-9]*Z[0-9]*";	// Grass
	QStringList delFiles = tempDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);

	QDir soloDir(g_Au.CacheGame() + "/Solo");
	delFiles = soloDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);
}

void AuSetProgram::ClearDesignFiles()
{
	QString smsg = tr("<html>All <i>Spacecraft Design</i> files will be deleted from the temporary files folder.<br><br>"
			"<b>%1</b></html>").arg(g_Au.CacheGame());
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Information, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);

	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	// Clear the cache files.
	QDir tempDir(g_Au.CacheGame());
	QStringList nameFilters;
	nameFilters << "D[0-9]*";
	QStringList delFiles = tempDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);

	QDir soloDir(g_Au.CacheGame() + "/Solo");
	delFiles = soloDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);
}

void AuSetProgram::ClearTerrainFiles()
{
	QString smsg = tr("<html>All <i>Terrain Model</i> files will be deleted from the temporary files folder.<br><br>"
			"<b>%1</b></html>").arg(g_Au.CacheGame());
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Information, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);

	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	// Clear the cache files.
	QDir tempDir(g_Au.CacheGame());
	QStringList nameFilters;
	nameFilters << "T[0-9]*";
	QStringList delFiles = tempDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);

	QDir soloDir(g_Au.CacheGame() + "/Solo");
	delFiles = soloDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);
}

void AuSetProgram::ClearGlobeFiles()
{
	QString smsg = tr("<html>All <i>World</i> files will be deleted from the temporary files folder.<br><br>"
			"<b>%1</b></html>").arg(g_Au.CacheGame());
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Information, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);

	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	// Clear the cache files.
	QDir tempDir(g_Au.CacheGame());
	QStringList nameFilters;
	nameFilters << "WG[AFN][0-9]*[0-9]";						// World at, flat and near models.
	nameFilters << "WTF[0-9]*[0-9]";							// World texture far
	QStringList delFiles = tempDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);

	QDir soloDir(g_Au.CacheGame() + "/Solo");
	delFiles = soloDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);
}

void AuSetProgram::ClearBuildingDesignFiles()
{
	QString smsg = tr("<html>All <i>Building Design</i> files will be deleted from the temporary files folder.<br><br>"
			"<b>%1</b></html>").arg(g_Au.CacheGame());
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Information, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);

	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	// Clear the cache files.
	QDir tempDir(g_Au.CacheGame());
	QStringList nameFilters;
	nameFilters << "BD[0-9]*";
	QStringList delFiles = tempDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);

	QDir soloDir(g_Au.CacheGame() + "/Solo");
	delFiles = soloDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);
}

void AuSetProgram::ClearAllFiles()
{
	QString smsg = tr("<html>All cached data files will be deleted from the temporary files folder.<br><br>"
			"<b>%1</b><p>"
			"Cached data files save a lot of time and reduce significant load from your computer. "
			"It is rarely necessary to delete cached data files.<p>"
			"Suggested reasons to clear all cached data files:<br>"
			"%2 You will not be playing <i>%3</i> on this computer any more.<br>"
			"%2 You want to reclaim disk space after playing <i>%3</i> for a long time.<br>"
			"%2 <i>%3</i> is acting weird and you don't know what else to do."
			"</html>").arg(g_Au.CacheApp()).arg(g_Au.MiddleDotSymbol()).arg(g_Au.ApplicationName());
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Information, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);

	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;
	g_Au.ClearAllCacheFiles();
}

void AuSetProgram::ClearNetCache()
{
	QString smsg = tr("<html>Confirm clearing of the <i>Internet Cache</i>."
			"</html>");
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);

	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	// Clear network cache files.
	auApp->Frame()->ClearNetCache();
}

void AuSetProgram::ClearSavedGames()
{
	QString smsg = tr("<html>Confirm clearing of all saved games, including the game in progress."
			"</html>");
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);

	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	// Clear saved game files.
	auApp->ClearSavedGames();
	
	// Clear current game in progress.
	auApp->ClearSolitaire();
}

void AuSetProgram::BitChanged(int state, int bit)
{
	//qDebug() << "AuSetProgram::BitChanged bit" << bit << "state" << state;
	switch(bit)
	{
	case 0:
		if(state)
		{
			g_Au.m_LogMessages |= LOG_ConnectDisconnect;

			g_Au.CreateCacheApp();
			QFile::remove(g_Au.CacheApp() + "/soh.txt");
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: CPU Cores %1").arg(QThread::idealThreadCount()));
		}
		else
			g_Au.m_LogMessages &= ~LOG_ConnectDisconnect;
		break;

	case 1:
		if(state)
			g_Au.m_DebugSwitches |= 0x00008000;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00008000;				// Turn the bit off.
		break;

	case 4:							// Show paths
		{
			m_Switches[bit]->blockSignals(true);
			m_Switches[bit]->setCheckState((state != Qt::Checked)?Qt::Checked:Qt::Unchecked);	// Turn it back the other way.
			m_Switches[bit]->blockSignals(false);
			AuSystemAccessCode dlg(tr("Access Code"), this);
			quint16 accessCode = (quint16)(AuTime::Now().CurrentDaysSinceEpoch() * (bit + 1) % 999);
			if(dlg.exec() == QDialog::Accepted && dlg.AccessCode() == accessCode)
			{
				m_Switches[bit]->blockSignals(true);
				m_Switches[bit]->setCheckState((state == Qt::Checked)?Qt::Checked:Qt::Unchecked);	// Change the state.
				m_Switches[bit]->blockSignals(false);
				if(state)
					g_Au.m_DebugSwitches |= 0x00020000;					// Turn the bit on.
				else
					g_Au.m_DebugSwitches &= ~0x00020000;				// Turn the bit off.
			}
		}
		break;

	case 5:	// AuEther thorough change parent
		{
			m_Switches[bit]->blockSignals(true);
			m_Switches[bit]->setCheckState((state != Qt::Checked)?Qt::Checked:Qt::Unchecked);	// Turn it back the other way.
			m_Switches[bit]->blockSignals(false);
			AuSystemAccessCode dlg(tr("Access Code"), this);
			quint16 accessCode = (quint16)(AuTime::Now().CurrentDaysSinceEpoch() * (bit + 1) % 999);
			if(dlg.exec() == QDialog::Accepted && dlg.AccessCode() == accessCode)
			{
				m_Switches[bit]->blockSignals(true);
				m_Switches[bit]->setCheckState((state == Qt::Checked)?Qt::Checked:Qt::Unchecked);	// Change the state.
				m_Switches[bit]->blockSignals(false);
				if(state)
					g_Au.m_DebugSwitches |= 0x00040000;					// Turn the bit on.
				else
					g_Au.m_DebugSwitches &= ~0x00040000;				// Turn the bit off.
			}
		}
		break;

	case 6:	// AuEther thorough trash
		{
			m_Switches[bit]->blockSignals(true);
			m_Switches[bit]->setCheckState((state != Qt::Checked)?Qt::Checked:Qt::Unchecked);	// Turn it back the other way.
			m_Switches[bit]->blockSignals(false);
			AuSystemAccessCode dlg(tr("Access Code"), this);
			quint16 accessCode = (quint16)(AuTime::Now().CurrentDaysSinceEpoch() * (bit + 1) % 999);
			if(dlg.exec() == QDialog::Accepted && dlg.AccessCode() == accessCode)
			{
				m_Switches[bit]->blockSignals(true);
				m_Switches[bit]->setCheckState((state == Qt::Checked)?Qt::Checked:Qt::Unchecked);	// Change the state.
				m_Switches[bit]->blockSignals(false);
				if(state)
					g_Au.m_DebugSwitches |= 0x00080000;					// Turn the bit on.
				else
					g_Au.m_DebugSwitches &= ~0x00080000;				// Turn the bit off.
			}
		}
		break;

	case 7:	// AuEther thorough remove
		{
			m_Switches[bit]->blockSignals(true);
			m_Switches[bit]->setCheckState((state != Qt::Checked)?Qt::Checked:Qt::Unchecked);	// Turn it back the other way.
			m_Switches[bit]->blockSignals(false);
			AuSystemAccessCode dlg(tr("Access Code"), this);
			quint16 accessCode = (quint16)(AuTime::Now().CurrentDaysSinceEpoch() * (bit + 1) % 999);
			if(dlg.exec() == QDialog::Accepted && dlg.AccessCode() == accessCode)
			{
				m_Switches[bit]->blockSignals(true);
				m_Switches[bit]->setCheckState((state == Qt::Checked)?Qt::Checked:Qt::Unchecked);	// Change the state.
				m_Switches[bit]->blockSignals(false);
				if(state)
					g_Au.m_DebugSwitches |= 0x00100000;					// Turn the bit on.
				else
					g_Au.m_DebugSwitches &= ~0x00100000;				// Turn the bit off.
			}
		}
		break;

	case 8:								// Frame time on screen.
		if(state)
			g_Au.m_DebugSwitches |= 0x00000002;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00000002;				// Turn the bit off.
		break;

	case 9:								// Frame rate on screen.
		if(state)
			g_Au.m_DebugSwitches |= 0x00000001;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00000001;				// Turn the bit off.
		break;

	case 10:
		if(state)
			g_Au.m_DebugSwitches |= 0x01000000;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x01000000;				// Turn the bit off.
		break;

	case 11:
		if(state)
			g_Au.m_DebugSwitches |= 0x02000000;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x02000000;				// Turn the bit off.
		break;

	case 12:
		if(state)
			g_Au.m_DebugSwitches |= 0x04000000;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x04000000;				// Turn the bit off.
		break;

	case 13:
		if(state)
			g_Au.m_DebugSwitches |= 0x08000000;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x08000000;				// Turn the bit off.
		break;

	case 14:
		if(state)
			g_Au.m_DebugSwitches |= 0x10000000;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x10000000;				// Turn the bit off.
		break;

	case 15:
		if(state)
			g_Au.m_DebugSwitches |= 0x20000000;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x20000000;				// Turn the bit off.
		break;

	case 16:							// Render cities.
		if(state)
			g_Au.m_DebugSwitches |= 0x00000004;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00000004;				// Turn the bit off.
		break;

	case 17:							// Render spacecraft.
		if(state)
			g_Au.m_DebugSwitches |= 0x00000010;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00000010;				// Turn the bit off.
		break;

	case 18:							// Render vehicles.
		if(state)
			g_Au.m_DebugSwitches |= 0x00000020;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00000020;				// Turn the bit off.
		break;

	case 19:
		// Render at worlds.
		if(state)
			g_Au.m_DebugSwitches |= 0x00000040;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00000040;				// Turn the bit off.
		break;

	case 20:
		// Render near worlds.
		if(state)
			g_Au.m_DebugSwitches |= 0x00000008;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00000008;				// Turn the bit off.
		break;

	case 21:
		// Render far worlds.
		if(state)
			g_Au.m_DebugSwitches |= 0x00000080;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00000080;				// Turn the bit off.
		break;

	case 22:
		// Render atmosphere.
		if(state)
			g_Au.m_DebugSwitches |= 0x00000100;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00000100;				// Turn the bit off.
		break;

	case 23:
		// Render flora.
		if(state)
			g_Au.m_DebugSwitches |= 0x00000200;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00000200;				// Turn the bit off.
		break;

	case 24:
		if(state)
			g_Au.m_DebugSwitches |= 0x00000400;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00000400;				// Turn the bit off.
		break;

	case 25:
		if(state)
			g_Au.m_DebugSwitches |= 0x00000800;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00000800;				// Turn the bit off.
		break;

	case 26:
		if(state)
			g_Au.m_DebugSwitches |= 0x00001000;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00001000;				// Turn the bit off.
		break;

	case 27:
		if(state)
			g_Au.m_DebugSwitches |= 0x00002000;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00002000;				// Turn the bit off.
		break;

	case 28:
		if(state)
			g_Au.m_DebugSwitches |= 0x00004000;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00004000;				// Turn the bit off.
		break;

	case 29:
		if(state)
			g_Au.m_DebugSwitches |= 0x00010000;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x00010000;				// Turn the bit off.
		break;

	case 30:
		if(state)
			g_Au.m_DebugSwitches |= 0x40000000;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x40000000;				// Turn the bit off.
		break;

	case 31:
		if(state)
			g_Au.m_DebugSwitches |= 0x80000000;					// Turn the bit on.
		else
			g_Au.m_DebugSwitches &= ~0x80000000;				// Turn the bit off.
		break;
	}
}

void AuSetProgram::AboutSoH()
{
	AuAbout about(this);
	about.exec();
}

void AuSetProgram::ClearBuildingExchangeFiles()
{
	QString smsg = tr("<html>All <i>Building Blueprint Exchange</i> files will be deleted from the temporary files folder.<br><br>"
			"<b>%1</b></html>").arg(g_Au.CacheGame());
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Information, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);

	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	// Clear the cache files.
	QDir tempDir(g_Au.CacheGame());
	QStringList nameFilters;
	nameFilters << "BdX[0-9]*";
	QStringList delFiles = tempDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);
}

void AuSetProgram::ClearSpacecraftExchangeFiles()
{
	QString smsg = tr("<html>All <i>Spacecraft Blueprint Exchange</i> files will be deleted from the temporary files "
			"folder.<br><br>"
			"<b>%1</b></html>").arg(g_Au.CacheGame());
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Information, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);

	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	// Clear the cache files.
	QDir tempDir(g_Au.CacheGame());
	QStringList nameFilters;
	nameFilters << "ScX[0-9]*";
	QStringList delFiles = tempDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);
}

