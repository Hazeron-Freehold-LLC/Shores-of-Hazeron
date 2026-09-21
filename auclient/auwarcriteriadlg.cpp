//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auwarcriteriadlg.cpp,v 1.3 2022/02/19 18:17:11 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auwarcriteriadlg.h"
//
#include "audossier.h"
#include "auglobal.h"
#include "dgempire.h"
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QVBoxLayout>

AuWarCriteriaDlg::AuWarCriteriaDlg(const AuWarCriteria& curCrit, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("War Policy"));
	setAuDlgIcon(QIcon(":data/DocWar.png"));

	QLabel* lb = new QLabel(tr("<html>"
				"<p><i>Neutral</i> empires might launch surprise attacks on our empire. "
				"Our political stance toward them should be changed to <i>enemy</i> if they cause too much trouble, "
				"tantamount to declaring war. "
				"This enables military units to protect our assets from them, in self defense.</p>"
				"<p>War policy triggers an automatic change of stance from <i>neutral</i> to <i>enemy</i>. "
				"A threshold can be set for each type of diplomatic incident. "
				"War is declared if the number of <b>New</b> and <b>Open</b> incidents equals or exceeds the limit set for "
				"any incident type.</p>"
				"</html>"), this);
	lb->setWordWrap(true);

	// Create a control for each document type that can be a factor in war criteria.
	QMap<int, quint8> sortOrder;	// Key is AuDocument::SortKey, value is AuDocument::DocumentType.
	QHash<quint8, QLabel*> pics;
	QHash<quint8, QLabel*> docs;
	for(quint8 docType=1; docType < 0xff; docType++)
	{
		AuDocument* doc = AuDocument::CreateDocument(docType);
		if(!doc)
			break;

		if(doc->WarCriteria())
		{
			// The document type can be used in war criteria.
			QLabel* lbpic = new QLabel(this);
			lbpic->setPixmap(QPixmap(doc->ListIcon()).scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));

			QLabel* lbdoc = new QLabel(doc->ListName(), this);

			quint8 curVal;
			QHash<quint8, quint8>::const_iterator ci = curCrit.find(docType);
			if(ci != curCrit.cend())
				curVal = *ci;
			else
				curVal = 0;

			QSlider* slcount = new QSlider(Qt::Horizontal, this);
			slcount->setRange(0, 10);
			slcount->setSingleStep(1);
			slcount->setPageStep(1);
			slcount->setValue(curVal);
			connect(slcount, SIGNAL(valueChanged(int)), SLOT(SliderChanged(int)));

			QLabel* lbcount = new QLabel(QVariant(curVal).toString(), this);
			if(curVal)
				lbcount->setText(tr("%1 Means War").arg(curVal));
			else
				lbcount->setText(tr("Ignored"));

			pics.insert(docType, lbpic);
			docs.insert(docType, lbdoc);
			m_SliderLabels.insert(docType, lbcount);
			m_Sliders.insert(docType, slcount);

			sortOrder.insert(doc->SortKey(), docType);
		}

		delete doc;
	}

	// Sort the sliders according to their desired sort order.
	QGridLayout* doclay = new QGridLayout;
	int row=0;
	for(QMap<int, quint8>::const_iterator si = sortOrder.cbegin(); si != sortOrder.cend(); si++)
	{
		QHash<quint8, QLabel*>::const_iterator pi = pics.find(*si);
		QHash<quint8, QLabel*>::const_iterator di = docs.find(*si);
		QHash<quint8, QLabel*>::const_iterator li = m_SliderLabels.find(*si);
		QHash<quint8, QSlider*>::const_iterator sli = m_Sliders.find(*si);

		doclay->addWidget(*pi, row,0);
		doclay->addWidget(*di, row,1);
		doclay->addWidget(*sli, row,2);
		doclay->addWidget(*li, row,3);
		row++;
	}
	doclay->setColumnMinimumWidth(2,30);
	doclay->setMargin(2);
	doclay->setSpacing(2);

	QPushButton* pbdef = new QPushButton(tr("Default"), this);
	pbdef->setToolTip(tr("Sets all war policy settings to default settings."));
	connect(pbdef, SIGNAL(clicked()), SLOT(Default()));

	QPushButton* pbclear = new QPushButton(tr("Ignore All"), this);
	pbclear->setToolTip(tr("Sets all war policy settings to <i>Ignore</i>. "
				"This disables automatic war."));
	connect(pbclear, SIGNAL(clicked()), SLOT(Clear()));

	QWidget* pubcks = new QWidget;
	pubcks->setLayout(doclay);

	QScrollArea* scroller = new QScrollArea(this);
	scroller->setWidget(pubcks);
	scroller->setWidgetResizable(true);

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("Accepts the war policy settings."));
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* butlay = new QHBoxLayout;
	butlay->addWidget(pbdef);
	butlay->addWidget(pbclear);
	butlay->addWidget(pbok);
	butlay->setMargin(2);
	butlay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->addWidget(scroller);
	lay->addLayout(butlay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuWarCriteriaDlg::~AuWarCriteriaDlg()
{
}

void AuWarCriteriaDlg::setVisible(bool visible)
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

void AuWarCriteriaDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/WarPolicyDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/WarPolicyDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/WarPolicyDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/WarPolicyDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuWarCriteriaDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/WarPolicyDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/WarPolicyDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/WarPolicyDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/WarPolicyDlg/SizeY",sz.height());
}

AuWarCriteria AuWarCriteriaDlg::Criteria() const
{
	AuWarCriteria crit;

	for(QHash<quint8, QSlider*>::const_iterator si = m_Sliders.cbegin(); si != m_Sliders.cend(); si++)
		crit.SetCriteria(si.key(), (*si)->value());

	return crit;
}

void AuWarCriteriaDlg::SliderChanged(int value)
{
	Q_UNUSED(value);
	
	// Insure the buddies all match up with the sliders.
	for(QHash<quint8, QSlider*>::const_iterator si = m_Sliders.cbegin(); si != m_Sliders.cend(); si++)
	{
		QHash<quint8, QLabel*>::const_iterator li = m_SliderLabels.find(si.key());
		if((*si)->value())
			(*li)->setText(tr("%1 Means War").arg((*si)->value()));
		else
			(*li)->setText(tr("Ignored"));
	}
}

void AuWarCriteriaDlg::Clear()
{
	for(QHash<quint8, QSlider*>::const_iterator si = m_Sliders.cbegin(); si != m_Sliders.cend(); si++)
	{
		(*si)->blockSignals(true);
		(*si)->setValue(0);
		(*si)->blockSignals(false);
	}

	// Update all the buddies at once.
	SliderChanged(0);
}

void AuWarCriteriaDlg::Default()
{
	// These values match the DgEmpire constructor defaults.
	DgEmpire def("default");	// For constructor defaults.
	for(QHash<quint8, QSlider*>::const_iterator si = m_Sliders.cbegin(); si != m_Sliders.cend(); si++)
	{
		AuWarCriteria::const_iterator ci = def.m_WarCriteria.find(si.key());
		if(ci == def.m_WarCriteria.end())
			continue;
		(*si)->blockSignals(true);
		(*si)->setValue(*ci);
		(*si)->blockSignals(false);
	}

	// Update all the buddies at once.
	SliderChanged(0);
}
