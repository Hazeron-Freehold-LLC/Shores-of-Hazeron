//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auintrobase.cpp,v 1.115 2022/05/19 15:52:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auintrobase.h"
//
#include "auapp.h"
#include "aubodyrenderwidget.h"
#include "aucity.h"
#include "aucombat.h"
#include "audna.h"
#include "audnasettings.h"
#include "aufilever.h"
#include "auglobal.h"
#include "auitem.h"
#include "aulogincomm.h"
#include "aumail.h"
#include "aupicturewidget.h"
#include "ausound.h"
#include "autabcolumn.h"
#include "autime.h"
#include "daperson.h"
#include "dgdata.h"
#include "dpbody.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
//#include <QMediaPlayer>
//#include <QMediaPlaylist>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QStackedLayout>
#include <QTableWidget>
#include <QToolButton>

AuIntroBase::AuIntroBase(QWidget* parent) : QWidget(parent)	//QVideoWidget(parent)
{
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	//m_Player = 0;
	//m_Playlist = 0;

	m_CreditsHt = -1;
	m_YPos = 0;

	QString copy;
	if(QString(AuUtil::CopyrightYear()) == QString("2022"))
		copy = QString("2022");
	else
		copy = QString("2022-%1").arg(AuUtil::CopyrightYear());

	//m_Credits << ":data/SoHLogo128.png";
	m_Credits << QString("*%1").arg(g_Au.ApplicationName());
	m_Credits << QString("  %1").arg(AuUtil::VersionFull());

	m_Credits << QString("    Copyright %1 %2 Hazeron Freehold LLC").arg(g_Au.CopyrightSymbol()).arg(copy);
	m_Credits << "      All rights reserved.";

	// Get message of the day from the settings file.
	QString motd = g_Au.Settings().readEntry("Settings/MOTD");
	if(!motd.isEmpty())
	{
		m_Credits << "____________________________________________________________";
		m_Credits << "";
		m_Credits << "Quote for Today...";
		m_Credits << "";
		// Display the message of the day, one line at a time.
		int iline;
		do
		{
			iline = motd.indexOf('\n');
			if(iline >= 0)
			{
				QString auline = motd.left(iline);
				m_Credits << auline;
				motd = motd.mid(iline+1);
			}
		}
		while(iline >= 0);
		// Display final line fragment, if any.
		if(!motd.isEmpty())
			m_Credits << motd;
	}

	auApp->PlayClientSound(MUSIC_ShroudOfGlory);
}

AuIntroBase::~AuIntroBase()
{
	auApp->StopClientSounds();
	/*
	if(m_Player)
		delete m_Player;
	if(m_Playlist)
		delete m_Playlist;
		*/
}

void AuIntroBase::setVisible(bool visible)
{
	QWidget::setVisible(visible);
}

void AuIntroBase::paintEvent(QPaintEvent* pe)
{
	//QVideoWidget::paintEvent(pe);
	QWidget::paintEvent(pe);

	QPainter p(this);
	if(!m_SplashImage.isNull())
		p.drawImage(rect(), m_SplashImage);

	// Roll credits.
	{
		QFont fntbig(g_Au.TypefaceSerif(), 48, QFont::Bold, true);
		p.setFont(fntbig);
		const int bight = p.fontMetrics().height();

		QFont fntsm(g_Au.TypefaceSerif(), 24, QFont::DemiBold, true);
		p.setFont(fntsm);	// Default font is small.
		const int smht = p.fontMetrics().height();

		// Calculate height of credits.
		if(m_CreditsHt < 0)
		{
			m_CreditsHt = 0;
			for(QStringList::const_iterator sit = m_Credits.constBegin(); sit != m_Credits.constEnd(); sit++)
			{
				if((*sit).left(1) == ":")
				{
					QImage img(*sit);
					m_CreditsHt += img.height();
				}
				else if((*sit).left(1) == "*")
					m_CreditsHt += bight;
				else
					m_CreditsHt += smht;
			}
		}

		int xpos = 24;
		int ypos = m_YPos + height();
		int ht3 = height()/3;
		if(ht3 == 0)
			return;
		int ht23 = ht3 + ht3;	// 2/3
		m_YPos -= 1;
		if(m_YPos < -(m_CreditsHt + ht23))
			m_YPos = 0;
		for(QStringList::const_iterator sit = m_Credits.constBegin(); sit != m_Credits.constEnd(); sit++)
		{
			// Calculate alpha for fade in-out appearance.
			int alpha = ht3 - abs(ypos - ht23);
			alpha = 80 * alpha / ht3;

			if((*sit).left(1) == ":")
			{
				QImage img(*sit);
				if(ypos >= ht3 && ypos < height() && alpha > 0)
				{
					// Alpha fade the image.
					QImage imga(img.width(), img.height(), QImage::Format_ARGB32);
					if(!imga.isNull())
					{
						imga.fill(QColor(0,0,0,alpha));
						QPainter pa(&imga);
						pa.setCompositionMode(QPainter::CompositionMode_SourceIn);
						pa.drawImage(0,0, img);
						pa.end();

						// Draw the alpha faded image.
						p.drawImage(xpos, ypos, imga);
					}
				}
				ypos += img.height();
			}
			else if((*sit).left(1) == "*")
			{
				// Use big font for title line.
				if(ypos >= ht3 && ypos < height() && alpha > 0)
				{
					p.setPen(QPen(QColor(155,165,164,alpha), 0.f));
					p.setFont(fntbig);
					p.drawText(xpos, ypos, width(), bight, Qt::AlignLeft|Qt::AlignTop, (*sit).mid(1));
					p.setFont(fntsm);	// Restore default small font.
				}
				ypos += bight;
			}
			else
			{
				if(ypos >= ht3 && ypos < height() && alpha > 0 && !(*sit).isEmpty())
				{
					p.setPen(QPen(QColor(155,165,164,alpha), 0.f));
					p.drawText(xpos, ypos, width(), smht, Qt::AlignLeft|Qt::AlignTop, *sit);
				}
				ypos += smht;
			}
		}
	}
}

void AuIntroBase::EmpireDataReceived()
{
}

void AuIntroBase::EnterNewAvatar()
{
}

bool AuIntroBase::SetSplashImage(const QImage& image)
{
	m_SplashImage = image;
	return !m_SplashImage.isNull();
}

bool AuIntroBase::SetSplashMovie(const QUrl& movie)
{
	Q_UNUSED(movie);
	/*
	if(!m_Player)
		m_Player = new QMediaPlayer;
	if(!m_Playlist)
		m_Playlist = new QMediaPlaylist(m_Player);
	m_Playlist->clear();
	m_Playlist->addMedia(movie);
	m_Playlist->setCurrentIndex(0);
	m_Player->setVideoOutput(this);
	m_Player->play();
	*/
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroAvatarWidget::AuIntroAvatarWidget(bool solo, int index, QWidget* parent) : QWidget(parent)
{
	m_Solo = solo;
	m_UnitDNA = 0;
	m_UnitIndex = index;

	m_Name = new QPushButton(this);
	m_Name->setCheckable(true);
	m_Name->setFlat(true);
	connect(m_Name, SIGNAL(clicked()), SLOT(Select()));

	m_NewDNA3 = new QPushButton(tr("New Avatar Gen III"), this);
	m_NewDNA3->setCheckable(true);
	m_NewDNA3->setFlat(true);
	m_NewDNA3->setToolTip(tr("<html>"
				"<p>"
				"Start a new avatar using third generation animal DNA. "
				"</p>"
				"<p>"
				"<i>%1</i> animals have evolved through three generations of design. "
				"The current third generation of animals is fully functional but it is being phased out. "
				"Trophy heads will remain in the universe. "
				"</p>"
				"<p>"
				"<b>You will be able to upgrade your avatar DNA when the fourth generation is ready.</b>"
				"</p>"
				"</html>").arg(g_Au.ApplicationName()));
	connect(m_NewDNA3, SIGNAL(clicked()), SLOT(SelectDNA3()));

	m_NewDNA4 = new QPushButton(tr("New Avatar Gen IV"), this);
	m_NewDNA4->setCheckable(true);
	m_NewDNA4->setFlat(true);
	m_NewDNA4->setToolTip(tr("<html>"
				"<p>"
				"Start a new avatar using fourth generation animal DNA. "
				"</p>"
				"<p>"
				"<i>%1</i> animals have evolved through three generations of design. "
				"The current third generation of animals is fully functional but it is being phased out. "
				"</p>"
				"<p>"
				"<b>Fourth generation animals are not fully functional.</b> "
				"When the fourth generation is ready, third generation animals will be phased out entirely. "
				"Trophy heads will remain in the universe. "
				"</p>"
				"</html>").arg(g_Au.ApplicationName()));
	connect(m_NewDNA4, SIGNAL(clicked()), SLOT(SelectDNA4()));

	m_Enter = new QToolButton(this);
	m_Enter->setAutoRaise(true);
	m_Enter->setIcon(QIcon(":data/StartExistingAvatar.png"));
	m_Enter->setIconSize(QSize(128,64));
	m_Enter->setToolTip(QObject::tr("<b>Return to Hazeron</b><br>"
				"Returns to the Hazeron universe in the persona of this avatar."));
	connect(m_Enter, SIGNAL(clicked()), SLOT(Enter()));

	m_Save = new QToolButton(this);
	m_Save->setAutoRaise(true);
	m_Save->setIcon(QIcon(":data/DNA.png"));
	m_Save->setIconSize(QSize(32,32));
	m_Save->setToolTip(QObject::tr("<b>Save DNA...</b><br>"
				"Saves this avatar's DNA to a file."));
	connect(m_Save, SIGNAL(clicked()), SLOT(Save()));

	m_Delete = new QToolButton(this);
	m_Delete->setAutoRaise(true);
	m_Delete->setIcon(QIcon(":data/DeleteAvatar.png"));
	m_Delete->setIconSize(QSize(32,32));
	m_Delete->setToolTip(QObject::tr("<b>Delete...</b><br>"
				"Deletes this avatar, after confirmation."));
	connect(m_Delete, SIGNAL(clicked()), SLOT(Delete()));

	m_Whereabouts = new QLabel(this);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(m_Save);
	blay->addStretch();
	blay->addWidget(m_Delete);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* vblay = new QVBoxLayout;
	vblay->addWidget(m_Enter);
	vblay->addLayout(blay);
	vblay->addStretch();
	vblay->setMargin(0);
	vblay->setSpacing(2);

	QHBoxLayout* hlay = new QHBoxLayout;
	hlay->addWidget(m_Whereabouts, 1);
	hlay->addLayout(vblay);
	hlay->setMargin(0);
	hlay->setSpacing(2);

	QHBoxLayout* slay = new QHBoxLayout;
	slay->addWidget(m_Name);
	slay->addWidget(m_NewDNA3);
	slay->addWidget(m_NewDNA4);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(slay);
	lay->addLayout(hlay);
	lay->setMargin(0);
	lay->setSpacing(2);
	setLayout(lay);
}

AuIntroAvatarWidget::~AuIntroAvatarWidget()
{
	if(m_UnitDNA)
		delete m_UnitDNA;
}

void AuIntroAvatarWidget::Clear()
{
	m_UnitId = AuId(0);
	if(m_UnitDNA)
	{
		delete m_UnitDNA;
		m_UnitDNA = 0;
	}
	m_Name->hide();
	m_NewDNA3->show();
	m_NewDNA4->show();
	SetCurrent(false);
}

void AuIntroAvatarWidget::SetAvatar(const AuId& id, const QString& name, const AuDNABase& dna, AuDNABase::CHROMATID chroma,
		HANDEDNESS hand)
{
	m_UnitChromatid = chroma;
	m_UnitDNA = dna.Clone();
	m_UnitId = id;
	m_UnitHand = hand;
	m_Name->setText(name);
	m_Name->setToolTip(tr("<b>%1</b> <i>%2</i>").arg(name).arg(id.StringId()));
	m_Name->show();
	m_NewDNA3->hide();
	m_NewDNA4->hide();

	m_Enter->setToolTip(QObject::tr("<b>Return to Hazeron as %1</b><br>"
				"Returns to the Hazeron universe in the persona of %1.").arg(name));
	if(m_UnitDNA->Version() == 3)
		m_Save->setToolTip(QObject::tr("<b>Save %1...</b><br>"
					"Saves %1 to a .chr file.").arg(name));
	else
		m_Save->setToolTip(QObject::tr("<b>Save %1...</b><br>"
					"Saves %1 to a .dna file.").arg(name));
	m_Save->setEnabled(true);
	m_Delete->setToolTip(QObject::tr("<b>Delete %1...</b><br>"
				"Deletes %1, after confirmation.").arg(name));
}

void AuIntroAvatarWidget::SetWhereabouts(const QString& where)
{
	if(where.isEmpty())
		m_Whereabouts->setText("Whereabouts Unknown");
	else
		m_Whereabouts->setText(where);
}

void AuIntroAvatarWidget::Enter()
{
	auApp->StopClientSounds();
	if(m_UnitId.IsNull())
		return;
	m_Delete->setEnabled(false);
	m_Enter->setEnabled(false);
	m_Save->setEnabled(false);
	emit EnterHazeron(m_UnitId);
}

void AuIntroAvatarWidget::Delete()
{
	AuId deleteId(m_UnitId);
	if(deleteId.IsNull())
		return;
	const DgData& gd = g_Au.LockDataForRead();
	QString sCharName = gd.ClientUnitName(deleteId);
	unsigned int confNum = gd.ConfirmationNumber();
	unsigned int idPlayer = gd.PlayerId();
	int avs = 0;
	for(int ai=0; ai<6; ai++)
		if(!gd.ClientUnitId(ai).IsNull())
			avs++;
	g_Au.UnlockData();
	QString sTitle = tr("Delete Avatar");
	int iResult;
	if(m_Solo && avs == 1)
	{
		iResult = QMessageBox::warning(this, sTitle,
				tr("<html><b>%1</b>"
					"<p>"
					"<b>Your avatar named %1 will be permanently deleted.</b> "
					"%1 will be removed from the <i>%2</i> universe. "
					"Mail messages sent to %1 will be removed from your computer."
					"</p>"
					"<p>"
					"<b>Blueprints created by %1 will be lost.</b> "
					"Blueprints created by %1 should be saved into .SoH files before doing this. "
					"</p>"
					"<p>"
					"<b>This is the last avatar so the universe will be deleted as well.</b> "
					"A new universe will be created when a new avatar is created. "
					"</p>"
					"<p>"
					"There is no way to undo this action.</p>"
					"</p>"
					"Are you sure you want to delete <b>%1</b>?</html>").arg(sCharName).arg(g_Au.ApplicationName()),
				QMessageBox::Yes, QMessageBox::No);
	}
	else
	{
		iResult = QMessageBox::warning(this, sTitle,
				tr("<html><b>%1</b>"
					"<p>"
					"<b>Your avatar named %1 will be permanently deleted.</b> "
					"%1 will be removed from the <i>%2</i> universe. "
					"Mail messages sent to %1 will be removed from your computer."
					"</p>"
					"<p>"
					"<b>Blueprints created by %1 will become inaccessible.</b> "
					"Blueprints created by %1 should be saved into .SoH files before doing this. "
					"</p>"
					"<p>"
					"There is no way to undo this action."
					"</p>"
					"Are you sure you want to delete <b>%1</b>?</html>").arg(sCharName).arg(g_Au.ApplicationName()),
				QMessageBox::Yes, QMessageBox::No);
	}
	if(iResult == QMessageBox::Yes)
	{
		DgData& gd = g_Au.LockDataForWrite();
		gd.ClientUnitDeleted(deleteId);
		g_Au.UnlockData();
		AuMail::DeleteLocalMail(deleteId);
		if(!m_Solo)
		{
			AuLoginComm login;
			login.DeletePlayerUnit(idPlayer, confNum, deleteId);
		}
		Clear();
		emit Deleted(deleteId);
	}
}

void AuIntroAvatarWidget::SetCurrent(bool isCurrent)
{
	m_Name->setChecked(isCurrent && !m_UnitId.IsNull());
	m_NewDNA3->setChecked(isCurrent && !m_UnitId.IsNull());
	m_NewDNA4->setChecked(isCurrent && !m_UnitId.IsNull());
	if(isCurrent)
	{
		m_Delete->setVisible(!m_UnitId.IsNull());
		m_Enter->setVisible(!m_UnitId.IsNull());
		m_Save->setVisible(!m_UnitId.IsNull());
		m_Whereabouts->setVisible(!m_UnitId.IsNull());
	}
	else
	{
		m_Delete->setVisible(false);
		m_Enter->setVisible(false);
		m_Save->setVisible(false);
		m_Whereabouts->setVisible(false);
	}
}

void AuIntroAvatarWidget::Select()
{
	emit Selected(m_UnitIndex);
}

void AuIntroAvatarWidget::SelectDNA3()
{
	auApp->StopClientSounds();
	emit NewDNA3(m_UnitIndex);
}

void AuIntroAvatarWidget::SelectDNA4()
{
	auApp->StopClientSounds();
	emit NewDNA4(m_UnitIndex);
}

void AuIntroAvatarWidget::Save()
{
	if(!m_UnitDNA)
		return;
	if(m_UnitDNA->Version() == 3)
	{
		// Load last known file folder.
		QString lastFile = g_Au.Settings().readEntry("/Settings/LastChrFile", "");
		if(lastFile.isEmpty())
			lastFile = QDir::currentPath();
		QFileInfo lastFileInfo(lastFile);
		if(!lastFileInfo.exists())
			lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

		// Ask the user for a file name.
		QString sFilter;
		sFilter.append("Avatar File (*.chr)");
		QString sFile = QFileDialog::getSaveFileName(this, "Save As...", lastFile, sFilter, 0,
				g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
		if(!sFile.isEmpty())
		{
			// Store last known file folder.
			g_Au.Settings().writeEntry("/Settings/LastChrFile", sFile);

			// Open/create the file.
			QFile file(sFile);
			if(file.open(QFile::WriteOnly))
			{
				// Store the file data.
				QByteArray baDNA;
				baDNA.resize(sizeof(DhDNA_));
				DhDNA_ dna3(*((DhDNA*)m_UnitDNA));
				memcpy(baDNA.data(), &dna3, sizeof(DhDNA_));

				QDataStream ds(&file);
				ds.setVersion(QDataStream::Qt_4_1);

				// File Type and Version Bytes
				const quint8 chrFileType = CHR_FILE_TYPE;	// Not 1 or 0 so we can identify old .chr files.
				const quint8 chrFileVersion = CHR_FILE_VER;	// First version stored in a .chr file.
				ds << chrFileType;
				ds << chrFileVersion;

				// Avatar Data
				ds << baDNA;
				ds << (quint8)(m_UnitChromatid == AuDNABase::Alpha ? Male : Female);
				ds << (quint8)(m_UnitHand == LeftHanded ? true : false);

				file.close();
			}
		}
	}
	else if(m_UnitDNA->Version() == 4)
	{
		QString lastFile = g_Au.Settings().readEntry("Settings/LastDNAFile", "");
		if(lastFile.isEmpty())
			lastFile = QDir::currentPath();

		QFileInfo lastFileInfo(lastFile);
		if(!lastFileInfo.exists())
			lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

		QString saveFilter;
		saveFilter += QString("Hazeron DNA (*.dna)");
		saveFilter += QString(";;All Files (*.*)");

		QString saveFile = QFileDialog::getSaveFileName(this, tr("Hazeron DNA File"), lastFile, saveFilter, 0,
				QFileDialog::DontUseNativeDialog);
		if(saveFile.isEmpty())
			return;

		// Insure the file has an appropriate suffix.
		QString suffix(".dna");
		if(saveFile.right(suffix.length()).compare(suffix, Qt::CaseInsensitive) != 0)
			saveFile += suffix;

		g_Au.Settings().writeEntry("Settings/LastDNAFile", saveFile);

		int result = ((const AuDNA*)m_UnitDNA)->SaveFile(saveFile);
		if(result == 1)
		{
			QString smsg = QObject::tr("Unable to open file for writing.\n\n%1").arg(saveFile);
			QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok);
		}
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroChooseAvatar::AuIntroChooseAvatar(QWidget* parent) : QWidget(parent)
{
	m_AvatarSampleRendered = 0;

	QToolButton* exitButton = new QToolButton(this);
	exitButton->setAutoRaise(true);
	exitButton->setIcon(QIcon(":data/ExitGame.png"));
	exitButton->setIconSize(QSize(32,32));
	exitButton->setToolTip(QObject::tr("<b>Exit</b><br>"
				"Exits the <i>%1</i> program.").arg(g_Au.ApplicationName()));
	connect(exitButton, SIGNAL(clicked()), parent, SIGNAL(ExitGame()));

	QHBoxLayout* xlay = new QHBoxLayout;
	xlay->addWidget(exitButton);
	xlay->addStretch();
	xlay->setSpacing(2);
	xlay->setMargin(2);

	QVBoxLayout* alay = new QVBoxLayout;
	for(int av=0; av<6; av++)
	{
		m_Avatar[av] = new AuIntroAvatarWidget(false, av, this);
		m_Avatar[av]->setVisible(false);
		alay->addWidget(m_Avatar[av]);
		connect(m_Avatar[av], SIGNAL(Selected(int)), SLOT(AvatarSelected(int)));
		connect(m_Avatar[av], SIGNAL(NewDNA3(int)), SLOT(AvatarNewDNA3(int)));
		connect(m_Avatar[av], SIGNAL(NewDNA4(int)), SLOT(AvatarNewDNA4(int)));
		connect(m_Avatar[av], SIGNAL(EnterHazeron(AuId)), SIGNAL(AvatarLogin(AuId)));
		connect(m_Avatar[av], SIGNAL(Deleted(AuId)), SLOT(AvatarDeleted(AuId)));
	}

	alay->addStretch(1);
	alay->addLayout(xlay);
	alay->setSpacing(2);
	alay->setMargin(2);

	m_AvatarSample = new AuBodyRenderWidget(this);
	m_AvatarSample->hide();
	m_AvatarSample->SetShowScene(true);

	m_Pose = new QComboBox(this);
	m_Pose->hide();
#if !defined(QT_NO_DEBUG)
	m_Pose->addItem(tr("Unposed"), AuPose::Init);
#endif
	m_Pose->addItem(tr("Stand"), AuPose::Stand);
	m_Pose->addItem(tr("Walk"), AuPose::StandWalk);
	m_Pose->addItem(tr("Run"), AuPose::StandRun);
	m_Pose->addItem(tr("Tread Water"), AuPose::TreadWater);
	m_Pose->addItem(tr("Swim"), AuPose::Swim);
	m_Pose->addItem(tr("Swim Fast"), AuPose::SwimFast);
	m_Pose->addItem(tr("Prone"), AuPose::Prone);
	m_Pose->addItem(tr("Crouch"), AuPose::Crouch);
	m_Pose->addItem(tr("Sit on Floor"), AuPose::SitOnGround);
	m_Pose->addItem(tr("Sit in Chair"), AuPose::Post_Sit);
	m_Pose->addItem(tr("Berth, Facing Down"), AuPose::BerthBottomFaceDown);
	m_Pose->addItem(tr("Berth, Facing Left"), AuPose::BerthBottomFaceLeft);
	m_Pose->addItem(tr("Berth, Facing Up"), AuPose::BerthBottomFaceUp);
	m_Pose->addItem(tr("Berth, Facing Right"), AuPose::BerthBottomFaceRight);
	m_Pose->addItem(tr("Head Only"), AuPose::EyesOnly);
#if !defined(QT_NO_DEBUG)
	m_Pose->addItem(tr("Mating (debug)"), AuPose::ReceptiveMate);
	m_Pose->addItem(tr("Mating on Top (debug)"), AuPose::MateOnTop);
#endif
	m_Pose->setToolTip(tr("<html>"
				"<p>"
				"The body is posed with the dominant arm bent. "
				"In the game, the dominant arm bends when holding an object. "
				"</p>"
				"</html>"));
	m_Pose->setCurrentIndex(m_Pose->findData(AuPose::Stand));
	connect(m_Pose, SIGNAL(currentIndexChanged(int)), SLOT(PoseChanged(int)));

	m_Mouth = new QComboBox(this);
	m_Mouth->hide();
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthOpenWide).Name()), AuPose::MouthOpenWide);
	m_Mouth->addItem(QString("%1 :D").arg(AuPose(AuPose::MouthGrin).Name()), AuPose::MouthGrin);
	m_Mouth->addItem(QString("%1 :)").arg(AuPose(AuPose::MouthSmile).Name()), AuPose::MouthSmile);
	m_Mouth->addItem(QString("%1 :O").arg(AuPose(AuPose::MouthAgape).Name()), AuPose::MouthAgape);
	m_Mouth->addItem(QString("%1 :P").arg(AuPose(AuPose::MouthSardonic).Name()), AuPose::MouthSardonic);
	m_Mouth->addItem(QString("%1 :o").arg(AuPose(AuPose::MouthOpen).Name()), AuPose::MouthOpen);
	m_Mouth->addItem(QString("%1 :|").arg(AuPose(AuPose::MouthClosed).Name()), AuPose::MouthClosed);
	m_Mouth->addItem(QString("%1 >.<").arg(AuPose(AuPose::MouthPursed).Name()), AuPose::MouthPursed);
	m_Mouth->addItem(QString("%1 :/").arg(AuPose(AuPose::MouthGrim).Name()), AuPose::MouthGrim);
	m_Mouth->addItem(QString("%1 :S").arg(AuPose(AuPose::MouthPerplexed).Name()), AuPose::MouthPerplexed);
	m_Mouth->addItem(QString("%1 :(").arg(AuPose(AuPose::MouthFrown).Name()), AuPose::MouthFrown);
	m_Mouth->addItem(QString("%1 D:").arg(AuPose(AuPose::MouthSad).Name()), AuPose::MouthSad);

	m_Mouth->addItem(QString("%1 lol").arg(AuPose(AuPose::MouthLaugh).Name()), AuPose::MouthLaugh);
	m_Mouth->addItem(QString("%1 :'(").arg(AuPose(AuPose::MouthCry).Name()), AuPose::MouthCry);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthTalk).Name()), AuPose::MouthTalk);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthJabber).Name()), AuPose::MouthJabber);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthWhisper).Name()), AuPose::MouthWhisper);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthYell).Name()), AuPose::MouthYell);

	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthBite).Name()), AuPose::MouthBite);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthYap).Name()), AuPose::MouthYap);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthYawn).Name()), AuPose::MouthYawn);
	m_Mouth->setToolTip(tr("<html>"
				"<p>"
				"<i>Visage</i> is your avatar's facial expression. "
				"In the game, your visage changes when you enter text into a comm channel. "
				"</p>"
				"<p>"
				"Your mouth moves to simulate talking. "
				"The <i>Talk</i> visage is used if you say ten words or less; "
				"<i>Jabber</i> if you say more. "
				"The <i>Yell</i> visage is used if all text is upper case; "
				"<i>Whisper</i> if all lower case. "
				"When finished, your face returns to the visage that was current before talking. "
				"</p>"
				"<p>"
				"Your visage also changes when you use emojis e.g. <b>😃</b> or emotes e.g. <b>:)</b> or <b>:D</b> or <b>rofl</b>. "
				"</p>"
				"</html>"));
	m_Mouth->setCurrentIndex(m_Mouth->findData(AuPose::MouthClosed));
	connect(m_Mouth, SIGNAL(currentIndexChanged(int)), SLOT(MouthChanged(int)));

	m_NewAvatarImage = new AuPictureWidget(true, this);
	m_NewAvatarImage->SetImage(QImage(":data/ButtonNewGS.png"));

	QHBoxLayout* poselay = new QHBoxLayout;
	poselay->addWidget(m_Pose);
	poselay->addWidget(m_Mouth);

	QVBoxLayout* play = new QVBoxLayout;
	play->addWidget(m_AvatarSample);
	play->addLayout(poselay);

	QHBoxLayout* glay = new QHBoxLayout;
	glay->addLayout(alay, 1);
	glay->addLayout(play, 1);
	glay->addWidget(m_NewAvatarImage, 1);

	QGroupBox* group = new QGroupBox(tr("Avatar"));
	group->setLayout(glay);

	// Build the final layout.
	QGridLayout* lay = new QGridLayout;
	int row=1;
	lay->addWidget(group, row,1, 2,1);
	row++;
	lay->setRowStretch(0, 1);
	lay->setRowStretch(row, 3);
	lay->setColumnStretch(0, 1);
	lay->setColumnStretch(1, 3);
	setLayout(lay);
}

