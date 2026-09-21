//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audnasettings.h,v 1.42 2022/02/19 17:55:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDNASettings_H
#define AuDNASettings_H

#include "audialog.h"
#include "audna.h"

class AuChromosomeWidget;
class AuToolButtonValue;
class AuDNAView;
class QComboBox;
class QLabel;
class QSplitter;
class QStackedLayout;
class QVBoxLayout;

class AuDNASettings : public QWidget
{
	Q_OBJECT;
	AuDNA								m_DNA;
	QMap<AuChromosome::CTYPE, int>		m_PageIndexes;

	// Widgets
	AuToolButtonValue*					m_AArmor;
	AuToolButtonValue*					m_AClothes;
	QComboBox*							m_AHaircut;
	AuToolButtonValue*					m_AHat;
	QComboBox*							m_AMouth;
	QComboBox*							m_APose;
	QComboBox*							m_AShave;
	QLabel*								m_AStatus;
	AuToolButtonValue*					m_BArmor;
	AuToolButtonValue*					m_BClothes;
	QComboBox*							m_BHaircut;
	AuToolButtonValue*					m_BHat;
	QComboBox*							m_BMouth;
	QComboBox*							m_BPose;
	QComboBox*							m_BShave;
	QLabel*								m_BStatus;
	QVBoxLayout*						m_Chromosomes;
	QList<AuChromosomeWidget*>			m_ChromosomeWidgets;
	QStackedLayout*						m_Pages;
	QComboBox*							m_ShowGround;
	QSplitter*							m_Splitter;
	AuDNAView*							m_View;

public:
	AuDNASettings(QWidget* parent=0);
	virtual ~AuDNASettings();

	inline const AuDNA& DNA() const
	{
		return m_DNA;
	}

	bool OkForAvatar();

	void SetDNA(const AuDNA& dna);

public slots:
	void AddChromosome();
	void AArmorClicked();
	void AClothesClicked();
	void AHaircutChanged(int index);
	void AHatClicked();
	void AMouthChanged(int);
	void APoseChanged(int);
	void ApplyChromosomeChanges();
	void ApplyChromosomeChangesToAll();
	void ApplyChromosomeChangesToPair();
	void AShaveChanged(int index);
	void BArmorClicked();
	void BClothesClicked();
	void BHaircutChanged(int index);
	void BHatClicked();
	void BMouthChanged(int);
	void BPoseChanged(int);
	void BShaveChanged(int index);
	void Human();
	void LoadDNA();
	void NewDNA();
	void Random();
	void SaveDNA();
	void SetShowGround(int index);
	void SplitterMoved(int pos, int index);
	void ChromosomeClicked(AuChromosomeWidget* tw);
	void ChromosomeDeleted(AuChromosomeWidget* tw);
	void Wizard();

protected:
	void ListChromosomes();
	void UpdateStatus();
	void UpdateView();

signals:
	void StatusUpdated();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDNASettingsDlg : public AuDialog
{
	Q_OBJECT;
	AuDNASettings*						m_DNASettings;
	QPushButton*						m_OkButton;

public:
	AuDNASettingsDlg(QWidget* parent=0);
	virtual ~AuDNASettingsDlg();

	const AuDNA& DNA() const;

	void SetDNA(const AuDNA& dna);

public slots:
	void EnableControls();
	void OkClicked();
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuDNASettings_H
