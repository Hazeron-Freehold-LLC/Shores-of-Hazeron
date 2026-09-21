//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auframe.cpp,v 1.37 2022/02/19 18:42:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auframe.h"
//
#include "audnasettings.h"
#include "auglobal.h"
#include "auview.h"
#include "auviewitem.h"
#include "dhdna.h"
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuFrame::AuFrame(QWidget *parent) : QMainWindow(parent)
{
	setBackgroundRole(QPalette::Background);
	setObjectName("AuFrame");
	setWindowIcon(QIcon(":data/Viewer.png"));
	setWindowTitle(g_Au.ApplicationName());

	m_DNASettings = 0;

	m_View = new AuView(this);
	setCentralWidget(m_View);

	ResetMenuBar(0);
}

AuFrame::~AuFrame()
{
}

QSize AuFrame::sizeHint() const
{
	return QSize(1280, 720);
}

void AuFrame::closeEvent(QCloseEvent* event)
{
	QMainWindow::closeEvent(event);
}

void AuFrame::ResetMenuBar(int mode)
{
	QMenuBar* mbar = menuBar();
	if(!mbar)
		return;

	mbar->clear();

	QMenu* mFile = mbar->addMenu(tr("&File"));
	mFile->addAction(tr("&New Meat Profile..."), this, SLOT(NewMeatProfile()));
	mFile->addAction(tr("&Save Meat Profile..."), this, SLOT(SaveMeatProfile()));
	mFile->addSeparator();
	mFile->addAction(tr("&New Meat Line..."), this, SLOT(NewMeatLine()));
	mFile->addAction(tr("&Save Meat Line..."), this, SLOT(SaveMeatLine()));

	// Load menu.
	//QMenu* mLoad = mFile->addMenu(tr("&Load"));
	mFile->addSeparator();
	QAction* a = mFile->addAction(tr("&Female Primate"), this, SLOT(ViewFemalePrimate()));
	a->setShortcut(QKeySequence(Qt::Key_1));

	a = mFile->addAction(tr("&Male Primate"), this, SLOT(ViewMalePrimate()));
	a->setShortcut(QKeySequence(Qt::Key_2));

	a = mFile->addAction(tr("&Body"), this, SLOT(ViewBody()));
	a->setShortcut(QKeySequence(Qt::Key_3));

	mFile->addAction(tr("&Item or Vehicle..."), this, SLOT(ViewItem()));
	mFile->addAction(tr("&Meat Line..."), this, SLOT(LoadMeatLine()));
	mFile->addAction(tr("&Meat Profile..."), this, SLOT(LoadMeatProfile()));
	mFile->addAction(tr("&Model File..."), this, SLOT(ViewModel()));

	switch(mode)
	{
	case 0:
		{
			mbar->addAction(tr("&DNA..."), this, SLOT(DNASettings()));
		}
		return;

	case 1:	// Animalia
		break;

	case 2:	// Body
		{
			QMenu* mBody = mbar->addMenu(tr("&Body"));
			mBody->addAction(tr("&Stand"), m_View, SLOT(BodyStand()));
			mBody->addAction(tr("&Walk"), m_View, SLOT(BodyWalk()));
			mBody->addAction(tr("&Run"), m_View, SLOT(BodyRun()));
		}
		break;

	case 3:	// Item or Vehicle
		{
			QMenu* mItem = mbar->addMenu(tr("&Item or Vehicle"));
			mItem->addAction(tr("&Toggle State (Lights, etc.)"), m_View, SLOT(ItemToggleState()));
		}
		break;

	case 4:	// Model File
		{
			QMenu* mModel = mbar->addMenu(tr("&Model"));
			mModel->addAction(tr("&Toggle State (Lights, etc.)"), m_View, SLOT(ModelToggleState()));
		}
		break;

	case 5:	// Meat Profile
		{
			QMenu* mMeat = mbar->addMenu(tr("&Meat Profile"));
			mMeat->addAction(tr("&Choose Image..."), m_View, SLOT(MeatProfileChooseImage()));

			mMeat->addSeparator();

			mMeat->addAction(tr("&Front Joints"), m_View, SLOT(MeatProfileInputFrontJoints()));

			mMeat->addAction(tr("&Side Joints"), m_View, SLOT(MeatProfileInputSideJoints()));

			mMeat->addSeparator();

			mMeat->addAction(tr("&Front Profile"), m_View, SLOT(MeatProfileInputFrontProfile()));

			mMeat->addAction(tr("&Side Profile"), m_View, SLOT(MeatProfileInputSideProfile()));

			mMeat->addSeparator();

			QAction* end = mMeat->addAction(tr("&Close Profile"), m_View, SLOT(MeatProfileEnd()));
			end->setShortcut(QKeySequence(Qt::Key_End));

			QAction* bs = mMeat->addAction(tr("&Back Space Input"), m_View, SLOT(MeatProfileBackspace()));
			bs->setShortcut(QKeySequence(Qt::Key_Backspace));

			QAction* stop = mMeat->addAction(tr("&Stop Input"), m_View, SLOT(MeatProfileStopInput()));
			stop->setShortcut(QKeySequence(Qt::Key_Escape));

			QAction* clear = mMeat->addAction(tr("&Clear Profile..."), m_View, SLOT(MeatProfileClear()));
			clear->setShortcut(QKeySequence(Qt::Key_Delete));

			mMeat->addSeparator();

			mMeat->addAction(tr("&Scale Profile..."), m_View, SLOT(MeatProfileScale()));
		}
		break;
	}

	// View menu.
	QMenu* mView = mbar->addMenu(tr("&View"));
	if(mode == 1)
	{
		QAction* a = mView->addAction(tr("&Lines"), m_View, SLOT(ViewLines()));
		a->setShortcut(QKeySequence(Qt::Key_F1));

		a = mView->addAction(tr("&Solid"), m_View, SLOT(ViewSolid()));
		a->setShortcut(QKeySequence(Qt::Key_F2));

		a = mView->addAction(tr("&Solid and Mesh"), m_View, SLOT(ViewLinesAndSolid()));
		a->setShortcut(QKeySequence(Qt::Key_F3));

		a = mView->addAction(tr("&Solid, Mesh and Intersections"), m_View, SLOT(ViewSolidMeshAndIntersections()));
		a->setShortcut(QKeySequence(Qt::Key_F4));

		mView->addSeparator();
	}
	if(mode == 5)
	{
		mView->addAction(tr("&Meat Image"), m_View, SLOT(MeatImageToggle()));
	}
	else
	{
		mView->addAction(tr("&Concrete"), m_View, SLOT(ConcreteToggle()));
		mView->addAction(tr("&Dirt"), m_View, SLOT(DirtToggle()));
		mView->addAction(tr("&Grass"), m_View, SLOT(GrassToggle()));
		mView->addAction(tr("&Tarmac"), m_View, SLOT(TarmacToggle()));
		mView->addSeparator();

		QAction* a = mView->addAction(tr("&Save"), m_View, SLOT(ViewSave()));
		a->setShortcut(QKeySequence(Qt::Key_F5));

		a = mView->addAction(tr("&Restore"), m_View, SLOT(ViewRestore()));
		a->setShortcut(QKeySequence(Qt::Key_F6));
	}
	QAction* aReset = mView->addAction(tr("&Reset"), m_View, SLOT(ResetView()));
	aReset->setShortcut(QKeySequence(Qt::Key_Home));
}

