//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitlibdeletedlg.cpp,v 1.6 2022/02/19 18:25:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auitlibdeletedlg.h"
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

AuItLibDeleteDlg::AuItLibDeleteDlg(const AuId& architectId, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Delete Building Blueprint from Library"));
	setAuDlgIcon(QIcon(":data/Library.png"));

	m_Designs = new AuItWidget(architectId, false, this);
	m_Designs->setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(m_Designs, SIGNAL(itemSelectionChanged()), SLOT(DesignSelectionChanged()));
	connect(auApp, SIGNAL(ItLibraryUpdated()), m_Designs, SLOT(RefreshContents()));
	connect(auApp, SIGNAL(ItLibraryUpdated()), SLOT(RefreshThumb()));

	m_DesignThumb = new QLabel(this);
	m_DesignThumb->setAlignment(Qt::AlignCenter);

	m_DesignReport = new QTextBrowser(this);
	m_DesignReport->setOpenExternalLinks(true);
	m_DesignReport->setReadOnly(true);

	m_RequestDesigns = new QPushButton(tr("Refresh Blueprints"), this);
	m_RequestDesigns->setToolTip(tr("<html>"
				"Requests all building blueprints that were designed by your avatar. "
				"</html>"));
	connect(m_RequestDesigns, SIGNAL(clicked()), SLOT(RequestDesigns()));

	m_Ok = new QPushButton(tr("Delete Blueprints..."), this);
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	m_Ok->setEnabled(false);
	m_Ok->setToolTip(tr("<html>"
				"Deletes the selected blueprints. "
				"Blueprints are deactivated in the library. "
				"They will no longer appear on any of your lists after you <i>Refresh Blueprints</i>. "
				"<br>"
				"Only the architect of the design can delete a blueprint. "
				"</html>"));
	connect(m_Ok, SIGNAL(clicked()), SLOT(DeleteDesign()));

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

	QVBoxLayout* blay = new QVBoxLayout;
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
	g_Au.Settings().readEntry("Widget/ItLibDeleteDlg/Splitter", splitterstate);
	m_Splitter->restoreState(splitterstate);
}

AuItLibDeleteDlg::~AuItLibDeleteDlg()
{
}

void AuItLibDeleteDlg::setVisible(bool visible)
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

void AuItLibDeleteDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/ItLibDeleteDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/ItLibDeleteDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/ItLibDeleteDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/ItLibDeleteDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuItLibDeleteDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/ItLibDeleteDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/ItLibDeleteDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/ItLibDeleteDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/ItLibDeleteDlg/SizeY",sz.height());
}

void AuItLibDeleteDlg::SetDesign(quint32 id, const QString& name, const QString& report)
{
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

	m_Ok->setEnabled(IsNameValid(name.trimmed()));
}

void AuItLibDeleteDlg::DeleteDesign()
{
	QHash<quint32, QString> designs;
	m_Designs->SelectedDesigns(designs);
	if(designs.empty())
		return;

	QString smsg;
	if(designs.size() == 1)
		smsg = tr("<html>"
				"Confirm delete of building blueprint <b>%1 %2</b>. "
				"</html>").arg(designs.cbegin().key()).arg(*designs.cbegin());
	else
		smsg = tr("<html>"
				"Confirm delete of %L1 selected building blueprints. "
				"</html>").arg(designs.size());

	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	for(QHash<quint32, QString>::const_iterator di = designs.cbegin(); di != designs.cend(); di++)
		auApp->DesignerLibraryDeleteIt(di.key());
	RequestDesigns();
}

void AuItLibDeleteDlg::RequestDesigns()
{
	auApp->ItLibraryRequestDesigns();
}

void AuItLibDeleteDlg::DesignSelectionChanged()
{
	quint32 designId = m_Designs->CurrentDesignId();
	QString designName = m_Designs->CurrentDesignName();
	QString designReport = m_Designs->CurrentDesignReport();

	SetDesign(designId, designName, designReport);
}

void AuItLibDeleteDlg::SplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/ItLibDeleteDlg/Splitter", m_Splitter->saveState());
}

void AuItLibDeleteDlg::RefreshThumb()
{
	quint32 designId = m_Designs->CurrentDesignId();
	if(designId)
	{
		QHash<quint32, AuItDesignListEntry>& designs = g_AuClient.LockItDesignList();
		QHash<quint32, AuItDesignListEntry>::const_iterator di = designs.find(designId);
		if(di != designs.end())
			m_DesignThumb->setPixmap(QPixmap::fromImage((*di).Thumb()));
		g_AuClient.UnlockItDesignList();
	}
}
