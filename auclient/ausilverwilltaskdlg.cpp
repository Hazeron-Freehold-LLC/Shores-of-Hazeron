//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausilverwilltaskdlg.cpp,v 1.12 2022/02/19 18:16:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausilverwilltaskdlg.h"
//
#include "auapp.h"
#include "aucommchannel.h"
#include "aucommpacket.h"
#include "auglobal.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QMessageBox>
#include <QVBoxLayout>
#if !defined(QT_NO_DEBUG)
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#endif

AuSilverwillTaskDlg::AuSilverwillTaskDlg(const AuId& fromId, const QString& fromName, const QString& text, AuCommChannel* parent) :
	AuDialog(parent)
{
	setAuDlgTitle(tr("Silverwill Task"));
	setAuDlgIcon(QIcon(":data/Silverwill.png"));

	m_FromName = fromName;
	m_TaskId = 0;

	// Channel name is empty when the task is spontaneously created, not from a message.
	QString origin = QString("%1 (%2)")
			.arg(fromName)
			.arg(fromId.StringId());

	QLabel* lbtitle = new QLabel(tr("Title"), this);
	lbtitle->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_Title = new QLineEdit(text, this);
	m_Title->setToolTip(tr("<html>"
				"Brief one line description of the task. "
				"May not be blank. "
				"</html>"));
	connect(m_Title, SIGNAL(textChanged(const QString&)), SLOT(TitleChanged(const QString&)));

	QLabel* lborigin = new QLabel(tr("Origin"), this);
	lborigin->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_Origin = new QLineEdit(origin, this);
	m_Origin->setToolTip(tr("<html>"
				"Who reported the task. "
				"They might like to know when the task is completed. "
				"Appears as an <i>origin</i> attachment on the task in Silverwill. "
				"</html>"));

	QLabel* lbpri = new QLabel(tr("Priority"), this);
	lbpri->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_Priority = new QComboBox(this);
	//m_Priority->addItem(QIcon(":data/question10x10.png"), "Undetermined", 0);
	m_Priority->addItem(QIcon(":data/fire10x10.png"), "ASAP", 1);
	m_Priority->addItem(QIcon(":data/required10x10.png"), "Required", 2);
	m_Priority->addItem(QIcon(":data/desired10x10.png"), "Desired", 3);
	m_Priority->addItem(QIcon(":data/postponed10x10.png"), "Postpone", 4);
	m_Priority->addItem(QIcon(":data/anchor10x10.png"), "Hold", 5);
	m_Priority->setCurrentIndex(m_Priority->findData(2));
	m_Priority->setToolTip(tr("<html>"
				"Priority for completing the task. "
				"</html>"));

	QLabel* lbcat = new QLabel(tr("Category"), this);
	lbcat->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_Categories = new QComboBox(this);
	ListCategories(m_Categories);
	m_Categories->setToolTip(tr("<html>"
				"New task's category. "
				"</html>"));
	m_Categories->setCurrentIndex(m_Categories->findText("Comm"));

	m_OkButton = new QPushButton(QIcon(":data/Silverwill.png"), tr("Create Task"), this);
	m_OkButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_OkButton->setToolTip(tr("<html>"
				"Creates a new Silverwill task with the information provided. "
				"Disabled if title is blank or no parent is selected. "
				"</html>"));
	connect(m_OkButton, SIGNAL(clicked()), SLOT(CreateTask()));

	m_OkReply = new QPushButton(QIcon(":data/Silverwill.png"), tr("Create and Reply"), this);
	m_OkReply->setDefault(true);
	m_OkReply->setAutoDefault(true);
	m_OkReply->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_OkReply->setToolTip(tr("<html>"
				"Creates a new Silverwill task with the information provided. "
				"Transmits a reply message on the comm channel. "
				"Disabled if title is blank or no parent is selected. "
				"</html>"));
	connect(m_OkReply, SIGNAL(clicked()), SLOT(CreateTaskAndReply()));

	QLabel* lbinfo = new QLabel(this);
	lbinfo->setPixmap(QPixmap(":data/note12x12.png"));

	QLabel* lbinfo2 = new QLabel(tr("Additional Information"), this);

	m_Info = new QTextEdit(this);
	m_Info->setToolTip(tr("<html>"
				"Additional information appears as a <i>note</i> attachment on the task in Silverwill. "
				"</html>"));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(m_OkButton);
	blay->addWidget(m_OkReply);
	blay->setSpacing(2);
	blay->setMargin(0);

	QGridLayout* lay = new QGridLayout();
	int row=0;
	lay->addWidget(lbtitle, row,0, 1,2);
	lay->addWidget(m_Title, row,2, 1,2);
	row++;
	lay->addWidget(lborigin, row,0, 1,2);
	lay->addWidget(m_Origin, row,2, 1,2);
	row++;
	lay->addWidget(lbpri, row,0, 1,2);
	lay->addWidget(m_Priority, row,2, 1,1);
	row++;
	lay->addWidget(lbcat, row,0, 1,2);
	lay->addWidget(m_Categories, row,2);
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

AuSilverwillTaskDlg::~AuSilverwillTaskDlg()
{
}

void AuSilverwillTaskDlg::setVisible(bool visible)
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

void AuSilverwillTaskDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SilverwillTaskDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SilverwillTaskDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SilverwillTaskDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SilverwillTaskDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuSilverwillTaskDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SilverwillTaskDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SilverwillTaskDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SilverwillTaskDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SilverwillTaskDlg/SizeY",sz.height());
}

