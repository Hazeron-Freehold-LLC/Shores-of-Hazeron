//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausound.cpp,v 1.23 2022/02/19 18:33:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausound.h"
//
#include "auglobal.h"
#include "hashpack.h"
#include <QDebug>
#include <QFile>

AuGLSoundInfo s_Sounds[SOUND_Count] =
{
	AuGLSoundInfo(SOUND_None, "", ""),
	AuGLSoundInfo(SOUND_BreathingInMask, QObject::tr("Creature Breathing in Helmet"), "BreathingInMask.wav"),
	AuGLSoundInfo(SOUND_BreathingScuba, QObject::tr("Creature Breathing in SCUBA Mask"), "BreathingScuba.wav"),
	AuGLSoundInfo(SOUND_Dig, QObject::tr("Creature Digging"), "Dig.wav"),
	AuGLSoundInfo(SOUND_DirtBikeIdle, QObject::tr("Vehicle, Motor Bike Idle"), "DirtBikeIdle.wav", 1.5*FeetPerMeter()),
	AuGLSoundInfo(SOUND_DirtBike2000, QObject::tr("Vehicle, Motor Bike"), "DirtBikeRPM2000.wav", 1.5*FeetPerMeter()),
	AuGLSoundInfo(SOUND_DoorCargoClose, QObject::tr("Door, Cargo Close"), "DoorCargoOpen.wav"),
	AuGLSoundInfo(SOUND_DoorCargoOpen, QObject::tr("Door, Cargo Open"), "DoorCargoClose.wav"),
	AuGLSoundInfo(SOUND_DoorExtClose, QObject::tr("Door, Exterior Close"), "DoorExtClose.wav"),
	AuGLSoundInfo(SOUND_DoorExtOpen, QObject::tr("Door, Exterior Open"), "DoorExtOpen.wav"),
	AuGLSoundInfo(SOUND_DoorHatchClose, QObject::tr("Door, Lift/Hatch Close"), "DoorHatchClose.wav"),
	AuGLSoundInfo(SOUND_DoorHatchOpen, QObject::tr("Door, Lift/Hatch Open"), "DoorHatchOpen.wav"),
	AuGLSoundInfo(SOUND_DoorIntClose, QObject::tr("Door, Interior Close"), "DoorIntClose.wav"),
	AuGLSoundInfo(SOUND_DoorIntOpen, QObject::tr("Door, Interior Open"), "DoorIntOpen.wav"),
	AuGLSoundInfo(SOUND_DragForest, QObject::tr("Creature Dragging, Forest"), "DragForest.wav"),
	AuGLSoundInfo(SOUND_DragHardFloor, QObject::tr("Creature Dragging, Hard Floor"), "DragHardFloor.wav"),
	AuGLSoundInfo(SOUND_DragMetal, QObject::tr("Creature Dragging, Metal"), "DragMetal.wav"),
	AuGLSoundInfo(SOUND_DragPavement, QObject::tr("Creature Dragging, Pavement"), "DragPavement.wav"),
	AuGLSoundInfo(SOUND_DragRock, QObject::tr("Creature Dragging, Rock"), "DragRock.wav"),
	AuGLSoundInfo(SOUND_DragSand, QObject::tr("Creature Dragging, Sand"), "DragSand.wav"),
	AuGLSoundInfo(SOUND_DragSavanna, QObject::tr("Creature Dragging, Savanna"), "DragSavanna.wav"),
	AuGLSoundInfo(SOUND_DragSnow, QObject::tr("Creature Dragging, Snow"), "DragSnow.wav"),
	AuGLSoundInfo(SOUND_DragWater, QObject::tr("Creature Dragging, Water"), "DragWater.wav"),
	AuGLSoundInfo(SOUND_DragWoodFloor, QObject::tr("Creature Dragging, Wood Floor"), "DragWoodFloor.wav"),
	AuGLSoundInfo(SOUND_EatingMeat, QObject::tr("Creature Eating Meat"), "EatingMeat.wav"),
	AuGLSoundInfo(SOUND_EatingPlants, QObject::tr("Creature Eating Plants"), "EatingPlants.wav"),
	AuGLSoundInfo(SOUND_ExplodeSpacecraft, QObject::tr("Spacecraft Destruction"), "ExplodeSpacecraft.wav", 3*FeetPerMeter()),
	AuGLSoundInfo(SOUND_ExplodeVehicle, QObject::tr("Vehicle Destruction"), "ExplodeVehicle.wav", 3*FeetPerMeter()),
	AuGLSoundInfo(SOUND_FireAK47, QObject::tr("Fire Кала́шников"), "FireAK47.wav", 3*FeetPerMeter()),
	AuGLSoundInfo(SOUND_FireCannon, QObject::tr("Fire Cannon"), "FireCannon.wav", 3*FeetPerMeter()),
	AuGLSoundInfo(SOUND_FireLaserBeam, QObject::tr("Fire Energy Weapon Bay|Turret"), "LaserCannon.wav", 3*FeetPerMeter()),
	AuGLSoundInfo(SOUND_FireLaserCannon, QObject::tr("Fire Laser Pistol|Rifle Mode 1"), "LaserBeam.wav", 3*FeetPerMeter()),
	AuGLSoundInfo(SOUND_FireLaserPulse, QObject::tr("Fire Laser Pistol|Rifle Mode 2"), "LaserPulse.wav", 3*FeetPerMeter()),
	AuGLSoundInfo(SOUND_FirePistol, QObject::tr("Fire Pistol"), "FireHandgun.wav", 3*FeetPerMeter()),
	AuGLSoundInfo(SOUND_FireRifle, QObject::tr("Fire Rifle"), "FireRifle.wav", 3*FeetPerMeter()),
	AuGLSoundInfo(SOUND_FireShotgun, QObject::tr("Fire Shotgun"), "FireShotgun.wav", 3*FeetPerMeter()),
	AuGLSoundInfo(SOUND_FootstepsForest, QObject::tr("Creature Footsteps, Forest"), "FootstepsForest.wav"),
	AuGLSoundInfo(SOUND_FootstepsHardFloor, QObject::tr("Creature Footsteps, Hard Floor"), "FootstepsHardFloor.wav"),
	AuGLSoundInfo(SOUND_FootstepsMetal, QObject::tr("Creature Footsteps, Metal"), "FootstepsMetal.wav"),
	AuGLSoundInfo(SOUND_FootstepsPavement, QObject::tr("Creature Footsteps, Pavement"), "FootstepsPavement.wav"),
	AuGLSoundInfo(SOUND_FootstepsRock, QObject::tr("Creature Footsteps, Rock"), "FootstepsRock.wav"),
	AuGLSoundInfo(SOUND_FootstepsSand, QObject::tr("Creature Footsteps, Sand"), "FootstepsGravel.wav"),
	AuGLSoundInfo(SOUND_FootstepsSavanna, QObject::tr("Creature Footsteps, Savanna"), "FootstepsSavanna.wav"),
	AuGLSoundInfo(SOUND_FootstepsSnow, QObject::tr("Creature Footsteps, Snow"), "FootstepsSnow.wav"),
	AuGLSoundInfo(SOUND_FootstepsWater, QObject::tr("Creature Footsteps, Water"), "FootstepsWater.wav"),
	AuGLSoundInfo(SOUND_FootstepsWoodFloor, QObject::tr("Creature Footsteps, Wood Floor"), "FootstepsWoodFloor.wav"),
	AuGLSoundInfo(SOUND_GasLamp, QObject::tr("Item, Gas Lamp"), "GasLamp.wav"),
	AuGLSoundInfo(SOUND_GravityDriveInt, QObject::tr("Maneuver Drive Inside"), "RocketBlastInt.wav", 2.5*FeetPerMeter()),
	AuGLSoundInfo(SOUND_GravityDriveExt, QObject::tr("Maneuver Drive Outside"), "RocketBlastExt.wav", 3*FeetPerMeter()),
	AuGLSoundInfo(SOUND_HitAcid, QObject::tr("Hit, Acid"), "HitAcid.wav"),
	AuGLSoundInfo(SOUND_HitAcidCritical, QObject::tr("Hit, Acid Critical"), "HitAcid.wav"),
	AuGLSoundInfo(SOUND_HitAcidNoDamage, QObject::tr("Hit, Acid No Damage"), "HitAcid.wav"),
	AuGLSoundInfo(SOUND_HitBludgeon, QObject::tr("Hit, Bludgeon"), "HitBludgeon.wav"),
	AuGLSoundInfo(SOUND_HitBludgeonCritical, QObject::tr("Hit, Bludgeon Critical"), "HitBludgeon.wav"),
	AuGLSoundInfo(SOUND_HitBludgeonNoDamage, QObject::tr("Hit, Bludgeon No Damage"), "HitBludgeon.wav"),
	AuGLSoundInfo(SOUND_HitCannon, QObject::tr("Hit, Cannon"), "HitCannon.wav"),
	AuGLSoundInfo(SOUND_HitCold, QObject::tr("Hit, Cold"), "HitCold.wav"),
	AuGLSoundInfo(SOUND_HitColdCritical, QObject::tr("Hit, Cold Critical"), "HitCold.wav"),
	AuGLSoundInfo(SOUND_HitColdNoDamage, QObject::tr("Hit, Cold No Damage"), "HitCold.wav"),
	AuGLSoundInfo(SOUND_HitDisease, QObject::tr("Hit, Disease"), "HitDisease.wav"),
	AuGLSoundInfo(SOUND_HitDiseaseCritical, QObject::tr("Hit, Disease Critical"), "HitDisease.wav"),
	AuGLSoundInfo(SOUND_HitDiseaseNoDamage, QObject::tr("Hit, Disease No Damage"), "HitDisease.wav"),
	AuGLSoundInfo(SOUND_HitElectric, QObject::tr("Hit, Electric"), "HitElectric.wav"),
	AuGLSoundInfo(SOUND_HitElectricCritical, QObject::tr("Hit, Electric Critical"), "HitElectric.wav"),
	AuGLSoundInfo(SOUND_HitElectricNoDamage, QObject::tr("Hit, Electric No Damage"), "HitElectric.wav"),
	AuGLSoundInfo(SOUND_HitFire, QObject::tr("Hit, Fire"), "HitFire.wav"),
	AuGLSoundInfo(SOUND_HitFireCritical, QObject::tr("Hit, Fire Critical"), "HitFire.wav"),
	AuGLSoundInfo(SOUND_HitFireNoDamage, QObject::tr("Hit, Fire No Damage"), "HitFire.wav"),
	AuGLSoundInfo(SOUND_HitGasping, QObject::tr("Hit, Suffocate"), "HitGasping.wav"),
	AuGLSoundInfo(SOUND_HitGaspingCritical, QObject::tr("Hit, Suffocate Critical"), "HitGasping.wav"),
	AuGLSoundInfo(SOUND_HitGaspingNoDamage, QObject::tr("Hit, Suffocate No Damage"), "HitGasping.wav"),
	AuGLSoundInfo(SOUND_HitParalysis, QObject::tr("Hit, Paralysis"), "HitParalysis.wav"),
	AuGLSoundInfo(SOUND_HitParalysisCritical, QObject::tr("Hit, Paralysis Critical"), "HitParalysis.wav"),
	AuGLSoundInfo(SOUND_HitParalysisNoDamage, QObject::tr("Hit, Paralysis No Damage"), "HitParalysis.wav"),
	AuGLSoundInfo(SOUND_HitPierce, QObject::tr("Hit, Pierce"), "HitPierce.wav"),
	AuGLSoundInfo(SOUND_HitPierceCritical, QObject::tr("Hit, Pierce Critical"), "HitPierceCritical.wav"),
	AuGLSoundInfo(SOUND_HitPierceNoDamage, QObject::tr("Hit, Pierce No Damage"), "HitPierceNoDamage.wav"),
	AuGLSoundInfo(SOUND_HitPoison, QObject::tr("Hit, Poison"), "HitPoison.wav"),
	AuGLSoundInfo(SOUND_HitPoisonCritical, QObject::tr("Hit, Poison Critical"), "HitPoison.wav"),
	AuGLSoundInfo(SOUND_HitPoisonNoDamage, QObject::tr("Hit, Poison No Damage"), "HitPoison.wav"),
	AuGLSoundInfo(SOUND_HitRadiation, QObject::tr("Hit, Radiation"), "HitRadiation.wav"),
	AuGLSoundInfo(SOUND_HitRadiationCritical, QObject::tr("Hit, Radiation Critical"), "HitRadiation.wav"),
	AuGLSoundInfo(SOUND_HitRadiationNoDamage, QObject::tr("Hit, Radiation No Damage"), "HitRadiation.wav"),
	AuGLSoundInfo(SOUND_HitLight, QObject::tr("Hit, Sleep"), "HitSleep.wav"),
	AuGLSoundInfo(SOUND_HitLightCritical, QObject::tr("Hit, Sleep Critical"), "HitSleep.wav"),
	AuGLSoundInfo(SOUND_HitLightNoDamage, QObject::tr("Hit, Sleep No Damage"), "HitSleep.wav"),
	AuGLSoundInfo(SOUND_HitSonic, QObject::tr("Hit, Sonic"), "HitSonic.wav"),
	AuGLSoundInfo(SOUND_HitSonicCritical, QObject::tr("Hit, Sonic Critical"), "HitSonic.wav"),
	AuGLSoundInfo(SOUND_HitSonicNoDamage, QObject::tr("Hit, Sonic No Damage"), "HitSonic.wav"),
	AuGLSoundInfo(SOUND_HitSeed, QObject::tr("Hit, Implant"), "HitSeed.wav"),
	AuGLSoundInfo(SOUND_HitSeedCritical, QObject::tr("Hit, Implant Critical"), "HitSeed.wav"),
	AuGLSoundInfo(SOUND_HitSeedNoDamage, QObject::tr("Hit, Implant No Damage"), "HitSeed.wav"),
	AuGLSoundInfo(SOUND_HitSlash, QObject::tr("Hit, Slash"), "HitSlash.wav"),
	AuGLSoundInfo(SOUND_HitSlashCritical, QObject::tr("Hit, Slash Critical"), "HitSlash.wav"),
	AuGLSoundInfo(SOUND_HitSlashNoDamage, QObject::tr("Hit, Slash No Damage"), "HitSlash.wav"),
	AuGLSoundInfo(SOUND_KnifeSwish, QObject::tr("Item, Knife Swish"), "KnifeSwish.wav"),
	AuGLSoundInfo(SOUND_Submarine, QObject::tr("Vehicle, Submarine"), "Submarine.wav", 1.5*FeetPerMeter()),
	AuGLSoundInfo(SOUND_SUVIdle, QObject::tr("Vehicle, APC and SUV"), "SUVIdle.wav", 2.5*FeetPerMeter()),
	AuGLSoundInfo(SOUND_PowerPlant, QObject::tr("Spacecraft Power Plant"), "SpaceShipRumble.wav", 2.5*FeetPerMeter()),
	AuGLSoundInfo(SOUND_SailBoat, QObject::tr("Vehicle, Sailboat"), "SailBoat.wav", 1.5*FeetPerMeter()),
	AuGLSoundInfo(SOUND_Telemetry, QObject::tr("Spacecraft Sensors"), "Telemetry.wav", .1*FeetPerMeter()),
	AuGLSoundInfo(SOUND_Torch, QObject::tr("Item, Torch Flame"), "TorchFlame.wav", .15*FeetPerMeter()),
	AuGLSoundInfo(SOUND_Wind, QObject::tr("Environment, Wind"), "WindWhistling.wav", .15*FeetPerMeter()),
	AuGLSoundInfo(SOUND_WingFlap, QObject::tr("Creature Wings Flapping"), "WingFlap.wav", .15*FeetPerMeter()),
	AuGLSoundInfo(SOUND_ToneThought, QObject::tr("Comm Tone, Thought"), "Chat1.wav"),
	AuGLSoundInfo(SOUND_ToneHelp, QObject::tr("Comm Tone, Help"), "Chat2.wav"),
	AuGLSoundInfo(SOUND_Rain, QObject::tr("Environment, Rain"), "Rain.wav", .15*FeetPerMeter()),
	AuGLSoundInfo(SOUND_ToneNarrator, QObject::tr("Comm Tone, Narrator"), "Chat3.wav"),
	AuGLSoundInfo(SOUND_ToneChat, QObject::tr("Comm Tone, Chat"), "Chat4.wav"),
	AuGLSoundInfo(SOUND_FireMachineGun, QObject::tr("Fire Machine Gun"), "FireMachineGun.wav", 3*FeetPerMeter()),
	AuGLSoundInfo(SOUND_Transporter, QObject::tr("Transporter Portal"), "WindPortal.wav", 2.5*FeetPerMeter()),
	AuGLSoundInfo(SOUND_SnipWire, QObject::tr("Item, Snip Wire"), "SnipWire.wav"),
	AuGLSoundInfo(SOUND_Whizzer, QObject::tr("Item, Whizzer"), "Whizzer.wav"),
	AuGLSoundInfo(SOUND_BombRelease, QObject::tr("Fire Bomb Release"), "BombRelease.wav", 4.5*FeetPerMeter()),
	AuGLSoundInfo(SOUND_RingworldStargate, QObject::tr("Environment, Stargate"), "RingworldStargate.wav", .15*FeetPerMeter()),
	AuGLSoundInfo(SOUND_Lava, QObject::tr("Environment, Lava"), "Lava.wav", FeetPerMeter(), .2f),
	AuGLSoundInfo(SOUND_ServoStopped, QObject::tr("Servo Stopped"), "ServoStopped.wav"),
	AuGLSoundInfo(SOUND_ServoWeak, QObject::tr("Servo Weak"), "ServoWeak.wav"),
	AuGLSoundInfo(SOUND_CracklingFire, QObject::tr("Item, Crackling Fire"), "CracklingFire.wav", 1.2*FeetPerMeter()),
	AuGLSoundInfo(SOUND_NativeDrums, QObject::tr("Item, Native Drums"), "NativeDrums.wav", .6*FeetPerMeter(), .2f),
	AuGLSoundInfo(SOUND_ToneSOS, QObject::tr("Comm Tone, SOS"), "ChatSOS.wav"),
	AuGLSoundInfo(SOUND_Harvest, QObject::tr("Fire Harvester Beam"), "Harvest.wav"),
	AuGLSoundInfo(SOUND_TractorBeam, QObject::tr("Fire Tractor Beam"), "TractorBeam.wav", .25*FeetPerMeter(), .5f),
	AuGLSoundInfo(SOUND_AlertRed, QObject::tr("Alert, Red"), "AlertRed.wav"),
	AuGLSoundInfo(SOUND_AlertYellow, QObject::tr("Alert, Yellow"), "AlertYellow.wav"),
	AuGLSoundInfo(SOUND_AlertGreen, QObject::tr("Alert, Green"), "AlertGreen.wav"),
	AuGLSoundInfo(SOUND_BuddyOnLine, QObject::tr("Comm Tone, Contact On Line"), "BuddyOnLine.wav"),
	AuGLSoundInfo(SOUND_ChannelOpened, QObject::tr("Comm Tone, Channel Opened"), "CommChannel.wav"),
	AuGLSoundInfo(SOUND_WorldClaimed, QObject::tr("World Claimed"), "PlantFlag.wav"),
	AuGLSoundInfo(SOUND_HitExplosion, QObject::tr("Hit Explosion"), "HitExplosion.wav", 3*FeetPerMeter()),
	AuGLSoundInfo(SOUND_Bleep1, QObject::tr("Console Button 1"), "Bleep1.wav"),
	AuGLSoundInfo(SOUND_Bleep2, QObject::tr("Console Button 2"), "Bleep2.wav"),
	AuGLSoundInfo(SOUND_Bleep3, QObject::tr("Console Button 3"), "Bleep3.wav"),
	AuGLSoundInfo(SOUND_Bleep4, QObject::tr("Console Button 4"), "Bleep4.wav"),
	AuGLSoundInfo(SOUND_Bleep5, QObject::tr("Console Button 5"), "Bleep5.wav"),
	AuGLSoundInfo(SOUND_Bleep6, QObject::tr("Console Button 6"), "Bleep6.wav"),
	AuGLSoundInfo(SOUND_Bleep7, QObject::tr("Console Button 7"), "Bleep7.wav"),
	AuGLSoundInfo(SOUND_Bleep8, QObject::tr("Console Button 8"), "Bleep8.wav"),
	AuGLSoundInfo(SOUND_Helicopter, QObject::tr("Helicopter"), "HelicopterEngine.wav", 3*FeetPerMeter()),
	AuGLSoundInfo(MUSIC_ShroudOfGlory, QObject::tr("Music, Shroud of Glory"), "ShroudOfGlory.wav"),
	AuGLSoundInfo(MUSIC_ColderThanIce, QObject::tr("Music, Colder Than Ice"), "ColderThanIce.wav"),
};
#if !defined(QT_NO_DEBUG)
const int s_SoundCount = sizeof(s_Sounds) / sizeof(AuGLSoundInfo);
#endif

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSound::AuSound()
{
	m_SoundErrorsRemaining = 10;

	// OpenAL
	m_ALContext = 0;
	for(int ab=0; ab<SOUND_Count; ab++)
	{
		m_AudioBufferCurrent[ab] = 0;
		m_AudioBufferDefault[ab] = 0;
	}
}