void AuFrame::ViewFemalePrimate()
{
	GENDER gender = Female;

	AuDNA dna;
	dna.InitDefaultHuman();

	m_View->ViewAnimal(dna, gender, LeftHanded);
	ResetMenuBar(1);
}

void AuFrame::ViewMalePrimate()
{
	GENDER gender = Male;

	AuDNA dna;
	dna.InitDefaultHuman();

	m_View->ViewAnimal(dna, gender, RightHanded);
	ResetMenuBar(1);
}

void AuFrame::ViewBody()
{
	DhDNA dna;
	dna.InitDefaultHuman();

	m_View->ViewBody(1, false, dna);
	ResetMenuBar(2);
}

void AuFrame::ViewItem()
{
	AuViewItem dlg(this);
	if(dlg.exec() != QDialog::Accepted)
		return;

	m_View->ViewItem(dlg.CommodityId());
	ResetMenuBar(3);
}

void AuFrame::ViewModel()
{
	QString lastFile = g_Au.Settings().readEntry("/ModelFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();

	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QString loadFilter;
	loadFilter += QString("3ds Model (*.3ds *.3dsX *.3dsN)");
	loadFilter += QString(";;All Files (*.*)");

	QString loadFile = QFileDialog::getOpenFileName(this, tr("3ds Model File"), lastFile, loadFilter, 0,
			QFileDialog::DontUseNativeDialog);
	if(loadFile.isEmpty())
		return;

	g_Au.Settings().writeEntry("/ModelFile", loadFile);

	m_View->ViewModel(loadFile);
	ResetMenuBar(4);
}

void AuFrame::NewMeatLine()
{
	m_View->NewMeatProfile();
	ResetMenuBar(5);
}

void AuFrame::NewMeatProfile()
{
	m_View->NewMeatProfile();
	ResetMenuBar(5);
}

void AuFrame::LoadMeatLine()
{
	QString lastFile = g_Au.Settings().readEntry("/MeatLineFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();

	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QString loadFilter;
	loadFilter += QString("Meat Line (*.ml)");
	loadFilter += QString(";;All Files (*.*)");

	QString loadFile = QFileDialog::getOpenFileName(this, tr("Meat Line File"), lastFile, loadFilter, 0,
			QFileDialog::DontUseNativeDialog);
	if(loadFile.isEmpty())
		return;

	g_Au.Settings().writeEntry("/MeatLineFile", loadFile);

	m_View->LoadMeatLine(loadFile);
	ResetMenuBar(5);
}

void AuFrame::LoadMeatProfile()
{
	QString lastFile = g_Au.Settings().readEntry("/MeatProfileFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();

	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QString loadFilter;
	loadFilter += QString("Meat Profile (*.mp)");
	loadFilter += QString(";;All Files (*.*)");

	QString loadFile = QFileDialog::getOpenFileName(this, tr("Meat Profile File"), lastFile, loadFilter, 0,
			QFileDialog::DontUseNativeDialog);
	if(loadFile.isEmpty())
		return;

	g_Au.Settings().writeEntry("/MeatProfileFile", loadFile);

	m_View->LoadMeatProfile(loadFile);
	ResetMenuBar(5);
}

void AuFrame::SaveMeatProfile()
{
	QString lastFile = g_Au.Settings().readEntry("/MeatProfileFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();

	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QString saveFilter;
	saveFilter += QString("Meat Profile (*.mp)");
	saveFilter += QString(";;All Files (*.*)");

	QString saveFile = QFileDialog::getSaveFileName(this, tr("Meat Profile File"), lastFile, saveFilter, 0,
			QFileDialog::DontUseNativeDialog);
	if(saveFile.isEmpty())
		return;

	// Insure the file has an appropriate suffix.
	QString suffix(".mp");
	if(saveFile.right(suffix.length()) != suffix)
		saveFile += suffix;

	g_Au.Settings().writeEntry("/MeatProfileFile", saveFile);

	m_View->SaveMeatProfile(saveFile);
}

void AuFrame::SaveMeatLine()
{
	QString lastFile = g_Au.Settings().readEntry("/MeatLineFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();

	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QString saveFilter;
	saveFilter += QString("Meat Line (*.ml)");
	saveFilter += QString(";;All Files (*.*)");

	QString saveFile = QFileDialog::getSaveFileName(this, tr("Meat Line File"), lastFile, saveFilter, 0,
			QFileDialog::DontUseNativeDialog);
	if(saveFile.isEmpty())
		return;

	// Insure the file has an appropriate suffix.
	QString suffix(".ml");
	if(saveFile.right(suffix.length()) != suffix)
		saveFile += suffix;

	g_Au.Settings().writeEntry("/MeatLineFile", saveFile);

	m_View->SaveMeatLine(saveFile);
}

void AuFrame::DNASettings()
{
	if(!m_DNASettings)
		m_DNASettings = new AuDNASettingsDlg(this);
	m_DNASettings->exec();
}