void AuSilverwillTaskDlg::ListCategories(QComboBox* cb)
{
	cb->addItem("<Uncategorized>", 0);
#if !defined(QT_NO_DEBUG)
	// Create the database object inside a scope so it will be destructed before I call removeDatabase.
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
			return;
		}

		// Select all the categories.
		QString sq("SELECT id, name FROM hg_category WHERE isa='t';");
		QSqlQuery qry(sq, db);
		while(qry.next())
		{
			int qvi=0;	// Query value index.
			int id = qry.value(qvi++).toInt();
			QString name = qry.value(qvi++).toString();
			if(name.length() > 100)
				name = name.left(100) + "...";
			cb->addItem(name, id);
		}

		// Close the database.
		db.close();
	}

	// Remove the database connection. Must be done after db is destructed.
	QSqlDatabase::removeDatabase("HgDb");
#endif
}

int AuSilverwillTaskDlg::CreateSilverwillTask()
{
	int tid = 0;	// The task id if we are successful.
#if !defined(QT_NO_DEBUG)

	// Create the database object inside a scope so it will be destructed before I call removeDatabase.
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
			return 0;
		}

		// Get current parent task id.
		int catid = m_Categories->currentData().toInt();

		// Create a new task record in the database.
		{
			// Acquire a new task id from the database.
			QSqlQuery qid("SELECT nextval('hg_task_tid_seq');", db);
			if(qid.first())
				tid = qid.value(0).toInt();
			if(tid)
			{
				QSqlQuery qa(QString(), db);
				QString sq("INSERT INTO hg_task (tid, name, origin, pri, cid) VALUES (?,?,?,?,?);");
				qa.prepare(sq);
				qa.addBindValue(tid);
				qa.addBindValue(m_Title->text().trimmed());
				qa.addBindValue(m_Origin->text().trimmed());
				qa.addBindValue(m_Priority->itemData(m_Priority->currentIndex()).toInt());
				qa.addBindValue(catid);
				if(!qa.exec())
				{
					qWarning() << "Failed to create Silverwill task with tid" << tid;
					tid = 0;
				}
			}
			else
				qWarning() << "Failed to get next Silverwill task id.";
		}

		// Create a title event record, needed for the create time.
		if(tid)
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
				qa.addBindValue(1);	// Title
				qa.addBindValue(0);
				qa.addBindValue(QDateTime::currentDateTime());
				qa.addBindValue(0);
				qa.addBindValue(m_Title->text().trimmed());

				if(!qa.exec())
				{
					qWarning() << "Failed to create Silverwill title event with eid" << eid;
					eid = 0;
				}
			}
			else
				qWarning() << "Failed to get next Silverwill event id for title.";
		}

		// Create an origin event record, to see the originator name easily.
		if(tid && !m_Info->toPlainText().trimmed().isEmpty())
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
				qa.addBindValue(11);	// Origin
				qa.addBindValue(0);
				qa.addBindValue(QDateTime::currentDateTime());
				qa.addBindValue(0);
				qa.addBindValue(m_Origin->text().trimmed());

				if(!qa.exec())
				{
					qWarning() << "Failed to create Silverwill origin event with eid" << eid;
					eid = 0;
				}
			}
			else
				qWarning() << "Failed to get next Silverwill event id for origin.";
		}

		// Create a note event record, needed for the info text.
		if(tid && !m_Info->toPlainText().trimmed().isEmpty())
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

				if(!qa.exec())
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
	return tid;
}

void AuSilverwillTaskDlg::CreateTask()
{
	m_TaskId = CreateSilverwillTask();	// The task id if we are successful.
	if(m_TaskId)
	{
		QString newid = tr("<html><i>Silverwill</i> task was created with tracking number <b>%1</b>.</html>")
			.arg(m_TaskId);
		QMessageBox mb(tr("Silverwill Task Created"), newid,
				QMessageBox::Information, QMessageBox::Ok, Qt::NoButton, Qt::NoButton, this);
		mb.exec();
	}
	accept();
}

void AuSilverwillTaskDlg::CreateTaskAndReply()
{
	if(!CommChannel())
	{
		CreateTask();
		return;
	}
	m_TaskId = CreateSilverwillTask();	// The task id if we are successful.
	if(m_TaskId)
	{
		QString sample(m_Title->text().trimmed());
		if(sample.length() > 100)
			sample = sample.left(100-3) + "...";

		// Send the result on the comm channel as a reply to the person who reported the issue.
		QString text = tr("Thank you %1. Your report '%3' was logged. The tracking number is %2.")
			.arg(m_FromName)
			.arg(m_TaskId)
			.arg(sample);
		AuCommSilverwill msg(text, CommChannel()->objectName());
		CommChannel()->SendChat(msg);
	}
	accept();
}

void AuSilverwillTaskDlg::ParentChanged(int index)
{
	Q_UNUSED(index);
	EnableControls();
}

void AuSilverwillTaskDlg::TitleChanged(const QString& text)
{
	Q_UNUSED(text);
	EnableControls();
}

void AuSilverwillTaskDlg::EnableControls()
{
	bool enableOk = true;

	if(m_Title->text().trimmed().isEmpty())
		enableOk = false;

	if(m_Priority->currentIndex() < 0)
		enableOk = false;

	m_OkButton->setEnabled(enableOk);
	m_OkReply->setEnabled(enableOk && CommChannel() && !CommChannel()->objectName().isEmpty());
}

int AuSilverwillTaskDlg::Category()
{
	return m_Categories->currentData().toInt();
}

QString AuSilverwillTaskDlg::TaskTitle() const
{
	return m_Title->text().trimmed();
}

AuCommChannel* AuSilverwillTaskDlg::CommChannel()
{
	return dynamic_cast<AuCommChannel*>(parent());
}