AuSound::~AuSound()
{
	TerminateAudio();	// Clean up OpenAL
}

//static
const AuGLSoundInfo& AuSound::SoundInfo(SOUND_EFFECT sound)
{
#if !defined(QT_NO_DEBUG)
	// If this assert fails, the number of entries in the SOUND_EFFECT enum does not match the number of entries in the table.
	// If this assert fails, you've probably added a SOUND_EFFECT enum without adding an entry to the table.
	Q_ASSERT(s_SoundCount == SOUND_Count);						// Verify the sound info table is the correct size.

	// If this assert fails, you have passed an invalid sound value.
	Q_ASSERT(sound >= SOUND_None && sound < s_SoundCount);		// Verify the sound is within safe range.

	// If this assert fails, the sound info table order does not exactly match the order of the SOUND_ enumeration.
	Q_ASSERT(s_Sounds[sound].Sound() == sound);					// Verify the sound matches the corresponding index.
#endif

	return s_Sounds[sound];
}

void AuSound::InitializeAudio()
{
	// Initialize OpenAL
	ALCdevice* device = alcOpenDevice(NULL);
	if(!device)
	{
		qWarning() << "error: AuSound::InitializeAudio ALUT_ERROR_OPEN_DEVICE";
		return;
	}

	ALCcontext* context = alcCreateContext(device, NULL);
	if(!context)
	{
		alcCloseDevice(device);
		qWarning() << "error: AuSound::InitializeAudio ALUT_ERROR_CREATE_CONTEXT";
		return;
	}

	if(!alcMakeContextCurrent(context))
	{
		alcDestroyContext(context);
		alcCloseDevice(device);
		qWarning() << "error: AuSound::InitializeAudio ALUT_ERROR_MAKE_CONTEXT_CURRENT";
		return;
	}
	
	m_ALContext = context;

	/*
	// Load all sounds from our compressed hash archive.
	QHash<QString, QByteArray>	wavFiles;	// Sound data loaded from all har files.
	Hashpack::ReadFiles("sfx.har", wavFiles);
	if(wavFiles.empty())
		return;

	// Load our default sound buffers.
	for(int sound=1; sound<SOUND_Count; sound++)
	{
		// Create the audio buffer.
		QString soundFile = s_Sounds[sound].FileName();
		int slash = soundFile.lastIndexOf('/');
		if(slash >= 0)
			soundFile = soundFile.mid(slash+1);
		//qDebug() << "Loading sound file" << soundFile;
		QHash<QString, QByteArray>::const_iterator psi = wavFiles.find(soundFile);
		if(psi != wavFiles.end())
		{
			QByteArray ba = AuUncompress(*psi, __FILE__, __LINE__,soundFile);
			m_AudioBufferDefault[sound] = ALCreateBufferFromFileImage(ba);
			if(!m_AudioBufferDefault[sound] && m_SoundErrorsRemaining)
			{
				qWarning() << "error: Failed to load audio buffer" << ba.count() << "bytes from file" << soundFile;
				m_SoundErrorsRemaining--;
			}
		}
		else
			qWarning() << "error: Sound data not found for" << soundFile;
	}
	*/

	// Configure custom sound overrides and gain using current global settings.
	for(int s=SOUND_None+1; s < SOUND_Count; s++)
	{
		SOUND_EFFECT sound = (SOUND_EFFECT)s;
		AuSoundOverride sndOver;
		if(g_Au.SoundOverride(sound, sndOver))
			SetSoundOverride(sound, sndOver);

		float gain = g_Au.SoundGain(sound);
		SetSoundGain(sound, gain);
	}
}

