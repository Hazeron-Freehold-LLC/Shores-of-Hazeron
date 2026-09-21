//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetsound.cpp,v 1.16 2022/02/19 18:26:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausetsound.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auglobal.h"
#include "aujoystick.h"
#include "aulineedit.h"
#include "auslider.h"
#include "ausound.h"
#include "autoolbutton.h"
#include <QCheckBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
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
			"Restores all sounds to their default volume, audio file, reference distance and rolloff factor. "
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
			double refDist;
			float rolloff;
			bool overridden;
			if(g_Au.SoundOverride(*si, soundO))
			{
				refDist = soundO.ReferenceDistance();
				rolloff = soundO.RolloffFactor();
				overridden = !soundO.FileName().isEmpty();
			}
			else
			{
				refDist = AuSound::SoundReferenceDistance(*si);
				rolloff = AuSound::SoundRolloffFactor(*si);
				overridden = false;
			}

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
						"<b>Cancel</b> the file chooser dialog to restore the original sound file, reference distance and "
						"rolloff factor. "
						"</html>"));
			connect(tbwav, SIGNAL(ClickedValue(int)), SLOT(SetSoundFile(int)));
			m_ChooseSound.insert(*si, tbwav);

			QString srefdist = QString("%1").arg(refDist/FeetPerMeter());	// Edited in meters.
			AuLineEditValue* slref = new AuLineEditValue(srefdist, *si, this);
			m_RefDist.insert(*si, slref);
			connect(slref, SIGNAL(EditingFinished(int)), SLOT(SetSoundRefDistance(int)));
			slref->setToolTip(tr("<html>"
						"<p>"
						"<b>Reference Distance in Meters</b><br>"
						"Reference distance is the distance at which the listener hears the sound at its sampled volume. "
						"When the source is closer than the reference distance, the sound is heard louder than the sample. "
						"When the source is farther than the reference distance, the sound is heard quieter than the sample. "
						"</p>"
						"<p>"
						"Reference distance must not be zero. "
						"</p>"
						"<p>"
						"This formula is used by OpenAL to calculate sound volume, "
						"based on the distance from the source to the listener. "
						"<b>Some sounds are played so volume does not drop off with distance.</b> "
						"<br>"
						"volume = Reference_Distance / (Reference_Distance + Rolloff_Factor * (distance – Reference_Distance))"
						"</p>"
						"</html>"));

			QString srolloff = QString("%1").arg(rolloff);
			AuLineEditValue* slroll = new AuLineEditValue(srolloff, *si, this);
			m_Rolloff.insert(*si, slroll);
			connect(slroll, SIGNAL(EditingFinished(int)), SLOT(SetSoundRolloff(int)));
			slroll->setToolTip(tr("<html>"
						"<p>"
						"<b>Rolloff Factor</b><br>"
						"Rolloff factor causes the volume of the sound to drop off more or less quickly over distance. "
						"It has the effect of making a sound carry farther, for sounds with a lot of energy, such as a gun shot. "
						"</p>"
						"<p>"
						"The smaller the rolloff factor, the farther the sound carries. "
						"A rolloff factor of 0 results in a sound volume of 1, regardless of distance. "
						"</p>"
						"<p>"
						"This formula is used by OpenAL to calculate sound volume, "
						"based on the distance from the source to the listener. "
						"Some sounds are played so volume does not drop off with distance, such as breathing sounds. "
						"<br>"
						"volume = Reference_Distance / (Reference_Distance + Rolloff_Factor * (distance – Reference_Distance))"
						"</p>"
						"</html>"));

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
			lay->addWidget(slref, row,3);
			lay->addWidget(slroll, row,4);
			lay->addWidget(tbplay, row,5);
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
		// Update edit controls.
		QHash<int, AuLineEditValue*>::iterator lvi = m_RefDist.find(sound);
		if(lvi != m_RefDist.end())
		{
			QString srefdist = QString("%1").arg(soundO.ReferenceDistance()/FeetPerMeter());
			(*lvi)->blockSignals(true);
			(*lvi)->setText(srefdist);
			(*lvi)->blockSignals(false);
		}
		lvi = m_Rolloff.find(sound);
		{
			QString srolloff = QString("%1").arg(soundO.RolloffFactor());
			(*lvi)->blockSignals(true);
			(*lvi)->setText(srolloff);
			(*lvi)->blockSignals(false);
		}
		QHash<int, AuToolButtonValue*>::iterator tvi = m_ChooseSound.find(sound);
		if(tvi != m_ChooseSound.end())
			(*tvi)->setIcon(QIcon(":data/OrderMusic.png"));

		QString smsg = tr("<html>The sound file, reference distance and rolloff factor were reset to their default settings. "
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

void AuSetSound::SetSoundRefDistance(int sound)
{
	// Get new value.
	QHash<int, AuLineEditValue*>::const_iterator lei = m_RefDist.find(sound);
	if(lei == m_RefDist.cend())
		return;
	const AuLineEditValue* le = *lei;
	double refDist = le->text().toDouble() * FeetPerMeter();	// Editor shows the value in meters. Convert to feet.
	if(refDist <= 0)
		return;

	// Get current sound override settings, if any.
	AuSoundOverride soundO;
	if(!g_Au.SoundOverride((SOUND_EFFECT)sound, soundO))
		soundO = AuSound::DefaultSoundOverride((SOUND_EFFECT)sound);

	// Build nu sound override settings.
	soundO = AuSoundOverride(soundO.FileName(), refDist, soundO.RolloffFactor());
	g_Au.SetSoundOverride((SOUND_EFFECT)sound, soundO);
	emit SoundOverrideChanged(sound);
}

void AuSetSound::SetSoundRolloff(int sound)
{
	// Get new value.
	QHash<int, AuLineEditValue*>::const_iterator lei = m_Rolloff.find(sound);
	if(lei == m_Rolloff.cend())
		return;
	const AuLineEditValue* le = *lei;
	float rolloff = le->text().toFloat();
	if(rolloff < 0)
		return;

	// Get current sound override settings, if any.
	AuSoundOverride soundO;
	if(!g_Au.SoundOverride((SOUND_EFFECT)sound, soundO))
		soundO = AuSound::DefaultSoundOverride((SOUND_EFFECT)sound);

	// Build nu sound override settings.
	soundO = AuSoundOverride(soundO.FileName(), soundO.ReferenceDistance(), rolloff);
	g_Au.SetSoundOverride((SOUND_EFFECT)sound, soundO);
	emit SoundOverrideChanged(sound);
}

void AuSetSound::RestoreDefaultSounds()
{
	QString smsg = tr("<html>All sounds will be restored to their default volume, audio file, reference distance and "
			"rolloff factor. "
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
		QHash<int, AuLineEditValue*>::iterator lvi = m_RefDist.find(sound);
		if(lvi != m_RefDist.end())
		{
			QString srefdist = QString("%1").arg(soundO.ReferenceDistance()/FeetPerMeter());	// Show dist in meters.
			(*lvi)->blockSignals(true);
			(*lvi)->setText(srefdist);
			(*lvi)->blockSignals(false);
		}
		lvi = m_Rolloff.find(sound);
		{
			QString srolloff = QString("%1").arg(soundO.RolloffFactor());
			(*lvi)->blockSignals(true);
			(*lvi)->setText(srolloff);
			(*lvi)->blockSignals(false);
		}
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

