//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitdesignprops.cpp,v 1.3 2022/02/19 18:25:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auitdesignprops.h"
//
#include "auglobal.h"
#include "auitreport.h"
#include "aulineedit.h"
#include "auscparts.h"
#include "auslider.h"
#include "dgdata.h"
#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QTableWidget>
#include <QTextBrowser>

AuItDesignProps::AuItDesignProps(const AuItDesign& shipRec, QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/c_ItDesignDrawing.png"));
	setAuDlgTitle("Item Design Properties");
	m_DesignRec = shipRec;

	// Design Name
	QLabel* lbName = new QLabel(tr("Design Name"));

	m_Name = new QLineEdit(shipRec.Name(), this);
	m_Name->setToolTip(tr("<html>"
				"The design name must be no more than %1 characters "
				"and it must not contain non-printable characters "
				"or certain reserved characters such as &lt; &gt; &amp; and |."
				"</html>")
			.arg(MaxNameLength()));
	connect(m_Name, SIGNAL(textChanged(const QString&)), SLOT(NameChanged(const QString&)));

	// Design Description
	QLabel* lbDesc = new QLabel(tr("Description"));
	lbDesc->setAlignment(Qt::AlignTop|Qt::AlignLeft);

	m_Description = new QTextEdit(shipRec.Description(), this);
	m_Description->setToolTip(tr("<html>"
				"Enter a description of the design, such as its intended purpose and noteworthy features. "
				"It is ok to summarize but don't literally repeat the design analysis. "
				"</html>"));
	connect(m_Description, SIGNAL(textChanged()), SLOT(DescriptionChanged()));

	// Ground Orientation
	QLabel* lbGroundOri = new QLabel(tr("Preferred Landing Orientation"), this);

	m_GroundOrient = new QComboBox(this);
	for(int s=GORI_ZPosUp; s<GORI_Count; s++)
		m_GroundOrient->addItem(GroundOrientationName((GROUND_ORIENT)s), s);
	m_GroundOrient->setCurrentIndex(m_GroundOrient->findData(shipRec.GroundOrientation()));
	m_GroundOrient->setToolTip(tr("<html>"
				"<b>Ground Orientation</b><br>"
				"<br>"
				"Specifies how the item lies on the ground. "
				"</html>"));
	connect(m_GroundOrient, SIGNAL(currentIndexChanged(int)), SLOT(GroundOrientationChanged(int)));

	m_Ok = new QPushButton(tr("Ok"));
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	connect(m_Ok, SIGNAL(clicked()), SLOT(accept()));
	m_Ok->setToolTip(tr("<html>Accepts changes to the item design. "
				"If this button is disabled, check <i>Design Name</i>."
				"</html>"));

	m_Analysis = new QTextBrowser(this);
	//m_Analysis->setOpenExternalLinks(true);
	m_Analysis->setReadOnly(true);

	m_CommoditiesCombo = new QComboBox(this);
	m_CommoditiesCombo->setToolTip(tr("<html><b>Intended Commodity</b><br>"
				"Evaluates the design for use as a commodity. "
				"Reports success or failure, with a reason for failing. "
				"<br>"
				"Intended commodity setting is used to filter blueprints. "
				"</html>"));
	connect(m_CommoditiesCombo, SIGNAL(currentIndexChanged(int)), SLOT(CommodityChanged(int)));

	m_CommoditiesInfo = new QLabel(this);
	m_CommoditiesInfo->setWordWrap(true);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbName, row,0);
	lay->addWidget(m_Name, row,1, 1,2);
	row++;
	lay->addWidget(lbDesc, row,0);
	lay->addWidget(m_Description, row,1, 1,2);
	row++;
	lay->addWidget(lbGroundOri, row,0);
	lay->addWidget(m_GroundOrient, row,1, 1,2);
	row++;
	lay->setColumnStretch(1,1);
	lay->setSpacing(2);
	lay->setMargin(0);

	QGridLayout* alay = new QGridLayout();
	row=0;
	alay->addWidget(m_CommoditiesCombo, row,0);
	alay->addWidget(m_CommoditiesInfo, row,1);
	row++;
	alay->addWidget(m_Analysis, row,0, 1,2);
	row++;
	alay->setSpacing(2);
	alay->setMargin(0);

	QWidget* controls = new QWidget(this);
	controls->setLayout(lay);

	QWidget* analysis = new QWidget(this);
	analysis->setLayout(alay);

	m_Splitter = new QSplitter;
	m_Splitter->addWidget(controls);
	m_Splitter->addWidget(analysis);
	m_Splitter->setChildrenCollapsible(false);
	m_Splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addStretch();
	blay->addWidget(m_Ok);
	blay->setSpacing(2);
	blay->setMargin(0);

	QVBoxLayout* dlay = new QVBoxLayout();
	dlay->addWidget(m_Splitter);
	dlay->addLayout(blay);

	dlay->setSpacing(2);
	dlay->setMargin(4);
	setAuDlgLayout(dlay);

	QByteArray splitterstate;
	g_Au.Settings().readEntry("Widget/ItDesignProps/Splitter", splitterstate);
	m_Splitter->restoreState(splitterstate);

	ListCommodities();
	EnableControls();
}