void AuSound::TerminateAudio()
{
	// Clean up OpenAL
	if(!m_ALContext)
		return;
	alcMakeContextCurrent(NULL);	// Make sure the context to be destroyed is not current.
	ALCdevice* device = alcGetContextsDevice(m_ALContext);
	alcDestroyContext(m_ALContext);
	if(device)
		alcCloseDevice(device);
	m_ALContext = 0;
}

ALuint AuSound::ALCreateBufferFromFileImage(const QByteArray& wavData) const
{
	if(!m_ALContext)
		return 0;
	ALuint buffer;
	QDataStream in(wavData);
	in.setByteOrder(QDataStream::LittleEndian);

	quint8 xbuffer[5];

	// ChunkID
	// 0-3 = 'RIFF'
	for(int bi=0; bi<4; bi++)
		in >> xbuffer[bi];
	xbuffer[4] = '\0';
	if(QString((const char*)xbuffer) != "RIFF")
	{
		qWarning() << "error: AuSound::ALCreateBufferFromFileImage no RIFF";
		return 0;
	}

	// ChunkSize
	// 4-7 = File size
	in.skipRawData(4);

	// Format
	// 8-11 = 'WAVE'
	for(int bi=0; bi<4; bi++)
		in >> xbuffer[bi];
	xbuffer[4] = '\0';
	if(QString((const char*)xbuffer) != "WAVE")
	{
		qWarning() << "error: AuSound::ALCreateBufferFromFileImage no WAVE";
		return 0;
	}

	// Subchunk1ID
	// 12-15 = 'fmt '
	for(int bi=0; bi<4; bi++)
		in >> xbuffer[bi];
	xbuffer[4] = '\0';
	if(QString((const char*)xbuffer) != "fmt ")
	{
		qWarning() << "error: AuSound::ALCreateBufferFromFileImage no fmt";
		return 0;
	}

	// Subchunk1Size
	// 16-19 = Length of format data as listed above.
	quint32 subchunk1size;
	in >> subchunk1size;

	// AudioFormat
	// 20-21 = Type of format (1 is PCM) - 2 byte integer
	quint16 audioFormat;
	in >> audioFormat;
	subchunk1size -= 2;

	// NumChannels
	// 22-23 = Number of channels - 2 byte integer
	quint16 channels;
	in >> channels;
	subchunk1size -= 2;

	// SampleRate
	// 24-27 = Sample rate - 4 byte integer.
	quint32 sampleRate;
	in >> sampleRate;
	subchunk1size -= 4;

	// ByteRate
	// 28-31 = (Sample Rate * BitsPerSample * Channels) / 8
	quint32 byteRate;
	in >> byteRate;
	subchunk1size -= 4;

	// BockAlign
	// 32-33 = (BitsPerSample * Channels) / 8 - 8 bit mono2 - 8 bit stereo/16 bit mono4 - 16 bit stereo
	quint16 blockAlign = 0;
	in >> blockAlign;
	subchunk1size -= 2;

	// BitsPerSample
	// 34-35 = Bits per sample
	quint16 bitsPerSample = 0;
	in >> bitsPerSample;
	subchunk1size -= 2;

	// Skip the remainder of the chunk.
	if(subchunk1size)
		in.skipRawData(subchunk1size);

	/*
	qDebug() << "AuSound::ALCreateBufferFromFileImage bytes" << wavData.count()
		<< "audioFormat" << audioFormat
		<< "channels" << channels
		<< "sampleRate" << sampleRate
		<< "byteRate" << byteRate
		<< "bitsPerSample" << bitsPerSample
		<< "subchunk1size" << subchunk1size;
		*/

	// Skip forward to the "data" chunk.

	// Subchunk1ID
	for(int bi=0; bi<4; bi++)
		in >> xbuffer[bi];
	xbuffer[4] = '\0';
	int sanity = 10;
	while(QString((const char*)xbuffer) != "data" && sanity--)
	{
		// Skip the data in this chunk.
		quint32 subchunksize;
		in >> subchunksize;
		in.skipRawData(subchunksize);

		// Read next subchunk id.
		for(int bi=0; bi<4; bi++)
			in >> xbuffer[bi];
		xbuffer[4] = '\0';
	}
	if(!sanity)
		return 0;

	// Subchunk2Size
	quint32 dataChunkSize;
	in >> dataChunkSize;

	// Data
	quint8* bufferData = new quint8[dataChunkSize];
	for(unsigned int di=0; di<dataChunkSize; di++)
		in >> bufferData[di];

	ALenum format;
	if(bitsPerSample == 8)
		format = channels == 1 ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
	else
		format = channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, bufferData, dataChunkSize, sampleRate);

	delete [] bufferData;
	return buffer;
}

