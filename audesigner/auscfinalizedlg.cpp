//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscfinalizedlg.cpp,v 1.10 2022/05/05 16:33:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscfinalizedlg.h"
//
#include "auapp.h"
#include "audesign.h"
#include "auglobal.h"
#include "auframe.h"
#include "dgdata.h"
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

AuScFinalizeDlg::AuScFinalizeDlg(const AuDesign& design, const QImage& thumb, const AuId& archId, const QString& archName,
		QWidget* parent) : AuDialog(parent)
{
	m_DesignId = 0;	// Can be specified by an agent.

	setAuDlgTitle(tr("Finalize %1 Blueprint").arg(design.StructureName()));
	setAuDlgIcon(QIcon(":data/ScFinish.png"));
	m_Assembly = false;
	m_Design = design.Clone();
	m_Design->SetId(0);				// Finalized designs are always saved as new. Architect changed as if it was saved/loaded.
	m_Design->ClearArchitectForFileSave(m_Design->ArchitectId());
	m_Design->SetArchitectAfterFileLoad(archId, archName);
	if(!thumb.isNull())
		m_Design->SetThumbnail(thumb);
	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	g_Au.UnlockData();

	QLabel* lbinfo;
	if(auApp->IsSingleUserDesignerMode())
		lbinfo = new QLabel(tr("<html>"
					"<p>"
					"Finalize a spacecraft blueprint in the off line designer to analyze the blueprint. "
					"Results are reported as if the blueprint were going to be saved to the library. "
					"</p>"

					"<p>"
					"Finalizing a blueprint does not change the current working blueprint. "
					"</p>"
					"</html>"), this);
	else
		lbinfo = new QLabel(tr("<html>"
					"<p>"
					"Finalizing a spacecraft blueprint saves a read-only copy of it to the library. "
					"A blueprint must be finalized before it can be published to the exchange or manufactured in the game universe. "
					"<b>Be sure part seals are set as you want them before finalizing.</b> "
					"</p>"

					"<p>"
					"A blueprint does not have to be published to be used. "
					"Use <i>File|Save to Gear...</i> to save a blueprint to a storage device in your gear. "
					"The <i>Spacecraft Factory</i> window lists blueprints in your gear. "
					"</p>"

					"<p>"
					"Finalizing a blueprint does not change the current working blueprint. "
					"</p>"

					"<p>"
					"The screen grab at the right will be saved with the blueprint. "
					"It will be visible in the exchange, if you publish the blueprint. "
					"</p>"

					"<p>"
					"Use <i>Shift+Esc</i> to hide screen elements if desired, before displaying this window. "
					"</p>"
					"</html>"), this);
	lbinfo->setWordWrap(true);

	QLabel* lbimg = new QLabel(this);
	lbimg->setPixmap(QPixmap::fromImage(thumb));

	m_Verification = new QTextBrowser(this);
	m_Verification->setReadOnly(true);
	m_Verification->setToolTip(tr("<html>"
				"Finalize messages appear here. "
				"</html>"));

	m_DoDesign = new QPushButton(tr("Finalize %2 '%1'...").arg(design.Name()).arg(design.StructureName()), this);
	if(auApp->IsSingleUserDesignerMode())
		m_DoDesign->setToolTip(tr("<html>"
					"The blueprint is analyzed to ensure it can be used to manufacture a spacecraft. "
					"Upon success, a confirmation dialog appears before closing this dialog. "
					"</html>"));
	else
		m_DoDesign->setToolTip(tr("<html>"
					"The blueprint is analyzed to ensure it can be used to manufacture a spacecraft. "
					"Upon success, a confirmation dialog appears before the spacecraft blueprint is saved into the library. "
					"</html>"));
	connect(m_DoDesign, SIGNAL(clicked()), SLOT(DoSpacecraft()));

	if(isAgent && !auApp->IsSingleUserDesignerMode())
	{
		m_DoDesignId = new QPushButton(tr("Finalize %2 '%1' with Id (Agent Only)...")
				.arg(design.Name()).arg(design.StructureName()), this);
		m_DoDesignId->setToolTip(tr("<html>"
					"The design id is specified by an agent for assignment to the design in the database. "
					"The blueprint is analyzed to ensure it can be used to manufacture a spacecraft. "
					"Upon success, a confirmation dialog appears before the spacecraft blueprint is saved into the library. "
					"</html>"));
		connect(m_DoDesignId, SIGNAL(clicked()), SLOT(DoSpacecraftId()));
	}
	else
		m_DoDesignId = 0;

	m_DoAssembly = new QPushButton(tr("Finalize Assembly '%1'...").arg(design.Name()), this);
	m_DoAssembly->setToolTip(tr("<html>"
				"The blueprint is analyzed to ensure it can be used as an assembly. "
				"Upon success, a confirmation dialog appears before the blueprint is saved into the library as an "
				"<i>assembly</i>. "
				"<br>"
				"Assemblies can be used as a source of parts when creating other blueprints. "
				"Any portion of a blueprint can be saved for use as an assembly, regardless of its completeness. "
				"</html>"));
	connect(m_DoAssembly, SIGNAL(clicked()), SLOT(DoAssembly()));

	QVBoxLayout* llay = new QVBoxLayout;
	llay->setMargin(0);
	llay->setSpacing(2);
	llay->addWidget(lbinfo);
	llay->addWidget(m_Verification);

	QVBoxLayout* rlay = new QVBoxLayout;
	rlay->setMargin(0);
	rlay->setSpacing(2);
	rlay->addWidget(lbimg);
	rlay->addStretch();
	rlay->addWidget(m_DoAssembly);
	rlay->addWidget(m_DoDesign);
	if(m_DoDesignId)
		rlay->addWidget(m_DoDesignId);

	QHBoxLayout* lay = new QHBoxLayout;
	lay->setMargin(4);
	lay->setSpacing(2);
	lay->addLayout(llay);
	lay->addLayout(rlay);
	setAuDlgLayout(lay);
}

