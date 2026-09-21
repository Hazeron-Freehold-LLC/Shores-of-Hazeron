//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscpublishdlg.cpp,v 1.8 2022/02/19 18:16:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscpublishdlg.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "audesign.h"
#include "auglobal.h"
#include "auframe.h"
#include "auscwidget.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSplitter>
#include <QTextBrowser>

AuScPublishDlg::AuScPublishDlg(const AuId& architectId, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Publish Spacecraft Blueprint"));
	setAuDlgIcon(QIcon(":data/Exchange.png"));

	m_Designs = new AuScWidget(architectId, true, this);
	m_Designs->setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(m_Designs, SIGNAL(itemSelectionChanged()), SLOT(DesignSelectionChanged()));
	connect(auApp, SIGNAL(ScLibraryUpdated()), m_Designs, SLOT(RefreshContents()));
	connect(auApp, SIGNAL(ScLibraryUpdated()), SLOT(RefreshThumb()));

	m_DesignThumb = new QLabel(this);
	m_DesignThumb->setAlignment(Qt::AlignCenter);

	m_DesignReport = new QTextBrowser(this);
	m_DesignReport->setOpenExternalLinks(true);
	m_DesignReport->setReadOnly(true);

	m_RequestDesigns = new QPushButton(tr("Refresh Blueprints"), this);
	m_RequestDesigns->setToolTip(tr("<html>"
				"Requests the list all blueprints that were designed by your avatar. "
				"</html>"));
	connect(m_RequestDesigns, SIGNAL(clicked()), SLOT(RequestDesigns()));

	m_Price = new QLineEdit("Free", this);
	m_Price->setToolTip(tr("<html>"
				"Price of the blueprint. "
				"A person must pay this price to buy a copy of the blueprint. "
				"The copy will be bound by the terms of this offer. "
				"</html>"));
	connect(m_Price, SIGNAL(textChanged(const QString&)), SLOT(PriceChanged(const QString&)));

	m_CanCopy = new QRadioButton(tr("Blueprint can be copied."), this);
	m_CanCopy->setToolTip(tr("<html>"
				"The blueprint can be opened in a design studio. "
				"Parts can be copied to the clipboard and the blueprint can be saved to a file or to the library as a new design. "
				"<br>"
				"<i>Final</i> blueprints can be built at a factory. "
				"</html>"));
	m_CanCopy->setChecked(true);
	connect(m_CanCopy, SIGNAL(clicked()), SLOT(EnableControls()));

	m_CanView = new QRadioButton(tr("Blueprint can be viewed."), this);
	m_CanView->setToolTip(tr("<html>"
				"The blueprint can be opened in a design studio. "
				"Parts cannot be copied to the clipboard and the blueprint cannot be saved to a file or to the library. "
				"<br>"
				"<i>Final</i> blueprints can be built at a factory. "
				"</html>"));
	connect(m_CanView, SIGNAL(clicked()), SLOT(EnableControls()));

	m_CanMfg = new QRadioButton(tr("Blueprint can be manufactured."), this);
	m_CanMfg->setToolTip(tr("<html>"
				"Only <i>Final</i> blueprints can be built at a factory. "
				"The blueprint cannot be opened in a design studio. "
				"<br>"
				"This control is disabled if the selected blueprint is an assembly. "
				"</html>"));
	connect(m_CanMfg, SIGNAL(clicked()), SLOT(EnableControls()));

	m_UseOnce = new QCheckBox(tr("Use Once"), this);
	m_UseOnce->setToolTip(tr("<html>"
				"The blueprint can be used at a factory only once. "
				"It can be used to purchase a company spacecraft or it can be used to commission a fleet spacecraft. "
				"After one use, the blueprint is destroyed. "
				"<br>"
				"This control is disabled if the selected blueprint is an assembly. "
				"</html>"));

	m_MinStance = new QComboBox(this);
	m_MinStance->addItem(tr("All Access. All empires can access this blueprint."), POL_Enemy);
	m_MinStance->addItem(tr("Neutral Access. Enemy empires may not access this blueprint."), POL_Neutral);
	m_MinStance->addItem(tr("Friend Access. Enemy and neutral empires may not access this blueprint."), POL_Friend);
	m_MinStance->addItem(tr("Empire Access. Only the publisher's empire may access this blueprint."), POL_Empire);
	m_MinStance->setToolTip(tr("<html>"
				"Recipients of this offer can be limited based upon the political stance of your empire toward their empire. "
				"This restriction is preserved with the blueprint and survives transfer of the media to another avatar. "
				"e.g. If enemy is excluded, no enemy empire may use the media to buy or commission a spacecraft. "
				"<br>"
				"Your current empire is stored with each offer. "
				"<b>If you change empires, you need to republish blueprints that have this restriction.</b>"
				"</html>"));

	m_Ok = new QPushButton(tr("Publish Blueprint"), this);
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	m_Ok->setEnabled(false);
	m_Ok->setToolTip(tr("<html>"
				"Publishes the blueprint with the terms specified. "
				"</html>"));
	connect(m_Ok, SIGNAL(clicked()), SLOT(PublishDesign()));

	// Put it together.
	QHBoxLayout* rdlay = new QHBoxLayout;
	rdlay->addWidget(m_RequestDesigns);
	rdlay->addStretch();
	rdlay->setMargin(0);
	rdlay->setSpacing(2);

	QVBoxLayout* liblay = new QVBoxLayout;
	liblay->addLayout(rdlay);
	liblay->addWidget(m_Designs);
	liblay->setMargin(0);
	liblay->setSpacing(2);

	QVBoxLayout* rlay = new QVBoxLayout;
	rlay->addWidget(m_DesignThumb);
	rlay->addWidget(m_DesignReport);
	rlay->setMargin(0);
	rlay->setSpacing(2);

	QWidget* lwid = new QWidget;
	lwid->setLayout(liblay);

	QWidget* rwid = new QWidget;
	rwid->setLayout(rlay);

	m_Splitter = new QSplitter;
	m_Splitter->addWidget(lwid);
	m_Splitter->addWidget(rwid);
	m_Splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(m_Ok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_Splitter, 10);
	lay->addWidget(m_Price);
	lay->addWidget(m_CanCopy);
	lay->addWidget(m_CanView);
	lay->addWidget(m_CanMfg);
	lay->addWidget(m_MinStance);
	lay->addWidget(m_UseOnce);
	lay->addLayout(blay);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	QByteArray splitterstate;
	g_Au.Settings().readEntry("Widget/ScPublishDlg/Splitter", splitterstate);
	m_Splitter->restoreState(splitterstate);
}

