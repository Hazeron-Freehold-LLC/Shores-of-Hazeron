//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audnawidgets.cpp,v 1.8 2022/02/19 18:20:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audnawidgets.h"
//
#include "aucheckbox.h"
#include "audna.h"
#include "audnawizard.h"
#include "auglobal.h"
#include "auhaircolorwidget.h"
#include "aupushbutton.h"
#include "auskincolorwidget.h"
#include "austylesheet.h"
#include "autoolbutton.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QSplitter>
#include <QStackedLayout>
#include <QVBoxLayout>

AuGeneDialog::AuGeneDialog(AuChromosome::CTYPE parentType, const AuChromoWhich& parentWhich, const AuGeneChain& genes,
		QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Genes"));
	setAuDlgIcon(QIcon(":data/Gene.png"));
	m_ApplyToBoth = false;
	m_ParentType = parentType;
	m_ParentWhich = parentWhich;
	m_Genes = genes;

	m_GeneList = new QListWidget(this);
	connect(m_GeneList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(GeneClicked(QListWidgetItem*)));
	connect(m_GeneList, SIGNAL(itemSelectionChanged()), SLOT(EnableControls()));
	ListGenes();

	m_Pages = new QStackedLayout;
	m_Pages->setSpacing(0);
	m_Pages->setMargin(0);
	m_Pages->addWidget(new AuGenePageBlank);	// Start with a blank page, for when nothing is selected.
	for(int t=0; t<AuGene::GTYPE_Count; t++)
	{
		AuGene::GTYPE tr = (AuGene::GTYPE)t;
		AuGene* trait = AuGene::CreateGene(tr);
		QWidget* p = trait->MakePropertiesPage(this);
		if(p)
		{
			m_PageIndexes.insert(tr, m_Pages->addWidget(p));
			connect(p, SIGNAL(Apply()), SLOT(ApplyGeneChanges()));
		}
		delete trait;
	}

	QPushButton* pbadd = new QPushButton(tr("Add Gene..."));
	pbadd->setToolTip(tr("<html>"
				"Adds a new gene. "
				"</html>"));
	connect(pbadd, SIGNAL(clicked()), SLOT(AddGene()));

	m_DelGene = new QPushButton(tr("Remove Gene..."));
	m_DelGene->setToolTip(tr("<html>"
				"Removes the selected gene."
				"</html>"));
	connect(m_DelGene, SIGNAL(clicked()), SLOT(DeleteGene()));

	QPushButton* pbok = new QPushButton(tr("Ok"));
	pbok->setToolTip(tr("<html>"
				"Closes this window and applies the changes to the parent chromatid. "
				"</html>"));
	pbok->setDefault(true);
	pbok->setAutoDefault(true);
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QPushButton* pbokboth = new QPushButton(tr("Ok for Both α and β Chromatids"));
	pbokboth->setToolTip(tr("<html>"
				"Closes this window and applies the changes to both chromatids of the parent chromosome. "
				"</html>"));
	connect(pbokboth, SIGNAL(clicked()), SLOT(AcceptBoth()));

	QHBoxLayout* tlay = new QHBoxLayout;
	tlay->addStretch();
	tlay->addWidget(pbadd);
	tlay->addWidget(m_DelGene);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);
	blay->addWidget(pbokboth);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(tlay);
	lay->addWidget(m_GeneList, 1);
	lay->addLayout(m_Pages);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuGeneDialog::~AuGeneDialog()
{
}

void AuGeneDialog::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
		EnableControls();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuGeneDialog::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GeneDialog/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GeneDialog/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/GeneDialog/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/GeneDialog/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuGeneDialog::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GeneDialog/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GeneDialog/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/GeneDialog/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/GeneDialog/SizeY",sz.height());
}

void AuGeneDialog::EnableControls()
{
	m_DelGene->setEnabled(!m_GeneList->selectedItems().empty());
}

void AuGeneDialog::AcceptBoth()
{
	m_ApplyToBoth = true;
	accept();
}

void AuGeneDialog::GeneClicked(QListWidgetItem* item)
{
	int index = item->data(Qt::UserRole).toInt();
	const AuGene* gene = m_Genes[index];
	if(!gene)
		return;

	QMap<AuGene::GTYPE, int>::const_iterator pi = m_PageIndexes.find(gene->Type());
	if(pi == m_PageIndexes.cend())
	{
		qWarning() << "error: AuGeneDialog::GeneClicked no page for" << gene->ListWidgetText();
		return;
	}
	m_Pages->setCurrentIndex(*pi);
	AuGenePageBase* curPage = dynamic_cast<AuGenePageBase*>(m_Pages->currentWidget());
	if(curPage)
		curPage->ShowProperties(m_ParentType, m_ParentWhich, gene);
}

void AuGeneDialog::ApplyGeneChanges()
{
	AuGenePageBase* curPage = dynamic_cast<AuGenePageBase*>(m_Pages->currentWidget());
	if(!curPage)
		return;
	QListWidgetItem* item = m_GeneList->currentItem();
	if(!item)
		return;
	int index = item->data(Qt::UserRole).toInt();
	if(index < 0 || index >= m_Genes.size())
		return;
	AuGeneChain::iterator ci = m_Genes.begin() + index;
	if(!*ci)
		return;
	if(curPage->ApplyProperties(*ci))
		item->setText((*ci)->ListWidgetText());
}

void AuGeneDialog::ListGenes()
{
	m_GeneList->clear();
	int index = 0;
	for(AuGeneChain::const_iterator si = m_Genes.cbegin(); si != m_Genes.cend(); si++)
	{
		QListWidgetItem* item = new QListWidgetItem(QIcon(":data/Gene.png"), (*si)->ListWidgetText(), m_GeneList);
		item->setData(Qt::UserRole, index);
		index++;
	}
}

void AuGeneDialog::AddGene()
{
	QMap<QString, AuGene::GTYPE> addable;
	m_Genes.AddableGenes(addable);
	if(addable.empty())
	{
		QString smsg = QObject::tr("No additional genes can be added to the chromatid.");
		QMessageBox::information(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok);
		return;
	}

	QStringList addNames = addable.keys();
	bool ok;
	QString addName = QInputDialog::getItem(this, tr("Add Gene"),
            tr("<html>"
                "Choose a gene to add to the chromatid. "
                "</html>"), addNames, 0, false, &ok);
	if(!ok)
		return;
	QMap<QString, AuGene::GTYPE>::const_iterator ai = addable.find(addName);
	if(ai == addable.cend())
		return;	// wut?

	// Add the gene.
	m_Genes.AddGene(*ai);
	m_Pages->setCurrentIndex(0);
	ListGenes();
}

