//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausound.h,v 1.9 2022/02/19 18:31:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSound_H
#define AuSound_H

#include <QtGlobal>	// For Q_OS_ definitions.
#if defined(__APPLE__)	//defined(Q_OS_APPLE) This fails.
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#elif defined(Q_OS_WIN)
#include <al.h>
#include <alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include "audef.h"
#include "auglobal.h"

using namespace std;
#include <unordered_map>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// NOTE: There is an order dependency between this enum and the ON_GROUND enum in auqueryenvironment.h.
enum SOUND_EFFECT
{
	SOUND_None,
	SOUND_BreathingInMask,
	SOUND_BreathingScuba,
	SOUND_Dig,
	SOUND_DirtBikeIdle,
	SOUND_DirtBike2000,
	SOUND_DoorCargoClose,
	SOUND_DoorCargoOpen,
	SOUND_DoorExtClose,
	SOUND_DoorExtOpen,
	SOUND_DoorHatchClose,
	SOUND_DoorHatchOpen,
	SOUND_DoorIntClose,
	SOUND_DoorIntOpen,
	SOUND_DragForest,			// ON_GROUND order in daunit.h
	SOUND_DragHardFloor,		// ON_GROUND order in daunit.h
	SOUND_DragMetal,			// ON_GROUND order in daunit.h
	SOUND_DragPavement,			// ON_GROUND order in daunit.h
	SOUND_DragRock,				// ON_GROUND order in daunit.h
	SOUND_DragSand,				// ON_GROUND order in daunit.h
	SOUND_DragSavanna,			// ON_GROUND order in daunit.h
	SOUND_DragSnow,				// ON_GROUND order in daunit.h
	SOUND_DragWater,			// ON_GROUND order in daunit.h
	SOUND_DragWoodFloor,		// ON_GROUND order in daunit.h
	SOUND_EatingMeat,
	SOUND_EatingPlants,
	SOUND_ExplodeSpacecraft,
	SOUND_ExplodeVehicle,
	SOUND_FireAK47,
	SOUND_FireCannon,
	SOUND_FireLaserBeam,
	SOUND_FireLaserCannon,
	SOUND_FireLaserPulse,
	SOUND_FirePistol,
	SOUND_FireRifle,
	SOUND_FireShotgun,
	SOUND_FootstepsForest,		// ON_GROUND order in daunit.h
	SOUND_FootstepsHardFloor,	// ON_GROUND order in daunit.h
	SOUND_FootstepsMetal,		// ON_GROUND order in daunit.h
	SOUND_FootstepsPavement,	// ON_GROUND order in daunit.h
	SOUND_FootstepsRock,		// ON_GROUND order in daunit.h
	SOUND_FootstepsSand,		// ON_GROUND order in daunit.h
	SOUND_FootstepsSavanna,		// ON_GROUND order in daunit.h
	SOUND_FootstepsSnow,		// ON_GROUND order in daunit.h
	SOUND_FootstepsWater,		// ON_GROUND order in daunit.h
	SOUND_FootstepsWoodFloor,	// ON_GROUND order in daunit.h
	SOUND_GasLamp,
	SOUND_GravityDriveInt,
	SOUND_GravityDriveExt,
	SOUND_HitAcid,				// NOTE: The groupings of three for the Hit
	SOUND_HitAcidCritical,		//   sounds must be maintained. This order
	SOUND_HitAcidNoDamage,		//   is expected when calculating variations
	SOUND_HitBludgeon,			//   off the primary sound. i.e. Critical is
	SOUND_HitBludgeonCritical,	//   always +1 and NoDamage is always +2.
	SOUND_HitBludgeonNoDamage,
	SOUND_HitCannon,
	SOUND_HitCold,
	SOUND_HitColdCritical,
	SOUND_HitColdNoDamage,
	SOUND_HitDisease,
	SOUND_HitDiseaseCritical,
	SOUND_HitDiseaseNoDamage,
	SOUND_HitElectric,
	SOUND_HitElectricCritical,
	SOUND_HitElectricNoDamage,
	SOUND_HitFire,
	SOUND_HitFireCritical,
	SOUND_HitFireNoDamage,
	SOUND_HitGasping,
	SOUND_HitGaspingCritical,
	SOUND_HitGaspingNoDamage,
	SOUND_HitParalysis,
	SOUND_HitParalysisCritical,
	SOUND_HitParalysisNoDamage,
	SOUND_HitPierce,
	SOUND_HitPierceCritical,
	SOUND_HitPierceNoDamage,
	SOUND_HitPoison,
	SOUND_HitPoisonCritical,
	SOUND_HitPoisonNoDamage,
	SOUND_HitRadiation,
	SOUND_HitRadiationCritical,
	SOUND_HitRadiationNoDamage,
	SOUND_HitLight,
	SOUND_HitLightCritical,
	SOUND_HitLightNoDamage,
	SOUND_HitSonic, 
	SOUND_HitSonicCritical, 
	SOUND_HitSonicNoDamage, 
	SOUND_HitSeed,
	SOUND_HitSeedCritical,
	SOUND_HitSeedNoDamage,
	SOUND_HitSlash,
	SOUND_HitSlashCritical,
	SOUND_HitSlashNoDamage,
	SOUND_KnifeSwish,
	SOUND_Submarine,
	SOUND_SUVIdle,
	SOUND_PowerPlant,
	SOUND_SailBoat,
	SOUND_Telemetry,
	SOUND_Torch,
	SOUND_Wind,
	SOUND_WingFlap,
	SOUND_ToneThought,
	SOUND_ToneHelp,
	SOUND_Rain,
	SOUND_ToneNarrator,
	SOUND_ToneChat,
	SOUND_FireMachineGun,
	SOUND_Transporter,
	SOUND_SnipWire,
	SOUND_Whizzer,
	SOUND_BombRelease,
	SOUND_RingworldStargate,
	SOUND_Lava,
	SOUND_ServoStopped,
	SOUND_ServoWeak,
	SOUND_CracklingFire,
	SOUND_NativeDrums,
	SOUND_ToneSOS,
	SOUND_Harvest,
	SOUND_TractorBeam,
	SOUND_AlertRed,
	SOUND_AlertYellow,
	SOUND_AlertGreen,
	SOUND_BuddyOnLine,
	SOUND_ChannelOpened,
	SOUND_WorldClaimed,
	SOUND_HitExplosion,
	SOUND_Bleep1,
	SOUND_Bleep2,
	SOUND_Bleep3,
	SOUND_Bleep4,
	SOUND_Bleep5,
	SOUND_Bleep6,
	SOUND_Bleep7,
	SOUND_Bleep8,
	SOUND_Helicopter,
	MUSIC_ShroudOfGlory,
	MUSIC_ColderThanIce,
	SOUND_Count			//NOTE: Last item in enum
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// From OpenAL documentation on the default Inverse Distance Rolloff Clamped (AL_INVERSE_DISTANCE_CLAMPED) model.
//   distance = max(distance, AL_REFERENCE_DISTANCE)
//   distance = min(distance, AL_MAX_DISTANCE)
//   gain = AL_REFERENCE_DISTANCE / (AL_REFERENCE_DISTANCE + AL_ROLLOFF_FACTOR * (distance – AL_REFERENCE_DISTANCE))
class AuGLSoundInfo
{
private:
	QString								m_FileName;				// Name of .wav file contained in sfx.har.
	float								m_Gain;					// The sound gain/volume. 0 to 1
	QString								m_Name;					// Name of sound for UI.
	AuSoundOverride						m_Override;
	double								m_ReferenceDistance;	// The distance the sound effect volume is 1, in feet.
	float								m_RolloffFactor;		// A lower number makes the sound carry farther, drop off slower.
	SOUND_EFFECT						m_Sound;				// The associated enum.

public:
	inline AuGLSoundInfo(SOUND_EFFECT sound, const QString& name, const char* fileName, double referenceDistance=FeetPerMeter(),
			float rolloffFactor=1.f) : m_Override("", referenceDistance, rolloffFactor)
	{
		m_FileName = fileName;
		m_Gain = 1;
		m_Name = name;
		m_ReferenceDistance = referenceDistance;
		m_RolloffFactor = rolloffFactor;
		m_Sound = sound;
	}

