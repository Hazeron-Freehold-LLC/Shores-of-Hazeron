//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulaunchersettings.cpp,v 1.13 2022/02/19 18:34:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aulaunchersettings.h"
//
#include <QComboBox>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>

AuLauncherSettings::AuLauncherSettings(const QString& sohFolder, const QString& picFolder, const QString& cacheFolder, int glVer,
		float myVer, const QString& arch, QWidget* parent) : QDialog(parent)
{
	setWindowTitle(tr("Shores of Hazeron Settings"));
	setWindowIcon(QIcon(":data/Settings.png"));

	QLabel* lbInfo = new QLabel(tr("<html>"
				"<big><b>Shores of Hazeron Launcher</b></big>"
				"<br>"
				"<small><i>Version %1 %2 %3</i></small>"
				"</html>")
			.arg(myVer, 0,'f',1)
			.arg(arch)
			.arg(__DATE__));
	lbInfo->setWordWrap(true);

	QLabel* lbOpenGL;
	QPushButton* pbHelpGL;
#if defined(Q_OS_MACOS)
	lbOpenGL = new QLabel(tr("<i>OpenGL</i> version."), this);

	m_OpenGLCombo = new QComboBox(this);
	m_OpenGLCombo->addItem(tr("OpenGL 4.1"), 41);
	m_OpenGLCombo->addItem(tr("OpenGL 3.3"), 33);
	m_OpenGLCombo->setCurrentIndex(m_OpenGLCombo->findData(glVer));
	m_OpenGLCombo->setToolTip(tr("<html>"
				"Selects the version of OpenGL to use. "
				"You should choose the version that is supported by your video device.<br>"
				"<b>You should restart the <i>Launcher</i> after changing this setting, to update the appropriate files.</b>"
				"</html>"));

	pbHelpGL = new QPushButton(tr("Help..."), this);
	pbHelpGL->setToolTip(tr("<html><b>OpenGL Version Help</b><br>"
				"Opens your web browser at the <i>OpenGL Capabilities Tables</i> page of the Apple web site.<p>"
				"https://developer.apple.com/opengl/capabilities/"
				"</html>"));
	connect(pbHelpGL, SIGNAL(clicked()), SLOT(OpenAppleOpenGLPage()));

#else
	lbOpenGL = 0;
	pbHelpGL = 0;
	m_OpenGLCombo = 0;
	Q_UNUSED(glVer);
#endif

	QLabel* lbfiles = new QLabel(tr("Location of <i>Shores of Hazeron</i> files."), this);

	m_FilesEdit = new QLineEdit(sohFolder, this);
	m_FilesEdit->setReadOnly(true);
	m_FilesEdit->setToolTip(tr("<html>"
				"Shows the path to the folder where <i>Shores of Hazeron</i> files will be stored. "
				"The launcher program downloads updates into this folder and "
				"it will run <i>Shores of Hazeron</i> from this folder.<br>"
				"You must have permission to write to this folder."
				"</html>"));

	QToolButton* browsefiles = new QToolButton(this);
	browsefiles->setAutoRaise(true);
	browsefiles->setIcon(QIcon(":data/Folder.png"));
	browsefiles->setIconSize(QSize(24,24));
	browsefiles->setToolTip(tr("<html>"
				"Locates a folder for <i>Shores of Hazeron</i> files."
				"</html>"));
	connect(browsefiles, SIGNAL(clicked()), SLOT(ChooseSoHFolder()));

	QLabel* lbpics = new QLabel(tr("Location of <i>Shores of Hazeron</i> screen grabs."), this);

	m_PicsEdit = new QLineEdit(picFolder, this);
	m_PicsEdit->setReadOnly(true);
	m_PicsEdit->setToolTip(tr("<html>"
				"Shows the path to the folder where <i>Shores of Hazeron</i> stores screen grabs.<br>"
				"This is also the folder where the <i>Shores of Hazeron</i> launcher finds pictures "
				"to show in the background.<br>"
				"You must have permission to write to this folder."
				"</html>"));

	QToolButton* browsepics = new QToolButton(this);
	browsepics->setAutoRaise(true);
	browsepics->setIcon(QIcon(":data/Folder.png"));
	browsepics->setIconSize(QSize(24,24));
	browsepics->setToolTip(tr("<html>"
				"Locates a folder for <i>Shores of Hazeron</i> pictures."
				"</html>"));
	connect(browsepics, SIGNAL(clicked()), SLOT(ChoosePictureFolder()));

	QLabel* lbcache = new QLabel(tr("Location of <i>Shores of Hazeron</i> temporary files."), this);

	m_CacheEdit = new QLineEdit(cacheFolder, this);
	m_CacheEdit->setReadOnly(true);
	m_CacheEdit->setToolTip(tr("<html>"
				"Shows the path to the folder where <i>Shores of Hazeron</i> stores temporary files.<br>"
				"You must have permission to write to this folder."
				"</html>"));

	QToolButton* browsecache = new QToolButton(this);
	browsecache->setAutoRaise(true);
	browsecache->setIcon(QIcon(":data/Folder.png"));
	browsecache->setIconSize(QSize(24,24));
	browsecache->setToolTip(tr("<html>"
				"Locates a folder for <i>Shores of Hazeron</i> temporary files."
				"</html>"));
	connect(browsecache, SIGNAL(clicked()), SLOT(ChooseCacheFolder()));

	QPushButton* pbok = new QPushButton("Ok");
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("Accepts the changes to the settings."));
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbInfo, row,0, 1,3);
	row++;
	if(lbOpenGL && m_OpenGLCombo)
	{
		lay->addWidget(lbOpenGL, row,0, 1,3);
		row++;
		lay->addWidget(m_OpenGLCombo, row,1, 1,1);
		lay->addWidget(pbHelpGL, row,2, 1,1);
		row++;
	}
	lay->addWidget(lbfiles, row,0, 1,3);
	row++;
	lay->addWidget(browsefiles, row,0);
	lay->addWidget(m_FilesEdit, row,1, 1,2);
	row++;
	lay->addWidget(lbpics, row,0, 1,3);
	row++;
	lay->addWidget(browsepics, row,0);
	lay->addWidget(m_PicsEdit, row,1, 1,2);
	row++;
	lay->addWidget(lbcache, row,0, 1,3);
	row++;
	lay->addWidget(browsecache, row,0);
	lay->addWidget(m_CacheEdit, row,1, 1,2);
	row++;
	lay->setColumnMinimumWidth(1, 250);
	lay->addWidget(pbok, row,2);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuLauncherSettings::~AuLauncherSettings()
{
}