void AuSound::SetSoundGain(SOUND_EFFECT sound, float gain)
{
	if(sound <= SOUND_None || sound >= SOUND_Count)
		return;
	if(gain < 0 || gain > 2)
		return;
	s_Sounds[sound].SetGain(gain);
}

void AuSound::SetSoundOverride(SOUND_EFFECT sound, const AuSoundOverride& soundO)
{
	if(sound <= SOUND_None || sound >= SOUND_Count)
		return;
	m_AudioBufferCurrent[sound] = 0;	// Force reload of the sound when it is next requested.
	s_Sounds[sound].SetOverride(soundO);
}

// Returns the id of an audio buffer for a sound effect.
// The buffer is loaded if this is the first reference to it.
ALuint AuSound::AudioBuffer(SOUND_EFFECT sound)
{
	if(sound <= SOUND_None || sound >= SOUND_Count)
		return 0;
	if(!m_AudioBufferCurrent[sound])
	{
		// This is the first request for the sound since program start or since it was overridden while running.
		// Attempt to load override. It might not succeed.
		if(!s_Sounds[sound].Override().isEmpty())
			m_AudioBufferCurrent[sound] = LoadAudioBuffer(s_Sounds[sound].Override());

		// If no override specified/succeeded, restore our default sound.
		if(!m_AudioBufferCurrent[sound])
		{
			if(!m_AudioBufferDefault[sound])
			{
				QString soundFile = s_Sounds[sound].FileName();
				//qDebug() << "AuSound::AudioBuffer loading default buffer for sound" << sound << soundFile
					//<< "override" << s_Sounds[sound].Override();

				// Load the sound from our compressed hash archive.
				QByteArray bacomp;
				Hashpack::ReadFile("sfx.har", soundFile, bacomp);
				if(bacomp.isEmpty())
				{
					if(m_SoundErrorsRemaining)
					{
						qWarning() << "error: Sound data not found for" << soundFile;
						m_SoundErrorsRemaining--;
					}
					return 0;
				}

				// Create the audio buffer.
				//qDebug() << "Loading sound file" << soundFile;
				QByteArray ba = AuUncompress(bacomp, __FILE__, __LINE__,soundFile);
				m_AudioBufferDefault[sound] = ALCreateBufferFromFileImage(ba);
				if(!m_AudioBufferDefault[sound] && m_SoundErrorsRemaining)
				{
					qWarning() << "error: Failed to load audio buffer" << ba.count() << "bytes from file" << soundFile;
					m_SoundErrorsRemaining--;
				}
			}
			m_AudioBufferCurrent[sound] = m_AudioBufferDefault[sound];
		}
	}
	return m_AudioBufferCurrent[sound];
}