void AuGeneDialog::DeleteGene()
{
	QListWidgetItem* item = m_GeneList->currentItem();
	if(!item)
		return;
	int index = item->data(Qt::UserRole).toInt();
	if(index < 0 || index >= m_Genes.size())
		return;
	AuGeneChain::iterator ci = m_Genes.begin() + index;
	if(*ci)
	{
		QString smsg = tr("<html>Confirm delete of gene <b>%1</b>."
				"</html>").arg((*ci)->ListWidgetText());
		if(QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok)
			return;
		delete *ci;
		m_Genes.erase(ci);
	}
	m_Pages->setCurrentIndex(0);		// Blank page
	ListGenes();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromosomeWidget::AuChromosomeWidget(const AuChromosome* alpha, const AuChromosome* beta, QWidget* parent) : QWidget(parent)
{
	Q_ASSERT(alpha->Type() == beta->Type());
	Q_ASSERT(alpha->Which() == beta->Which());
	m_Selected = false;
	m_AlphaChromosome = alpha->Clone();
	m_BetaChromosome = beta->Clone();

	m_Delete = new QToolButton(this);
	m_Delete->setIcon(QIcon(":data/Chromosome.png"));
	m_Delete->setToolTip(QObject::tr("<html>"
				"<b>Delete...</b>"
				"<br>"
				"Removes the chromosome. "
				"</html>"));
	connect(m_Delete, SIGNAL(clicked()), SLOT(DeleteChromosome()));

	m_BetaLabel = new QLabel(beta->ListWidgetText());
	m_BetaLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_BetaLabel->setWordWrap(true);

	m_AlphaLabel = new QLabel(alpha->ListWidgetText());
	m_AlphaLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	m_AlphaLabel->setWordWrap(true);

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(m_AlphaLabel);
	lay->addWidget(m_Delete);
	lay->addWidget(m_BetaLabel);
	lay->setSpacing(0);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromosomeWidget::~AuChromosomeWidget()
{
	if(m_AlphaChromosome)
		delete m_AlphaChromosome;
	if(m_BetaChromosome)
		delete m_BetaChromosome;
}

void AuChromosomeWidget::DeleteChromosome()
{
	//qDebug() << "AuChromosomeWidget::DeleteChromosome";
	QString chromosome = m_AlphaLabel->text();
	int bri = chromosome.indexOf("<br>");
	if(bri > 0)
		chromosome = chromosome.left(bri);

	QString bchromosome = m_BetaLabel->text();
	bri = bchromosome.indexOf("<br>");
	if(bri > 0)
		bchromosome = bchromosome.left(bri);

	if(chromosome != bchromosome)
		chromosome += " | " + bchromosome;

#if defined(QT_NO_DEBUG)	//DHH animalia
	QString smsg = tr("<html>Delete of <b>%1</b> chromosome is currently disabled. This is an unfinished feature."
			"</html>").arg(chromosome);
	QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok, QMessageBox::Cancel);
#else
	QString smsg = tr("<html>Confirm delete of <b>%1</b> chromosome."
			"</html>").arg(chromosome);
	if(QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok)
		return;
	emit Deleted(this);
#endif
}

void AuChromosomeWidget::mousePressEvent(QMouseEvent* e)
{
	QWidget::mousePressEvent(e);
	emit Clicked(this);
}

void AuChromosomeWidget::SetSelected(bool sel)
{
	m_Selected = sel;
	QString selStyle;
	if(sel)
	{
		QPalette pal;
		AuStyleSheet::MakePalette(pal);
		selStyle = QString("color: %1; background: %2")
			.arg(pal.color(QPalette::Active, QPalette::HighlightedText).name())
			.arg(pal.color(QPalette::Active, QPalette::Highlight).name());
	}
	setStyleSheet(selStyle);
}

void AuChromosomeWidget::ChromosomesChanged()
{
	m_AlphaLabel->setText(m_AlphaChromosome->ListWidgetText());
	m_BetaLabel->setText(m_BetaChromosome->ListWidgetText());
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageBase::AuChromoPageBase(QWidget* parent) : QWidget(parent)
{
}

AuChromoPageBase::~AuChromoPageBase()
{
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageBlank::AuChromoPageBlank(QWidget* parent) : AuChromoPageBase(parent)
{
}

AuChromoPageBlank::~AuChromoPageBlank()
{
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageChordata::AuChromoPageChordata(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Chordata</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_ASkel = new QComboBox;
		for(int s=AuChromoChordata::Bones; s <= AuChromoChordata::Cartilage; s++)
			m_ASkel->addItem(AuChromoChordata::SkeletonName((AuChromoChordata::SKELETON)s), s);
		m_ASkel->setToolTip(QObject::tr("<html>"
					"Skeleton"
					"</html>"));
		connect(m_ASkel, SIGNAL(currentIndexChanged(int)), SLOT(ASkeletonChanged(int)));

		QVBoxLayout* glay = new QVBoxLayout;
		glay->addWidget(m_ASkel);
		glay->addStretch();
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BSkel = new QComboBox;
		for(int s=AuChromoChordata::Bones; s <= AuChromoChordata::Cartilage; s++)
			m_BSkel->addItem(AuChromoChordata::SkeletonName((AuChromoChordata::SKELETON)s), s);
		m_BSkel->setToolTip(QObject::tr("<html>"
					"Skeleton"
					"</html>"));
		connect(m_BSkel, SIGNAL(currentIndexChanged(int)), SLOT(BSkeletonChanged(int)));

		QVBoxLayout* glay = new QVBoxLayout;
		glay->addWidget(m_BSkel);
		glay->addStretch();
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageChordata::~AuChromoPageChordata()
{
}

void AuChromoPageChordata::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoChordata* at = dynamic_cast<const AuChromoChordata*>(alpha);
	const AuChromoChordata* bt = dynamic_cast<const AuChromoChordata*>(beta);
	if(!at || !bt)
		return;

	m_ASkel->setCurrentIndex(m_ASkel->findData(at->Skeleton()));

	m_BSkel->setCurrentIndex(m_BSkel->findData(bt->Skeleton()));
}

int AuChromoPageChordata::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoChordata* at = dynamic_cast<AuChromoChordata*>(alpha);
	AuChromoChordata* bt = dynamic_cast<AuChromoChordata*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;

	AuChromoChordata::SKELETON ask = (AuChromoChordata::SKELETON)m_ASkel->currentData().toInt();
	if(at->SetSkeleton(ask))
		achg = true;

	AuChromoChordata::SKELETON bsk = (AuChromoChordata::SKELETON)m_BSkel->currentData().toInt();
	if(bt->SetSkeleton(bsk))
		bchg = true;

	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

void AuChromoPageChordata::ASkeletonChanged(int index)
{
	// Keep skeletons in sync.
	m_BSkel->blockSignals(true);
	m_BSkel->setCurrentIndex(index);
	m_BSkel->blockSignals(false);
	EnableControls();
}

void AuChromoPageChordata::BSkeletonChanged(int index)
{
	// Keep skeletons in sync.
	m_ASkel->blockSignals(true);
	m_ASkel->setCurrentIndex(index);
	m_ASkel->blockSignals(false);
	EnableControls();
}

void AuChromoPageChordata::EnableControls()
{
	bool applyOk = true;
	if(m_ASkel->currentData() != m_BSkel->currentData())
		applyOk = false;
	m_Apply->setEnabled(applyOk);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageMollusca::AuChromoPageMollusca(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Mollusca</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_ACover = new QComboBox;
		for(int s=AuChromoMollusca::Mantle; s <= AuChromoMollusca::Shell; s++)
			m_ACover->addItem(AuChromoMollusca::CoverName((AuChromoMollusca::COVER)s), s);
		m_ACover->setToolTip(QObject::tr("<html>"
					"Cover"
					"</html>"));
		connect(m_ACover, SIGNAL(currentIndexChanged(int)), SLOT(ACoverChanged(int)));

		QVBoxLayout* glay = new QVBoxLayout;
		glay->addWidget(m_ACover);
		glay->addStretch();
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BCover = new QComboBox;
		for(int s=AuChromoMollusca::Mantle; s <= AuChromoMollusca::Shell; s++)
			m_BCover->addItem(AuChromoMollusca::CoverName((AuChromoMollusca::COVER)s), s);
		m_BCover->setToolTip(QObject::tr("<html>"
					"Cover"
					"</html>"));
		connect(m_BCover, SIGNAL(currentIndexChanged(int)), SLOT(BCoverChanged(int)));

		QVBoxLayout* glay = new QVBoxLayout;
		glay->addWidget(m_BCover);
		glay->addStretch();
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageMollusca::~AuChromoPageMollusca()
{
}

void AuChromoPageMollusca::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoMollusca* at = dynamic_cast<const AuChromoMollusca*>(alpha);
	const AuChromoMollusca* bt = dynamic_cast<const AuChromoMollusca*>(beta);
	if(!at || !bt)
		return;

	m_ACover->setCurrentIndex(m_ACover->findData(at->Cover()));

	m_BCover->setCurrentIndex(m_BCover->findData(bt->Cover()));
}

int AuChromoPageMollusca::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoMollusca* at = dynamic_cast<AuChromoMollusca*>(alpha);
	AuChromoMollusca* bt = dynamic_cast<AuChromoMollusca*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;

	AuChromoMollusca::COVER ask = (AuChromoMollusca::COVER)m_ACover->currentData().toInt();
	if(at->SetCover(ask))
		achg = true;

	AuChromoMollusca::COVER bsk = (AuChromoMollusca::COVER)m_BCover->currentData().toInt();
	if(bt->SetCover(bsk))
		bchg = true;

	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

void AuChromoPageMollusca::ACoverChanged(int index)
{
	Q_UNUSED(index);
	EnableControls();
}

void AuChromoPageMollusca::BCoverChanged(int index)
{
	Q_UNUSED(index);
	EnableControls();
}

void AuChromoPageMollusca::EnableControls()
{
	bool applyOk = true;
	m_Apply->setEnabled(applyOk);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageAnnelida::AuChromoPageAnnelida(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Annelida</b></html>"));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageAnnelida::~AuChromoPageAnnelida()
{
}

void AuChromoPageAnnelida::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoAnnelida* at = dynamic_cast<const AuChromoAnnelida*>(alpha);
	const AuChromoAnnelida* bt = dynamic_cast<const AuChromoAnnelida*>(beta);
	if(!at || !bt)
		return;

}

int AuChromoPageAnnelida::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoAnnelida* at = dynamic_cast<AuChromoAnnelida*>(alpha);
	AuChromoAnnelida* bt = dynamic_cast<AuChromoAnnelida*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;


	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

void AuChromoPageAnnelida::EnableControls()
{
	bool applyOk = true;
	m_Apply->setEnabled(applyOk);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageArthropoda::AuChromoPageArthropoda(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Arthropoda</b></html>"));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageArthropoda::~AuChromoPageArthropoda()
{
}

void AuChromoPageArthropoda::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoArthropoda* at = dynamic_cast<const AuChromoArthropoda*>(alpha);
	const AuChromoArthropoda* bt = dynamic_cast<const AuChromoArthropoda*>(beta);
	if(!at || !bt)
		return;

}

int AuChromoPageArthropoda::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoArthropoda* at = dynamic_cast<AuChromoArthropoda*>(alpha);
	AuChromoArthropoda* bt = dynamic_cast<AuChromoArthropoda*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;


	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageSkin::AuChromoPageSkin(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Skin</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_ASkinTop = new AuSkinColorWidget;
		m_ASkinTop->setToolTip(QObject::tr("<html>"
					"Color of top skin."
					"</html>"));

		m_ATopToBottom = new QPushButton(tr("Copy Top to Bottom"), this);
		m_ATopToBottom->setToolTip(QObject::tr("<html>"
					"Copies the top skin color to the bottom skin color."
					"</html>"));
		connect(m_ATopToBottom, SIGNAL(clicked()), SLOT(ATopToBottomClicked()));

		m_ASkinBottom = new AuSkinColorWidget;
		m_ASkinBottom->setToolTip(QObject::tr("<html>"
					"Color of bottom skin."
					"</html>"));

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_ASkinTop, row++,0, 1,2);
		glay->addWidget(m_ATopToBottom, row++,0, 1,2);
		glay->addWidget(m_ASkinBottom, row++,0, 1,2);
		row++;
		glay->setRowStretch(row, 1);
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BSkinTop = new AuSkinColorWidget;
		m_BSkinTop->setToolTip(QObject::tr("<html>"
					"Color of top skin."
					"</html>"));

		m_BTopToBottom = new QPushButton(tr("Copy Top to Bottom"), this);
		m_BTopToBottom->setToolTip(QObject::tr("<html>"
					"Copies the top skin color to the bottom skin color."
					"</html>"));
		connect(m_BTopToBottom, SIGNAL(clicked()), SLOT(BTopToBottomClicked()));

		m_BSkinBottom = new AuSkinColorWidget;
		m_BSkinBottom->setToolTip(QObject::tr("<html>"
					"Color of bottom skin."
					"</html>"));

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_BSkinTop, row++,0, 1,2);
		glay->addWidget(m_BTopToBottom, row++,0, 1,2);
		glay->addWidget(m_BSkinBottom, row++,0, 1,2);
		row++;
		glay->setRowStretch(row, 1);
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageSkin::~AuChromoPageSkin()
{
}

void AuChromoPageSkin::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoSkin* at = dynamic_cast<const AuChromoSkin*>(alpha);
	const AuChromoSkin* bt = dynamic_cast<const AuChromoSkin*>(beta);
	if(!at || !bt)
		return;

	m_ASkinBottom->SetSkinColor(at->BottomColor());
	m_ASkinTop->SetSkinColor(at->TopColor());

	m_BSkinBottom->SetSkinColor(bt->BottomColor());
	m_BSkinTop->SetSkinColor(bt->TopColor());
}

int AuChromoPageSkin::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoSkin* at = dynamic_cast<AuChromoSkin*>(alpha);
	AuChromoSkin* bt = dynamic_cast<AuChromoSkin*>(beta);
	if(!at || !bt)
		return -1;

	// Color changes only affect shader parameters. The model does not have to be regenerated.

	if(m_ASkinBottom->SkinColor().value() >= 9)
		at->SetBottomColor(m_ASkinBottom->SkinColor());
	if(m_ASkinTop->SkinColor().value() >= 9)
		at->SetTopColor(m_ASkinTop->SkinColor());

	// Color changes only affect shader parameters. The model does not have to be regenerated.
	if(m_BSkinBottom->SkinColor().value() >= 9)
		bt->SetBottomColor(m_BSkinBottom->SkinColor());
	if(m_BSkinTop->SkinColor().value() >= 9)
		bt->SetTopColor(m_BSkinTop->SkinColor());

	return 0;
}

void AuChromoPageSkin::ATopToBottomClicked()
{
	m_ASkinBottom->SetSkinColor(m_ASkinTop->SkinColor());
}

void AuChromoPageSkin::BTopToBottomClicked()
{
	m_BSkinBottom->SetSkinColor(m_BSkinTop->SkinColor());
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageFeathers::AuChromoPageFeathers(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Feathers</b></html>"));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageFeathers::~AuChromoPageFeathers()
{
}

void AuChromoPageFeathers::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoFeathers* at = dynamic_cast<const AuChromoFeathers*>(alpha);
	const AuChromoFeathers* bt = dynamic_cast<const AuChromoFeathers*>(beta);
	if(!at || !bt)
		return;

}

int AuChromoPageFeathers::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoFeathers* at = dynamic_cast<AuChromoFeathers*>(alpha);
	AuChromoFeathers* bt = dynamic_cast<AuChromoFeathers*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;


	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageFur::AuChromoPageFur(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Fur</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_AColorTop = new AuHairColorWidget;
		m_AColorTop->setToolTip(QObject::tr("<html>"
					"Color of top fur."
					"</html>"));

		m_ATopToBottom = new QPushButton(tr("Copy Top to Bottom"), this);
		m_ATopToBottom->setToolTip(QObject::tr("<html>"
					"Copies the top fur color to the bottom fur color."
					"</html>"));
		connect(m_ATopToBottom, SIGNAL(clicked()), SLOT(ATopToBottomClicked()));

		m_AColorBottom = new AuHairColorWidget;
		m_AColorBottom->setToolTip(QObject::tr("<html>"
					"Color of bottom fur."
					"</html>"));

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_AColorTop, row++,0, 1,2);
		glay->addWidget(m_ATopToBottom, row++,0, 1,2);
		glay->addWidget(m_AColorBottom, row++,0, 1,2);
		row++;
		glay->setRowStretch(row, 1);
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BColorTop = new AuHairColorWidget;
		m_BColorTop->setToolTip(QObject::tr("<html>"
					"Color of top fur."
					"</html>"));

		m_BTopToBottom = new QPushButton(tr("Copy Top to Bottom"), this);
		m_BTopToBottom->setToolTip(QObject::tr("<html>"
					"Copies the top fur color to the bottom fur color."
					"</html>"));
		connect(m_BTopToBottom, SIGNAL(clicked()), SLOT(BTopToBottomClicked()));

		m_BColorBottom = new AuHairColorWidget;
		m_BColorBottom->setToolTip(QObject::tr("<html>"
					"Color of bottom fur."
					"</html>"));

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_BColorTop, row++,0, 1,2);
		glay->addWidget(m_BTopToBottom, row++,0, 1,2);
		glay->addWidget(m_BColorBottom, row++,0, 1,2);
		row++;
		glay->setRowStretch(row, 1);
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageFur::~AuChromoPageFur()
{
}

void AuChromoPageFur::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoFur* at = dynamic_cast<const AuChromoFur*>(alpha);
	const AuChromoFur* bt = dynamic_cast<const AuChromoFur*>(beta);
	if(!at || !bt)
		return;

	m_AColorBottom->SetHairColor(at->BottomColor());
	m_AColorTop->SetHairColor(at->TopColor());

	m_BColorBottom->SetHairColor(bt->BottomColor());
	m_BColorTop->SetHairColor(bt->TopColor());
}

int AuChromoPageFur::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoFur* at = dynamic_cast<AuChromoFur*>(alpha);
	AuChromoFur* bt = dynamic_cast<AuChromoFur*>(beta);
	if(!at || !bt)
		return -1;

	// Color changes only affect shader parameters. The model does not have to be regenerated.

	if(m_AColorBottom->HairColor().value() >= 9)
		at->SetBottomColor(m_AColorBottom->HairColor());
	if(m_AColorTop->HairColor().value() >= 9)
		at->SetTopColor(m_AColorTop->HairColor());

	// Color changes only affect shader parameters. The model does not have to be regenerated.
	if(m_BColorBottom->HairColor().value() >= 9)
		bt->SetBottomColor(m_BColorBottom->HairColor());
	if(m_BColorTop->HairColor().value() >= 9)
		bt->SetTopColor(m_BColorTop->HairColor());

	return 0;
}

void AuChromoPageFur::ATopToBottomClicked()
{
	m_AColorBottom->SetHairColor(m_AColorTop->HairColor());
}

void AuChromoPageFur::BTopToBottomClicked()
{
	m_BColorBottom->SetHairColor(m_BColorTop->HairColor());
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageScales::AuChromoPageScales(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Scales</b></html>"));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageScales::~AuChromoPageScales()
{
}

void AuChromoPageScales::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoScales* at = dynamic_cast<const AuChromoScales*>(alpha);
	const AuChromoScales* bt = dynamic_cast<const AuChromoScales*>(beta);
	if(!at || !bt)
		return;

}

int AuChromoPageScales::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoScales* at = dynamic_cast<AuChromoScales*>(alpha);
	AuChromoScales* bt = dynamic_cast<AuChromoScales*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;


	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageHead::AuChromoPageHead(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(tr("<html><b>Head</b></html>"));

	m_Apply = new QPushButton(tr("Apply"));
	m_Apply->setToolTip(tr("<html>"
				"Saves chromosome changes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_AGeneButton = new AuIconButton(":data/Gene.png");
		m_AGeneButton->setToolTip(tr("<html>"
					"Genes"
					"</html>"));
		connect(m_AGeneButton, SIGNAL(clicked()), SLOT(AGenes()));

		m_AAttach = new QComboBox;
		m_AAttach->addItem(tr("Attach at Bottom"), AuChromoHead::AttachBottom);
		m_AAttach->addItem(tr("Attach at Back"), AuChromoHead::AttachBack);
		m_AAttach->setToolTip(tr("<html>"
					"Attachment location on head."
					"</html>"));

		m_AAttachDist = new QSpinBox;
		m_AAttachDist->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AAttachDist->setRange(0, d_DNAMaxMM);
		m_AAttachDist->setToolTip(tr("<html>"
					"Attachment distance from center of head, in mm. "
					"0 attaches at the center of the head. "
					"</html>"));
		QLabel* lbatt = new QLabel("mm From Center");

		m_AWidth = new QSpinBox;
		m_AWidth->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AWidth->setRange(1, d_DNAMaxMM);
		m_AWidth->setToolTip(tr("<html>"
					"Side to side size of head, in mm. "
					"</html>"));
		QLabel* lbwid = new QLabel("mm Width");

		m_ALength = new QSpinBox;
		m_ALength->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ALength->setRange(1, d_DNAMaxMM);
		m_ALength->setToolTip(tr("<html>"
					"Back to front size of head, in mm. "
					"</html>"));
		QLabel* lblen = new QLabel("mm Length");

		m_AHeight = new QSpinBox;
		m_AHeight->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AHeight->setRange(1, d_DNAMaxMM);
		m_AHeight->setToolTip(tr("<html>"
					"Bottom to top size of head, in mm. "
					"</html>"));
		QLabel* lbht = new QLabel("mm Height");

		m_AEyeAng = new QSpinBox;
		m_AEyeAng->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AEyeAng->setRange(0, 180);
		m_AEyeAng->setToolTip(tr("<html>"
					"Angle from front of head to eyes, in degrees. "
					"0 is front of head; 90 is side of head. "
					"</html>"));
		QLabel* lbeyeang = new QLabel(tr("%1 Eye Angle").arg(AuGlobal::DegreeSymbol()));

		m_AEarAng = new QSpinBox;
		m_AEarAng->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AEarAng->setRange(0, 180);
		m_AEarAng->setToolTip(tr("<html>"
					"Angle from top of head to ears, in degrees. "
					"0 is top of head; 90 is side of head. "
					"</html>"));
		QLabel* lbearang = new QLabel(tr("%1 Ear Angle").arg(AuGlobal::DegreeSymbol()));

		m_ANoseAng = new QSpinBox;
		m_ANoseAng->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ANoseAng->setRange(0, 180);
		m_ANoseAng->setToolTip(tr("<html>"
					"Angle from top of head to nose, in degrees. "
					"0 is top of head; 90 is front of head. "
					"</html>"));
		QLabel* lbnoseang = new QLabel(tr("%1 Nose Angle").arg(AuGlobal::DegreeSymbol()));

		m_AMouthAng = new QSpinBox;
		m_AMouthAng->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AMouthAng->setRange(0, 180);
		m_AMouthAng->setToolTip(tr("<html>"
					"Angle from top of head to mouth, in degrees. "
					"0 is top of head; 90 is front of head. "
					"</html>"));
		QLabel* lbmouthang = new QLabel(tr("%1 Mouth Angle").arg(AuGlobal::DegreeSymbol()));

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_AAttach, row,0, 1,2);
		glay->addWidget(m_AGeneButton, row,2);
		row++;
		glay->addWidget(m_AAttachDist, row,0);
		glay->addWidget(lbatt, row,1);
		row++;
		glay->addWidget(m_AWidth, row,0);
		glay->addWidget(lbwid, row,1);
		row++;
		glay->addWidget(m_ALength, row,0);
		glay->addWidget(lblen, row,1);
		row++;
		glay->addWidget(m_AHeight, row,0);
		glay->addWidget(lbht, row,1);
		row++;
		glay->addWidget(m_AEyeAng, row,0);
		glay->addWidget(lbeyeang, row,1);
		row++;
		glay->addWidget(m_AEarAng, row,0);
		glay->addWidget(lbearang, row,1);
		row++;
		glay->addWidget(m_ANoseAng, row,0);
		glay->addWidget(lbnoseang, row,1);
		row++;
		glay->addWidget(m_AMouthAng, row,0);
		glay->addWidget(lbmouthang, row,1);
		row++;
		glay->setRowStretch(row, 1);
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BGeneButton = new AuIconButton(":data/Gene.png");
		m_BGeneButton->setToolTip(tr("<html>"
					"Genes"
					"</html>"));
		connect(m_BGeneButton, SIGNAL(clicked()), SLOT(BGenes()));

		m_BAttach = new QComboBox;
		m_BAttach->addItem(tr("Attach at Bottom"), AuChromoHead::AttachBottom);
		m_BAttach->addItem(tr("Attach at Back"), AuChromoHead::AttachBack);
		m_BAttach->setToolTip(tr("<html>"
					"Attachment location on head."
					"</html>"));

		QLabel* lbatt = new QLabel("mm From Center");
		m_BAttachDist = new QSpinBox;
		m_BAttachDist->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BAttachDist->setRange(0, d_DNAMaxMM);
		m_BAttachDist->setToolTip(tr("<html>"
					"Attachment distance from center of head, in mm. "
					"0 attaches at the center of the head. "
					"</html>"));

		m_BWidth = new QSpinBox;
		m_BWidth->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BWidth->setRange(1, d_DNAMaxMM);
		m_BWidth->setToolTip(tr("<html>"
					"Side to side size of head, in mm. "
					"</html>"));
		QLabel* lbwid = new QLabel("mm Width");

		m_BLength = new QSpinBox;
		m_BLength->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BLength->setRange(1, d_DNAMaxMM);
		m_BLength->setToolTip(tr("<html>"
					"Back to front size of head, in mm. "
					"</html>"));
		QLabel* lblen = new QLabel("mm Length");

		m_BHeight = new QSpinBox;
		m_BHeight->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BHeight->setRange(1, d_DNAMaxMM);
		m_BHeight->setToolTip(tr("<html>"
					"Bottom to top size of head, in mm. "
					"</html>"));
		QLabel* lbht = new QLabel("mm Height");

		m_BEyeAng = new QSpinBox;
		m_BEyeAng->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BEyeAng->setRange(0, 180);
		m_BEyeAng->setToolTip(tr("<html>"
					"Angle from front of head to eyes, in degrees. "
					"0 is front of head; 90 is side of head. "
					"</html>"));
		QLabel* lbeyeang = new QLabel(tr("%1 Eye Angle").arg(AuGlobal::DegreeSymbol()));

		m_BEarAng = new QSpinBox;
		m_BEarAng->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BEarAng->setRange(0, 180);
		m_BEarAng->setToolTip(tr("<html>"
					"Angle from top of head to ears, in degrees. "
					"0 is top of head; 90 is side of head. "
					"</html>"));
		QLabel* lbearang = new QLabel(tr("%1 Ear Angle").arg(AuGlobal::DegreeSymbol()));

		m_BNoseAng = new QSpinBox;
		m_BNoseAng->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BNoseAng->setRange(0, 180);
		m_BNoseAng->setToolTip(tr("<html>"
					"Angle from top of head to nose, in degrees. "
					"0 is top of head; 90 is front of head. "
					"</html>"));
		QLabel* lbnoseang = new QLabel(tr("%1 Nose Angle").arg(AuGlobal::DegreeSymbol()));

		m_BMouthAng = new QSpinBox;
		m_BMouthAng->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BMouthAng->setRange(0, 180);
		m_BMouthAng->setToolTip(tr("<html>"
					"Angle from top of head to mouth, in degrees. "
					"0 is top of head; 90 is front of head. "
					"</html>"));
		QLabel* lbmouthang = new QLabel(tr("%1 Mouth Angle").arg(AuGlobal::DegreeSymbol()));

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_BAttach, row,0, 1,2);
		glay->addWidget(m_BGeneButton, row,2);
		row++;
		glay->addWidget(m_BAttachDist, row,0);
		glay->addWidget(lbatt, row,1);
		row++;
		glay->addWidget(m_BWidth, row,0);
		glay->addWidget(lbwid, row,1);
		row++;
		glay->addWidget(m_BLength, row,0);
		glay->addWidget(lblen, row,1);
		row++;
		glay->addWidget(m_BHeight, row,0);
		glay->addWidget(lbht, row,1);
		row++;
		glay->addWidget(m_BEyeAng, row,0);
		glay->addWidget(lbeyeang, row,1);
		row++;
		glay->addWidget(m_BEarAng, row,0);
		glay->addWidget(lbearang, row,1);
		row++;
		glay->addWidget(m_BNoseAng, row,0);
		glay->addWidget(lbnoseang, row,1);
		row++;
		glay->addWidget(m_BMouthAng, row,0);
		glay->addWidget(lbmouthang, row,1);
		row++;
		glay->setRowStretch(row, 1);
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageHead::~AuChromoPageHead()
{
}

void AuChromoPageHead::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoHead* at = dynamic_cast<const AuChromoHead*>(alpha);
	const AuChromoHead* bt = dynamic_cast<const AuChromoHead*>(beta);
	if(!at || !bt)
		return;
	if(at->Genes())
		m_AGenes = *at->Genes();
	else
		m_AGenes.Clear();
	if(bt->Genes())
		m_BGenes = *bt->Genes();
	else
		m_BGenes.Clear();

	m_AAttach->setCurrentIndex(m_AAttach->findData(at->Attachment()));
	m_AAttachDist->setValue(at->AttachDistanceMM());
	m_AWidth->setValue(at->WidthMM());
	m_ALength->setValue(at->LengthMM());
	m_AHeight->setValue(at->HeightMM());
	m_AEarAng->setValue(at->EarAngleDeg());
	m_AEyeAng->setValue(at->EyeAngleDeg());
	m_AMouthAng->setValue(at->MouthAngleDeg());
	m_ANoseAng->setValue(at->NoseAngleDeg());

	m_BAttach->setCurrentIndex(m_BAttach->findData(bt->Attachment()));
	m_BAttachDist->setValue(bt->AttachDistanceMM());
	m_BWidth->setValue(bt->WidthMM());
	m_BLength->setValue(bt->LengthMM());
	m_BHeight->setValue(bt->HeightMM());
	m_BEarAng->setValue(bt->EarAngleDeg());
	m_BEyeAng->setValue(bt->EyeAngleDeg());
	m_BMouthAng->setValue(bt->MouthAngleDeg());
	m_BNoseAng->setValue(bt->NoseAngleDeg());
}

int AuChromoPageHead::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoHead* at = dynamic_cast<AuChromoHead*>(alpha);
	AuChromoHead* bt = dynamic_cast<AuChromoHead*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;

	AuChromoHead::ATTACHMENT aat = (AuChromoHead::ATTACHMENT)m_AAttach->currentData().toInt();
	if(at->SetAttachment(aat))
		achg = true;
	quint16 aad = m_AAttachDist->value();
	if(at->SetAttachDistanceMM(aad))
		achg = true;
	quint16 arx = m_AWidth->value();
	if(at->SetWidthMM(arx))
		achg = true;
	quint16 ary = m_ALength->value();
	if(at->SetLengthMM(ary))
		achg = true;
	quint16 arz = m_AHeight->value();
	if(at->SetHeightMM(arz))
		achg = true;
	qint16 aea = m_AEarAng->value();
	if(at->SetEarAngleDeg(aea))
		achg = true;
	qint16 aya = m_AEyeAng->value();
	if(at->SetEyeAngleDeg(aya))
		achg = true;
	qint16 ama = m_AMouthAng->value();
	if(at->SetMouthAngleDeg(ama))
		achg = true;
	qint16 ana = m_ANoseAng->value();
	if(at->SetNoseAngleDeg(ana))
		achg = true;
	if(at->SetGenes(m_AGenes))
		achg = true;

	AuChromoHead::ATTACHMENT bat = (AuChromoHead::ATTACHMENT)m_BAttach->currentData().toInt();
	if(bt->SetAttachment(bat))
		bchg = true;
	quint16 bad = m_BAttachDist->value();
	if(bt->SetAttachDistanceMM(bad))
		bchg = true;
	quint16 brx = m_BWidth->value();
	if(bt->SetWidthMM(brx))
		bchg = true;
	quint16 bry = m_BLength->value();
	if(bt->SetLengthMM(bry))
		bchg = true;
	quint16 brz = m_BHeight->value();
	if(bt->SetHeightMM(brz))
		bchg = true;
	qint16 bea = m_BEarAng->value();
	if(bt->SetEarAngleDeg(bea))
		bchg = true;
	qint16 bya = m_BEyeAng->value();
	if(bt->SetEyeAngleDeg(bya))
		bchg = true;
	qint16 bma = m_BMouthAng->value();
	if(bt->SetMouthAngleDeg(bma))
		bchg = true;
	qint16 bna = m_BNoseAng->value();
	if(bt->SetNoseAngleDeg(bna))
		bchg = true;
	if(bt->SetGenes(m_BGenes))
		bchg = true;

	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

void AuChromoPageHead::AGenes()
{
	AuGeneDialog dlg(AuChromosome::Head, AuChromoWhich(), m_AGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_AGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_BGenes = dlg.Genes();
}

void AuChromoPageHead::BGenes()
{
	AuGeneDialog dlg(AuChromosome::Head, AuChromoWhich(), m_BGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_BGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_AGenes = dlg.Genes();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageTorso::AuChromoPageTorso(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Torso</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_AGeneButton = new AuIconButton(":data/Gene.png");
		m_AGeneButton->setToolTip(tr("<html>"
					"Genes"
					"</html>"));
		connect(m_AGeneButton, SIGNAL(clicked()), SLOT(AGenes()));

		m_ANeckLen = new QSpinBox;
		m_ANeckLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ANeckLen->setRange(1, d_DNAMaxMM);
		m_ANeckLen->setToolTip(tr("<html>"
					"Length of neck, in mm. "
					"</html>"));
		QLabel* lbNeckLen = new QLabel("mm Neck Length");

		m_AUpperLen = new QSpinBox;
		m_AUpperLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AUpperLen->setRange(0, d_DNAMaxMM);
		m_AUpperLen->setToolTip(tr("<html>"
					"Length of upper torso, in mm. "
					"The sum of the upper and lower torso lengths may not be zero. "
					"</html>"));
		QLabel* lbUpperLen = new QLabel("mm Upper Length");

		m_AUpperWid = new QSpinBox;
		m_AUpperWid->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AUpperWid->setRange(1, d_DNAMaxMM);
		m_AUpperWid->setToolTip(tr("<html>"
					"Width of upper torso, in mm. "
					"</html>"));
		QLabel* lbUpperWid = new QLabel("mm Upper Width");

		m_AUpperThick = new QSpinBox;
		m_AUpperThick->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AUpperThick->setRange(1, d_DNAMaxMM);
		m_AUpperThick->setToolTip(tr("<html>"
					"Thickness of upper torso, in mm. "
					"</html>"));
		QLabel* lbUpperThick = new QLabel("mm Upper Thickness");

		m_ALowerLen = new QSpinBox;
		m_ALowerLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ALowerLen->setRange(0, d_DNAMaxMM);
		m_ALowerLen->setToolTip(tr("<html>"
					"Length of lower torso, in mm. "
					"The sum of the upper and lower torso lengths may not be zero. "
					"</html>"));
		QLabel* lbLowerLen = new QLabel("mm Lower Length");

		m_ALowerWid = new QSpinBox;
		m_ALowerWid->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ALowerWid->setRange(1, d_DNAMaxMM);
		m_ALowerWid->setToolTip(tr("<html>"
					"Width of lower torso, in mm. "
					"</html>"));
		QLabel* lbLowerWid = new QLabel("mm Lower Width");

		m_ALowerThick = new QSpinBox;
		m_ALowerThick->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ALowerThick->setRange(1, d_DNAMaxMM);
		m_ALowerThick->setToolTip(tr("<html>"
					"Thickness of lower torso, in mm. "
					"</html>"));
		QLabel* lbLowerThick = new QLabel("mm Lower Thickness");

		m_ATailLen = new QSpinBox;
		m_ATailLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ATailLen->setRange(0, d_DNAMaxMM);
		m_ATailLen->setToolTip(tr("<html>"
					"Length of tail, in mm. "
					"</html>"));
		QLabel* lbTailLen = new QLabel("mm Tail Length");

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_ANeckLen, row,0);
		glay->addWidget(lbNeckLen, row,1);
		glay->addWidget(m_AGeneButton, row,2);
		row++;
		glay->addWidget(m_AUpperLen, row,0);
		glay->addWidget(lbUpperLen, row,1);
		row++;
		glay->addWidget(m_AUpperWid, row,0);
		glay->addWidget(lbUpperWid, row,1);
		row++;
		glay->addWidget(m_AUpperThick, row,0);
		glay->addWidget(lbUpperThick, row,1);
		row++;
		glay->addWidget(m_ALowerLen, row,0);
		glay->addWidget(lbLowerLen, row,1);
		row++;
		glay->addWidget(m_ALowerWid, row,0);
		glay->addWidget(lbLowerWid, row,1);
		row++;
		glay->addWidget(m_ALowerThick, row,0);
		glay->addWidget(lbLowerThick, row,1);
		row++;
		glay->addWidget(m_ATailLen, row,0);
		glay->addWidget(lbTailLen, row,1);
		row++;
		glay->setRowStretch(row, 1);
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BGeneButton = new AuIconButton(":data/Gene.png");
		m_BGeneButton->setToolTip(tr("<html>"
					"Genes"
					"</html>"));
		connect(m_BGeneButton, SIGNAL(clicked()), SLOT(BGenes()));

		m_BNeckLen = new QSpinBox;
		m_BNeckLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BNeckLen->setRange(1, d_DNAMaxMM);
		m_BNeckLen->setToolTip(tr("<html>"
					"Length of neck, in mm. "
					"</html>"));
		QLabel* lbNeckLen = new QLabel("mm Neck Length");

		m_BUpperLen = new QSpinBox;
		m_BUpperLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BUpperLen->setRange(0, d_DNAMaxMM);
		m_BUpperLen->setToolTip(tr("<html>"
					"Length of upper torso, in mm. "
					"The sum of the upper and lower torso lengths may not be zero. "
					"</html>"));
		QLabel* lbUpperLen = new QLabel("mm Upper Length");

		m_BUpperWid = new QSpinBox;
		m_BUpperWid->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BUpperWid->setRange(1, d_DNAMaxMM);
		m_BUpperWid->setToolTip(tr("<html>"
					"Width of upper torso, in mm. "
					"</html>"));
		QLabel* lbUpperWid = new QLabel("mm Upper Width");

		m_BUpperThick = new QSpinBox;
		m_BUpperThick->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BUpperThick->setRange(1, d_DNAMaxMM);
		m_BUpperThick->setToolTip(tr("<html>"
					"Thickness of upper torso, in mm. "
					"</html>"));
		QLabel* lbUpperThick = new QLabel("mm Upper Thickness");

		m_BLowerLen = new QSpinBox;
		m_BLowerLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BLowerLen->setRange(0, d_DNAMaxMM);
		m_BLowerLen->setToolTip(tr("<html>"
					"Length of lower torso, in mm. "
					"The sum of the upper and lower torso lengths may not be zero. "
					"</html>"));
		QLabel* lbLowerLen = new QLabel("mm Lower Length");

		m_BLowerWid = new QSpinBox;
		m_BLowerWid->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BLowerWid->setRange(1, d_DNAMaxMM);
		m_BLowerWid->setToolTip(tr("<html>"
					"Width of lower torso, in mm. "
					"</html>"));
		QLabel* lbLowerWid = new QLabel("mm Lower Width");

		m_BLowerThick = new QSpinBox;
		m_BLowerThick->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BLowerThick->setRange(1, d_DNAMaxMM);
		m_BLowerThick->setToolTip(tr("<html>"
					"Thickness of lower torso, in mm. "
					"</html>"));
		QLabel* lbLowerThick = new QLabel("mm Lower Thickness");

		m_BTailLen = new QSpinBox;
		m_BTailLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BTailLen->setRange(0, d_DNAMaxMM);
		m_BTailLen->setToolTip(tr("<html>"
					"Length of tail, in mm. "
					"</html>"));
		QLabel* lbTailLen = new QLabel("mm Tail Length");

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_BNeckLen, row,0);
		glay->addWidget(lbNeckLen, row,1);
		glay->addWidget(m_BGeneButton, row,2);
		row++;
		glay->addWidget(m_BUpperLen, row,0);
		glay->addWidget(lbUpperLen, row,1);
		row++;
		glay->addWidget(m_BUpperWid, row,0);
		glay->addWidget(lbUpperWid, row,1);
		row++;
		glay->addWidget(m_BUpperThick, row,0);
		glay->addWidget(lbUpperThick, row,1);
		row++;
		glay->addWidget(m_BLowerLen, row,0);
		glay->addWidget(lbLowerLen, row,1);
		row++;
		glay->addWidget(m_BLowerWid, row,0);
		glay->addWidget(lbLowerWid, row,1);
		row++;
		glay->addWidget(m_BLowerThick, row,0);
		glay->addWidget(lbLowerThick, row,1);
		row++;
		glay->addWidget(m_BTailLen, row,0);
		glay->addWidget(lbTailLen, row,1);
		row++;
		glay->setRowStretch(row, 1);
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageTorso::~AuChromoPageTorso()
{
}

void AuChromoPageTorso::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoTorso* at = dynamic_cast<const AuChromoTorso*>(alpha);
	const AuChromoTorso* bt = dynamic_cast<const AuChromoTorso*>(beta);
	if(!at || !bt)
		return;
	if(at->Genes())
		m_AGenes = *at->Genes();
	else
		m_AGenes.Clear();
	if(bt->Genes())
		m_BGenes = *bt->Genes();
	else
		m_BGenes.Clear();

	m_ALowerLen->setValue(at->LowerLengthMM());
	m_ALowerWid->setValue(at->LowerWidthMM());
	m_ALowerThick->setValue(at->LowerThicknessMM());
	m_ANeckLen->setValue(at->NeckLengthMM());
	m_ATailLen->setValue(at->TailLengthMM());
	m_AUpperLen->setValue(at->UpperLengthMM());
	m_AUpperWid->setValue(at->UpperWidthMM());
	m_AUpperThick->setValue(at->UpperThicknessMM());

	m_BLowerLen->setValue(bt->LowerLengthMM());
	m_BLowerWid->setValue(bt->LowerWidthMM());
	m_BLowerThick->setValue(bt->LowerThicknessMM());
	m_BNeckLen->setValue(bt->NeckLengthMM());
	m_BTailLen->setValue(bt->TailLengthMM());
	m_BUpperLen->setValue(bt->UpperLengthMM());
	m_BUpperWid->setValue(bt->UpperWidthMM());
	m_BUpperThick->setValue(bt->UpperThicknessMM());
}

int AuChromoPageTorso::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoTorso* at = dynamic_cast<AuChromoTorso*>(alpha);
	AuChromoTorso* bt = dynamic_cast<AuChromoTorso*>(beta);
	if(!at || !bt)
		return -1;

	// Insure we don't end up with a zero length torso.
	if(!m_ALowerLen->value() && !m_AUpperLen->value())
		m_ALowerLen->setValue(1);
	if(!m_BLowerLen->value() && !m_BUpperLen->value())
		m_BLowerLen->setValue(1);

	bool achg = false;
	bool bchg = false;

	if(at->SetLowerLengthMM(m_ALowerLen->value()))
		achg = true;
	if(at->SetLowerThicknessMM(m_ALowerThick->value()))
		achg = true;
	if(at->SetLowerWidthMM(m_ALowerWid->value()))
		achg = true;
	if(at->SetNeckLengthMM(m_ANeckLen->value()))
		achg = true;
	if(at->SetTailLengthMM(m_ATailLen->value()))
		achg = true;
	if(at->SetUpperLengthMM(m_AUpperLen->value()))
		achg = true;
	if(at->SetUpperThicknessMM(m_AUpperThick->value()))
		achg = true;
	if(at->SetUpperWidthMM(m_AUpperWid->value()))
		achg = true;
	if(at->SetGenes(m_AGenes))
		achg = true;

	if(bt->SetLowerLengthMM(m_BLowerLen->value()))
		bchg = true;
	if(bt->SetLowerThicknessMM(m_BLowerThick->value()))
		bchg = true;
	if(bt->SetLowerWidthMM(m_BLowerWid->value()))
		bchg = true;
	if(bt->SetNeckLengthMM(m_BNeckLen->value()))
		bchg = true;
	if(bt->SetTailLengthMM(m_BTailLen->value()))
		bchg = true;
	if(bt->SetUpperLengthMM(m_BUpperLen->value()))
		bchg = true;
	if(bt->SetUpperThicknessMM(m_BUpperThick->value()))
		bchg = true;
	if(bt->SetUpperWidthMM(m_BUpperWid->value()))
		bchg = true;
	if(bt->SetGenes(m_BGenes))
		bchg = true;

	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

void AuChromoPageTorso::AGenes()
{
	AuGeneDialog dlg(AuChromosome::Torso, AuChromoWhich(), m_AGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_AGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_BGenes = dlg.Genes();
}

void AuChromoPageTorso::BGenes()
{
	AuGeneDialog dlg(AuChromosome::Torso, AuChromoWhich(), m_BGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_BGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_AGenes = dlg.Genes();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageArm::AuChromoPageArm(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Arm</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	m_ApplyPair = new QPushButton(QObject::tr("Apply to Pair of Arms"));
	m_ApplyPair->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes to both arms of the same pair. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_ApplyPair, SIGNAL(clicked()), SIGNAL(ApplyPair()));

	m_ApplyAll = new QPushButton(QObject::tr("Apply to All Arms"));
	m_ApplyAll->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes to all arms. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_ApplyAll, SIGNAL(clicked()), SIGNAL(ApplyAll()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_AGeneButton = new AuIconButton(":data/Gene.png");
		m_AGeneButton->setToolTip(tr("<html>"
					"Genes"
					"</html>"));
		connect(m_AGeneButton, SIGNAL(clicked()), SLOT(AGenes()));

		m_AShoulderToSpine = new QSpinBox;
		m_AShoulderToSpine->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AShoulderToSpine->setRange(1, d_DNAMaxMM);
		m_AShoulderToSpine->setToolTip(tr("<html>"
					"Shoulder to spine distance, in mm. "
					"</html>"));
		QLabel* lbShoulderToSpine = new QLabel("mm Shoulder to Spine");

		m_AShoulderRad = new QSpinBox;
		m_AShoulderRad->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AShoulderRad->setRange(1, d_DNAMaxMM);
		m_AShoulderRad->setToolTip(tr("<html>"
					"Shoulder radius, in mm. "
					"</html>"));
		QLabel* lbShoulderRad = new QLabel("mm Shoulder Radius");

		m_AUpperArmLen = new QSpinBox;
		m_AUpperArmLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AUpperArmLen->setRange(1, d_DNAMaxMM);
		m_AUpperArmLen->setToolTip(tr("<html>"
					"Upper arm length, in mm. "
					"</html>"));
		QLabel* lbUpperArmLen = new QLabel("mm Upper Arm Length");

		m_AElbowRad = new QSpinBox;
		m_AElbowRad->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AElbowRad->setRange(1, d_DNAMaxMM);
		m_AElbowRad->setToolTip(tr("<html>"
					"Elbow radius, in mm. "
					"</html>"));
		QLabel* lbElbowRad = new QLabel("mm Elbow Radius");

		m_AForeArmLen = new QSpinBox;
		m_AForeArmLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AForeArmLen->setRange(1, d_DNAMaxMM);
		m_AForeArmLen->setToolTip(tr("<html>"
					"Fore arm length, in mm. "
					"</html>"));
		QLabel* lbForeArmLen = new QLabel("mm Fore Arm Length");

		m_AWristRad = new QSpinBox;
		m_AWristRad->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AWristRad->setRange(1, d_DNAMaxMM);
		m_AWristRad->setToolTip(tr("<html>"
					"Wrist radius, in mm. "
					"</html>"));
		QLabel* lbWristRad = new QLabel("mm Wrist Radius");

		m_AHandLen = new QSpinBox;
		m_AHandLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AHandLen->setRange(1, d_DNAMaxMM);
		m_AHandLen->setToolTip(tr("<html>"
					"Hand length, in mm. "
					"</html>"));
		QLabel* lbHandLen = new QLabel("mm Hand Length");

		m_AArmWalks = new QCheckBox(tr("Walks"), this);
		m_AArmWalks->setToolTip(tr("<html>"
					"When checked, arm is used as a leg when walking. "
					"</html>"));

		m_AArmRuns = new QCheckBox(tr("Runs"), this);
		m_AArmRuns->setToolTip(tr("<html>"
					"When checked, arm is used as a leg when running. "
					"</html>"));

		m_AArmJumps = new QCheckBox(tr("Jumps"), this);
		m_AArmJumps->setToolTip(tr("<html>"
					"When checked, arm is used as a leg when jumping. "
					"</html>"));

		m_AStandTone = new QSpinBox(this);
		m_AStandTone->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AStandTone->setRange(0, 100);
		m_AStandTone->setSingleStep(1);
		m_AStandTone->setToolTip(tr("<html>"
					"Standing muscle tone, from 0 to 100. "
					"<br>"
					"At 100 the arm is as straight as possible when standing. "
					"<br>"
					"At 50 the arm bends partly, holding the body above ground. "
					"<br>"
					"At 0 the arm bends fully, does not prevent the body from touching ground. "
					"</html>"));
		QLabel* lbStandTone = new QLabel("Standing Muscle Tone");

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_AShoulderToSpine, row,0);
		glay->addWidget(lbShoulderToSpine, row,1);
		glay->addWidget(m_AGeneButton, row,2);
		row++;
		glay->addWidget(m_AShoulderRad, row,0);
		glay->addWidget(lbShoulderRad, row,1);
		row++;
		glay->addWidget(m_AUpperArmLen, row,0);
		glay->addWidget(lbUpperArmLen, row,1);
		row++;
		glay->addWidget(m_AElbowRad, row,0);
		glay->addWidget(lbElbowRad, row,1);
		row++;
		glay->addWidget(m_AForeArmLen, row,0);
		glay->addWidget(lbForeArmLen, row,1);
		row++;
		glay->addWidget(m_AWristRad, row,0);
		glay->addWidget(lbWristRad, row,1);
		row++;
		glay->addWidget(m_AHandLen, row,0);
		glay->addWidget(lbHandLen, row,1);
		row++;
		glay->addWidget(m_AStandTone, row,0);
		glay->addWidget(lbStandTone, row,1);
		row++;
		glay->addWidget(m_AArmWalks, row,0, 1,2);
		row++;
		glay->addWidget(m_AArmRuns, row,0, 1,2);
		row++;
		glay->addWidget(m_AArmJumps, row,0, 1,2);
		row++;
		glay->setRowStretch(row, 1);
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BGeneButton = new AuIconButton(":data/Gene.png");
		m_BGeneButton->setToolTip(tr("<html>"
					"Genes"
					"</html>"));
		connect(m_BGeneButton, SIGNAL(clicked()), SLOT(BGenes()));

		m_BShoulderToSpine = new QSpinBox;
		m_BShoulderToSpine->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BShoulderToSpine->setRange(1, d_DNAMaxMM);
		m_BShoulderToSpine->setToolTip(tr("<html>"
					"Shoulder to spine distance, in mm. "
					"</html>"));
		QLabel* lbShoulderToSpine = new QLabel("mm Shoulder to Spine");

		m_BShoulderRad = new QSpinBox;
		m_BShoulderRad->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BShoulderRad->setRange(1, d_DNAMaxMM);
		m_BShoulderRad->setToolTip(tr("<html>"
					"Shoulder radius, in mm. "
					"</html>"));
		QLabel* lbShoulderRad = new QLabel("mm Shoulder Radius");

		m_BUpperArmLen = new QSpinBox;
		m_BUpperArmLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BUpperArmLen->setRange(1, d_DNAMaxMM);
		m_BUpperArmLen->setToolTip(tr("<html>"
					"Upper arm length, in mm. "
					"</html>"));
		QLabel* lbUpperArmLen = new QLabel("mm Upper Arm Length");

		m_BElbowRad = new QSpinBox;
		m_BElbowRad->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BElbowRad->setRange(1, d_DNAMaxMM);
		m_BElbowRad->setToolTip(tr("<html>"
					"Elbow radius, in mm. "
					"</html>"));
		QLabel* lbElbowRad = new QLabel("mm Elbow Radius");

		m_BForeArmLen = new QSpinBox;
		m_BForeArmLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BForeArmLen->setRange(1, d_DNAMaxMM);
		m_BForeArmLen->setToolTip(tr("<html>"
					"Fore arm length, in mm. "
					"</html>"));
		QLabel* lbForeArmLen = new QLabel("mm Fore Arm Length");

		m_BWristRad = new QSpinBox;
		m_BWristRad->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BWristRad->setRange(1, d_DNAMaxMM);
		m_BWristRad->setToolTip(tr("<html>"
					"Wrist radius, in mm. "
					"</html>"));
		QLabel* lbWristRad = new QLabel("mm Wrist Radius");

		m_BHandLen = new QSpinBox;
		m_BHandLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BHandLen->setRange(1, d_DNAMaxMM);
		m_BHandLen->setToolTip(tr("<html>"
					"Hand length, in mm. "
					"</html>"));
		QLabel* lbHandLen = new QLabel("mm Hand Length");

		m_BArmWalks = new QCheckBox(tr("Walks"), this);
		m_BArmWalks->setToolTip(tr("<html>"
					"When checked, arm is used as a leg when walking. "
					"</html>"));

		m_BArmRuns = new QCheckBox(tr("Runs"), this);
		m_BArmRuns->setToolTip(tr("<html>"
					"When checked, arm is used as a leg when running. "
					"</html>"));

		m_BArmJumps = new QCheckBox(tr("Jumps"), this);
		m_BArmJumps->setToolTip(tr("<html>"
					"When checked, arm is used as a leg when jumping. "
					"</html>"));

		m_BStandTone = new QSpinBox(this);
		m_BStandTone->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BStandTone->setRange(0, 100);
		m_BStandTone->setSingleStep(1);
		m_BStandTone->setToolTip(tr("<html>"
					"Standing muscle tone, from 0 to 100. "
					"<br>"
					"At 100 the arm is as straight as possible when standing. "
					"<br>"
					"At 50 the arm bends partly, holding the body above ground. "
					"<br>"
					"At 0 the arm bends fully, does not prevent the body from touching ground. "
					"</html>"));
		QLabel* lbStandTone = new QLabel("Standing Muscle Tone");

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_BShoulderToSpine, row,0);
		glay->addWidget(lbShoulderToSpine, row,1);
		glay->addWidget(m_BGeneButton, row,2);
		row++;
		glay->addWidget(m_BShoulderRad, row,0);
		glay->addWidget(lbShoulderRad, row,1);
		row++;
		glay->addWidget(m_BUpperArmLen, row,0);
		glay->addWidget(lbUpperArmLen, row,1);
		row++;
		glay->addWidget(m_BElbowRad, row,0);
		glay->addWidget(lbElbowRad, row,1);
		row++;
		glay->addWidget(m_BForeArmLen, row,0);
		glay->addWidget(lbForeArmLen, row,1);
		row++;
		glay->addWidget(m_BWristRad, row,0);
		glay->addWidget(lbWristRad, row,1);
		row++;
		glay->addWidget(m_BHandLen, row,0);
		glay->addWidget(lbHandLen, row,1);
		row++;
		glay->addWidget(m_BStandTone, row,0);
		glay->addWidget(lbStandTone, row,1);
		row++;
		glay->addWidget(m_BArmWalks, row,0, 1,2);
		row++;
		glay->addWidget(m_BArmRuns, row,0, 1,2);
		row++;
		glay->addWidget(m_BArmJumps, row,0, 1,2);
		row++;
		glay->setRowStretch(row, 1);
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);
	blay->addWidget(m_ApplyPair);
	blay->addWidget(m_ApplyAll);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageArm::~AuChromoPageArm()
{
}

void AuChromoPageArm::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoArm* at = dynamic_cast<const AuChromoArm*>(alpha);
	const AuChromoArm* bt = dynamic_cast<const AuChromoArm*>(beta);
	if(!at || !bt)
		return;
	m_Which = alpha->Which();
	if(at->Genes())
		m_AGenes = *at->Genes();
	else
		m_AGenes.Clear();
	if(bt->Genes())
		m_BGenes = *bt->Genes();
	else
		m_BGenes.Clear();

	m_AArmJumps->setChecked(at->Jumps());
	m_AArmRuns->setChecked(at->Runs());
	m_AArmWalks->setChecked(at->Walks());
	m_AElbowRad->setValue(at->ElbowRadiusMM());
	m_AForeArmLen->setValue(at->ForeArmLengthMM());
	m_AHandLen->setValue(at->HandLengthMM());
	m_AShoulderRad->setValue(at->ShoulderRadiusMM());
	m_AShoulderToSpine->setValue(at->ShoulderToSpineMM());
	m_AStandTone->setValue(at->StandTone());
	m_AUpperArmLen->setValue(at->UpperArmLengthMM());
	m_AWristRad->setValue(at->WristRadiusMM());

	m_BArmJumps->setChecked(bt->Jumps());
	m_BArmRuns->setChecked(bt->Runs());
	m_BArmWalks->setChecked(bt->Walks());
	m_BElbowRad->setValue(bt->ElbowRadiusMM());
	m_BForeArmLen->setValue(bt->ForeArmLengthMM());
	m_BHandLen->setValue(bt->HandLengthMM());
	m_BShoulderRad->setValue(bt->ShoulderRadiusMM());
	m_BShoulderToSpine->setValue(bt->ShoulderToSpineMM());
	m_BStandTone->setValue(bt->StandTone());
	m_BUpperArmLen->setValue(bt->UpperArmLengthMM());
	m_BWristRad->setValue(bt->WristRadiusMM());
}

int AuChromoPageArm::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoArm* at = dynamic_cast<AuChromoArm*>(alpha);
	AuChromoArm* bt = dynamic_cast<AuChromoArm*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;

	quint8 legBits = 0;
	if(m_AArmJumps->isChecked())
		legBits |= AuChromoArm::LEG_Jump;
	if(m_AArmRuns->isChecked())
		legBits |= AuChromoArm::LEG_Run;
	if(m_AArmWalks->isChecked())
		legBits |= AuChromoArm::LEG_Walk;
	if(at->SetLegBits(legBits))
		achg = true;
	if(at->SetElbowRadiusMM(m_AElbowRad->value()))
		achg = true;
	if(at->SetForeArmLengthMM(m_AForeArmLen->value()))
		achg = true;
	if(at->SetHandLengthMM(m_AHandLen->value()))
		achg = true;
	if(at->SetShoulderRadiusMM(m_AShoulderRad->value()))
		achg = true;
	if(at->SetShoulderToSpineMM(m_AShoulderToSpine->value()))
		achg = true;
	if(at->SetStandTone(m_AStandTone->value()))
		achg = true;
	if(at->SetUpperArmLengthMM(m_AUpperArmLen->value()))
		achg = true;
	if(at->SetWristRadiusMM(m_AWristRad->value()))
		achg = true;
	if(at->SetGenes(m_AGenes))
		achg = true;

	legBits = 0;
	if(m_BArmJumps->isChecked())
		legBits |= AuChromoArm::LEG_Jump;
	if(m_BArmRuns->isChecked())
		legBits |= AuChromoArm::LEG_Run;
	if(m_BArmWalks->isChecked())
		legBits |= AuChromoArm::LEG_Walk;
	if(bt->SetLegBits(legBits))
		bchg = true;
	if(bt->SetElbowRadiusMM(m_BElbowRad->value()))
		bchg = true;
	if(bt->SetForeArmLengthMM(m_BForeArmLen->value()))
		bchg = true;
	if(bt->SetHandLengthMM(m_BHandLen->value()))
		bchg = true;
	if(bt->SetShoulderRadiusMM(m_BShoulderRad->value()))
		bchg = true;
	if(bt->SetStandTone(m_BStandTone->value()))
		bchg = true;
	if(bt->SetShoulderToSpineMM(m_BShoulderToSpine->value()))
		bchg = true;
	if(bt->SetUpperArmLengthMM(m_BUpperArmLen->value()))
		bchg = true;
	if(bt->SetWristRadiusMM(m_BWristRad->value()))
		bchg = true;
	if(bt->SetGenes(m_BGenes))
		bchg = true;

	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

void AuChromoPageArm::AGenes()
{
	AuGeneDialog dlg(AuChromosome::Arm, m_Which, m_AGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_AGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_BGenes = dlg.Genes();
}

void AuChromoPageArm::BGenes()
{
	AuGeneDialog dlg(AuChromosome::Arm, m_Which, m_BGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_BGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_AGenes = dlg.Genes();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageLeg::AuChromoPageLeg(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Leg</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	m_ApplyPair = new QPushButton(QObject::tr("Apply to Pair of Legs"));
	m_ApplyPair->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes to both legs of the same pair. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_ApplyPair, SIGNAL(clicked()), SIGNAL(ApplyPair()));

	m_ApplyAll = new QPushButton(QObject::tr("Apply to All Legs"));
	m_ApplyAll->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes to all legs. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_ApplyAll, SIGNAL(clicked()), SIGNAL(ApplyAll()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_AGeneButton = new AuIconButton(":data/Gene.png");
		m_AGeneButton->setToolTip(tr("<html>"
					"Genes"
					"</html>"));
		connect(m_AGeneButton, SIGNAL(clicked()), SLOT(AGenes()));

		m_AAnkleLen = new QSpinBox;
		m_AAnkleLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AAnkleLen->setRange(1, d_DNAMaxMM);
		m_AAnkleLen->setToolTip(tr("<html>"
					"Ankle length, in mm. "
					"Ankle length is the distance from the ankle joint to the bottom of the foot. "
					"</html>"));
		QLabel* lbAnkleLen = new QLabel("mm Ankle Length");

		m_AAnkleRad = new QSpinBox;
		m_AAnkleRad->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AAnkleRad->setRange(1, d_DNAMaxMM);
		m_AAnkleRad->setToolTip(tr("<html>"
					"Ankle radius, in mm. "
					"</html>"));
		QLabel* lbAnkleRad = new QLabel("mm Ankle Radius");

		m_AButtRad = new QSpinBox;
		m_AButtRad->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AButtRad->setRange(1, d_DNAMaxMM);
		m_AButtRad->setToolTip(tr("<html>"
					"Butt radius, in mm. "
					"Butt radius is the distance from the hip joint to the back of the butt muscle. "
					"</html>"));
		QLabel* lbButtRad = new QLabel("mm Butt Radius");

		m_ACalfLen = new QSpinBox;
		m_ACalfLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ACalfLen->setRange(1, d_DNAMaxMM);
		m_ACalfLen->setToolTip(tr("<html>"
					"Calf length, in mm. "
					"</html>"));
		QLabel* lbCalfLen = new QLabel("mm Calf Length");

		m_AFootLen = new QSpinBox;
		m_AFootLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AFootLen->setRange(1, d_DNAMaxMM);
		m_AFootLen->setToolTip(tr("<html>"
					"Foot length, in mm. "
					"</html>"));
		QLabel* lbFootLen = new QLabel("mm Foot Length");

		m_AFootWid = new QSpinBox;
		m_AFootWid->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AFootWid->setRange(1, d_DNAMaxMM);
		m_AFootWid->setToolTip(tr("<html>"
					"Foot width, in mm. "
					"</html>"));
		QLabel* lbFootWid = new QLabel("mm Foot Width");

		m_AHipRad = new QSpinBox;
		m_AHipRad->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AHipRad->setRange(1, d_DNAMaxMM);
		m_AHipRad->setToolTip(tr("<html>"
					"Hip radius, in mm. "
					"Hip radius is the left to right thickness of the muscle around the hip joint. "
					"</html>"));
		QLabel* lbHipRad = new QLabel("mm Hip Radius");

		m_AHipToSpine = new QSpinBox;
		m_AHipToSpine->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AHipToSpine->setRange(1, d_DNAMaxMM);
		m_AHipToSpine->setToolTip(tr("<html>"
					"Hip to spine distance, in mm. "
					"</html>"));
		QLabel* lbHipToSpine = new QLabel("mm Hip to Spine");

		m_AKneeRad = new QSpinBox;
		m_AKneeRad->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AKneeRad->setRange(1, d_DNAMaxMM);
		m_AKneeRad->setToolTip(tr("<html>"
					"Knee radius, in mm. "
					"</html>"));
		QLabel* lbKneeRad = new QLabel("mm Knee Radius");

		m_AThighLen = new QSpinBox;
		m_AThighLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AThighLen->setRange(1, d_DNAMaxMM);
		m_AThighLen->setToolTip(tr("<html>"
					"Thigh length, in mm. "
					"</html>"));
		QLabel* lbThighLen = new QLabel("mm Thigh Length");

		m_AStandTone = new QSpinBox(this);
		m_AStandTone->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AStandTone->setRange(0, 100);
		m_AStandTone->setSingleStep(1);
		m_AStandTone->setToolTip(tr("<html>"
					"Standing muscle tone, from 0 to 100. "
					"<br>"
					"At 100 the leg is as straight as possible when standing. "
					"<br>"
					"At 50 the leg bends partly, holding the body above ground. "
					"<br>"
					"At 0 the leg bends fully, does not prevent the body from touching ground. "
					"</html>"));
		QLabel* lbStandTone = new QLabel("Standing Muscle Tone");

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_AHipToSpine, row,0);
		glay->addWidget(lbHipToSpine, row,1);
		glay->addWidget(m_AGeneButton, row,2);
		row++;
		glay->addWidget(m_AButtRad, row,0);
		glay->addWidget(lbButtRad, row,1);
		row++;
		glay->addWidget(m_AHipRad, row,0);
		glay->addWidget(lbHipRad, row,1);
		row++;
		glay->addWidget(m_AThighLen, row,0);
		glay->addWidget(lbThighLen, row,1);
		row++;
		glay->addWidget(m_AKneeRad, row,0);
		glay->addWidget(lbKneeRad, row,1);
		row++;
		glay->addWidget(m_ACalfLen, row,0);
		glay->addWidget(lbCalfLen, row,1);
		row++;
		glay->addWidget(m_AAnkleRad, row,0);
		glay->addWidget(lbAnkleRad, row,1);
		row++;
		glay->addWidget(m_AAnkleLen, row,0);
		glay->addWidget(lbAnkleLen, row,1);
		row++;
		glay->addWidget(m_AFootLen, row,0);
		glay->addWidget(lbFootLen, row,1);
		row++;
		glay->addWidget(m_AFootWid, row,0);
		glay->addWidget(lbFootWid, row,1);
		row++;
		glay->addWidget(m_AStandTone, row,0);
		glay->addWidget(lbStandTone, row,1);
		row++;
		glay->setRowStretch(row, 1);
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BGeneButton = new AuIconButton(":data/Gene.png");
		m_BGeneButton->setToolTip(tr("<html>"
					"Genes"
					"</html>"));
		connect(m_BGeneButton, SIGNAL(clicked()), SLOT(BGenes()));

		m_BAnkleLen = new QSpinBox;
		m_BAnkleLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BAnkleLen->setRange(1, d_DNAMaxMM);
		m_BAnkleLen->setToolTip(tr("<html>"
					"Ankle length, in mm. "
					"Ankle length is the distance from the ankle joint to the bottom of the foot. "
					"</html>"));
		QLabel* lbAnkleLen = new QLabel("mm Ankle Length");

		m_BAnkleRad = new QSpinBox;
		m_BAnkleRad->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BAnkleRad->setRange(1, d_DNAMaxMM);
		m_BAnkleRad->setToolTip(tr("<html>"
					"Ankle radius, in mm. "
					"</html>"));
		QLabel* lbAnkleRad = new QLabel("mm Ankle Radius");

		m_BButtRad = new QSpinBox;
		m_BButtRad->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BButtRad->setRange(1, d_DNAMaxMM);
		m_BButtRad->setToolTip(tr("<html>"
					"Butt radius, in mm. "
					"Butt radius is the distance from the hip joint to the back of the butt muscle. "
					"</html>"));
		QLabel* lbButtRad = new QLabel("mm Butt Radius");

		m_BCalfLen = new QSpinBox;
		m_BCalfLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BCalfLen->setRange(1, d_DNAMaxMM);
		m_BCalfLen->setToolTip(tr("<html>"
					"Calf length, in mm. "
					"</html>"));
		QLabel* lbCalfLen = new QLabel("mm Calf Length");

		m_BFootLen = new QSpinBox;
		m_BFootLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BFootLen->setRange(1, d_DNAMaxMM);
		m_BFootLen->setToolTip(tr("<html>"
					"Foot length, in mm. "
					"</html>"));
		QLabel* lbFootLen = new QLabel("mm Foot Length");

		m_BFootWid = new QSpinBox;
		m_BFootWid->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BFootWid->setRange(1, d_DNAMaxMM);
		m_BFootWid->setToolTip(tr("<html>"
					"Foot width, in mm. "
					"</html>"));
		QLabel* lbFootWid = new QLabel("mm Foot Width");

		m_BHipRad = new QSpinBox;
		m_BHipRad->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BHipRad->setRange(1, d_DNAMaxMM);
		m_BHipRad->setToolTip(tr("<html>"
					"Hip radius, in mm. "
					"Hip radius is the left to right thickness of the muscle around the hip joint. "
					"</html>"));
		QLabel* lbHipRad = new QLabel("mm Hip Radius");

		m_BHipToSpine = new QSpinBox;
		m_BHipToSpine->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BHipToSpine->setRange(1, d_DNAMaxMM);
		m_BHipToSpine->setToolTip(tr("<html>"
					"Hip to spine distance, in mm. "
					"</html>"));
		QLabel* lbHipToSpine = new QLabel("mm Hip to Spine");

		m_BKneeRad = new QSpinBox;
		m_BKneeRad->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BKneeRad->setRange(1, d_DNAMaxMM);
		m_BKneeRad->setToolTip(tr("<html>"
					"Knee radius, in mm. "
					"</html>"));
		QLabel* lbKneeRad = new QLabel("mm Knee Radius");

		m_BThighLen = new QSpinBox;
		m_BThighLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BThighLen->setRange(1, d_DNAMaxMM);
		m_BThighLen->setToolTip(tr("<html>"
					"Thigh length, in mm. "
					"</html>"));
		QLabel* lbThighLen = new QLabel("mm Thigh Length");

		m_BStandTone = new QSpinBox(this);
		m_BStandTone->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BStandTone->setRange(0, 100);
		m_BStandTone->setSingleStep(1);
		m_BStandTone->setToolTip(tr("<html>"
					"Standing muscle tone, from 0 to 100. "
					"<br>"
					"At 100 the leg is as straight as possible when standing. "
					"<br>"
					"At 50 the leg bends partly, holding the body above ground. "
					"<br>"
					"At 0 the leg bends fully, does not prevent the body from touching ground. "
					"</html>"));
		QLabel* lbStandTone = new QLabel("Standing Muscle Tone");

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_BHipToSpine, row,0);
		glay->addWidget(lbHipToSpine, row,1);
		glay->addWidget(m_BGeneButton, row,2);
		row++;
		glay->addWidget(m_BButtRad, row,0);
		glay->addWidget(lbButtRad, row,1);
		row++;
		glay->addWidget(m_BHipRad, row,0);
		glay->addWidget(lbHipRad, row,1);
		row++;
		glay->addWidget(m_BThighLen, row,0);
		glay->addWidget(lbThighLen, row,1);
		row++;
		glay->addWidget(m_BKneeRad, row,0);
		glay->addWidget(lbKneeRad, row,1);
		row++;
		glay->addWidget(m_BCalfLen, row,0);
		glay->addWidget(lbCalfLen, row,1);
		row++;
		glay->addWidget(m_BAnkleRad, row,0);
		glay->addWidget(lbAnkleRad, row,1);
		row++;
		glay->addWidget(m_BAnkleLen, row,0);
		glay->addWidget(lbAnkleLen, row,1);
		row++;
		glay->addWidget(m_BFootLen, row,0);
		glay->addWidget(lbFootLen, row,1);
		row++;
		glay->addWidget(m_BFootWid, row,0);
		glay->addWidget(lbFootWid, row,1);
		row++;
		glay->addWidget(m_BStandTone, row,0);
		glay->addWidget(lbStandTone, row,1);
		row++;
		glay->setRowStretch(row, 1);
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);
	blay->addWidget(m_ApplyPair);
	blay->addWidget(m_ApplyAll);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageLeg::~AuChromoPageLeg()
{
}

void AuChromoPageLeg::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoLeg* at = dynamic_cast<const AuChromoLeg*>(alpha);
	const AuChromoLeg* bt = dynamic_cast<const AuChromoLeg*>(beta);
	if(!at || !bt)
		return;
	m_Which = alpha->Which();
	if(at->Genes())
		m_AGenes = *at->Genes();
	else
		m_AGenes.Clear();
	if(bt->Genes())
		m_BGenes = *bt->Genes();
	else
		m_BGenes.Clear();

	m_AAnkleLen->setValue(at->AnkleLengthMM());
	m_AAnkleRad->setValue(at->AnkleRadiusMM());
	m_AButtRad->setValue(at->ButtRadiusMM());
	m_ACalfLen->setValue(at->CalfLengthMM());
	m_AFootLen->setValue(at->FootLengthMM());
	m_AFootWid->setValue(at->FootWidthMM());
	m_AHipRad->setValue(at->HipRadiusXMM());
	m_AHipToSpine->setValue(at->HipToSpineMM());
	m_AKneeRad->setValue(at->KneeRadiusXMM());
	m_AStandTone->setValue(at->StandTone());
	m_AThighLen->setValue(at->ThighLengthMM());

	m_BAnkleLen->setValue(bt->AnkleLengthMM());
	m_BAnkleRad->setValue(bt->AnkleRadiusMM());
	m_BButtRad->setValue(bt->ButtRadiusMM());
	m_BCalfLen->setValue(bt->CalfLengthMM());
	m_BFootLen->setValue(bt->FootLengthMM());
	m_BFootWid->setValue(bt->FootWidthMM());
	m_BHipRad->setValue(bt->HipRadiusXMM());
	m_BHipToSpine->setValue(bt->HipToSpineMM());
	m_BKneeRad->setValue(bt->KneeRadiusXMM());
	m_BStandTone->setValue(bt->StandTone());
	m_BThighLen->setValue(bt->ThighLengthMM());
}

int AuChromoPageLeg::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoLeg* at = dynamic_cast<AuChromoLeg*>(alpha);
	AuChromoLeg* bt = dynamic_cast<AuChromoLeg*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;

	if(at->SetAnkleLengthMM(m_AAnkleLen->value()))
		achg = true;
	if(at->SetAnkleRadiusMM(m_AAnkleRad->value()))
		achg = true;
	if(at->SetButtRadiusMM(m_AButtRad->value()))
		achg = true;
	if(at->SetCalfLengthMM(m_ACalfLen->value()))
		achg = true;
	if(at->SetFootLengthMM(m_AFootLen->value()))
		achg = true;
	if(at->SetFootWidthMM(m_AFootWid->value()))
		achg = true;
	if(at->SetHipRadiusXMM(m_AHipRad->value()))
		achg = true;
	if(at->SetHipToSpineMM(m_AHipToSpine->value()))
		achg = true;
	if(at->SetKneeRadiusXMM(m_AKneeRad->value()))
		achg = true;
	if(at->SetStandTone(m_AStandTone->value()))
		achg = true;
	if(at->SetThighLengthMM(m_AThighLen->value()))
		achg = true;
	if(at->SetGenes(m_AGenes))
		achg = true;

	if(bt->SetAnkleLengthMM(m_BAnkleLen->value()))
		bchg = true;
	if(bt->SetAnkleRadiusMM(m_BAnkleRad->value()))
		bchg = true;
	if(bt->SetButtRadiusMM(m_BButtRad->value()))
		bchg = true;
	if(bt->SetCalfLengthMM(m_BCalfLen->value()))
		bchg = true;
	if(bt->SetFootLengthMM(m_BFootLen->value()))
		bchg = true;
	if(bt->SetFootWidthMM(m_BFootWid->value()))
		bchg = true;
	if(bt->SetHipRadiusXMM(m_BHipRad->value()))
		bchg = true;
	if(bt->SetHipToSpineMM(m_BHipToSpine->value()))
		bchg = true;
	if(bt->SetKneeRadiusXMM(m_BKneeRad->value()))
		bchg = true;
	if(bt->SetStandTone(m_BStandTone->value()))
		bchg = true;
	if(bt->SetThighLengthMM(m_BThighLen->value()))
		bchg = true;
	if(bt->SetGenes(m_BGenes))
		bchg = true;

	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

void AuChromoPageLeg::AGenes()
{
	AuGeneDialog dlg(AuChromosome::Leg, m_Which, m_AGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_AGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_BGenes = dlg.Genes();
}

void AuChromoPageLeg::BGenes()
{
	AuGeneDialog dlg(AuChromosome::Leg, m_Which, m_BGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_BGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_AGenes = dlg.Genes();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageFin::AuChromoPageFin(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Fin</b></html>"));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageFin::~AuChromoPageFin()
{
}

void AuChromoPageFin::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoFin* at = dynamic_cast<const AuChromoFin*>(alpha);
	const AuChromoFin* bt = dynamic_cast<const AuChromoFin*>(beta);
	if(!at || !bt)
		return;
	m_Which = alpha->Which();
	if(at->Genes())
		m_AGenes = *at->Genes();
	else
		m_AGenes.Clear();
	if(bt->Genes())
		m_BGenes = *bt->Genes();
	else
		m_BGenes.Clear();

}

int AuChromoPageFin::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoFin* at = dynamic_cast<AuChromoFin*>(alpha);
	AuChromoFin* bt = dynamic_cast<AuChromoFin*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;

	if(at->SetGenes(m_AGenes))
		achg = true;

	if(bt->SetGenes(m_BGenes))
		bchg = true;

	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

void AuChromoPageFin::AGenes()
{
	AuGeneDialog dlg(AuChromosome::Fin, m_Which, m_AGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_AGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_BGenes = dlg.Genes();
}

void AuChromoPageFin::BGenes()
{
	AuGeneDialog dlg(AuChromosome::Fin, m_Which, m_BGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_BGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_AGenes = dlg.Genes();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageTentacle::AuChromoPageTentacle(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Tentacle</b></html>"));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageTentacle::~AuChromoPageTentacle()
{
}

void AuChromoPageTentacle::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoTentacle* at = dynamic_cast<const AuChromoTentacle*>(alpha);
	const AuChromoTentacle* bt = dynamic_cast<const AuChromoTentacle*>(beta);
	if(!at || !bt)
		return;
	m_Which = alpha->Which();
	if(at->Genes())
		m_AGenes = *at->Genes();
	else
		m_AGenes.Clear();
	if(bt->Genes())
		m_BGenes = *bt->Genes();
	else
		m_BGenes.Clear();

}

int AuChromoPageTentacle::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoTentacle* at = dynamic_cast<AuChromoTentacle*>(alpha);
	AuChromoTentacle* bt = dynamic_cast<AuChromoTentacle*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;

	if(at->SetGenes(m_AGenes))
		achg = true;

	if(bt->SetGenes(m_BGenes))
		bchg = true;

	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

void AuChromoPageTentacle::AGenes()
{
	AuGeneDialog dlg(AuChromosome::Tentacle, m_Which, m_AGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_AGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_BGenes = dlg.Genes();
}

void AuChromoPageTentacle::BGenes()
{
	AuGeneDialog dlg(AuChromosome::Tentacle, m_Which, m_BGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_BGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_AGenes = dlg.Genes();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageWing::AuChromoPageWing(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Wing</b></html>"));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageWing::~AuChromoPageWing()
{
}

void AuChromoPageWing::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoWing* at = dynamic_cast<const AuChromoWing*>(alpha);
	const AuChromoWing* bt = dynamic_cast<const AuChromoWing*>(beta);
	if(!at || !bt)
		return;
	m_Which = alpha->Which();
	if(at->Genes())
		m_AGenes = *at->Genes();
	else
		m_AGenes.Clear();
	if(bt->Genes())
		m_BGenes = *bt->Genes();
	else
		m_BGenes.Clear();

}

int AuChromoPageWing::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoWing* at = dynamic_cast<AuChromoWing*>(alpha);
	AuChromoWing* bt = dynamic_cast<AuChromoWing*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;

	if(at->SetGenes(m_AGenes))
		achg = true;

	if(bt->SetGenes(m_BGenes))
		bchg = true;

	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

void AuChromoPageWing::AGenes()
{
	AuGeneDialog dlg(AuChromosome::Wing, m_Which, m_AGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_AGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_BGenes = dlg.Genes();
}

void AuChromoPageWing::BGenes()
{
	AuGeneDialog dlg(AuChromosome::Wing, m_Which, m_BGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_BGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_AGenes = dlg.Genes();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageAntenna::AuChromoPageAntenna(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Antenna</b></html>"));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageAntenna::~AuChromoPageAntenna()
{
}

void AuChromoPageAntenna::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoAntenna* at = dynamic_cast<const AuChromoAntenna*>(alpha);
	const AuChromoAntenna* bt = dynamic_cast<const AuChromoAntenna*>(beta);
	if(!at || !bt)
		return;
	m_Which = alpha->Which();

}

int AuChromoPageAntenna::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoAntenna* at = dynamic_cast<AuChromoAntenna*>(alpha);
	AuChromoAntenna* bt = dynamic_cast<AuChromoAntenna*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;


	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageHorn::AuChromoPageHorn(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Horn</b></html>"));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageHorn::~AuChromoPageHorn()
{
}

void AuChromoPageHorn::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoHorn* at = dynamic_cast<const AuChromoHorn*>(alpha);
	const AuChromoHorn* bt = dynamic_cast<const AuChromoHorn*>(beta);
	if(!at || !bt)
		return;
	m_Which = alpha->Which();

}

int AuChromoPageHorn::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoHorn* at = dynamic_cast<AuChromoHorn*>(alpha);
	AuChromoHorn* bt = dynamic_cast<AuChromoHorn*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;


	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageEar::AuChromoPageEar(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Ear</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	m_ApplyPair = new QPushButton(QObject::tr("Apply to Pair of Ears"));
	m_ApplyPair->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes to both ears of the same pair. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_ApplyPair, SIGNAL(clicked()), SIGNAL(ApplyPair()));

	m_ApplyAll = new QPushButton(QObject::tr("Apply to All Ears"));
	m_ApplyAll->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes to all ears. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_ApplyAll, SIGNAL(clicked()), SIGNAL(ApplyAll()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_AGeneButton = new AuIconButton(":data/Gene.png");
		m_AGeneButton->setToolTip(tr("<html>"
					"Genes"
					"</html>"));
		connect(m_AGeneButton, SIGNAL(clicked()), SLOT(AGenes()));

		m_AStyle = new QComboBox;
		m_AStyle->addItem(tr("Circle"), AuChromoEar::Circle);
		m_AStyle->setToolTip(QObject::tr("<html>"
					"Ear Shape"
					"</html>"));

		m_AAng = new QSpinBox;
		m_AAng->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AAng->setRange(-180, 180);
		m_AAng->setToolTip(tr("<html>"
					"Rotation of ear, in degrees. "
					"</html>"));
		QLabel* lbang = new QLabel(tr("%1 Rotation").arg(AuGlobal::DegreeSymbol()));

		m_ADia = new QSpinBox;
		m_ADia->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ADia->setRange(1, d_DNAMaxMM);
		m_ADia->setToolTip(tr("<html>"
					"Diameter of ear, in mm. "
					"</html>"));
		QLabel* lbdia = new QLabel("mm Diameter");

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_AStyle, row,0, 1,2);
		glay->addWidget(m_AGeneButton, row,2);
		row++;
		glay->addWidget(m_ADia, row,0);
		glay->addWidget(lbdia, row,1);
		row++;
		glay->addWidget(m_AAng, row,0);
		glay->addWidget(lbang, row,1);
		row++;
		glay->setRowStretch(row, 1);
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BGeneButton = new AuIconButton(":data/Gene.png");
		m_BGeneButton->setToolTip(tr("<html>"
					"Genes"
					"</html>"));
		connect(m_BGeneButton, SIGNAL(clicked()), SLOT(BGenes()));

		m_BStyle = new QComboBox;
		m_BStyle->addItem(tr("Circle"), AuChromoEar::Circle);
		m_BStyle->setToolTip(QObject::tr("<html>"
					"Ear Shape"
					"</html>"));

		m_BAng = new QSpinBox;
		m_BAng->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BAng->setRange(-180, 180);
		m_BAng->setToolTip(tr("<html>"
					"Rotation of ear, in degrees. "
					"</html>"));
		QLabel* lbang = new QLabel(tr("%1 Rotation").arg(AuGlobal::DegreeSymbol()));

		m_BDia = new QSpinBox;
		m_BDia->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BDia->setRange(1, d_DNAMaxMM);
		m_BDia->setToolTip(tr("<html>"
					"Diameter of ear, in mm. "
					"</html>"));
		QLabel* lbdia = new QLabel("mm Diameter");

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_BStyle, row,0, 1,2);
		glay->addWidget(m_BGeneButton, row,2);
		row++;
		glay->addWidget(m_BDia, row,0);
		glay->addWidget(lbdia, row,1);
		row++;
		glay->addWidget(m_BAng, row,0);
		glay->addWidget(lbang, row,1);
		row++;
		glay->setRowStretch(row, 1);
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);
	blay->addWidget(m_ApplyPair);
	blay->addWidget(m_ApplyAll);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageEar::~AuChromoPageEar()
{
}

void AuChromoPageEar::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	Q_UNUSED(dna);
	const AuChromoEar* at = dynamic_cast<const AuChromoEar*>(alpha);
	const AuChromoEar* bt = dynamic_cast<const AuChromoEar*>(beta);
	if(!at || !bt)
		return;
	m_Which = alpha->Which();
	if(at->Genes())
		m_AGenes = *at->Genes();
	else
		m_AGenes.Clear();
	if(bt->Genes())
		m_BGenes = *bt->Genes();
	else
		m_BGenes.Clear();

	m_AStyle->setCurrentIndex(m_AStyle->findData(at->Style()));
	m_ADia->setValue(at->DiameterMM());
	m_AAng->setValue(at->AngleDeg());

	m_BStyle->setCurrentIndex(m_AStyle->findData(bt->Style()));
	m_BDia->setValue(bt->DiameterMM());
	m_BAng->setValue(bt->AngleDeg());
}

int AuChromoPageEar::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoEar* at = dynamic_cast<AuChromoEar*>(alpha);
	AuChromoEar* bt = dynamic_cast<AuChromoEar*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;

	AuChromoEar::STYLE as = (AuChromoEar::STYLE)m_AStyle->currentData().toInt();
	at->SetStyle(as);
	if(at->SetAngleDeg(m_AAng->value()))
		achg = true;
	if(at->SetDiameterMM(m_ADia->value()))
		achg = true;
	if(at->SetGenes(m_AGenes))
		achg = true;

	AuChromoEar::STYLE bs = (AuChromoEar::STYLE)m_BStyle->currentData().toInt();
	bt->SetStyle(bs);
	if(bt->SetAngleDeg(m_BAng->value()))
		bchg = true;
	if(bt->SetDiameterMM(m_BDia->value()))
		bchg = true;
	if(bt->SetGenes(m_BGenes))
		bchg = true;

	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

void AuChromoPageEar::AGenes()
{
	AuGeneDialog dlg(AuChromosome::Ear, m_Which, m_AGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_AGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_BGenes = dlg.Genes();
}

void AuChromoPageEar::BGenes()
{
	AuGeneDialog dlg(AuChromosome::Ear, m_Which, m_BGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_BGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_AGenes = dlg.Genes();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoEyeDecalDialog::AuChromoEyeDecalDialog(MEAT_DECAL eyeDecal, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Eye Genes"));
	setAuDlgIcon(QIcon(":data/Gene.png"));
	m_EyeDecal = eyeDecal;

	QHBoxLayout* clay = new QHBoxLayout;
	QImage decals(":data/SkinDecal.png");
	QImage imgeye(64,64, QImage::Format_ARGB32);	// We will reuse this for every decal.
	for(int decal=MD_HumanEye1; decal <= MD_HumanEye10; decal++)
	{
		// Get texture coordinates of the eye decal.
		float ts, tt, tu, tv;
		AuMeat::DecalTexture((MEAT_DECAL)decal, ts, tt, tu, tv);

		// Convert texture coordinates into pixels.
		int dl = round(decals.width() * ts);
		int db = round(decals.height() * (1-tt));	// Invert Y for painter
		int dr = round(decals.width() * tu);
		int dt = round(decals.height() * (1-tv));	// Invert Y for painter
		QRect drc(dl, dt, dr - dl, db - dt);

		imgeye.fill(QColor(0,0,0));
		QPainter p(&imgeye);
		p.drawImage(imgeye.rect(), decals, drc);
		if(decal == m_EyeDecal)
		{
			QPen pen(QColor(255,255,0));
			pen.setStyle(Qt::DotLine);
			pen.setWidth(1);
			p.setPen(pen);
			p.drawRect(1,1,imgeye.width()-3,imgeye.height()-3);
		}

		QLabel* lbeye = new QLabel;
		lbeye->setPixmap(QPixmap::fromImage(imgeye));
		clay->addWidget(lbeye);
		m_EyeDecals.insert(lbeye, (MEAT_DECAL)decal);
	}

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(clay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuChromoEyeDecalDialog::~AuChromoEyeDecalDialog()
{
}

void AuChromoEyeDecalDialog::setVisible(bool visible)
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

void AuChromoEyeDecalDialog::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/EyeDialog/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/EyeDialog/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/EyeDialog/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/EyeDialog/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuChromoEyeDecalDialog::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/EyeDialog/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/EyeDialog/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/EyeDialog/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/EyeDialog/SizeY",sz.height());
}

void AuChromoEyeDecalDialog::mousePressEvent(QMouseEvent* e)
{
	AuDialog::mousePressEvent(e);

	QLabel* c = dynamic_cast<QLabel*>(childAt(e->pos()));
	if(c)
	{
		QHash<QLabel*, MEAT_DECAL>::const_iterator li = m_EyeDecals.find(c);
		if(li != m_EyeDecals.cend())
		{
			m_EyeDecal = *li;
			accept();
		}
	}
}

MEAT_DECAL AuChromoEyeDecalDialog::EyeDecal() const
{
	return m_EyeDecal;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoEyeDecalButton::AuChromoEyeDecalButton(QWidget* parent) : QPushButton(parent)
{
	m_EyeDecal = MD_HumanEye1;
	connect(this, SIGNAL(clicked()), SLOT(PickEyeDecal()));
}

AuChromoEyeDecalButton::~AuChromoEyeDecalButton()
{
}

void AuChromoEyeDecalButton::SetEyeDecal(MEAT_DECAL eyeDecal)
{
	m_EyeDecal = eyeDecal;
	update();
}

void AuChromoEyeDecalButton::paintEvent(QPaintEvent* pe)
{
	QWidget::paintEvent(pe);

	QImage decals(":data/SkinDecal.png");

	// Get texture coordinates of the eye decal.
	float ts, tt, tu, tv;
	AuMeat::DecalTexture(m_EyeDecal, ts, tt, tu, tv);

	// Convert texture coordinates into pixels.
	int dl = round(decals.width() * ts);
	int db = round(decals.height() * (1-tt));	// Invert Y for painter
	int dr = round(decals.width() * tu);
	int dt = round(decals.height() * (1-tv));	// Invert Y for painter
	QRect drc(dl, dt, dr - dl, db - dt);

	QPainter p(this);
	p.drawImage(rect(), decals, drc);
}

void AuChromoEyeDecalButton::PickEyeDecal()
{
	AuChromoEyeDecalDialog dlg(m_EyeDecal, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_EyeDecal = dlg.EyeDecal();
	update();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageEye::AuChromoPageEye(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Eye</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	m_ApplyPair = new QPushButton(QObject::tr("Apply to Pair of Eyes"));
	m_ApplyPair->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes to both eyes of the same pair. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_ApplyPair, SIGNAL(clicked()), SIGNAL(ApplyPair()));

	m_ApplyAll = new QPushButton(QObject::tr("Apply to All Eyes"));
	m_ApplyAll->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes to all eyes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_ApplyAll, SIGNAL(clicked()), SIGNAL(ApplyAll()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_AGeneButton = new AuIconButton(":data/Gene.png");
		m_AGeneButton->setToolTip(tr("<html>"
					"Genes"
					"</html>"));
		connect(m_AGeneButton, SIGNAL(clicked()), SLOT(AGenes()));

		m_AFacing = new QComboBox;
		m_AFacing->addItem(QObject::tr("Looks Forward"), AuChromoEye::Forward);
		m_AFacing->addItem(QObject::tr("Looks to Side"), AuChromoEye::Side);
		m_AFacing->addItem(QObject::tr("Looks Away"), AuChromoEye::Away);
		m_AFacing->setToolTip(QObject::tr("<html>"
					"Eye Facing Direction"
					"</html>"));

		m_AEye = new AuChromoEyeDecalButton;
		m_AEye->setToolTip(QObject::tr("<html>"
					"Eye Ball"
					"</html>"));
		m_AEye->setMaximumHeight(64);
		m_AEye->setMinimumHeight(64);
		m_AEye->setMaximumWidth(64);
		m_AEye->setMinimumWidth(64);

		m_ABall = new QSpinBox;
		m_ABall->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ABall->setRange(1, d_DNAMaxMM);
		m_ABall->setToolTip(tr("<html>"
					"Diameter of eye ball, in mm. "
					"</html>"));
		QLabel* lbrad = new QLabel("mm Ball Diameter");

		m_ASock = new QSpinBox;
		m_ASock->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ASock->setRange(1, d_DNAMaxMM);
		m_ASock->setToolTip(tr("<html>"
					"Diameter of eye socket, in mm. "
					"</html>"));
		QLabel* lbsock = new QLabel("mm Socket Diameter");

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_AFacing, row,0, 1,2);
		glay->addWidget(m_AGeneButton, row,2);
		row++;
		glay->addWidget(m_AEye, row,0, 1,2);
		row++;
		glay->addWidget(m_ABall, row,0);
		glay->addWidget(lbrad, row,1);
		row++;
		glay->addWidget(m_ASock, row,0);
		glay->addWidget(lbsock, row,1);
		row++;
		glay->setRowStretch(row, 1);
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BGeneButton = new AuIconButton(":data/Gene.png");
		m_BGeneButton->setToolTip(tr("<html>"
					"Genes"
					"</html>"));
		connect(m_BGeneButton, SIGNAL(clicked()), SLOT(BGenes()));

		m_BFacing = new QComboBox;
		m_BFacing->addItem(QObject::tr("Looks Forward"), AuChromoEye::Forward);
		m_BFacing->addItem(QObject::tr("Looks to Side"), AuChromoEye::Side);
		m_BFacing->addItem(QObject::tr("Looks Away"), AuChromoEye::Away);
		m_BFacing->setToolTip(QObject::tr("<html>"
					"Eye Facing Direction"
					"</html>"));

		m_BEye = new AuChromoEyeDecalButton;
		m_BEye->setToolTip(QObject::tr("<html>"
					"Eye Ball"
					"</html>"));
		m_BEye->setMaximumHeight(64);
		m_BEye->setMinimumHeight(64);
		m_BEye->setMaximumWidth(64);
		m_BEye->setMinimumWidth(64);

		m_BBall = new QSpinBox;
		m_BBall->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BBall->setRange(1, d_DNAMaxMM);
		m_BBall->setToolTip(tr("<html>"
					"Diameter of eye ball, in mm. "
					"</html>"));
		QLabel* lbrad = new QLabel("mm Ball Diameter");

		m_BSock = new QSpinBox;
		m_BSock->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BSock->setRange(1, d_DNAMaxMM);
		m_BSock->setToolTip(tr("<html>"
					"Diameter of eye socket, in mm. "
					"</html>"));
		QLabel* lbsock = new QLabel("mm Socket Diameter");

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_BFacing, row,0, 1,2);
		glay->addWidget(m_BGeneButton, row,2);
		row++;
		glay->addWidget(m_BEye, row,0, 1,2);
		row++;
		glay->addWidget(m_BBall, row,0);
		glay->addWidget(lbrad, row,1);
		row++;
		glay->addWidget(m_BSock, row,0);
		glay->addWidget(lbsock, row,1);
		row++;
		glay->setRowStretch(row, 1);
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);
	blay->addWidget(m_ApplyPair);
	blay->addWidget(m_ApplyAll);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageEye::~AuChromoPageEye()
{
}

void AuChromoPageEye::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	Q_UNUSED(dna);
	const AuChromoEye* at = dynamic_cast<const AuChromoEye*>(alpha);
	const AuChromoEye* bt = dynamic_cast<const AuChromoEye*>(beta);
	if(!at || !bt)
		return;
	m_Which = alpha->Which();
	if(at->Genes())
		m_AGenes = *at->Genes();
	else
		m_AGenes.Clear();
	if(bt->Genes())
		m_BGenes = *bt->Genes();
	else
		m_BGenes.Clear();

	m_AEye->SetEyeDecal(at->Decal());
	m_AFacing->setCurrentIndex(m_AFacing->findData(at->Facing()));
	m_ABall->setValue(at->EyeDiameterMM());
	m_ASock->setValue(at->SocketDiameterMM());

	m_BEye->SetEyeDecal(bt->Decal());
	m_BFacing->setCurrentIndex(m_BFacing->findData(bt->Facing()));
	m_BBall->setValue(bt->EyeDiameterMM());
	m_BSock->setValue(bt->SocketDiameterMM());
}

int AuChromoPageEye::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoEye* at = dynamic_cast<AuChromoEye*>(alpha);
	AuChromoEye* bt = dynamic_cast<AuChromoEye*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;

	int afac = m_AFacing->currentData().toInt();
	if(at->SetFacing((AuChromoEye::FACING)afac))
		achg = true;
	if(at->SetDecal(m_AEye->EyeDecal()))
		achg = true;
	if(at->SetEyeDiameterMM(m_ABall->value()))
		achg = true;
	if(at->SetSocketDiameterMM(m_ASock->value()))
		achg = true;
	if(at->SetGenes(m_AGenes))
		achg = true;

	int bfac = m_BFacing->currentData().toInt();
	if(bt->SetFacing((AuChromoEye::FACING)bfac))
		bchg = true;
	if(bt->SetDecal(m_BEye->EyeDecal()))
		bchg = true;
	if(bt->SetEyeDiameterMM(m_BBall->value()))
		bchg = true;
	if(bt->SetSocketDiameterMM(m_BSock->value()))
		bchg = true;
	if(bt->SetGenes(m_BGenes))
		bchg = true;

	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

void AuChromoPageEye::AGenes()
{
	AuGeneDialog dlg(AuChromosome::Eye, m_Which, m_AGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_AGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_BGenes = dlg.Genes();
}

void AuChromoPageEye::BGenes()
{
	AuGeneDialog dlg(AuChromosome::Eye, m_Which, m_BGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_BGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_AGenes = dlg.Genes();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageNose::AuChromoPageNose(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Nose</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_AGeneButton = new AuIconButton(":data/Gene.png");
		m_AGeneButton->setToolTip(tr("<html>"
					"Genes"
					"</html>"));
		connect(m_AGeneButton, SIGNAL(clicked()), SLOT(AGenes()));

		m_AAng = new QSpinBox;
		m_AAng->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AAng->setRange(-180, 180);
		m_AAng->setToolTip(tr("<html>"
					"Angle of nose, in degrees. "
					"0 is straight ahead; 90 is up; -90 is down. "
					"</html>"));
		QLabel* lbang = new QLabel(tr("%1 Angle").arg(AuGlobal::DegreeSymbol()));

		m_ALen = new QSpinBox;
		m_ALen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ALen->setRange(1, d_DNAMaxMM);
		m_ALen->setToolTip(tr("<html>"
					"Length of nose, in mm. "
					"</html>"));
		QLabel* lblen = new QLabel("mm Length");

		m_AWidHead = new QSpinBox;
		m_AWidHead->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AWidHead->setRange(1, d_DNAMaxMM);
		m_AWidHead->setToolTip(tr("<html>"
					"Width of nose at the head, in mm. "
					"</html>"));
		QLabel* lbhwid = new QLabel("mm Width at Head");

		m_AWidTip = new QSpinBox;
		m_AWidTip->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AWidTip->setRange(1, d_DNAMaxMM);
		m_AWidTip->setToolTip(tr("<html>"
					"Width of nose at the tip, in mm. "
					"</html>"));
		QLabel* lbtwid = new QLabel("mm Width at Tip");

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_AAng, row,0);
		glay->addWidget(lbang, row,1);
		glay->addWidget(m_AGeneButton, row,2);
		row++;
		glay->addWidget(m_ALen, row,0);
		glay->addWidget(lblen, row,1);
		row++;
		glay->addWidget(m_AWidHead, row,0);
		glay->addWidget(lbhwid, row,1);
		row++;
		glay->addWidget(m_AWidTip, row,0);
		glay->addWidget(lbtwid, row,1);
		row++;
		glay->setRowStretch(row,1);
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BGeneButton = new AuIconButton(":data/Gene.png");
		m_BGeneButton->setToolTip(tr("<html>"
					"Genes"
					"</html>"));
		connect(m_BGeneButton, SIGNAL(clicked()), SLOT(BGenes()));

		m_BAng = new QSpinBox;
		m_BAng->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BAng->setRange(-180, 180);
		m_BAng->setToolTip(tr("<html>"
					"Angle of nose, in degrees. "
					"0 is straight ahead; 90 is up; -90 is down. "
					"</html>"));
		QLabel* lbang = new QLabel(tr("%1 Angle").arg(AuGlobal::DegreeSymbol()));

		m_BLen = new QSpinBox;
		m_BLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BLen->setRange(1, d_DNAMaxMM);
		m_BLen->setToolTip(tr("<html>"
					"Length of nose, in mm. "
					"</html>"));
		QLabel* lblen = new QLabel("mm Length");

		m_BWidHead = new QSpinBox;
		m_BWidHead->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BWidHead->setRange(1, d_DNAMaxMM);
		m_BWidHead->setToolTip(tr("<html>"
					"Width of nose at the head, in mm. "
					"</html>"));
		QLabel* lbhwid = new QLabel("mm Width at Head");

		m_BWidTip = new QSpinBox;
		m_BWidTip->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BWidTip->setRange(1, d_DNAMaxMM);
		m_BWidTip->setToolTip(tr("<html>"
					"Width of nose at the tip, in mm. "
					"</html>"));
		QLabel* lbtwid = new QLabel("mm Width at Tip");

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_BAng, row,0);
		glay->addWidget(lbang, row,1);
		glay->addWidget(m_BGeneButton, row,2);
		row++;
		glay->addWidget(m_BLen, row,0);
		glay->addWidget(lblen, row,1);
		row++;
		glay->addWidget(m_BWidHead, row,0);
		glay->addWidget(lbhwid, row,1);
		row++;
		glay->addWidget(m_BWidTip, row,0);
		glay->addWidget(lbtwid, row,1);
		row++;
		glay->setRowStretch(row,1);
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageNose::~AuChromoPageNose()
{
}

void AuChromoPageNose::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoNose* at = dynamic_cast<const AuChromoNose*>(alpha);
	const AuChromoNose* bt = dynamic_cast<const AuChromoNose*>(beta);
	if(!at || !bt)
		return;
	if(at->Genes())
		m_AGenes = *at->Genes();
	else
		m_AGenes.Clear();
	if(bt->Genes())
		m_BGenes = *bt->Genes();
	else
		m_BGenes.Clear();

	m_AAng->setValue(at->AngleDeg());
	m_ALen->setValue(at->LengthMM());
	m_AWidHead->setValue(at->WidthHeadMM());
	m_AWidTip->setValue(at->WidthTipMM());

	m_BAng->setValue(bt->AngleDeg());
	m_BLen->setValue(bt->LengthMM());
	m_BWidHead->setValue(bt->WidthHeadMM());
	m_BWidTip->setValue(bt->WidthTipMM());
}

int AuChromoPageNose::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoNose* at = dynamic_cast<AuChromoNose*>(alpha);
	AuChromoNose* bt = dynamic_cast<AuChromoNose*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;

	if(at->SetAngleDeg(m_AAng->value()))
		achg = true;
	if(at->SetLengthMM(m_ALen->value()))
		achg = true;
	if(at->SetWidthHeadMM(m_AWidHead->value()))
		achg = true;
	if(at->SetWidthTipMM(m_AWidTip->value()))
		achg = true;
	if(at->SetGenes(m_AGenes))
		achg = true;

	if(bt->SetAngleDeg(m_BAng->value()))
		bchg = true;
	if(bt->SetLengthMM(m_BLen->value()))
		bchg = true;
	if(bt->SetWidthHeadMM(m_BWidHead->value()))
		bchg = true;
	if(bt->SetWidthTipMM(m_BWidTip->value()))
		bchg = true;
	if(bt->SetGenes(m_BGenes))
		bchg = true;

	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

void AuChromoPageNose::AGenes()
{
	AuGeneDialog dlg(AuChromosome::Nose, AuChromoWhich(), m_AGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_AGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_BGenes = dlg.Genes();
}

void AuChromoPageNose::BGenes()
{
	AuGeneDialog dlg(AuChromosome::Nose, AuChromoWhich(), m_BGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_BGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_AGenes = dlg.Genes();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageMouth::AuChromoPageMouth(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Mouth</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_AGeneButton = new AuIconButton(":data/Gene.png");
		m_AGeneButton->setToolTip(tr("<html>"
					"Genes"
					"</html>"));
		connect(m_AGeneButton, SIGNAL(clicked()), SLOT(AGenes()));

		m_ALen = new QSpinBox;
		m_ALen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ALen->setRange(1, d_DNAMaxMM);
		m_ALen->setToolTip(tr("<html>"
					"Length of mouth, in mm. "
					"</html>"));
		QLabel* lbLen = new QLabel("mm Length");

		m_AWid = new QSpinBox;
		m_AWid->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AWid->setRange(1, d_DNAMaxMM);
		m_AWid->setToolTip(tr("<html>"
					"Width of mouth, in mm. "
					"</html>"));
		QLabel* lbWid = new QLabel("mm Width");

		m_AHt = new QSpinBox;
		m_AHt->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_AHt->setRange(1, d_DNAMaxMM);
		m_AHt->setToolTip(tr("<html>"
					"Height of upper jaw, in mm. "
					"</html>"));
		QLabel* lbHt = new QLabel("mm Upper Jaw");

		m_ADep = new QSpinBox;
		m_ADep->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ADep->setRange(1, d_DNAMaxMM);
		m_ADep->setToolTip(tr("<html>"
					"Depth of lower jaw, in mm. "
					"</html>"));
		QLabel* lbDep = new QLabel("mm Lower Jaw");

		m_ALipThick = new QSpinBox;
		m_ALipThick->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ALipThick->setRange(1, d_DNAMaxMM);
		m_ALipThick->setToolTip(tr("<html>"
					"Thickness of lips, in mm. "
					"Distance between face and teeth. "
					"</html>"));
		QLabel* lbLipThick = new QLabel("mm Lip Thickness");

		m_ATeeth = new QComboBox;
		m_ATeeth->addItem(QObject::tr("No Teeth"), AuChromoMouth::NoTeeth);
		m_ATeeth->addItem(QObject::tr("Flat"), AuChromoMouth::Flat);
		m_ATeeth->addItem(QObject::tr("Fangs"), AuChromoMouth::Fangs);
		m_ATeeth->setToolTip(QObject::tr("<html>"
					"Teeth"
					"</html>"));

		m_ATeethHtBot = new QSpinBox;
		m_ATeethHtBot->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ATeethHtBot->setRange(1, d_DNAMaxMM);
		m_ATeethHtBot->setToolTip(tr("<html>"
					"Length of tallest teeth in the lower jaw, in mm. "
					"</html>"));
		QLabel* lbTeethHtBot = new QLabel("mm Lower Teeth");

		m_ATeethHtTop = new QSpinBox;
		m_ATeethHtTop->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ATeethHtTop->setRange(1, d_DNAMaxMM);
		m_ATeethHtTop->setToolTip(tr("<html>"
					"Length of tallest teeth in the upper jaw, in mm. "
					"</html>"));
		QLabel* lbTeethHtTop = new QLabel("mm Upper Teeth");

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_ALen, row,0);
		glay->addWidget(lbLen, row,1);
		glay->addWidget(m_AGeneButton, row,2);
		row++;
		glay->addWidget(m_AWid, row,0);
		glay->addWidget(lbWid, row,1);
		row++;
		glay->addWidget(m_AHt, row,0);
		glay->addWidget(lbHt, row,1);
		row++;
		glay->addWidget(m_ADep, row,0);
		glay->addWidget(lbDep, row,1);
		row++;
		glay->addWidget(m_ALipThick, row,0);
		glay->addWidget(lbLipThick, row,1);
		row++;
		glay->addWidget(m_ATeeth, row,0, 1,2);
		row++;
		glay->addWidget(m_ATeethHtTop, row,0);
		glay->addWidget(lbTeethHtTop, row,1);
		row++;
		glay->addWidget(m_ATeethHtBot, row,0);
		glay->addWidget(lbTeethHtBot, row,1);
		row++;
		glay->setRowStretch(row, 1);
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BGeneButton = new AuIconButton(":data/Gene.png");
		m_BGeneButton->setToolTip(tr("<html>"
					"Genes"
					"</html>"));
		connect(m_BGeneButton, SIGNAL(clicked()), SLOT(BGenes()));

		m_BLen = new QSpinBox;
		m_BLen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BLen->setRange(1, d_DNAMaxMM);
		m_BLen->setToolTip(tr("<html>"
					"Length of mouth, in mm. "
					"</html>"));
		QLabel* lbLen = new QLabel("mm Length");

		m_BWid = new QSpinBox;
		m_BWid->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BWid->setRange(1, d_DNAMaxMM);
		m_BWid->setToolTip(tr("<html>"
					"Width of mouth, in mm. "
					"</html>"));
		QLabel* lbWid = new QLabel("mm Width");

		m_BHt = new QSpinBox;
		m_BHt->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BHt->setRange(1, d_DNAMaxMM);
		m_BHt->setToolTip(tr("<html>"
					"Height of upper jaw, in mm. "
					"</html>"));
		QLabel* lbHt = new QLabel("mm Upper Jaw");

		m_BDep = new QSpinBox;
		m_BDep->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BDep->setRange(1, d_DNAMaxMM);
		m_BDep->setToolTip(tr("<html>"
					"Depth of lower jaw, in mm. "
					"</html>"));
		QLabel* lbDep = new QLabel("mm Lower Jaw");

		m_BLipThick = new QSpinBox;
		m_BLipThick->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BLipThick->setRange(1, d_DNAMaxMM);
		m_BLipThick->setToolTip(tr("<html>"
					"Thickness of lips, in mm. "
					"Distance between face and teeth. "
					"</html>"));
		QLabel* lbLipThick = new QLabel("mm Lip Thickness");

		m_BTeeth = new QComboBox;
		m_BTeeth->addItem(QObject::tr("No Teeth"), AuChromoMouth::NoTeeth);
		m_BTeeth->addItem(QObject::tr("Flat"), AuChromoMouth::Flat);
		m_BTeeth->addItem(QObject::tr("Fangs"), AuChromoMouth::Fangs);
		m_BTeeth->setToolTip(QObject::tr("<html>"
					"Teeth"
					"</html>"));

		m_BTeethHtBot = new QSpinBox;
		m_BTeethHtBot->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BTeethHtBot->setRange(1, d_DNAMaxMM);
		m_BTeethHtBot->setToolTip(tr("<html>"
					"Length of tallest teeth in the lower jaw, in mm. "
					"</html>"));
		QLabel* lbTeethHtTop = new QLabel("mm Lower Teeth");

		m_BTeethHtTop = new QSpinBox;
		m_BTeethHtTop->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BTeethHtTop->setRange(1, d_DNAMaxMM);
		m_BTeethHtTop->setToolTip(tr("<html>"
					"Length of tallest teeth in the upper jaw, in mm. "
					"</html>"));
		QLabel* lbTeethHtBot = new QLabel("mm Upper Teeth");

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_BLen, row,0);
		glay->addWidget(lbLen, row,1);
		glay->addWidget(m_BGeneButton, row,2);
		row++;
		glay->addWidget(m_BWid, row,0);
		glay->addWidget(lbWid, row,1);
		row++;
		glay->addWidget(m_BHt, row,0);
		glay->addWidget(lbHt, row,1);
		row++;
		glay->addWidget(m_BDep, row,0);
		glay->addWidget(lbDep, row,1);
		row++;
		glay->addWidget(m_BLipThick, row,0);
		glay->addWidget(lbLipThick, row,1);
		row++;
		glay->addWidget(m_BTeeth, row,0, 1,2);
		row++;
		glay->addWidget(m_BTeethHtTop, row,0);
		glay->addWidget(lbTeethHtTop, row,1);
		row++;
		glay->addWidget(m_BTeethHtBot, row,0);
		glay->addWidget(lbTeethHtBot, row,1);
		row++;
		glay->setRowStretch(row, 1);
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageMouth::~AuChromoPageMouth()
{
}

void AuChromoPageMouth::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoMouth* at = dynamic_cast<const AuChromoMouth*>(alpha);
	const AuChromoMouth* bt = dynamic_cast<const AuChromoMouth*>(beta);
	if(!at || !bt)
		return;
	if(at->Genes())
		m_AGenes = *at->Genes();
	else
		m_AGenes.Clear();
	if(bt->Genes())
		m_BGenes = *bt->Genes();
	else
		m_BGenes.Clear();

	m_ADep->setValue(at->DepthMM());
	m_AHt->setValue(at->HeightMM());
	m_ALen->setValue(at->LengthMM());
	m_ALipThick->setValue(at->LipThicknessMM());
	m_ATeeth->setCurrentIndex(m_ATeeth->findData(at->Teeth()));
	m_ATeethHtBot->setValue(at->TeethHeightBottomMM());
	m_ATeethHtTop->setValue(at->TeethHeightTopMM());
	m_AWid->setValue(at->WidthMM());

	m_BDep->setValue(bt->DepthMM());
	m_BHt->setValue(bt->HeightMM());
	m_BLen->setValue(bt->LengthMM());
	m_BLipThick->setValue(bt->LipThicknessMM());
	m_BTeeth->setCurrentIndex(m_BTeeth->findData(bt->Teeth()));
	m_BTeethHtBot->setValue(bt->TeethHeightBottomMM());
	m_BTeethHtTop->setValue(bt->TeethHeightTopMM());
	m_BWid->setValue(bt->WidthMM());
}

int AuChromoPageMouth::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoMouth* at = dynamic_cast<AuChromoMouth*>(alpha);
	AuChromoMouth* bt = dynamic_cast<AuChromoMouth*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;

	if(at->SetDepthMM(m_ADep->value()))
		achg = true;
	if(at->SetHeightMM(m_AHt->value()))
		achg = true;
	if(at->SetLengthMM(m_ALen->value()))
		achg = true;
	if(at->SetLipThicknessMM(m_ALipThick->value()))
		achg = true;
	if(at->SetTeeth((AuChromoMouth::TEETH)m_ATeeth->currentData().toInt()))
		achg = true;
	if(at->SetTeethHeightBottomMM(m_ATeethHtBot->value()))
		achg = true;
	if(at->SetTeethHeightTopMM(m_ATeethHtTop->value()))
		achg = true;
	if(at->SetWidthMM(m_AWid->value()))
		achg = true;
	if(at->SetGenes(m_AGenes))
		achg = true;

	if(bt->SetDepthMM(m_BDep->value()))
		bchg = true;
	if(bt->SetHeightMM(m_BHt->value()))
		bchg = true;
	if(bt->SetLengthMM(m_BLen->value()))
		bchg = true;
	if(bt->SetLipThicknessMM(m_BLipThick->value()))
		bchg = true;
	if(bt->SetTeeth((AuChromoMouth::TEETH)m_BTeeth->currentData().toInt()))
		bchg = true;
	if(bt->SetTeethHeightBottomMM(m_BTeethHtBot->value()))
		bchg = true;
	if(bt->SetTeethHeightTopMM(m_BTeethHtTop->value()))
		bchg = true;
	if(bt->SetWidthMM(m_BWid->value()))
		bchg = true;
	if(bt->SetGenes(m_BGenes))
		bchg = true;

	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

void AuChromoPageMouth::AGenes()
{
	AuGeneDialog dlg(AuChromosome::Mouth, AuChromoWhich(), m_AGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_AGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_BGenes = dlg.Genes();
}

void AuChromoPageMouth::BGenes()
{
	AuGeneDialog dlg(AuChromosome::Mouth, AuChromoWhich(), m_BGenes, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	m_BGenes = dlg.Genes();
	if(dlg.ApplyToBoth())
		m_AGenes = dlg.Genes();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageLungs::AuChromoPageLungs(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Lungs</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_AAtmo = new QComboBox;
		m_AAtmo->addItem(QObject::tr("Standard"), ATMTYPE_Standard);
		m_AAtmo->addItem(QObject::tr("Tainted"), ATMTYPE_Tainted);
		m_AAtmo->addItem(QObject::tr("Exotic"), ATMTYPE_Exotic);
		m_AAtmo->addItem(QObject::tr("Corrosive"), ATMTYPE_Corrosive);
		m_AAtmo->addItem(QObject::tr("Insidious"), ATMTYPE_Insidious);
		m_AAtmo->setToolTip(QObject::tr("<html>"
					"Atmosphere. Standard and tainted are compatible. Exotic can only breathe exotic."
					"</html>"));
		connect(m_AAtmo, SIGNAL(currentIndexChanged(int)), SLOT(AAtmosphereChanged(int)));

		QVBoxLayout* glay = new QVBoxLayout;
		glay->addWidget(m_AAtmo);
		glay->addStretch();
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BAtmo = new QComboBox;
		m_BAtmo->addItem(QObject::tr("Standard"), ATMTYPE_Standard);
		m_BAtmo->addItem(QObject::tr("Tainted"), ATMTYPE_Tainted);
		m_BAtmo->addItem(QObject::tr("Exotic"), ATMTYPE_Exotic);
		m_BAtmo->addItem(QObject::tr("Corrosive"), ATMTYPE_Corrosive);
		m_BAtmo->addItem(QObject::tr("Insidious"), ATMTYPE_Insidious);
		m_BAtmo->setToolTip(QObject::tr("<html>"
					"Atmosphere. Standard and tainted are compatible. Exotic can only breathe exotic."
					"</html>"));
		connect(m_BAtmo, SIGNAL(currentIndexChanged(int)), SLOT(BAtmosphereChanged(int)));

		QVBoxLayout* glay = new QVBoxLayout;
		glay->addWidget(m_BAtmo);
		glay->addStretch();
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageLungs::~AuChromoPageLungs()
{
}

void AuChromoPageLungs::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	Q_UNUSED(dna);
	const AuChromoLungs* at = dynamic_cast<const AuChromoLungs*>(alpha);
	const AuChromoLungs* bt = dynamic_cast<const AuChromoLungs*>(beta);
	if(!at || !bt)
		return;

	m_AAtmo->setCurrentIndex(m_AAtmo->findData(at->AtmosphereType()));
	m_BAtmo->setCurrentIndex(m_BAtmo->findData(bt->AtmosphereType()));
}

int AuChromoPageLungs::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoLungs* at = dynamic_cast<AuChromoLungs*>(alpha);
	AuChromoLungs* bt = dynamic_cast<AuChromoLungs*>(beta);
	if(!at || !bt)
		return -1;

	ATMTYPE aat = (ATMTYPE)m_AAtmo->currentData().toInt();
	at->SetAtmosphereType(aat);

	ATMTYPE bat = (ATMTYPE)m_BAtmo->currentData().toInt();
	bt->SetAtmosphereType(bat);

	return 0;
}

void AuChromoPageLungs::AAtmosphereChanged(int index)
{
	// Keep atmos in sync.
	m_BAtmo->blockSignals(true);
	m_BAtmo->setCurrentIndex(index);
	m_BAtmo->blockSignals(false);
	EnableControls();
}

void AuChromoPageLungs::BAtmosphereChanged(int index)
{
	// Keep atmos in sync.
	m_AAtmo->blockSignals(true);
	m_AAtmo->setCurrentIndex(index);
	m_AAtmo->blockSignals(false);
	EnableControls();
}

void AuChromoPageLungs::EnableControls()
{
	bool applyOk = true;
	if(m_AAtmo->currentData() != m_BAtmo->currentData())
		applyOk = false;
	m_Apply->setEnabled(applyOk);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageGills::AuChromoPageGills(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Gills</b></html>"));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageGills::~AuChromoPageGills()
{
}

void AuChromoPageGills::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	const AuChromoGills* at = dynamic_cast<const AuChromoGills*>(alpha);
	const AuChromoGills* bt = dynamic_cast<const AuChromoGills*>(beta);
	if(!at || !bt)
		return;

}

int AuChromoPageGills::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoGills* at = dynamic_cast<AuChromoGills*>(alpha);
	AuChromoGills* bt = dynamic_cast<AuChromoGills*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;


	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageBlood::AuChromoPageBlood(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Blood</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_AType = new QComboBox;
		m_AType->addItem(QObject::tr("Cold"), AuChromoBlood::Cold);
		m_AType->addItem(QObject::tr("Warm"), AuChromoBlood::Warm);
		m_AType->setToolTip(QObject::tr("<html>"
					"Blood Type"
					"</html>"));
		connect(m_AType, SIGNAL(currentIndexChanged(int)), SLOT(ATypeChanged(int)));

		QVBoxLayout* glay = new QVBoxLayout;
		glay->addWidget(m_AType);
		glay->addStretch();
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BType = new QComboBox;
		m_BType->addItem(QObject::tr("Cold"), AuChromoBlood::Cold);
		m_BType->addItem(QObject::tr("Warm"), AuChromoBlood::Warm);
		m_BType->setToolTip(QObject::tr("<html>"
					"Blood Type"
					"</html>"));
		connect(m_BType, SIGNAL(currentIndexChanged(int)), SLOT(BTypeChanged(int)));

		QVBoxLayout* glay = new QVBoxLayout;
		glay->addWidget(m_BType);
		glay->addStretch();
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageBlood::~AuChromoPageBlood()
{
}

void AuChromoPageBlood::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	Q_UNUSED(dna);
	const AuChromoBlood* at = dynamic_cast<const AuChromoBlood*>(alpha);
	const AuChromoBlood* bt = dynamic_cast<const AuChromoBlood*>(beta);
	if(!at || !bt)
		return;

	m_AType->setCurrentIndex(m_AType->findData(at->Style()));

	m_BType->setCurrentIndex(m_BType->findData(bt->Style()));
}

int AuChromoPageBlood::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoBlood* at = dynamic_cast<AuChromoBlood*>(alpha);
	AuChromoBlood* bt = dynamic_cast<AuChromoBlood*>(beta);
	if(!at || !bt)
		return -1;

	AuChromoBlood::STYLE abt = (AuChromoBlood::STYLE)m_AType->currentData().toInt();
	at->SetStyle(abt);

	AuChromoBlood::STYLE bbt = (AuChromoBlood::STYLE)m_BType->currentData().toInt();
	bt->SetStyle(bbt);

	return 0;
}

void AuChromoPageBlood::ATypeChanged(int index)
{
	// Keep atmos in sync.
	m_BType->blockSignals(true);
	m_BType->setCurrentIndex(index);
	m_BType->blockSignals(false);
	EnableControls();
}

void AuChromoPageBlood::BTypeChanged(int index)
{
	// Keep atmos in sync.
	m_AType->blockSignals(true);
	m_AType->setCurrentIndex(index);
	m_AType->blockSignals(false);
	EnableControls();
}

void AuChromoPageBlood::EnableControls()
{
	bool applyOk = true;
	if(m_AType->currentData() != m_BType->currentData())
		applyOk = false;
	m_Apply->setEnabled(applyOk);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageReproduction::AuChromoPageReproduction(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Reproduction</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_ABirthPlace = new QComboBox;
		m_ABirthPlace->addItem(QObject::tr("On Land"), AuChromoReproduction::OnLand);
		m_ABirthPlace->addItem(QObject::tr("In Water"), AuChromoReproduction::InWater);
		m_ABirthPlace->addItem(QObject::tr("Dead Host"), AuChromoReproduction::DeadHost);
		m_ABirthPlace->addItem(QObject::tr("Live Host"), AuChromoReproduction::LiveHost);
		m_ABirthPlace->setToolTip(QObject::tr("<html>"
					"Birthing Place"
					"</html>"));

		m_AGenMale = new QCheckBox(tr("Male Genitals"));
		m_AGenMale->setToolTip(QObject::tr("<html>"
					"Male genitals transfer DNA to female genitals. "
					"</html>"));
		connect(m_AGenMale, SIGNAL(stateChanged(int)), SLOT(AGenMaleChanged(int)));
		m_AGenFemale = new QCheckBox(tr("Female Genitals"));
		m_AGenFemale->setToolTip(QObject::tr("<html>"
					"Female genitals accept DNA from male genitals. "
					"</html>"));
		connect(m_AGenFemale, SIGNAL(stateChanged(int)), SLOT(AGenFemaleChanged(int)));

		m_ALitterMin = new QSpinBox;
		m_ALitterMin->setRange(1, 15);
		m_ALitterMin->setSingleStep(1);
		m_ALitterMin->setToolTip(QObject::tr("<html>"
					"The least number of young produced when gestating."
					"</html>"));
		connect(m_ALitterMin, SIGNAL(valueChanged(int)), SLOT(ALitterMinChanged(int)));

		m_ALitterMax = new QSpinBox;
		m_ALitterMax->setRange(1, 15);
		m_ALitterMax->setSingleStep(1);
		m_ALitterMax->setToolTip(QObject::tr("<html>"
					"The most number of young produced when gestating."
					"</html>"));
		connect(m_ALitterMax, SIGNAL(valueChanged(int)), SLOT(ALitterMaxChanged(int)));

		m_AYoung = new QComboBox;
		m_AYoung->addItem(QObject::tr("Eggs"), AuChromoReproduction::Eggs);
		m_AYoung->addItem(QObject::tr("Live Young"), AuChromoReproduction::Live);
		m_AYoung->setToolTip(QObject::tr("<html>"
					"Animals that lay <i>eggs</i> do so shortly after becoming pregnant. "
					"Animals that birth <i>live young</i> do so after a gestation period. "
					"</html>"));

		m_ALitterLabel = new QLabel(tr("to"), this);

		QHBoxLayout* llay = new QHBoxLayout;
		llay->addWidget(m_ALitterMin);
		llay->addWidget(m_ALitterLabel);
		llay->addWidget(m_ALitterMax);
		llay->addWidget(m_AYoung);
		llay->addWidget(m_ABirthPlace);

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_AGenMale, row,0, 1,2);
		row++;
		glay->addWidget(m_AGenFemale, row,0, 1,2);
		row++;
		glay->addLayout(llay, row,0, 1,2);
		row++;
		glay->setRowStretch(row,1);
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BBirthPlace = new QComboBox;
		m_BBirthPlace->addItem(QObject::tr("On Land"), AuChromoReproduction::OnLand);
		m_BBirthPlace->addItem(QObject::tr("In Water"), AuChromoReproduction::InWater);
		m_BBirthPlace->addItem(QObject::tr("Dead Host"), AuChromoReproduction::DeadHost);
		m_BBirthPlace->addItem(QObject::tr("Live Host"), AuChromoReproduction::LiveHost);
		m_BBirthPlace->setToolTip(QObject::tr("<html>"
					"Birthing Place"
					"</html>"));

		m_BGenMale = new QCheckBox(tr("Male Genitals"));
		m_BGenMale->setToolTip(QObject::tr("<html>"
					"Male genitals transfer DNA to female genitals. "
					"</html>"));
		connect(m_BGenMale, SIGNAL(stateChanged(int)), SLOT(BGenMaleChanged(int)));
		m_BGenFemale = new QCheckBox(tr("Female Genitals"));
		m_BGenFemale->setToolTip(QObject::tr("<html>"
					"Female genitals accept DNA from male genitals. "
					"</html>"));
		connect(m_BGenFemale, SIGNAL(stateChanged(int)), SLOT(BGenFemaleChanged(int)));

		m_BLitterMin = new QSpinBox;
		m_BLitterMin->setRange(1, 15);
		m_BLitterMin->setSingleStep(1);
		m_BLitterMin->setToolTip(QObject::tr("<html>"
					"The least number of young produced when gestating."
					"</html>"));
		connect(m_BLitterMin, SIGNAL(valueChanged(int)), SLOT(BLitterMinChanged(int)));

		m_BLitterMax = new QSpinBox;
		m_BLitterMax->setRange(1, 15);
		m_BLitterMax->setSingleStep(1);
		m_BLitterMax->setToolTip(QObject::tr("<html>"
					"The most number of young produced when gestating."
					"</html>"));
		connect(m_BLitterMax, SIGNAL(valueChanged(int)), SLOT(BLitterMaxChanged(int)));

		m_BYoung = new QComboBox;
		m_BYoung->addItem(QObject::tr("Eggs"), AuChromoReproduction::Eggs);
		m_BYoung->addItem(QObject::tr("Live Young"), AuChromoReproduction::Live);
		m_BYoung->setToolTip(QObject::tr("<html>"
					"Animals that lay <i>eggs</i> do so shortly after becoming pregnant. "
					"Animals that birth <i>live young</i> do so after a gestation period. "
					"</html>"));

		m_BLitterLabel = new QLabel(tr("to"), this);

		QHBoxLayout* llay = new QHBoxLayout;
		llay->addWidget(m_BLitterMin);
		llay->addWidget(m_BLitterLabel);
		llay->addWidget(m_BLitterMax);
		llay->addWidget(m_BYoung);
		llay->addWidget(m_BBirthPlace);

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_BGenMale, row,0, 1,2);
		row++;
		glay->addWidget(m_BGenFemale, row,0, 1,2);
		row++;
		glay->addLayout(llay, row,0, 1,2);
		row++;
		glay->setRowStretch(row,1);
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageReproduction::~AuChromoPageReproduction()
{
}

void AuChromoPageReproduction::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	Q_UNUSED(dna);
	const AuChromoReproduction* at = dynamic_cast<const AuChromoReproduction*>(alpha);
	const AuChromoReproduction* bt = dynamic_cast<const AuChromoReproduction*>(beta);
	if(!at || !bt)
		return;

	m_ABirthPlace->setCurrentIndex(m_ABirthPlace->findData(at->BirthingPlace()));
	m_AGenFemale->setChecked(at->HasFemaleGenitals());
	m_AGenMale->setChecked(at->HasMaleGenitals());
	m_ALitterMax->setValue(at->LitterMax());
	m_ALitterMin->setValue(at->LitterMin());
	m_AYoung->setCurrentIndex(m_AYoung->findData(at->Young()));

	m_BBirthPlace->setCurrentIndex(m_ABirthPlace->findData(bt->BirthingPlace()));
	m_BGenFemale->setChecked(bt->HasFemaleGenitals());
	m_BGenMale->setChecked(bt->HasMaleGenitals());
	m_BLitterMax->setValue(bt->LitterMax());
	m_BLitterMin->setValue(bt->LitterMin());
	m_BYoung->setCurrentIndex(m_AYoung->findData(bt->Young()));
}

int AuChromoPageReproduction::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoReproduction* at = dynamic_cast<AuChromoReproduction*>(alpha);
	AuChromoReproduction* bt = dynamic_cast<AuChromoReproduction*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;

	bool afemale = m_AGenFemale->isChecked();
	bool amale = m_AGenMale->isChecked();
	if(afemale || amale)
	{
		at->SetBirthingPlace((AuChromoReproduction::BIRTHPLACE)m_ABirthPlace->currentData().toInt());
		at->SetLitterRange(m_ALitterMin->value(), m_ALitterMax->value());
		at->SetYoung((AuChromoReproduction::YOUNG)m_AYoung->currentData().toInt());
	}
	if(at->SetHasFemaleGenitals(afemale))
		achg = true;
	if(at->SetHasMaleGenitals(amale))
		achg = true;

	bool bfemale = m_BGenFemale->isChecked();
	bool bmale = m_BGenMale->isChecked();
	if(bfemale || bmale)
	{
		bt->SetBirthingPlace((AuChromoReproduction::BIRTHPLACE)m_BBirthPlace->currentData().toInt());
		bt->SetLitterRange(m_BLitterMin->value(), m_BLitterMax->value());
		bt->SetYoung((AuChromoReproduction::YOUNG)m_BYoung->currentData().toInt());
	}
	if(bt->SetHasFemaleGenitals(bfemale))
		bchg = true;
	if(bt->SetHasMaleGenitals(bmale))
		bchg = true;

	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

void AuChromoPageReproduction::AGenFemaleChanged(int value)
{
	Q_UNUSED(value);
	EnableControls();
}

void AuChromoPageReproduction::AGenMaleChanged(int value)
{
	Q_UNUSED(value);
	EnableControls();
}

void AuChromoPageReproduction::ALitterMinChanged(int value)
{
	Q_UNUSED(value);
	EnableControls();
}

void AuChromoPageReproduction::ALitterMaxChanged(int value)
{
	Q_UNUSED(value);
	EnableControls();
}

void AuChromoPageReproduction::BGenFemaleChanged(int value)
{
	Q_UNUSED(value);
	EnableControls();
}

void AuChromoPageReproduction::BGenMaleChanged(int value)
{
	Q_UNUSED(value);
	EnableControls();
}

void AuChromoPageReproduction::BLitterMinChanged(int value)
{
	Q_UNUSED(value);
	EnableControls();
}

void AuChromoPageReproduction::BLitterMaxChanged(int value)
{
	Q_UNUSED(value);
	EnableControls();
}

void AuChromoPageReproduction::EnableControls()
{
	bool applyOk = true;

	// If either has genitals, the other must have corresponding genitals or its pointless.
	bool aMale = m_AGenMale->isChecked();
	bool aFemale = m_AGenFemale->isChecked();
	bool bMale = m_BGenMale->isChecked();
	bool bFemale = m_BGenFemale->isChecked();
	bool canAGestate, canBGestate;
	if(aMale || aFemale || bMale || bFemale)
	{
		canAGestate = aFemale;

		// Sexual reproduction requires a female.
		if(!aFemale && !bFemale)
			applyOk = false;
		else if(aMale && !bFemale)
			applyOk = false;
		else if(aFemale && !bMale)
			applyOk = false;
		else if(bMale && !aFemale)
			applyOk = false;
		else if(bFemale && !aMale)
			applyOk = false;

		if(aFemale)
		{
			if(m_ALitterMin->value() > m_ALitterMax->value())
				applyOk = false;
		}
	}
	else
	{
		// Asexual reproduction.
		canAGestate = true;

		//if(m_ABirthPlace->currentData() != m_BBirthPlace->currentData())
			//applyOk = false;

		//if(m_AYoung->currentData() != m_BYoung->currentData())
			//applyOk = false;
	}

	if(aMale || aFemale || bMale || bFemale)
	{
		canBGestate = bFemale;

		// Sexual reproduction requires a female.
		if(!aFemale && !bFemale)
			applyOk = false;
		else if(aMale && !bFemale)
			applyOk = false;
		else if(aFemale && !bMale)
			applyOk = false;
		else if(bMale && !aFemale)
			applyOk = false;
		else if(bFemale && !aMale)
			applyOk = false;

		if(bFemale)
		{
			if(m_BLitterMin->value() > m_BLitterMax->value())
				applyOk = false;
		}
	}
	else
	{
		// Asexual reproduction.
		canBGestate = true;

		//if(m_ABirthPlace->currentData() != m_BBirthPlace->currentData())
			//applyOk = false;

		//if(m_AYoung->currentData() != m_BYoung->currentData())
			//applyOk = false;
	}

	m_ALitterLabel->setVisible(canAGestate);
	m_ALitterMin->setVisible(canAGestate);
	m_ALitterMax->setVisible(canAGestate);
	m_AYoung->setVisible(canAGestate);
	m_ABirthPlace->setVisible(canAGestate);

	m_BLitterLabel->setVisible(canBGestate);
	m_BLitterMin->setVisible(canBGestate);
	m_BLitterMax->setVisible(canBGestate);
	m_BYoung->setVisible(canBGestate);
	m_BBirthPlace->setVisible(canBGestate);

	m_Apply->setEnabled(applyOk);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageMammary::AuChromoPageMammary(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Mammary</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	m_ApplyPair = new QPushButton(QObject::tr("Apply to Pair of Mammaries"));
	m_ApplyPair->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes to both mammaries of the same pair. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_ApplyPair, SIGNAL(clicked()), SIGNAL(ApplyPair()));

	m_ApplyAll = new QPushButton(QObject::tr("Apply to All Mammaries"));
	m_ApplyAll->setToolTip(QObject::tr("<html>"
				"Saves chromosome changes to all mammaries. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_ApplyAll, SIGNAL(clicked()), SIGNAL(ApplyAll()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_AOrgan = new QComboBox;
		m_AOrgan->addItem(tr("Breast"), AuChromoMammary::Breast);
		m_AOrgan->addItem(tr("Nipple"), AuChromoMammary::Nipple);
		//DHH animalia m_AOrgan->addItem(tr("Teat"), AuChromoMammary::Teat);
		//DHH animalia m_AOrgan->addItem(tr("Udder"), AuChromoMammary::Udder);
		m_AOrgan->setToolTip(QObject::tr("<html>"
					"Mammary organ."
					"</html>"));

		m_ADia = new QSpinBox;
		m_ADia->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_ADia->setRange(1, d_DNAMaxMM);
		m_ADia->setToolTip(tr("<html>"
					"Diameter of mammary organ, in mm. "
					"</html>"));
		QLabel* lbDia = new QLabel("mm Diameter");

		m_AFeed = new QCheckBox(tr("Makes Milk"));
		m_AFeed->setToolTip(QObject::tr("<html>"
					"When checked, the mammary organ produces milk. "
					"When unchecked, the mammary organ is only vestigial. "
					"</html>"));

		m_AHide = new QCheckBox(tr("Hidden"));
		m_AHide->setToolTip(QObject::tr("<html>"
					"When checked, the mammary organ is not visible on the body. "
					"When unchecked, the mammary organ is visible on the body. "
					"</html>"));

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_AOrgan, row,0, 1,2);
		row++;
		glay->addWidget(m_AFeed, row,0, 1,2);
		row++;
		glay->addWidget(m_AHide, row,0, 1,2);
		row++;
		glay->addWidget(m_ADia, row,0);
		glay->addWidget(lbDia, row,1);
		row++;
		glay->setRowStretch(row, 1);
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BOrgan = new QComboBox;
		m_BOrgan->addItem(tr("Breast"), AuChromoMammary::Breast);
		m_BOrgan->addItem(tr("Nipple"), AuChromoMammary::Nipple);
		//DHH animalia m_BOrgan->addItem(tr("Teat"), AuChromoMammary::Teat);
		//DHH animalia m_BOrgan->addItem(tr("Udder"), AuChromoMammary::Udder);
		m_BOrgan->setToolTip(QObject::tr("<html>"
					"Mammary organ."
					"</html>"));

		m_BDia = new QSpinBox;
		m_BDia->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_BDia->setRange(1, d_DNAMaxMM);
		m_BDia->setToolTip(tr("<html>"
					"Diameter of mammary organ, in mm. "
					"</html>"));
		QLabel* lbDia = new QLabel("mm Diameter");

		m_BFeed = new QCheckBox(tr("Makes Milk"));
		m_BFeed->setToolTip(QObject::tr("<html>"
					"When checked, the mammary organ produces milk. "
					"When unchecked, the mammary organ is only vestigial. "
					"</html>"));

		m_BHide = new QCheckBox(tr("Hidden"));
		m_BHide->setToolTip(QObject::tr("<html>"
					"When checked, the mammary organ is not visible on the body. "
					"When unchecked, the mammary organ is visible on the body. "
					"</html>"));

		QGridLayout* glay = new QGridLayout;
		int row=0;
		glay->addWidget(m_BOrgan, row,0, 1,2);
		row++;
		glay->addWidget(m_BFeed, row,0, 1,2);
		row++;
		glay->addWidget(m_BHide, row,0, 1,2);
		row++;
		glay->addWidget(m_BDia, row,0);
		glay->addWidget(lbDia, row,1);
		row++;
		glay->setRowStretch(row, 1);
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);
	blay->addWidget(m_ApplyPair);
	blay->addWidget(m_ApplyAll);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageMammary::~AuChromoPageMammary()
{
}

void AuChromoPageMammary::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	Q_UNUSED(dna);
	const AuChromoMammary* at = dynamic_cast<const AuChromoMammary*>(alpha);
	const AuChromoMammary* bt = dynamic_cast<const AuChromoMammary*>(beta);
	if(!at || !bt)
		return;

	m_ADia->setValue(at->SizeMM());
	m_AFeed->setChecked(at->Feed());
	m_AHide->setChecked(at->Show() ? false : true);
	m_AOrgan->setCurrentIndex(m_AOrgan->findData(at->Organ()));

	m_BDia->setValue(bt->SizeMM());
	m_BFeed->setChecked(bt->Feed());
	m_BHide->setChecked(bt->Show() ? false : true);
	m_BOrgan->setCurrentIndex(m_BOrgan->findData(bt->Organ()));
}

int AuChromoPageMammary::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoMammary* at = dynamic_cast<AuChromoMammary*>(alpha);
	AuChromoMammary* bt = dynamic_cast<AuChromoMammary*>(beta);
	if(!at || !bt)
		return -1;

	bool achg = false;
	bool bchg = false;

	if(at->SetFeed(m_AFeed->isChecked()))
		achg = true;
	if(at->SetOrgan((AuChromoMammary::ORGAN)m_AOrgan->currentData().toInt()))
		achg = true;
	if(at->SetShow(m_AHide->isChecked() ? false : true))
		achg = true;
	if(at->SetSizeMM(m_ADia->value()))
		achg = true;

	if(bt->SetFeed(m_BFeed->isChecked()))
		bchg = true;
	if(bt->SetOrgan((AuChromoMammary::ORGAN)m_BOrgan->currentData().toInt()))
		bchg = true;
	if(bt->SetShow(m_BHide->isChecked() ? false : true))
		bchg = true;
	if(bt->SetSizeMM(m_BDia->value()))
		bchg = true;

	if(achg && bchg)
		return 3;
	else if(bchg)
		return 2;
	else if(achg)
		return 1;
	else
		return 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageEatsPlants::AuChromoPageEatsPlants(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Eats Plants</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves changes to the chromosome. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	QMap<QString, COMMODITYID> sorted;
	for(int cid=0; cid<CID_Count; cid++)
		if(DbCommodity::IsHerbivoreFood((COMMODITYID)cid) && cid != CID_Water)
			sorted.insert(g_Au.CommodityName((COMMODITYID)cid), (COMMODITYID)cid);

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_APref = new QComboBox;
		m_APref->setToolTip(QObject::tr("<html>"
					"Preferred Plant Food"
					"</html>"));
		m_APref->addItem(tr("No Preference"), CID_Invalid);
		for(QMap<QString, COMMODITYID>::const_iterator si = sorted.cbegin(); si != sorted.cend(); si++)
			m_APref->addItem(QIcon(g_Au.CommodityBitmapName(*si)), si.key(), *si);

		QVBoxLayout* glay = new QVBoxLayout;
		glay->addWidget(m_APref);
		glay->addStretch();
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BPref = new QComboBox;
		m_BPref->setToolTip(QObject::tr("<html>"
					"Preferred Plant Food"
					"</html>"));
		m_BPref->addItem(tr("No Preference"), CID_Invalid);
		for(QMap<QString, COMMODITYID>::const_iterator si = sorted.cbegin(); si != sorted.cend(); si++)
			m_BPref->addItem(QIcon(g_Au.CommodityBitmapName(*si)), si.key(), *si);

		QVBoxLayout* glay = new QVBoxLayout;
		glay->addWidget(m_BPref);
		glay->addStretch();
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageEatsPlants::~AuChromoPageEatsPlants()
{
}

void AuChromoPageEatsPlants::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	Q_UNUSED(dna);
	const AuChromoEatsPlants* at = dynamic_cast<const AuChromoEatsPlants*>(alpha);
	const AuChromoEatsPlants* bt = dynamic_cast<const AuChromoEatsPlants*>(beta);
	if(!at || !bt)
		return;

	m_APref->setCurrentIndex(m_APref->findData(at->Prefers()));
	m_BPref->setCurrentIndex(m_BPref->findData(bt->Prefers()));
}

int AuChromoPageEatsPlants::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoEatsPlants* at = dynamic_cast<AuChromoEatsPlants*>(alpha);
	AuChromoEatsPlants* bt = dynamic_cast<AuChromoEatsPlants*>(beta);
	if(!at || !bt)
		return -1;

	int apref = m_APref->currentData().toInt();
	at->SetPrefers((COMMODITYID)apref);

	int bpref = m_BPref->currentData().toInt();
	bt->SetPrefers((COMMODITYID)bpref);

	return 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoPageEatsAnimals::AuChromoPageEatsAnimals(QWidget* parent) : AuChromoPageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Eats Animals</b></html>"));

	m_Apply = new QPushButton(QObject::tr("Apply"));
	m_Apply->setToolTip(QObject::tr("<html>"
				"Saves changes to the chromosome. "
				"This button is disabled when incompatible chromatid settings are selected. "
				"</html>"));
	connect(m_Apply, SIGNAL(clicked()), SIGNAL(Apply()));

	QGroupBox* aGroup = new QGroupBox(tr("α Chromatid"));
	{
		m_ABloodSucker = new QCheckBox(tr("Blood Sucker"));
		m_ABloodSucker->setToolTip(QObject::tr("<html>"
					"Drains blood from live animals for food. "
					"</html>"));

		m_ABrain = new QCheckBox(tr("Brain"));
		m_ABrain->setToolTip(QObject::tr("<html>"
					"Eats brains. "
					"</html>"));

		m_ACannibal = new QCheckBox(tr("Cannibal"));
		m_ACannibal->setToolTip(QObject::tr("<html>"
					"Eats its own kind. "
					"</html>"));

		m_ACarne = new QCheckBox(tr("Carne"));
		m_ACarne->setToolTip(QObject::tr("<html>"
					"Eats fresh meat and bones. "
					"</html>"));

		m_ACarrion = new QCheckBox(tr("Carrion"));
		m_ACarrion->setToolTip(QObject::tr("<html>"
					"Eats rotting meat and bones. "
					"</html>"));

		m_AOrgans = new QCheckBox(tr("Organs"));
		m_AOrgans->setToolTip(QObject::tr("<html>"
					"Eats internal organs. "
					"</html>"));

		m_APredator = new QCheckBox(tr("Predator"));
		m_APredator->setToolTip(QObject::tr("<html>"
					"Kills live animals for food. "
					"</html>"));

		QVBoxLayout* glay = new QVBoxLayout;
		glay->addWidget(m_ABloodSucker);
		glay->addWidget(m_ABrain);
		glay->addWidget(m_ACannibal);
		glay->addWidget(m_ACarne);
		glay->addWidget(m_ACarrion);
		glay->addWidget(m_AOrgans);
		glay->addWidget(m_APredator);
		glay->addStretch();
		aGroup->setLayout(glay);
	}

	QGroupBox* bGroup = new QGroupBox(tr("β Chromatid"));
	{
		m_BBloodSucker = new QCheckBox(tr("Blood Sucker"));
		m_BBloodSucker->setToolTip(QObject::tr("<html>"
					"Drains blood from live animals for food. "
					"</html>"));

		m_BBrain = new QCheckBox(tr("Brain"));
		m_BBrain->setToolTip(QObject::tr("<html>"
					"Eats brains. "
					"</html>"));

		m_BCannibal = new QCheckBox(tr("Cannibal"));
		m_BCannibal->setToolTip(QObject::tr("<html>"
					"Eats its own kind. "
					"</html>"));

		m_BCarne = new QCheckBox(tr("Carne"));
		m_BCarne->setToolTip(QObject::tr("<html>"
					"Eats fresh meat and bones. "
					"</html>"));

		m_BCarrion = new QCheckBox(tr("Carrion"));
		m_BCarrion->setToolTip(QObject::tr("<html>"
					"Eats rotting meat and bones. "
					"</html>"));

		m_BOrgans = new QCheckBox(tr("Organs"));
		m_BOrgans->setToolTip(QObject::tr("<html>"
					"Eats internal organs. "
					"</html>"));

		m_BPredator = new QCheckBox(tr("Predator"));
		m_BPredator->setToolTip(QObject::tr("<html>"
					"Kills live animals for food. "
					"</html>"));

		QVBoxLayout* glay = new QVBoxLayout;
		glay->addWidget(m_BBloodSucker);
		glay->addWidget(m_BBrain);
		glay->addWidget(m_BCannibal);
		glay->addWidget(m_BCarne);
		glay->addWidget(m_BCarrion);
		glay->addWidget(m_BOrgans);
		glay->addWidget(m_BPredator);
		glay->addStretch();
		bGroup->setLayout(glay);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lb, 1);
	blay->addWidget(m_Apply);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(aGroup, 1);
	sslay->addWidget(bGroup, 1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addLayout(sslay);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuChromoPageEatsAnimals::~AuChromoPageEatsAnimals()
{
}

void AuChromoPageEatsAnimals::ShowProperties(const AuDNA& dna, const AuChromosome* alpha, const AuChromosome* beta)
{
	Q_UNUSED(dna);
	const AuChromoEatsAnimals* at = dynamic_cast<const AuChromoEatsAnimals*>(alpha);
	const AuChromoEatsAnimals* bt = dynamic_cast<const AuChromoEatsAnimals*>(beta);
	if(!at || !bt)
		return;

	quint8 aEats = at->EatBits();
	m_ABloodSucker->setChecked(aEats & AuChromoEatsAnimals::BloodSucker);
	m_ABrain->setChecked(aEats & AuChromoEatsAnimals::Brain);
	m_ACannibal->setChecked(aEats & AuChromoEatsAnimals::Cannibal);
	m_ACarne->setChecked(aEats & AuChromoEatsAnimals::Carne);
	m_ACarrion->setChecked(aEats & AuChromoEatsAnimals::Carrion);
	m_AOrgans->setChecked(aEats & AuChromoEatsAnimals::Organs);
	m_APredator->setChecked(aEats & AuChromoEatsAnimals::Predator);

	quint8 bEats = at->EatBits();
	m_BBloodSucker->setChecked(bEats & AuChromoEatsAnimals::BloodSucker);
	m_BBrain->setChecked(bEats & AuChromoEatsAnimals::Brain);
	m_BCannibal->setChecked(bEats & AuChromoEatsAnimals::Cannibal);
	m_BCarne->setChecked(bEats & AuChromoEatsAnimals::Carne);
	m_BCarrion->setChecked(bEats & AuChromoEatsAnimals::Carrion);
	m_BOrgans->setChecked(bEats & AuChromoEatsAnimals::Organs);
	m_BPredator->setChecked(bEats & AuChromoEatsAnimals::Predator);
}

int AuChromoPageEatsAnimals::ApplyProperties(AuChromosome* alpha, AuChromosome* beta)
{
	AuChromoEatsAnimals* at = dynamic_cast<AuChromoEatsAnimals*>(alpha);
	AuChromoEatsAnimals* bt = dynamic_cast<AuChromoEatsAnimals*>(beta);
	if(!at || !bt)
		return -1;

	quint8 aEats = 0;
	if(m_ABloodSucker->isChecked())
		aEats |= AuChromoEatsAnimals::BloodSucker;
	if(m_ABrain->isChecked())
		aEats |= AuChromoEatsAnimals::Brain;
	if(m_ACannibal->isChecked())
		aEats |= AuChromoEatsAnimals::Cannibal;
	if(m_ACarne->isChecked())
		aEats |= AuChromoEatsAnimals::Carne;
	if(m_ACarrion->isChecked())
		aEats |= AuChromoEatsAnimals::Carrion;
	if(m_AOrgans->isChecked())
		aEats |= AuChromoEatsAnimals::Organs;
	if(m_APredator->isChecked())
		aEats |= AuChromoEatsAnimals::Predator;
	at->SetEatBits(aEats);

	quint8 bEats = 0;
	if(m_BBloodSucker->isChecked())
		bEats |= AuChromoEatsAnimals::BloodSucker;
	if(m_BBrain->isChecked())
		bEats |= AuChromoEatsAnimals::Brain;
	if(m_BCannibal->isChecked())
		bEats |= AuChromoEatsAnimals::Cannibal;
	if(m_BCarne->isChecked())
		bEats |= AuChromoEatsAnimals::Carne;
	if(m_BCarrion->isChecked())
		bEats |= AuChromoEatsAnimals::Carrion;
	if(m_BOrgans->isChecked())
		bEats |= AuChromoEatsAnimals::Organs;
	if(m_BPredator->isChecked())
		bEats |= AuChromoEatsAnimals::Predator;
	bt->SetEatBits(bEats);

	return 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGenePageBase::AuGenePageBase(QWidget* parent) : QWidget(parent)
{
}

AuGenePageBase::~AuGenePageBase()
{
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGenePageBlank::AuGenePageBlank(QWidget* parent) : AuGenePageBase(parent)
{
}

AuGenePageBlank::~AuGenePageBlank()
{
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGenePageFeathers::AuGenePageFeathers(QWidget* parent) : AuGenePageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Feathers</b></html>"));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuGenePageFeathers::~AuGenePageFeathers()
{
}

void AuGenePageFeathers::ShowProperties(AuChromosome::CTYPE parentType, const AuChromoWhich& parentWhich, const AuGene* gene)
{
	const AuGeneFeathers* st = dynamic_cast<const AuGeneFeathers*>(gene);
	if(!st)
		return;

}

bool AuGenePageFeathers::ApplyProperties(AuGene* gene)
{
	AuGeneFeathers* st = dynamic_cast<AuGeneFeathers*>(gene);
	if(!st)
		return false;

	bool schg = false;


	return schg;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGenePageHair::AuGenePageHair(QWidget* parent) : AuGenePageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Hair</b></html>"));

	QMap<QString, AuFolliclePoint::ROLE> hairRoles;
	AuFolliclePoint::Roles(hairRoles);
	for(QMap<QString, AuFolliclePoint::ROLE>::const_iterator ri = hairRoles.cbegin(); ri != hairRoles.cend(); ri++)
	{
		AuCheckBoxValue64* ck = new AuCheckBoxValue64(ri.key(), *ri);
		ck->hide();
		connect(ck, SIGNAL(StateChanged(int, qint64)), SLOT(HairStateChanged(int, qint64)));
		m_WhereChecks.append(ck);
	}

	m_Length = new QSpinBox;
	m_Length->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_Length->setRange(1, d_DNAMaxMM);
	m_Length->setToolTip(tr("<html>"
				"Terminal length of hair growth. "
				"Hair can be shortened by styling but it can never be longer than its terminal length. "
				"</html>"));
	QLabel* lblen = new QLabel("mm Length");
	connect(m_Length, SIGNAL(valueChanged(int)), SIGNAL(Apply()));

	m_Color = new AuHairColorWidget;
	connect(m_Color, SIGNAL(HairColorChanged(QColor)), SIGNAL(Apply()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lb, row,0, 1,2);
	row++;
	lay->addWidget(m_Color, row,0, 1,2);
	row++;
	lay->addWidget(m_Length, row,0);
	lay->addWidget(lblen, row,1);
	row++;
	for(QList<AuCheckBoxValue64*>::const_iterator ci = m_WhereChecks.cbegin(); ci != m_WhereChecks.cend(); ci++)
	{
		lay->addWidget(*ci, row,0, 1,2);
		row++;
	}
	lay->setRowStretch(row, 1);
	lay->setSpacing(2);
	setLayout(lay);
}

AuGenePageHair::~AuGenePageHair()
{
}

void AuGenePageHair::ShowProperties(AuChromosome::CTYPE parentType, const AuChromoWhich& parentWhich, const AuGene* gene)
{
	const AuGeneHair* st = dynamic_cast<const AuGeneHair*>(gene);
	if(!st)
		return;

	m_Color->blockSignals(true);
	m_Color->SetHairColor(st->Color());
	m_Color->blockSignals(false);

	m_Length->blockSignals(true);
	m_Length->setValue(st->LengthMM());
	m_Length->blockSignals(false);

	// Show/hide the hair location check boxes appropriate to the parent chromosome.
	quint64 wherePossible = AuChromosome::HairWhereBits(parentType, parentWhich);
	for(QList<AuCheckBoxValue64*>::iterator wi = m_WhereChecks.begin(); wi != m_WhereChecks.end(); wi++)
	{
		(*wi)->blockSignals(true);
		(*wi)->setVisible(wherePossible & AuFolliclePoint::RoleBit((AuFolliclePoint::ROLE)(*wi)->Value()));
		(*wi)->setChecked(st->Where() & AuFolliclePoint::RoleBit((AuFolliclePoint::ROLE)(*wi)->Value()));
		(*wi)->blockSignals(false);
	}
}

bool AuGenePageHair::ApplyProperties(AuGene* gene)
{
	AuGeneHair* st = dynamic_cast<AuGeneHair*>(gene);
	if(!st)
		return false;

	bool schg = false;
	// Pure black is not allowed. Our default 'Black' is 9,8,6, which has an hsv value of 9. That is our minimum.
	if(m_Color->HairColor().value() >= 9 && st->SetColor(m_Color->HairColor()))
		schg = true;
	if(st->SetLengthMM(m_Length->value()))
		schg = true;

	quint64 where = 0;
	for(QList<AuCheckBoxValue64*>::iterator wi = m_WhereChecks.begin(); wi != m_WhereChecks.end(); wi++)
		if((*wi)->isChecked())
			where |= AuFolliclePoint::RoleBit((AuFolliclePoint::ROLE)(*wi)->Value());
	if(st->SetWhere(where))
		schg = true;

	return schg;
}

void AuGenePageHair::HairStateChanged(int state, qint64 value)
{
	Q_UNUSED(state);
	Q_UNUSED(value);
	emit Apply();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGenePageScales::AuGenePageScales(QWidget* parent) : AuGenePageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Scales</b></html>"));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuGenePageScales::~AuGenePageScales()
{
}

void AuGenePageScales::ShowProperties(AuChromosome::CTYPE parentType, const AuChromoWhich& parentWhich, const AuGene* gene)
{
	const AuGeneScales* st = dynamic_cast<const AuGeneScales*>(gene);
	if(!st)
		return;

}

bool AuGenePageScales::ApplyProperties(AuGene* gene)
{
	AuGeneScales* st = dynamic_cast<AuGeneScales*>(gene);
	if(!st)
		return false;

	bool schg = false;


	return schg;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGenePageSpines::AuGenePageSpines(QWidget* parent) : AuGenePageBase(parent)
{
	QLabel* lb = new QLabel(QObject::tr("<html><b>Spines</b></html>"));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->setSpacing(2);
	lay->setMargin(0);
	setLayout(lay);
}

AuGenePageSpines::~AuGenePageSpines()
{
}

void AuGenePageSpines::ShowProperties(AuChromosome::CTYPE parentType, const AuChromoWhich& parentWhich, const AuGene* gene)
{
	const AuGeneSpines* st = dynamic_cast<const AuGeneSpines*>(gene);
	if(!st)
		return;

}

bool AuGenePageSpines::ApplyProperties(AuGene* gene)
{
	AuGeneSpines* st = dynamic_cast<AuGeneSpines*>(gene);
	if(!st)
		return false;

	bool schg = false;


	return schg;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

