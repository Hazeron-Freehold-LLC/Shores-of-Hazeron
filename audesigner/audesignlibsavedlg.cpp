//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignlibsavedlg.cpp,v 1.8 2022/02/19 18:25:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audesignlibsavedlg.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "audesign.h"
#include "auframe.h"
#include "auglobal.h"
#include "auscwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTextBrowser>

AuDesignLibSaveDlg::AuDesignLibSaveDlg(const AuId& architectId, const QImage& thumb, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Save Spacecraft Blueprint to Library"));
	setAuDlgIcon(QIcon(":data/Library.png"));

	m_Designs = new AuScWidget(architectId, false, this);
	connect(m_Designs, SIGNAL(itemSelectionChanged()), SLOT(DesignSelectionChanged()));
	connect(auApp, SIGNAL(ScLibraryUpdated()), SLOT(RefreshDesigns()));
	connect(auApp, SIGNAL(ScLibraryUpdated()), SLOT(RefreshThumb()));

	m_DesignName = new QLineEdit(this);
	m_DesignName->setToolTip(tr("<html>"
				"Enter a name for the blueprint in the library. "
				"Initially shows the name of the current blueprint. "
				"Blueprints do not have to have unique names. "
				"More than one blueprint can share the same name. "
				"<br>"
				"<b>This changes the <i>Blueprint Name</i> field in the blueprint properties.</b>"
				"</html>"));
	connect(m_DesignName, SIGNAL(textEdited(const QString&)), SLOT(NameEdited(const QString&)));

	m_DesignId = new QLineEdit(this);
	m_DesignId->setReadOnly(true);
	m_DesignId->setToolTip(tr("<html>"
				"Number of the current blueprint. "
				"Blueprints are identified by number. "
				"</html>"));

	m_DesignThumb = new QLabel(this);
	m_DesignThumb->setAlignment(Qt::AlignCenter);

	QLabel* lbthumb = new QLabel(tr("<html>"
				"This screen grab will be saved with the blueprint. "
				"It will be visible in the blueprint exchange, if you publish the blueprint. "
				"<br>"
				"Use <i>Shift+Esc</i> to hide screen elements if desired, before displaying this window. "
				"</html>"), this);
	lbthumb->setAlignment(Qt::AlignRight|Qt::AlignBottom);
	lbthumb->setWordWrap(true);

	QLabel* lbscreen = new QLabel(this);
	lbscreen->setPixmap(QPixmap::fromImage(thumb));

	m_DesignReport = new QTextBrowser(this);
	m_DesignReport->setOpenExternalLinks(true);
	m_DesignReport->setReadOnly(true);

	m_RequestDesigns = new QPushButton(tr("Refresh Blueprints"), this);
	m_RequestDesigns->setToolTip(tr("<html>"
				"Requests all blueprints that were designed by your avatar. "
				"</html>"));
	connect(m_RequestDesigns, SIGNAL(clicked()), SLOT(RequestDesigns()));

	m_SaveNew = new QPushButton(tr("Save New"), this);
	connect(m_SaveNew, SIGNAL(clicked()), SLOT(SaveNew()));
	m_SaveNew->setAutoDefault(true);
	m_SaveNew->setDefault(true);
	m_SaveNew->setToolTip(tr("<html>"
				"Saves the current blueprint into the library as a new design, with the name in the edit box. "
				"<i>Refresh Blueprints</i> will be needed to list it in library windows. "
				"</html>"));

	m_Replace = new QPushButton(tr("Replace..."), this);
	m_Replace->setToolTip(tr("<html>"
				"Replaces the selected blueprint in the library with the current blueprint. "
				"The name will be changed to the name in the edit box. "
				"If the name changes, <i>Refresh Blueprints</i> will be needed to list its new name in library windows. "
				"</html>"));
	connect(m_Replace, SIGNAL(clicked()), SLOT(Replace()));

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
	blay->addStretch();
	blay->addLayout(didlay);
	blay->addWidget(m_Replace);
	blay->addWidget(m_SaveNew);
	blay->setMargin(0);
	blay->setSpacing(2);

	QHBoxLayout* rowlay = new QHBoxLayout;
	rowlay->addWidget(lbthumb);
	rowlay->addWidget(lbscreen);
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
	g_Au.Settings().readEntry("Widget/DesignLibSaveDlg/Splitter", splitterstate);
	m_Splitter->restoreState(splitterstate);

	EnableControls();
}