AuIntroChooseAvatar::~AuIntroChooseAvatar()
{
}

void AuIntroChooseAvatar::SetAccountInfo()
{
	const DgData& gd = g_Au.LockDataForRead();
	for(int unitIndex=0; unitIndex<6; unitIndex++)
	{
		AuId unitId = gd.ClientUnitId(unitIndex);
		QString unitName = gd.ClientUnitName(unitId);
		const AuDNABase* dna = gd.ClientUnitDNA(unitId);

		if(unitId.IsNull() || unitName.isEmpty() || !dna)
			m_Avatar[unitIndex]->Clear();
		else
			m_Avatar[unitIndex]->SetAvatar(unitId, unitName, *dna, gd.ClientUnitChromatid(unitId), gd.ClientUnitHand(unitId));
		m_Avatar[unitIndex]->setVisible(true);
	}
	g_Au.UnlockData();

	// Default to last used account, otherwise use first account
	bool didSel = false;				// Set true if we actually call AvatarSelected.
	AuId idLastPlayer(g_Au.Settings().readEntry("/Account/LastPlayerId"));
	if(!idLastPlayer.IsNull())
	{
		for(int iu=0; iu<6; iu++)
		{
			if(m_Avatar[iu]->Id() == idLastPlayer)
			{
				AvatarSelected(iu);
				didSel = true;
				break;
			}
		}
	}
	if(!didSel)
		AvatarSelected(0);				// Select the first avatar slot.
}

void AuIntroChooseAvatar::AvatarSelected(int index)
{
	// Get Player info from unit id if necessary
	if(!m_Avatar[index]->Id().IsNull())
	{
		// Load unit values from local settings, based on their string id.
		QString unitStringId = m_Avatar[index]->Id().StringId();

		// Load last known location.
		QString section = QString("/Whereabouts/%1").arg(unitStringId);
		QString prevLoc = g_Au.Settings().readEntry(section, "");
		for(int av=0; av<6; av++)
			m_Avatar[av]->SetWhereabouts(prevLoc);

		// Load last known armor/clothing.
		const DgData& gd = g_Au.LockDataForRead();
		COMMODITYID armor = gd.ClientUnitArmor(m_Avatar[index]->Id());
		COMMODITYID clothes = gd.ClientUnitClothes(m_Avatar[index]->Id());
		if(clothes == CID_Invalid && !g_Au.IsNudityOk())
			clothes = CID_Underwear;
		COMMODITYID helmet = gd.ClientUnitHat(m_Avatar[index]->Id());
		quint8 visorState = gd.ClientUnitVisor(m_Avatar[index]->Id());
		AuHairStyle::STYLE haircut = gd.ClientUnitHaircut(m_Avatar[index]->Id());
		AuHairStyle::SHAVE shave = gd.ClientUnitShave(m_Avatar[index]->Id());
		g_Au.UnlockData();

		// Set DNA and invalidate GL model of body.
		m_NewAvatarImage->hide();
		m_AvatarSample->show();
		m_AvatarSample->SetId(m_Avatar[index]->Id());
		m_Pose->show();
		m_Mouth->show();
		const AuDNABase* dna = m_Avatar[index]->DNA();
		if(dna && dna->Version() == 3)
		{
			m_Mouth->setEnabled(false);
			if(DbCommodity::IsEnvironmentSuit(armor))
				m_AvatarSample->SetBody(*(DhDNA*)dna, m_Avatar[index]->Chromatid(), m_Avatar[index]->Handedness(), armor, helmet,
						visorState);
			else
				m_AvatarSample->SetBody(*(DhDNA*)dna, m_Avatar[index]->Chromatid(), m_Avatar[index]->Handedness(), clothes, helmet,
						visorState);
		}
		else if(dna && dna->Version() == 4)
		{
			m_Mouth->setEnabled(true);
			m_AvatarSample->SetAnimalia(*(AuDNA*)dna, m_Avatar[index]->Chromatid(), m_Avatar[index]->Handedness(), clothes, armor,
					helmet, visorState, haircut, shave);
		}
		AuPose pose((AuPose::POSE)m_Pose->currentData().toInt());
		AuPose mouth((AuPose::POSE)m_Mouth->currentData().toInt());
		m_AvatarSample->SetPose(pose, true, mouth);
	}

	// Update unit selection buttons.
	for(int unitButtonIndex=0; unitButtonIndex<6; unitButtonIndex++)
		m_Avatar[unitButtonIndex]->SetCurrent(index == unitButtonIndex);
}

void AuIntroChooseAvatar::AvatarNewDNA3(int index)
{
	// Get Player info from unit id if necessary
	if(m_Avatar[index]->Id().IsNull())
	{
		m_AvatarSample->hide();
		m_Pose->hide();
		m_Mouth->hide();
		m_NewAvatarImage->show();
		emit NewAvatarDNA3(index);
	}

	// Update unit selection buttons.
	for(int unitButtonIndex=0; unitButtonIndex<6; unitButtonIndex++)
		m_Avatar[unitButtonIndex]->SetCurrent(index == unitButtonIndex);
}

void AuIntroChooseAvatar::AvatarNewDNA4(int index)
{
	// Get Player info from unit id if necessary
	if(m_Avatar[index]->Id().IsNull())
	{
		m_AvatarSample->hide();
		m_Pose->hide();
		m_Mouth->hide();
		m_NewAvatarImage->show();
		emit NewAvatarDNA4(index);
	}

	// Update unit selection buttons.
	for(int unitButtonIndex=0; unitButtonIndex<6; unitButtonIndex++)
		m_Avatar[unitButtonIndex]->SetCurrent(index == unitButtonIndex);
}

void AuIntroChooseAvatar::UpdateGL()
{
	if(m_AvatarSampleRendered >= 2)
		m_AvatarSample->Animate();
	else
		m_AvatarSampleRendered++;
	m_AvatarSample->update();
}

void AuIntroChooseAvatar::AvatarDeleted(AuId avatarId)
{
	Q_UNUSED(avatarId);
	m_AvatarSample->hide();
	m_Pose->hide();
	m_Mouth->hide();
	m_NewAvatarImage->show();
}

void AuIntroChooseAvatar::PoseChanged(int index)
{
	AuPose pose((AuPose::POSE)m_Pose->itemData(index).toInt());
	AuPose mouth((AuPose::POSE)m_Mouth->currentData().toInt());
	m_AvatarSample->SetPose(pose, true, mouth);
}

void AuIntroChooseAvatar::MouthChanged(int index)
{
	AuPose pose((AuPose::POSE)m_Pose->currentData().toInt());
	AuPose mouth((AuPose::POSE)m_Mouth->itemData(index).toInt());
	m_AvatarSample->SetPose(pose, true, mouth);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroSoloAvatar::AuIntroSoloAvatar(QWidget* parent) : QWidget(parent)
{
	m_AvatarSampleRendered = 0;

	QToolButton* exitButton = new QToolButton(this);
	exitButton->setAutoRaise(true);
	exitButton->setIcon(QIcon(":data/ExitGame.png"));
	exitButton->setIconSize(QSize(32,32));
	exitButton->setToolTip(QObject::tr("<b>Exit</b><br>"
				"Exits the <i>%1</i> program.").arg(g_Au.ApplicationName()));
	connect(exitButton, SIGNAL(clicked()), parent, SIGNAL(ExitGame()));

	QHBoxLayout* xlay = new QHBoxLayout;
	xlay->addWidget(exitButton);
	xlay->addStretch();
	xlay->setSpacing(2);
	xlay->setMargin(2);

	QVBoxLayout* alay = new QVBoxLayout;
	for(int av=0; av<6; av++)
	{
		m_Avatar[av] = new AuIntroAvatarWidget(true, av, this);
		m_Avatar[av]->setVisible(false);
		alay->addWidget(m_Avatar[av]);
		connect(m_Avatar[av], SIGNAL(Selected(int)), SLOT(AvatarSelected(int)));
		connect(m_Avatar[av], SIGNAL(NewDNA3(int)), SLOT(AvatarNewDNA3(int)));
		connect(m_Avatar[av], SIGNAL(NewDNA4(int)), SLOT(AvatarNewDNA4(int)));
		connect(m_Avatar[av], SIGNAL(EnterHazeron(AuId)), SIGNAL(AvatarLogin(AuId)));
		connect(m_Avatar[av], SIGNAL(Deleted(AuId)), SLOT(AvatarDeleted(AuId)));
	}

	alay->addStretch(1);
	alay->addLayout(xlay);
	alay->setSpacing(2);
	alay->setMargin(2);

	m_AvatarSample = new AuBodyRenderWidget(this);
	m_AvatarSample->hide();
	m_AvatarSample->SetShowScene(true);

	m_Pose = new QComboBox(this);
	m_Pose->hide();
#if !defined(QT_NO_DEBUG)
	m_Pose->addItem(tr("Unposed"), AuPose::Init);
#endif
	m_Pose->addItem(tr("Stand"), AuPose::Stand);
	m_Pose->addItem(tr("Walk"), AuPose::StandWalk);
	m_Pose->addItem(tr("Run"), AuPose::StandRun);
	m_Pose->addItem(tr("Tread Water"), AuPose::TreadWater);
	m_Pose->addItem(tr("Swim"), AuPose::Swim);
	m_Pose->addItem(tr("Swim Fast"), AuPose::SwimFast);
	m_Pose->addItem(tr("Prone"), AuPose::Prone);
	m_Pose->addItem(tr("Crouch"), AuPose::Crouch);
	m_Pose->addItem(tr("Sit on Floor"), AuPose::SitOnGround);
	m_Pose->addItem(tr("Sit in Chair"), AuPose::Post_Sit);
	m_Pose->addItem(tr("Berth, Facing Down"), AuPose::BerthBottomFaceDown);
	m_Pose->addItem(tr("Berth, Facing Left"), AuPose::BerthBottomFaceLeft);
	m_Pose->addItem(tr("Berth, Facing Up"), AuPose::BerthBottomFaceUp);
	m_Pose->addItem(tr("Berth, Facing Right"), AuPose::BerthBottomFaceRight);
	m_Pose->addItem(tr("Head Only"), AuPose::EyesOnly);
#if !defined(QT_NO_DEBUG)
	m_Pose->addItem(tr("Mating (debug)"), AuPose::ReceptiveMate);
	m_Pose->addItem(tr("Mating on Top (debug)"), AuPose::MateOnTop);
#endif
	m_Pose->setToolTip(tr("<html>"
				"<p>"
				"The body is posed with the dominant arm bent. "
				"In the game, the dominant arm bends when holding an object. "
				"</p>"
				"</html>"));
	m_Pose->setCurrentIndex(m_Pose->findData(AuPose::Stand));
	connect(m_Pose, SIGNAL(currentIndexChanged(int)), SLOT(PoseChanged(int)));

	m_Mouth = new QComboBox(this);
	m_Mouth->hide();
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthOpenWide).Name()), AuPose::MouthOpenWide);
	m_Mouth->addItem(QString("%1 :D").arg(AuPose(AuPose::MouthGrin).Name()), AuPose::MouthGrin);
	m_Mouth->addItem(QString("%1 :)").arg(AuPose(AuPose::MouthSmile).Name()), AuPose::MouthSmile);
	m_Mouth->addItem(QString("%1 :O").arg(AuPose(AuPose::MouthAgape).Name()), AuPose::MouthAgape);
	m_Mouth->addItem(QString("%1 :P").arg(AuPose(AuPose::MouthSardonic).Name()), AuPose::MouthSardonic);
	m_Mouth->addItem(QString("%1 :o").arg(AuPose(AuPose::MouthOpen).Name()), AuPose::MouthOpen);
	m_Mouth->addItem(QString("%1 :|").arg(AuPose(AuPose::MouthClosed).Name()), AuPose::MouthClosed);
	m_Mouth->addItem(QString("%1 >.<").arg(AuPose(AuPose::MouthPursed).Name()), AuPose::MouthPursed);
	m_Mouth->addItem(QString("%1 :/").arg(AuPose(AuPose::MouthGrim).Name()), AuPose::MouthGrim);
	m_Mouth->addItem(QString("%1 :S").arg(AuPose(AuPose::MouthPerplexed).Name()), AuPose::MouthPerplexed);
	m_Mouth->addItem(QString("%1 :(").arg(AuPose(AuPose::MouthFrown).Name()), AuPose::MouthFrown);
	m_Mouth->addItem(QString("%1 D:").arg(AuPose(AuPose::MouthSad).Name()), AuPose::MouthSad);

	m_Mouth->addItem(QString("%1 lol").arg(AuPose(AuPose::MouthLaugh).Name()), AuPose::MouthLaugh);
	m_Mouth->addItem(QString("%1 :'(").arg(AuPose(AuPose::MouthCry).Name()), AuPose::MouthCry);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthTalk).Name()), AuPose::MouthTalk);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthJabber).Name()), AuPose::MouthJabber);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthWhisper).Name()), AuPose::MouthWhisper);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthYell).Name()), AuPose::MouthYell);

	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthBite).Name()), AuPose::MouthBite);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthYap).Name()), AuPose::MouthYap);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthYawn).Name()), AuPose::MouthYawn);
	m_Mouth->setToolTip(tr("<html>"
				"<p>"
				"<i>Visage</i> is your avatar's facial expression. "
				"In the game, your visage changes when you enter text into a comm channel. "
				"</p>"
				"<p>"
				"Your mouth moves to simulate talking. "
				"The <i>Talk</i> visage is used if you say ten words or less; "
				"<i>Jabber</i> if you say more. "
				"The <i>Yell</i> visage is used if all text is upper case; "
				"<i>Whisper</i> if all lower case. "
				"When finished, your face returns to the visage that was current before talking. "
				"</p>"
				"<p>"
				"Your visage also changes when you use emojis e.g. <b>😃</b> or emotes e.g. <b>:)</b> or <b>:D</b> or <b>rofl</b>. "
				"</p>"
				"</html>"));
	m_Mouth->setCurrentIndex(m_Mouth->findData(AuPose::MouthClosed));
	connect(m_Mouth, SIGNAL(currentIndexChanged(int)), SLOT(MouthChanged(int)));

	m_NewAvatarImage = new AuPictureWidget(true, this);
	m_NewAvatarImage->SetImage(QImage(":data/ButtonNewGS.png"));

	QHBoxLayout* poselay = new QHBoxLayout;
	poselay->addWidget(m_Pose);
	poselay->addWidget(m_Mouth);

	QVBoxLayout* play = new QVBoxLayout;
	play->addWidget(m_AvatarSample);
	play->addLayout(poselay);

	QHBoxLayout* glay = new QHBoxLayout;
	glay->addLayout(alay, 1);
	glay->addLayout(play, 1);
	glay->addWidget(m_NewAvatarImage, 1);

	QGroupBox* group = new QGroupBox(tr("Avatar"));
	group->setLayout(glay);

	// Build the final layout.
	QGridLayout* lay = new QGridLayout;
	int row=1;
	lay->addWidget(group, row,1, 2,1);
	row++;
	lay->setRowStretch(0, 1);
	lay->setRowStretch(row, 3);
	lay->setColumnStretch(0, 1);
	lay->setColumnStretch(1, 3);
	setLayout(lay);
}

AuIntroSoloAvatar::~AuIntroSoloAvatar()
{
}

void AuIntroSoloAvatar::SetAccountInfo()
{
	if(g_Au.m_LogMessages & LOG_Debug)
		g_Au.Log(QObject::tr("Preparing the depths of space."));

	int unitIndex = 0;
	{
		// GD file
		DgData& gd = g_Au.LockDataForWrite();
		int result = gd.LoadBasicGameData();
		if(result == 1)
			qWarning() << "error: Basic game data file was not found. This program will crash.";
		else if(result == 2)
			qWarning() << "error: Basic game data file could not be opened. This program will crash.";
		else if(result == 3)
			qWarning() << "error: Basic game data file is broken. This program will crash.";
		gd.SoloAdjustGameData();
		g_Au.UnlockData();
	}

	// Try loading the solo universe database.
	if(auApp->RestoreSolitaireUniverse())
	{
		// Successfully loaded the solo universe.
		// Load solo avatar info.
		DaPerson* avatars[6];
		int result = auApp->RestoreSolitaireAvatar(avatars);
		if(result == 0)
		{
			// Create bogus account credentials for our restored avatars.
			AuId idUnits[6];
			QString names[6];
			const AuDNABase* dna[6];
			AuDNABase::CHROMATID chroma[6];
			HANDEDNESS hand[6];
			COMMODITYID clothes[6];
			COMMODITYID armor[6];
			COMMODITYID hat[6];
			quint8 visor[6];
			AuHairStyle::STYLE hair[6];
			AuHairStyle::SHAVE shave[6];
			for(int ai=0; ai<6; ai++)
			{
				DaPerson* av = avatars[ai];
				if(av)
				{
					idUnits[ai] = av->Id();
					names[ai] = av->Name();
					dna[ai] = av->DNAnew();
					chroma[ai] = av->Chromatid();
					hand[ai] = av->IsLeftHanded() ? LeftHanded : RightHanded;
					clothes[ai] = av->WornClothing();
					armor[ai] = av->WornArmor();
					hat[ai] = av->WornHat();
					visor[ai] = av->WornHelmetState();
					hair[ai] = av->Haircut();
					shave[ai] = av->Shave();
					delete av;
				}
				else
				{
					idUnits[ai] = AuId();
					names[ai] = QString();
					dna[ai] = 0;
					chroma[ai] = AuDNABase::Alpha;
					hand[ai] = LeftHanded;
					clothes[ai] = CID_Invalid;
					armor[ai] = CID_Invalid;
					hat[ai] = CID_Invalid;
					visor[ai] = 0;
					hair[ai] = AuHairStyle::Unkempt;
					shave[ai] = AuHairStyle::Unshaven;
				}
			}

#if !defined(QT_NO_DEBUG)
			ACCOUNT acctType = ACCT_Architect;		// Debug build plays in architect mode.
#else
			ACCOUNT acctType = ACCT_Active;			// Release build plays in normal player mode.
#endif

			DgData& gd = g_Au.LockDataForWrite();
			gd.SetClientInfo(1, 1, idUnits, names, dna, chroma, hand, clothes, armor, hat, visor, hair, shave, acctType, AuTime());
			gd.SetActiveUnit(idUnits[unitIndex]);
			g_Au.UnlockData();

			for(int ai=0; ai<6; ai++)
				if(dna[ai])
					delete dna[ai];
		}
	}
	else
	{
		// No solo universe loaded. Insure cache is clean for a nu universe.
		g_Au.ClearSoloCacheFiles();
	}

	const DgData& gd = g_Au.LockDataForRead();
	for(int unitIndex=0; unitIndex<6; unitIndex++)
	{
		AuId unitId = gd.ClientUnitId(unitIndex);
		QString unitName = gd.ClientUnitName(unitId);
		const AuDNABase* dna = gd.ClientUnitDNA(unitId);

		if(unitId.IsNull() || unitName.isEmpty() || !dna)
			m_Avatar[unitIndex]->Clear();
		else
			m_Avatar[unitIndex]->SetAvatar(unitId, unitName, *dna, gd.ClientUnitChromatid(unitId), gd.ClientUnitHand(unitId));
		m_Avatar[unitIndex]->setVisible(true);
	}
	g_Au.UnlockData();

	// Default to last used account, otherwise use first account
	bool didSel = false;				// Set true if we actually call AvatarSelected.
	AuId idLastPlayer(g_Au.Settings().readEntry("/Account/LastSoloPlayerId"));
	if(!idLastPlayer.IsNull())
	{
		for(int iu=0; iu<6; iu++)
		{
			if(m_Avatar[iu]->Id() == idLastPlayer)
			{
				AvatarSelected(iu);
				didSel = true;
				break;
			}
		}
	}
	if(!didSel)
		AvatarSelected(0);				// Select the first avatar slot.
}

void AuIntroSoloAvatar::AvatarSelected(int index)
{
	// Get Player info from unit id if necessary
	if(!m_Avatar[index]->Id().IsNull())
	{
		// Load unit values from local settings, based on their string id.
		QString unitStringId = m_Avatar[index]->Id().StringId();

		// Load last known location.
		QString section = QString("/Whereabouts/%1").arg(unitStringId);
		QString prevLoc = g_Au.Settings().readEntry(section, "");
		for(int av=0; av<6; av++)
			m_Avatar[av]->SetWhereabouts(prevLoc);

		// Load last known armor/clothing.
		const DgData& gd = g_Au.LockDataForRead();
		COMMODITYID armor = gd.ClientUnitArmor(m_Avatar[index]->Id());
		COMMODITYID clothes = gd.ClientUnitClothes(m_Avatar[index]->Id());
		if(clothes == CID_Invalid && !g_Au.IsNudityOk())
			clothes = CID_Underwear;
		COMMODITYID helmet = gd.ClientUnitHat(m_Avatar[index]->Id());
		quint8 visorState = gd.ClientUnitVisor(m_Avatar[index]->Id());
		AuHairStyle::STYLE haircut = gd.ClientUnitHaircut(m_Avatar[index]->Id());
		AuHairStyle::SHAVE shave = gd.ClientUnitShave(m_Avatar[index]->Id());
		g_Au.UnlockData();

		// Set DNA and invalidate GL model of body.
		m_NewAvatarImage->hide();
		m_AvatarSample->show();
		m_AvatarSample->SetId(m_Avatar[index]->Id());
		m_Pose->show();
		m_Mouth->show();
		const AuDNABase* dna = m_Avatar[index]->DNA();
		if(dna && dna->Version() == 3)
		{
			m_Mouth->setEnabled(false);
			if(DbCommodity::IsEnvironmentSuit(armor))
				m_AvatarSample->SetBody(*(DhDNA*)dna, m_Avatar[index]->Chromatid(), m_Avatar[index]->Handedness(), armor, helmet,
						visorState);
			else
				m_AvatarSample->SetBody(*(DhDNA*)dna, m_Avatar[index]->Chromatid(), m_Avatar[index]->Handedness(), clothes, helmet,
						visorState);
		}
		else if(dna && dna->Version() == 4)
		{
			m_Mouth->setEnabled(true);
			m_AvatarSample->SetAnimalia(*(AuDNA*)dna, m_Avatar[index]->Chromatid(), m_Avatar[index]->Handedness(), clothes, armor,
					helmet, visorState, haircut, shave);
		}
		AuPose pose((AuPose::POSE)m_Pose->currentData().toInt());
		AuPose mouth((AuPose::POSE)m_Mouth->currentData().toInt());
		m_AvatarSample->SetPose(pose, true, mouth);
	}

	// Update unit selection buttons.
	for(int unitButtonIndex=0; unitButtonIndex<6; unitButtonIndex++)
		m_Avatar[unitButtonIndex]->SetCurrent(index == unitButtonIndex);
}

void AuIntroSoloAvatar::AvatarNewDNA3(int index)
{
	// Get Player info from unit id if necessary
	if(m_Avatar[index]->Id().IsNull())
	{
		m_AvatarSample->hide();
		m_Pose->hide();
		m_Mouth->hide();
		m_NewAvatarImage->show();
		emit NewAvatarDNA3(index);
	}

	// Update unit selection buttons.
	for(int unitButtonIndex=0; unitButtonIndex<6; unitButtonIndex++)
		m_Avatar[unitButtonIndex]->SetCurrent(index == unitButtonIndex);
}

void AuIntroSoloAvatar::AvatarNewDNA4(int index)
{
	// Get Player info from unit id if necessary
	if(m_Avatar[index]->Id().IsNull())
	{
		m_AvatarSample->hide();
		m_Pose->hide();
		m_Mouth->hide();
		m_NewAvatarImage->show();
		emit NewAvatarDNA4(index);
	}

	// Update unit selection buttons.
	for(int unitButtonIndex=0; unitButtonIndex<6; unitButtonIndex++)
		m_Avatar[unitButtonIndex]->SetCurrent(index == unitButtonIndex);
}

void AuIntroSoloAvatar::UpdateGL()
{
	if(m_AvatarSampleRendered >= 2)
		m_AvatarSample->Animate();
	else
		m_AvatarSampleRendered++;
	m_AvatarSample->update();
}

void AuIntroSoloAvatar::AvatarDeleted(AuId avatarId)
{
	m_AvatarSample->hide();
	m_Pose->hide();
	m_Mouth->hide();
	m_NewAvatarImage->show();
	auApp->DeleteSolitaireAvatar(avatarId);
}

void AuIntroSoloAvatar::PoseChanged(int index)
{
	AuPose pose((AuPose::POSE)m_Pose->itemData(index).toInt());
	AuPose mouth((AuPose::POSE)m_Mouth->currentData().toInt());
	m_AvatarSample->SetPose(pose, true, mouth);
}

void AuIntroSoloAvatar::MouthChanged(int index)
{
	AuPose pose((AuPose::POSE)m_Pose->currentData().toInt());
	AuPose mouth((AuPose::POSE)m_Mouth->itemData(index).toInt());
	m_AvatarSample->SetPose(pose, true, mouth);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroEmpireSpecies::AuIntroEmpireSpecies(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Empire DNA"));
	setAuDlgIcon(QIcon(":data/DNA.png"));

	m_Empires = new QTableWidget(this);
	m_Empires->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_Empires->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_Empires->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Empires->setShowGrid(false);
	QStringList hls;
	hls << tr("Empire") << tr("Capital Citizen");
	m_Empires->setColumnCount(hls.count());
	m_Empires->setHorizontalHeaderLabels(hls);
	m_Empires->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
	m_Empires->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	m_Empires->setColumnWidth(0, 150);
	m_Empires->setColumnWidth(2, 200);
	QHeaderView* vhdr = m_Empires->verticalHeader();
	vhdr->hide();
	connect(m_Empires, SIGNAL(itemSelectionChanged()), SLOT(EmpireChanged()));

	m_OkButton = new QPushButton(tr("Ok"), this);
	m_OkButton->setAutoDefault(true);
	m_OkButton->setDefault(true);
	m_OkButton->setEnabled(false);
	connect(m_OkButton, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(m_OkButton);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_Empires);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	connect(parent, SIGNAL(EmpireDataReceived()), this, SLOT(EmpireDataReceived()));

	// Request empire data now that I exist.
	auApp->RequestEmpiresAll();
}

AuIntroEmpireSpecies::~AuIntroEmpireSpecies()
{
}

void AuIntroEmpireSpecies::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuIntroEmpireSpecies::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/IntroEmpireDNA/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/IntroEmpireDNA/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/IntroEmpireDNA/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/IntroEmpireDNA/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuIntroEmpireSpecies::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/IntroEmpireDNA/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/IntroEmpireDNA/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/IntroEmpireDNA/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/IntroEmpireDNA/SizeY",sz.height());
}