	inline AuGLSoundInfo(const AuGLSoundInfo& c) : m_Override(c.m_Override)
	{
		m_FileName = c.m_FileName;
		m_Gain = c.m_Gain;
		m_Name = c.m_Name;
		m_ReferenceDistance = c.m_ReferenceDistance;
		m_RolloffFactor = c.m_RolloffFactor;
		m_Sound = c.m_Sound;
	}

public:
	// Returns a sound override object that matches the defaults for this sound.
	inline AuSoundOverride DefaultOverride() const
	{
		return AuSoundOverride("", m_ReferenceDistance, m_RolloffFactor);
	}

	inline double DefaultReferenceDistance() const
	{
		return m_ReferenceDistance;
	}

	inline float DefaultRolloffFactor() const
	{
		return m_RolloffFactor;
	}

	// This is the file name in the sfx.har file.
	inline const QString& FileName() const
	{
		return m_FileName;
	}

	inline float Gain() const
	{
		return m_Gain;
	}

	inline const QString& Name() const
	{
		return m_Name;
	}

	inline const QString& Override() const
	{
		return m_Override.FileName();
	}

	inline double ReferenceDistance() const
	{
		return m_Override.ReferenceDistance();
	}

	inline float RolloffFactor() const
	{
		return m_Override.RolloffFactor();
	}

