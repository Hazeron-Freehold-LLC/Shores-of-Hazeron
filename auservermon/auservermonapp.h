//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auservermonapp.h,v 1.11 2022/03/10 22:37:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuServerMonApp_H
#define AuServerMonApp_H

#include <QApplication>
#include <QSettings>

class AuGLModelCache;
class AuServerMonFrame;
class AuSound;
class DgData;

#define CLIENT_SOUNDS 4					// Number of simultaneous client sounds that an play, i.e. not sounds from atoms or effects.

class AuServerMonApp : public QApplication
{
	Q_OBJECT;
private:
	DgData*								m_Data;
	AuGLModelCache*						m_ModelCache;
	QSettings							m_Settings;
	AuServerMonFrame*					m_MainWindow;

	// OpenAL
	AuSound*							m_AL;					// OpenAL sound functions.
	unsigned int						m_Audio[CLIENT_SOUNDS];	// audio sources for simultaneous client sounds
	unsigned int						m_AudioSource;			// audio source for queued sounds
	int									m_Sound[CLIENT_SOUNDS];	// current SOUND_EFFECT for simultaneous client sounds

public:
	AuServerMonApp(int& argc, char** argv);
	virtual ~AuServerMonApp();

	inline QSettings& Settings()
	{
		return m_Settings;
	}

	QString Name() const;

public:	// Sound
	// Creates the AuSound object. This should be done after global settings exist so it can't be done in my constructor.
	void InitializeSound();

	// 'sound' is a SOUND_EFFECT
	bool IsClientSoundPlaying(int sound) const;

	inline AuServerMonFrame* MainWindow() const
	{
		return m_MainWindow;
	}

	// 'sound' is a SOUND_EFFECT
	// 'gain' is further modulated by g_Au.SoundEffectsGain and sound effect specific gain setting.
	void PlayClientSound(int sound, float gain=1.f, float pitch=1.f);

	inline void SetMainWindow(AuServerMonFrame* w)
	{
		m_MainWindow = w;
	}

	inline AuSound* Sound()
	{
		return m_AL;
	}

	// 'sound' is a SOUND_EFFECT
	void StopClientSound(int sound);

	// Stops all client sounds.
	void StopClientSounds();

	// Destroys the OpenAL context. Deletes the sound object. No more sounds can be made after this call, without InitializeSound.
	void TerminateSound();
};

#define auServerMonApp	((AuServerMonApp*)qApp)

#endif // AuServerMonApp_H