void AuIntroEmpireSpecies::EmpireDataReceived()
{
	QMap<QString, quint32> sortedE;
	const DgData& gd = g_Au.LockDataForRead();
	for(DgEmpireHash::const_iterator ei = gd.m_gEmpires.cbegin(); ei != gd.m_gEmpires.cend(); ei++)
		if(!(*ei).second.m_capitolSpecies.IsNull())
			sortedE.insert((*ei).second.Name().toUpper(), (*ei).second.m_id);

	int trow=0;
	QIcon icondna(":data/DNA.png");
	m_Empires->setSortingEnabled(false);
	m_Empires->setRowCount(sortedE.count());
	for(QMap<QString, quint32>::const_iterator si = sortedE.cbegin(); si != sortedE.cend(); si++, trow++)
	{
		DgEmpire emp;
		emp.m_id = *si;
		gd.Empire(emp);
		QIcon icon(QPixmap::fromImage(emp.m_flag));
		QString specie = gd.SpeciesName(emp.m_capitolSpecies, true);

		int col=0;
		QTableWidgetItem* twi;
		twi = new QTableWidgetItem(icon, emp.Name());
		twi->setData(Qt::UserRole, *si);
		m_Empires->setItem(trow, col++, twi);

		twi = new QTableWidgetItem(icondna, specie);
		twi->setData(Qt::UserRole, *si);
		m_Empires->setItem(trow, col++, twi);
	}
	m_Empires->setSortingEnabled(true);

	g_Au.UnlockData();
}

void AuIntroEmpireSpecies::EmpireChanged()
{
	bool selOk = false;
	QItemSelectionModel* ism = m_Empires->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				QTableWidgetItem* item = m_Empires->item((*si).row(), 0);
				if(item)
				{
					quint32 empireId = item->data(Qt::UserRole).toUInt();
					if(empireId)
					{
						const DgData& gd = g_Au.LockDataForRead();
						DgEmpireHash::const_iterator ei = gd.m_gEmpires.find(empireId);
						if(ei != gd.m_gEmpires.cend())
						{
							m_Species = (*ei).second.m_capitolSpecies;
							selOk = true;
						}
						g_Au.UnlockData();
					}
				}
				break;
			}
	}
	m_OkButton->setEnabled(selOk);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroCreateAvatar::AuIntroCreateAvatar(QWidget* parent) : QWidget(parent)
{
	m_AvatarIndex = -1;
	m_AvatarSampleRendered = 0;
	m_EmpireDNADialog = 0;
	m_Chromatid = AuDNABase::Alpha;
	m_Hand = RightHanded;
	m_Solo = false;

	m_Tabs = new AuTabColumn(this);
	connect(m_Tabs, SIGNAL(CurrentChanged(int)), SLOT(TabChanged(int)));

	m_Pages = new QStackedLayout();

	// Create all body configuration pages.
	{
		m_Tabs->AddTab(QIcon(":data/c_VacSuitHelmet.png"), tr("Avatar"));
		m_Individual = new AuIntroCreateIndividual(this);
		connect(m_Individual, SIGNAL(NameChanged()), SLOT(EnableControls()));
		connect(m_Individual, SIGNAL(ChromatidChanged()), SLOT(DNAChanged()));
		connect(m_Individual, SIGNAL(HandChanged()), SLOT(HandednessChanged()));
		m_Pages->addWidget(m_Individual);
	}
	{
		m_Tabs->AddTab(QIcon(":data/Brain.png"), tr("General"));
		m_General = new AuIntroCreateGeneral(this);
		connect(m_General, SIGNAL(DNAChanged()), SLOT(DNAChanged()));
		m_Pages->addWidget(m_General);
	}
	{
		m_Tabs->AddTab(QIcon(":data/wl_Eye.png"), tr("Head"));
		m_Head = new AuIntroCreateHead(this);
		connect(m_Head, SIGNAL(DNAChanged()), SLOT(DNAChanged()));
		m_Pages->addWidget(m_Head);
	}
	{
		m_Tabs->AddTab(QIcon(":data/wl_Torso.png"), tr("Body"));
		m_Body = new AuIntroCreateBody(this);
		connect(m_Body, SIGNAL(DNAChanged()), SLOT(DNAChanged()));
		m_Pages->addWidget(m_Body);
	}
	{
		m_Tabs->AddTab(QIcon(":data/wl_Hand.png"), tr("Arms"));
		m_Arms = new AuIntroCreateArms(this);
		connect(m_Arms, SIGNAL(DNAChanged()), SLOT(DNAChanged()));
		m_Pages->addWidget(m_Arms);
	}
	{
		m_Tabs->AddTab(QIcon(":data/wl_Foot.png"), tr("Legs"));
		m_Legs = new AuIntroCreateLegs(this);
		connect(m_Legs, SIGNAL(DNAChanged()), SLOT(DNAChanged()));
		m_Pages->addWidget(m_Legs);
	}
	{
		m_Tabs->AddTab(QIcon(":data/wl_Tail.png"), tr("Tail"));
		m_Tail = new AuIntroCreateTail(this);
		connect(m_Tail, SIGNAL(DNAChanged()), SLOT(DNAChanged()));
		m_Pages->addWidget(m_Tail);
	}
	{
		m_Tabs->AddTab(QIcon(":data/wl_Horns.png"), tr("Weapon"));
		m_Weapon = new AuIntroCreateWeapon(this);
		connect(m_Weapon, SIGNAL(DNAChanged()), SLOT(DNAChanged()));
		m_Pages->addWidget(m_Weapon);
	}

	m_Summary = new QLabel(this);
	m_Summary->setAlignment(Qt::AlignLeft|Qt::AlignTop);
	m_Summary->setWordWrap(true);

	m_BackButton = new QToolButton(this);
	m_BackButton->setAutoRaise(true);
	m_BackButton->setIcon(QIcon(":data/CommCrew.png"));
	m_BackButton->setIconSize(QSize(32,32));
	m_BackButton->setToolTip(QObject::tr("<b>Back</b><br>"
				"Returns to avatar selection."));
	connect(m_BackButton, SIGNAL(clicked()), SIGNAL(Back()));

	m_Enter = new QToolButton(this);
	m_Enter->setAutoRaise(true);
	m_Enter->setIcon(QIcon(":data/StartNewAvatar.png"));
	m_Enter->setIconSize(QSize(64,64));
	m_Enter->setToolTip(QObject::tr("<b>Go To Hazeron</b><br>"
				"This is the final step to create an avatar. "
				"<br>"
				"%1 Creates a homeworld. "
				"<br>"
				"%1 Creates an avatar there. "
				"<br>"
				"%1 Enters the Hazeron universe in the persona of your new avatar. "
				"<br>"
				"%1 This button will be disabled if any setting is not valid. ").arg(g_Au.MiddleDotSymbol()));
	connect(m_Enter, SIGNAL(clicked()), SLOT(Enter()));

	m_ResetDNA = new QToolButton(this);
	m_ResetDNA->setAutoRaise(true);
	m_ResetDNA->setIcon(QIcon(":data/Passenger.png"));
	m_ResetDNA->setIconSize(QSize(32,32));
	m_ResetDNA->setToolTip(QObject::tr("<b>Reset DNA</b><br>"
				"Selects a default set of DNA traits."));
	connect(m_ResetDNA, SIGNAL(clicked()), SLOT(Reset()));

	m_RandomDNA = new QToolButton(this);
	m_RandomDNA->setAutoRaise(true);
	m_RandomDNA->setIcon(QIcon(":data/Game.png"));
	m_RandomDNA->setIconSize(QSize(32,32));
	m_RandomDNA->setToolTip(QObject::tr("<b>Random DNA</b><br>"
				"Selects a random set of DNA traits. "
				"The result may not be valid. "
				"Adjustments may be needed."));
	connect(m_RandomDNA, SIGNAL(clicked()), SLOT(Random()));

	m_EmpireDNA = new QToolButton(this);
	m_EmpireDNA->setAutoRaise(true);
	m_EmpireDNA->setIcon(QIcon(":data/bCapitol.png"));
	m_EmpireDNA->setIconSize(QSize(32,32));
	m_EmpireDNA->setToolTip(QObject::tr("<b>Empire DNA</b><br>"
				"Selects DNA traits from the worker population at the capital city of an existing empire. "
				"The result may not be valid. "
				"Adjustments may be needed. "
				"<br>"
				"<b>This posts a query for data from the server. It could take a few minutes.</b>"));
	connect(m_EmpireDNA, SIGNAL(clicked()), SLOT(EmpireDNA()));

	m_Load = new QToolButton(this);
	m_Load->setAutoRaise(true);
	m_Load->setIcon(QIcon(":data/dp_Load.png"));
	m_Load->setIconSize(QSize(32,32));
	m_Load->setToolTip(QObject::tr("<b>Load Avatar...</b><br>"
				"Loads avatar settings from a .chr file."));
	connect(m_Load, SIGNAL(clicked()), SLOT(Load()));

	m_Save = new QToolButton(this);
	m_Save->setAutoRaise(true);
	m_Save->setIcon(QIcon(":data/dp_Save.png"));
	m_Save->setIconSize(QSize(32,32));
	m_Save->setToolTip(QObject::tr("<b>Save Avatar...</b><br>"
				"Saves avatar DNA to a .chr file."));
	connect(m_Save, SIGNAL(clicked()), SLOT(Save()));

	m_Scenarios = new AuIntroStartScenarios(this);
	connect(m_Scenarios, SIGNAL(OkToGoChanged()), SLOT(EnableControls()));

	QToolButton* exitButton = new QToolButton(this);
	exitButton->setAutoRaise(true);
	exitButton->setIcon(QIcon(":data/ExitGame.png"));
	exitButton->setIconSize(QSize(32,32));
	exitButton->setToolTip(QObject::tr("<b>Exit Game</b><br>"
				"Exits the <i>%1</i> game.").arg(g_Au.ApplicationName()));
	connect(exitButton, SIGNAL(clicked()), parent, SIGNAL(ExitGame()));

	m_AvatarSample = new AuBodyRenderWidget(this);
	m_AvatarSample->SetShowScene(true);

	m_Pose = new QComboBox(this);
#if !defined(QT_NO_DEBUG)
	m_Pose->addItem(tr("Unposed"), AuPose::Init);
#endif
	m_Pose->addItem(tr("Stand"), AuPose::Stand);
	m_Pose->addItem(tr("Walk"), AuPose::StandWalk);
	m_Pose->addItem(tr("Run"), AuPose::StandRun);
	m_Pose->addItem(tr("Tread Water"), AuPose::TreadWater);
	m_Pose->addItem(tr("Swim"), AuPose::Swim);
	m_Pose->addItem(tr("Swim Fast"), AuPose::SwimFast);
	m_Pose->addItem(tr("Prone"), AuPose::Prone);
	m_Pose->addItem(tr("Crouch"), AuPose::Crouch);
	m_Pose->addItem(tr("Sit on Floor"), AuPose::SitOnGround);
	m_Pose->addItem(tr("Sit in Chair"), AuPose::Post_Sit);
	m_Pose->addItem(tr("Berth, Facing Down"), AuPose::BerthBottomFaceDown);
	m_Pose->addItem(tr("Berth, Facing Left"), AuPose::BerthBottomFaceLeft);
	m_Pose->addItem(tr("Berth, Facing Up"), AuPose::BerthBottomFaceUp);
	m_Pose->addItem(tr("Berth, Facing Right"), AuPose::BerthBottomFaceRight);
	m_Pose->addItem(tr("Head Only"), AuPose::EyesOnly);
#if !defined(QT_NO_DEBUG)
	m_Pose->addItem(tr("Mating (debug)"), AuPose::ReceptiveMate);
	m_Pose->addItem(tr("Mating on Top (debug)"), AuPose::MateOnTop);
#endif
	m_Pose->setToolTip(tr("<html>"
				"<p>"
				"The body is posed with the dominant arm bent. "
				"In the game, the dominant arm bends when holding an object. "
				"</p>"
				"</html>"));
	m_Pose->setCurrentIndex(m_Pose->findData(AuPose::Stand));
	connect(m_Pose, SIGNAL(currentIndexChanged(int)), SLOT(PoseChanged(int)));

	QHBoxLayout* flay = new QHBoxLayout;
	flay->addStretch();
	flay->addWidget(m_ResetDNA);
	flay->addWidget(m_RandomDNA);
	flay->addWidget(m_EmpireDNA);
	flay->addStretch();
	flay->addWidget(m_Load);
	flay->addWidget(m_Save);
	flay->setSpacing(2);
	flay->setMargin(2);

	QGridLayout* tlay = new QGridLayout;
	int col=0;
	tlay->addWidget(m_BackButton, 0,col, 2,1);
	col++;
	tlay->addLayout(flay, 1,col);
	tlay->setColumnStretch(col, 1);
	tlay->setRowStretch(0, 1);
	tlay->setSpacing(2);
	tlay->setMargin(2);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(m_Tabs);
	blay->addLayout(m_Pages);
	blay->setSpacing(0);
	blay->setMargin(2);

	QVBoxLayout* clay = new QVBoxLayout;
	clay->addLayout(tlay);
	clay->addLayout(blay);
	clay->addWidget(m_Summary, 10);
	clay->setSpacing(2);
	clay->setMargin(2);

	QVBoxLayout* play = new QVBoxLayout;
	play->addWidget(m_AvatarSample);
	play->addWidget(m_Pose);

	QHBoxLayout* alay = new QHBoxLayout;
	alay->addLayout(clay);
	alay->addLayout(play);

	QHBoxLayout* xlay = new QHBoxLayout;
	xlay->addWidget(exitButton, 0, Qt::AlignLeft|Qt::AlignBottom);
	xlay->addWidget(m_Scenarios, 1);
	xlay->addWidget(m_Enter, 0, Qt::AlignRight|Qt::AlignBottom);
	xlay->setSpacing(2);
	xlay->setMargin(0);

	QVBoxLayout* glay = new QVBoxLayout;
	glay->addLayout(alay, 1);
	glay->addLayout(xlay);

	QGroupBox* group = new QGroupBox(tr("New Avatar"));
	group->setLayout(glay);

	// Build the final layout.
	QGridLayout* lay = new QGridLayout;
	int row=1;
	lay->addWidget(group, row,1, 2,1);
	row++;
	lay->setRowStretch(0, 1);
	lay->setRowStretch(row, 3);
	lay->setColumnStretch(0, 1);
	lay->setColumnStretch(1, 3);
	setLayout(lay);
}

AuIntroCreateAvatar::~AuIntroCreateAvatar()
{
}

void AuIntroCreateAvatar::TabChanged(int idx)
{
	m_Pages->setCurrentIndex(idx);
}

void AuIntroCreateAvatar::UpdateGL()
{
	if(m_AvatarSampleRendered == 0)
	{
		Reset();
		m_AvatarSampleRendered++;
	}
	else if(m_AvatarSampleRendered >= 2)
	{
		m_AvatarSample->Animate();
		if(m_AvatarSampleRendered == 2)
		{
			// First enable of controls with default DNA has to wait until the body is animated the first time.
			m_AvatarSampleRendered++;
			EnableControls();
		}
	}
	else
		m_AvatarSampleRendered++;
	m_AvatarSample->update();
}

void AuIntroCreateAvatar::Enter()
{
	if(m_AvatarIndex < 0 || m_AvatarIndex > 5)
		return;
	QString name = m_Individual->Name();
	if(!IsNameValid(name))
		return;
	if(m_AvatarSample->AvatarCheck() != 0)
		return;
	m_BackButton->setEnabled(false);
	m_EmpireDNA->setEnabled(false);
	m_Enter->setEnabled(false);
	m_Load->setEnabled(false);
	m_RandomDNA->setEnabled(false);
	m_ResetDNA->setEnabled(false);
	m_Save->setEnabled(false);
	emit NewAvatarLogin(m_AvatarIndex);
}

