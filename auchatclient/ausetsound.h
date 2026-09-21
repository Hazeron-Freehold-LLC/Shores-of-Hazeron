//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetsound.h,v 1.4 2022/02/19 17:47:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSetSound_H
#define AuSetSound_H

#include <QWidget>

class AuLineEditValue;
class AuSliderValue;
class AuToolButtonValue;
class QSlider;

class AuSetSound : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	QHash<int, AuToolButtonValue*>		m_ChooseSound;
	QSlider*							m_SoundVolume;			// Overall volume slider.
	QHash<int, AuSliderValue*>			m_Volume;

public:
	AuSetSound(QWidget *parent=0);
	~AuSetSound();

protected slots:
	void Play(int value);
	void RestoreDefaultSounds();
	void SetSoundFile(int sound);
	void SetSoundVolume(int value, int sound);
	void SetSoundEffectsVolume(int value);

signals:
	void SoundGainChanged(int sound);
	void SoundOverrideChanged(int sound);
};

#endif // AuSetSound_H
