//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auintrobase.h,v 1.31 2022/02/19 17:57:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuIntroBase_H
#define AuIntroBase_H

#include "auclientlib.h"
#include "audialog.h"
#include "audnabase.h"
#include "auid.h"
#include "auspecies.h"
//#include <QVideoWidget>
#include <QWidget>

class AuBodyRenderWidget;
class AuDNA;
class AuDNABase;
class AuIntroCreateArms;
class AuIntroCreateBody;
class AuIntroCreateGeneral;
class AuIntroCreateHead;
class AuIntroCreateIndividual;
class AuIntroCreateIndividual4;
class AuIntroCreateLegs;
class AuIntroCreateTail;
class AuIntroCreateWeapon;
class AuIntroStartScenarios;
class AuPictureWidget;
class AuTabColumn;
class DhDNA;
class QCheckBox;
class QComboBox;
class QDateTimeEdit;
class QLabel;
class QLineEdit;
//class QMediaPlayer;
//class QMediaPlaylist;
class QPushButton;
class QSlider;
class QStackedLayout;
class QTableWidget;
class QToolButton;

class AUCLIENT_DLL AuIntroBase : public QWidget	//public QVideoWidget
{
	Q_OBJECT;
protected:
	QStringList							m_Credits;
private:
	int									m_CreditsHt;
	//QMediaPlayer*						m_Player;
	//QMediaPlaylist*						m_Playlist;
	QImage								m_SplashImage;
	int									m_YPos;

public:	// Constructors
	AuIntroBase(QWidget* parent=0);
	virtual ~AuIntroBase();

public:	// Methods
	bool SetSplashImage(const QImage& image);

	bool SetSplashMovie(const QUrl& movie);