void AuSound::Play(unsigned int& audioSource, SOUND_EFFECT& curSound, SOUND_EFFECT sound, float posX, float posY, float posZ,
		float gain, float pitch, bool loopSound, float referenceDistance, float rolloffFactor, float secOffset)
{
	if(!m_ALContext)
		return;
	float gainSnd = gain * s_Sounds[sound].Gain() * g_Au.SoundGain(sound);	// Adjust gain with user's gain adjustment.
	const float minSoundGain = .001f;
	if(sound != SOUND_None && gainSnd > minSoundGain)
	{
		if(curSound != sound)
		{
			// Sound has changed.
			if(!audioSource)
				alGenSources(1, &audioSource);	// Create a new source.
			else
				alSourceStop(audioSource);	// Stop the current buffer.

			// Select the appropriate sound into the buffer.
			alSourcei(audioSource, AL_BUFFER, AudioBuffer(sound));
		}

		// Configure sound parameters.
		alSource3f(audioSource, AL_POSITION, posX, posY, posZ);
		alSourcef(audioSource, AL_PITCH, pitch);
		alSourcef(audioSource, AL_GAIN, gainSnd);
		alSourcef(audioSource, AL_REFERENCE_DISTANCE, referenceDistance);
		alSourcef(audioSource, AL_ROLLOFF_FACTOR, rolloffFactor);

		// Start the sound.
		ALenum state;
		alGetSourcei(audioSource, AL_SOURCE_STATE, &state);
		if(state == AL_INITIAL)
		{
			alSourcei(audioSource, AL_LOOPING, loopSound?AL_TRUE:AL_FALSE);
			alSourcef(audioSource, AL_SEC_OFFSET, secOffset);
			alSourcePlay(audioSource);
		}
		else if(state == AL_PAUSED || state == AL_STOPPED)
		{
			alSourcePlay(audioSource);
		}
		curSound = sound;
	}
	else
	{
		// Delete the sound source.
		StopSound(audioSource);
		curSound = SOUND_None;
	}
}