QString AuLauncherSettings::CacheFolder() const
{
	return m_CacheEdit->text();
}

QString AuLauncherSettings::PictureFolder() const
{
	return m_PicsEdit->text();
}

QString AuLauncherSettings::SoHFolder() const
{
	return m_FilesEdit->text();
}

void AuLauncherSettings::ChooseCacheFolder()
{
	QString cacheFolder = CacheFolder();
	cacheFolder = QFileDialog::getExistingDirectory(this, tr("Shores of Hazeron Temporary Files Folder"), cacheFolder,
			QFileDialog::DontUseNativeDialog);
	if(!cacheFolder.isEmpty())
		m_CacheEdit->setText(cacheFolder);
}

void AuLauncherSettings::ChoosePictureFolder()
{
	QString picFolder = PictureFolder();
	picFolder = QFileDialog::getExistingDirectory(this, tr("Shores of Hazeron Pictures Folder"), picFolder,
			QFileDialog::DontUseNativeDialog);
	if(!picFolder.isEmpty())
		m_PicsEdit->setText(picFolder);
}

void AuLauncherSettings::ChooseSoHFolder()
{
	QString sohFolder = SoHFolder();
	sohFolder = QFileDialog::getExistingDirectory(this, tr("Shores of Hazeron Folder"), sohFolder,
			QFileDialog::DontUseNativeDialog);
	if(!sohFolder.isEmpty())
		m_FilesEdit->setText(sohFolder);
}

int AuLauncherSettings::OpenGLVersion() const
{
	if(!m_OpenGLCombo)
		return 32;
	return m_OpenGLCombo->currentData().toInt();
}

void AuLauncherSettings::OpenAppleOpenGLPage()
{
	QUrl url("https://developer.apple.com/opengl/capabilities/");
	QDesktopServices::openUrl(url);
}
