//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausilverwillnotedlg.cpp,v 1.7 2022/02/19 18:16:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausilverwillnotedlg.h"
//
#include "auapp.h"
#include "aucommchannel.h"
#include "aucommpacket.h"
#include "auglobal.h"
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QSpinBox>
#if !defined(QT_NO_DEBUG)
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#endif

AuSilverwillNoteDlg::AuSilverwillNoteDlg(const QString& fromName, int taskId, const QString& taskTitle, const QString& text,
		AuCommChannel* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Silverwill Note"));
	setAuDlgIcon(QIcon(":data/Silverwill.png"));

	m_FromName = fromName;

	QLabel* lbid = new QLabel(tr("Id"), this);
	lbid->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_IdSpin = new QSpinBox(this);
	m_IdSpin->setRange(1,100000);
	m_IdSpin->setValue(taskId);
	m_IdSpin->setToolTip(tr("<html>"
				"Select task id to which this note will be added. "
				"The id was <b>%1</b> when this dialog was displayed. "
				"</html>").arg(taskId));

	QLabel* lbtitle = new QLabel(tr("Title"), this);
	lbtitle->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_Title = new QLineEdit(taskTitle, this);
	m_Title->setReadOnly(true);
	m_Title->setToolTip(tr("<html>"
				"Title of task %1. "
				"</html>").arg(taskId));

	m_OkButton = new QPushButton(QIcon(":data/note12x12.png"), tr("Create Note"), this);
	m_OkButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_OkButton->setToolTip(tr("<html>"
				"Creates a new Silverwill note with the information provided. "
				"Disabled if additional information is blank. "
				"</html>"));
	connect(m_OkButton, SIGNAL(clicked()), SLOT(CreateNote()));

	m_OkReply = new QPushButton(QIcon(":data/note12x12.png"), tr("Create and Reply"), this);
	m_OkReply->setDefault(true);
	m_OkReply->setAutoDefault(true);
	m_OkReply->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_OkReply->setToolTip(tr("<html>"
				"Creates a new Silverwill note with the information provided. "
				"Transmits a reply message on the comm channel. "
				"Disabled if additional information is blank. "
				"</html>"));
	connect(m_OkReply, SIGNAL(clicked()), SLOT(CreateNoteAndReply()));

	QLabel* lbinfo = new QLabel(this);
	lbinfo->setPixmap(QPixmap(":data/note12x12.png"));

	QLabel* lbinfo2 = new QLabel(tr("Additional Information"), this);

	m_Info = new QTextEdit(text, this);
	m_Info->setToolTip(tr("<html>"
				"Additional information appears as a <i>note</i> attachment on the task in Silverwill. "
				"</html>"));
	connect(m_Info, SIGNAL(textChanged()), SLOT(EnableControls()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(m_OkButton);
	blay->addWidget(m_OkReply);
	blay->setSpacing(2);
	blay->setMargin(0);

	QGridLayout* lay = new QGridLayout();
	int row=0;
	lay->addWidget(lbid, row,0, 1,2);
	lay->addWidget(m_IdSpin, row,2, 1,2);
	row++;
	lay->addWidget(lbtitle, row,0, 1,2);
	lay->addWidget(m_Title, row,2, 1,2);
	row++;
	lay->addWidget(lbinfo, row,0, 1,1);
	lay->addWidget(lbinfo2, row,1, 1,2);
	row++;
	lay->addWidget(m_Info, row,1, 1,3);
	row++;
	lay->addLayout(blay, row,0, 2,3);
	row++;
	lay->setColumnStretch(2,1);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuSilverwillNoteDlg::~AuSilverwillNoteDlg()
{
}

void AuSilverwillNoteDlg::setVisible(bool visible)
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

void AuSilverwillNoteDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SilverwillNoteDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SilverwillNoteDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SilverwillNoteDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SilverwillNoteDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuSilverwillNoteDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SilverwillNoteDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SilverwillNoteDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SilverwillNoteDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SilverwillNoteDlg/SizeY",sz.height());
}

bool AuSilverwillNoteDlg::CreateSilverwillNote()
{
	bool success = false;
#if !defined(QT_NO_DEBUG)

	// Create the database object inside a scope so it will be destructed before I call removeDatabase.
	int tid = m_IdSpin->value();
	if(tid)
	{
		// Open the database.
		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL7", "HgDb");
		db.setDatabaseName("silverwill");	        				// NOTE: Name is case sensitive.
		db.setHostName("Huey");
		if(!db.open("hguser","hguser"))
		{
			QSqlError er = db.lastError();
			QString sdv = er.driverText();
			QString sdb = er.databaseText();
			qWarning() << "Failed to open Silverwill database connection:" << sdv << sdb;
			return false;
		}

		// Create a note event record, needed for the info text.
		if(!m_Info->toPlainText().trimmed().isEmpty())
		{
			int eid = 0;
			QSqlQuery qid("SELECT nextval('hg_event_eid_seq');", db);
			if(qid.first())
				eid = qid.value(0).toInt();
			if(eid)
			{
				QSqlQuery qa(QString(), db);
				QString sq("INSERT INTO hg_event (eid, tid, type, pid, dt, ival, sval) VALUES (?,?,?,?,?,?,?);");
				qa.prepare(sq);
				qa.addBindValue(eid);
				qa.addBindValue(tid);
				qa.addBindValue(0);	// Note
				qa.addBindValue(0);
				qa.addBindValue(QDateTime::currentDateTime());
				qa.addBindValue(0);
				qa.addBindValue(m_Info->toPlainText().trimmed());

				if(qa.exec())
					success = true;
				else
				{
					qWarning() << "Failed to create Silverwill note event with eid" << eid;
					eid = 0;
				}
			}
			else
				qWarning() << "Failed to get next Silverwill event id for note.";
		}

		// Close the database.
		db.close();
	}

	// Remove the database connection. Must be done after db is destructed.
	QSqlDatabase::removeDatabase("HgDb");
#endif
	return success;
}

void AuSilverwillNoteDlg::CreateNote()
{
	if(CreateSilverwillNote())
	{
		QString newid = tr("<html>The note was attached to <i>Silverwill</i> task <b>%1</b>.</html>")
			.arg(m_IdSpin->value());
		QMessageBox mb(tr("Silverwill Note Added"), newid,
				QMessageBox::Information, QMessageBox::Ok, Qt::NoButton, Qt::NoButton, this);
		mb.exec();
	}
	accept();
}

void AuSilverwillNoteDlg::CreateNoteAndReply()
{
	if(!CommChannel())
	{
		CreateNote();
		return;
	}
	if(CreateSilverwillNote())
	{
		QString sample(m_Info->toPlainText().trimmed());
		if(sample.length() > 100)
			sample = sample.left(100-3) + "...";

		// Send the result on the comm channel as a reply to the person who reported the issue.
		QString text = tr("Thank you %1. Your note '%3' was attached to task %2.")
			.arg(m_FromName)
			.arg(m_IdSpin->value())
			.arg(sample);
		AuCommSilverwill msg(text, CommChannel()->objectName());
		CommChannel()->SendChat(msg);
	}
	accept();
}

void AuSilverwillNoteDlg::EnableControls()
{
	bool enableOk = true;

	if(m_Info->toPlainText().trimmed().isEmpty())
		enableOk = false;

	m_OkButton->setEnabled(enableOk);
	m_OkReply->setEnabled(enableOk && CommChannel() && !CommChannel()->objectName().isEmpty());
}

AuCommChannel* AuSilverwillNoteDlg::CommChannel()
{
	return dynamic_cast<AuCommChannel*>(parent());
}
