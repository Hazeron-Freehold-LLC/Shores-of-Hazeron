//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetsound.cpp,v 1.11 2022/02/19 17:48:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausetsound.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auglobal.h"
#include "aujoystick.h"
#include "aupushbutton.h"
#include "auslider.h"
#include "ausound.h"
#include "autoolbutton.h"
#include <QCheckBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QScrollArea>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>

AuSetSound::AuSetSound(QWidget *parent) : QWidget(parent)
{
	// Sound Effects
	QLabel* icsoundvol = new QLabel(this);
	icsoundvol->setPixmap(QPixmap(":data/Speaker.png"));

	QLabel* lbsoundvol = new QLabel(tr("Sound Effects Volume"), this);

	int soundVol = (int)(g_Au.SoundEffectsGain() * 100);	// In the range 0 to 100.

	m_SoundVolume = new QSlider(Qt::Horizontal, this);
	m_SoundVolume->setRange(0, 100);
	m_SoundVolume->setSingleStep(1);
	m_SoundVolume->setPageStep(10);
	m_SoundVolume->setValue(soundVol);
	connect(m_SoundVolume, SIGNAL(valueChanged(int)), SLOT(SetSoundEffectsVolume(int)));
	m_SoundVolume->setToolTip(tr("<html>"
			"Adjusts the overall volume of all program sounds."
			"</html>"));

	QPushButton* pbreset = new QPushButton(tr("Restore Default Sound Settings..."), this);
	pbreset->setToolTip(tr("<html>"
			"Restores all sounds to their default volume and audio file. "
			"Does not change the overall volume control. "
			"</html>"));
	connect(pbreset, SIGNAL(clicked()), SLOT(RestoreDefaultSounds()));

	// Scroll area and controls for sound effects.
	QScrollArea* scroll = new QScrollArea;
	{
		// Sort alphabetically by name.
		QMap<QString, SOUND_EFFECT> sortedSounds;
		for(int s=SOUND_None+1; s<SOUND_Count; s++)
			sortedSounds.insert(AuSound::SoundName((SOUND_EFFECT)s).toUpper(), (SOUND_EFFECT)s);

		QGridLayout* lay = new QGridLayout;
		int row=0;
		for(QMap<QString, SOUND_EFFECT>::const_iterator si = sortedSounds.cbegin(); si != sortedSounds.cend(); si++)
		{
			int soundVol = (int)(g_Au.SoundGain(*si) * 100);	// In the range 0 to 100.
			AuSoundOverride soundO;
			bool overridden;
			if(g_Au.SoundOverride(*si, soundO))
				overridden = !soundO.FileName().isEmpty();
			else
				overridden = false;

			QLabel* lb = new QLabel(AuSound::SoundName(*si), this);

			AuSliderValue* slvol = new AuSliderValue(Qt::Horizontal, *si, this);
			slvol->setRange(0, 200);
			slvol->setSingleStep(1);
			slvol->setPageStep(10);
			slvol->setValue(soundVol);
			m_Volume.insert(*si, slvol);
			connect(slvol, SIGNAL(ValueChanged(int, int)), SLOT(SetSoundVolume(int, int)));
			slvol->setToolTip(tr("<html>"
					"Adjusts the volume of the sound. "
					"This is further modulated by the <i>Sound Effects Volume</i> setting. "
					"</html>"));

			AuToolButtonValue* tbwav = new AuToolButtonValue(*si, this);
			if(overridden)
				tbwav->setIcon(QIcon(":data/MusicCustom.png"));
			else
				tbwav->setIcon(QIcon(":data/OrderMusic.png"));
			tbwav->setAutoRaise(true);
			tbwav->setToolTip(tr("<html><b>Choose Sound File...</b><br>"
						"Selects a file to use for the sound.<br>"
						"<b>Cancel</b> the file chooser dialog to restore the original sound file. "
						"</html>"));
			connect(tbwav, SIGNAL(ClickedValue(int)), SLOT(SetSoundFile(int)));
			m_ChooseSound.insert(*si, tbwav);

			AuToolButtonValue* tbplay = new AuToolButtonValue(*si, this);
			tbplay->setIcon(QIcon(":data/Speaker.png"));
			tbplay->setAutoRaise(true);
			tbplay->setToolTip(tr("<html><b>Play|Stop Sound</b><br>"
						"Plays the sound if it is not being played. "
						"Stops playing the sound if it is being played. "
						"</html>"));
			connect(tbplay, SIGNAL(ClickedValue(int)), SLOT(Play(int)));

			lay->addWidget(lb, row,0);
			lay->addWidget(slvol, row,1);
			lay->addWidget(tbwav, row,2);
			lay->addWidget(tbplay, row,3);
			row++;
		}
		lay->setColumnStretch(1, 100);
		lay->setMargin(2);
		lay->setSpacing(2);

		QWidget* wf = new QWidget();
		wf->setLayout(lay);

		scroll->setWidget(wf);
		scroll->setWidgetResizable(true);
	}

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(icsoundvol, row,0);
	lay->addWidget(lbsoundvol, row,1);
	lay->addWidget(m_SoundVolume, row,2, 1,2);
	row++;
	lay->addWidget(pbreset, row,3);
	row++;
	lay->addWidget(scroll, row,1, 1,3);
	lay->setColumnStretch(2, 100);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuSetSound::~AuSetSound()
{
}

void AuSetSound::SetSoundEffectsVolume(int value)
{
	float gain = (float)value/100.f;
	g_Au.SetSoundEffectsGain(gain);
}

void AuSetSound::SetSoundVolume(int value, int sound)
{
	float gain = (float)value/100.f;
	g_Au.SetSoundGain((SOUND_EFFECT)sound, gain);
	emit SoundGainChanged(sound);
}

void AuSetSound::SetSoundFile(int sound)
{
	// Get new value.
	QString lastFile = g_Au.Settings().readEntry(QString("/Sound/Sound%1/File").arg(sound), "");
	//qDebug() << "AuSetSound::SetSoundFile" << sound << "lastFile" << lastFile;
	QString file = QFileDialog::getOpenFileName(this, tr("Select Sound File"), lastFile, tr("WAV Files (*.wav)"), 0,
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);

	// Get current sound override settings, if any.
	AuSoundOverride soundO;
	if(file.isEmpty() || !g_Au.SoundOverride((SOUND_EFFECT)sound, soundO))
		soundO = AuSound::DefaultSoundOverride((SOUND_EFFECT)sound);

	// Build nu sound override settings.
	soundO = AuSoundOverride(file, soundO.ReferenceDistance(), soundO.RolloffFactor());
	g_Au.SetSoundOverride((SOUND_EFFECT)sound, soundO);
	emit SoundOverrideChanged(sound);

	if(file.isEmpty())
	{
		// Default settings were restored.
		QHash<int, AuToolButtonValue*>::iterator tvi = m_ChooseSound.find(sound);
		if(tvi != m_ChooseSound.end())
			(*tvi)->setIcon(QIcon(":data/OrderMusic.png"));

		QString smsg = tr("<html>The sound file was reset to its default setting. "
				"</html>");
		AuFrame* frame = auApp->Frame();
		QMessageBox mb(g_Au.ApplicationName(), smsg,
				QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, frame);
		frame->ExecMessageBox(mb);
	}
	else
	{
		QHash<int, AuToolButtonValue*>::iterator tvi = m_ChooseSound.find(sound);
		if(tvi != m_ChooseSound.end())
			(*tvi)->setIcon(QIcon(":data/MusicCustom.png"));
	}
}

void AuSetSound::RestoreDefaultSounds()
{
	QString smsg = tr("<html>All sounds will be restored to their default volume and audio file. "
			"</html>");
	QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Information, QMessageBox::Ok, QMessageBox::Cancel,
			QMessageBox::NoButton, auApp->Frame());
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	for(int s=SOUND_None+1; s<SOUND_Count; s++)
	{
		// Sound overrides.
		SOUND_EFFECT sound = (SOUND_EFFECT)s;
		AuSoundOverride soundO(AuSound::DefaultSoundOverride(sound));
		g_Au.SetSoundOverride(sound, soundO);
		emit SoundOverrideChanged(sound);

		// Update edit controls.
		QHash<int, AuToolButtonValue*>::iterator tvi = m_ChooseSound.find(sound);
		if(tvi != m_ChooseSound.end())
			(*tvi)->setIcon(QIcon(":data/OrderMusic.png"));

		// Sound volume.
		QHash<int, AuSliderValue*>::iterator svi = m_Volume.find(sound);
		if(svi != m_Volume.end())
		{
			float gain = AuSound::DefaultSoundGain(sound);
			int soundVol = (int)(gain * 100);	// In the range 0 to 100.
			(*svi)->setValue(soundVol);			// Let the signal update the settings.
		}
	}
}

void AuSetSound::Play(int sound)
{
	if(auApp->IsClientSoundPlaying((SOUND_EFFECT)sound))
		auApp->StopClientSound((SOUND_EFFECT)sound);
	else
		auApp->PlayClientSound((SOUND_EFFECT)sound);
}

