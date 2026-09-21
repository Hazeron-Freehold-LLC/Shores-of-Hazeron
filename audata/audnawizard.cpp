//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audnawizard.cpp,v 1.13 2022/02/19 18:20:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audnawizard.h"
//
#include "auglobal.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStackedLayout>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDNAWizardPage::AuDNAWizardPage(QWidget* parent) : QWidget(parent)
{
}

AuDNAWizardPage::~AuDNAWizardPage()
{
}

void AuDNAWizardPage::setVisible(bool visible)
{
	QWidget::setVisible(visible);
	if(visible)
		CheckOkToProceed();
	else
		SaveSettings();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDNAWizardPageIntro::AuDNAWizardPageIntro(QWidget* parent) : AuDNAWizardPage(parent)
{
	QLabel* lbicon = new QLabel(this);
	lbicon->setPixmap(QPixmap(":data/DNA.png").scaled(128,128));
	lbicon->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<b>Animalia</b>"
				"<p>"
				"The kingdom <i>Animalia</i>, or <i>Metazoa</i>, includes all animals. "
				"Animals are multicellular, eukaryotic organisms, which are heterotrophic, "
				"meaning they obtain nutrition from organic sources. "
				"Most animals obtain nutrition by ingesting other organisms or decomposing organic material. "
				"</p>"
				"<p>"
				"A <b>eu %1 kar %1 y %1 ote</b> is an organism consisting of a cell or cells in which the genetic "
				"material is DNA in the form of chromosomes contained within a distinct nucleus. "
				"Eukaryotes include all living organisms other than the eubacteria and archaebacteria. "
				"</p>"
				"<p>"
				"<i>%2</i> simulates DNA to generate all animals in the game universe. "
				"Compact data objects serve as the chromosomes. "
				"These chromosomes exist to build a model of the animal and to use it in the game, "
				"not to accurately simulate terrestrial biology. "
				"</p>"
				"<p>"
				"All animals in <i>%2</i> are <b>diploid</b>, "
				"meaning they have two copies of all genetic information for most of their life cycle. "
				"</p>"
				"<p>"
				"Press <i>Next</i> to begin building DNA for the general form of an animal. "
				"Every detail of the DNA can be adjusted after the general form is built. "
				"</p>"
				"</html>").arg(AuGlobal::MiddleDotSymbol()).arg(g_Au.ApplicationName()), this);
	lbinfo->setWordWrap(true);

	QVBoxLayout* wlay = new QVBoxLayout;
	wlay->addWidget(lbinfo);
	wlay->addStretch();

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(lbicon);
	lay->addLayout(wlay, 1);
	setLayout(lay);
}

AuDNAWizardPageIntro::~AuDNAWizardPageIntro()
{
}