	inline void SetGain(float gain)
	{
		m_Gain = gain;
	}

	inline void SetOverride(const AuSoundOverride& soundO)
	{
		m_Override = soundO;
	}

	inline SOUND_EFFECT Sound() const
	{
		return m_Sound;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuSound
{
private:
	ALCcontext*							m_ALContext;			// OpenAL device context.
	ALuint								m_AudioBufferCurrent[SOUND_Count];
	ALuint								m_AudioBufferDefault[SOUND_Count];
	QString								m_ChatTone;				// File name of chat tone .wav file override. Default is blank.
	QString								m_HelpTone;				// File name of help tone .wav file override. Default is blank.
	QString								m_NarratorTone;			// File name of narrator tone .wav file override. Default is blank.
	QString								m_SOSTone;				// File name of SOS tone .wav file override. Default is blank.
	mutable int							m_SoundErrorsRemaining; // Number of sound load errors allowed to be displayed.

	QString								m_ThoughtTone;			// File name of thought tone .wav file override. Default is blank.

public:
	AuSound();
	~AuSound();

public:
	// Returns the id of an audio buffer for a sound effect.
	// The buffer is loaded if this is the first reference to it.
	ALuint AudioBuffer(SOUND_EFFECT sound);

	// Restores a sound to its default settings.
	void ClearSoundOverride(SOUND_EFFECT sound);

	// OpenAL audio buffers
	// Creates an audio buffer given a byte array of sound file data.
	inline ALuint CreateAudioBuffer(const QByteArray& soundData) const
	{
		// The buffer has not been loaded yet.
		return ALCreateBufferFromFileImage(soundData);
	}

	static float DefaultSoundGain(SOUND_EFFECT sound);

	static AuSoundOverride DefaultSoundOverride(SOUND_EFFECT sound);

	inline static double DefaultSoundReferenceDistance(SOUND_EFFECT sound)
	{
		return SoundInfo(sound).DefaultReferenceDistance();
	}

	inline static float DefaultSoundRolloffFactor(SOUND_EFFECT sound)
	{
		return SoundInfo(sound).DefaultRolloffFactor();
	}

	inline void DeleteSound(ALuint audioBuffer)
	{
		if(!m_ALContext)
			return;
		alDeleteBuffers(1, &audioBuffer);
	}

	void InitializeAudio();

	inline bool IsSoundPlaying(unsigned int audioSource) const
	{
		if(!m_ALContext)
			return false;
		if(audioSource)
		{
			// An audio source id was provided.
			ALenum state;
			alGetSourcei(audioSource, AL_SOURCE_STATE, &state);
			if(state == AL_PLAYING)
				return true;	// A sound is playing in the audio source.
		}
		return false;	// Not playing or zero audio source id.
	}

	// audioSource is a sound source. May be 0. Allocated when needed. Freed when called with SOUND_None.
	// curSound is the current sound playing in the sound source, if any. It will be changed if the sound changes.
	// sound is the sound to play
	// position
	// gain
	// pitch
	// loopSound is true to loop the sound
	// secOffset
	inline void Play(unsigned int& audioSource, SOUND_EFFECT& curSound, SOUND_EFFECT sound, float posX, float posY, float posZ,
			float gain, float pitch, bool loopSound)
	{
		Play(audioSource, curSound, sound, posX, posY, posZ, gain, pitch, loopSound,
				(float)SoundReferenceDistance(sound), SoundRolloffFactor(sound), 0);
	}

	// 'audioSource' is a return parameter. It will be allocated if 0.
	// From OpenAL documentation on the default Inverse Distance Rolloff Clamped (AL_INVERSE_DISTANCE_CLAMPED) model.
	//   distance = max(distance, AL_REFERENCE_DISTANCE)
	//   distance = min(distance, AL_MAX_DISTANCE)
	//   gain = AL_REFERENCE_DISTANCE / (AL_REFERENCE_DISTANCE + AL_ROLLOFF_FACTOR * (distance – AL_REFERENCE_DISTANCE))
	void Play(unsigned int& audioSource, SOUND_EFFECT& curSound, SOUND_EFFECT sound, float posX, float posY, float posZ,
			float gain, float pitch, bool loopSound, float referenceDistance, float rolloffFactor, float secOffset);

	// 'audioSource' is a return parameter. It will be allocated if 0.
	void Play(unsigned int& audioSource, ALuint audioBuffer, float posX, float posY, float posZ, float gain, float pitch,
			float referenceDistance, float rolloffFactor);

	// This is a runtime override. The setting is not stored anywhere by this code.
	void SetSoundGain(SOUND_EFFECT sound, float gain);

	// This is a runtime override, for restoring and changing current settings. The setting is not stored anywhere by this code.
	void SetSoundOverride(SOUND_EFFECT sound, const AuSoundOverride& soundOver);

	inline static float SoundGain(SOUND_EFFECT sound)
	{
		return SoundInfo(sound).Gain();
	}

	static const AuGLSoundInfo& SoundInfo(SOUND_EFFECT sound);

	inline static const QString& SoundName(SOUND_EFFECT sound)
	{
		return SoundInfo(sound).Name();
	}

	inline static const QString& SoundOverride(SOUND_EFFECT sound)
	{
		return SoundInfo(sound).Override();
	}

	inline static double SoundReferenceDistance(SOUND_EFFECT sound)
	{
		return SoundInfo(sound).ReferenceDistance();
	}

	inline static float SoundRolloffFactor(SOUND_EFFECT sound)
	{
		return SoundInfo(sound).RolloffFactor();
	}

	inline void StopSound(unsigned int& audioSource)
	{
		if(!m_ALContext)
			return;
		if(audioSource)
		{
			// OpenAL doc says its ok to delete a source that is playing. In
			// practice, this does not always work unless it is stopped.
			alSourceStop(audioSource);
			alSourcei(audioSource, AL_BUFFER, 0);
			alDeleteSources(1, &audioSource);
			audioSource = 0;
		}
	}

	// Destroys the sound context.
	void TerminateAudio();

private:
	ALuint ALCreateBufferFromFileImage(const QByteArray& wavData) const;

	ALuint LoadAudioBuffer(const QString& fileName) const;
};

#endif // AuSound_H