// 'audioSource' is a return parameter. It will be allocated if 0.
void AuSound::Play(unsigned int& audioSource, ALuint audioBuffer, float posX, float posY, float posZ, float gain, float pitch,
		float referenceDistance, float rolloffFactor)
{
	if(!m_ALContext)
		return;
	Q_ASSERT(audioBuffer);
	// Create the audio source if needed.
	if(!audioSource)
		alGenSources(1, &audioSource);	// Create a new source.

	// Configure sound parameters.
	alSource3f(audioSource, AL_POSITION, posX, posY, posZ);
	alSourcef(audioSource, AL_PITCH, pitch);
	alSourcef(audioSource, AL_GAIN, gain);
	alSourcef(audioSource, AL_REFERENCE_DISTANCE, referenceDistance);
	alSourcef(audioSource, AL_ROLLOFF_FACTOR, rolloffFactor);

	// Determine the current state of the audioSource.
	ALenum state;
	alGetSourcei(audioSource, AL_SOURCE_STATE, &state);
	if(state != AL_PLAYING)
	{
		// A sound is not currently playing.
		// Select the sound buffer into the audio source.
		alSourcei(audioSource, AL_BUFFER, audioBuffer);

		// Start playing the sound from the beginning.
		alSourcePlay(audioSource);
	}
}