AuScFinalizeDlg::~AuScFinalizeDlg()
{
	if(m_Design)
		delete m_Design;
}

void AuScFinalizeDlg::setVisible(bool visible)
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

void AuScFinalizeDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/FinalizeSpacecraftDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/FinalizeSpacecraftDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/FinalizeSpacecraftDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/FinalizeSpacecraftDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScFinalizeDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/FinalizeSpacecraftDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/FinalizeSpacecraftDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/FinalizeSpacecraftDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/FinalizeSpacecraftDlg/SizeY",sz.height());
}

void AuScFinalizeDlg::DoAssembly()
{
	// We will only do this once.
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	m_DoAssembly->setEnabled(false);
	m_DoDesign->setEnabled(false);
	if(m_DoDesignId)
		m_DoDesignId->setEnabled(false);

	// Start with an initial message.
	m_Verification->setHtml(tr("Working..."));

	// Finalize the assembly.
	QStringList msgs;
	bool ok = m_Design->FinalizeAssembly(msgs);

	// Show the result.
	QString result;
	for(QStringList::const_iterator mi = msgs.cbegin(); mi != msgs.cend(); mi++)
	{
		if(mi != msgs.cbegin())
			result += "<br>";
		result += *mi;
	}
	m_Verification->setHtml(result);
	QApplication::restoreOverrideCursor();

	// Stop on failure.
	if(!ok)
		return;

	// Assembly is ok. See if they really want to save it.
	QString smsg;
	if(auApp->IsSingleUserDesignerMode())
		smsg = tr("<html>"
				"<p>"
				"The spacecraft assembly blueprint finalized successfully. "
				"If you were on line, the finalized assembly blueprint could be saved to the library at this time. "
				"</p>"
				"</html>");
	else
		smsg = tr("<html>"
				"<p>"
				"The spacecraft assembly blueprint finalized successfully. "
				"Press 'Ok' to save the finalized assembly blueprint to the library. "
				"</p>"
				"</html>");
	AuFrame* frame = auApp->Frame();
	QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Information, QMessageBox::Ok, QMessageBox::Cancel,
			QMessageBox::NoButton, frame);
	if(frame->ExecMessageBox(mb) != QMessageBox::Ok)
	{
		reject();
		return;
	}

	// That's as far as we go in single-player mode. The button should have been disabled.
	if(auApp->IsSingleUserDesignerMode())
		reject();

	m_Assembly = true;
	accept();
}

void AuScFinalizeDlg::DoSpacecraftId()
{
	// 2007 is the design id of the ship in the 8.Beacon story script. So far it is the only one required to have a specific id.
	m_DesignId = QInputDialog::getInt(this, g_Au.ApplicationName(), QObject::tr("Enter the design id.\n"
				"2007 is Dr. Karlov's ship."), 2007);
	DoSpacecraft();
}

void AuScFinalizeDlg::DoSpacecraft()
{
	// We will only do this once.
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	m_DoAssembly->setEnabled(false);
	m_DoDesign->setEnabled(false);
	if(m_DoDesignId)
		m_DoDesignId->setEnabled(false);

	// Start with an initial message.
	m_Verification->setHtml(tr("Working..."));

	QProgressDialog progress(tr("Finalizing Spacecraft Blueprint..."), tr("Cancel Finalize"), 0, 1, this);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMinimumDuration(0);
	progress.setValue(0);

	// Finalize the design.
	QStringList msgs;
	bool ok = m_Design->FinalizeDesign(msgs, progress);
	if(progress.wasCanceled())
		msgs.append(tr("Finalize canceled."));

	// Show the result.
	QString result;
	for(QStringList::const_iterator mi = msgs.cbegin(); mi != msgs.cend(); mi++)
	{
		if(mi != msgs.cbegin())
			result += "<br>";
		result += *mi;
	}
	m_Verification->setHtml(result);
	QApplication::restoreOverrideCursor();
	progress.cancel();

	// Stop on failure.
	if(!ok)
		return;

	// Design is ok. See if they really want to save it.
	QString smsg;
	if(auApp->IsSingleUserDesignerMode())
		smsg = tr("<html>"
				"<p>"
				"The spacecraft blueprint finalized successfully. "
				"If you were on line, the finalized spacecraft blueprint could be saved to the library at this time. "
				"</p>"
				"</html>");
	else
		smsg = tr("<html>"
				"<p>"
				"The spacecraft blueprint finalized successfully. "
				"Press 'Ok' to save the finalized spacecraft blueprint to the library. "
				"</p>"
				"</html>");
	AuFrame* frame = auApp->Frame();
	QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Information, QMessageBox::Ok, QMessageBox::Cancel,
			QMessageBox::NoButton, frame);
	if(frame->ExecMessageBox(mb) != QMessageBox::Ok)
	{
		reject();
		return;
	}

	// That's as far as we go in single-player mode.
	if(auApp->IsSingleUserDesignerMode())
		reject();

	m_Assembly = false;
	accept();
}

QString AuScFinalizeDlg::Report() const
{
	return m_Verification->toHtml();
}
