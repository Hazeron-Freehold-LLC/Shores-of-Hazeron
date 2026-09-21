//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auservermonapp.cpp,v 1.17 2022/03/02 23:32:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auservermonapp.h"
//
#include "auglmodel.h"
#include "ausound.h"
#include "austubglobal.h"
#include "austylesheet.h"
#include "dgdata.h"

AuServerMonApp::AuServerMonApp(int& argc, char** argv) : QApplication(argc, argv),
	m_Settings("Hazeron Freehold", "Hazeron Server Monitor", this)
{
	AuStyleSheet::SetStyle(false);

	m_Data = new DgData;

	gp_AuGlobal = new AuStubGlobal("Hazeron Server Monitor", "Hazeron Freehold", AuGlobal::WebCGI, m_Data);

	m_ModelCache = new AuGLModelCache;
	g_Au.SetModelCache(m_ModelCache);

	m_AL = 0;
	m_AudioSource = 0;
	for(int cs=0; cs<CLIENT_SOUNDS; cs++)
	{
		m_Audio[cs] = 0;
		m_Sound[cs] = SOUND_None;
	}

	InitializeSound();
}

AuServerMonApp::~AuServerMonApp()
{
	// Sound
	if(m_AL)
		delete m_AL;
	if(m_ModelCache)
		delete m_ModelCache;
	if(m_Data)
		delete m_Data;
	if(gp_AuGlobal)
		delete gp_AuGlobal;
}

QString AuServerMonApp::Name() const
{
	return m_Settings.applicationName();
}

void AuServerMonApp::InitializeSound()
{
	Q_ASSERT(!m_AL);	// Don't call this twice.
	m_AudioSource = 0;
	for(int cs=0; cs<CLIENT_SOUNDS; cs++)
	{
		m_Audio[cs] = 0;
		m_Sound[cs] = SOUND_None;
	}

	m_AL = new AuSound;
	m_AL->InitializeAudio();
}

void AuServerMonApp::TerminateSound()
{
	if(!m_AL)
		return;
	//qDebug() << "AuServerMonApp::TerminateSound destroying OpenAL context";
	delete m_AL;
	m_AL = 0;
}

void AuServerMonApp::PlayClientSound(int sound, float gain, float pitch)
{
	if(!m_AL)
		return;
	//gain *= g_Au.SoundEffectsGain();
	SOUND_EFFECT cur;
	for(int i = 0; i < CLIENT_SOUNDS; i++)
	{
		if(!m_AL->IsSoundPlaying(m_Audio[i]))
		{
			cur = (SOUND_EFFECT)m_Sound[i];
			m_AL->Play(m_Audio[i], cur, (SOUND_EFFECT)sound, 0,0,0, gain, pitch, 0, 1,0, 0);
			m_Sound[i] = cur;
			break;
		}
	}
}

void AuServerMonApp::StopClientSound(int sound)
{
	if(!m_AL)
		return;
	for(int i = 0; i < CLIENT_SOUNDS; i++)
		if(m_Sound[i] == sound)
		{
			m_AL->StopSound(m_Audio[i]);
			m_Sound[i] = SOUND_None;
		}
}

void AuServerMonApp::StopClientSounds()
{
	if(!m_AL)
		return;
	for(int i = 0; i < CLIENT_SOUNDS; i++)
		if(m_Sound[i] != SOUND_None)
		{
			m_AL->StopSound(m_Audio[i]);
			m_Sound[i] = SOUND_None;
		}
}

bool AuServerMonApp::IsClientSoundPlaying(int sound) const
{
	if(!m_AL)
		return false;
	for(int i = 0; i < CLIENT_SOUNDS; i++)
		if(m_Sound[i] == sound && m_AL->IsSoundPlaying(m_Audio[i]))
			return true;
	return false;
}