	virtual bool HasAccountInfo() const = 0;					// Return true when account info has been set using SetAccountInfo.
	virtual void paintEvent(QPaintEvent* pe);
	virtual void SetAccountInfo() = 0;
	virtual void ShowWaitInfoMsg() = 0;

public slots:
	// These slots are just stubs in this class.
	virtual void EmpireDataReceived();
	virtual void EnterNewAvatar();								// Proceed with entry into hazeron with new avatar.
	virtual void setVisible(bool visible);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroAvatarWidget : public QWidget
{
	Q_OBJECT;
	bool								m_Solo;
	AuDNABase::CHROMATID				m_UnitChromatid;
	AuDNABase*							m_UnitDNA;
	AuId								m_UnitId;
	int									m_UnitIndex;
	HANDEDNESS							m_UnitHand;

	// Widgets
	QToolButton*						m_Enter;
	QToolButton*						m_Delete;
	QPushButton*						m_Name;
	QPushButton*						m_NewDNA3;
	QPushButton*						m_NewDNA4;
	QToolButton*						m_Save;
	QLabel*								m_Whereabouts;

public:
	AuIntroAvatarWidget(bool solo, int index, QWidget* parent=0);
	virtual ~AuIntroAvatarWidget();

	void Clear();

	// Can be 0.
	inline const AuDNABase* DNA() const
	{
		return m_UnitDNA;
	}

	inline AuDNABase::CHROMATID Chromatid() const
	{
		return m_UnitChromatid;
	}

	inline HANDEDNESS Handedness() const
	{
		return m_UnitHand;
	}

	inline const AuId& Id() const
	{
		return m_UnitId;
	}

	void SetAvatar(const AuId& id, const QString& name, const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand);

	void SetCurrent(bool isCurrent);

	void SetWhereabouts(const QString& where);

protected slots:
	void Delete();
	void Enter();
	void Save();
	void Select();
	void SelectDNA3();
	void SelectDNA4();

signals:
	void Deleted(AuId avatarId);
	void EnterHazeron(AuId avatarId);
	void NewDNA3(int index);
	void NewDNA4(int index);
	void Selected(int index);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroChooseAvatar : public QWidget
{
	Q_OBJECT;
	int									m_AvatarSampleRendered;	// Number of frames of sample rendered.

	// Widgets
	AuIntroAvatarWidget*				m_Avatar[6];
	AuBodyRenderWidget* 				m_AvatarSample;
	QComboBox*							m_Mouth;
	AuPictureWidget*					m_NewAvatarImage;
	QComboBox*							m_Pose;

public:
	AuIntroChooseAvatar(QWidget* parent=0);
	virtual ~AuIntroChooseAvatar();

	void SetAccountInfo();

public slots:
	void UpdateGL();

protected slots:
	void AvatarDeleted(AuId avatarId);
	void AvatarNewDNA3(int index);
	void AvatarNewDNA4(int index);
	void AvatarSelected(int index);
	void MouthChanged(int index);
	void PoseChanged(int index);

signals:
	void AvatarLogin(AuId);
	void NewAvatarDNA3(int index);
	void NewAvatarDNA4(int index);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroSoloAvatar : public QWidget
{
	Q_OBJECT;
	int									m_AvatarSampleRendered;	// Number of frames of sample rendered.

	// Widgets
	AuIntroAvatarWidget*				m_Avatar[6];
	AuBodyRenderWidget* 				m_AvatarSample;
	QComboBox*							m_Mouth;
	AuPictureWidget*					m_NewAvatarImage;
	QComboBox*							m_Pose;

public:
	AuIntroSoloAvatar(QWidget* parent=0);
	virtual ~AuIntroSoloAvatar();

	void SetAccountInfo();

public slots:
	void UpdateGL();

protected slots:
	void AvatarDeleted(AuId avatarId);
	void AvatarNewDNA3(int index);
	void AvatarNewDNA4(int index);
	void AvatarSelected(int index);
	void MouthChanged(int index);
	void PoseChanged(int index);

signals:
	void AvatarLogin(AuId);
	void NewAvatarDNA3(int index);
	void NewAvatarDNA4(int index);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroEmpireSpecies : public AuDialog
{
	Q_OBJECT;
	AuSpecies							m_Species;

	// Widgets
	QTableWidget*						m_Empires;
	QPushButton*						m_OkButton;

public:
	AuIntroEmpireSpecies(QWidget* parent=0);
	virtual ~AuIntroEmpireSpecies();

	inline const AuSpecies& Species() const
	{
		return m_Species;
	}

public slots:
	void EmpireChanged();
	void EmpireDataReceived();
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroCreateAvatar : public QWidget
{
	Q_OBJECT;
	int									m_AvatarIndex;			// Index of slot for which we are creating an avatar.
	int									m_AvatarSampleRendered;	// Number of frames of sample rendered.
	AuDNABase::CHROMATID				m_Chromatid;
	HANDEDNESS							m_Hand;
	bool								m_Solo;

	// Widgets
	AuIntroCreateArms*					m_Arms;
	AuBodyRenderWidget* 				m_AvatarSample;
	QToolButton*						m_BackButton;
	AuIntroCreateBody*					m_Body;
	QToolButton*						m_EmpireDNA;
	AuIntroEmpireSpecies*				m_EmpireDNADialog;
	QToolButton*						m_Enter;
	AuIntroCreateGeneral*				m_General;
	AuIntroCreateHead*					m_Head;
	AuIntroCreateIndividual*			m_Individual;
	AuIntroCreateLegs*					m_Legs;
	QToolButton*						m_Load;
	QStackedLayout*						m_Pages;
	QComboBox*							m_Pose;
	QToolButton*						m_RandomDNA;
	QToolButton*						m_ResetDNA;
	QToolButton*						m_Save;
	AuIntroStartScenarios*				m_Scenarios;
	QLabel*								m_Summary;
	AuTabColumn*						m_Tabs;
	AuIntroCreateTail*					m_Tail;
	AuIntroCreateWeapon*				m_Weapon;

public:
	AuIntroCreateAvatar(QWidget* parent=0);
	virtual ~AuIntroCreateAvatar();

	inline void SetAvatarIndex(int index)
	{
		m_AvatarIndex = index;
	}

	void SetDNA(const DhDNA& dna);

	void SetSolo(bool solo);

public slots:
	void EnableControls();
	void EnterNewAvatar();				// Proceed with entry into hazeron with new avatar.
	void PoseChanged(int index);
	virtual void setVisible(bool visible);
	void UpdateGL();

protected:
	void SetAvatar(const DhDNA& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand);

protected slots:
	void DNAChanged();
	void EmpireDNA();
	void Enter();
	void HandednessChanged();
	void Load();
	void Random();
	void Reset();
	void Save();
	void TabChanged(int idx);

signals:
	void Back();
	void EmpireDataReceived();
	void NewAvatarLogin(int index);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroCreateAvatar4 : public QWidget
{
	Q_OBJECT;
	int									m_AvatarIndex;			// Index of slot for which we are creating an avatar.
	int									m_AvatarSampleRendered;	// Number of frames of sample rendered.
	AuDNABase::CHROMATID				m_Chromatid;
	HANDEDNESS							m_Hand;
	bool								m_Solo;

	// Widgets
	AuBodyRenderWidget* 				m_AvatarSample;
	QToolButton*						m_BackButton;
	QToolButton*						m_CreateDNA;
	QToolButton*						m_EmpireDNA;
	AuIntroEmpireSpecies*				m_EmpireDNADialog;
	QToolButton*						m_Enter;
	AuIntroCreateIndividual4*			m_Individual;
	QComboBox*							m_Mouth;
	QComboBox*							m_Pose;
	AuIntroStartScenarios*				m_Scenarios;
	QLabel*								m_Summary;

public:
	AuIntroCreateAvatar4(QWidget* parent=0);
	virtual ~AuIntroCreateAvatar4();

	inline void SetAvatarIndex(int index)
	{
		m_AvatarIndex = index;
	}

	void SetDNA(const AuDNA& dna);

	void SetSolo(bool solo);

public slots:
	void EnableControls();
	void EnterNewAvatar();				// Proceed with entry into hazeron with new avatar.
	void MouthChanged(int index);
	void PoseChanged(int index);
	virtual void setVisible(bool visible);
	void UpdateGL();

protected:
	void SetAvatar(const AuDNA& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand);

protected slots:
	void CreateDNA();
	void DNAChanged();
	void EmpireDNA();
	void Enter();
	void HandednessChanged();

signals:
	void Back();
	void EmpireDataReceived();
	void NewAvatarLogin(int index);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroCreateIndividual : public QWidget
{
	Q_OBJECT;
	bool								m_Solo;

	// Widgets
	QComboBox*							m_Chromatid;
	QComboBox*							m_Hand;
	QLineEdit*							m_Name;

public:
	AuIntroCreateIndividual(QWidget* parent=0);
	virtual ~AuIntroCreateIndividual();

	AuDNABase::CHROMATID Chromatid() const;

	HANDEDNESS Handedness() const;

	QString Name() const;

	void SetChromatid(AuDNABase::CHROMATID chroma);

	void SetHandedness(HANDEDNESS hand);

	void SetSolo(bool solo);

signals:
	void ChromatidChanged();
	void HandChanged();
	void NameChanged();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroCreateIndividual4 : public QWidget
{
	Q_OBJECT;
	bool								m_Solo;

	// Widgets
	QComboBox*							m_Chromatid;
	QComboBox*							m_Hand;
	QLineEdit*							m_Name;

public:
	AuIntroCreateIndividual4(QWidget* parent=0);
	virtual ~AuIntroCreateIndividual4();

	AuDNABase::CHROMATID Chromatid() const;

	HANDEDNESS Handedness() const;

	QString Name() const;

	void SetChromatid(AuDNABase::CHROMATID chroma);

	void SetDNA(const AuDNA& dna);

	void SetHandedness(HANDEDNESS hand);

	void SetSolo(bool solo);

signals:
	void ChromatidChanged();
	void HandChanged();
	void NameChanged();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroCreateGeneral : public QWidget
{
friend class AuIntroCreateAvatar;
	Q_OBJECT;
	// Widgets
	QSlider*							m_Agility;
	QComboBox*							m_Brain;
	QSlider*							m_BottomSkinThickness;
	QComboBox*							m_BottomSkinType;
	QComboBox*							m_CrestType;
	QComboBox*							m_EcoRole;
	QSlider*							m_Endurance;
	QSlider*							m_OverallSize;
	QSlider*							m_Speed;
	QSlider*							m_Strength;
	QComboBox*							m_Phylum;
	QSlider*							m_TopSkinThickness;
	QComboBox*							m_TopSkinType;

public:
	AuIntroCreateGeneral(AuIntroCreateAvatar* parent=0);
	virtual ~AuIntroCreateGeneral();

	// Collects traits from my controls and sets corresponding values in 'dna'.
	void DNATraits(DhDNA& dna) const;

	void SetDNA(const DhDNA& dna);

signals:
	void DNAChanged();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroCreateHead : public QWidget
{
friend class AuIntroCreateAvatar;
	Q_OBJECT;
	// Widgets
	QSlider*							m_CrestSize;
	QSlider*							m_EarSize;
	QComboBox*							m_EarType;
	QSlider*							m_EyeSize;
	QComboBox*							m_EyeType;
	QComboBox*							m_Heads;
	QSlider*							m_HornSize;
	QComboBox*							m_HornType;
	QSlider*							m_MouthSize;
	QComboBox*							m_MouthType;
	QSlider*							m_NeckVertebrae;
	QSlider*							m_NoseSize;
	QComboBox*							m_NoseType;

public:
	AuIntroCreateHead(AuIntroCreateAvatar* parent=0);
	virtual ~AuIntroCreateHead();

	// Collects traits from my controls and sets corresponding values in 'dna'.
	void DNATraits(DhDNA& dna) const;

	void SetDNA(const DhDNA& dna);

	void SetPhylum(/*DhDNA::SKIN*/int phylum);

signals:
	void DNAChanged();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroCreateBody : public QWidget
{
friend class AuIntroCreateAvatar;
	Q_OBJECT;
	// Widgets
	QComboBox*							m_BodyCarriage;
	QCheckBox*							m_CanJump;
	QSlider*							m_CrestSize;
	QSlider*							m_TorsoLength;
	QSlider*							m_TorsoThickness;
	QSlider*							m_TorsoVertebrae;

public:
	AuIntroCreateBody(AuIntroCreateAvatar* parent=0);
	virtual ~AuIntroCreateBody();

	// Collects traits from my controls and sets corresponding values in 'dna'.
	void DNATraits(DhDNA& dna) const;

	void SetDNA(const DhDNA& dna);

signals:
	void DNAChanged();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroCreateArms : public QWidget
{
friend class AuIntroCreateAvatar;
	Q_OBJECT;
	// Widgets
	QComboBox*							m_ArmPairs;
	QComboBox*							m_ArmType;
	QSlider*							m_HandLength;
	QComboBox*							m_HandType;
	QSlider*							m_LowerLength;
	QSlider*							m_LowerThickness;
	QSlider*							m_UpperLength;
	QSlider*							m_UpperThickness;
	QComboBox*							m_WingType;

public:
	AuIntroCreateArms(AuIntroCreateAvatar* parent=0);
	virtual ~AuIntroCreateArms();

	// Collects traits from my controls and sets corresponding values in 'dna'.
	void DNATraits(DhDNA& dna) const;

	void SetDNA(const DhDNA& dna);

signals:
	void DNAChanged();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroCreateLegs : public QWidget
{
friend class AuIntroCreateAvatar;
	Q_OBJECT;
	// Widgets
	QSlider*							m_FootLength;
	QComboBox*							m_FootType;
	QComboBox*							m_LegPairs;
	QComboBox*							m_LegType;
	QSlider*							m_LowerLength;
	QSlider*							m_LowerThickness;
	QSlider*							m_UpperLength;
	QSlider*							m_UpperThickness;

public:
	AuIntroCreateLegs(AuIntroCreateAvatar* parent=0);
	virtual ~AuIntroCreateLegs();

	// Collects traits from my controls and sets corresponding values in 'dna'.
	void DNATraits(DhDNA& dna) const;

	void SetDNA(const DhDNA& dna);

signals:
	void DNAChanged();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroCreateTail : public QWidget
{
friend class AuIntroCreateAvatar;
	Q_OBJECT;
	// Widgets
	QSlider*							m_Size;
	QComboBox*							m_Type;

public:
	AuIntroCreateTail(AuIntroCreateAvatar* parent=0);
	virtual ~AuIntroCreateTail();

	// Collects traits from my controls and sets corresponding values in 'dna'.
	void DNATraits(DhDNA& dna) const;

	void SetDNA(const DhDNA& dna);

	void SetPhylum(/*DhDNA::SKIN*/int phylum);

signals:
	void DNAChanged();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroCreateWeapon : public QWidget
{
friend class AuIntroCreateAvatar;
	Q_OBJECT;
	// Widgets
	QComboBox*							m_Location;
	QComboBox*							m_Mode;
	QSlider*							m_Size;
	QComboBox*							m_Type;

public:
	AuIntroCreateWeapon(AuIntroCreateAvatar* parent=0);
	virtual ~AuIntroCreateWeapon();

	// Collects traits from my controls and sets corresponding values in 'dna'.
	void DNATraits(DhDNA& dna) const;

	void SetDNA(const DhDNA& dna);

	void UpdateWeaponLocations(const DhDNA& dna);

signals:
	void DNAChanged();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroStartReach : public QWidget
{
	Q_OBJECT;
	bool								m_Solo;

	// Widgets
	QDateTimeEdit*						m_Asteroid;
	QComboBox*							m_Region;
	QLabel*								m_RoidIco;

public:
	AuIntroStartReach(QWidget* parent=0);
	virtual ~AuIntroStartReach();

public:
	QDateTime AsteroidImpact() const;

	// Returns true if the settings ok to start a game.
	bool OkToGo() const;

	quint8 Region() const;

	void SetSolo(bool solo);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroStartCitizen : public QWidget
{
	Q_OBJECT;
	QHash<AuId, GALAXY>					m_SectorGalaxies;

	// Widgets
	QComboBox*							m_EmpiresCombo;
	QLineEdit*							m_JoinPass;
	QComboBox*							m_Sectors;

public:
	AuIntroStartCitizen(QWidget* parent=0);
	virtual ~AuIntroStartCitizen();

public:
	QByteArray EmpireJoinPassCrypt() const;

	AuId EmpireJoinSector() const;

	quint32 EmpireToJoin() const;

	GALAXY EmpireJoinGalaxy() const;

	// Returns true if the settings ok to start a game.
	bool OkToGo() const;

	void SetSolo(bool solo);

public slots:
	virtual void setVisible(bool visible);

protected slots:
	void EmpireChanged(int);
	void PasswordChanged(const QString&);

protected:
	void ListEmpireSectors(quint32 empireId);

signals:
	void OkToGoChanged();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroStartSavage : public QWidget
{
	Q_OBJECT;
	// Widgets
	QComboBox*							m_Region;

public:
	AuIntroStartSavage(QWidget* parent=0);
	virtual ~AuIntroStartSavage();

public:
	// Returns true if the settings ok to start a game.
	bool OkToGo() const;

	quint8 Region() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroStartLost : public QWidget
{
	Q_OBJECT;
	QImage								m_Flag;
	bool								m_IsOk;

	// Widgets
	QToolButton*						m_FlagButton;
	QLineEdit*							m_NameEdit;
	QLabel*								m_NameReason;
	QComboBox*							m_Region;

public:
	AuIntroStartLost(QWidget* parent=0);
	virtual ~AuIntroStartLost();

public:
	QString NewEmpireName() const;

	inline QImage NewEmpireFlag() const
	{
		return m_Flag;
	}

	// Returns true if the settings ok to start a game.
	bool OkToGo() const;

	quint8 Region() const;

public slots:
	virtual void setVisible(bool visible);

protected slots:
	void ChooseFlag();
	void EnableControls();

signals:
	void OkToGoChanged();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuIntroStartScenarios : public QWidget
{
	Q_OBJECT;
	bool								m_Solo;

	// Widgets
	AuIntroStartCitizen*				m_Citizen;
	AuIntroStartLost*					m_Lost;
	QStackedLayout*						m_Pages;
	AuIntroStartReach*					m_Reach;
	AuIntroStartSavage*					m_Savage;
	QComboBox*							m_Scenarios;

public:
	AuIntroStartScenarios(QWidget* parent=0);
	virtual ~AuIntroStartScenarios();

	QDateTime AsteroidImpact() const;

	QByteArray EmpireJoinPassCrypt() const;

	AuId EmpireJoinSector() const;

	quint32 EmpireToJoin() const;

	GALAXY Galaxy() const;

	QImage NewEmpireFlag() const;

	QString NewEmpireName() const;

	// Returns true if the settings on the current page are ok to start a game.
	bool OkToGo() const;

	quint8 Region() const;

	SCENARIO Scenario() const;

	void SetSolo(bool solo);

public slots:
	virtual void setVisible(bool visible);

protected slots:
	void ScenarioChanged(int);

signals:
	void OkToGoChanged();
};

#endif // AuIntroBase_H
