//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audnawidgets.h,v 1.3 2022/02/19 18:19:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDNAWidgets_H
#define AuDNAWidgets_H

#include "audialog.h"
#include "audna.h"
#include <QPushButton>

class AuCheckBoxValue64;
class AuColorButton;
class AuChromosome;
class AuHairColorWidget;
class AuIconButton;
class AuSkinColorWidget;
class AuToolButtonValue;
class AuView;
class QCheckBox;
class QComboBox;
class QListWidget;
class QListWidgetItem;
class QMouseEvent;
class QSpinBox;
class QSplitter;
class QStackedLayout;
class QVBoxLayout;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGeneDialog : public AuDialog
{
	Q_OBJECT;
	bool								m_ApplyToBoth;
	AuGeneChain							m_Genes;				// Genes from the parent trait.
	QMap<AuGene::GTYPE, int>			m_PageIndexes;
	AuChromosome::CTYPE					m_ParentType;			// Type of the parent chromosome.
	AuChromoWhich							m_ParentWhich;

	QPushButton*						m_DelGene;
	QListWidget*						m_GeneList;
	QStackedLayout*						m_Pages;

public:
	AuGeneDialog(AuChromosome::CTYPE parentType, const AuChromoWhich& parentWhich, const AuGeneChain& genes,
			QWidget* parent=0);
	virtual ~AuGeneDialog();

	// Test on accept to see who should receive the result.
	inline bool ApplyToBoth() const
	{
		return m_ApplyToBoth;
	}