void AuIntroCreateAvatar::Load()
{
	// Load last known file folder.
	QString lastFile = g_Au.Settings().readEntry("/Settings/LastChrFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();
	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QFileDialog fileDialog(this, tr("Load Avatar File..."), lastFile);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	if(!g_Au.UseNativeFileDialog())
		fileDialog.setOptions(QFileDialog::DontUseNativeDialog);
	QStringList sFilters;
	sFilters.append("Avatar File (*.chr)");
	fileDialog.setNameFilters(sFilters);
	if(fileDialog.exec() != QDialog::Accepted)
		return;
	if(!fileDialog.selectedFiles().isEmpty())
	{
		// Store last known file folder.
		QString sFile = fileDialog.selectedFiles().first();
		g_Au.Settings().writeEntry("/Settings/LastChrFile", sFile);

		QFile file(sFile);
		if(file.open(QFile::ReadOnly))
		{
			QDataStream ds(&file);
			ds.setVersion(QDataStream::Qt_4_1);

			DhDNA_ dna;
			quint8 gender = 0;
			bool bLefty = false;

			// File Type Byte
			quint8 chrFileType;
			ds >> chrFileType;
			if(chrFileType == CHR_FILE_TYPE)
			{
				// Version Byte
				quint8 chrFileVersion;
				ds >> chrFileVersion;

				// Character Data
				QByteArray baDNA;
				ds >> baDNA;
				dna = DhDNA_(baDNA);

				ds >> gender;
				ds >> bLefty;
			}
			else
			{
				// Old 'Jacob' format file.
				file.seek(0);		// Restore file pos back to beginning.
				QByteArray baFileData = file.read(sizeof(DhDNA_));
				dna = DhDNA_(baFileData);
				QByteArray ba = file.read(sizeof(bool)*2);
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds >> gender;
				ds >> bLefty;

				// Update size values for DNA changes that doubled length and thickness values. D.H.H. 12/22/2010
				if(dna.CalfLength*2 > 31)
					dna.CalfLength = 31;
				else
					dna.CalfLength *= 2;

				if(dna.CalfThickness*2 > 31)
					dna.CalfThickness = 31;
				else
					dna.CalfThickness *= 2;

				if(dna.ForeArmLength*2 > 31)
					dna.ForeArmLength = 31;
				else
					dna.ForeArmLength *= 2;

				if(dna.ForeArmThickness*2 > 31)
					dna.ForeArmThickness = 31;
				else
					dna.ForeArmThickness *= 2;

				if(dna.FootLength*2 > 31)
					dna.FootLength = 31;
				else
					dna.FootLength *= 2;

				if(dna.HandLength*2 > 31)
					dna.HandLength = 31;
				else
					dna.HandLength *= 2;

				if(dna.OverallSize*2 > 31)
					dna.OverallSize = 31;
				else
					dna.OverallSize *= 2;

				if(dna.ThighLength*2 > 31)
					dna.ThighLength = 31;
				else
					dna.ThighLength *= 2;

				if(dna.ThighThickness*2 > 31)
					dna.ThighThickness = 31;
				else
					dna.ThighThickness *= 2;

				if(dna.TorsoSegmentLength*2 > 31)
					dna.TorsoSegmentLength = 31;
				else
					dna.TorsoSegmentLength *= 2;

				if(dna.TorsoThickness*2 > 31)
					dna.TorsoThickness = 31;
				else
					dna.TorsoThickness *= 2;

				if(dna.UpperArmLength*2 > 31)
					dna.UpperArmLength = 31;
				else
					dna.UpperArmLength *= 2;

				if(dna.UpperArmThickness*2 > 31)
					dna.UpperArmThickness = 31;
				else
					dna.UpperArmThickness *= 2;
			}
			file.close();
			SetAvatar(dna, gender == Male ? AuDNABase::Alpha : AuDNABase::Beta, bLefty ? LeftHanded : RightHanded);
		}
	}
}

void AuIntroCreateAvatar::SetAvatar(const DhDNA& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand)
{
	m_Chromatid = chroma;
	m_Hand = hand;
	SetDNA(dna);
	m_AvatarSample->ResetView();
	m_Individual->SetChromatid(chroma);
	m_Individual->SetHandedness(hand);
}

void AuIntroCreateAvatar::Save()
{
	// Load last known file folder.
	QString lastFile = g_Au.Settings().readEntry("/Settings/LastChrFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();
	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	// Ask the user for a file name.
	QString sFilter;
	sFilter.append("Avatar File (*.chr)");
	QString sFile = QFileDialog::getSaveFileName(this, "Save As...", lastFile, sFilter, 0,
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(!sFile.isEmpty())
	{
		// Store last known file folder.
		g_Au.Settings().writeEntry("/Settings/LastChrFile", sFile);

		// Open/create the file.
		QFile file(sFile);
		if(file.open(QFile::WriteOnly))
		{
			// Store the file data.
			QByteArray baDNA;
			baDNA.resize(sizeof(DhDNA_));
			DhDNA_ dna3(m_AvatarSample->DNA3());
			memcpy(baDNA.data(), &dna3, sizeof(DhDNA_));

			QDataStream ds(&file);
			ds.setVersion(QDataStream::Qt_4_1);

			// File Type and Version Bytes
			const quint8 chrFileType = CHR_FILE_TYPE;	// Not 1 or 0 so we can identify old .chr files.
			const quint8 chrFileVersion = CHR_FILE_VER;	// First version stored in a .chr file.
			ds << chrFileType;
			ds << chrFileVersion;

			// Avatar Data
			ds << baDNA;
			ds << (quint8)(m_Chromatid == AuDNABase::Alpha ? Male : Female);
			ds << (quint8)(m_Hand == LeftHanded ? true : false);

			file.close();
		}
	}
}

void AuIntroCreateAvatar::EmpireDNA()
{
	if(!m_EmpireDNADialog)
		m_EmpireDNADialog = new AuIntroEmpireSpecies(this);
	if(m_EmpireDNADialog->exec() != QDialog::Accepted)
		return;
	AuDNABase* dna = m_EmpireDNADialog->Species().CreateDNA();
	if(!dna)
		return;
	DhDNA* dna3 = dynamic_cast<DhDNA*>(dna);
	if(!dna3)
	{
		delete dna;
		return;
	}
	SetDNA(*dna3);
	delete dna;
}

void AuIntroCreateAvatar::Random()
{
	AuDice dice;
	dice.TimeSeed();
	DhDNA dna;
	dna.Randomize(0, true, TOD_Diurnal, dice, true);
	dna.LivesOnLand = 1;
	SetDNA(dna);
}

void AuIntroCreateAvatar::Reset()
{
	DhDNA dna;
	dna.InitDefaultHuman();
	SetDNA(dna);
}

void AuIntroCreateAvatar::EnableControls()
{
	m_EmpireDNA->setVisible(!m_Solo);

	if(m_AvatarSample->DNAVersion() != 3)
		return;

	QString name = m_Individual->Name();
	int nameOk;
	if(name.isEmpty())
		nameOk = 1;	// blank
	else if(!IsNameValid(name))
		nameOk = 2;	// invalid
	else
		nameOk = 0;	// good
	int dnaCheck = m_AvatarSample->AvatarCheck();
	bool scenarioOk = m_Scenarios->OkToGo();
	m_Enter->setEnabled(nameOk == 0 && dnaCheck == 0 && scenarioOk);
	if(m_AvatarSampleRendered < 1)
		return;

	AuItemDNA mySkin(&m_AvatarSample->DNA3(), m_Chromatid, true, false, m_Hand, Mature, AuHairStyle::Unkempt, AuHairStyle::Unshaven,
			CID_LeatherClothes, 128, 0, 1);
	float baseAbsorb, baseBlock;
	mySkin.CombatArmorEffectiveness(WM_Contact, WE_Generic, baseAbsorb, baseBlock);

	int hitPoints = m_AvatarSample->MaxHitPoints();
	int pointsUsed = m_AvatarSample->AbilityPointsUsed();
	int pointsLeft = 60 - pointsUsed;
	double cgHeight = m_AvatarSample->CGHeight();
	double headHeight = m_AvatarSample->HeadHeight();
	double eyeHeight = m_AvatarSample->EyeHeight();

	// Weapon 1
	AuCombatWeapon weapon(WM_Not);
	AuDNABase::WEAPONLOC wLoc;
	m_AvatarSample->Body()->UseSpecialWeapon(AuTime::Now());	// Set use time now so BestWeapon won't pick spec wpn.
	m_AvatarSample->Body()->BestWeaponAtRange(0, wLoc, weapon);
	QString physDmg;
	if(weapon.m_Range > 0)
	{
		if(weapon.m_w1.m_Radius > 0)
			physDmg = tr("%1 to %2 hp, range %3m, %4m blast radius")
				.arg(weapon.m_w1.m_DamageMod)
				.arg(weapon.m_w1.m_Damage + weapon.m_w1.m_DamageMod)
				.arg(weapon.m_Range / FeetPerMeter(), 0,'f',1)
				.arg(weapon.m_w1.m_Radius / FeetPerMeter(), 0,'f',1);
		else
			physDmg = tr("%1 to %2 hp, range %3m")
				.arg(weapon.m_w1.m_DamageMod)
				.arg(weapon.m_w1.m_Damage + weapon.m_w1.m_DamageMod)
				.arg(weapon.m_Range / FeetPerMeter(), 0,'f',1);
	}
	else
	{
		if(weapon.m_w1.m_Radius > 0)
			physDmg = tr("%1 to %2 hp, %4m blast radius")
				.arg(weapon.m_w1.m_DamageMod)
				.arg(weapon.m_w1.m_Damage + weapon.m_w1.m_DamageMod)
				.arg(weapon.m_w1.m_Radius / FeetPerMeter(), 0,'f',1);
		else
			physDmg = tr("%1 to %2 hp")
				.arg(weapon.m_w1.m_DamageMod)
				.arg(weapon.m_w1.m_Damage + weapon.m_w1.m_DamageMod);
	}

	// Weapon 2
	AuTime bday = AuTime::Now().AddHours(-1);					// Past time to force special weapon to appear not recently used.
	m_AvatarSample->Body()->UseSpecialWeapon(bday);			// Set use time an hour ago so BestWeapon can pick spec wpn.
	m_AvatarSample->Body()->BestWeaponAtRange(0, wLoc, weapon);
	QString specDmg;
	if(m_AvatarSample->DNA3().WeaponType != DhDNA::WT_None)
	{
		if(weapon.m_Range > 0)
		{
			if(weapon.m_w1.m_Radius > 0)
				specDmg = tr("%1 to %2 hp, range %3m, %4m blast radius")
					.arg(weapon.m_w1.m_DamageMod)
					.arg(weapon.m_w1.m_Damage + weapon.m_w1.m_DamageMod)
					.arg(weapon.m_Range / FeetPerMeter(), 0,'f',1)
					.arg(weapon.m_w1.m_Radius / FeetPerMeter(), 0,'f',1);
			else
				specDmg = tr("%1 to %2 hp, range %3m")
					.arg(weapon.m_w1.m_DamageMod)
					.arg(weapon.m_w1.m_Damage + weapon.m_w1.m_DamageMod)
					.arg(weapon.m_Range / FeetPerMeter(), 0,'f',1);
		}
		else
		{
			if(weapon.m_w1.m_Radius > 0)
				specDmg = tr("%1 to %2 hp, %4m blast radius")
					.arg(weapon.m_w1.m_DamageMod)
					.arg(weapon.m_w1.m_Damage + weapon.m_w1.m_DamageMod)
					.arg(weapon.m_w1.m_Radius / FeetPerMeter(), 0,'f',1);
			else
				specDmg = tr("%1 to %2 hp")
					.arg(weapon.m_w1.m_DamageMod)
					.arg(weapon.m_w1.m_Damage + weapon.m_w1.m_DamageMod);
		}
	}

	// Build the summary text.
	QString sum = tr("<html>");
	QString note("%1");
	QString error("<span style=\"color: rgb(255, 32, 32);\">%1</span>");

	sum += "<p>";
	if(dnaCheck & 0x0001)
	{
		sum += error.arg(tr("Ability Points Available: <b>%1</b>").arg(pointsLeft));
		sum += error.arg(tr("<br>Ability Points Used: <b>%1</b>").arg(pointsUsed));
	}
	else
	{
		sum += note.arg(tr("Ability Points Available: <b>%1</b>").arg(pointsLeft));
		sum += note.arg(tr("<br>Ability Points Used: <b>%1</b>").arg(pointsUsed));
	}
	sum += "</p>";

	sum += "<p>";
	if(dnaCheck & 0x0002)
		sum += error.arg(tr("Hit Points: <b>%1</b>").arg(hitPoints));
	else
		sum += note.arg(tr("Hit Points: <b>%1</b>").arg(hitPoints));

	sum += note.arg(tr("<br>Armor: <b>%1</b>").arg(g_Au.FormatArmorFactors(baseAbsorb, baseBlock)));

	sum += note.arg(tr("<br>Physical Damage: <b>%1</b>").arg(physDmg));

	if(!specDmg.isEmpty())
		sum += note.arg(tr("<br>Special Damage: <b>%1</b>").arg(specDmg));
	sum += "</p>";

	sum += "<p>";
	if(dnaCheck & 0x0004)
		sum += error.arg(tr("Eye Height: <b>%1m</b>").arg(eyeHeight/FeetPerMeter(), 0,'f',2));
	else
		sum += note.arg(tr("Eye Height: <b>%1m</b>").arg(eyeHeight/FeetPerMeter(), 0,'f',2));

	if(dnaCheck & 0x0010)
		sum += error.arg(tr("<br>Head Height: <b>%1m</b>").arg(headHeight/FeetPerMeter(), 0,'f',2));
	else
		sum += note.arg(tr("<br>Head Height: <b>%1m</b>").arg(headHeight/FeetPerMeter(), 0,'f',2));

	if(dnaCheck & 0x0008)
		sum += error.arg(tr("<br>CG Height: <b>%1m</b>").arg(cgHeight/FeetPerMeter(), 0,'f',2));
	else
		sum += note.arg(tr("<br>CG Height: <b>%1m</b>").arg(cgHeight/FeetPerMeter(), 0,'f',2));
	sum += "</p>";

	if(nameOk)	// non-zero is an error
	{
		sum += "<p>";
		if(nameOk == 1)
			sum += error.arg(tr("Avatar name needed."));
		else if(nameOk == 2)
			sum += error.arg(tr("Avatar name is not valid."));
		sum += "</p>";
	}
	sum += QString("</html>");
	m_Summary->setText(sum);
}

void AuIntroCreateAvatar::setVisible(bool visible)
{
	QWidget::setVisible(visible);
	if(visible)
		DNAChanged();
}

void AuIntroCreateAvatar::HandednessChanged()
{
	m_Hand = m_Individual->Handedness();
	m_AvatarSample->SetHandedness(m_Hand);
	EnableControls();
}

void AuIntroCreateAvatar::DNAChanged()
{
	//qDebug() << "AuIntroCreateAvatar::DNAChanged";
	DhDNA dna;
	m_General->DNATraits(dna);
	m_Head->DNATraits(dna);
	m_Body->DNATraits(dna);
	m_Arms->DNATraits(dna);
	m_Legs->DNATraits(dna);
	m_Tail->DNATraits(dna);
	m_Weapon->DNATraits(dna);
	if(m_AvatarSample->DNAVersion() != 3 || dna.SkinType != m_AvatarSample->DNA3().SkinType)
	{
		// Phylum affects choices in combo boxes.
		m_Head->SetPhylum((DhDNA::SKIN)dna.SkinType);
		m_Tail->SetPhylum((DhDNA::SKIN)dna.SkinType);
	}
	m_Weapon->UpdateWeaponLocations(dna);
	m_AvatarSample->SetBody(dna, m_Individual->Chromatid(), m_Individual->Handedness(), CID_Invalid, CID_Invalid, 0);
	AuPose pose((AuPose::POSE)m_Pose->currentData().toInt());
	m_AvatarSample->SetPose(pose, true, AuPose::MouthClosed);	// Mouth n/a on gen3 avatars.
	EnableControls();
}

void AuIntroCreateAvatar::SetDNA(const DhDNA& dna)
{
	m_General->SetDNA(dna);
	m_Head->SetDNA(dna);
	m_Body->SetDNA(dna);
	m_Arms->SetDNA(dna);
	m_Legs->SetDNA(dna);
	m_Tail->SetDNA(dna);
	m_Weapon->SetDNA(dna);
	if(m_AvatarSample->DNAVersion() != 3 || dna.SkinType != m_AvatarSample->DNA3().SkinType)
	{
		// Phylum affects choices in combo boxes.
		m_Head->SetPhylum((DhDNA::SKIN)dna.SkinType);
		m_Tail->SetPhylum((DhDNA::SKIN)dna.SkinType);
	}
	m_Weapon->UpdateWeaponLocations(dna);
	m_AvatarSample->SetBody(dna, m_Chromatid, m_Hand, CID_Invalid, CID_Invalid, 0);
	AuPose pose((AuPose::POSE)m_Pose->currentData().toInt());
	m_AvatarSample->SetPose(pose, true, AuPose::MouthClosed);	// Mouth n/a on gen3 avatars.
	EnableControls();
}

void AuIntroCreateAvatar::EnterNewAvatar()
{
	if(m_AvatarSample->DNAVersion() != 3)
		return;
	const DgData& gd = g_Au.LockDataForRead();
	int avatarIndex = gd.ClientUnitAvailableIndex();
	g_Au.UnlockData();
	if(avatarIndex < 0)
		return;

	QString name = m_Individual->Name();
	AuDNABase::CHROMATID chroma = m_Individual->Chromatid();
	HANDEDNESS hand = m_Individual->Handedness();
	GALAXY galaxy = m_Scenarios->Galaxy();
	DhDNA dna(m_AvatarSample->DNA3());
	SCENARIO scenario = m_Scenarios->Scenario();
	quint8 proximity = m_Scenarios->Region();
	AuTime asteroidImpact;
	asteroidImpact = m_Scenarios->AsteroidImpact().toUTC();
	quint32 joinEmpire = m_Scenarios->EmpireToJoin();
	QByteArray joinPass = m_Scenarios->EmpireJoinPassCrypt();
	AuId joinSector = m_Scenarios->EmpireJoinSector();
	QString newEmpireName = m_Scenarios->NewEmpireName();
	QImage newEmpireFlag = m_Scenarios->NewEmpireFlag();
	if(m_Solo)
		auApp->StartNewAvatarSolo(avatarIndex, name, dna, chroma, hand,
				proximity, galaxy, scenario, asteroidImpact, joinEmpire, joinSector, newEmpireName, newEmpireFlag);
	else
		auApp->StartNewAvatar(name, dna, chroma, hand,
				proximity, galaxy, scenario, asteroidImpact, joinEmpire, joinPass, joinSector);
}

void AuIntroCreateAvatar::PoseChanged(int index)
{
	AuPose pose((AuPose::POSE)m_Pose->itemData(index).toInt());
	m_AvatarSample->SetPose(pose, true, AuPose::MouthClosed);	// Mouth n/a on gen3 avatars.
}

void AuIntroCreateAvatar::SetSolo(bool solo)
{
	m_Solo = solo;
	m_Individual->SetSolo(solo);
	m_Scenarios->SetSolo(solo);
	EnableControls();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroCreateAvatar4::AuIntroCreateAvatar4(QWidget* parent) : QWidget(parent)
{
	m_AvatarIndex = -1;
	m_AvatarSampleRendered = 0;
	m_Chromatid = AuDNABase::Alpha;
	m_EmpireDNADialog = 0;
	m_Hand = RightHanded;
	m_Solo = false;

	m_Individual = new AuIntroCreateIndividual4(this);
	connect(m_Individual, SIGNAL(NameChanged()), SLOT(EnableControls()));
	connect(m_Individual, SIGNAL(ChromatidChanged()), SLOT(DNAChanged()));
	connect(m_Individual, SIGNAL(HandChanged()), SLOT(HandednessChanged()));

	m_Summary = new QLabel(this);
	m_Summary->setAlignment(Qt::AlignLeft|Qt::AlignTop);
	m_Summary->setWordWrap(true);

	m_BackButton = new QToolButton(this);
	m_BackButton->setAutoRaise(true);
	m_BackButton->setIcon(QIcon(":data/CommCrew.png"));
	m_BackButton->setIconSize(QSize(32,32));
	m_BackButton->setToolTip(QObject::tr("<b>Back</b><br>"
				"Returns to avatar selection."));
	connect(m_BackButton, SIGNAL(clicked()), SIGNAL(Back()));

	m_Enter = new QToolButton(this);
	m_Enter->setAutoRaise(true);
	m_Enter->setIcon(QIcon(":data/StartNewAvatar.png"));
	m_Enter->setIconSize(QSize(64,64));
	m_Enter->setToolTip(QObject::tr("<b>Go To Hazeron</b><br>"
				"This is the final step to create an avatar. "
				"<br>"
				"%1 Creates a homeworld. "
				"<br>"
				"%1 Creates an avatar there. "
				"<br>"
				"%1 Enters the Hazeron universe in the persona of your new avatar. "
				"<br>"
				"%1 This button will be disabled if any setting is not valid. ").arg(g_Au.MiddleDotSymbol()));
	connect(m_Enter, SIGNAL(clicked()), SLOT(Enter()));

	m_CreateDNA = new QToolButton(this);
	m_CreateDNA->setAutoRaise(true);
	m_CreateDNA->setIcon(QIcon(":data/DNA.png"));
	m_CreateDNA->setIconSize(QSize(32,32));
	m_CreateDNA->setToolTip(QObject::tr("<b>Create DNA...</b><br>"
				"Creates a new avatar DNA."));
	connect(m_CreateDNA, SIGNAL(clicked()), SLOT(CreateDNA()));

	m_EmpireDNA = new QToolButton(this);
	m_EmpireDNA->setAutoRaise(true);
	m_EmpireDNA->setIcon(QIcon(":data/bCapitol.png"));
	m_EmpireDNA->setIconSize(QSize(32,32));
	m_EmpireDNA->setToolTip(QObject::tr("<b>Empire DNA</b><br>"
				"Selects DNA traits from the worker population at the capital city of an existing empire. "
				"The result may not be valid. "
				"Adjustments may be needed. "
				"<br>"
				"<b>This posts a query for data from the server. It could take a few minutes.</b>"));
	connect(m_EmpireDNA, SIGNAL(clicked()), SLOT(EmpireDNA()));

	m_Scenarios = new AuIntroStartScenarios(this);
	connect(m_Scenarios, SIGNAL(OkToGoChanged()), SLOT(EnableControls()));

	QToolButton* exitButton = new QToolButton(this);
	exitButton->setAutoRaise(true);
	exitButton->setIcon(QIcon(":data/ExitGame.png"));
	exitButton->setIconSize(QSize(32,32));
	exitButton->setToolTip(QObject::tr("<b>Exit Game</b><br>"
				"Exits the <i>%1</i> game.").arg(g_Au.ApplicationName()));
	connect(exitButton, SIGNAL(clicked()), parent, SIGNAL(ExitGame()));

	m_AvatarSample = new AuBodyRenderWidget(this);
	m_AvatarSample->SetShowScene(true);

	m_Pose = new QComboBox(this);
#if !defined(QT_NO_DEBUG)
	m_Pose->addItem(tr("Unposed"), AuPose::Init);
#endif
	m_Pose->addItem(tr("Stand"), AuPose::Stand);
	m_Pose->addItem(tr("Walk"), AuPose::StandWalk);
	m_Pose->addItem(tr("Run"), AuPose::StandRun);
	m_Pose->addItem(tr("Tread Water"), AuPose::TreadWater);
	m_Pose->addItem(tr("Swim"), AuPose::Swim);
	m_Pose->addItem(tr("Swim Fast"), AuPose::SwimFast);
	m_Pose->addItem(tr("Prone"), AuPose::Prone);
	m_Pose->addItem(tr("Crouch"), AuPose::Crouch);
	m_Pose->addItem(tr("Sit on Floor"), AuPose::SitOnGround);
	m_Pose->addItem(tr("Sit in Chair"), AuPose::Post_Sit);
	m_Pose->addItem(tr("Berth, Facing Down"), AuPose::BerthBottomFaceDown);
	m_Pose->addItem(tr("Berth, Facing Left"), AuPose::BerthBottomFaceLeft);
	m_Pose->addItem(tr("Berth, Facing Up"), AuPose::BerthBottomFaceUp);
	m_Pose->addItem(tr("Berth, Facing Right"), AuPose::BerthBottomFaceRight);
	m_Pose->addItem(tr("Head Only"), AuPose::EyesOnly);
#if !defined(QT_NO_DEBUG)
	m_Pose->addItem(tr("Mating (debug)"), AuPose::ReceptiveMate);
	m_Pose->addItem(tr("Mating on Top (debug)"), AuPose::MateOnTop);
#endif
	m_Pose->setToolTip(tr("<html>"
				"<p>"
				"The body is posed with the dominant arm bent. "
				"In the game, the dominant arm bends when holding an object. "
				"</p>"
				"</html>"));
	m_Pose->setCurrentIndex(m_Pose->findData(AuPose::Stand));
	connect(m_Pose, SIGNAL(currentIndexChanged(int)), SLOT(PoseChanged(int)));

	m_Mouth = new QComboBox(this);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthOpenWide).Name()), AuPose::MouthOpenWide);
	m_Mouth->addItem(QString("%1 :D").arg(AuPose(AuPose::MouthGrin).Name()), AuPose::MouthGrin);
	m_Mouth->addItem(QString("%1 :)").arg(AuPose(AuPose::MouthSmile).Name()), AuPose::MouthSmile);
	m_Mouth->addItem(QString("%1 :O").arg(AuPose(AuPose::MouthAgape).Name()), AuPose::MouthAgape);
	m_Mouth->addItem(QString("%1 :P").arg(AuPose(AuPose::MouthSardonic).Name()), AuPose::MouthSardonic);
	m_Mouth->addItem(QString("%1 :o").arg(AuPose(AuPose::MouthOpen).Name()), AuPose::MouthOpen);
	m_Mouth->addItem(QString("%1 :|").arg(AuPose(AuPose::MouthClosed).Name()), AuPose::MouthClosed);
	m_Mouth->addItem(QString("%1 >.<").arg(AuPose(AuPose::MouthPursed).Name()), AuPose::MouthPursed);
	m_Mouth->addItem(QString("%1 :/").arg(AuPose(AuPose::MouthGrim).Name()), AuPose::MouthGrim);
	m_Mouth->addItem(QString("%1 :S").arg(AuPose(AuPose::MouthPerplexed).Name()), AuPose::MouthPerplexed);
	m_Mouth->addItem(QString("%1 :(").arg(AuPose(AuPose::MouthFrown).Name()), AuPose::MouthFrown);
	m_Mouth->addItem(QString("%1 D:").arg(AuPose(AuPose::MouthSad).Name()), AuPose::MouthSad);

	m_Mouth->addItem(QString("%1 lol").arg(AuPose(AuPose::MouthLaugh).Name()), AuPose::MouthLaugh);
	m_Mouth->addItem(QString("%1 :'(").arg(AuPose(AuPose::MouthCry).Name()), AuPose::MouthCry);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthTalk).Name()), AuPose::MouthTalk);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthJabber).Name()), AuPose::MouthJabber);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthWhisper).Name()), AuPose::MouthWhisper);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthYell).Name()), AuPose::MouthYell);

	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthBite).Name()), AuPose::MouthBite);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthYap).Name()), AuPose::MouthYap);
	m_Mouth->addItem(QString("%1").arg(AuPose(AuPose::MouthYawn).Name()), AuPose::MouthYawn);
	m_Mouth->setToolTip(tr("<html>"
				"<p>"
				"<i>Visage</i> is your avatar's facial expression. "
				"In the game, your visage changes when you enter text into a comm channel. "
				"</p>"
				"<p>"
				"Your mouth moves to simulate talking. "
				"The <i>Talk</i> visage is used if you say ten words or less; "
				"<i>Jabber</i> if you say more. "
				"The <i>Yell</i> visage is used if all text is upper case; "
				"<i>Whisper</i> if all lower case. "
				"When finished, your face returns to the visage that was current before talking. "
				"</p>"
				"<p>"
				"Your visage also changes when you use emojis e.g. <b>😃</b> or emotes e.g. <b>:)</b> or <b>:D</b> or <b>rofl</b>. "
				"</p>"
				"</html>"));
	m_Mouth->setCurrentIndex(m_Mouth->findData(AuPose::MouthClosed));
	connect(m_Mouth, SIGNAL(currentIndexChanged(int)), SLOT(MouthChanged(int)));

	QHBoxLayout* flay = new QHBoxLayout;
	flay->addStretch();
	flay->addWidget(m_CreateDNA);
	flay->addWidget(m_EmpireDNA);
	flay->setSpacing(2);
	flay->setMargin(2);

	QGridLayout* tlay = new QGridLayout;
	int col=0;
	tlay->addWidget(m_BackButton, 0,col, 2,1);
	col++;
	tlay->addLayout(flay, 1,col);
	tlay->setColumnStretch(col, 1);
	tlay->setRowStretch(0, 1);
	tlay->setSpacing(2);
	tlay->setMargin(2);

	QVBoxLayout* clay = new QVBoxLayout;
	clay->addLayout(tlay);
	clay->addWidget(m_Individual);
	clay->addWidget(m_Summary, 10);
	clay->setSpacing(2);
	clay->setMargin(2);

	QHBoxLayout* poselay = new QHBoxLayout;
	poselay->addWidget(m_Pose);
	poselay->addWidget(m_Mouth);

	QVBoxLayout* play = new QVBoxLayout;
	play->addWidget(m_AvatarSample);
	play->addLayout(poselay);

	QHBoxLayout* alay = new QHBoxLayout;
	alay->addLayout(clay);
	alay->addLayout(play);

	QHBoxLayout* xlay = new QHBoxLayout;
	xlay->addWidget(exitButton, 0, Qt::AlignLeft|Qt::AlignBottom);
	xlay->addWidget(m_Scenarios, 1);
	xlay->addWidget(m_Enter, 0, Qt::AlignRight|Qt::AlignBottom);
	xlay->setSpacing(2);
	xlay->setMargin(0);

	QVBoxLayout* glay = new QVBoxLayout;
	glay->addLayout(alay, 1);
	glay->addLayout(xlay);

	QGroupBox* group = new QGroupBox(tr("New Avatar"));
	group->setLayout(glay);

	// Build the final layout.
	QGridLayout* lay = new QGridLayout;
	int row=1;
	lay->addWidget(group, row,1, 2,1);
	row++;
	lay->setRowStretch(0, 1);
	lay->setRowStretch(row, 3);
	lay->setColumnStretch(0, 1);
	lay->setColumnStretch(1, 3);
	setLayout(lay);
}

AuIntroCreateAvatar4::~AuIntroCreateAvatar4()
{
}

void AuIntroCreateAvatar4::UpdateGL()
{
	if(m_AvatarSampleRendered == 0)
	{
		AuDNA dna;
		dna.InitDefaultHuman();
		SetDNA(dna);
		m_AvatarSampleRendered++;
	}
	else if(m_AvatarSampleRendered >= 2)
	{
		m_AvatarSample->Animate();
		if(m_AvatarSampleRendered == 2)
		{
			// First enable of controls with default DNA has to wait until the body is animated the first time.
			m_AvatarSampleRendered++;
			EnableControls();
		}
	}
	else
		m_AvatarSampleRendered++;
	m_AvatarSample->update();
}

void AuIntroCreateAvatar4::Enter()
{
	if(m_AvatarIndex < 0 || m_AvatarIndex > 5)
		return;
	QString name = m_Individual->Name();
	if(!IsNameValid(name))
		return;
	if(m_AvatarSample->AvatarCheck() != 0)
		return;
	m_BackButton->setEnabled(false);
	m_EmpireDNA->setEnabled(false);
	m_Enter->setEnabled(false);
	m_CreateDNA->setEnabled(false);
	emit NewAvatarLogin(m_AvatarIndex);
}

/*
void AuIntroCreateAvatar4::Load()
{
	QString lastFile = g_Au.Settings().readEntry("Settings/LastDNAFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();

	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QString loadFilter;
	loadFilter += QString("Hazeron DNA (*.dna)");
	loadFilter += QString(";;All Files (*.*)");

	QString loadFile = QFileDialog::getOpenFileName(this, tr("Hazeron DNA File"), lastFile, loadFilter, 0,
			QFileDialog::DontUseNativeDialog);
	if(loadFile.isEmpty())
		return;

	g_Au.Settings().writeEntry("Settings/LastDNAFile", loadFile);

	AuDNA dna;
	int result = dna.LoadFile(loadFile);
	switch(result)
	{
	case 0:	// ok
		SetAvatar(dna, m_Hand, m_Gender);
		break;
	case 1:	// cannot open file
		{
			QString smsg = QObject::tr("Unable to open file for reading.\n\n%1").arg(loadFile);
			QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok);
			break;
		}
	case 2:	// bad file version
		{
			QString smsg = QObject::tr("File does not contain Hazeron DNA.\n\n%1").arg(loadFile);
			QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok);
			break;
		}
	}
}
*/

void AuIntroCreateAvatar4::SetAvatar(const AuDNA& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand)
{
	m_Chromatid = chroma;
	m_Hand = hand;
	SetDNA(dna);
	m_AvatarSample->ResetView();
	m_Individual->SetChromatid(chroma);
	m_Individual->SetHandedness(hand);
}

/*
void AuIntroCreateAvatar4::Save()
{
	if(m_AvatarSample->DNAVersion() != 4)
		return;

	QString lastFile = g_Au.Settings().readEntry("Settings/LastDNAFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();

	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QString saveFilter;
	saveFilter += QString("Hazeron DNA (*.dna)");
	saveFilter += QString(";;All Files (*.*)");

	QString saveFile = QFileDialog::getSaveFileName(this, tr("Hazeron DNA File"), lastFile, saveFilter, 0,
			QFileDialog::DontUseNativeDialog);
	if(saveFile.isEmpty())
		return;

	// Insure the file has an appropriate suffix.
	QString suffix(".dna");
	if(saveFile.right(suffix.length()).compare(suffix, Qt::CaseInsensitive) != 0)
		saveFile += suffix;

	g_Au.Settings().writeEntry("Settings/LastDNAFile", saveFile);

	int result = m_AvatarSample->DNA4().SaveFile(saveFile);
	if(result == 1)
	{
		QString smsg = QObject::tr("Unable to open file for writing.\n\n%1").arg(saveFile);
		QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok);
	}
}
*/

void AuIntroCreateAvatar4::EmpireDNA()
{
	/*DHH animalia
	if(!m_EmpireDNADialog)
		m_EmpireDNADialog = new AuIntroEmpireDNA(this);
	if(m_EmpireDNADialog->exec() != QDialog::Accepted)
		return;
	SetDNA(m_EmpireDNADialog->DNA());
	*/
}

void AuIntroCreateAvatar4::CreateDNA()
{
	AuDNASettingsDlg dnaDlg(this);
	if(m_AvatarSample->DNAVersion() == 4)
		dnaDlg.SetDNA(m_AvatarSample->DNA4());
	if(dnaDlg.exec() != QDialog::Accepted)
		return;
	//DHH animalia verify DNA is ok for a human.
	SetDNA(dnaDlg.DNA());
}

void AuIntroCreateAvatar4::EnableControls()
{
	m_EmpireDNA->setVisible(!m_Solo);

	if(m_AvatarSample->DNAVersion() != 4)
		return;

	QString name = m_Individual->Name();
	int nameOk;
	if(name.isEmpty())
		nameOk = 1;	// blank
	else if(!IsNameValid(name))
		nameOk = 2;	// invalid
	else
		nameOk = 0;	// good
	int dnaCheck = m_AvatarSample->AvatarCheck();
	bool scenarioOk = m_Scenarios->OkToGo();
	m_Enter->setEnabled(nameOk == 0 && dnaCheck == 0 && scenarioOk);
	if(m_AvatarSampleRendered < 1)
		return;

	AuItemDNA mySkin(&m_AvatarSample->DNA4(), m_Chromatid, true, false, m_Hand, Mature, AuHairStyle::Unkempt, AuHairStyle::Unshaven,
			CID_LeatherClothes, 128, 0, 1);
	float baseAbsorb, baseBlock;
	mySkin.CombatArmorEffectiveness(WM_Contact, WE_Generic, baseAbsorb, baseBlock);

	int hitPoints = m_AvatarSample->MaxHitPoints();
	int pointsUsed = m_AvatarSample->AbilityPointsUsed();
	int pointsLeft = 60 - pointsUsed;
	double cgHeight = m_AvatarSample->CGHeight();
	double headHeight = m_AvatarSample->HeadHeight();
	double eyeHeight = m_AvatarSample->EyeHeight();

	/*DHH animalia
	// Weapon 1
	AuCombatWeapon weapon(WM_Not);
	AuDNABase::WEAPONLOC wLoc;
	m_AvatarSample->Body()->UseSpecialWeapon(AuTime::Now());	// Set use time now so BestWeapon won't pick spec wpn.
	m_AvatarSample->Body()->BestWeaponAtRange(0, wLoc, weapon);
	QString physDmg;
	if(weapon.m_Range > 0)
	{
		if(weapon.m_w1.m_Radius > 0)
			physDmg = tr("%1 to %2 hp, range %3m, %4m blast radius")
				.arg(weapon.m_w1.m_DamageMod)
				.arg(weapon.m_w1.m_Damage + weapon.m_w1.m_DamageMod)
				.arg(weapon.m_Range / FeetPerMeter(), 0,'f',1)
				.arg(weapon.m_w1.m_Radius / FeetPerMeter(), 0,'f',1);
		else
			physDmg = tr("%1 to %2 hp, range %3m")
				.arg(weapon.m_w1.m_DamageMod)
				.arg(weapon.m_w1.m_Damage + weapon.m_w1.m_DamageMod)
				.arg(weapon.m_Range / FeetPerMeter(), 0,'f',1);
	}
	else
	{
		if(weapon.m_w1.m_Radius > 0)
			physDmg = tr("%1 to %2 hp, %4m blast radius")
				.arg(weapon.m_w1.m_DamageMod)
				.arg(weapon.m_w1.m_Damage + weapon.m_w1.m_DamageMod)
				.arg(weapon.m_w1.m_Radius / FeetPerMeter(), 0,'f',1);
		else
			physDmg = tr("%1 to %2 hp")
				.arg(weapon.m_w1.m_DamageMod)
				.arg(weapon.m_w1.m_Damage + weapon.m_w1.m_DamageMod);
	}

	// Weapon 2
	AuTime bday = AuTime::Now().AddHours(-1);					// Past time to force special weapon to appear not recently used.
	m_AvatarSample->Body()->UseSpecialWeapon(bday);			// Set use time an hour ago so BestWeapon can pick spec wpn.
	m_AvatarSample->Body()->BestWeaponAtRange(0, wLoc, weapon);
	QString specDmg;
	if(m_AvatarSample->DNA3().WeaponType != DhDNA::WT_None)
	{
		if(weapon.m_Range > 0)
		{
			if(weapon.m_w1.m_Radius > 0)
				specDmg = tr("%1 to %2 hp, range %3m, %4m blast radius")
					.arg(weapon.m_w1.m_DamageMod)
					.arg(weapon.m_w1.m_Damage + weapon.m_w1.m_DamageMod)
					.arg(weapon.m_Range / FeetPerMeter(), 0,'f',1)
					.arg(weapon.m_w1.m_Radius / FeetPerMeter(), 0,'f',1);
			else
				specDmg = tr("%1 to %2 hp, range %3m")
					.arg(weapon.m_w1.m_DamageMod)
					.arg(weapon.m_w1.m_Damage + weapon.m_w1.m_DamageMod)
					.arg(weapon.m_Range / FeetPerMeter(), 0,'f',1);
		}
		else
		{
			if(weapon.m_w1.m_Radius > 0)
				specDmg = tr("%1 to %2 hp, %4m blast radius")
					.arg(weapon.m_w1.m_DamageMod)
					.arg(weapon.m_w1.m_Damage + weapon.m_w1.m_DamageMod)
					.arg(weapon.m_w1.m_Radius / FeetPerMeter(), 0,'f',1);
			else
				specDmg = tr("%1 to %2 hp")
					.arg(weapon.m_w1.m_DamageMod)
					.arg(weapon.m_w1.m_Damage + weapon.m_w1.m_DamageMod);
		}
	}
	*/

	// Build the summary text.
	QString sum = tr("<html>");
	QString note("%1");
	QString error("<span style=\"color: rgb(255, 32, 32);\">%1</span>");
	QString warning("<span style=\"color: rgb(255, 130, 32);\">%1</span>");

	sum += "<p>";
	if(dnaCheck & 0x0001)
	{
		sum += error.arg(tr("Ability Points Available: <b>%1</b>").arg(pointsLeft));
		sum += error.arg(tr("<br>Ability Points Used: <b>%1</b>").arg(pointsUsed));
	}
	else if(dnaCheck & 0x00010000)
	{
		sum += error.arg(tr("Ability Points Available: <b>%1</b> Bad Recessive Chromatid").arg(pointsLeft));
		sum += error.arg(tr("<br>Ability Points Used: <b>%1</b> Bad Recessive Chromatid").arg(pointsUsed));
	}
	else
	{
		sum += note.arg(tr("Ability Points Available: <b>%1</b>").arg(pointsLeft));
		sum += note.arg(tr("<br>Ability Points Used: <b>%1</b>").arg(pointsUsed));
	}
	sum += "</p>";

	sum += "<p>";
	if(dnaCheck & 0x0002)
		sum += error.arg(tr("Hit Points: <b>%1</b>").arg(hitPoints));
	else if(dnaCheck & 0x00020000)
		sum += error.arg(tr("Hit Points: <b>%1</b> Bad Recessive Chromatid").arg(hitPoints));
	else
		sum += note.arg(tr("Hit Points: <b>%1</b>").arg(hitPoints));

	sum += note.arg(tr("<br>Armor: <b>%1</b>").arg(g_Au.FormatArmorFactors(baseAbsorb, baseBlock)));

	/*DHH animalia
	sum += note.arg(tr("Physical Damage: <b>%1</b>").arg(physDmg));

	if(!specDmg.isEmpty())
		sum += note.arg(tr("Special Damage: <b>%1</b>").arg(specDmg));
	*/
	sum += "</p>";

	sum += "<p>";
	if(dnaCheck & 0x0004)
		sum += error.arg(tr("Eye Height: <b>%1m</b>").arg(eyeHeight/FeetPerMeter(), 0,'f',2));
	else if(dnaCheck & 0x00040000)
		sum += error.arg(tr("Eye Height: <b>%1m</b> Bad Recessive Chromatid").arg(eyeHeight/FeetPerMeter(), 0,'f',2));
	else
		sum += note.arg(tr("Eye Height: <b>%1m</b>").arg(eyeHeight/FeetPerMeter(), 0,'f',2));

	if(dnaCheck & 0x0010)
		sum += error.arg(tr("<br>Head Height: <b>%1m</b>").arg(headHeight/FeetPerMeter(), 0,'f',2));
	else if(dnaCheck & 0x00100000)
		sum += error.arg(tr("<br>Head Height: <b>%1m</b> Bad Recessive Chromatid").arg(headHeight/FeetPerMeter(), 0,'f',2));
	else
		sum += note.arg(tr("<br>Head Height: <b>%1m</b>").arg(headHeight/FeetPerMeter(), 0,'f',2));

	if(dnaCheck & 0x0008)
		sum += error.arg(tr("<br>CG Height: <b>%1m</b>").arg(cgHeight/FeetPerMeter(), 0,'f',2));
	else if(dnaCheck & 0x00080000)
		sum += error.arg(tr("<br>CG Height: <b>%1m</b> Bad Recessive Chromatid").arg(cgHeight/FeetPerMeter(), 0,'f',2));
	else
		sum += note.arg(tr("<br>CG Height: <b>%1m</b>").arg(cgHeight/FeetPerMeter(), 0,'f',2));

	if(dnaCheck & 0x0080)
		sum += error.arg(tr("<br>Lungs: <b>Cannot Breathe Standard or Tainted Air</b>"));
	else if(dnaCheck & 0x00800000)
		sum += error.arg(tr("<br>Lungs: <b>Cannot Breathe Standard or Tainted Air</b> Bad Recessive Chromatid"));
	else
		sum += note.arg(tr("<br>Lungs: <b>Ok</b>"));
	sum += "</p>";

	if(nameOk)	// non-zero is an error
	{
		sum += "<p>";
		if(nameOk == 1)
			sum += error.arg(tr("Avatar name needed."));
		else if(nameOk == 2)
			sum += error.arg(tr("Avatar name is not valid."));
		sum += "</p>";
	}
	sum += warning.arg(tr("<p>Generation IV DNA is not complete enough for use by all creatures in the game. "
				"To the extent that it is complete, generation IV DNA can be used for avatars. "
				"</p>"
				"<p>This is a work in progress. "
				"Some features are disabled or they offer limited choices. "
				"Features will become usable as they are finished."
				"</p>"));	//DHH animalia
	sum += QString("</html>");
	m_Summary->setText(sum);
}