AuDesignLibSaveDlg::~AuDesignLibSaveDlg()
{
}

void AuDesignLibSaveDlg::setVisible(bool visible)
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

void AuDesignLibSaveDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/DesignLibSaveDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/DesignLibSaveDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/DesignLibSaveDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/DesignLibSaveDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuDesignLibSaveDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/DesignLibSaveDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/DesignLibSaveDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/DesignLibSaveDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/DesignLibSaveDlg/SizeY",sz.height());
}

quint32 AuDesignLibSaveDlg::DesignId() const
{
	return m_DesignId->text().toUInt();
}

QString AuDesignLibSaveDlg::DesignName() const
{
	return m_DesignName->text().trimmed();
}

void AuDesignLibSaveDlg::SetDesign(quint32 id, const QString& name, const QString& report)
{
	if(id)
		m_DesignId->setText(QVariant(id).toString());
	else
		m_DesignId->setText(tr("New Blueprint"));

	m_DesignName->setText(name.trimmed());

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
	else
		m_DesignThumb->setPixmap(QPixmap());

	m_Designs->blockSignals(true);
	m_Designs->SetCurrent(id);
	m_Designs->blockSignals(false);

	EnableControls();
}

void AuDesignLibSaveDlg::NameEdited(const QString& name)
{
	Q_UNUSED(name);
	EnableControls();
}

void AuDesignLibSaveDlg::SaveNew()
{
	m_DesignId->setText("0");
	accept();
}

void AuDesignLibSaveDlg::Replace()
{
	quint32 designId = DesignId();
	if(!designId)
		return;

	QString smsg = tr("<html>"
			"Confirm overwrite of spacecraft blueprint <b>%1</b> with blueprint named <b>%2</b>. "
			"</html>").arg(designId).arg(DesignName());
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	accept();
}

void AuDesignLibSaveDlg::RequestDesigns()
{
	auApp->ScLibraryRequestDesigns();
}

void AuDesignLibSaveDlg::DesignSelectionChanged()
{
	quint32 designId = m_Designs->CurrentDesignId();
	QString designName = m_Designs->CurrentDesignName();
	QString designReport = m_Designs->CurrentDesignReport();

	SetDesign(designId, designName, designReport);
}

void AuDesignLibSaveDlg::SplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/DesignLibSaveDlg/Splitter", m_Splitter->saveState());
}

quint8 AuDesignLibSaveDlg::DesignState() const
{
	return m_Designs->CurrentDesignState();
}

void AuDesignLibSaveDlg::RefreshDesigns()
{
	m_Designs->RefreshContents();
	EnableControls();
}

void AuDesignLibSaveDlg::EnableControls()
{
	m_SaveNew->setEnabled(IsNameValid(DesignName()));
	m_Replace->setEnabled(DesignId() && IsNameValid(DesignName()) && DesignState() == AuDesign::Edit);
}

void AuDesignLibSaveDlg::RefreshThumb()
{
	quint32 designId = DesignId();
	if(designId)
	{
		QHash<quint32, AuScDesignListEntry>& designs = g_AuClient.LockScDesignList();
		QHash<quint32, AuScDesignListEntry>::const_iterator di = designs.find(designId);
		if(di != designs.end())
			m_DesignThumb->setPixmap(QPixmap::fromImage((*di).Thumb()));
		g_AuClient.UnlockScDesignList();
	}
	else
	{
		m_DesignThumb->setPixmap(QPixmap());
	}
}