AuItDesignProps::~AuItDesignProps()
{
}

void AuItDesignProps::setVisible(bool visible)
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

void AuItDesignProps::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/DesignItemProperties/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/DesignItemProperties/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/DesignItemProperties/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/DesignItemProperties/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuItDesignProps::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/DesignItemProperties/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/DesignItemProperties/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/DesignItemProperties/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/DesignItemProperties/SizeY",sz.height());
}

void AuItDesignProps::UpdateReport()
{
	QString rpt(m_DesignRec.ReportAnalysis());
	if(rpt.isEmpty())
	{
		rpt = tr("<html>"
				"<p>"
				"Design contains %1 parts. "
				"<br>"
				"Design reports no model size. "
				"</p>"

				"<p>"
				"Obstruction model data is needed to analyze a design and report its details. "
				"Be sure the obstruction model is up-to-date and the design contains at least one mesh part. "
				"</p>"

				"<p>"
				"Most changes to the geometry invalidate the obstruction model. "
				"Use <i>Refresh Obstructions</i> to reconstruct the obstruction model after making changes. "
				"</p>"

				"</html>")
			.arg(m_DesignRec.Parts().empty() ? tr("no") : QString("%1").arg(m_DesignRec.Parts().size()));
	}
	m_Analysis->setHtml(rpt);

	int index = m_CommoditiesCombo->currentIndex();
	if(index < 0)
	{
		m_CommoditiesInfo->setText("");
	}
	else
	{
		COMMODITYID cid = (COMMODITYID)m_CommoditiesCombo->itemData(index).toInt();
		if(cid != CID_Invalid)
		{
			AuItReport rpt;
			m_DesignRec.ReportDesign(rpt);

			const DbCommodity& c = g_Au.Commodity(cid);
			int result = rpt.CommodityCanUse(c);
			QString resultMsg = AuItReport::CommodityCanUseMessage(result);
			m_CommoditiesInfo->setText(resultMsg);
		}
		else
			m_CommoditiesInfo->setText("");
	}
}

void AuItDesignProps::EnableControls()
{
	bool enableApply = true;

	if(!m_Name->text().isEmpty() && !IsNameValid(m_Name->text().trimmed()))	// Empty name is ok
		enableApply = false;

	m_Ok->setEnabled(enableApply);
}

void AuItDesignProps::SplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/ItDesignProps/Splitter", m_Splitter->saveState());
}

void AuItDesignProps::NameChanged(const QString& text)
{
	m_DesignRec.SetName(text.trimmed());
	UpdateReport();
	EnableControls();
}

void AuItDesignProps::DescriptionChanged()
{
	QString text;
	if(!m_Description->toPlainText().isEmpty())
		text = m_Description->toHtml();
	m_DesignRec.SetDescription(text);
	UpdateReport();
}

void AuItDesignProps::ListCommodities()
{
	// Alphabetize industries.
	QMap<QString, COMMODITYID> cids;
	for(DbCommodityArray::const_iterator ii = g_Au.m_CommodityArray.cbegin(); ii != g_Au.m_CommodityArray.cend(); ii++)
		if((*ii).m_id != CID_Invalid)
			cids.insert((*ii).m_name, (COMMODITYID)(*ii).m_id);

	m_CommoditiesCombo->blockSignals(true);
	m_CommoditiesCombo->addItem(tr("<Not Specific>"), CID_Invalid);
	for(QMap<QString, COMMODITYID>::const_iterator si = cids.cbegin(); si != cids.cend(); si++)
		m_CommoditiesCombo->addItem(si.key(), *si);
	int curi = m_CommoditiesCombo->findData(m_DesignRec.CommodityId());
	if(curi < 0)
		curi = m_CommoditiesCombo->findData(CID_Invalid);
	m_CommoditiesCombo->setCurrentIndex(curi);
	m_CommoditiesCombo->blockSignals(false);
}

void AuItDesignProps::CommodityChanged(int index)
{
	if(index < 0)
	{
		m_DesignRec.SetCommodityId(CID_Invalid);
	}
	else
	{
		COMMODITYID cid = (COMMODITYID)m_CommoditiesCombo->itemData(index).toInt();
		m_DesignRec.SetCommodityId(cid);
	}

	UpdateReport();
	EnableControls();
}

void AuItDesignProps::GroundOrientationChanged(int index)
{
	m_DesignRec.SetGroundOrientation((GROUND_ORIENT)m_GroundOrient->itemData(index).toInt());
	UpdateReport();
}

