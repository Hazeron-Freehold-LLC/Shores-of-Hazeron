//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitlibopendlg.cpp,v 1.6 2022/02/19 18:25:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auitlibopendlg.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auglobal.h"
#include "auitwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTextBrowser>

AuItLibOpenDlg::AuItLibOpenDlg(const AuId& architectId, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Open Building Blueprint from Library"));
	setAuDlgIcon(QIcon(":data/Library.png"));

	m_Designs = new AuItWidget(architectId, false, this);
	connect(m_Designs, SIGNAL(itemSelectionChanged()), SLOT(DesignSelectionChanged()));
	connect(auApp, SIGNAL(ItLibraryUpdated()), m_Designs, SLOT(RefreshContents()));
	connect(auApp, SIGNAL(ItLibraryUpdated()), SLOT(RefreshThumb()));

	m_DesignName = new QLineEdit(this);
	m_DesignName->setReadOnly(true);
	m_DesignName->setToolTip(tr("<html>"
				"Name of the blueprint in the library. "
				"Blueprints do not have to have unique names. "
				"More than one blueprint can share the same name. "
				"</html>"));

	m_DesignId = new QLineEdit(this);
	m_DesignId->setReadOnly(true);
	m_DesignId->setToolTip(tr("<html>"
				"Number of the current blueprint. "
				"Blueprints are identified by number. "
				"</html>"));

	m_DesignThumb = new QLabel(this);
	m_DesignThumb->setAlignment(Qt::AlignCenter);

	m_DesignReport = new QTextBrowser(this);
	m_DesignReport->setOpenExternalLinks(true);
	m_DesignReport->setReadOnly(true);

	m_RequestDesigns = new QPushButton(tr("Refresh Blueprints"), this);
	m_RequestDesigns->setToolTip(tr("<html>"
				"Requests all blueprints that were designed by your avatar. "
				"</html>"));
	connect(m_RequestDesigns, SIGNAL(clicked()), SLOT(RequestDesigns()));

	m_Ok = new QPushButton(tr("Open Building Blueprint"), this);
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	m_Ok->setEnabled(false);
	m_Ok->setToolTip(tr("<html>"
				"Opens the selected blueprint from the library. "
				"Only the owner of the design studio can open a blueprint. "
				"</html>"));
	connect(m_Ok, SIGNAL(clicked()), SLOT(OpenDesign()));

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

	QHBoxLayout* didlay = new QHBoxLayout;
	didlay->addWidget(m_DesignId);
	didlay->addWidget(m_DesignName, 10);

	QVBoxLayout* blay = new QVBoxLayout;
	blay->addLayout(didlay);
	blay->addWidget(m_Ok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QHBoxLayout* rowlay = new QHBoxLayout;
	rowlay->addStretch();
	rowlay->addLayout(blay);
	rowlay->setMargin(0);
	rowlay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_Splitter, 10);
	lay->addLayout(rowlay);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	QByteArray splitterstate;
	g_Au.Settings().readEntry("Widget/ItLibOpenDlg/Splitter", splitterstate);
	m_Splitter->restoreState(splitterstate);
}

AuItLibOpenDlg::~AuItLibOpenDlg()
{
}

void AuItLibOpenDlg::setVisible(bool visible)
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

void AuItLibOpenDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/ItLibOpenDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/ItLibOpenDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/ItLibOpenDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/ItLibOpenDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuItLibOpenDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/ItLibOpenDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/ItLibOpenDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/ItLibOpenDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/ItLibOpenDlg/SizeY",sz.height());
}

quint32 AuItLibOpenDlg::DesignId() const
{
	return m_DesignId->text().toUInt();
}

QString AuItLibOpenDlg::DesignName() const
{
	return m_DesignName->text().trimmed();
}

void AuItLibOpenDlg::SetDesign(quint32 id, const QString& name, const QString& report)
{
	if(id)
		m_DesignId->setText(QVariant(id).toString());
	else
		m_DesignId->setText("");

	m_DesignName->setText(name.trimmed());

	m_DesignReport->setHtml(report);

	// See if we have a thumb of the design available.
	if(id)
	{
		QHash<quint32, AuItDesignListEntry>& designs = g_AuClient.LockItDesignList();
		QHash<quint32, AuItDesignListEntry>::const_iterator di = designs.find(id);
		if(di != designs.end())
			m_DesignThumb->setPixmap(QPixmap::fromImage((*di).Thumb()));
		g_AuClient.UnlockItDesignList();
	}
	else
		m_DesignThumb->setPixmap(QPixmap());

	m_Designs->blockSignals(true);
	m_Designs->SetCurrent(id);
	m_Designs->blockSignals(false);

	m_Ok->setEnabled(IsNameValid(name.trimmed()));
}

void AuItLibOpenDlg::OpenDesign()
{
	if(DesignId())
		accept();
}

void AuItLibOpenDlg::RequestDesigns()
{
	auApp->ItLibraryRequestDesigns();
}

void AuItLibOpenDlg::DesignSelectionChanged()
{
	quint32 designId = m_Designs->CurrentDesignId();
	QString designName = m_Designs->CurrentDesignName();
	QString designReport = m_Designs->CurrentDesignReport();

	SetDesign(designId, designName, designReport);
}

void AuItLibOpenDlg::SplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/ItLibOpenDlg/Splitter", m_Splitter->saveState());
}

void AuItLibOpenDlg::RefreshThumb()
{
	quint32 designId = DesignId();
	if(designId)
	{
		QHash<quint32, AuItDesignListEntry>& designs = g_AuClient.LockItDesignList();
		QHash<quint32, AuItDesignListEntry>::const_iterator di = designs.find(designId);
		if(di != designs.end())
			m_DesignThumb->setPixmap(QPixmap::fromImage((*di).Thumb()));
		g_AuClient.UnlockItDesignList();
	}
	else
	{
		m_DesignThumb->setPixmap(QPixmap());
	}
}
