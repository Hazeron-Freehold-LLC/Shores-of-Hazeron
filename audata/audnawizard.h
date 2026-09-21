//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audnawizard.h,v 1.5 2022/02/19 18:19:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDNAWizard_H
#define AuDNAWizard_H

#include "audialog.h"
#include "audna.h"

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QStackedLayout;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDNAWizardPage : public QWidget
{
	Q_OBJECT;
public:
	enum PAGE
	{
		PAGE_Invalid,
		PAGE_Intro,
		PAGE_Phylum,

		PAGE_Blood,
		PAGE_Breath,
		PAGE_Eat,
		PAGE_Hide,
		PAGE_Reproduction,

		PAGE_Head,
		PAGE_Body,
		PAGE_Limbs,
		PAGE_Finish,
	};

public:
	AuDNAWizardPage(QWidget* parent=0);
	virtual ~AuDNAWizardPage();

	// Applies this page's properties to 'dna'.
	// 'headDiaMM' is the largest axis diameter of the head, for calculating initial sizes that are sensible.
	// When this is called, the wizard has finished ok therefore we know the necessary pages have signaled ok to proceed or finish.
	// BuildDNA is called on each page, in the order they were presented by the wizard.
	// Returns false if settings on this page would render 'dna' invalid.
	// The finish page returns false if the final 'dna' is invalid.
	virtual bool BuildDNA(AuDNA& dna, quint16 headDiaMM) const = 0;

	virtual PAGE PageType() const = 0;

public slots:
	virtual void CheckOkToProceed() = 0;
	virtual void LoadSettings() = 0;
	virtual void SaveSettings() const = 0;
	virtual void setVisible(bool visible);

signals:
	void OkToFinish();
	void OkToProceed(int nextPage);	// 'nextPage' is a PAGE enum value
	void NotOkToProceed();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDNAWizardPageIntro : public AuDNAWizardPage
{
	Q_OBJECT;
public:
	AuDNAWizardPageIntro(QWidget* parent=0);
	virtual ~AuDNAWizardPageIntro();

	virtual bool BuildDNA(AuDNA& dna, quint16 headDiaMM) const
	{
		Q_UNUSED(dna);
		Q_UNUSED(headDiaMM);
		return true;
	}

	virtual PAGE PageType() const
	{
		return PAGE_Intro;
	}

public slots:
	virtual void CheckOkToProceed();
	virtual void LoadSettings()
	{
	}
	virtual void SaveSettings() const
	{
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDNAWizardPageFinish : public AuDNAWizardPage
{
	Q_OBJECT;
public:
	AuDNAWizardPageFinish(QWidget* parent=0);
	virtual ~AuDNAWizardPageFinish();

	// The final page tests the complete 'dna' that was built, to reject an invalid final result.
	virtual bool BuildDNA(AuDNA& dna, quint16 headDiaMM) const
	{
		Q_UNUSED(headDiaMM);
		return dna.IsValid();
	}

	virtual PAGE PageType() const
	{
		return PAGE_Finish;
	}

public slots:
	virtual void CheckOkToProceed();
	virtual void LoadSettings()
	{
	}
	virtual void SaveSettings() const
	{
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDNAWizardPagePhylum : public AuDNAWizardPage
{
	Q_OBJECT;
	QRadioButton*						m_AnnelRadio;
	QRadioButton*						m_ArthRadio;
	QComboBox*							m_ChorCombo;
	QRadioButton*						m_ChorRadio;
	QComboBox*							m_MollCombo;
	QRadioButton*						m_MollRadio;

public:
	AuDNAWizardPagePhylum(QWidget* parent=0);
	virtual ~AuDNAWizardPagePhylum();

	virtual bool BuildDNA(AuDNA& dna, quint16 headDiaMM) const;

	virtual PAGE PageType() const
	{
		return PAGE_Phylum;
	}

public slots:
	virtual void CheckOkToProceed();
	virtual void LoadSettings();
	virtual void SaveSettings() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDNAWizardPageHide : public AuDNAWizardPage
{
	Q_OBJECT;
	QRadioButton*						m_FeathersRadio;
	QRadioButton*						m_FurRadio;
	QRadioButton*						m_ScalesRadio;
	QRadioButton*						m_SkinRadio;

public:
	AuDNAWizardPageHide(QWidget* parent=0);
	virtual ~AuDNAWizardPageHide();

	virtual bool BuildDNA(AuDNA& dna, quint16 headDiaMM) const;

	virtual PAGE PageType() const
	{
		return PAGE_Hide;
	}

public slots:
	virtual void CheckOkToProceed();
	virtual void LoadSettings();
	virtual void SaveSettings() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDNAWizardPageBlood : public AuDNAWizardPage
{
	Q_OBJECT;
	QRadioButton*						m_ColdRadio;
	QRadioButton*						m_WarmRadio;

public:
	AuDNAWizardPageBlood(QWidget* parent=0);
	virtual ~AuDNAWizardPageBlood();

	virtual bool BuildDNA(AuDNA& dna, quint16 headDiaMM) const;

	virtual PAGE PageType() const
	{
		return PAGE_Blood;
	}

public slots:
	virtual void CheckOkToProceed();
	virtual void LoadSettings();
	virtual void SaveSettings() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDNAWizardPageBreath : public AuDNAWizardPage
{
	Q_OBJECT;
	QCheckBox*							m_GillsCheck;
	QCheckBox*							m_LungsCheck;

public:
	AuDNAWizardPageBreath(QWidget* parent=0);
	virtual ~AuDNAWizardPageBreath();

	virtual bool BuildDNA(AuDNA& dna, quint16 headDiaMM) const;

	virtual PAGE PageType() const
	{
		return PAGE_Breath;
	}

public slots:
	virtual void CheckOkToProceed();
	virtual void LoadSettings();
	virtual void SaveSettings() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDNAWizardPageEat : public AuDNAWizardPage
{
	Q_OBJECT;
	QCheckBox*							m_AnimalsCheck;
	QCheckBox*							m_BloodSucker;
	QCheckBox*							m_Brain;
	QCheckBox*							m_Cannibal;
	QCheckBox*							m_Carne;
	QCheckBox*							m_Carrion;
	QCheckBox*							m_Organs;
	QCheckBox*							m_PlantsCheck;
	QCheckBox*							m_Predator;

public:
	AuDNAWizardPageEat(QWidget* parent=0);
	virtual ~AuDNAWizardPageEat();

	virtual bool BuildDNA(AuDNA& dna, quint16 headDiaMM) const;

	virtual PAGE PageType() const
	{
		return PAGE_Eat;
	}

public slots:
	virtual void CheckOkToProceed();
	virtual void LoadSettings();
	virtual void SaveSettings() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDNAWizardPageReproduction : public AuDNAWizardPage
{
	Q_OBJECT;
	QRadioButton*						m_AsexualRadio;
	QComboBox*							m_BirthPlace;
	QSpinBox*							m_LitterMax;
	QSpinBox*							m_LitterMin;
	QCheckBox*							m_MilkCheck;
	QRadioButton*						m_SexualRadio;
	QComboBox*							m_Young;

public:
	AuDNAWizardPageReproduction(QWidget* parent=0);
	virtual ~AuDNAWizardPageReproduction();

	virtual bool BuildDNA(AuDNA& dna, quint16 headDiaMM) const;

	virtual PAGE PageType() const
	{
		return PAGE_Reproduction;
	}

public slots:
	virtual void CheckOkToProceed();
	virtual void LoadSettings();
	virtual void SaveSettings() const;
	void LitterMaxChanged(int value);
	void LitterMinChanged(int value);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDNAWizardPageHead : public AuDNAWizardPage
{
	Q_OBJECT;
	QRadioButton*						m_BackRadio;
	QRadioButton*						m_BottomRadio;
	QSpinBox*							m_Diameter;

public:
	AuDNAWizardPageHead(QWidget* parent=0);
	virtual ~AuDNAWizardPageHead();

	virtual bool BuildDNA(AuDNA& dna, quint16 headDiaMM) const;

	virtual PAGE PageType() const
	{
		return PAGE_Head;
	}

	quint16 DiameterMM() const;

public slots:
	virtual void CheckOkToProceed();
	virtual void LoadSettings();
	virtual void SaveSettings() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDNAWizardPageBody : public AuDNAWizardPage
{
	Q_OBJECT;
public:
	AuDNAWizardPageBody(QWidget* parent=0);
	virtual ~AuDNAWizardPageBody();

	virtual bool BuildDNA(AuDNA& dna, quint16 headDiaMM) const;

	virtual PAGE PageType() const
	{
		return PAGE_Body;
	}

public slots:
	virtual void CheckOkToProceed();
	virtual void LoadSettings();
	virtual void SaveSettings() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDNAWizardPageLimbs : public AuDNAWizardPage
{
	Q_OBJECT;
	QSpinBox*							m_Arms;
	QCheckBox*							m_ArmsJump;
	QCheckBox*							m_ArmsRun;
	QSpinBox*							m_ArmStandTone;
	QCheckBox*							m_ArmsWalk;
	QSpinBox*							m_Fins;
	QCheckBox*							m_FlyCheck;
	QSpinBox*							m_Legs;
	QSpinBox*							m_LegStandTone;
	QSpinBox*							m_Tentacles;
	QSpinBox*							m_Wings;

public:
	AuDNAWizardPageLimbs(QWidget* parent=0);
	virtual ~AuDNAWizardPageLimbs();

	virtual bool BuildDNA(AuDNA& dna, quint16 headDiaMM) const;

	virtual PAGE PageType() const
	{
		return PAGE_Limbs;
	}

public slots:
	virtual void CheckOkToProceed();
	virtual void LoadSettings();
	virtual void SaveSettings() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDNAWizard : public AuDialog
{
	Q_OBJECT;
	AuDNAWizardPage::PAGE				m_NextPageType;			// Type of page selected by Next button.
	QHash<AuDNAWizardPage::PAGE, AuDNAWizardPage*>	m_PagesByType;		// Pages by page type.
	QList<AuDNAWizardPage*>				m_PageSeq;				// Previous page sequence. Back button disabled when empty.

	// Widgets
	QPushButton*						m_BackButton;
	QPushButton*						m_FinishButton;
	QPushButton*						m_NextButton;
	QStackedLayout*						m_Pages;

public:
	AuDNAWizard(QWidget* parent=0);
	virtual ~AuDNAWizard();

	// Returns an empty constructed AuDNA if the DNA described by the wizard pages is not valid.
	AuDNA DNA() const;

public slots:
	virtual void setVisible(bool visible);
	void BackClicked();
	void DisableBackNextFinish();
	void DisableNext();
	void EnableNext(int nextPageType);	// 'nextPageType' is an AuDNAWizardPage::PAGE enum value.
	void EnableFinish();
	void FinishClicked();
	void NextClicked();

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuDNAWizard_H