	inline const AuGeneChain& Genes() const
	{
		return m_Genes;
	}

public slots:
	void AcceptBoth();
	void AddGene();
	void ApplyGeneChanges();
	void DeleteGene();
	void EnableControls();
	virtual void setVisible(bool visible);
	void GeneClicked(QListWidgetItem* item);

protected:
	void ListGenes();
	void RestoreGeometry();
	void SaveGeometry();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromosomeWidget : public QWidget
{
	Q_OBJECT;
	AuChromosome*							m_AlphaChromosome;
	AuChromosome*							m_BetaChromosome;
	bool								m_Selected;

	// Widgets
	QLabel*								m_AlphaLabel;
	QLabel*								m_BetaLabel;
	QToolButton*						m_Delete;

public:
	AuChromosomeWidget(const AuChromosome* alpha, const AuChromosome* beta, QWidget* parent=0);
	virtual ~AuChromosomeWidget();

	inline const AuChromosome* AlphaChromosome() const
	{
		return m_AlphaChromosome;
	}

	inline AuChromosome* AlphaChromosome()
	{
		return m_AlphaChromosome;
	}

	inline const AuChromosome* BetaChromosome() const
	{
		return m_BetaChromosome;
	}

	inline AuChromosome* BetaChromosome()
	{
		return m_BetaChromosome;
	}

	inline bool IsSelected() const
	{
		return m_Selected;
	}

	inline AuChromosome::CTYPE ChromosomeType() const
	{
		if(!m_AlphaChromosome)
			return AuChromosome::CTYPE_Count;
		return m_AlphaChromosome->Type();
	}

public slots:
	void DeleteChromosome();
	void SetSelected(bool sel);
	void ChromosomesChanged();

signals:
	void Clicked(AuChromosomeWidget* tw);
	void Deleted(AuChromosomeWidget* tw);

protected:
	virtual void mousePressEvent(QMouseEvent* e);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageBase : public QWidget
{
	Q_OBJECT;
public:
	AuChromoPageBase(QWidget* parent=0);
	virtual ~AuChromoPageBase();

	// Returns:
	// 0 = Ok
	// 1 = Ok, alpha chromatid affected
	// 2 = Ok, beta chromatid affected
	// 3 = Ok, both chromatids (whole chromosome) affected
	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta) = 0;

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta) = 0;

signals:
	void Apply();
	void ApplyAll();
	void ApplyPair();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageBlank : public AuChromoPageBase
{
	Q_OBJECT;
public:
	AuChromoPageBlank(QWidget* parent=0);
	virtual ~AuChromoPageBlank();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
	{
		Q_UNUSED(alpha);
		Q_UNUSED(beta);
		return 0;
	}

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
	{
		Q_UNUSED(dna);
		Q_UNUSED(alpha);
		Q_UNUSED(beta);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageAnnelida : public AuChromoPageBase
{
	Q_OBJECT;
	QPushButton*						m_Apply;

public:
	AuChromoPageAnnelida(QWidget* parent=0);
	virtual ~AuChromoPageAnnelida();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

protected slots:
	void EnableControls();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageArthropoda : public AuChromoPageBase
{
	Q_OBJECT;
public:
	AuChromoPageArthropoda(QWidget* parent=0);
	virtual ~AuChromoPageArthropoda();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageChordata : public AuChromoPageBase
{
	Q_OBJECT;
	QPushButton*						m_Apply;

	QComboBox*							m_ASkel;

	QComboBox*							m_BSkel;

public:
	AuChromoPageChordata(QWidget* parent=0);
	virtual ~AuChromoPageChordata();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

protected slots:
	void ASkeletonChanged(int index);
	void BSkeletonChanged(int index);
	void EnableControls();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageMollusca : public AuChromoPageBase
{
	Q_OBJECT;
	QPushButton*						m_Apply;

	QComboBox*							m_ACover;

	QComboBox*							m_BCover;

public:
	AuChromoPageMollusca(QWidget* parent=0);
	virtual ~AuChromoPageMollusca();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

protected slots:
	void ACoverChanged(int index);
	void BCoverChanged(int index);
	void EnableControls();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageSkin : public AuChromoPageBase
{
	Q_OBJECT;
	QPushButton*						m_Apply;

	AuSkinColorWidget*					m_ASkinBottom;
	AuSkinColorWidget*					m_ASkinTop;
	QPushButton*						m_ATopToBottom;

	AuSkinColorWidget*					m_BSkinBottom;
	AuSkinColorWidget*					m_BSkinTop;
	QPushButton*						m_BTopToBottom;

public:
	AuChromoPageSkin(QWidget* parent=0);
	virtual ~AuChromoPageSkin();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

protected slots:
	void ATopToBottomClicked();
	void BTopToBottomClicked();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageFeathers : public AuChromoPageBase
{
	Q_OBJECT;
public:
	AuChromoPageFeathers(QWidget* parent=0);
	virtual ~AuChromoPageFeathers();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageFur : public AuChromoPageBase
{
	Q_OBJECT;
	QPushButton*						m_Apply;

	AuHairColorWidget*					m_AColorBottom;
	AuHairColorWidget*					m_AColorTop;
	QPushButton*						m_ATopToBottom;

	AuHairColorWidget*					m_BColorBottom;
	AuHairColorWidget*					m_BColorTop;
	QPushButton*						m_BTopToBottom;

public:
	AuChromoPageFur(QWidget* parent=0);
	virtual ~AuChromoPageFur();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

protected slots:
	void ATopToBottomClicked();
	void BTopToBottomClicked();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageScales : public AuChromoPageBase
{
	Q_OBJECT;
public:
	AuChromoPageScales(QWidget* parent=0);
	virtual ~AuChromoPageScales();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageHead : public AuChromoPageBase
{
	Q_OBJECT;
	AuGeneChain							m_AGenes;
	AuGeneChain							m_BGenes;

	QPushButton*						m_Apply;

	QComboBox*							m_AAttach;
	QSpinBox*							m_AAttachDist;
	QSpinBox*							m_AEarAng;
	QSpinBox*							m_AEyeAng;
	QSpinBox*							m_AHeight;
	QSpinBox*							m_ALength;
	QSpinBox*							m_AMouthAng;
	QSpinBox*							m_ANoseAng;
	AuIconButton*						m_AGeneButton;
	QSpinBox*							m_AWidth;

	QComboBox*							m_BAttach;
	QSpinBox*							m_BAttachDist;
	QSpinBox*							m_BEarAng;
	QSpinBox*							m_BEyeAng;
	QSpinBox*							m_BHeight;
	QSpinBox*							m_BLength;
	QSpinBox*							m_BMouthAng;
	QSpinBox*							m_BNoseAng;
	AuIconButton*						m_BGeneButton;
	QSpinBox*							m_BWidth;

public:
	AuChromoPageHead(QWidget* parent=0);
	virtual ~AuChromoPageHead();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

public slots:
	void AGenes();
	void BGenes();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageTorso : public AuChromoPageBase
{
	Q_OBJECT;
	AuGeneChain							m_AGenes;
	AuGeneChain							m_BGenes;

	QPushButton*						m_Apply;

	QSpinBox*							m_ALowerLen;
	QSpinBox*							m_ALowerThick;
	QSpinBox*							m_ALowerWid;
	QSpinBox*							m_ANeckLen;
	AuIconButton*						m_AGeneButton;
	QSpinBox*							m_ATailLen;
	QSpinBox*							m_AUpperLen;
	QSpinBox*							m_AUpperThick;
	QSpinBox*							m_AUpperWid;

	QSpinBox*							m_BLowerLen;
	QSpinBox*							m_BLowerThick;
	QSpinBox*							m_BLowerWid;
	QSpinBox*							m_BNeckLen;
	AuIconButton*						m_BGeneButton;
	QSpinBox*							m_BTailLen;
	QSpinBox*							m_BUpperLen;
	QSpinBox*							m_BUpperThick;
	QSpinBox*							m_BUpperWid;

public:
	AuChromoPageTorso(QWidget* parent=0);
	virtual ~AuChromoPageTorso();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

public slots:
	void AGenes();
	void BGenes();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageArm : public AuChromoPageBase
{
	Q_OBJECT;
	AuGeneChain							m_AGenes;
	AuGeneChain							m_BGenes;
	AuChromoWhich							m_Which;

	QPushButton*						m_Apply;
	QPushButton*						m_ApplyAll;
	QPushButton*						m_ApplyPair;

	QCheckBox*							m_AArmJumps;
	QCheckBox*							m_AArmRuns;
	QCheckBox*							m_AArmWalks;
	QSpinBox*							m_AElbowRad;
	QSpinBox*							m_AForeArmLen;
	AuIconButton*						m_AGeneButton;
	QSpinBox*							m_AHandLen;
	QSpinBox*							m_AShoulderRad;
	QSpinBox*							m_AShoulderToSpine;
	QSpinBox*							m_AStandTone;
	QSpinBox*							m_AUpperArmLen;
	QSpinBox*							m_AWristRad;

	QCheckBox*							m_BArmJumps;
	QCheckBox*							m_BArmRuns;
	QCheckBox*							m_BArmWalks;
	QSpinBox*							m_BElbowRad;
	QSpinBox*							m_BForeArmLen;
	AuIconButton*						m_BGeneButton;
	QSpinBox*							m_BHandLen;
	QSpinBox*							m_BShoulderRad;
	QSpinBox*							m_BShoulderToSpine;
	QSpinBox*							m_BStandTone;
	QSpinBox*							m_BUpperArmLen;
	QSpinBox*							m_BWristRad;

public:
	AuChromoPageArm(QWidget* parent=0);
	virtual ~AuChromoPageArm();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

public slots:
	void AGenes();
	void BGenes();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageLeg : public AuChromoPageBase
{
	Q_OBJECT;
	AuGeneChain							m_AGenes;
	AuGeneChain							m_BGenes;
	AuChromoWhich							m_Which;

	QPushButton*						m_Apply;
	QPushButton*						m_ApplyAll;
	QPushButton*						m_ApplyPair;

	QSpinBox*							m_AAnkleLen;
	QSpinBox*							m_AAnkleRad;
	QSpinBox*							m_AButtRad;
	QSpinBox*							m_ACalfLen;
	QSpinBox*							m_AFootLen;
	QSpinBox*							m_AFootWid;
	QSpinBox*							m_AHipRad;
	QSpinBox*							m_AHipToSpine;
	QSpinBox*							m_AKneeRad;
	AuIconButton*						m_AGeneButton;
	QSpinBox*							m_AStandTone;
	QSpinBox*							m_AThighLen;

	QSpinBox*							m_BAnkleLen;
	QSpinBox*							m_BAnkleRad;
	QSpinBox*							m_BButtRad;
	QSpinBox*							m_BCalfLen;
	QSpinBox*							m_BFootLen;
	QSpinBox*							m_BFootWid;
	QSpinBox*							m_BHipRad;
	QSpinBox*							m_BHipToSpine;
	QSpinBox*							m_BKneeRad;
	AuIconButton*						m_BGeneButton;
	QSpinBox*							m_BStandTone;
	QSpinBox*							m_BThighLen;

public:
	AuChromoPageLeg(QWidget* parent=0);
	virtual ~AuChromoPageLeg();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

public slots:
	void AGenes();
	void BGenes();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageFin : public AuChromoPageBase
{
	Q_OBJECT;
	AuGeneChain							m_AGenes;
	AuGeneChain							m_BGenes;
	AuChromoWhich							m_Which;

public:
	AuChromoPageFin(QWidget* parent=0);
	virtual ~AuChromoPageFin();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

public slots:
	void AGenes();
	void BGenes();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageTentacle : public AuChromoPageBase
{
	Q_OBJECT;
	AuGeneChain							m_AGenes;
	AuGeneChain							m_BGenes;
	AuChromoWhich							m_Which;

public:
	AuChromoPageTentacle(QWidget* parent=0);
	virtual ~AuChromoPageTentacle();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

public slots:
	void AGenes();
	void BGenes();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageWing : public AuChromoPageBase
{
	Q_OBJECT;
	AuGeneChain							m_AGenes;
	AuGeneChain							m_BGenes;
	AuChromoWhich							m_Which;

public:
	AuChromoPageWing(QWidget* parent=0);
	virtual ~AuChromoPageWing();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

public slots:
	void AGenes();
	void BGenes();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageAntenna : public AuChromoPageBase
{
	Q_OBJECT;
	AuChromoWhich							m_Which;

public:
	AuChromoPageAntenna(QWidget* parent=0);
	virtual ~AuChromoPageAntenna();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageHorn : public AuChromoPageBase
{
	Q_OBJECT;
	AuChromoWhich							m_Which;

public:
	AuChromoPageHorn(QWidget* parent=0);
	virtual ~AuChromoPageHorn();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageEar : public AuChromoPageBase
{
	Q_OBJECT;
	AuGeneChain							m_AGenes;
	AuGeneChain							m_BGenes;
	QPushButton*						m_Apply;
	QPushButton*						m_ApplyAll;
	QPushButton*						m_ApplyPair;
	AuChromoWhich							m_Which;

	QSpinBox*							m_AAng;
	QSpinBox*							m_ADia;
	QComboBox*							m_AStyle;
	AuIconButton*						m_AGeneButton;

	QSpinBox*							m_BAng;
	QSpinBox*							m_BDia;
	QComboBox*							m_BStyle;
	AuIconButton*						m_BGeneButton;

public:
	AuChromoPageEar(QWidget* parent=0);
	virtual ~AuChromoPageEar();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

public slots:
	void AGenes();
	void BGenes();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoEyeDecalDialog : public AuDialog
{
	Q_OBJECT;
	MEAT_DECAL							m_EyeDecal;
	QHash<QLabel*, MEAT_DECAL>			m_EyeDecals;

public:
	AuChromoEyeDecalDialog(MEAT_DECAL eyeDecal, QWidget* parent=0);
	virtual ~AuChromoEyeDecalDialog();

	MEAT_DECAL EyeDecal() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	virtual void mousePressEvent(QMouseEvent* e);
	void RestoreGeometry();
	void SaveGeometry();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoEyeDecalButton : public QPushButton
{
	Q_OBJECT;
	MEAT_DECAL							m_EyeDecal;

public:
	AuChromoEyeDecalButton(QWidget* parent=0);
	virtual ~AuChromoEyeDecalButton();

	inline MEAT_DECAL EyeDecal() const
	{
		return m_EyeDecal;
	}

	void SetEyeDecal(MEAT_DECAL eyeDecal);

protected:
	virtual void paintEvent(QPaintEvent* pe);

protected slots:
	void PickEyeDecal();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageEye : public AuChromoPageBase
{
	Q_OBJECT;
	AuGeneChain							m_AGenes;
	AuGeneChain							m_BGenes;
	QPushButton*						m_Apply;
	QPushButton*						m_ApplyAll;
	QPushButton*						m_ApplyPair;
	AuChromoWhich							m_Which;

	QSpinBox*							m_ABall;
	AuChromoEyeDecalButton*				m_AEye;
	QComboBox*							m_AFacing;
	QSpinBox*							m_ASock;
	AuIconButton*						m_AGeneButton;

	QSpinBox*							m_BBall;
	AuChromoEyeDecalButton*				m_BEye;
	QComboBox*							m_BFacing;
	QSpinBox*							m_BSock;
	AuIconButton*						m_BGeneButton;

public:
	AuChromoPageEye(QWidget* parent=0);
	virtual ~AuChromoPageEye();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

public slots:
	void AGenes();
	void BGenes();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageNose : public AuChromoPageBase
{
	Q_OBJECT;
	AuGeneChain							m_AGenes;
	AuGeneChain							m_BGenes;
	QPushButton*						m_Apply;

	QSpinBox*							m_AAng;
	QSpinBox*							m_ALen;
	AuIconButton*						m_AGeneButton;
	QSpinBox*							m_AWidHead;
	QSpinBox*							m_AWidTip;

	QSpinBox*							m_BAng;
	QSpinBox*							m_BLen;
	AuIconButton*						m_BGeneButton;
	QSpinBox*							m_BWidHead;
	QSpinBox*							m_BWidTip;

public:
	AuChromoPageNose(QWidget* parent=0);
	virtual ~AuChromoPageNose();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

public slots:
	void AGenes();
	void BGenes();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageMouth : public AuChromoPageBase
{
	Q_OBJECT;
	AuGeneChain							m_AGenes;
	AuGeneChain							m_BGenes;
	QPushButton*						m_Apply;

	QSpinBox*							m_ADep;
	QSpinBox*							m_AHt;
	QSpinBox*							m_ALen;
	QSpinBox*							m_ALipThick;
	AuIconButton*						m_AGeneButton;
	QComboBox*							m_ATeeth;
	QSpinBox*							m_ATeethHtBot;
	QSpinBox*							m_ATeethHtTop;
	QSpinBox*							m_AWid;

	QSpinBox*							m_BDep;
	QSpinBox*							m_BHt;
	QSpinBox*							m_BLen;
	QSpinBox*							m_BLipThick;
	AuIconButton*						m_BGeneButton;
	QComboBox*							m_BTeeth;
	QSpinBox*							m_BTeethHtBot;
	QSpinBox*							m_BTeethHtTop;
	QSpinBox*							m_BWid;

public:
	AuChromoPageMouth(QWidget* parent=0);
	virtual ~AuChromoPageMouth();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

public slots:
	void AGenes();
	void BGenes();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageLungs : public AuChromoPageBase
{
	Q_OBJECT;
	QPushButton*						m_Apply;

	QComboBox*							m_AAtmo;
	QComboBox*							m_BAtmo;

public:
	AuChromoPageLungs(QWidget* parent=0);
	virtual ~AuChromoPageLungs();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

protected slots:
	void AAtmosphereChanged(int index);
	void BAtmosphereChanged(int index);
	void EnableControls();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageGills : public AuChromoPageBase
{
	Q_OBJECT;
public:
	AuChromoPageGills(QWidget* parent=0);
	virtual ~AuChromoPageGills();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageBlood : public AuChromoPageBase
{
	Q_OBJECT;
	QPushButton*						m_Apply;

	QComboBox*							m_AType;

	QComboBox*							m_BType;

public:
	AuChromoPageBlood(QWidget* parent=0);
	virtual ~AuChromoPageBlood();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

protected slots:
	void ATypeChanged(int index);
	void BTypeChanged(int index);
	void EnableControls();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageReproduction : public AuChromoPageBase
{
	Q_OBJECT;
	QPushButton*						m_Apply;

	QComboBox*							m_ABirthPlace;
	QCheckBox*							m_AGenFemale;
	QCheckBox*							m_AGenMale;
	QLabel*								m_ALitterLabel;
	QSpinBox*							m_ALitterMax;
	QSpinBox*							m_ALitterMin;
	QComboBox*							m_AYoung;

	QComboBox*							m_BBirthPlace;
	QCheckBox*							m_BGenFemale;
	QCheckBox*							m_BGenMale;
	QLabel*								m_BLitterLabel;
	QSpinBox*							m_BLitterMax;
	QSpinBox*							m_BLitterMin;
	QComboBox*							m_BYoung;

public:
	AuChromoPageReproduction(QWidget* parent=0);
	virtual ~AuChromoPageReproduction();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);

protected slots:
	void AGenFemaleChanged(int value);
	void AGenMaleChanged(int value);
	void ALitterMinChanged(int value);
	void ALitterMaxChanged(int value);
	void BGenFemaleChanged(int value);
	void BGenMaleChanged(int value);
	void BLitterMinChanged(int value);
	void BLitterMaxChanged(int value);
	void EnableControls();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageMammary : public AuChromoPageBase
{
	Q_OBJECT;
	QPushButton*						m_Apply;
	QPushButton*						m_ApplyAll;
	QPushButton*						m_ApplyPair;

	QSpinBox*							m_ADia;
	QCheckBox*							m_AFeed;
	QCheckBox*							m_AHide;
	QComboBox*							m_AOrgan;

	QSpinBox*							m_BDia;
	QCheckBox*							m_BFeed;
	QCheckBox*							m_BHide;
	QComboBox*							m_BOrgan;

public:
	AuChromoPageMammary(QWidget* parent=0);
	virtual ~AuChromoPageMammary();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageEatsPlants : public AuChromoPageBase
{
	Q_OBJECT;
	QPushButton*						m_Apply;

	QComboBox*							m_APref;

	QComboBox*							m_BPref;

public:
	AuChromoPageEatsPlants(QWidget* parent=0);
	virtual ~AuChromoPageEatsPlants();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoPageEatsAnimals : public AuChromoPageBase
{
	Q_OBJECT;
	QPushButton*						m_Apply;

	QCheckBox*							m_ABloodSucker;
	QCheckBox*							m_ABrain;
	QCheckBox*							m_ACannibal;
	QCheckBox*							m_ACarne;
	QCheckBox*							m_ACarrion;
	QCheckBox*							m_AOrgans;
	QCheckBox*							m_APredator;

	QCheckBox*							m_BBloodSucker;
	QCheckBox*							m_BBrain;
	QCheckBox*							m_BCannibal;
	QCheckBox*							m_BCarne;
	QCheckBox*							m_BCarrion;
	QCheckBox*							m_BOrgans;
	QCheckBox*							m_BPredator;

public:
	AuChromoPageEatsAnimals(QWidget* parent=0);
	virtual ~AuChromoPageEatsAnimals();

	virtual int ApplyProperties(AuChromosome* alpha, AuChromosome* beta);

	virtual void ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Gene property pages.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGenePageBase : public QWidget
{
	Q_OBJECT;
public:
	AuGenePageBase(QWidget* parent=0);
	virtual ~AuGenePageBase();

	// Returns true if anything changes that would change the ListWidgetText of the gene.
	virtual bool ApplyProperties(AuGene* gene) = 0;

	virtual void ShowProperties(AuChromosome::CTYPE parentType, const AuChromoWhich& parentWhich, const AuGene* gene) = 0;

signals:
	void Apply();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGenePageBlank : public AuGenePageBase
{
	Q_OBJECT;
public:
	AuGenePageBlank(QWidget* parent=0);
	virtual ~AuGenePageBlank();

	virtual bool ApplyProperties(AuGene* gene)
	{
		Q_UNUSED(gene);
		return false;
	}

	virtual void ShowProperties(AuChromosome::CTYPE parentType, const AuChromoWhich& parentWhich, const AuGene* gene)
	{
		Q_UNUSED(parentType);
		Q_UNUSED(parentWhich);
		Q_UNUSED(gene);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGenePageFeathers : public AuGenePageBase
{
	Q_OBJECT;
public:
	AuGenePageFeathers(QWidget* parent=0);
	virtual ~AuGenePageFeathers();

	virtual bool ApplyProperties(AuGene* gene);

	virtual void ShowProperties(AuChromosome::CTYPE parentType, const AuChromoWhich& parentWhich, const AuGene* gene);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGenePageHair : public AuGenePageBase
{
	Q_OBJECT;
	AuHairColorWidget*					m_Color;
	QSpinBox*							m_Length;
	QList<AuCheckBoxValue64*>			m_WhereChecks;

public:
	AuGenePageHair(QWidget* parent=0);
	virtual ~AuGenePageHair();

	virtual bool ApplyProperties(AuGene* gene);

	virtual void ShowProperties(AuChromosome::CTYPE parentType, const AuChromoWhich& parentWhich, const AuGene* gene);

public slots:
	void HairStateChanged(int state, qint64 value);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGenePageScales : public AuGenePageBase
{
	Q_OBJECT;
public:
	AuGenePageScales(QWidget* parent=0);
	virtual ~AuGenePageScales();

	virtual bool ApplyProperties(AuGene* gene);

	virtual void ShowProperties(AuChromosome::CTYPE parentType, const AuChromoWhich& parentWhich, const AuGene* gene);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGenePageSpines : public AuGenePageBase
{
	Q_OBJECT;
public:
	AuGenePageSpines(QWidget* parent=0);
	virtual ~AuGenePageSpines();

	virtual bool ApplyProperties(AuGene* gene);

	virtual void ShowProperties(AuChromosome::CTYPE parentType, const AuChromoWhich& parentWhich, const AuGene* gene);
};

#endif // AuDNAWidgets_H