AuScPublishDlg::~AuScPublishDlg()
{
}

void AuScPublishDlg::setVisible(bool visible)
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

void AuScPublishDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/ScPublishDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/ScPublishDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/ScPublishDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/ScPublishDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScPublishDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/ScPublishDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/ScPublishDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/ScPublishDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/ScPublishDlg/SizeY",sz.height());
}

void AuScPublishDlg::SetDesign(quint32 id, const QString& name, const QString& report)
{
	m_DesignReport->setHtml(report);

	// See if we have a thumb of the design available.
	if(id)
	{
		QHash<quint32, AuScDesignListEntry>& designs = g_AuClient.LockScDesignList();
		QHash<quint32, AuScDesignListEntry>::const_iterator di = designs.find(id);
		if(di != designs.end())
			m_DesignThumb->setPixmap(QPixmap::fromImage((*di).Thumb()));
		g_AuClient.UnlockScDesignList();
	}

	m_Ok->setEnabled(IsNameValid(name.trimmed()));
}

void AuScPublishDlg::RequestDesigns()
{
	auApp->ScLibraryRequestDesigns();
}

void AuScPublishDlg::DesignSelectionChanged()
{
	quint32 designId = m_Designs->CurrentDesignId();
	QString designName = m_Designs->CurrentDesignName();
	QString designReport = m_Designs->CurrentDesignReport();

	SetDesign(designId, designName, designReport);
	EnableControls();
}

void AuScPublishDlg::SplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/ScPublishDlg/Splitter", m_Splitter->saveState());
}

void AuScPublishDlg::RefreshThumb()
{
	quint32 designId = m_Designs->CurrentDesignId();
	if(designId)
	{
		QHash<quint32, AuScDesignListEntry>& designs = g_AuClient.LockScDesignList();
		QHash<quint32, AuScDesignListEntry>::const_iterator di = designs.find(designId);
		if(di != designs.end())
			m_DesignThumb->setPixmap(QPixmap::fromImage((*di).Thumb()));
		g_AuClient.UnlockScDesignList();
	}
}

void AuScPublishDlg::PublishDesign()
{
	QHash<quint32, QString> designs;
	m_Designs->SelectedDesigns(designs);
	if(designs.empty())
		return;

	QString smsg;
	if(designs.size() == 1)
		smsg = tr("<html>"
			"Confirm publish of spacecraft blueprint <b>%1 %2</b> for the terms specified. "
			"</html>").arg(designs.cbegin().key()).arg(*designs.cbegin());
	else
		smsg = tr("<html>"
			"Confirm publish of %L1 spacecraft blueprints for the terms specified. "
			"</html>").arg(designs.size());

	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	accept();
}

void AuScPublishDlg::PriceChanged(const QString& text)
{
	Q_UNUSED(text);
	EnableControls();
}

void AuScPublishDlg::EnableControls()
{
	bool canMake = m_Designs->CurrentDesignState() == AuDesign::Final;

	m_CanMfg->setEnabled(canMake);

	m_UseOnce->setEnabled(canMake);

	bool enableOk = true;

	double price = m_Price->text().toDouble();
	if(price < 0 || price > 2000000000)
		enableOk = false;

	QHash<quint32, QString> designs;
	m_Designs->SelectedDesigns(designs);
	if(designs.empty())
		enableOk = false;
	else if(m_CanMfg->isChecked())
	{
		// Manufacture check box does not apply to assemblies.
		designs.clear();
		m_Designs->SelectedDesigns(designs, AuDesign::FinalAssembly);
		if(!designs.empty())
			enableOk = false;
	}

	m_Ok->setEnabled(enableOk);
}

bool AuScPublishDlg::CanCopy() const
{
	return m_CanCopy->isChecked();
}

bool AuScPublishDlg::CanView() const
{
	return m_CanCopy->isChecked() || m_CanView->isChecked();
}

POLITICAL_STANCE AuScPublishDlg::MinStance() const
{
	return (POLITICAL_STANCE)m_MinStance->itemData(m_MinStance->currentIndex()).toInt();
}

quint32 AuScPublishDlg::Price() const
{
	return m_Price->text().toUInt();
}

bool AuScPublishDlg::UseOnce() const
{
	return m_UseOnce->isChecked();
}

void AuScPublishDlg::DesignIds(QList<quint32>& designIds) const
{
	QHash<quint32, QString> designs;
	m_Designs->SelectedDesigns(designs);
	designIds = designs.keys();
}