//static
AuSoundOverride AuSound::DefaultSoundOverride(SOUND_EFFECT sound)
{
	return s_Sounds[sound].DefaultOverride();
}

//static
float AuSound::DefaultSoundGain(SOUND_EFFECT sound)
{
	switch(sound)
	{
	case MUSIC_ColderThanIce:
	case MUSIC_ShroudOfGlory:
		return .5f;
	case SOUND_ToneChat:
	case SOUND_ToneSOS:
		return .5f;
	case SOUND_ToneNarrator:
		return .6f;
	case SOUND_ToneThought:
		return .4f;
	case SOUND_Bleep1:
	case SOUND_Bleep2:
	case SOUND_Bleep3:
	case SOUND_Bleep4:
	case SOUND_Bleep5:
	case SOUND_Bleep6:
	case SOUND_Bleep7:
	case SOUND_Bleep8:
		return .25f;
	default:
		return 1.f;
	}
}

ALuint AuSound::LoadAudioBuffer(const QString& fileName) const
{
	if(!m_ALContext)
		return 0;
	Q_ASSERT(!fileName.isEmpty());
	QFile sfx(fileName);
	if(sfx.open(QIODevice::ReadOnly))
	{
		// Successfully opened the file. Read the audio data.
		QByteArray sfxdata = sfx.readAll();
		sfx.close();

		// Create the audio buffer.
		ALuint bufferName = ALCreateBufferFromFileImage(sfxdata);
		if(!bufferName && m_SoundErrorsRemaining)
		{
			qWarning() << "error: Failed to load audio buffer" << sfxdata.count() << "bytes from file" << fileName;
			m_SoundErrorsRemaining--;
		}
		return bufferName;
	}
	else if(m_SoundErrorsRemaining)
	{
		qWarning() << "error: Failed to open audio file" << fileName;
		m_SoundErrorsRemaining--;
	}
	return 0;
}