void AuIntroCreateAvatar4::setVisible(bool visible)
{
	QWidget::setVisible(visible);
	if(visible)
		DNAChanged();
}

void AuIntroCreateAvatar4::HandednessChanged()
{
	m_Hand = m_Individual->Handedness();
	m_AvatarSample->SetHandedness(m_Hand);
	EnableControls();
}

void AuIntroCreateAvatar4::DNAChanged()
{
	m_Chromatid = m_Individual->Chromatid();
	m_Hand = m_Individual->Handedness();
	AuDNA dna;
	if(m_AvatarSample->DNAVersion() == 4)
		dna = m_AvatarSample->DNA4();
	else
		dna.InitDefaultHuman();
	COMMODITYID clothes = g_Au.IsNudityOk() ? CID_Invalid : CID_Underwear;
	m_AvatarSample->SetAnimalia(dna, m_Chromatid, m_Hand, clothes, CID_Invalid, CID_Invalid, 0, AuHairStyle::Unkempt,
			AuHairStyle::Unshaven);
	AuPose pose((AuPose::POSE)m_Pose->currentData().toInt());
	AuPose mouth((AuPose::POSE)m_Mouth->currentData().toInt());
	m_AvatarSample->SetPose(pose, true, mouth);
	EnableControls();
}

void AuIntroCreateAvatar4::SetDNA(const AuDNA& dna)
{
	bool anyChange = m_AvatarSample->IsChange(dna, m_Chromatid, CID_Invalid, CID_Invalid, CID_Invalid, AuHairStyle::Unkempt,
                AuHairStyle::Unshaven);
	if(anyChange)
	{
		COMMODITYID clothes = g_Au.IsNudityOk() ? CID_Invalid : CID_Underwear;
		m_AvatarSample->SetAnimalia(dna, m_Chromatid, m_Hand, clothes, CID_Invalid, CID_Invalid, 0, AuHairStyle::Unkempt,
				AuHairStyle::Unshaven);
	}
	AuPose pose((AuPose::POSE)m_Pose->currentData().toInt());
	AuPose mouth((AuPose::POSE)m_Mouth->currentData().toInt());
	m_AvatarSample->SetPose(pose, true, mouth);
	m_Individual->SetDNA(dna);
	EnableControls();
}

void AuIntroCreateAvatar4::EnterNewAvatar()
{
	if(m_AvatarSample->DNAVersion() != 4)
		return;
	const DgData& gd = g_Au.LockDataForRead();
	int avatarIndex = gd.ClientUnitAvailableIndex();
	g_Au.UnlockData();
	if(avatarIndex < 0)
		return;

	QString name = m_Individual->Name();
	AuDNABase::CHROMATID chroma = m_Individual->Chromatid();
	HANDEDNESS hand = m_Individual->Handedness();
	GALAXY galaxy = m_Scenarios->Galaxy();
	AuDNA dna(m_AvatarSample->DNA4());
	SCENARIO scenario = m_Scenarios->Scenario();
	quint8 proximity = m_Scenarios->Region();
	AuTime asteroidImpact;
	asteroidImpact = m_Scenarios->AsteroidImpact().toUTC();
	quint32 joinEmpire = m_Scenarios->EmpireToJoin();
	QByteArray joinPass = m_Scenarios->EmpireJoinPassCrypt();
	AuId joinSector = m_Scenarios->EmpireJoinSector();
	QString newEmpireName = m_Scenarios->NewEmpireName();
	QImage newEmpireFlag = m_Scenarios->NewEmpireFlag();
	if(m_Solo)
		auApp->StartNewAvatarSolo(avatarIndex, name, dna, chroma, hand,
				proximity, galaxy, scenario, asteroidImpact, joinEmpire, joinSector, newEmpireName, newEmpireFlag);
	else
		auApp->StartNewAvatar(name, dna, chroma, hand,
				proximity, galaxy, scenario, asteroidImpact, joinEmpire, joinPass, joinSector);
}

void AuIntroCreateAvatar4::PoseChanged(int index)
{
	AuPose pose((AuPose::POSE)m_Pose->itemData(index).toInt());
	AuPose mouth((AuPose::POSE)m_Mouth->currentData().toInt());
	m_AvatarSample->SetPose(pose, true, mouth);
}

void AuIntroCreateAvatar4::MouthChanged(int index)
{
	AuPose pose((AuPose::POSE)m_Pose->currentData().toInt());
	AuPose mouth((AuPose::POSE)m_Mouth->itemData(index).toInt());
	m_AvatarSample->SetPose(pose, true, mouth);
}