void AuDNAWizardPageIntro::CheckOkToProceed()
{
	//qDebug() << "AuDNAWizardPageIntro::CheckOkToProceed";
	emit OkToProceed(PAGE_Phylum);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDNAWizardPageFinish::AuDNAWizardPageFinish(QWidget* parent) : AuDNAWizardPage(parent)
{
	QLabel* lbicon = new QLabel(this);
	lbicon->setPixmap(QPixmap(":data/DNA.png").scaled(128,128));
	lbicon->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<b>Done</b>"
				"<p>"
				"That's all. "
				"Go back to verify choices. "
				"Then, press <i>Finish</i> to proceed. "
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	QVBoxLayout* wlay = new QVBoxLayout;
	wlay->addWidget(lbinfo);
	wlay->addStretch();

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(lbicon);
	lay->addLayout(wlay, 1);
	setLayout(lay);
}

AuDNAWizardPageFinish::~AuDNAWizardPageFinish()
{
}

void AuDNAWizardPageFinish::CheckOkToProceed()
{
	//qDebug() << "AuDNAWizardPageFinish::CheckOkToProceed";
	emit OkToFinish();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDNAWizardPagePhylum::AuDNAWizardPagePhylum(QWidget* parent) : AuDNAWizardPage(parent)
{
	QLabel* lbicon = new QLabel(this);
	lbicon->setPixmap(QPixmap(":data/Chromosome.png").scaled(128,128));
	lbicon->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<b>Phylum</b>"
				"<p>"
				"<i>%1</i> simulates four terrestrial phylums within the kingdom of animalia. "
				"Further classification into <i>class</i>, <i>order</i>, <i>family</i>, <i>genus</i> and <i>species</i> "
				"is left to the observer. "
				"</p>"
				"</html>").arg(g_Au.ApplicationName()), this);
	lbinfo->setWordWrap(true);

	m_AnnelRadio = new QRadioButton(tr("Annelida"), this);
	connect(m_AnnelRadio, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* annelLabel = new QLabel(tr("<html>"
				"<p>"
				"Invertebrate animals having a segmented body. "
				"</p>"
				"<p>"
				"Example: leeches, worms"
				"</p>"
				"</html>"), this);
	annelLabel->setWordWrap(true);

	m_MollRadio = new QRadioButton(tr("Mollusca"), this);
	connect(m_MollRadio, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* mollLabel = new QLabel(tr("<html>"
				"<p>"
				"Invertebrate animals having a soft body covered by a shell or mantle. "
				"</p>"
				"<p>"
				"Example: slugs, snails"
				"</p>"
				"</html>"), this);
	mollLabel->setWordWrap(true);

	m_MollCombo = new QComboBox(this);
	m_MollCombo->addItem(AuChromoMollusca::CoverName(AuChromoMollusca::Mantle), AuChromoMollusca::Mantle);
	m_MollCombo->addItem(AuChromoMollusca::CoverName(AuChromoMollusca::Shell), AuChromoMollusca::Shell);
	m_MollCombo->setCurrentIndex(-1);
	connect(m_MollCombo, SIGNAL(currentIndexChanged(int)), SLOT(CheckOkToProceed()));

	m_ChorRadio = new QRadioButton(tr("Chordata"), this);
	connect(m_ChorRadio, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* chorLabel = new QLabel(tr("<html>"
				"<p>"
				"Animals having a dorsal nervous cord. "
				"</p>"
				"<p>"
				"Example: fish, birds, amphibians, reptiles, mammals"
				"</p>"
				"</html>"), this);
	chorLabel->setWordWrap(true);

	m_ChorCombo = new QComboBox(this);
	m_ChorCombo->addItem(AuChromoChordata::SkeletonName(AuChromoChordata::Bones), AuChromoChordata::Bones);
	m_ChorCombo->addItem(AuChromoChordata::SkeletonName(AuChromoChordata::Cartilage), AuChromoChordata::Cartilage);
	m_ChorCombo->setCurrentIndex(-1);
	connect(m_ChorCombo, SIGNAL(currentIndexChanged(int)), SLOT(CheckOkToProceed()));

	m_ArthRadio = new QRadioButton(tr("Arthropoda"), this);
	connect(m_ArthRadio, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* arthLabel = new QLabel(tr("<html>"
				"<p>"
				"Animals that have jointed legs. "
				"</p>"
				"<p>"
				"Example: insects, spiders, crabs"
				"</p>"
				"</html>"), this);
	arthLabel->setWordWrap(true);

	QGridLayout* molllay = new QGridLayout;
	int row=0;
	molllay->addWidget(m_MollCombo, row++,1);
	molllay->setColumnMinimumWidth(0, 20);

	QGridLayout* chorlay = new QGridLayout;
	row=0;
	chorlay->addWidget(m_ChorCombo, row++,1);
	chorlay->setColumnMinimumWidth(0, 20);

	QVBoxLayout* wlay = new QVBoxLayout;
	wlay->addWidget(lbinfo);
	wlay->addWidget(m_AnnelRadio);
	wlay->addWidget(annelLabel);
	wlay->addWidget(m_ArthRadio);
	wlay->addWidget(arthLabel);
	wlay->addWidget(m_ChorRadio);
	wlay->addWidget(chorLabel);
	wlay->addLayout(chorlay);
	wlay->addWidget(m_MollRadio);
	wlay->addWidget(mollLabel);
	wlay->addLayout(molllay);
	wlay->addStretch();

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(lbicon);
	lay->addLayout(wlay, 1);
	setLayout(lay);

	LoadSettings();
}

AuDNAWizardPagePhylum::~AuDNAWizardPagePhylum()
{
}

void AuDNAWizardPagePhylum::CheckOkToProceed()
{
	bool chor = m_ChorRadio->isChecked();
	bool moll = m_MollRadio->isChecked();
	m_ChorCombo->setEnabled(chor);
	m_MollCombo->setEnabled(moll);

	if(chor)
	{
		if(m_ChorCombo->currentIndex() < 0)
			emit NotOkToProceed();
		else
			emit OkToProceed(PAGE_Hide);
	}
	else if(moll)
	{
		if(m_MollCombo->currentIndex() < 0)
			emit NotOkToProceed();
		else
			emit OkToProceed(PAGE_Blood);
	}
	else if(m_AnnelRadio->isChecked())
		emit OkToProceed(PAGE_Blood);
	else if(m_ArthRadio->isChecked())
		emit OkToProceed(PAGE_Blood);
}

bool AuDNAWizardPagePhylum::BuildDNA(AuDNA& dna, quint16 headDiaMM) const
{
	Q_UNUSED(headDiaMM);
	if(m_ChorRadio->isChecked())
	{
		if(m_ChorCombo->currentIndex() >= 0)
		{
			dna.AddChromosome(AuChromoChordata((AuChromoChordata::SKELETON)m_ChorCombo->currentData().toInt()));
			return true;
		}
	}
	else if(m_MollRadio->isChecked())
	{
		if(m_MollCombo->currentIndex() >= 0)
		{
			dna.AddChromosome(AuChromoMollusca((AuChromoMollusca::COVER)m_MollCombo->currentData().toInt()));
			return true;
		}
	}
	else if(m_AnnelRadio->isChecked())
	{
		dna.AddChromosome(AuChromoAnnelida());
		return true;
	}
	else if(m_ArthRadio->isChecked())
	{
		dna.AddChromosome(AuChromoArthropoda());
		return true;
	}
	return false;
}

void AuDNAWizardPagePhylum::SaveSettings() const
{
	int phylum;
	if(m_AnnelRadio->isChecked())
		phylum = AuChromoAnnelida::ChromosomeType();
	else if(m_ArthRadio->isChecked())
		phylum = AuChromoArthropoda::ChromosomeType();
	else if(m_ChorRadio->isChecked())
		phylum = AuChromoChordata::ChromosomeType();
	else if(m_MollRadio->isChecked())
		phylum = AuChromoMollusca::ChromosomeType();
	else
		phylum = -1;

	g_Au.Settings().writeEntry("DNAWizardPhylum/Phylum", phylum);
	if(m_ChorCombo->currentIndex() >= 0)
		g_Au.Settings().writeEntry("DNAWizardPhylum/ChordataSkeleton", m_ChorCombo->currentData().toInt());
	else
		g_Au.Settings().writeEntry("DNAWizardPhylum/ChordataSkeleton", -1);
	if(m_MollCombo->currentIndex() >= 0)
		g_Au.Settings().writeEntry("DNAWizardPhylum/MolluscaCover", m_MollCombo->currentData().toInt());
	else
		g_Au.Settings().writeEntry("DNAWizardPhylum/MolluscaCover", -1);
}

void AuDNAWizardPagePhylum::LoadSettings()
{
	int lastPhylum = g_Au.Settings().readNumEntry("DNAWizardPhylum/Phylum", -1);
	int lastChor = g_Au.Settings().readNumEntry("DNAWizardPhylum/ChordataSkeleton", -1);
	int lastMoll = g_Au.Settings().readNumEntry("DNAWizardPhylum/MolluscaCover", -1);
	switch(lastPhylum)
	{
	case AuChromosome::Annelida:
		m_AnnelRadio->setChecked(true);
		break;
	case AuChromosome::Arthropoda:
		m_ArthRadio->setChecked(true);
		break;
	case AuChromosome::Chordata:
		m_ChorRadio->setChecked(true);
		m_ChorCombo->setCurrentIndex(m_ChorCombo->findData(lastChor));
		break;
	case AuChromosome::Mollusca:
		m_MollRadio->setChecked(true);
		m_MollCombo->setCurrentIndex(m_MollCombo->findData(lastMoll));
		break;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDNAWizardPageHide::AuDNAWizardPageHide(QWidget* parent) : AuDNAWizardPage(parent)
{
	QLabel* lbicon = new QLabel(this);
	lbicon->setPixmap(QPixmap(":data/Chromosome.png").scaled(128,128));
	lbicon->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<b>Hide</b>"
				"<p>"
				"Hide describes the outer sheath covering the skeleton and organs of the body. "
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	m_FeathersRadio = new QRadioButton(tr("Feathers"), this);
	connect(m_FeathersRadio, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* feathersLabel = new QLabel(tr("<html>"
				"<p>"
				"Hide is covered in feathers. "
				"</p>"
				"<p>"
				"Example: birds"
				"</p>"
				"</html>"), this);
	feathersLabel->setWordWrap(true);

	m_FurRadio = new QRadioButton(tr("Fur"), this);
	connect(m_FurRadio, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* furLabel = new QLabel(tr("<html>"
				"<p>"
				"Hide is covered in fur. "
				"</p>"
				"<p>"
				"Example: dogs, cats, cows, horses"
				"</p>"
				"</html>"), this);
	furLabel->setWordWrap(true);

	m_ScalesRadio = new QRadioButton(tr("Scales"), this);
	connect(m_ScalesRadio, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* scalesLabel = new QLabel(tr("<html>"
				"<p>"
				"Hide is covered in scales. "
				"</p>"
				"<p>"
				"Example: fish, reptiles"
				"</p>"
				"</html>"), this);
	scalesLabel->setWordWrap(true);

	m_SkinRadio = new QRadioButton(tr("Skin"), this);
	connect(m_SkinRadio, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* skinLabel = new QLabel(tr("<html>"
				"<p>"
				"Hide is covered in skin. "
				"</p>"
				"<p>"
				"Example: humans, amphibians"
				"</p>"
				"</html>"), this);
	skinLabel->setWordWrap(true);

	QVBoxLayout* wlay = new QVBoxLayout;
	wlay->addWidget(lbinfo);
	wlay->addWidget(m_FeathersRadio);
	wlay->addWidget(feathersLabel);
	wlay->addWidget(m_FurRadio);
	wlay->addWidget(furLabel);
	wlay->addWidget(m_ScalesRadio);
	wlay->addWidget(scalesLabel);
	wlay->addWidget(m_SkinRadio);
	wlay->addWidget(skinLabel);
	wlay->addStretch();

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(lbicon);
	lay->addLayout(wlay, 1);
	setLayout(lay);

	LoadSettings();
}

AuDNAWizardPageHide::~AuDNAWizardPageHide()
{
}

void AuDNAWizardPageHide::CheckOkToProceed()
{
	if(!m_FurRadio->isChecked() &&
			!m_FeathersRadio->isChecked() &&
			!m_ScalesRadio->isChecked() &&
			!m_SkinRadio->isChecked())
		return;

	emit OkToProceed(PAGE_Blood);
}

bool AuDNAWizardPageHide::BuildDNA(AuDNA& dna, quint16 headDiaMM) const
{
	Q_UNUSED(headDiaMM);
	switch(dna.Phylum())
	{
	case AuChromosome::Annelida:
		break;	//DHH Not implemented
	case AuChromosome::Arthropoda:
		break;	//DHH Not implemented
	case AuChromosome::Chordata:
		if(m_FeathersRadio->isChecked())
		{
			dna.AddChromosome(AuChromoFeathers());
			return true;
		}
		else if(m_FurRadio->isChecked())
		{
			dna.AddChromosome(AuChromoFur());
			return true;
		}
		else if(m_ScalesRadio->isChecked())
		{
			dna.AddChromosome(AuChromoScales());
			return true;
		}
		else if(m_SkinRadio->isChecked())
		{
			dna.AddChromosome(AuChromoSkin());
			return true;
		}
		break;
	case AuChromosome::Mollusca:
		break;	//DHH Not implemented
	default:
		break;
	}
	return false;
}

void AuDNAWizardPageHide::SaveSettings() const
{
	int hide;
	if(m_FeathersRadio->isChecked())
		hide = AuChromoFeathers::ChromosomeType();
	else if(m_FurRadio->isChecked())
		hide = AuChromoFur::ChromosomeType();
	else if(m_ScalesRadio->isChecked())
		hide = AuChromoScales::ChromosomeType();
	else if(m_SkinRadio->isChecked())
		hide = AuChromoSkin::ChromosomeType();
	else
		hide = -1;

	g_Au.Settings().writeEntry("DNAWizardHide/Hide", hide);
}

void AuDNAWizardPageHide::LoadSettings()
{
	int lastHide = g_Au.Settings().readNumEntry("DNAWizardHide/Hide", -1);
	switch(lastHide)
	{
	case AuChromosome::Feathers:
		m_FeathersRadio->setChecked(true);
		break;
	case AuChromosome::Fur:
		m_FurRadio->setChecked(true);
		break;
	case AuChromosome::Scales:
		m_ScalesRadio->setChecked(true);
		break;
	case AuChromosome::Skin:
		m_SkinRadio->setChecked(true);
		break;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDNAWizardPageBlood::AuDNAWizardPageBlood(QWidget* parent) : AuDNAWizardPage(parent)
{
	QLabel* lbicon = new QLabel(this);
	lbicon->setPixmap(QPixmap(":data/Chromosome.png").scaled(128,128));
	lbicon->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<b>Blood</b>"
				"<p>"
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	m_WarmRadio = new QRadioButton(tr("Warm Blooded"), this);
	connect(m_WarmRadio, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* warmLabel = new QLabel(tr("<html>"
				"<p>"
				"Warm blooded animals can make their own body heat even when it is cold outside. "
				"</p>"
				"<p>"
				"Example: mammals, birds"
				"</p>"
				"</html>"), this);
	warmLabel->setWordWrap(true);

	m_ColdRadio = new QRadioButton(tr("Cold Blooded"), this);
	connect(m_ColdRadio, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* coldLabel = new QLabel(tr("<html>"
				"<p>"
				"Cold-blooded animals become hotter and colder, depending on the the temperature outside. "
				"</p>"
				"<p>"
				"Example: fish, reptiles, amphibians"
				"</p>"
				"</html>"), this);
	coldLabel->setWordWrap(true);

	QVBoxLayout* wlay = new QVBoxLayout;
	wlay->addWidget(lbinfo);
	wlay->addWidget(m_WarmRadio);
	wlay->addWidget(warmLabel);
	wlay->addWidget(m_ColdRadio);
	wlay->addWidget(coldLabel);
	wlay->addStretch();

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(lbicon);
	lay->addLayout(wlay, 1);
	setLayout(lay);

	LoadSettings();
}

AuDNAWizardPageBlood::~AuDNAWizardPageBlood()
{
}

void AuDNAWizardPageBlood::CheckOkToProceed()
{
	if(!m_WarmRadio->isChecked() &&
			!m_ColdRadio->isChecked())
		return;

	emit OkToProceed(PAGE_Breath);
}

bool AuDNAWizardPageBlood::BuildDNA(AuDNA& dna, quint16 headDiaMM) const
{
	Q_UNUSED(headDiaMM);
	if(m_WarmRadio->isChecked())
	{
		dna.AddChromosome(AuChromoBlood(AuChromoBlood::Warm));
		return true;
	}
	else if(m_ColdRadio->isChecked())
	{
		dna.AddChromosome(AuChromoBlood(AuChromoBlood::Cold));
		return true;
	}
	return false;
}

void AuDNAWizardPageBlood::SaveSettings() const
{
	int blood;
	if(m_WarmRadio->isChecked())
		blood = AuChromoBlood::Warm;
	else if(m_ColdRadio->isChecked())
		blood = AuChromoBlood::Cold;
	else
		blood = -1;

	g_Au.Settings().writeEntry("DNAWizardBlood/Blood", blood);
}

void AuDNAWizardPageBlood::LoadSettings()
{
	int lastBlood = g_Au.Settings().readNumEntry("DNAWizardBlood/Blood", -1);
	switch(lastBlood)
	{
	case AuChromoBlood::Cold:
		m_ColdRadio->setChecked(true);
		break;
	case AuChromoBlood::Warm:
		m_WarmRadio->setChecked(true);
		break;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDNAWizardPageBreath::AuDNAWizardPageBreath(QWidget* parent) : AuDNAWizardPage(parent)
{
	QLabel* lbicon = new QLabel(this);
	lbicon->setPixmap(QPixmap(":data/Chromosome.png").scaled(128,128));
	lbicon->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<b>Breath</b>"
				"<p>"
				"Oxygen is essential to many chemical processes of life. "
				"Animals collect oxygen from the environment. "
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	m_GillsCheck = new QCheckBox(tr("Gills"), this);
	connect(m_GillsCheck, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* gillsLabel = new QLabel(tr("<html>"
				"<p>"
				"Gills absorb oxygen from a liquid environment. "
				"</p>"
				"<p>"
				"Example: fish"
				"</p>"
				"</html>"), this);
	gillsLabel->setWordWrap(true);

	m_LungsCheck = new QCheckBox(tr("Lungs"), this);
	connect(m_LungsCheck, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* lungsLabel = new QLabel(tr("<html>"
				"<p>"
				"Lungs absorb oxygen from the atmosphere. "
				"</p>"
				"<p>"
				"Example: mammals, reptiles, amphibians"
				"</p>"
				"</html>"), this);
	lungsLabel->setWordWrap(true);

	QVBoxLayout* wlay = new QVBoxLayout;
	wlay->addWidget(lbinfo);
	wlay->addWidget(m_GillsCheck);
	wlay->addWidget(gillsLabel);
	wlay->addWidget(m_LungsCheck);
	wlay->addWidget(lungsLabel);
	wlay->addStretch();

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(lbicon);
	lay->addLayout(wlay, 1);
	setLayout(lay);

	LoadSettings();
}

AuDNAWizardPageBreath::~AuDNAWizardPageBreath()
{
}

void AuDNAWizardPageBreath::CheckOkToProceed()
{
	bool ok = true;
	if(!m_GillsCheck->isChecked() &&
			!m_LungsCheck->isChecked())
		ok = false;

	if(ok)
		emit OkToProceed(PAGE_Eat);
	else
		emit NotOkToProceed();
}

bool AuDNAWizardPageBreath::BuildDNA(AuDNA& dna, quint16 headDiaMM) const
{
	Q_UNUSED(headDiaMM);
	if(!m_GillsCheck->isChecked() &&
			!m_LungsCheck->isChecked())
		return false;	// This has already been checked but I can't not check. :/

	if(m_GillsCheck->isChecked())
		dna.AddChromosome(AuChromoGills());

	if(m_LungsCheck->isChecked())
		dna.AddChromosome(AuChromoLungs());

	return true;
}

void AuDNAWizardPageBreath::SaveSettings() const
{
	bool gills = m_GillsCheck->isChecked();
	bool lungs = m_LungsCheck->isChecked();

	g_Au.Settings().writeEntry("DNAWizardBreath/Gills", gills);
	g_Au.Settings().writeEntry("DNAWizardBreath/Lungs", lungs);
}

void AuDNAWizardPageBreath::LoadSettings()
{
	bool lastGills = g_Au.Settings().readBoolEntry("DNAWizardBreath/Gills", false);
	bool lastLungs = g_Au.Settings().readBoolEntry("DNAWizardBreath/Lungs", false);
	m_GillsCheck->setChecked(lastGills);
	m_LungsCheck->setChecked(lastLungs);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDNAWizardPageEat::AuDNAWizardPageEat(QWidget* parent) : AuDNAWizardPage(parent)
{
	QLabel* lbicon = new QLabel(this);
	lbicon->setPixmap(QPixmap(":data/Chromosome.png").scaled(128,128));
	lbicon->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<b>Eat</b>"
				"<p>"
				"Animals are heterotrophic, meaning they obtain nutrition from organic sources. "
				"Most animals obtain nutrition by ingesting other organisms or decomposing organic material. "
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	m_AnimalsCheck = new QCheckBox(tr("Animals"), this);
	connect(m_AnimalsCheck, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* animalsLabel = new QLabel(tr("<html>"
				"<p>"
				"Nutrition is obtained by consuming the bodies of animals. "
				"</p>"
				"</html>"), this);
	animalsLabel->setWordWrap(true);

	m_BloodSucker = new QCheckBox(tr("Drains blood from live animals for food."));
	connect(m_BloodSucker, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	m_Brain = new QCheckBox(tr("Eats brains."));
	connect(m_Brain, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	m_Cannibal = new QCheckBox(tr("Eats its own kind."));
	connect(m_Cannibal, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	m_Carne = new QCheckBox(tr("Eats fresh meat and bones."));
	connect(m_Carne, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	m_Carrion = new QCheckBox(tr("Eats rotting meat and bones."));
	connect(m_Carrion, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	m_Organs = new QCheckBox(tr("Eats internal organs."));
	connect(m_Organs, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	m_Predator = new QCheckBox(tr("Kills live animals for food."));
	connect(m_Predator, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	m_PlantsCheck = new QCheckBox(tr("Plants"), this);
	connect(m_PlantsCheck, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* plantsLabel = new QLabel(tr("<html>"
				"<p>"
				"Nutrition is obtained by consuming plants. "
				"</p>"
				"</html>"), this);
	plantsLabel->setWordWrap(true);

	QGridLayout* alay = new QGridLayout;
	int row=0;
	alay->addWidget(m_BloodSucker, row++,1);
	alay->addWidget(m_Brain, row++,1);
	alay->addWidget(m_Cannibal, row++,1);
	alay->addWidget(m_Carne, row++,1);
	alay->addWidget(m_Carrion, row++,1);
	alay->addWidget(m_Organs, row++,1);
	alay->addWidget(m_Predator, row++,1);
	alay->setColumnMinimumWidth(0,20);
	alay->setColumnStretch(1,1);

	QVBoxLayout* wlay = new QVBoxLayout;
	wlay->addWidget(lbinfo);
	wlay->addWidget(m_AnimalsCheck);
	wlay->addWidget(animalsLabel);
	wlay->addLayout(alay);
	wlay->addWidget(m_PlantsCheck);
	wlay->addWidget(plantsLabel);
	wlay->addStretch();

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(lbicon);
	lay->addLayout(wlay, 1);
	setLayout(lay);

	LoadSettings();
}

AuDNAWizardPageEat::~AuDNAWizardPageEat()
{
}

void AuDNAWizardPageEat::CheckOkToProceed()
{
	bool meat = m_AnimalsCheck->isChecked();
	m_BloodSucker->setEnabled(meat);
	m_Brain->setEnabled(meat);
	m_Cannibal->setEnabled(meat);
	m_Carne->setEnabled(meat);
	m_Carrion->setEnabled(meat);
	m_Organs->setEnabled(meat);
	m_Predator->setEnabled(meat);

	bool anymeat = m_BloodSucker->isChecked() ||
		m_Brain->isChecked() ||
		m_Cannibal->isChecked() ||
		m_Carne->isChecked() ||
		m_Carrion->isChecked() ||
		m_Organs->isChecked() ||
		m_Predator->isChecked();

	bool ok = true;
	if(!(m_AnimalsCheck->isChecked() && anymeat) &&
		!m_PlantsCheck->isChecked())
		ok = false;

	if(ok)
		emit OkToProceed(PAGE_Reproduction);
	else
		emit NotOkToProceed();
}

bool AuDNAWizardPageEat::BuildDNA(AuDNA& dna, quint16 headDiaMM) const
{
	Q_UNUSED(headDiaMM);
	if(!m_AnimalsCheck->isChecked() &&
			!m_PlantsCheck->isChecked())
		return false;	// This has already been checked but I can't not check. :/

	if(m_AnimalsCheck->isChecked())
	{
		quint8 eatBits = 0;
		if(m_BloodSucker->isChecked())
			eatBits |= AuChromoEatsAnimals::BloodSucker;
		if(m_Brain->isChecked())
			eatBits |= AuChromoEatsAnimals::Brain;
		if(m_Cannibal->isChecked())
			eatBits |= AuChromoEatsAnimals::Cannibal;
		if(m_Carne->isChecked())
			eatBits |= AuChromoEatsAnimals::Carne;
		if(m_Carrion->isChecked())
			eatBits |= AuChromoEatsAnimals::Carrion;
		if(m_Organs->isChecked())
			eatBits |= AuChromoEatsAnimals::Organs;
		if(m_Predator->isChecked())
			eatBits |= AuChromoEatsAnimals::Predator;
		if(!eatBits)
			return false;
		dna.AddChromosome(AuChromoEatsAnimals(eatBits));
	}

	if(m_PlantsCheck->isChecked())
		dna.AddChromosome(AuChromoEatsPlants());
	return true;
}

void AuDNAWizardPageEat::SaveSettings() const
{
	bool animals = m_AnimalsCheck->isChecked();
	bool bloodSucker = m_BloodSucker->isChecked();
	bool brain = m_Brain->isChecked();
	bool cannibal = m_Cannibal->isChecked();
	bool carne = m_Carne->isChecked();
	bool carrion = m_Carrion->isChecked();
	bool organs = m_Organs->isChecked();
	bool predator = m_Predator->isChecked();
	bool plants = m_PlantsCheck->isChecked();

	g_Au.Settings().writeEntry("DNAWizardEat/Animals", animals);
	g_Au.Settings().writeEntry("DNAWizardEat/AnimalsBloodSucker", bloodSucker);
	g_Au.Settings().writeEntry("DNAWizardEat/AnimalsBrain", brain);
	g_Au.Settings().writeEntry("DNAWizardEat/AnimalsCannibal", cannibal);
	g_Au.Settings().writeEntry("DNAWizardEat/AnimalsCarne", carne);
	g_Au.Settings().writeEntry("DNAWizardEat/AnimalsCarrion", carrion);
	g_Au.Settings().writeEntry("DNAWizardEat/AnimalsOrgans", organs);
	g_Au.Settings().writeEntry("DNAWizardEat/AnimalsPredator", predator);
	g_Au.Settings().writeEntry("DNAWizardEat/Plants", plants);
}

void AuDNAWizardPageEat::LoadSettings()
{
	bool lastAnimals = g_Au.Settings().readBoolEntry("DNAWizardEat/Animals", false);
	bool lastBloodSucker = g_Au.Settings().readBoolEntry("DNAWizardEat/AnimalsBloodSucker", false);
	bool lastBrain = g_Au.Settings().readBoolEntry("DNAWizardEat/AnimalsBrain", false);
	bool lastCannibal = g_Au.Settings().readBoolEntry("DNAWizardEat/AnimalsCannibal", false);
	bool lastCarne = g_Au.Settings().readBoolEntry("DNAWizardEat/AnimalsCarne", false);
	bool lastCarrion = g_Au.Settings().readBoolEntry("DNAWizardEat/AnimalsCarrion", false);
	bool lastOrgans = g_Au.Settings().readBoolEntry("DNAWizardEat/AnimalsOrgans", false);
	bool lastPredator = g_Au.Settings().readBoolEntry("DNAWizardEat/AnimalsPredator", false);
	bool lastPlants = g_Au.Settings().readBoolEntry("DNAWizardEat/Plants", false);

	m_AnimalsCheck->setChecked(lastAnimals);
	m_BloodSucker->setChecked(lastBloodSucker);
	m_Brain->setChecked(lastBrain);
	m_Cannibal->setChecked(lastCannibal);
	m_Carne->setChecked(lastCarne);
	m_Carrion->setChecked(lastCarrion);
	m_Organs->setChecked(lastOrgans);
	m_Predator->setChecked(lastPredator);
	m_PlantsCheck->setChecked(lastPlants);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDNAWizardPageReproduction::AuDNAWizardPageReproduction(QWidget* parent) : AuDNAWizardPage(parent)
{
	QLabel* lbicon = new QLabel(this);
	lbicon->setPixmap(QPixmap(":data/Chromosome.png").scaled(128,128));
	lbicon->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<b>Reproduction</b>"
				"<p>"
				"Life must reproduce to multiply its numbers and to compete as a species. "
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	m_AsexualRadio = new QRadioButton(tr("Asexual"), this);
	connect(m_AsexualRadio, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* asexLabel = new QLabel(tr("<html>"
				"<p>"
				"Asexual reproduction does not require DNA from a mate. "
				"Mutations in offspring are rare. "
				"Evolution is slow. "
				"</p>"
				"</html>"), this);
	asexLabel->setWordWrap(true);

	m_SexualRadio = new QRadioButton(tr("Sexual"), this);
	connect(m_SexualRadio, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* sexLabel = new QLabel(tr("<html>"
				"<p>"
				"Sexual reproduction requires a male to share DNA with a female. "
				"Mutations in offspring result from the blend of chromosomes inherited from each parent. "
				"Evolution is rapid. "
				"</p>"
				"</html>"), this);
	sexLabel->setWordWrap(true);

	m_LitterMin = new QSpinBox();
	m_LitterMin->setRange(1, 15);
	m_LitterMin->setSingleStep(1);
	connect(m_LitterMin, SIGNAL(valueChanged(int)), SLOT(LitterMinChanged(int)));

	QLabel* birthLabel = new QLabel(QObject::tr("Births"));

	QLabel* toLabel = new QLabel(QObject::tr("to"));

	m_LitterMax = new QSpinBox();
	m_LitterMax->setRange(1, 15);
	m_LitterMax->setSingleStep(1);
	connect(m_LitterMax, SIGNAL(valueChanged(int)), SLOT(LitterMaxChanged(int)));

	m_Young = new QComboBox;
	m_Young->addItem(QObject::tr("Eggs"), AuChromoReproduction::Eggs);
	m_Young->addItem(QObject::tr("Live Young"), AuChromoReproduction::Live);
	m_Young->setCurrentIndex(-1);
	connect(m_Young, SIGNAL(currentIndexChanged(int)), SLOT(CheckOkToProceed()));

	QLabel* youngLabel = new QLabel(QObject::tr("<html>"
				"<p>"
				"Animals that lay <i>eggs</i> do so shortly after becoming pregnant. "
				"Animals that birth <i>live young</i> do so after a gestation period. "
				"</p>"
				"</html>"));
	youngLabel->setWordWrap(true);

	m_BirthPlace = new QComboBox;
	m_BirthPlace->addItem(QObject::tr("On Land"), AuChromoReproduction::OnLand);
	m_BirthPlace->addItem(QObject::tr("In Water"), AuChromoReproduction::InWater);
	m_BirthPlace->addItem(QObject::tr("Dead Host"), AuChromoReproduction::DeadHost);
	m_BirthPlace->addItem(QObject::tr("Live Host"), AuChromoReproduction::LiveHost);
	m_BirthPlace->setCurrentIndex(-1);
	connect(m_BirthPlace, SIGNAL(currentIndexChanged(int)), SLOT(CheckOkToProceed()));

	QLabel* bpLabel = new QLabel(QObject::tr("<html>"
				"Birthing place specifies where the young are placed in preparation for birth. "
				"Live young are positioned in the birthing place as they are born from the mother. "
				"Eggs are positioned in the birthing place when the eggs are laid. "
				"</html>"));
	bpLabel->setWordWrap(true);

	m_MilkCheck = new QCheckBox(tr("Mammary"), this);
	connect(m_MilkCheck, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* milkLabel = new QLabel(QObject::tr("<html>"
				"Animals with mammary glands nourish their young with milk. "
				"</html>"));
	milkLabel->setWordWrap(true);

	QHBoxLayout* ylay = new QHBoxLayout;
	ylay->addWidget(birthLabel);
	ylay->addWidget(m_LitterMin);
	ylay->addWidget(toLabel);
	ylay->addWidget(m_LitterMax);
	ylay->addWidget(m_Young);
	ylay->addWidget(m_BirthPlace);
	ylay->addStretch();

	QVBoxLayout* wlay = new QVBoxLayout;
	wlay->addWidget(lbinfo);
	wlay->addWidget(m_AsexualRadio);
	wlay->addWidget(asexLabel);
	wlay->addWidget(m_SexualRadio);
	wlay->addWidget(sexLabel);
	wlay->addLayout(ylay);
	wlay->addWidget(youngLabel);
	wlay->addWidget(bpLabel);
	wlay->addWidget(m_MilkCheck);
	wlay->addWidget(milkLabel);
	wlay->addStretch();

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(lbicon);
	lay->addLayout(wlay, 1);
	setLayout(lay);

	LoadSettings();
}

AuDNAWizardPageReproduction::~AuDNAWizardPageReproduction()
{
}

void AuDNAWizardPageReproduction::CheckOkToProceed()
{
	if(!m_AsexualRadio->isChecked() &&
			!m_SexualRadio->isChecked())
		return;
	if(m_Young->currentIndex() < 0)
		return;
	if(m_BirthPlace->currentIndex() < 0)
		return;
	if(m_LitterMin->value() > m_LitterMax->value())
		return;

	emit OkToProceed(PAGE_Head);
}

void AuDNAWizardPageReproduction::LitterMaxChanged(int value)
{
	Q_UNUSED(value);
	CheckOkToProceed();
}

void AuDNAWizardPageReproduction::LitterMinChanged(int value)
{
	Q_UNUSED(value);
	CheckOkToProceed();
}

bool AuDNAWizardPageReproduction::BuildDNA(AuDNA& dna, quint16 headDiaMM) const
{
	quint8 litterMax = m_LitterMax->value();
	quint8 litterMin = m_LitterMin->value();
	if(litterMax < litterMin)
		return false;	// This has already been checked but I can't not check. :/
	AuChromoReproduction::BIRTHPLACE birthPlace = (AuChromoReproduction::BIRTHPLACE)m_BirthPlace->currentData().toInt();
	AuChromoReproduction::YOUNG young = (AuChromoReproduction::YOUNG)m_Young->currentData().toInt();

	quint8 boobs = litterMax;
	if(boobs & 1)
		boobs++;	// Bump up to an even number.
	if(boobs > AuChromoWhich::MaxCount())
		boobs = AuChromoWhich::MaxCount();

	quint16 boobSizeMM = headDiaMM * 2 / 3;

	if(m_AsexualRadio->isChecked())
	{
		dna.Chain(AuDNA::Alpha).AddChromosome(AuChromoReproduction(AuChromoReproduction::GEN_None, litterMin, litterMax, young,
					birthPlace));
		dna.Chain(AuDNA::Beta).AddChromosome(AuChromoReproduction(AuChromoReproduction::GEN_None, litterMin, litterMax, young,
					birthPlace));

		if(m_MilkCheck->isChecked())
		{
			for(int m=0; m<boobs; m++)
			{
				dna.Chain(AuDNA::Alpha)
					.AddChromosome(AuChromoMammary(AuChromoWhich(m, boobs), AuChromoMammary::Nipple, boobSizeMM/3, true, true));
				dna.Chain(AuDNA::Beta)
					.AddChromosome(AuChromoMammary(AuChromoWhich(m, boobs), AuChromoMammary::Nipple, boobSizeMM/3, true, true));
			}
		}
		return true;
	}
	else if(m_SexualRadio->isChecked())
	{
		dna.Chain(AuDNA::Alpha).AddChromosome(AuChromoReproduction(AuChromoReproduction::GEN_Male, litterMin, litterMax, young,
					birthPlace));
		dna.Chain(AuDNA::Beta).AddChromosome(AuChromoReproduction(AuChromoReproduction::GEN_Female, litterMin, litterMax, young,
					birthPlace));

		if(m_MilkCheck->isChecked())
		{
			for(int m=0; m<boobs; m++)
			{
				dna.Chain(AuDNA::Alpha)
					.AddChromosome(AuChromoMammary(AuChromoWhich(m, boobs), AuChromoMammary::Nipple, boobSizeMM/3, false, true));
				dna.Chain(AuDNA::Beta)
					.AddChromosome(AuChromoMammary(AuChromoWhich(m, boobs), AuChromoMammary::Breast, boobSizeMM, true, true));
			}
		}
		return true;
	}
	else
		return false;
}

void AuDNAWizardPageReproduction::SaveSettings() const
{
	int sex;
	if(m_AsexualRadio->isChecked())
		sex = 0;
	else if(m_SexualRadio->isChecked())
		sex = 1;
	else
		sex = -1;

	g_Au.Settings().writeEntry("DNAWizardReproduction/Sex", sex);
	g_Au.Settings().writeEntry("DNAWizardReproduction/LitterMin", m_LitterMin->value());
	g_Au.Settings().writeEntry("DNAWizardReproduction/LitterMax", m_LitterMax->value());
	if(m_Young->currentIndex() >= 0)
		g_Au.Settings().writeEntry("DNAWizardReproduction/Young", m_Young->currentData().toInt());
	else
		g_Au.Settings().writeEntry("DNAWizardReproduction/Young", -1);
	if(m_BirthPlace->currentIndex() >= 0)
		g_Au.Settings().writeEntry("DNAWizardReproduction/BirthPlace", m_BirthPlace->currentData().toInt());
	else
		g_Au.Settings().writeEntry("DNAWizardReproduction/BirthPlace", -1);
	g_Au.Settings().writeEntry("DNAWizardReproduction/Mammary", m_MilkCheck->isChecked());
}

void AuDNAWizardPageReproduction::LoadSettings()
{
	int lastSex = g_Au.Settings().readNumEntry("DNAWizardReproduction/Sex", -1);
	int lastLitterMin = g_Au.Settings().readNumEntry("DNAWizardReproduction/LitterMin", -1);
	int lastLitterMax = g_Au.Settings().readNumEntry("DNAWizardReproduction/LitterMax", -1);
	int young = g_Au.Settings().readNumEntry("DNAWizardReproduction/Young", -1);
	int birthPlace = g_Au.Settings().readNumEntry("DNAWizardReproduction/BirthPlace", -1);
	bool mammary = g_Au.Settings().readBoolEntry("DNAWizardReproduction/Mammary", false);
	switch(lastSex)
	{
	case 0:
		m_AsexualRadio->setChecked(true);
		break;
	case 1:
		m_SexualRadio->setChecked(true);
		break;
	}
	m_LitterMin->setValue(lastLitterMin);
	m_LitterMax->setValue(lastLitterMax);
	m_Young->setCurrentIndex(m_Young->findData(young));
	m_BirthPlace->setCurrentIndex(m_BirthPlace->findData(birthPlace));
	m_MilkCheck->setChecked(mammary);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDNAWizardPageHead::AuDNAWizardPageHead(QWidget* parent) : AuDNAWizardPage(parent)
{
	QLabel* lbicon = new QLabel(this);
	lbicon->setPixmap(QPixmap(":data/Chromosome.png").scaled(128,128));
	lbicon->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<b>Head</b>"
				"<p>"
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	m_Diameter = new QSpinBox();
	m_Diameter->setRange(10, d_DNAMaxMM);
	m_Diameter->setValue(200);
	m_Diameter->setSingleStep(1);
	m_Diameter->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	connect(m_Diameter, SIGNAL(valueChanged(int)), SLOT(CheckOkToProceed()));

	QLabel* htLabel = new QLabel(tr("<html>"
				"<b>Diameter in mm</b>"
				"<p>"
				"Head diameter determines the size of the head. "
				"If the head is elliptical, this is the largest dimension of the head. "
				"</p>"
				"<p>"
				"This is the only size factor in this wizard. "
				"It is used to calculate sensible sizes for all other body parts. "
				"All sizes can be individually adjusted after the wizard is complete. "
				"</p>"
				"</html>"), this);
	htLabel->setWordWrap(true);

	m_BackRadio = new QRadioButton(tr("Attaches at Back of Skull"), this);
	connect(m_BackRadio, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* backLabel = new QLabel(tr("<html>"
				"<p>"
				"Head is attached at the back of the skull. "
				"</p>"
				"<p>"
				"Example: snakes, lizards, fish, birds"
				"</p>"
				"</html>"), this);
	backLabel->setWordWrap(true);

	m_BottomRadio = new QRadioButton(tr("Attaches at Bottom of Skull"), this);
	connect(m_BottomRadio, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QLabel* bottomLabel = new QLabel(tr("<html>"
				"<p>"
				"Head is attached at the bottom of the skull. "
				"</p>"
				"<p>"
				"Example: humans, birds"
				"</p>"
				"</html>"), this);
	bottomLabel->setWordWrap(true);

	QHBoxLayout* hlay = new QHBoxLayout;
	hlay->addWidget(m_Diameter);
	hlay->addWidget(htLabel, 1);

	QVBoxLayout* wlay = new QVBoxLayout;
	wlay->addWidget(lbinfo);
	wlay->addLayout(hlay);
	wlay->addWidget(m_BackRadio);
	wlay->addWidget(backLabel);
	wlay->addWidget(m_BottomRadio);
	wlay->addWidget(bottomLabel);
	wlay->addStretch();

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(lbicon);
	lay->addLayout(wlay, 1);
	setLayout(lay);

	LoadSettings();
}

AuDNAWizardPageHead::~AuDNAWizardPageHead()
{
}

void AuDNAWizardPageHead::CheckOkToProceed()
{
	if(!m_BackRadio->isChecked() &&
			!m_BottomRadio->isChecked())
		return;

	emit OkToProceed(PAGE_Body);
}

bool AuDNAWizardPageHead::BuildDNA(AuDNA& dna, quint16 headDiaMM) const
{
	switch(dna.Phylum())
	{
	case AuChromosome::Annelida:
		break;	//DHH Not implemented
	case AuChromosome::Arthropoda:
		break;	//DHH Not implemented
	case AuChromosome::Chordata:
		{
			quint16 lengthMM = headDiaMM;
			quint16 heightMM = headDiaMM;
			quint16 widthMM = headDiaMM;

			AuChromoHead::ATTACHMENT attach;
			quint16 attachOffMM;
			if(m_BackRadio->isChecked())
			{
				attach = AuChromoHead::AttachBack;
				attachOffMM = lengthMM/4;
			}
			else if(m_BottomRadio->isChecked())
			{
				attach = AuChromoHead::AttachBottom;
				attachOffMM = heightMM/4;
			}
			else
				return false;

			qint16 headEarAngDeg;
			qint16 headEyeAngDeg;
			qint16 mouthAngDeg = 135;
			qint16 headNoseAngDeg = 90;
			AuChromoEye::FACING eyeFacing;
			quint16 mouthLengthMM = lengthMM / 4;
			quint16 mouthWidthMM = widthMM / 2;
			quint16 mouthHeightMM = mouthWidthMM / 2;
			quint16 mouthDepthMM = mouthWidthMM / 3;
			quint16 lipThicknessMM = mouthWidthMM / 20 + 1;
			AuChromoMouth::TEETH teeth;
			quint16 teethHtBottomMM;
			quint16 teethHtTopMM;
			qint16 earAngDeg;
			if(dna.Chain(AuDNA::Alpha).IsPredator())
			{
				headEarAngDeg = 90;
				headEyeAngDeg = 22;
				eyeFacing = AuChromoEye::Forward;
				teeth = AuChromoMouth::Fangs;
				teethHtBottomMM = lipThicknessMM;
				teethHtTopMM = lipThicknessMM + 1;
				earAngDeg = 45;
			}
			else
			{
				headEarAngDeg = 60;
				headEyeAngDeg = 60;
				eyeFacing = AuChromoEye::Away;
				teeth = AuChromoMouth::Flat;
				teethHtBottomMM = lipThicknessMM/2 + 1;
				teethHtTopMM = lipThicknessMM/2 + 1;
				earAngDeg = 90;
			}

			AuChromoHead head(attach, attachOffMM, widthMM, lengthMM, heightMM, headEarAngDeg, headEyeAngDeg, mouthAngDeg,
					headNoseAngDeg);
			dna.AddChromosome(head);

			AuChromoEar::STYLE earStyle = AuChromoEar::Circle;
			quint16 earDiaMM = lengthMM / 4;
			for(int ear=0; ear<2; ear++)
				dna.AddChromosome(AuChromoEar(AuChromoWhich(ear, 2), earStyle, earDiaMM, earAngDeg));

			MEAT_DECAL eyeDecal = MD_HumanEye1;
			quint16 eyeDiaMM = widthMM / 8;
			quint16 socketDiaMM = eyeDiaMM * 3 / 2;
			for(int eye=0; eye<2; eye++)
				dna.AddChromosome(AuChromoEye(AuChromoWhich(eye, 2), eyeFacing, eyeDecal, eyeDiaMM, socketDiaMM));

			quint16 noseLenMM = lengthMM / 5;
			quint16 noseWidAtHeadMM = noseLenMM / 2;
			quint16 noseWidAtTipMM = noseLenMM * 3 / 4;
			qint16 noseAngDeg = -10;
			dna.AddChromosome(AuChromoNose(noseLenMM, noseWidAtHeadMM, noseWidAtTipMM, noseAngDeg));

			dna.AddChromosome(AuChromoMouth(mouthLengthMM, mouthWidthMM, mouthHeightMM, mouthDepthMM, lipThicknessMM, teeth,
						teethHtBottomMM, teethHtTopMM));
			return true;
		}
		break;
	case AuChromosome::Mollusca:
		break;	//DHH Not implemented
	default:
		break;
	}
	return false;
}

quint16 AuDNAWizardPageHead::DiameterMM() const
{
	return m_Diameter->value();
}

void AuDNAWizardPageHead::SaveSettings() const
{
	int attach;
	if(m_BackRadio->isChecked())
		attach = AuChromoHead::AttachBack;
	else if(m_BottomRadio->isChecked())
		attach = AuChromoHead::AttachBottom;
	else
		attach = -1;

	g_Au.Settings().writeEntry("DNAWizardHead/Attach", attach);
	g_Au.Settings().writeEntry("DNAWizardHead/Diameter", m_Diameter->value());
}

void AuDNAWizardPageHead::LoadSettings()
{
	int lastAttach = g_Au.Settings().readNumEntry("DNAWizardHead/Attach", -1);
	int lastDia = g_Au.Settings().readNumEntry("DNAWizardHead/Diameter", 200);
	switch(lastAttach)
	{
	case AuChromoHead::AttachBack:
		m_BackRadio->setChecked(true);
		break;
	case AuChromoHead::AttachBottom:
		m_BottomRadio->setChecked(true);
		break;
	}
	m_Diameter->setValue(lastDia);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDNAWizardPageBody::AuDNAWizardPageBody(QWidget* parent) : AuDNAWizardPage(parent)
{
	QLabel* lbicon = new QLabel(this);
	lbicon->setPixmap(QPixmap(":data/Chromosome.png").scaled(128,128));
	lbicon->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<b>Body</b>"
				"<p>"
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	QVBoxLayout* wlay = new QVBoxLayout;
	wlay->addWidget(lbinfo);
	wlay->addStretch();

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(lbicon);
	lay->addLayout(wlay, 1);
	setLayout(lay);

	LoadSettings();
}

AuDNAWizardPageBody::~AuDNAWizardPageBody()
{
}

void AuDNAWizardPageBody::CheckOkToProceed()
{
	emit OkToProceed(PAGE_Limbs);
}

bool AuDNAWizardPageBody::BuildDNA(AuDNA& dna, quint16 headDiaMM) const
{
	switch(dna.Phylum())
	{
	case AuChromosome::Annelida:
		break;	//DHH Not implemented
	case AuChromosome::Arthropoda:
		break;	//DHH Not implemented
	case AuChromosome::Chordata:
		{
			quint16 neckLengthMM = headDiaMM;
			quint16 upperLengthMM = headDiaMM;
			quint16 upperWidthMM = headDiaMM;
			quint16 upperThicknessMM = headDiaMM;
			quint16 lowerLengthMM = headDiaMM;
			quint16 lowerWidthMM = upperWidthMM / 2;
			quint16 lowerThicknessMM = upperThicknessMM;
			quint16 tailLengthMM = headDiaMM * 2;
			dna.AddChromosome(AuChromoTorso(neckLengthMM, upperLengthMM, upperWidthMM, upperThicknessMM, lowerLengthMM, lowerWidthMM,
					lowerThicknessMM, tailLengthMM));
			return true;
		}
		break;
	case AuChromosome::Mollusca:
		break;	//DHH Not implemented
	default:
		break;
	}
	return false;
}

void AuDNAWizardPageBody::SaveSettings() const
{
}

void AuDNAWizardPageBody::LoadSettings()
{
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDNAWizardPageLimbs::AuDNAWizardPageLimbs(QWidget* parent) : AuDNAWizardPage(parent)
{
	QLabel* lbicon = new QLabel(this);
	lbicon->setPixmap(QPixmap(":data/Chromosome.png").scaled(128,128));
	lbicon->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<b>Limbs</b>"
				"<p>"
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	m_Arms = new QSpinBox();
	m_Arms->setRange(0, AuChromoWhich::MaxCount()/2);
	m_Arms->setSingleStep(1);
	m_Arms->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	connect(m_Arms, SIGNAL(valueChanged(int)), SLOT(CheckOkToProceed()));

	QLabel* lbArms = new QLabel("<html>"
			"<b>Arm Pairs</b>"
			"<p>"
			"Arms enable manipulation of objects. "
			"Arms assist in locomotion on land and in water. "
			"</p>"
			"</html>", this);
	lbArms->setWordWrap(true);

	m_ArmsWalk = new QCheckBox(tr("Arms are used as legs when walking. "), this);
	connect(m_ArmsWalk, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	m_ArmsRun = new QCheckBox(tr("Arms are used as legs when running. "), this);
	connect(m_ArmsRun, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	m_ArmsJump = new QCheckBox(tr("Arms are used as legs when jumping. "), this);
	connect(m_ArmsJump, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	m_ArmStandTone = new QSpinBox(this);
	m_ArmStandTone->setRange(0, 100);
	m_ArmStandTone->setSingleStep(1);
	m_ArmStandTone->setToolTip(tr("<html>"
				"Standing muscle tone, from 0 to 100. "
				"<br>"
				"At 100 the arm is as straight as possible when standing. "
				"<br>"
				"At 50 the arm bends partly, holding the body above ground. "
				"<br>"
				"At 0 the arm bends fully, does not prevent the body from touching ground. "
				"</html>"));
	connect(m_ArmStandTone, SIGNAL(valueChanged(int)), SLOT(CheckOkToProceed()));

	m_Legs = new QSpinBox();
	m_Legs->setRange(0, AuChromoWhich::MaxCount()/2);
	m_Legs->setSingleStep(1);
	m_Legs->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	connect(m_Legs, SIGNAL(valueChanged(int)), SLOT(CheckOkToProceed()));

	m_LegStandTone = new QSpinBox(this);
	m_LegStandTone->setRange(0, 100);
	m_LegStandTone->setSingleStep(1);
	m_LegStandTone->setToolTip(tr("<html>"
				"Standing muscle tone, from 0 to 100. "
				"<br>"
				"At 100 the leg is as straight as possible when standing. "
				"<br>"
				"At 50 the leg bends partly, holding the body above ground. "
				"<br>"
				"At 0 the leg bends fully, does not prevent the body from touching ground. "
				"</html>"));
	connect(m_LegStandTone, SIGNAL(valueChanged(int)), SLOT(CheckOkToProceed()));

	QLabel* lbLegs = new QLabel("<html>"
			"<b>Leg Pairs</b>"
			"<p>"
			"Legs support the body above the ground. "
			"Legs provide locomotion to creatures on land and in water. "
			"</p>"
			"</html>", this);
	lbLegs->setWordWrap(true);

	m_Wings = new QSpinBox();
	m_Wings->setRange(0, AuChromoWhich::MaxCount()/2);
	m_Wings->setSingleStep(1);
	m_Wings->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	connect(m_Wings, SIGNAL(valueChanged(int)), SLOT(CheckOkToProceed()));

	QLabel* lbWings = new QLabel("<html>"
			"<b>Wing Pairs</b>"
			"<p>"
			"Wings provide locomotion to flying creatures. "
			"Wings assist in locomotion on land. "
			"</p>"
			"</html>", this);
	lbWings->setWordWrap(true);

	m_Tentacles = new QSpinBox();
	m_Tentacles->setRange(0, AuChromoWhich::MaxCount());
	m_Tentacles->setSingleStep(1);
	m_Tentacles->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	connect(m_Tentacles, SIGNAL(valueChanged(int)), SLOT(CheckOkToProceed()));

	QLabel* lbTentacles = new QLabel("<html>"
			"<b>Tentacles</b>"
			"<p>"
			"Tentacles enable manipulation of objects. "
			"Tentacles provide locomotion on land and in water. "
			"</p>"
			"</html>", this);
	lbTentacles->setWordWrap(true);

	m_Fins = new QSpinBox();
	m_Fins->setRange(0, AuChromoWhich::MaxCount());
	m_Fins->setSingleStep(1);
	m_Fins->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	connect(m_Fins, SIGNAL(valueChanged(int)), SLOT(CheckOkToProceed()));

	QLabel* lbFins = new QLabel("<html>"
			"<b>Fins</b>"
			"<p>"
			"Fins provide locomotion in water. "
			"</p>"
			"</html>", this);
	lbFins->setWordWrap(true);

	m_FlyCheck = new QCheckBox(tr("Creature can fly. "), this);
	connect(m_FlyCheck, SIGNAL(clicked()), SLOT(CheckOkToProceed()));

	QGridLayout* wlay = new QGridLayout;
	int row=0;
	wlay->addWidget(lbinfo, row,0, 1,2);
	row++;
	wlay->addWidget(m_Arms, row,0);
	wlay->addWidget(lbArms, row,1);
	row++;
	wlay->addWidget(m_ArmStandTone, row,1);
	row++;
	wlay->addWidget(m_ArmsWalk, row,1);
	row++;
	wlay->addWidget(m_ArmsRun, row,1);
	row++;
	wlay->addWidget(m_ArmsJump, row,1);
	row++;
	wlay->addWidget(m_Legs, row,0);
	wlay->addWidget(lbLegs, row,1);
	row++;
	wlay->addWidget(m_LegStandTone, row,1);
	row++;
	wlay->addWidget(m_Wings, row,0);
	wlay->addWidget(lbWings, row,1);
	row++;
	wlay->addWidget(m_FlyCheck, row,1);
	row++;
	wlay->addWidget(m_Fins, row,0);
	wlay->addWidget(lbFins, row,1);
	row++;
	wlay->addWidget(m_Tentacles, row,0);
	wlay->addWidget(lbTentacles, row,1);
	row++;
	wlay->setRowStretch(row, 1);
	wlay->setColumnStretch(1, 1);

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(lbicon);
	lay->addLayout(wlay, 1);
	setLayout(lay);

	LoadSettings();
}

AuDNAWizardPageLimbs::~AuDNAWizardPageLimbs()
{
}

void AuDNAWizardPageLimbs::CheckOkToProceed()
{
	m_FlyCheck->setEnabled(m_Wings->value() > 0);
	m_ArmsJump->setEnabled(m_Arms->value() > 0);
	m_ArmsRun->setEnabled(m_Arms->value() > 0);
	m_ArmStandTone->setEnabled(m_Arms->value() > 0);
	m_ArmsWalk->setEnabled(m_Arms->value() > 0);
	m_LegStandTone->setEnabled(m_Legs->value() > 0);

	emit OkToProceed(PAGE_Finish);
}

bool AuDNAWizardPageLimbs::BuildDNA(AuDNA& dna, quint16 headDiaMM) const
{
	int armPairs = m_Arms->value();
	int armCount = armPairs*2;
	for(int arm=0; arm<armCount; arm++)
	{
		quint16 shoulderToSpineMM = headDiaMM * 3 / 4;
		quint16 shoulderRadMM = headDiaMM / 3;
		quint16 upperArmLengthMM = headDiaMM;
		quint16 elbowRadMM = shoulderRadMM / 2;
		quint16 foreArmLenMM = headDiaMM;
		quint16 wristRadMM = shoulderRadMM / 3;
		quint16 handLenMM = headDiaMM * 2 / 3;
		quint8 legBits = 0;
		if(m_ArmsWalk->isChecked())
			legBits |= AuChromoArm::LEG_Walk;
		if(m_ArmsRun->isChecked())
			legBits |= AuChromoArm::LEG_Run;
		if(m_ArmsJump->isChecked())
			legBits |= AuChromoArm::LEG_Jump;
		dna.AddChromosome(AuChromoArm(AuChromoWhich(arm,armCount), shoulderToSpineMM, shoulderRadMM, upperArmLengthMM, elbowRadMM,
					foreArmLenMM, wristRadMM, handLenMM, legBits, m_ArmStandTone->value()));
	}

	int legPairs = m_Legs->value();
	int legCount = legPairs * 2;
	for(int leg=0; leg<legCount; leg++)
	{
		quint16 thighLenMM = headDiaMM;
		quint16 calfLenMM = headDiaMM;
		quint16 buttRadMM = headDiaMM * 40 / 100;
		quint16 hipRadXMM = headDiaMM * 35 / 100;
		quint16 hipToSpineMM = hipRadXMM;
		quint16 kneeRadXMM = hipRadXMM / 2;
		quint16 ankleHtMM = headDiaMM / 2;
		quint16 ankleRadYMM = hipRadXMM / 3;
		quint16 footLenMM = headDiaMM;
		quint16 footWidMM = footLenMM / 2;
		dna.AddChromosome(AuChromoLeg(AuChromoWhich(leg,legCount), hipToSpineMM, thighLenMM, calfLenMM, buttRadMM, hipRadXMM, kneeRadXMM,
					ankleHtMM, ankleRadYMM, footLenMM, footWidMM, m_LegStandTone->value()));
	}

	/*DHH not implemented fins, wings, tentacles
	QSpinBox*							m_Fins;
	QCheckBox*							m_FlyCheck;
	QSpinBox*							m_Tentacles;
	QSpinBox*							m_Wings;
	*/
	return true;
}

void AuDNAWizardPageLimbs::SaveSettings() const
{
	g_Au.Settings().writeEntry("DNAWizardLimbs/ArmPairs", m_Arms->value());
	g_Au.Settings().writeEntry("DNAWizardLimbs/ArmsJump", m_ArmsJump->isChecked());
	g_Au.Settings().writeEntry("DNAWizardLimbs/ArmsRun", m_ArmsRun->isChecked());
	g_Au.Settings().writeEntry("DNAWizardLimbs/ArmsWalk", m_ArmsWalk->isChecked());
	g_Au.Settings().writeEntry("DNAWizardLimbs/ArmStandTone", m_ArmStandTone->value());
	g_Au.Settings().writeEntry("DNAWizardLimbs/LegPairs", m_Legs->value());
	g_Au.Settings().writeEntry("DNAWizardLimbs/LegStandTone", m_LegStandTone->value());
	g_Au.Settings().writeEntry("DNAWizardLimbs/WingPairs", m_Wings->value());
	g_Au.Settings().writeEntry("DNAWizardLimbs/WingsCanFly", m_FlyCheck->isChecked());
	g_Au.Settings().writeEntry("DNAWizardLimbs/Fins", m_Fins->value());
	g_Au.Settings().writeEntry("DNAWizardLimbs/Tentacles", m_Tentacles->value());
}

void AuDNAWizardPageLimbs::LoadSettings()
{
	int lastArms = g_Au.Settings().readNumEntry("DNAWizardLimbs/ArmPairs", -1);
	bool lastHandsJump = g_Au.Settings().readBoolEntry("DNAWizardLimbs/ArmsJump", false);
	bool lastHandsRun = g_Au.Settings().readBoolEntry("DNAWizardLimbs/ArmsRun", false);
	bool lastHandsWalk = g_Au.Settings().readBoolEntry("DNAWizardLimbs/ArmsWalk", false);
	int lastArmStandTone = g_Au.Settings().readNumEntry("DNAWizardLimbs/ArmStandTone", 100);
	int lastLegs = g_Au.Settings().readNumEntry("DNAWizardLimbs/LegPairs", -1);
	int lastLegStandTone = g_Au.Settings().readNumEntry("DNAWizardLimbs/LegStandTone", 100);
	int lastWings = g_Au.Settings().readNumEntry("DNAWizardLimbs/WingPairs", -1);
	bool lastCanFly = g_Au.Settings().readBoolEntry("DNAWizardLimbs/WingsCanFly", false);
	int lastFins = g_Au.Settings().readNumEntry("DNAWizardLimbs/Fins", -1);
	int lastTentacles = g_Au.Settings().readNumEntry("DNAWizardLimbs/Tentacles", -1);
	m_Arms->setValue(lastArms);
	m_ArmStandTone->setValue(lastArmStandTone);
	m_ArmsJump->setChecked(lastHandsJump);
	m_ArmsRun->setChecked(lastHandsRun);
	m_ArmsWalk->setChecked(lastHandsWalk);
	m_Fins->setValue(lastFins);
	m_FlyCheck->setChecked(lastCanFly);
	m_Legs->setValue(lastLegs);
	m_LegStandTone->setValue(lastLegStandTone);
	m_Tentacles->setValue(lastTentacles);
	m_Wings->setValue(lastWings);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDNAWizard::AuDNAWizard(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("DNA Wizard"));
	setAuDlgIcon(QIcon(":data/DNA.png"));

	m_Pages = new QStackedLayout;

	AuDNAWizardPageIntro* pageIntro = new AuDNAWizardPageIntro(this);
	m_Pages->insertWidget(m_Pages->count(), pageIntro);
	m_PagesByType.insert(pageIntro->PageType(), pageIntro);
	connect(pageIntro, SIGNAL(OkToFinish()), SLOT(EnableFinish()));
	connect(pageIntro, SIGNAL(OkToProceed(int)), SLOT(EnableNext(int)));
	connect(pageIntro, SIGNAL(NotOkToProceed()), SLOT(DisableNext()));

	AuDNAWizardPagePhylum* pagePhylum = new AuDNAWizardPagePhylum(this);
	m_Pages->insertWidget(m_Pages->count(), pagePhylum);
	m_PagesByType.insert(pagePhylum->PageType(), pagePhylum);
	connect(pagePhylum, SIGNAL(OkToFinish()), SLOT(EnableFinish()));
	connect(pagePhylum, SIGNAL(OkToProceed(int)), SLOT(EnableNext(int)));
	connect(pagePhylum, SIGNAL(NotOkToProceed()), SLOT(DisableNext()));

	m_NextPageType = pagePhylum->PageType();

	AuDNAWizardPageHide* pageHide = new AuDNAWizardPageHide(this);
	m_Pages->insertWidget(m_Pages->count(), pageHide);
	m_PagesByType.insert(pageHide->PageType(), pageHide);
	connect(pageHide, SIGNAL(OkToFinish()), SLOT(EnableFinish()));
	connect(pageHide, SIGNAL(OkToProceed(int)), SLOT(EnableNext(int)));
	connect(pageHide, SIGNAL(NotOkToProceed()), SLOT(DisableNext()));

	AuDNAWizardPageBlood* pageBlood = new AuDNAWizardPageBlood(this);
	m_Pages->insertWidget(m_Pages->count(), pageBlood);
	m_PagesByType.insert(pageBlood->PageType(), pageBlood);
	connect(pageBlood, SIGNAL(OkToFinish()), SLOT(EnableFinish()));
	connect(pageBlood, SIGNAL(OkToProceed(int)), SLOT(EnableNext(int)));
	connect(pageBlood, SIGNAL(NotOkToProceed()), SLOT(DisableNext()));

	AuDNAWizardPageBreath* pageBreath = new AuDNAWizardPageBreath(this);
	m_Pages->insertWidget(m_Pages->count(), pageBreath);
	m_PagesByType.insert(pageBreath->PageType(), pageBreath);
	connect(pageBreath, SIGNAL(OkToFinish()), SLOT(EnableFinish()));
	connect(pageBreath, SIGNAL(OkToProceed(int)), SLOT(EnableNext(int)));
	connect(pageBreath, SIGNAL(NotOkToProceed()), SLOT(DisableNext()));

	AuDNAWizardPageEat* pageEat = new AuDNAWizardPageEat(this);
	m_Pages->insertWidget(m_Pages->count(), pageEat);
	m_PagesByType.insert(pageEat->PageType(), pageEat);
	connect(pageEat, SIGNAL(OkToFinish()), SLOT(EnableFinish()));
	connect(pageEat, SIGNAL(OkToProceed(int)), SLOT(EnableNext(int)));
	connect(pageEat, SIGNAL(NotOkToProceed()), SLOT(DisableNext()));

	AuDNAWizardPageReproduction* pageReproduction = new AuDNAWizardPageReproduction(this);
	m_Pages->insertWidget(m_Pages->count(), pageReproduction);
	m_PagesByType.insert(pageReproduction->PageType(), pageReproduction);
	connect(pageReproduction, SIGNAL(OkToFinish()), SLOT(EnableFinish()));
	connect(pageReproduction, SIGNAL(OkToProceed(int)), SLOT(EnableNext(int)));
	connect(pageReproduction, SIGNAL(NotOkToProceed()), SLOT(DisableNext()));

	AuDNAWizardPageHead* pageHead = new AuDNAWizardPageHead(this);
	m_Pages->insertWidget(m_Pages->count(), pageHead);
	m_PagesByType.insert(pageHead->PageType(), pageHead);
	connect(pageHead, SIGNAL(OkToFinish()), SLOT(EnableFinish()));
	connect(pageHead, SIGNAL(OkToProceed(int)), SLOT(EnableNext(int)));
	connect(pageHead, SIGNAL(NotOkToProceed()), SLOT(DisableNext()));

	AuDNAWizardPageBody* pageBody = new AuDNAWizardPageBody(this);
	m_Pages->insertWidget(m_Pages->count(), pageBody);
	m_PagesByType.insert(pageBody->PageType(), pageBody);
	connect(pageBody, SIGNAL(OkToFinish()), SLOT(EnableFinish()));
	connect(pageBody, SIGNAL(OkToProceed(int)), SLOT(EnableNext(int)));
	connect(pageBody, SIGNAL(NotOkToProceed()), SLOT(DisableNext()));

	AuDNAWizardPageLimbs* pageLimbs = new AuDNAWizardPageLimbs(this);
	m_Pages->insertWidget(m_Pages->count(), pageLimbs);
	m_PagesByType.insert(pageLimbs->PageType(), pageLimbs);
	connect(pageLimbs, SIGNAL(OkToFinish()), SLOT(EnableFinish()));
	connect(pageLimbs, SIGNAL(OkToProceed(int)), SLOT(EnableNext(int)));
	connect(pageLimbs, SIGNAL(NotOkToProceed()), SLOT(DisableNext()));

	AuDNAWizardPageFinish* pageFinish = new AuDNAWizardPageFinish(this);
	m_Pages->insertWidget(m_Pages->count(), pageFinish);
	m_PagesByType.insert(pageFinish->PageType(), pageFinish);
	connect(pageFinish, SIGNAL(OkToFinish()), SLOT(EnableFinish()));
	connect(pageFinish, SIGNAL(OkToProceed(int)), SLOT(EnableNext(int)));
	connect(pageFinish, SIGNAL(NotOkToProceed()), SLOT(DisableNext()));

	m_BackButton = new QPushButton(QIcon(":data/hazLeftArrow.png"), tr("Back"),this);
	m_BackButton->setEnabled(false);
	connect(m_BackButton, SIGNAL(clicked()), SLOT(BackClicked()));

	m_NextButton = new QPushButton(QIcon(":data/hazRightArrow.png"), tr("Next"),this);
	m_NextButton->setEnabled(true);
	connect(m_NextButton, SIGNAL(clicked()), SLOT(NextClicked()));

	m_FinishButton = new QPushButton(QIcon(":data/new12x12.png"), tr("Finish"),this);
	m_FinishButton->setEnabled(false);
	m_FinishButton->setToolTip(tr("<html>"
				"Finish button is disabled because this wizard is not ready for use. "	//DHH animalia
				"</html>"));
	connect(m_FinishButton, SIGNAL(clicked()), SLOT(FinishClicked()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(m_BackButton);
	blay->addWidget(m_NextButton);
	blay->addWidget(m_FinishButton);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(m_Pages);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuDNAWizard::~AuDNAWizard()
{
}

void AuDNAWizard::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuDNAWizard::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/DNAWizard/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/DNAWizard/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/DNAWizard/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/DNAWizard/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuDNAWizard::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/DNAWizard/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/DNAWizard/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/DNAWizard/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/DNAWizard/SizeY",sz.height());
}

void AuDNAWizard::BackClicked()
{
	if(m_PageSeq.empty())
		return;							// No previous page.
	QWidget* prev = m_PageSeq.takeLast();
	DisableBackNextFinish();
	m_Pages->setCurrentWidget(prev);	// This will cause the page to signal if Next is ok.
}

void AuDNAWizard::NextClicked()
{
	AuDNAWizardPage* curPage = dynamic_cast<AuDNAWizardPage*>(m_Pages->currentWidget());
	if(!curPage)
		return;
	QHash<AuDNAWizardPage::PAGE, AuDNAWizardPage*>::const_iterator pi = m_PagesByType.find(m_NextPageType);
	if(pi == m_PagesByType.cend())
		return;							// Next page not found.
	m_PageSeq.append(curPage);
	DisableBackNextFinish();
	m_Pages->setCurrentWidget(*pi);		// This will cause the page to signal if Next is ok.
}

void AuDNAWizard::FinishClicked()
{
	// Insure the page sequence includes the finish page, for the expected call to DNA().
	AuDNAWizardPage* curPage = dynamic_cast<AuDNAWizardPage*>(m_Pages->currentWidget());
	if(!curPage)
		return;
	m_PageSeq.append(curPage);
	accept();
}

void AuDNAWizard::DisableBackNextFinish()
{
	//qDebug() << "AuDNAWizard::DisableBackNextFinish";
	m_BackButton->setEnabled(!m_PageSeq.empty());
	m_NextButton->setEnabled(false);	// The page has to signal that its ok.
	m_FinishButton->setEnabled(false);	// The page has to signal that its ok.
}

void AuDNAWizard::EnableFinish()
{
	//qDebug() << "AuDNAWizard::EnableFinish";
	//DHH animalia m_FinishButton->setEnabled(true);
	m_FinishButton->setEnabled(false);	//DHH animalia
}

void AuDNAWizard::EnableNext(int nextPageType)
{
	//qDebug() << "AuDNAWizard::EnableNext";
	m_NextPageType = (AuDNAWizardPage::PAGE)nextPageType;
	m_NextButton->setEnabled(m_NextPageType != AuDNAWizardPage::PAGE_Invalid);
}

void AuDNAWizard::DisableNext()
{
	//qDebug() << "AuDNAWizard::DisableNext";
	m_NextButton->setEnabled(false);
}

AuDNA AuDNAWizard::DNA() const
{
	QHash<AuDNAWizardPage::PAGE, AuDNAWizardPage*>::const_iterator pi = m_PagesByType.find(AuDNAWizardPage::PAGE_Head);
	if(pi == m_PagesByType.cend())
		return AuDNA();
	AuDNAWizardPageHead* pageHead = dynamic_cast<AuDNAWizardPageHead*>(*pi);
	if(!pageHead)
		return AuDNA();	// wut?

	quint16 headDiaMM = pageHead->DiameterMM();

	// Collect DNA from each page in the order they were navigated by the user.
	AuDNA dna;
	for(QList<AuDNAWizardPage*>::const_iterator pi = m_PageSeq.cbegin(); pi != m_PageSeq.cend(); pi++)
		if(!(*pi)->BuildDNA(dna, headDiaMM))
			return AuDNA();				// Return empty dna on failure.
	return dna;
}