void AuIntroCreateAvatar4::SetSolo(bool solo)
{
	m_Solo = solo;
	m_Individual->SetSolo(solo);
	m_Scenarios->SetSolo(solo);
	EnableControls();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroCreateIndividual::AuIntroCreateIndividual(QWidget* parent) : QWidget(parent)
{
	m_Solo = false;

	m_Name = new QLineEdit(this);
	m_Name->setToolTip(QObject::tr("<html>"
				"Enter the new avatar's name here. " 
				"The name must be less than %1 characters and "
				"it must not contain certain reserved characters such as | and newline. "
				"<br>"
				"Avatar names do not have to be unique in the universe. "
				"</html>").arg(MaxNameLength()+1));
	connect(m_Name, SIGNAL(textChanged(const QString&)), SIGNAL(NameChanged()));

	m_Hand = new QComboBox(this);
	m_Hand->setToolTip(tr("Select handedness."));
	m_Hand->addItem(QIcon(":data/Hand.png"), tr("Right Handed"), RightHanded);
	m_Hand->addItem(QIcon(":data/HandLeft.png"), tr("Left Handed"), LeftHanded);
	connect(m_Hand, SIGNAL(currentIndexChanged(int)), SIGNAL(HandChanged()));

	m_Chromatid = new QComboBox(this);
	m_Chromatid->setToolTip(tr("Select gender."));
	m_Chromatid->addItem(QIcon(":data/Male.png"), tr("Male"), AuDNABase::Alpha);
	m_Chromatid->addItem(QIcon(":data/Female.png"), tr("Female"), AuDNABase::Beta);
	connect(m_Chromatid, SIGNAL(currentIndexChanged(int)), SIGNAL(ChromatidChanged()));

	QLabel* name = new QLabel(tr("Avatar Name"), this);

	QLabel* hand = new QLabel(tr("Dominant Hand"), this);

	QLabel* chromatid = new QLabel(tr("Gender"), this);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(name, row,0);
	lay->addWidget(m_Name, row,1);
	row++;
	lay->addWidget(hand, row,0);
	lay->addWidget(m_Hand, row,1);
	row++;
	lay->addWidget(chromatid, row,0);
	lay->addWidget(m_Chromatid, row,1);
	row++;
	lay->setRowStretch(row, 1);
	lay->setColumnStretch(1, 1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuIntroCreateIndividual::~AuIntroCreateIndividual()
{
}

QString AuIntroCreateIndividual::Name() const
{
	return m_Name->text().trimmed();
}

void AuIntroCreateIndividual::SetChromatid(AuDNABase::CHROMATID chroma)
{
	m_Chromatid->setCurrentIndex(m_Chromatid->findData(chroma));
}

AuDNABase::CHROMATID AuIntroCreateIndividual::Chromatid() const
{
	return (AuDNABase::CHROMATID)m_Chromatid->itemData(m_Chromatid->currentIndex()).toUInt();
}

void AuIntroCreateIndividual::SetHandedness(HANDEDNESS hand)
{
	m_Hand->setCurrentIndex(m_Hand->findData(hand));
}

HANDEDNESS AuIntroCreateIndividual::Handedness() const
{
	return (HANDEDNESS)m_Hand->itemData(m_Hand->currentIndex()).toUInt();
}

void AuIntroCreateIndividual::SetSolo(bool solo)
{
	m_Solo = solo;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroCreateIndividual4::AuIntroCreateIndividual4(QWidget* parent) : QWidget(parent)
{
	m_Solo = false;

	m_Name = new QLineEdit(this);
	m_Name->setToolTip(QObject::tr("<html>"
				"Enter the new avatar's name here. " 
				"The name must be less than %1 characters and "
				"it must not contain certain reserved characters such as | and newline. "
				"<br>"
				"Avatar names do not have to be unique in the universe. "
				"</html>").arg(MaxNameLength()+1));
	connect(m_Name, SIGNAL(textChanged(const QString&)), SIGNAL(NameChanged()));

	m_Hand = new QComboBox(this);
	m_Hand->setToolTip(tr("Select handedness."));
	m_Hand->addItem(QIcon(":data/Hand.png"), tr("Right Handed"), RightHanded);
	m_Hand->addItem(QIcon(":data/HandLeft.png"), tr("Left Handed"), LeftHanded);
	connect(m_Hand, SIGNAL(currentIndexChanged(int)), SIGNAL(HandChanged()));

	m_Chromatid = new QComboBox(this);
	m_Chromatid->setToolTip(tr("<html>"
				"Diploid DNA contains two copies of all genetic information. "
				"Each half is a <i>chromatid</i>. "
				"Gender is determined by which chromatid the avatar's body uses."
				"</html>"));
	m_Chromatid->addItem(QIcon(":data/ChromatidAlpha.png"), tr("α Chromatid"), AuDNABase::Alpha);
	m_Chromatid->addItem(QIcon(":data/ChromatidBeta.png"), tr("β Chromatid"), AuDNABase::Beta);
	connect(m_Chromatid, SIGNAL(currentIndexChanged(int)), SIGNAL(ChromatidChanged()));

	QLabel* name = new QLabel(tr("Avatar Name"), this);

	QLabel* hand = new QLabel(tr("Dominant Hand"), this);

	QLabel* chromatid = new QLabel(tr("Chromatid"), this);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(name, row,0);
	lay->addWidget(m_Name, row,1);
	row++;
	lay->addWidget(hand, row,0);
	lay->addWidget(m_Hand, row,1);
	row++;
	lay->addWidget(chromatid, row,0);
	lay->addWidget(m_Chromatid, row,1);
	row++;
	lay->setRowStretch(row, 1);
	lay->setColumnStretch(1, 1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuIntroCreateIndividual4::~AuIntroCreateIndividual4()
{
}

QString AuIntroCreateIndividual4::Name() const
{
	return m_Name->text().trimmed();
}

void AuIntroCreateIndividual4::SetChromatid(AuDNABase::CHROMATID chroma)
{
	m_Chromatid->setCurrentIndex(m_Chromatid->findData(chroma));
}

AuDNABase::CHROMATID AuIntroCreateIndividual4::Chromatid() const
{
	return (AuDNABase::CHROMATID)m_Chromatid->itemData(m_Chromatid->currentIndex()).toUInt();
}

void AuIntroCreateIndividual4::SetHandedness(HANDEDNESS hand)
{
	m_Hand->setCurrentIndex(m_Hand->findData(hand));
}

HANDEDNESS AuIntroCreateIndividual4::Handedness() const
{
	return (HANDEDNESS)m_Hand->itemData(m_Hand->currentIndex()).toUInt();
}

void AuIntroCreateIndividual4::SetDNA(const AuDNA& dna)
{
	AuDNABase::CHROMATID chroma = Chromatid();
	m_Chromatid->blockSignals(true);
	m_Chromatid->clear();
	m_Chromatid->addItem(QIcon(":data/ChromatidAlpha.png"), tr("α Chromatid %1").arg(GenderName(dna.Gender(AuDNABase::Alpha))),
			AuDNABase::Alpha);
	m_Chromatid->addItem(QIcon(":data/ChromatidBeta.png"), tr("β Chromatid %1").arg(GenderName(dna.Gender(AuDNABase::Beta))),
			AuDNABase::Beta);
	m_Chromatid->setCurrentIndex(m_Chromatid->findData(chroma));
	m_Chromatid->blockSignals(false);
}

void AuIntroCreateIndividual4::SetSolo(bool solo)
{
	m_Solo = solo;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroCreateGeneral::AuIntroCreateGeneral(AuIntroCreateAvatar* parent) : QWidget(parent)
{
	m_EcoRole = new QComboBox(this);
	m_EcoRole->addItem(tr("Carnivore"), ECO_Carnivore);
	m_EcoRole->addItem(tr("Herbivore"), ECO_Herbivore);
	m_EcoRole->addItem(tr("Omnivore"), ECO_Omnivore);
	m_EcoRole->addItem(tr("Scavenger"), ECO_Scavenger);
	m_EcoRole->setToolTip("<html>"
			"Ecological role determines the kinds of foods that can be eaten. "
			"</html>");
	connect(m_EcoRole, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_Phylum = new QComboBox(this);
	m_Phylum->addItem(DhDNA::PhylumName(DhDNA::SKIN_DryExoskeleton), DhDNA::SKIN_DryExoskeleton);
	m_Phylum->addItem(DhDNA::PhylumName(DhDNA::SKIN_DryScales), DhDNA::SKIN_DryScales);
	m_Phylum->addItem(DhDNA::PhylumName(DhDNA::SKIN_DrySkin), DhDNA::SKIN_DrySkin);
	m_Phylum->addItem(DhDNA::PhylumName(DhDNA::SKIN_Feathers), DhDNA::SKIN_Feathers);
	m_Phylum->addItem(DhDNA::PhylumName(DhDNA::SKIN_Fur), DhDNA::SKIN_Fur);
	m_Phylum->addItem(DhDNA::PhylumName(DhDNA::SKIN_WetExoskeleton), DhDNA::SKIN_WetExoskeleton);
	m_Phylum->addItem(DhDNA::PhylumName(DhDNA::SKIN_WetScales), DhDNA::SKIN_WetScales);
	m_Phylum->addItem(DhDNA::PhylumName(DhDNA::SKIN_WetSkin), DhDNA::SKIN_WetSkin);
	m_Phylum->setToolTip("<html>"
			"Phylum affects the appearance of the skin and body. "
			"</html>");
	connect(m_Phylum, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_Brain = new QComboBox(this);
	m_Brain->addItem(tr("Head"), DhDNA::BRAIN_Head);
	m_Brain->addItem(tr("Torso"), DhDNA::BRAIN_Torso);
	m_Brain->setToolTip("<html>"
			"Brain location determines where the brain exists in the body. "
			"</html>");
	connect(m_Brain, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_BottomSkinThickness = new QSlider(this);
	m_BottomSkinThickness->setOrientation(Qt::Horizontal);
	m_BottomSkinThickness->setRange(0,3);
	m_BottomSkinThickness->setSingleStep(1);
	m_BottomSkinThickness->setPageStep(1);
	m_BottomSkinThickness->setToolTip("<html>"
			"Skin thickness affects the skin's ability to protect the body against damage. "
			"</html>");
	connect(m_BottomSkinThickness, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_BottomSkinType = new QComboBox(this);
	m_BottomSkinType->addItem("1", 0);
	m_BottomSkinType->addItem("2", 1);
	m_BottomSkinType->addItem("3", 2);
	m_BottomSkinType->addItem("4", 3);
	m_BottomSkinType->addItem("5", 4);
	m_BottomSkinType->addItem("6", 5);
	m_BottomSkinType->addItem("7", 6);
	m_BottomSkinType->addItem("8", 7);
	m_BottomSkinType->setToolTip("<html>"
			"Skin type affects the skin's appearance. "
			"</html>");
	connect(m_BottomSkinType, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_TopSkinThickness = new QSlider(this);
	m_TopSkinThickness->setOrientation(Qt::Horizontal);
	m_TopSkinThickness->setRange(0,3);
	m_TopSkinThickness->setSingleStep(1);
	m_TopSkinThickness->setPageStep(1);
	m_TopSkinThickness->setToolTip("<html>"
			"Skin thickness affects the skin's ability to protect the body against damage. "
			"</html>");
	connect(m_TopSkinThickness, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_TopSkinType = new QComboBox(this);
	m_TopSkinType->addItem("1", 0);
	m_TopSkinType->addItem("2", 1);
	m_TopSkinType->addItem("3", 2);
	m_TopSkinType->addItem("4", 3);
	m_TopSkinType->addItem("5", 4);
	m_TopSkinType->addItem("6", 5);
	m_TopSkinType->addItem("7", 6);
	m_TopSkinType->addItem("8", 7);
	m_TopSkinType->setToolTip("<html>"
			"Skin type affects the skin's appearance. "
			"</html>");
	connect(m_TopSkinType, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_OverallSize = new QSlider(this);
	m_OverallSize->setOrientation(Qt::Horizontal);
	m_OverallSize->setRange(0,31);
	m_OverallSize->setSingleStep(1);
	m_OverallSize->setPageStep(1);
	m_OverallSize->setToolTip("<html>"
			"Overall size uniformly changes the creature's size. "
			"</html>");
	connect(m_OverallSize, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_CrestType = new QComboBox(this);
	m_CrestType->setToolTip("<html>"
			"Crest type affects the appearance of crests on the head and torso. "
			"Head and body traits determine if those parts have crests. "
			"</html>");
	m_CrestType->addItem("1", 0);
	m_CrestType->addItem("2", 1);
	m_CrestType->addItem("3", 2);
	m_CrestType->addItem("4", 3);
	connect(m_CrestType, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_Strength = new QSlider(this);
	m_Strength->setOrientation(Qt::Horizontal);
	m_Strength->setRange(0,15);
	m_Strength->setSingleStep(1);
	m_Strength->setPageStep(1);
	m_Strength->setToolTip("<html>"
			"Strength affects how much weight you can carry. "
			"</html>");
	connect(m_Strength, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_Speed = new QSlider(this);
	m_Speed->setOrientation(Qt::Horizontal);
	m_Speed->setRange(0,15);
	m_Speed->setSingleStep(1);
	m_Speed->setPageStep(1);
	m_Speed->setToolTip("<html>"
			"Speed affects how quickly you move. "
			"</html>");
	connect(m_Speed, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_Agility = new QSlider(this);
	m_Agility->setOrientation(Qt::Horizontal);
	m_Agility->setRange(0,15);
	m_Agility->setSingleStep(1);
	m_Agility->setPageStep(1);
	m_Agility->setToolTip("<html>"
			"</html>");
	connect(m_Agility, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_Endurance = new QSlider(this);
	m_Endurance->setOrientation(Qt::Horizontal);
	m_Endurance->setRange(0,15);
	m_Endurance->setSingleStep(1);
	m_Endurance->setPageStep(1);
	m_Endurance->setToolTip("<html>"
			"Endurance affects how quickly you grow tired when running. "
			"</html>");
	connect(m_Endurance, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	QLabel* strength = new QLabel(tr("Strength"), this);

	QLabel* speed = new QLabel(tr("Speed"), this);

	QLabel* agility = new QLabel(tr("Agility"), this);

	QLabel* endurance = new QLabel(tr("Endurance"), this);

	QLabel* phylum = new QLabel(tr("Phylum"), this);

	QLabel* top = new QLabel(tr("Top Skin"), this);

	QLabel* bottom = new QLabel(tr("Bottom Skin"), this);

	QLabel* subtype = new QLabel(tr("Skin Sub Type"), this);

	QLabel* thickness = new QLabel(tr("Skin Thickness"), this);

	QLabel* brain = new QLabel(tr("Brain Location"), this);

	QLabel* osize = new QLabel(tr("Overall Size"), this);

	QLabel* ecorole = new QLabel(tr("Ecological Role"), this);

	QLabel* crest = new QLabel(tr("Crest Type"), this);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(osize, row,0);
	lay->addWidget(m_OverallSize, row,1, 1,2);
	row++;
	lay->addWidget(ecorole, row,0);
	lay->addWidget(m_EcoRole, row,1);
	row++;
	lay->addWidget(phylum, row,0);
	lay->addWidget(m_Phylum, row,1);
	row++;
	lay->addWidget(strength, row,0);
	lay->addWidget(m_Strength, row,1);
	row++;
	lay->addWidget(speed, row,0);
	lay->addWidget(m_Speed, row,1);
	row++;
	lay->addWidget(agility, row,0);
	lay->addWidget(m_Agility, row,1);
	row++;
	lay->addWidget(endurance, row,0);
	lay->addWidget(m_Endurance, row,1);
	row++;
	lay->addWidget(brain, row,0);
	lay->addWidget(m_Brain, row,1);
	row++;
	lay->addWidget(crest, row,0);
	lay->addWidget(m_CrestType, row,1);
	row++;
	lay->addWidget(subtype, row,1);
	lay->addWidget(thickness, row,2);
	row++;
	lay->addWidget(top, row,0);
	lay->addWidget(m_TopSkinType, row,1);
	lay->addWidget(m_TopSkinThickness, row,2);
	row++;
	lay->addWidget(bottom, row,0);
	lay->addWidget(m_BottomSkinType, row,1);
	lay->addWidget(m_BottomSkinThickness, row,2);
	row++;
	lay->setRowStretch(row, 1);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuIntroCreateGeneral::~AuIntroCreateGeneral()
{
}

void AuIntroCreateGeneral::DNATraits(DhDNA& dna) const
{
	if(m_EcoRole->currentIndex() < 0 ||
			m_Phylum->currentIndex() < 0 ||
			m_Brain->currentIndex() < 0 ||
			m_CrestType->currentIndex() < 0 ||
			m_TopSkinType->currentIndex() < 0 ||
			m_BottomSkinType->currentIndex() < 0)
		return;
	dna.OverallSize = m_OverallSize->value();
	dna.EcologicalRole = m_EcoRole->itemData(m_EcoRole->currentIndex()).toUInt();
	dna.SkinType = m_Phylum->itemData(m_Phylum->currentIndex()).toUInt();
	dna.Strength = m_Strength->value();
	dna.Speed = m_Speed->value();
	dna.Agility = m_Agility->value();
	dna.Endurance = m_Endurance->value();
	dna.BrainLocation = m_Brain->itemData(m_Brain->currentIndex()).toUInt();
	dna.CrestSubtype = m_CrestType->itemData(m_CrestType->currentIndex()).toUInt();
	dna.TopSkinSubtype = m_TopSkinType->itemData(m_TopSkinType->currentIndex()).toUInt();
	dna.TopSkinThickness = m_TopSkinThickness->value();
	dna.BottomSkinSubtype = m_BottomSkinType->itemData(m_BottomSkinType->currentIndex()).toUInt();
	dna.BottomSkinThickness = m_BottomSkinThickness->value();
	dna.EcologicalStrategy = AuDNABase::ECO_Hunter;
	dna.FightReflex = 0;
	dna.FlightReflex = 0;
	dna.TimeActive = TOD_Diurnal;
}

void AuIntroCreateGeneral::SetDNA(const DhDNA& dna)
{
	m_OverallSize->blockSignals(true);
	m_EcoRole->blockSignals(true);
	m_Phylum->blockSignals(true);
	m_Strength->blockSignals(true);
	m_Speed->blockSignals(true);
	m_Agility->blockSignals(true);
	m_Endurance->blockSignals(true);
	m_Brain->blockSignals(true);
	m_CrestType->blockSignals(true);
	m_TopSkinType->blockSignals(true);
	m_TopSkinThickness->blockSignals(true);
	m_BottomSkinType->blockSignals(true);
	m_BottomSkinThickness->blockSignals(true);

	m_OverallSize->setValue(dna.OverallSize);
	m_EcoRole->setCurrentIndex(m_EcoRole->findData(dna.EcologicalRole));
	m_Phylum->setCurrentIndex(m_Phylum->findData(dna.SkinType));
	m_Strength->setValue(dna.Strength);
	m_Speed->setValue(dna.Speed);
	m_Agility->setValue(dna.Agility);
	m_Endurance->setValue(dna.Endurance);
	m_Brain->setCurrentIndex(m_Brain->findData(dna.BrainLocation));
	m_CrestType->setCurrentIndex(m_CrestType->findData(dna.CrestSubtype));
	m_TopSkinType->setCurrentIndex(m_TopSkinType->findData(dna.TopSkinSubtype));
	m_TopSkinThickness->setValue(dna.TopSkinThickness);
	m_BottomSkinType->setCurrentIndex(m_BottomSkinType->findData(dna.BottomSkinSubtype));
	m_BottomSkinThickness->setValue(dna.BottomSkinThickness);

	m_OverallSize->blockSignals(false);
	m_EcoRole->blockSignals(false);
	m_Phylum->blockSignals(false);
	m_Strength->blockSignals(false);
	m_Speed->blockSignals(false);
	m_Agility->blockSignals(false);
	m_Endurance->blockSignals(false);
	m_Brain->blockSignals(false);
	m_CrestType->blockSignals(false);
	m_TopSkinType->blockSignals(false);
	m_TopSkinThickness->blockSignals(false);
	m_BottomSkinType->blockSignals(false);
	m_BottomSkinThickness->blockSignals(false);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroCreateHead::AuIntroCreateHead(AuIntroCreateAvatar* parent) : QWidget(parent)
{
	m_CrestSize = new QSlider(this);
	m_CrestSize->setOrientation(Qt::Horizontal);
	m_CrestSize->setRange(0,3);
	m_CrestSize->setSingleStep(1);
	m_CrestSize->setPageStep(1);
	connect(m_CrestSize, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_EarSize = new QSlider(this);
	m_EarSize->setOrientation(Qt::Horizontal);
	m_EarSize->setRange(0,3);
	m_EarSize->setSingleStep(1);
	m_EarSize->setPageStep(1);
	connect(m_EarSize, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_EarType = new QComboBox(this);
	m_EarType->addItem("1", 0);
	m_EarType->addItem("2", 1);
	m_EarType->addItem("3", 2);
	m_EarType->addItem("4", 3);
	m_EarType->addItem("5", 4);
	m_EarType->addItem("6", 5);
	m_EarType->addItem("7", 6);
	m_EarType->addItem("8", 7);
	connect(m_EarType, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_EyeSize = new QSlider(this);
	m_EyeSize->setOrientation(Qt::Horizontal);
	m_EyeSize->setRange(0,3);
	m_EyeSize->setSingleStep(1);
	m_EyeSize->setPageStep(1);
	connect(m_EyeSize, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_EyeType = new QComboBox(this);
	m_EyeType->addItem("1", 0);
	m_EyeType->addItem("2", 1);
	m_EyeType->addItem("3", 2);
	m_EyeType->addItem("4", 3);
	m_EyeType->addItem("5", 4);
	m_EyeType->addItem("6", 5);
	m_EyeType->addItem("7", 6);
	m_EyeType->addItem("8", 7);
	connect(m_EyeType, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_Heads = new QComboBox(this);
	m_Heads->addItem("1", 0);
	m_Heads->addItem("2", 1);
	connect(m_Heads, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_HornSize = new QSlider(this);
	m_HornSize->setOrientation(Qt::Horizontal);
	m_HornSize->setRange(0,3);
	m_HornSize->setSingleStep(1);
	m_HornSize->setPageStep(1);
	connect(m_HornSize, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_HornType = new QComboBox(this);
	m_HornType->addItem("1", 0);
	m_HornType->addItem("2", 1);
	m_HornType->addItem("3", 2);
	m_HornType->addItem("4", 3);
	connect(m_HornType, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_MouthSize = new QSlider(this);
	m_MouthSize->setOrientation(Qt::Horizontal);
	m_MouthSize->setRange(0,3);
	m_MouthSize->setSingleStep(1);
	m_MouthSize->setPageStep(1);
	connect(m_MouthSize, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_MouthType = new QComboBox(this);
	m_MouthType->addItem("1", 0);
	m_MouthType->addItem("2", 1);
	m_MouthType->addItem("3", 2);
	m_MouthType->addItem("4", 3);
	m_MouthType->addItem("5", 4);
	m_MouthType->addItem("6", 5);
	m_MouthType->addItem("7", 6);
	m_MouthType->addItem("8", 7);
	connect(m_MouthType, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_NeckVertebrae = new QSlider(this);
	m_NeckVertebrae->setOrientation(Qt::Horizontal);
	m_NeckVertebrae->setRange(0,3);
	m_NeckVertebrae->setSingleStep(1);
	m_NeckVertebrae->setPageStep(1);
	connect(m_NeckVertebrae, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_NoseSize = new QSlider(this);
	m_NoseSize->setOrientation(Qt::Horizontal);
	m_NoseSize->setRange(0,3);
	m_NoseSize->setSingleStep(1);
	m_NoseSize->setPageStep(1);
	connect(m_NoseSize, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_NoseType = new QComboBox(this);
	m_NoseType->addItem("1", 0);
	m_NoseType->addItem("2", 1);
	m_NoseType->addItem("3", 2);
	m_NoseType->addItem("4", 3);
	m_NoseType->addItem("5", 4);
	m_NoseType->addItem("6", 5);
	m_NoseType->addItem("7", 6);
	m_NoseType->addItem("8", 7);

	QLabel* heads = new QLabel(tr("Heads"), this);

	QLabel* crest = new QLabel(tr("Crest Size"), this);

	QLabel* type = new QLabel(tr("Type"), this);

	QLabel* size = new QLabel(tr("Size"), this);

	QLabel* eyes = new QLabel(tr("Eyes"), this);

	QLabel* neck = new QLabel(tr("Neck Vertebrae"), this);

	QLabel* ears = new QLabel(tr("Ears"), this);

	QLabel* nose = new QLabel(tr("Nose"), this);

	QLabel* mouth = new QLabel(tr("Mouth"), this);

	QLabel* horns = new QLabel(tr("Horns"), this);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(heads, row,0);
	lay->addWidget(m_Heads, row,1);
	row++;
	lay->addWidget(crest, row,0);
	lay->addWidget(m_CrestSize, row,1);
	row++;
	lay->addWidget(neck, row,0);
	lay->addWidget(m_NeckVertebrae, row,1);
	row++;
	lay->addWidget(type, row,1);
	lay->addWidget(size, row,2);
	row++;
	lay->addWidget(eyes, row,0);
	lay->addWidget(m_EyeType, row,1);
	lay->addWidget(m_EyeSize, row,2);
	row++;
	lay->addWidget(ears, row,0);
	lay->addWidget(m_EarType, row,1);
	lay->addWidget(m_EarSize, row,2);
	row++;
	lay->addWidget(nose, row,0);
	lay->addWidget(m_NoseType, row,1);
	lay->addWidget(m_NoseSize, row,2);
	row++;
	lay->addWidget(mouth, row,0);
	lay->addWidget(m_MouthType, row,1);
	lay->addWidget(m_MouthSize, row,2);
	row++;
	lay->addWidget(horns, row,0);
	lay->addWidget(m_HornType, row,1);
	lay->addWidget(m_HornSize, row,2);
	row++;
	lay->setRowStretch(row, 1);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuIntroCreateHead::~AuIntroCreateHead()
{
}

void AuIntroCreateHead::SetPhylum(/*DhDNA::SKIN*/int phylum)
{
	int eye = m_EyeType->currentIndex();
	m_EyeType->blockSignals(true);
	m_EyeType->clear();

	int ear = m_EarType->currentIndex();
	m_EarType->blockSignals(true);
	m_EarType->clear();

	int nose = m_NoseType->currentIndex();
	m_NoseType->blockSignals(true);
	m_NoseType->clear();

	int mouth = m_MouthType->currentIndex();
	m_MouthType->blockSignals(true);
	m_MouthType->clear();

	int horn = m_HornType->currentIndex();
	m_HornType->blockSignals(true);
	m_HornType->clear();

	switch(phylum)
	{
	case DhDNA::SKIN_DryExoskeleton:
		// Eye Type
		m_EyeType->addItem("Cluster Simple");
		m_EyeType->addItem("Cluster");
		m_EyeType->addItem("Complex");
		m_EyeType->addItem("Facet");
		m_EyeType->addItem("Not Apparent");
		m_EyeType->addItem("Simple");
		m_EyeType->addItem("Squint");
		m_EyeType->addItem("Alien");
		// Ear Type
		m_EarType->addItem("Holes");
		m_EarType->addItem("Antenna I");
		m_EarType->addItem("Antenna II");
		m_EarType->addItem("Floppy");
		m_EarType->addItem("Not Apparent");
		m_EarType->addItem("Antenna III");
		m_EarType->addItem("Spike");
		m_EarType->addItem("Moth Antenna");
		// Nose Type
		m_NoseType->addItem("Spike I");
		m_NoseType->addItem("Holes");
		m_NoseType->addItem("Nose");
		m_NoseType->addItem("Not Apparent");
		m_NoseType->addItem("Proboscis");
		m_NoseType->addItem("Spike II");
		m_NoseType->addItem("Bubbles");
		m_NoseType->addItem("Sprout");
		// Mouth Type
		m_MouthType->addItem("Beak");
		m_MouthType->addItem("Cilia");
		m_MouthType->addItem("Fangs");
		m_MouthType->addItem("Mandible");
		m_MouthType->addItem("Mouth");
		m_MouthType->addItem("Not Apparent");
		m_MouthType->addItem("Teeth");
		m_MouthType->addItem("Tusks");
		// Horn Type
		m_HornType->addItem("Antenna I");
		m_HornType->addItem("Beetle");
		m_HornType->addItem("Antenna II");
		m_HornType->addItem("Rhino");
		break;
	case DhDNA::SKIN_DryScales:
		// Eye Type
		m_EyeType->addItem("Simple");
		m_EyeType->addItem("Bird");
		m_EyeType->addItem("Complex");
		m_EyeType->addItem("Reptile");
		m_EyeType->addItem("Not Apparent");
		m_EyeType->addItem("Sockets");
		m_EyeType->addItem("Chameleon");
		m_EyeType->addItem("Facet");
		// Ear Type
		m_EarType->addItem("Antenna");
		m_EarType->addItem("Holes");
		m_EarType->addItem("Elf");
		m_EarType->addItem("Floppy");
		m_EarType->addItem("Not Apparent");
		m_EarType->addItem("Spike");
		m_EarType->addItem("Fish");
		m_EarType->addItem("Rabbit");
		// Nose Type
		m_NoseType->addItem("Spikes");
		m_NoseType->addItem("Rhino");
		m_NoseType->addItem("Holes");
		m_NoseType->addItem("Not Apparent");
		m_NoseType->addItem("Spike");
		m_NoseType->addItem("Snorkel");
		m_NoseType->addItem("Missing");
		m_NoseType->addItem("Lump");
		// Mouth Type
		m_MouthType->addItem("Beak");
		m_MouthType->addItem("Tongue");
		m_MouthType->addItem("Fangs");
		m_MouthType->addItem("Mandible");
		m_MouthType->addItem("Mouth");
		m_MouthType->addItem("Not Apparent");
		m_MouthType->addItem("Teeth");
		m_MouthType->addItem("Tusks");
		// Horn Type
		m_HornType->addItem("Ram");
		m_HornType->addItem("Beetle");
		m_HornType->addItem("Demon");
		m_HornType->addItem("Unicorn");
		break;
	case DhDNA::SKIN_DrySkin:
		// Eye Type
		m_EyeType->addItem("Cluster Simple");
		m_EyeType->addItem("Triclops");
		m_EyeType->addItem("Complex");
		m_EyeType->addItem("Cyclops");
		m_EyeType->addItem("Not Apparent");
		m_EyeType->addItem("Simple");
		m_EyeType->addItem("Sockets");
		m_EyeType->addItem("Stalk");
		// Ear Type
		m_EarType->addItem("Antenna");
		m_EarType->addItem("Ear");
		m_EarType->addItem("Elf");
		m_EarType->addItem("Floppy");
		m_EarType->addItem("Not Apparent");
		m_EarType->addItem("Spot");
		m_EarType->addItem("Fish");
		m_EarType->addItem("Cat");
		// Nose Type
		m_NoseType->addItem("Beak");
		m_NoseType->addItem("Holes");
		m_NoseType->addItem("Nose");
		m_NoseType->addItem("Not Apparent");
		m_NoseType->addItem("Dog");
		m_NoseType->addItem("Point");
		m_NoseType->addItem("Snout");
		m_NoseType->addItem("Trunk");
		// Mouth Type
		m_MouthType->addItem("Butterfly");
		m_MouthType->addItem("Cilia");
		m_MouthType->addItem("Fangs");
		m_MouthType->addItem("Sucker");
		m_MouthType->addItem("Lips");
		m_MouthType->addItem("Not Apparent");
		m_MouthType->addItem("Mouth");
		m_MouthType->addItem("Tusks");
		// Horn Type
		m_HornType->addItem("Ram");
		m_HornType->addItem("Beetle");
		m_HornType->addItem("Demon");
		m_HornType->addItem("Antlers");
		break;
	case DhDNA::SKIN_Feathers:
		// Eye Type
		m_EyeType->addItem("Cluster Simple");
		m_EyeType->addItem("Bird");
		m_EyeType->addItem("Complex");
		m_EyeType->addItem("Shadow");
		m_EyeType->addItem("Not Apparent");
		m_EyeType->addItem("Simple");
		m_EyeType->addItem("Reptile");
		m_EyeType->addItem("Blindfold");
		// Ear Type
		m_EarType->addItem("Antenna");
		m_EarType->addItem("Phoenix");
		m_EarType->addItem("Elf");
		m_EarType->addItem("Floppy");
		m_EarType->addItem("Not Apparent");
		m_EarType->addItem("Spot");
		m_EarType->addItem("Stalk");
		m_EarType->addItem("Cat");
		// Nose Type
		m_NoseType->addItem("Hook");
		m_NoseType->addItem("Holes");
		m_NoseType->addItem("Bird");
		m_NoseType->addItem("Not Apparent");
		m_NoseType->addItem("Spike");
		m_NoseType->addItem("Crazy");
		m_NoseType->addItem("Antenna");
		m_NoseType->addItem("Turkey");
		// Mouth Type
		m_MouthType->addItem("Duck");
		m_MouthType->addItem("Tongue");
		m_MouthType->addItem("Fangs");
		m_MouthType->addItem("Penguin");
		m_MouthType->addItem("Rooster");
		m_MouthType->addItem("Not Apparent");
		m_MouthType->addItem("Beak");
		m_MouthType->addItem("Open Beak");
		// Horn Type
		m_HornType->addItem("Ram");
		m_HornType->addItem("Antlers");
		m_HornType->addItem("Curved");
		m_HornType->addItem("Unicorn");
		break;
	case DhDNA::SKIN_Fur:
		// Eye Type
		m_EyeType->addItem("Alien");
		m_EyeType->addItem("Reptile");
		m_EyeType->addItem("Wolf");
		m_EyeType->addItem("Wink");
		m_EyeType->addItem("Not Apparent");
		m_EyeType->addItem("Simple");
		m_EyeType->addItem("Human");
		m_EyeType->addItem("Triclops");
		// Ear Type
		m_EarType->addItem("Primate");
		m_EarType->addItem("Wolf");
		m_EarType->addItem("Bear");
		m_EarType->addItem("Elephant");
		m_EarType->addItem("Not Apparent");
		m_EarType->addItem("Holes");
		m_EarType->addItem("Cow");
		m_EarType->addItem("Spike");
		// Nose Type
		m_NoseType->addItem("Beak");
		m_NoseType->addItem("Ape");
		m_NoseType->addItem("Canine");
		m_NoseType->addItem("Not Apparent");
		m_NoseType->addItem("Crazy");
		m_NoseType->addItem("Cow");
		m_NoseType->addItem("Feline");
		m_NoseType->addItem("Trunk");
		// Mouth Type
		m_MouthType->addItem("Tongue");
		m_MouthType->addItem("Feline");
		m_MouthType->addItem("Fangs");
		m_MouthType->addItem("Ape");
		m_MouthType->addItem("Mouth");
		m_MouthType->addItem("Not Apparent");
		m_MouthType->addItem("Teeth");
		m_MouthType->addItem("Tusks");
		// Horn Type
		m_HornType->addItem("Ram");
		m_HornType->addItem("Beetle");
		m_HornType->addItem("Demon");
		m_HornType->addItem("Rhino");
		break;
	case DhDNA::SKIN_WetExoskeleton:
		// Eye Type
		m_EyeType->addItem("Cluster Simple");
		m_EyeType->addItem("Cluster Stalk");
		m_EyeType->addItem("Stalk");
		m_EyeType->addItem("Facet");
		m_EyeType->addItem("Not Apparent");
		m_EyeType->addItem("Crazy");
		m_EyeType->addItem("Cyclops Stalk");
		m_EyeType->addItem("Cartoon");
		// Ear Type
		m_EarType->addItem("Antenna");
		m_EarType->addItem("Curly");
		m_EarType->addItem("Hairs");
		m_EarType->addItem("Stalk");
		m_EarType->addItem("Not Apparent");
		m_EarType->addItem("Spike");
		m_EarType->addItem("Spot");
		m_EarType->addItem("Sucker");
		// Nose Type
		m_NoseType->addItem("Drill");
		m_NoseType->addItem("Holes");
		m_NoseType->addItem("Catfish");
		m_NoseType->addItem("Not Apparent");
		m_NoseType->addItem("Spike");
		m_NoseType->addItem("Nose");
		m_NoseType->addItem("Snout");
		m_NoseType->addItem("Hair");
		// Mouth Type
		m_MouthType->addItem("Beak");
		m_MouthType->addItem("Cilia");
		m_MouthType->addItem("Fangs");
		m_MouthType->addItem("Mandible");
		m_MouthType->addItem("Mouth");
		m_MouthType->addItem("Not Apparent");
		m_MouthType->addItem("Teeth");
		m_MouthType->addItem("Tongue");
		// Horn Type
		m_HornType->addItem("Eyes");
		m_HornType->addItem("Lobster");
		m_HornType->addItem("Demon");
		m_HornType->addItem("Bent");
		break;
	case DhDNA::SKIN_WetScales:
		// Eye Type
		m_EyeType->addItem("Hammerhead");
		m_EyeType->addItem("White");
		m_EyeType->addItem("Complex");
		m_EyeType->addItem("Facet");
		m_EyeType->addItem("Not Apparent");
		m_EyeType->addItem("Simple");
		m_EyeType->addItem("Slit");
		m_EyeType->addItem("Stalk");
		// Ear Type
		m_EarType->addItem("Antenna");
		m_EarType->addItem("Fish");
		m_EarType->addItem("Prong");
		m_EarType->addItem("Orc");
		m_EarType->addItem("Not Apparent");
		m_EarType->addItem("Spot");
		m_EarType->addItem("Stalk");
		m_EarType->addItem("Wing");
		// Nose Type
		m_NoseType->addItem("Catfish");
		m_NoseType->addItem("Narwhal");
		m_NoseType->addItem("Shark");
		m_NoseType->addItem("Not Apparent");
		m_NoseType->addItem("Spike");
		m_NoseType->addItem("Marlin");
		m_NoseType->addItem("Snout");
		m_NoseType->addItem("Trunk");
		// Mouth Type
		m_MouthType->addItem("Dolphin");
		m_MouthType->addItem("Cilia");
		m_MouthType->addItem("Shark");
		m_MouthType->addItem("Mandible");
		m_MouthType->addItem("Fish");
		m_MouthType->addItem("Not Apparent");
		m_MouthType->addItem("Teeth");
		m_MouthType->addItem("Baleen");
		// Horn Type
		m_HornType->addItem("Trap");
		m_HornType->addItem("Snorkel");
		m_HornType->addItem("Fin");
		m_HornType->addItem("Demon");
		break;
	case DhDNA::SKIN_WetSkin:
		// Eye Type
		m_EyeType->addItem("Cluster");
		m_EyeType->addItem("Toad");
		m_EyeType->addItem("Frog");
		m_EyeType->addItem("Facet");
		m_EyeType->addItem("Not Apparent");
		m_EyeType->addItem("Simple");
		m_EyeType->addItem("Oops");
		m_EyeType->addItem("Stalk");
		// Ear Type
		m_EarType->addItem("Antenna");
		m_EarType->addItem("Hole");
		m_EarType->addItem("Hair");
		m_EarType->addItem("Floppy");
		m_EarType->addItem("Not Apparent");
		m_EarType->addItem("Warts");
		m_EarType->addItem("Fin");
		m_EarType->addItem("Spot");
		// Nose Type
		m_NoseType->addItem("Spikes");
		m_NoseType->addItem("Rhino");
		m_NoseType->addItem("Nose");
		m_NoseType->addItem("Not Apparent");
		m_NoseType->addItem("Proboscis");
		m_NoseType->addItem("Sharp");
		m_NoseType->addItem("Snout");
		m_NoseType->addItem("Trunk");
		// Mouth Type
		m_MouthType->addItem("Inflated");
		m_MouthType->addItem("Smile");
		m_MouthType->addItem("Fangs");
		m_MouthType->addItem("Bug");
		m_MouthType->addItem("Mouth");
		m_MouthType->addItem("Not Apparent");
		m_MouthType->addItem("Teeth");
		m_MouthType->addItem("Open");
		// Horn Type
		m_HornType->addItem("Unicorn");
		m_HornType->addItem("Beetle");
		m_HornType->addItem("Stack");
		m_HornType->addItem("Demon");
		break;
	}

	m_EyeType->setCurrentIndex(eye);
	m_EyeType->blockSignals(false);

	m_EarType->setCurrentIndex(ear);
	m_EarType->blockSignals(false);

	m_NoseType->setCurrentIndex(nose);
	m_NoseType->blockSignals(false);

	m_MouthType->setCurrentIndex(mouth);
	m_MouthType->blockSignals(false);

	m_HornType->setCurrentIndex(horn);
	m_HornType->blockSignals(false);
}

void AuIntroCreateHead::DNATraits(DhDNA& dna) const
{
	if(m_Heads->currentIndex() < 0 ||
			m_EyeType->currentIndex() < 0 ||
			m_EarType->currentIndex() < 0 ||
			m_NoseType->currentIndex() < 0 ||
			m_MouthType->currentIndex() < 0 ||
			m_HornType->currentIndex() < 0)
		return;
	dna.Heads = m_Heads->itemData(m_Heads->currentIndex()).toUInt();
	dna.HeadCrestSize = m_CrestSize->value();
	dna.NeckVertebrae = m_NeckVertebrae->value();
	dna.EyeType = m_EyeType->currentIndex();
	dna.EyeSize = m_EyeSize->value();
	dna.EarType = m_EarType->currentIndex();
	dna.EarSize = m_EarSize->value();
	dna.NoseType = m_NoseType->currentIndex();
	dna.NoseSize = m_NoseSize->value();
	dna.MouthType = m_MouthType->currentIndex();
	dna.MouthSize = m_MouthSize->value();
	dna.HornBranches = m_HornType->currentIndex();
	dna.HornSize = m_HornSize->value();
}

void AuIntroCreateHead::SetDNA(const DhDNA& dna)
{
	m_Heads->blockSignals(true);
	m_CrestSize->blockSignals(true);
	m_NeckVertebrae->blockSignals(true);
	m_EyeType->blockSignals(true);
	m_EyeSize->blockSignals(true);
	m_EarType->blockSignals(true);
	m_EarSize->blockSignals(true);
	m_NoseType->blockSignals(true);
	m_NoseSize->blockSignals(true);
	m_MouthType->blockSignals(true);
	m_MouthSize->blockSignals(true);
	m_HornType->blockSignals(true);
	m_HornSize->blockSignals(true);

	m_Heads->setCurrentIndex(m_Heads->findData(dna.Heads));
	m_CrestSize->setValue(dna.HeadCrestSize);
	m_NeckVertebrae->setValue(dna.NeckVertebrae);
	m_EyeType->setCurrentIndex(dna.EyeType);
	m_EyeSize->setValue(dna.EyeSize);
	m_EarType->setCurrentIndex(dna.EarType);
	m_EarSize->setValue(dna.EarSize);
	m_NoseType->setCurrentIndex(dna.NoseType);
	m_NoseSize->setValue(dna.NoseSize);
	m_MouthType->setCurrentIndex(dna.MouthType);
	m_MouthSize->setValue(dna.MouthSize);
	m_HornType->setCurrentIndex(dna.HornBranches);
	m_HornSize->setValue(dna.HornSize);

	m_Heads->blockSignals(false);
	m_CrestSize->blockSignals(false);
	m_NeckVertebrae->blockSignals(false);
	m_EyeType->blockSignals(false);
	m_EyeSize->blockSignals(false);
	m_EarType->blockSignals(false);
	m_EarSize->blockSignals(false);
	m_NoseType->blockSignals(false);
	m_NoseSize->blockSignals(false);
	m_MouthType->blockSignals(false);
	m_MouthSize->blockSignals(false);
	m_HornType->blockSignals(false);
	m_HornSize->blockSignals(false);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroCreateBody::AuIntroCreateBody(AuIntroCreateAvatar* parent) : QWidget(parent)
{
	m_BodyCarriage = new QComboBox(this);
	m_BodyCarriage->addItem("1", 0);
	m_BodyCarriage->addItem("2", 1);
	m_BodyCarriage->addItem("3", 2);
	m_BodyCarriage->addItem("4", 3);
	connect(m_BodyCarriage, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_CanJump = new QCheckBox(tr("Can Jump or Fly"), this);
	m_CanJump->setToolTip(tr("<html>"
			"When checked this will cause the avatar to become a flying creature <u>if it has wings</u>. "
			"If the avatar does not have wings, this setting improves its jumping ability."
			"</html>"));
	connect(m_CanJump, SIGNAL(stateChanged(int)), SIGNAL(DNAChanged()));

	m_CrestSize = new QSlider(this);
	m_CrestSize->setOrientation(Qt::Horizontal);
	m_CrestSize->setRange(0,3);
	m_CrestSize->setSingleStep(1);
	m_CrestSize->setPageStep(1);
	connect(m_CrestSize, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_TorsoLength = new QSlider(this);
	m_TorsoLength->setOrientation(Qt::Horizontal);
	m_TorsoLength->setRange(0,31);
	m_TorsoLength->setSingleStep(1);
	m_TorsoLength->setPageStep(1);
	connect(m_TorsoLength, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_TorsoThickness = new QSlider(this);
	m_TorsoThickness->setOrientation(Qt::Horizontal);
	m_TorsoThickness->setRange(0,31);
	m_TorsoThickness->setSingleStep(1);
	m_TorsoThickness->setPageStep(1);
	connect(m_TorsoThickness, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_TorsoVertebrae = new QSlider(this);
	m_TorsoVertebrae->setOrientation(Qt::Horizontal);
	m_TorsoVertebrae->setRange(0,7);
	m_TorsoVertebrae->setSingleStep(1);
	m_TorsoVertebrae->setPageStep(1);
	connect(m_TorsoVertebrae, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	QLabel* carriage = new QLabel(tr("Body Carriage"), this);

	QLabel* tlen = new QLabel(tr("Torso Length"), this);

	QLabel* tthick = new QLabel(tr("Torso Thickness"), this);

	QLabel* tverts = new QLabel(tr("Torso Vertebrae"), this);

	QLabel* crest = new QLabel(tr("Crest Size"), this);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(carriage, row,0);
	lay->addWidget(m_BodyCarriage, row,1);
	row++;
	lay->addWidget(crest, row,0);
	lay->addWidget(m_CrestSize, row,1);
	row++;
	lay->addWidget(tverts, row,0);
	lay->addWidget(m_TorsoVertebrae, row,1);
	row++;
	lay->addWidget(tlen, row,0);
	lay->addWidget(m_TorsoLength, row,1);
	row++;
	lay->addWidget(tthick, row,0);
	lay->addWidget(m_TorsoThickness, row,1);
	row++;
	lay->addWidget(m_CanJump, row,1);
	row++;
	lay->setRowStretch(row, 1);
	lay->setColumnStretch(1, 1);
	lay->setColumnStretch(2, 1);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuIntroCreateBody::~AuIntroCreateBody()
{
}

void AuIntroCreateBody::DNATraits(DhDNA& dna) const
{
	if(m_BodyCarriage->currentIndex() < 0)
		return;
	dna.BodyCarriage = m_BodyCarriage->itemData(m_BodyCarriage->currentIndex()).toUInt();
	dna.TorsoCrestSize = m_CrestSize->value();
	dna.TorsoVertebrae = m_TorsoVertebrae->value();
	dna.TorsoSegmentLength = m_TorsoLength->value();
	dna.TorsoThickness = m_TorsoThickness->value();
	dna.LivesInAir = m_CanJump->isChecked();
	dna.LivesOnLand = 1;
}

void AuIntroCreateBody::SetDNA(const DhDNA& dna)
{
	m_BodyCarriage->blockSignals(true);
	m_CrestSize->blockSignals(true);
	m_TorsoVertebrae->blockSignals(true);
	m_TorsoLength->blockSignals(true);
	m_TorsoThickness->blockSignals(true);
	m_CanJump->blockSignals(true);

	m_BodyCarriage->setCurrentIndex(m_BodyCarriage->findData(dna.BodyCarriage));
	m_CrestSize->setValue(dna.TorsoCrestSize);
	m_TorsoVertebrae->setValue(dna.TorsoVertebrae);
	m_TorsoLength->setValue(dna.TorsoSegmentLength);
	m_TorsoThickness->setValue(dna.TorsoThickness);
	m_CanJump->setChecked(dna.LivesInAir);

	m_BodyCarriage->blockSignals(false);
	m_CrestSize->blockSignals(false);
	m_TorsoVertebrae->blockSignals(false);
	m_TorsoLength->blockSignals(false);
	m_TorsoThickness->blockSignals(false);
	m_CanJump->blockSignals(false);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroCreateArms::AuIntroCreateArms(AuIntroCreateAvatar* parent) : QWidget(parent)
{
	m_ArmPairs = new QComboBox(this);
	m_ArmPairs->addItem("No Arms", 0);
	m_ArmPairs->addItem("Two Arms", 1);
	m_ArmPairs->addItem("Four Arms", 2);
	m_ArmPairs->addItem("Six Arms", 3);
	connect(m_ArmPairs, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_ArmType = new QComboBox(this);
	m_ArmType->addItem(tr("Limb"), DhDNA::ARM_Limb);
	m_ArmType->addItem(tr("Tentacle"), DhDNA::ARM_Tentacle);
	m_ArmType->addItem(tr("Butterfly Wings Glide"), DhDNA::ARM_WingButterfly);
	m_ArmType->addItem(tr("Dragon Fly Wings Hover"), DhDNA::ARM_WingDragonFly);
	m_ArmType->addItem(tr("Fairy Wings Hover"), DhDNA::ARM_WingFairy);
	m_ArmType->addItem(tr("Feathered Wings Glide"), DhDNA::ARM_WingFeathered);
	m_ArmType->addItem(tr("Leathery Wings Glide"), DhDNA::ARM_WingLeathery);
	m_ArmType->addItem(tr("Moth Wings Glide"), DhDNA::ARM_WingMoth);
	m_ArmType->setToolTip(QObject::tr("<html>"
				"<p>"
				"Only the first pair of arms become wings. "
				"After that, additional pairs of arms will be limbs. "
				"</p>"
				"<p>"
				"Bodies with wings that <i>glide</i> continue moving in flight, when no forward movement is applied. "
				"Bodies with wings that <i>hover</i> stop moving in flight, when no forward movement is applied. "
				"</p>"
				"</html>"));
	connect(m_ArmType, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_HandLength = new QSlider(this);
	m_HandLength->setOrientation(Qt::Horizontal);
	m_HandLength->setRange(0,31);
	m_HandLength->setSingleStep(1);
	m_HandLength->setPageStep(1);
	connect(m_HandLength, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_HandType = new QComboBox(this);
	m_HandType->addItem(tr("Claw"), DhDNA::HAND_Claw);
	m_HandType->addItem(tr("Flipper"), DhDNA::HAND_Flipper);
	m_HandType->addItem(tr("Foot"), DhDNA::HAND_Foot);
	m_HandType->addItem(tr("Hand"), DhDNA::HAND_Hand);
	m_HandType->addItem(tr("Hoof"), DhDNA::HAND_Hoof);
	m_HandType->addItem(tr("Paw"), DhDNA::HAND_Paw);
	m_HandType->addItem(tr("Pincher"), DhDNA::HAND_Pincher);
	m_HandType->addItem(tr("Sucker"), DhDNA::HAND_Sucker);
	connect(m_HandType, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_LowerLength = new QSlider(this);
	m_LowerLength->setOrientation(Qt::Horizontal);
	m_LowerLength->setRange(0,31);
	m_LowerLength->setSingleStep(1);
	m_LowerLength->setPageStep(1);
	connect(m_LowerLength, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_LowerThickness = new QSlider(this);
	m_LowerThickness->setOrientation(Qt::Horizontal);
	m_LowerThickness->setRange(0,31);
	m_LowerThickness->setSingleStep(1);
	m_LowerThickness->setPageStep(1);
	connect(m_LowerThickness, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_UpperLength = new QSlider(this);
	m_UpperLength->setOrientation(Qt::Horizontal);
	m_UpperLength->setRange(0,31);
	m_UpperLength->setSingleStep(1);
	m_UpperLength->setPageStep(1);
	connect(m_UpperLength, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_UpperThickness = new QSlider(this);
	m_UpperThickness->setOrientation(Qt::Horizontal);
	m_UpperThickness->setRange(0,31);
	m_UpperThickness->setSingleStep(1);
	m_UpperThickness->setPageStep(1);
	connect(m_UpperThickness, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_WingType = new QComboBox(this);
	m_WingType->addItem("1", 0);
	m_WingType->addItem("2", 1);
	m_WingType->addItem("3", 2);
	m_WingType->addItem("4", 3);
	m_WingType->addItem("5", 4);
	m_WingType->addItem("6", 5);
	m_WingType->addItem("7", 6);
	m_WingType->addItem("8", 7);
	connect(m_WingType, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	QLabel* type = new QLabel(tr("Arm Type"), this);

	QLabel* pairs = new QLabel(tr("Arms"), this);

	QLabel* len = new QLabel(tr("Length"), this);

	QLabel* thick = new QLabel(tr("Thickness"), this);

	QLabel* upper = new QLabel(tr("Upper"), this);

	QLabel* lower = new QLabel(tr("Fore"), this);

	QLabel* hand = new QLabel(tr("Hand"), this);

	QLabel* wing = new QLabel(tr("Wing Style"), this);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(pairs, row,0);
	lay->addWidget(m_ArmPairs, row,1);
	row++;
	lay->addWidget(type, row,0);
	lay->addWidget(m_ArmType, row,1);
	row++;
	lay->addWidget(wing, row,0);
	lay->addWidget(m_WingType, row,1);
	row++;
	lay->addWidget(len, row,1);
	lay->addWidget(thick, row,2);
	row++;
	lay->addWidget(upper, row,0);
	lay->addWidget(m_UpperLength, row,1);
	lay->addWidget(m_UpperThickness, row,2);
	row++;
	lay->addWidget(lower, row,0);
	lay->addWidget(m_LowerLength, row,1);
	lay->addWidget(m_LowerThickness, row,2);
	row++;
	lay->addWidget(hand, row,0);
	lay->addWidget(m_HandLength, row,1);
	lay->addWidget(m_HandType, row,2);
	row++;
	lay->setRowStretch(row, 1);
	lay->setColumnStretch(1, 1);
	lay->setColumnStretch(2, 1);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuIntroCreateArms::~AuIntroCreateArms()
{
}

void AuIntroCreateArms::DNATraits(DhDNA& dna) const
{
	if(m_ArmPairs->currentIndex() < 0 ||
			m_ArmType->currentIndex() < 0 ||
			m_WingType->currentIndex() < 0 ||
			m_HandType->currentIndex() < 0)
		return;
	dna.ArmPairs = m_ArmPairs->itemData(m_ArmPairs->currentIndex()).toUInt();
	dna.ArmType = m_ArmType->itemData(m_ArmType->currentIndex()).toUInt();
	dna.WingSubType = m_WingType->itemData(m_WingType->currentIndex()).toUInt();
	dna.UpperArmLength = m_UpperLength->value();
	dna.UpperArmThickness = m_UpperThickness->value();
	dna.ForeArmLength = m_LowerLength->value();
	dna.ForeArmThickness = m_LowerThickness->value();
	dna.HandLength = m_HandLength->value();
	dna.HandType = m_HandType->itemData(m_HandType->currentIndex()).toUInt();
}

void AuIntroCreateArms::SetDNA(const DhDNA& dna)
{
	m_ArmPairs->blockSignals(true);
	m_ArmType->blockSignals(true);
	m_WingType->blockSignals(true);
	m_UpperLength->blockSignals(true);
	m_UpperThickness->blockSignals(true);
	m_LowerLength->blockSignals(true);
	m_LowerThickness->blockSignals(true);
	m_HandLength->blockSignals(true);
	m_HandType->blockSignals(true);

	m_ArmPairs->setCurrentIndex(dna.ArmPairs);
	m_ArmType->setCurrentIndex(dna.ArmType);
	m_WingType->setCurrentIndex(dna.WingSubType);
	m_UpperLength->setValue(dna.UpperArmLength);
	m_UpperThickness->setValue(dna.UpperArmThickness);
	m_LowerLength->setValue(dna.ForeArmLength);
	m_LowerThickness->setValue(dna.ForeArmThickness);
	m_HandLength->setValue(dna.HandLength);
	m_HandType->setCurrentIndex(dna.HandType);

	m_ArmPairs->blockSignals(false);
	m_ArmType->blockSignals(false);
	m_WingType->blockSignals(false);
	m_UpperLength->blockSignals(false);
	m_UpperThickness->blockSignals(false);
	m_LowerLength->blockSignals(false);
	m_LowerThickness->blockSignals(false);
	m_HandLength->blockSignals(false);
	m_HandType->blockSignals(false);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroCreateLegs::AuIntroCreateLegs(AuIntroCreateAvatar* parent) : QWidget(parent)
{
	m_LegPairs = new QComboBox(this);
	m_LegPairs->addItem("No Legs", 0);
	m_LegPairs->addItem("Two Legs", 1);
	m_LegPairs->addItem("Four Legs", 2);
	m_LegPairs->addItem("Six Legs", 3);
	connect(m_LegPairs, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_LegType = new QComboBox(this);
	m_LegType->addItem(tr("Cilia"), DhDNA::LEG_Cilia);
	m_LegType->addItem(tr("Limb"), DhDNA::LEG_Limb);
	m_LegType->addItem(tr("Pseudopod"), DhDNA::LEG_Pseudopod);
	m_LegType->addItem(tr("Tentacle"), DhDNA::LEG_Tentacle);
	connect(m_LegType, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_FootLength = new QSlider(this);
	m_FootLength->setOrientation(Qt::Horizontal);
	m_FootLength->setRange(0,31);
	m_FootLength->setSingleStep(1);
	m_FootLength->setPageStep(1);
	connect(m_FootLength, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_FootType = new QComboBox(this);
	m_FootType->addItem(tr("Claw"), DhDNA::FOOT_Claw);
	m_FootType->addItem(tr("Flipper"), DhDNA::FOOT_Flipper);
	m_FootType->addItem(tr("Foot"), DhDNA::FOOT_Foot);
	m_FootType->addItem(tr("Hand"), DhDNA::FOOT_Hand);
	m_FootType->addItem(tr("Hoof"), DhDNA::FOOT_Hoof);
	m_FootType->addItem(tr("Paw"), DhDNA::FOOT_Paw);
	m_FootType->addItem(tr("Pincher"), DhDNA::FOOT_Pincher);
	m_FootType->addItem(tr("Sucker"), DhDNA::FOOT_Sucker);
	connect(m_FootType, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_LowerLength = new QSlider(this);
	m_LowerLength->setOrientation(Qt::Horizontal);
	m_LowerLength->setRange(0,31);
	m_LowerLength->setSingleStep(1);
	m_LowerLength->setPageStep(1);
	connect(m_LowerLength, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_LowerThickness = new QSlider(this);
	m_LowerThickness->setOrientation(Qt::Horizontal);
	m_LowerThickness->setRange(0,31);
	m_LowerThickness->setSingleStep(1);
	m_LowerThickness->setPageStep(1);
	connect(m_LowerThickness, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_UpperLength = new QSlider(this);
	m_UpperLength->setOrientation(Qt::Horizontal);
	m_UpperLength->setRange(0,31);
	m_UpperLength->setSingleStep(1);
	m_UpperLength->setPageStep(1);
	connect(m_UpperLength, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_UpperThickness = new QSlider(this);
	m_UpperThickness->setOrientation(Qt::Horizontal);
	m_UpperThickness->setRange(0,31);
	m_UpperThickness->setSingleStep(1);
	m_UpperThickness->setPageStep(1);
	connect(m_UpperThickness, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	QLabel* type = new QLabel(tr("Leg Type"), this);

	QLabel* pairs = new QLabel(tr("Legs"), this);

	QLabel* len = new QLabel(tr("Length"), this);

	QLabel* thick = new QLabel(tr("Thickness"), this);

	QLabel* upper = new QLabel(tr("Thigh"), this);

	QLabel* lower = new QLabel(tr("Calf"), this);

	QLabel* foot = new QLabel(tr("Foot"), this);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(pairs, row,0);
	lay->addWidget(m_LegPairs, row,1);
	row++;
	lay->addWidget(type, row,0);
	lay->addWidget(m_LegType, row,1);
	row++;
	lay->addWidget(len, row,1);
	lay->addWidget(thick, row,2);
	row++;
	lay->addWidget(upper, row,0);
	lay->addWidget(m_UpperLength, row,1);
	lay->addWidget(m_UpperThickness, row,2);
	row++;
	lay->addWidget(lower, row,0);
	lay->addWidget(m_LowerLength, row,1);
	lay->addWidget(m_LowerThickness, row,2);
	row++;
	lay->addWidget(foot, row,0);
	lay->addWidget(m_FootLength, row,1);
	lay->addWidget(m_FootType, row,2);
	row++;
	lay->setRowStretch(row, 1);
	lay->setColumnStretch(1, 1);
	lay->setColumnStretch(2, 1);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuIntroCreateLegs::~AuIntroCreateLegs()
{
}

void AuIntroCreateLegs::DNATraits(DhDNA& dna) const
{
	if(m_LegPairs->currentIndex() < 0 ||
			m_LegType->currentIndex() < 0 ||
			m_FootType->currentIndex() < 0)
		return;
	dna.LegPairs = m_LegPairs->itemData(m_LegPairs->currentIndex()).toUInt();
	dna.LegType = m_LegType->itemData(m_LegType->currentIndex()).toUInt();
	dna.ThighLength = m_UpperLength->value();
	dna.ThighThickness = m_UpperThickness->value();
	dna.CalfLength = m_LowerLength->value();
	dna.CalfThickness = m_LowerThickness->value();
	dna.FootLength = m_FootLength->value();
	dna.FootType = m_FootType->itemData(m_FootType->currentIndex()).toUInt();
}

void AuIntroCreateLegs::SetDNA(const DhDNA& dna)
{
	m_LegPairs->blockSignals(true);
	m_LegType->blockSignals(true);
	m_UpperLength->blockSignals(true);
	m_UpperThickness->blockSignals(true);
	m_LowerLength->blockSignals(true);
	m_LowerThickness->blockSignals(true);
	m_FootLength->blockSignals(true);
	m_FootType->blockSignals(true);

	m_LegPairs->setCurrentIndex(dna.LegPairs);
	m_LegType->setCurrentIndex(dna.LegType);
	m_UpperLength->setValue(dna.ThighLength);
	m_UpperThickness->setValue(dna.ThighThickness);
	m_LowerLength->setValue(dna.CalfLength);
	m_LowerThickness->setValue(dna.CalfThickness);
	m_FootLength->setValue(dna.FootLength);
	m_FootType->setCurrentIndex(dna.FootType);

	m_LegPairs->blockSignals(false);
	m_LegType->blockSignals(false);
	m_UpperLength->blockSignals(false);
	m_UpperThickness->blockSignals(false);
	m_LowerLength->blockSignals(false);
	m_LowerThickness->blockSignals(false);
	m_FootLength->blockSignals(false);
	m_FootType->blockSignals(false);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroCreateTail::AuIntroCreateTail(AuIntroCreateAvatar* parent) : QWidget(parent)
{
	m_Size = new QSlider(this);
	m_Size->setOrientation(Qt::Horizontal);
	m_Size->setRange(0,3);
	m_Size->setSingleStep(1);
	m_Size->setPageStep(1);
	connect(m_Size, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_Type = new QComboBox(this);
	m_Type->addItem("1", 0);
	m_Type->addItem("2", 1);
	m_Type->addItem("3", 2);
	m_Type->addItem("4", 3);
	connect(m_Type, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	QLabel* size = new QLabel(tr("Tail Size"), this);

	QLabel* type = new QLabel(tr("Tail Type"), this);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(type, row,0);
	lay->addWidget(m_Type, row,1);
	row++;
	lay->addWidget(size, row,0);
	lay->addWidget(m_Size, row,1);
	row++;
	lay->setRowStretch(row, 1);
	lay->setColumnStretch(1, 1);
	lay->setColumnStretch(2, 1);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuIntroCreateTail::~AuIntroCreateTail()
{
}

void AuIntroCreateTail::DNATraits(DhDNA& dna) const
{
	if(m_Type->currentIndex() < 0)
		return;
	dna.TailType = m_Type->itemData(m_Type->currentIndex()).toUInt();
	dna.TailVertebrae = m_Size->value();
}

void AuIntroCreateTail::SetDNA(const DhDNA& dna)
{
	m_Type->blockSignals(true);
	m_Size->blockSignals(true);

	m_Type->setCurrentIndex(dna.TailType);
	m_Size->setValue(dna.TailVertebrae);

	m_Type->blockSignals(false);
	m_Size->blockSignals(false);
}

void AuIntroCreateTail::SetPhylum(/*DhDNA::SKIN*/int phylum)
{
	int ct = m_Type->currentIndex();
	m_Type->blockSignals(true);
	m_Type->clear();
	switch(phylum)
	{
	case DhDNA::SKIN_DryExoskeleton:
		m_Type->addItem("None", 0);
		m_Type->addItem("Abdomen", 1);
		m_Type->addItem("Pincher", 2);
		m_Type->addItem("Scorpion", 3);
		break;
	case DhDNA::SKIN_DryScales:
		m_Type->addItem("None", 0);
		m_Type->addItem("Dragon", 1);
		m_Type->addItem("Hammer", 2);
		m_Type->addItem("Scorpion", 3);
		break;
	case DhDNA::SKIN_DrySkin:
		m_Type->addItem("None", 0);
		m_Type->addItem("Monkey", 1);
		m_Type->addItem("Peacock", 2);
		m_Type->addItem("Dragon", 3);
		break;
	case DhDNA::SKIN_Feathers:
		m_Type->addItem("None", 0);
		m_Type->addItem("Scorpion", 1);
		m_Type->addItem("Peacock", 2);
		m_Type->addItem("Feathers", 3);
		break;
	case DhDNA::SKIN_Fur:
		m_Type->addItem("None", 0);
		m_Type->addItem("Lion", 1);
		m_Type->addItem("Hammer", 2);
		m_Type->addItem("Shark", 3);
		break;
	case DhDNA::SKIN_WetExoskeleton:
		m_Type->addItem("None", 0);
		m_Type->addItem("Lobster", 1);
		m_Type->addItem("Shark", 2);
		m_Type->addItem("Pincher", 3);
		break;
	case DhDNA::SKIN_WetScales:
		m_Type->addItem("None", 0);
		m_Type->addItem("Dragon", 1);
		m_Type->addItem("Whale", 2);
		m_Type->addItem("Shark", 3);
		break;
	case DhDNA::SKIN_WetSkin:
		m_Type->addItem("None", 0);
		m_Type->addItem("Dragon", 1);
		m_Type->addItem("Whale", 2);
		m_Type->addItem("Tadpole", 3);
		break;
	}
	m_Type->setCurrentIndex(ct);
	m_Type->blockSignals(false);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroCreateWeapon::AuIntroCreateWeapon(AuIntroCreateAvatar* parent) : QWidget(parent)
{
	m_Location = new QComboBox(this);
	m_Location->addItem(tr("Primary"), 0);
	m_Location->addItem(tr("Secondary"), 1);
	m_Location->setToolTip(QObject::tr("<html>"
				"Body location of the weapon. "
				"</html>"));
	connect(m_Location, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_Mode = new QComboBox(this);
	m_Mode->addItem(QIcon(":data/wm_Area.png"), tr("Area"), DhDNA::DWM_Area);
	m_Mode->addItem(QIcon(":data/wm_Contact.png"), tr("Contact"), DhDNA::DWM_Contact);
	m_Mode->addItem(QIcon(":data/wm_Projectile.png"), tr("Projectile"), DhDNA::DWM_Projectile);
	m_Mode->addItem(QIcon(":data/wm_Psionic.png"), tr("Psionic"), DhDNA::DWM_Psionic);
	m_Mode->setToolTip(QObject::tr("<html>"
				"Method of inflicting damage using the weapon. "
				"<br>"
				"<i>Area</i> weapons damage things within a radius of the creature. "
				"Area weapons do not affect other creatures with the same DNA. "
				"<br>"
				"<i>Contact</i> weapons require the target to be touched, typically by striking. "
				"<br>"
				"<i>Projectile</i> weapons damage a specific target within range, by projecting the weapon's effect. "
				"<br>"
				"<i>Psionic</i> weapons use mental energy to attack a specific target within range, often negating armor. "
				"</html>"));
	connect(m_Mode, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	m_Size = new QSlider(this);
	m_Size->setOrientation(Qt::Horizontal);
	m_Size->setRange(0,7);
	m_Size->setSingleStep(1);
	m_Size->setPageStep(1);
	connect(m_Size, SIGNAL(valueChanged(int)), SIGNAL(DNAChanged()));

	m_Type = new QComboBox(this);
	m_Type->addItem(tr("None"), DhDNA::WT_None);
	m_Type->addItem(QIcon(":data/wt_Acid.png"), tr("Acid"), DhDNA::WT_Acid);
	m_Type->addItem(QIcon(":data/wt_Suffocate.png"), tr("Suffocation"), DhDNA::WT_Suffocate);
	m_Type->addItem(QIcon(":data/wt_Stun.png"), tr("Paralysis"), DhDNA::WT_Stun);
	m_Type->addItem(QIcon(":data/wt_Poison.png"), tr("Poisonous"), DhDNA::WT_Poison);
	m_Type->addItem(QIcon(":data/wt_Radiation.png"), tr("Radiation"), DhDNA::WT_Radiation);
	m_Type->addItem(QIcon(":data/wt_Disease.png"), tr("Disease"), DhDNA::WT_Disease);
	m_Type->addItem(QIcon(":data/wt_Electric.png"), tr("Electrical"), DhDNA::WT_Electric);
	m_Type->addItem(QIcon(":data/wt_Bludgeon.png"), tr("Bludgeon"), DhDNA::WT_Bludgeon);
	m_Type->addItem(QIcon(":data/wt_Cold.png"), tr("Freezing"), DhDNA::WT_Cold);
	m_Type->addItem(QIcon(":data/wt_Crush.png"), tr("Crushing"), DhDNA::WT_Crush);
	m_Type->addItem(QIcon(":data/wt_Sonic.png"), tr("Sonic"), DhDNA::WT_Sonic);
	m_Type->addItem(QIcon(":data/wt_Heat.png"), tr("Burning"), DhDNA::WT_Heat);
	m_Type->addItem(QIcon(":data/wt_Pierce.png"), tr("Piercing"), DhDNA::WT_Pierce);
	m_Type->addItem(QIcon(":data/wt_Implant.png"), tr("Implant"), DhDNA::WT_Implant);
	m_Type->addItem(QIcon(":data/wt_Slash.png"), tr("Slashing"), DhDNA::WT_Slash);
	connect(m_Type, SIGNAL(currentIndexChanged(int)), SIGNAL(DNAChanged()));

	QLabel* size = new QLabel(tr("Weapon Size"), this);

	QLabel* type = new QLabel(tr("Weapon Type"), this);

	QLabel* loca = new QLabel(tr("Weapon Location"), this);

	QLabel* mode = new QLabel(tr("Weapon Mode"), this);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(mode, row,0);
	lay->addWidget(m_Mode, row,1);
	row++;
	lay->addWidget(type, row,0);
	lay->addWidget(m_Type, row,1);
	row++;
	lay->addWidget(loca, row,0);
	lay->addWidget(m_Location, row,1);
	row++;
	lay->addWidget(size, row,0);
	lay->addWidget(m_Size, row,1);
	row++;
	lay->setRowStretch(row, 1);
	lay->setColumnStretch(1, 1);
	lay->setColumnStretch(2, 1);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuIntroCreateWeapon::~AuIntroCreateWeapon()
{
}

void AuIntroCreateWeapon::DNATraits(DhDNA& dna) const
{
	if(m_Mode->currentIndex() < 0 ||
			m_Type->currentIndex() < 0 ||
			m_Location->currentIndex() < 0)
		return;
	dna.WeaponMode = m_Mode->itemData(m_Mode->currentIndex()).toUInt();
	dna.WeaponType = m_Type->itemData(m_Type->currentIndex()).toUInt();
	dna.WeaponAltLocation = m_Location->currentIndex();
	dna.WeaponSize = m_Size->value();
}

void AuIntroCreateWeapon::SetDNA(const DhDNA& dna)
{
	m_Mode->blockSignals(true);
	m_Type->blockSignals(true);
	m_Location->blockSignals(true);
	m_Size->blockSignals(true);

	m_Mode->setCurrentIndex(dna.WeaponMode);
	m_Type->setCurrentIndex(dna.WeaponType);
	m_Location->setCurrentIndex(dna.WeaponAltLocation);
	m_Size->setValue(dna.WeaponSize);

	m_Mode->blockSignals(false);
	m_Type->blockSignals(false);
	m_Size->blockSignals(false);
	m_Location->blockSignals(false);
}

void AuIntroCreateWeapon::UpdateWeaponLocations(const DhDNA& dna)
{
	int wpnLoc = m_Location->currentIndex();
	m_Location->blockSignals(true);
	m_Location->clear();
	if(dna.WeaponType == DhDNA::WT_None)
	{
		m_Location->addItem(tr("Primary"));
		m_Location->addItem(tr("Alternate"));
	}
	else
	{
		// Primary and alternate weapon locations.
		DhDNA pdna(dna);
		pdna.WeaponAltLocation = 0;
		m_Location->addItem(QIcon(pdna.WeaponImageName()), pdna.WeaponLocationName());
		pdna.WeaponAltLocation = 1;
		m_Location->addItem(QIcon(pdna.WeaponImageName()), pdna.WeaponLocationName());
	}
	m_Location->setCurrentIndex(wpnLoc);
	m_Location->blockSignals(false);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroStartReach::AuIntroStartReach(QWidget* parent) : QWidget(parent)
{
	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"<span style=\"color: #ffffff;\"><big><b>%1</b></big></span>"
				"<br>"
				"Begin as an indigenous villager. "
				"Build a civilization or die. "
				"</p>"
				"<p>"
				"Your avatar enters Hazeron on an unexplored habitable world. "
				"</p>"
				"<p>"
				"Targoss arrives to show how to play the game. "
				"Later, Гага́рин shows how to fly a rocket. "
				"</p>"
				"</html>")
			.arg(ScenarioName(ReachForTheStars)), this);
	lbinfo->setAlignment(Qt::AlignLeft|Qt::AlignTop);
	lbinfo->setWordWrap(true);

	QLabel* regionlabel = new QLabel(tr("Homeworld Region"), this);

	m_Region = new QComboBox(this);
	m_Region->addItem(QObject::tr("Eastern Cluster"), 0);
	m_Region->addItem(QObject::tr("Western Cluster"), 1);
	m_Region->addItem(QObject::tr("Northern Cluster"), 2);
	m_Region->addItem(QObject::tr("Southern Cluster"), 3);
	m_Region->addItem(QObject::tr("Eastern Frontier"), 4);
	m_Region->addItem(QObject::tr("Western Frontier"), 5);
	m_Region->addItem(QObject::tr("Northern Frontier"), 6);
	m_Region->addItem(QObject::tr("Southern Frontier"), 7);
	m_Region->addItem(QObject::tr("Remote"), 8);
	m_Region->setToolTip(tr("<html>"
				"<p>"
				"Choose a region of the Shores of Hazeron galaxy to start. "
				"</p>"
				"<p>"
				"When starting in a <i>cluster</i>, your homeworld may be in a sector that contains other avatars. "
				"</p>"
				"<p>"
				"When starting on the <i>frontier</i>, your homeworld will be in a sector that does not contain other avatars. "
				"Frontier sectors are uninhabited and unnamed. "
				"They are always next to a named sector, which means someone built a city there once. "
				"</p>"
				"<p>"
				"When starting in a <i>remote</i> region, you are least likely to start near existing avatars. "
				"Remote sectors are uninhabited and unnamed. "
				"They are not next to any named sector. "
				"</p>"
				"</html>"));
	m_Region->setCurrentIndex(m_Region->findData(0));	// Eastern cluster of default galaxy Shores of Hazeron

	m_RoidIco = new QLabel(this);
	m_RoidIco->setText(tr("<html>"
				"<p>"
				"<img height=\"32\" align=\"left\" src=\"qrc:data/Asteroid.png\"> "
				"<span style=\"color: rgb(255, 130, 32);\"><b>A giant asteroid will annihilate your homeworld one week from now, "
				"whether you are online or not.</b></span> "
				"Destroy the asteroid or colonize another planet before it hits. "
				"</p>"
				"</html>"));
	m_RoidIco->setMinimumWidth(400);
	m_RoidIco->setWordWrap(true);

	QDateTime impact = QDateTime::currentDateTime().addDays(7);
	QDateTime impactMin = impact.addDays(-1);
	QDateTime impactMax = impact.addDays(1);
	m_Asteroid = new QDateTimeEdit(this);
	m_Asteroid->setCalendarPopup(true);
	m_Asteroid->setDateTime(impact);
	m_Asteroid->setDateTimeRange(impactMin, impactMax);
	m_Asteroid->setToolTip(tr("<html>"
				"<p>"
				"Asteroid impact time may be adjusted, up to one day sooner or later, to accommodate your real life. "
				"Plan to intercept the asteroid an hour or more before the impact time. "
				"</p>"
				"</html>"));

	QLabel* lbimpact = new QLabel(tr("Impact Time"), this);

	QGridLayout* alay = new QGridLayout;
	int row=0;
	alay->addWidget(lbimpact, row,0);
	alay->addWidget(m_Asteroid, row,1);
	row++;
	alay->addWidget(regionlabel, row,0);
	alay->addWidget(m_Region, row,1);
	alay->setRowStretch(0,1);
	alay->setRowStretch(row,1);
	alay->setSpacing(2);
	alay->setMargin(0);

	QVBoxLayout* vlay = new QVBoxLayout;
	vlay->addWidget(m_RoidIco);
	vlay->addLayout(alay);
	vlay->setMargin(0);

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(lbinfo, 1);
	lay->addLayout(vlay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuIntroStartReach::~AuIntroStartReach()
{
}

QDateTime AuIntroStartReach::AsteroidImpact() const
{
	return m_Asteroid->dateTime();
}

void AuIntroStartReach::SetSolo(bool solo)
{
	m_Solo = solo;
	if(solo)
	{
		m_RoidIco->setText(tr("<html>"
					"<p>"
					"<img height=\"32\" align=\"left\" src=\"qrc:data/Asteroid.png\"> "
					"<span style=\"color: rgb(255, 130, 32);\">"
					"<b>A giant asteroid will annihilate your homeworld one week from now, "
					"while you are playing the game.</b></span> "
					"Destroy the asteroid or colonize another planet before it hits. "
					"The asteroid will pass harmlessly if the game is not running during the scheduled impact. "
					"</p>"
					"</html>"));
	}
	else
	{
		m_RoidIco->setText(tr("<html>"
					"<p>"
					"<img height=\"32\" align=\"left\" src=\"qrc:data/Asteroid.png\"> "
					"<span style=\"color: rgb(255, 130, 32);\">"
					"<b>A giant asteroid will annihilate your homeworld one week from now, "
					"whether you are online or not.</b></span> "
					"Destroy the asteroid or colonize another planet before it hits. "
					"</p>"
					"</html>"));
	}
}

bool AuIntroStartReach::OkToGo() const
{
	return true;
}

quint8 AuIntroStartReach::Region() const
{
	return m_Region->itemData(m_Region->currentIndex()).toUInt();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroStartCitizen::AuIntroStartCitizen(QWidget* parent) : QWidget(parent)
{
	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"<span style=\"color: #ffffff;\"><big><b>%1</b></big></span>"
				"<br>"
				"Begin as a citizen of an existing empire. "
				"</p>"
				"<p>"
				"Your avatar enters Hazeron on an unexplored habitable world. "
				"</p>"
				"</html>")
			.arg(ScenarioName(CitizenOfTheGalaxy)), this);
	lbinfo->setAlignment(Qt::AlignLeft|Qt::AlignTop);
	lbinfo->setWordWrap(true);

	AuPictureWidget* pic = new AuPictureWidget(true, this);
	pic->SetImage(QImage(":data/GovCommerce.png"));
	pic->setMaximumSize(QSize(48,48));

	m_EmpiresCombo = new QComboBox(this);
	m_EmpiresCombo->setToolTip(tr("<html>"
				"Choose an empire to join. "
				"To be listed, an empire must have at least one city and the emperor must set a join password. "
				"</html>"));
	connect(m_EmpiresCombo, SIGNAL(currentIndexChanged(int)), SLOT(EmpireChanged(int)));

	m_JoinPass = new QLineEdit(this);
	m_JoinPass->setToolTip(tr("<html>"
				"The empire requires a password to join. Enter the password here. "
				"</html>"));
	m_JoinPass->setEchoMode(QLineEdit::Password);
	connect(m_JoinPass, SIGNAL(textChanged(const QString&)), SLOT(PasswordChanged(const QString&)));

	m_Sectors = new QComboBox(this);
	m_Sectors->setToolTip(tr("<html>"
				"Choose a sector to start. "
				"Sectors are listed that contain at least one city of the empire. "
				"Your homeworld will be in or near the sector you choose. "
				"</html>"));

	QGridLayout* lay = new QGridLayout;
	lay->addWidget(lbinfo, 0,0, 5,1);
	lay->addWidget(pic, 0,1, 5,1);
	lay->addWidget(m_EmpiresCombo, 1,2, 1,1);
	lay->addWidget(m_JoinPass, 2,2, 1,1);
	lay->addWidget(m_Sectors, 3,2, 1,1);
	lay->setMargin(0);
	lay->setColumnStretch(0, 1);
	setLayout(lay);
}

AuIntroStartCitizen::~AuIntroStartCitizen()
{
}

void AuIntroStartCitizen::setVisible(bool visible)
{
	if(visible)
	{
		const DgData& gd = g_Au.LockDataForRead();
		gd.ListEmpiresToJoin(*m_EmpiresCombo);
		g_Au.UnlockData();
	}
	QWidget::setVisible(visible);
}

quint32 AuIntroStartCitizen::EmpireToJoin() const
{
	return m_EmpiresCombo->currentData().toUInt();
}

QByteArray AuIntroStartCitizen::EmpireJoinPassCrypt() const
{
	QString password = m_JoinPass->text();

	// Create a SHA-1 hash key from the password.
	QByteArray passCrypt;
	passCrypt.append(password.toUtf8());
	QCryptographicHash passCryptHash(QCryptographicHash::Sha1);
	passCryptHash.addData(passCrypt);
	return passCryptHash.result();
}

bool AuIntroStartCitizen::OkToGo() const
{
	quint32 empireId = EmpireToJoin();
	if(!empireId)
		return false;
	AuId sectorId = EmpireJoinSector();
	if(sectorId.IsNull())
		return false;
	QString password = m_JoinPass->text();
	const DgData& gd = g_Au.LockDataForRead();
	bool restricted = gd.EmpireJoinRestricted(empireId);	// true if join is not open
	bool testOk = gd.EmpireTestJoinPassword(empireId, password);	// true if has a password and password matches
	g_Au.UnlockData();
	if(!restricted)
		return true;	// Empire immig is open.
	return testOk;
}

void AuIntroStartCitizen::SetSolo(bool solo)
{
	m_JoinPass->setVisible(!solo);
	if(!solo)
		EmpireChanged(m_EmpiresCombo->currentIndex());	// Hide/show the password box as appropriate.
}

void AuIntroStartCitizen::EmpireChanged(int index)
{
	quint32 empireId = m_EmpiresCombo->itemData(index).toUInt();
	const DgData& gd = g_Au.LockDataForRead();
	bool hasPass = gd.EmpireHasJoinPassword(empireId);
	g_Au.UnlockData();

	m_JoinPass->clear();
	m_JoinPass->setVisible(hasPass);

	m_Sectors->setVisible(!hasPass);
	ListEmpireSectors(empireId);

	emit OkToGoChanged();
}

void AuIntroStartCitizen::PasswordChanged(const QString& password)
{
	quint32 empireId = EmpireToJoin();
	const DgData& gd = g_Au.LockDataForRead();
	bool testOk = gd.EmpireTestJoinPassword(empireId, password);	// true if has a password and password matches
	g_Au.UnlockData();

	m_Sectors->setVisible(testOk);

	emit OkToGoChanged();
}

void AuIntroStartCitizen::ListEmpireSectors(quint32 empireId)
{
	// Get a copy of the empire record for its cities.
	DgEmpire emp;
	emp.m_id = empireId;
	const DgData& gd = g_Au.LockDataForRead();
	gd.Empire(emp);
	g_Au.UnlockData();

	// Make a unique list of sectors that contain empire cities.
	m_SectorGalaxies.clear();
	m_Sectors->clear();
	for(AuCityHash::const_iterator ci = emp.m_Cities->cbegin(); ci != emp.m_Cities->cend(); ci++)
	{
		if(m_SectorGalaxies.contains((*ci).second.SectorId()))
			continue;
		m_SectorGalaxies.insert((*ci).second.SectorId(), (*ci).second.Galaxy());

		QString nm = tr("%1 %2").arg((*ci).second.SectorName()).arg(GalaxyName((*ci).second.Galaxy()));
		m_Sectors->addItem(QIcon(":data/Sector.png"), nm, QVariant::fromValue((*ci).second.SectorId()));
	}
}

GALAXY AuIntroStartCitizen::EmpireJoinGalaxy() const
{
	AuId sectorId = EmpireJoinSector();
	return m_SectorGalaxies[sectorId];
}

AuId AuIntroStartCitizen::EmpireJoinSector() const
{
	return m_Sectors->currentData().value<AuId>();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroStartSavage::AuIntroStartSavage(QWidget* parent) : QWidget(parent)
{
	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"<span style=\"color: #ffffff;\"><big><b>%1</b></big></span>"
				"<br>"
				"Begin as a savage in the wilderness. "
				"</p>"
				"<p>"
				"Your avatar enters Hazeron on an unexplored habitable world. "
				"</p>"
				"</html>")
			.arg(ScenarioName(SavageState)), this);
	lbinfo->setAlignment(Qt::AlignLeft|Qt::AlignTop);
	lbinfo->setWordWrap(true);

	QLabel* regionlabel = new QLabel(tr("Homeworld Region"), this);

	m_Region = new QComboBox(this);
	m_Region->addItem(QObject::tr("Eastern Cluster"), 0);
	m_Region->addItem(QObject::tr("Western Cluster"), 1);
	m_Region->addItem(QObject::tr("Northern Cluster"), 2);
	m_Region->addItem(QObject::tr("Southern Cluster"), 3);
	m_Region->addItem(QObject::tr("Eastern Frontier"), 4);
	m_Region->addItem(QObject::tr("Western Frontier"), 5);
	m_Region->addItem(QObject::tr("Northern Frontier"), 6);
	m_Region->addItem(QObject::tr("Southern Frontier"), 7);
	m_Region->addItem(QObject::tr("Remote"), 8);
	m_Region->setToolTip(tr("<html>"
				"<p>"
				"Choose a region of the Shores of Hazeron galaxy to start. "
				"</p>"
				"<p>"
				"When starting in a <i>cluster</i>, your homeworld may be in a sector that contains other avatars. "
				"</p>"
				"<p>"
				"When starting on the <i>frontier</i>, your homeworld will be in a sector that does not contain other avatars. "
				"Frontier sectors are uninhabited and unnamed. "
				"They are always next to a named sector, which means someone built a city there once. "
				"</p>"
				"<p>"
				"When starting in a <i>remote</i> region, you are least likely to start near existing avatars. "
				"Remote sectors are uninhabited and unnamed. "
				"They are not next to any named sector. "
				"</p>"
				"</html>"));
	m_Region->setCurrentIndex(m_Region->findData(0));	// Eastern cluster of default galaxy Shores of Hazeron

	AuPictureWidget* pic = new AuPictureWidget(true, this);
	pic->SetImage(QImage(":data/ThomasCole.1.Savage.png"));
	pic->setMaximumSize(QSize(240,80));

	QHBoxLayout* rlay = new QHBoxLayout;
	rlay->addWidget(regionlabel);
	rlay->addWidget(m_Region);
	rlay->setMargin(0);

	QVBoxLayout* vlay = new QVBoxLayout;
	vlay->addStretch();
	vlay->addWidget(pic);
	vlay->addLayout(rlay);
	vlay->addStretch();
	vlay->setMargin(0);

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(lbinfo, 1);
	lay->addLayout(vlay);
	lay->setMargin(0);
	setLayout(lay);
}

AuIntroStartSavage::~AuIntroStartSavage()
{
}

bool AuIntroStartSavage::OkToGo() const
{
	return true;
}

quint8 AuIntroStartSavage::Region() const
{
	return m_Region->itemData(m_Region->currentIndex()).toUInt();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroStartLost::AuIntroStartLost(QWidget* parent) : QWidget(parent)
{
	m_IsOk = false;

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"<span style=\"color: #ffffff;\"><big><b>%1</b></big></span>"
				"<br>"
				"Begin as captain of the starship <i>Atlantis</i>, lost in space and time. "
				"No trace of your former civilization exists. "
				"</p>"
				"<p>"
				"Your avatar enters Hazeron aboard a starship in deep space. "
				"</p>"
				"</html>")
			.arg(ScenarioName(LostAtlantis)), this);
	lbinfo->setAlignment(Qt::AlignLeft|Qt::AlignTop);
	lbinfo->setWordWrap(true);

	QLabel* regionlabel = new QLabel(tr("Homeworld Region"), this);

	m_Region = new QComboBox(this);
	m_Region->addItem(QObject::tr("Eastern Cluster"), 0);
	m_Region->addItem(QObject::tr("Western Cluster"), 1);
	m_Region->addItem(QObject::tr("Northern Cluster"), 2);
	m_Region->addItem(QObject::tr("Southern Cluster"), 3);
	m_Region->addItem(QObject::tr("Eastern Frontier"), 4);
	m_Region->addItem(QObject::tr("Western Frontier"), 5);
	m_Region->addItem(QObject::tr("Northern Frontier"), 6);
	m_Region->addItem(QObject::tr("Southern Frontier"), 7);
	m_Region->addItem(QObject::tr("Remote"), 8);
	m_Region->setToolTip(tr("<html>"
				"<p>"
				"Choose a region of the Shores of Hazeron galaxy to start. "
				"</p>"
				"<p>"
				"When starting in a <i>cluster</i>, you will start in a sector that contains other avatars. "
				"</p>"
				"<p>"
				"When starting on the <i>frontier</i>, you will start in a sector that does not contain other avatars. "
				"Frontier sectors are uninhabited and unnamed. "
				"They are always next to a named sector, which means someone built a city there once. "
				"</p>"
				"<p>"
				"When starting in a <i>remote</i> region, you are least likely to start near existing avatars. "
				"Remote sectors are uninhabited and unnamed. "
				"They are not next to any named sector. "
				"</p>"
				"</html>"));
	m_Region->setCurrentIndex(m_Region->findData(0));	// Eastern cluster of default galaxy Shores of Hazeron

	m_FlagButton = new QToolButton(this);
	QPixmap flagpix = QPixmap(":data/SurrenderFlag.png");
	m_FlagButton->setIcon(QIcon(flagpix));
	m_FlagButton->setIconSize(QSize(64,64));
	m_FlagButton->setToolTip(tr("<html>"
				"Choose the picture of the flag for the new empire. "
				"</html>"));
	m_FlagButton->setMaximumSize(QSize(68,68));
	m_FlagButton->setMinimumSize(QSize(68,68));
	connect(m_FlagButton, SIGNAL(clicked()), SLOT(ChooseFlag()));

	m_NameEdit = new QLineEdit(this);
	m_NameEdit->setToolTip(tr("<html>"
			"Enter a name for the new empire. "
			"The name must be less than %1 characters and "
			"it must not contain certain reserved characters such as | and newline. "
			"</html>").arg(MaxNameLength()+1));
	connect(m_NameEdit, SIGNAL(textChanged(const QString&)), SLOT(EnableControls()));

	m_NameReason = new QLabel(this);

	AuPictureWidget* pic = new AuPictureWidget(true, this);
	pic->SetImage(QImage(":data/LostAtlantis.png"));
	pic->setMaximumSize(QSize(235,64));

	QGridLayout* glay = new QGridLayout;
	int row=1;
	glay->addWidget(m_FlagButton, row,0, 2,1);
	glay->addWidget(m_NameEdit, row,1, 1,2);
	row++;
	glay->addWidget(m_NameReason, row,1, 1,2);
	row++;
	glay->addWidget(regionlabel, row,1);
	glay->addWidget(m_Region, row,2);
	row++;
	glay->setRowStretch(0,1);
	glay->setRowStretch(row,1);
	glay->setMargin(0);

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(lbinfo, 1);
	lay->addWidget(pic);
	lay->addLayout(glay);
	lay->setMargin(0);
	setLayout(lay);
}

AuIntroStartLost::~AuIntroStartLost()
{
}

void AuIntroStartLost::setVisible(bool visible)
{
	QWidget::setVisible(visible);
	if(visible)
		EnableControls();
}

bool AuIntroStartLost::OkToGo() const
{
	return m_IsOk;
}

QString AuIntroStartLost::NewEmpireName() const
{
	return m_NameEdit->text().trimmed();
}

void AuIntroStartLost::EnableControls()
{
	QString reason;
	QString name = NewEmpireName();
	if(name.isEmpty())
		reason = tr("Empire name needed.");
	else if(!IsNameValid(name))
		reason = tr("Empire name is not valid.");
	else
	{
		const DgData& gd = g_Au.LockDataForRead();
		quint32 empireId = gd.EmpireId(name);
		g_Au.UnlockData();
		if(empireId)
			reason = tr("Empire name already exists.");
		else if(m_Flag.isNull())
			reason = tr("Flag picture needed.");
	}
	m_NameReason->setText(reason);
	m_IsOk = reason.isEmpty();
	emit OkToGoChanged();
}

void AuIntroStartLost::ChooseFlag()
{
	// Load last known file folder.
	QString lastFile = g_Au.Settings().readEntry("/Settings/LastFlagFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();
	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QString file = QFileDialog::getOpenFileName(this, tr("Choose Flag Image"), lastFile, "", 0,
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(!file.isEmpty())
	{
		// Store last known file folder.
		g_Au.Settings().writeEntry("/Settings/LastFlagFile", file);

		QImage imFlag;
		if(imFlag.load(file))
		{
			if(imFlag.width() > DgEmpire::FlagImageMax() || imFlag.height() > DgEmpire::FlagImageMax())
			{
				// Resize the image.
				m_Flag = imFlag.scaled(DgEmpire::FlagImageMax(),DgEmpire::FlagImageMax(), Qt::KeepAspectRatio,
						Qt::SmoothTransformation);
			}
			else
			{
				m_Flag = imFlag;
				if(m_Flag.width() < 2 || m_Flag.height() < 2)
				{
					QMessageBox::critical(this, g_Au.ApplicationName(),
							tr("<html>"
								"The flag picture is too small. "
								"Please select a picture that is at least 2 pixels in width and height. "
								"</html>"),
							QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
					m_Flag = QImage();	// Discard invalid image.
				}
			}
			m_Flag = m_Flag.convertToFormat(QImage::Format_Indexed8);	// Reduce color depth for size.

			QPixmap flagpix = QPixmap::fromImage(m_Flag);
			m_FlagButton->blockSignals(true);
			m_FlagButton->setIcon(QIcon(flagpix));
			m_FlagButton->setIconSize(m_Flag.size());
			m_FlagButton->blockSignals(false);
			EnableControls();
		}
		else
		{
			QMessageBox::critical(this, g_Au.ApplicationName(),
					tr("The flag image could not be loaded. "
						"Many image formats are supported but some JPEG sub-formats give us trouble. "
						"If the image can be loaded with a paint program, try saving it as a different "
						"format of image such as PNG or BMP. "),
				QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		}
	}
}

quint8 AuIntroStartLost::Region() const
{
	return m_Region->itemData(m_Region->currentIndex()).toUInt();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIntroStartScenarios::AuIntroStartScenarios(QWidget* parent) : QWidget(parent)
{
	m_Solo = false;
	m_Pages = new QStackedLayout();

	m_Scenarios = new QComboBox(this);
	m_Scenarios->setToolTip(tr("<html>"
				"<p>"
				"Choose a starting scenario for the avatar. "
				"</p>"
				"</html>"));
	connect(m_Scenarios, SIGNAL(currentIndexChanged(int)), SLOT(ScenarioChanged(int)));

	// Create all scenario configuration pages.
	{
		m_Reach = new AuIntroStartReach(this);
		m_Pages->addWidget(m_Reach);
	}
	{
		m_Citizen = new AuIntroStartCitizen(this);
		connect(m_Citizen, SIGNAL(OkToGoChanged()), SIGNAL(OkToGoChanged()));
		m_Pages->addWidget(m_Citizen);
	}
	{
		m_Savage = new AuIntroStartSavage(this);
		m_Pages->addWidget(m_Savage);
	}
	{
		m_Lost = new AuIntroStartLost(this);
		connect(m_Lost, SIGNAL(OkToGoChanged()), SIGNAL(OkToGoChanged()));
		m_Pages->addWidget(m_Lost);
	}
	m_Pages->setCurrentIndex(0);

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addLayout(m_Pages);
	lay->addWidget(m_Scenarios);
	lay->setMargin(0);
	setLayout(lay);
}

AuIntroStartScenarios::~AuIntroStartScenarios()
{
}

void AuIntroStartScenarios::setVisible(bool visible)
{
	if(visible)
	{
		if(!m_Solo)
		{
			// We will need these for the CitizenOfTheGalaxy scenario.
			//qDebug() << "AuIntroStartScenarios::setVisible requesting joinable empires";
			auApp->RequestEmpiresJoinable();
		}
	}
	QWidget::setVisible(visible);
}

void AuIntroStartScenarios::ScenarioChanged(int index)
{
	m_Pages->setCurrentIndex(index);
	emit OkToGoChanged();
}

void AuIntroStartScenarios::SetSolo(bool solo)
{
	m_Solo = solo;
	m_Reach->SetSolo(solo);
	m_Citizen->SetSolo(solo);

	// Update scenarios available.
	m_Scenarios->clear();
	if(solo)
	{
		for(int s=0; s<SCENARIO_Count; s++)
			if(ScenarioOkSolo((SCENARIO)s))
				m_Scenarios->addItem(ScenarioName((SCENARIO)s), s);
	}
	else
	{
		for(int s=0; s<SCENARIO_Count; s++)
			if(ScenarioOkMultiplayer((SCENARIO)s))
				m_Scenarios->addItem(ScenarioName((SCENARIO)s), s);
	}
	m_Scenarios->setCurrentIndex(m_Scenarios->findData(ReachForTheStars));
}

SCENARIO AuIntroStartScenarios::Scenario() const
{
	return (SCENARIO)m_Scenarios->currentData().toInt();
}

QDateTime AuIntroStartScenarios::AsteroidImpact() const
{
	switch(Scenario())
	{
	case ReachForTheStars:
		return m_Reach->AsteroidImpact();
	case CitizenOfTheGalaxy:
	case SavageState:
	case LostAtlantis:
	case SCENARIO_Count:
		break;
	}
	return QDateTime();
}

bool AuIntroStartScenarios::OkToGo() const
{
	switch(Scenario())
	{
	case CitizenOfTheGalaxy:
		return m_Citizen->OkToGo();
	case ReachForTheStars:
		return m_Reach->OkToGo();
	case SavageState:
		return m_Savage->OkToGo();
	case LostAtlantis:
		return m_Lost->OkToGo();
	case SCENARIO_Count:
		break;
	}
	return false;
}

QByteArray AuIntroStartScenarios::EmpireJoinPassCrypt() const
{
	switch(Scenario())
	{
	case CitizenOfTheGalaxy:
		return m_Citizen->EmpireJoinPassCrypt();
	case ReachForTheStars:
	case SavageState:
	case LostAtlantis:
	case SCENARIO_Count:
		break;
	}
	return QByteArray();
}

quint32 AuIntroStartScenarios::EmpireToJoin() const
{
	switch(Scenario())
	{
	case CitizenOfTheGalaxy:
		return m_Citizen->EmpireToJoin();
	case ReachForTheStars:
	case SavageState:
	case LostAtlantis:
	case SCENARIO_Count:
		break;
	}
	return 0;
}

QString AuIntroStartScenarios::NewEmpireName() const
{
	switch(Scenario())
	{
	case CitizenOfTheGalaxy:
	case ReachForTheStars:
	case SavageState:
		break;
	case LostAtlantis:
		return m_Lost->NewEmpireName();
	case SCENARIO_Count:
		break;
	}
	return QString();
}

QImage AuIntroStartScenarios::NewEmpireFlag() const
{
	switch(Scenario())
	{
	case CitizenOfTheGalaxy:
	case ReachForTheStars:
	case SavageState:
		break;
	case LostAtlantis:
		return m_Lost->NewEmpireFlag();
	case SCENARIO_Count:
		break;
	}
	return QImage();
}

quint8 AuIntroStartScenarios::Region() const
{
	switch(Scenario())
	{
	case ReachForTheStars:
		return m_Reach->Region();
	case SavageState:
		return m_Savage->Region();
	case LostAtlantis:
		return m_Lost->Region();
	case CitizenOfTheGalaxy:
	case SCENARIO_Count:
		break;
	}
	return 0;
}

GALAXY AuIntroStartScenarios::Galaxy() const
{
	switch(Scenario())
	{
	case CitizenOfTheGalaxy:
		return m_Citizen->EmpireJoinGalaxy();
	case ReachForTheStars:
	case SavageState:
	case LostAtlantis:
	case SCENARIO_Count:
		break;
	}
	return ShoresOfHazeron;
}

AuId AuIntroStartScenarios::EmpireJoinSector() const
{
	switch(Scenario())
	{
	case CitizenOfTheGalaxy:
		return m_Citizen->EmpireJoinSector();
	case ReachForTheStars:
	case SavageState:
	case LostAtlantis:
	case SCENARIO_Count:
		break;
	}
	return AuId();
}

