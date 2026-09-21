//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auaccountmanagerdlg.cpp,v 1.22 2022/02/19 18:40:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auaccountmanagerdlg.h"
//
#include "auaccountchoosedlg.h"
#include "auid.h"
#include "aupushbutton.h"
#include "auservermonapp.h"
#include "autime.h"
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextBrowser>
#include <QVBoxLayout>

AuAccountManagerDlg::AuAccountManagerDlg(QWidget* parent) : QDialog(parent)
{
	setWindowIcon(QIcon(":data/ServerMon.png"));
	setWindowTitle("Shores of Hazeron Account Manager");

	QGridLayout* acctlay = new QGridLayout;
	{
		QLabel* lbid = new QLabel(tr("Account Id"), this);

		m_Id = new QLineEdit(this);
		m_Id->setReadOnly(true);

		m_Accounts = new QComboBox(this);
		connect(m_Accounts, SIGNAL(currentIndexChanged(int)), SLOT(AccountChanged(int)));

		QLabel* lblogged = new QLabel(tr("Logged In"), this);

		m_Logged = new QLineEdit(this);
		m_Logged->setReadOnly(true);

		m_Email = new QLineEdit(this);
		m_Email->setReadOnly(true);

		QLabel* lbact = new QLabel(tr("Active Days"), this);

		m_Days = new QLineEdit(this);
		m_Days->setReadOnly(true);

		m_Active = new QLineEdit(this);
		m_Active->setReadOnly(true);

		int row=0;
		acctlay->addWidget(lbid, row,0);
		acctlay->addWidget(m_Id, row,1);
		acctlay->addWidget(m_Accounts, row,2);
		row++;
		acctlay->addWidget(lblogged, row,0);
		acctlay->addWidget(m_Logged, row,1);
		acctlay->addWidget(m_Email, row,2);
		row++;
		acctlay->addWidget(lbact, row,0);
		acctlay->addWidget(m_Days, row,1);
		acctlay->addWidget(m_Active, row,2);
		row++;
		acctlay->setColumnStretch(2, 10);
		acctlay->setMargin(0);
		acctlay->setSpacing(2);
	}

	QGridLayout* unitlay = new QGridLayout;
	for(int unit=0; unit<6; unit++)
	{
		m_UnitId[unit] = new QLineEdit(this);
		m_UnitId[unit]->setReadOnly(true);

		m_UnitName[unit] = new QLineEdit(this);
		m_UnitName[unit]->setReadOnly(true);

		m_UnitSex[unit] = new QLineEdit(this);
		m_UnitSex[unit]->setReadOnly(true);

		m_UnitAcct[unit] = new AuPushButtonValue(tr("Change Account..."), unit, this);
		m_UnitAcct[unit]->setToolTip(tr("<html>"
					"Moves the avatar to another account."
					"</html>"));
		connect(m_UnitAcct[unit], SIGNAL(ClickedValue(int)), SLOT(ChangeUnitAccount(int)));

		unitlay->addWidget(m_UnitId[unit], unit,0);
		unitlay->addWidget(m_UnitName[unit], unit,1);
		unitlay->addWidget(m_UnitSex[unit], unit,2);
		unitlay->addWidget(m_UnitAcct[unit], unit,3);
		unitlay->setColumnStretch(1, 10);
		unitlay->setMargin(0);
		unitlay->setSpacing(2);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	{
		QPushButton* findEmail = new QPushButton(tr("Find Email"), this);
		connect(findEmail, SIGNAL(clicked()), SLOT(FindEmail()));

		QPushButton* findName = new QPushButton(tr("Find Avatar Name"), this);
		connect(findName, SIGNAL(clicked()), SLOT(FindAvatarName()));

		QPushButton* findId = new QPushButton(tr("Find Avatar Id"), this);
		connect(findId, SIGNAL(clicked()), SLOT(FindAvatarId()));

		blay->addStretch();
		blay->addWidget(findEmail);
		blay->addWidget(findId);
		blay->addWidget(findName);
	}

	QVBoxLayout* mainLay = new QVBoxLayout;
	mainLay->addLayout(acctlay);
	mainLay->addLayout(unitlay);
	mainLay->addLayout(blay);
	mainLay->addStretch();
	mainLay->setMargin(4);
	mainLay->setSpacing(2);
	setLayout(mainLay);
}

AuAccountManagerDlg::~AuAccountManagerDlg()
{
}

void AuAccountManagerDlg::RestoreGeometry()
{
	int posX = auServerMonApp->Settings().value("AccountManagerDlg/PosX", -123).toInt();
	int posY = auServerMonApp->Settings().value("AccountManagerDlg/PosY", -123).toInt();
	QSize szdef = sizeHint();
	int sizeX = auServerMonApp->Settings().value("AccountManagerDlg/SizeX", szdef.width()).toInt();
	int sizeY = auServerMonApp->Settings().value("AccountManagerDlg/SizeY", szdef.height()).toInt();
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuAccountManagerDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	auServerMonApp->Settings().setValue("AccountManagerDlg/PosX", ps.x());
	auServerMonApp->Settings().setValue("AccountManagerDlg/PosY", ps.y());
	auServerMonApp->Settings().setValue("AccountManagerDlg/SizeX", sz.width());
	auServerMonApp->Settings().setValue("AccountManagerDlg/SizeY", sz.height());
}

void AuAccountManagerDlg::setVisible(bool visible)
{
	if(visible)
	{
		QDialog::setVisible(visible);
		RestoreGeometry();
		ListAccounts();
	}
	else
	{
		SaveGeometry();
		QDialog::setVisible(visible);
	}
}

void AuAccountManagerDlg::ListAccounts()
{
	// Create a scope so the databases are destructed before we call removeDatabase.
	{
		QSqlDatabase dbLogin = QSqlDatabase::addDatabase("QPSQL7", "AuLogin");
		if(!dbLogin.isValid())
		{
			qWarning() << "error: AuAccountManagerDlg::ListAccounts unable to addDatabase AuLogin";
			return;
		}

		dbLogin.setDatabaseName("auLogin");			// NOTE: Name is case sensitive.
		//dbLogin.setHostName("LoginDb.Hazeron.net");
		dbLogin.setHostName("u0.Hazeron.net");

		if(!dbLogin.open("aumaster","Light"))
		{
			QSqlError er = dbLogin.lastError();
			QString sdv = er.driverText();
			QString sdb = er.databaseText();
			qWarning() << "error: AuAccountManagerDlg::ListAccounts Failed to open auLogin database" << sdv << sdb;
			return;
		}

		ListAccounts(dbLogin);
	}
	QSqlDatabase::removeDatabase("AuLogin");
}

void AuAccountManagerDlg::ListAccounts(QSqlDatabase& dbLogin)
{
	// Login database is only opened in response to certain queries.
	QMultiMap<QString, QByteArray> accountData;			// key is account name in upper case.

	QString sq("SELECT id, name, status, email, isloggedin, activated, days, "
			"idu1, "
			"idu2, "
			"idu3, "
			"idu4, "
			"idu5, "
			"idu6 "
			"FROM g_Player;");
	QSqlQuery qry(QString(), dbLogin);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qv = 0;
		quint32 id = qry.value(qv++).toUInt();
		QString name = qry.value(qv++).toString();
		ACCOUNT acctType = (ACCOUNT)qry.value(qv++).toInt();
		QString email = qry.value(qv++).toString();
		bool loggedIn = qry.value(qv++).toBool();
		QDateTime active = qry.value(qv++).toDateTime();
		active.setTimeSpec(Qt::UTC);
		quint32 days = qry.value(qv++).toUInt();

		AuId unitId[6];
		for(int unit=0; unit<6; unit++)
			unitId[unit] = AuId(qry.value(qv++).toUInt());

		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << id;
		out << name;
		out << (quint8)acctType;
		out << email;
		out << loggedIn;
		out << active;
		out << days;
		for(int unit=0; unit<6; unit++)
			out << unitId[unit];

		accountData.insert(name.toUpper(), ba);
	}

	quint32 curId = 0;
	{
		int curIndex = m_Accounts->currentIndex();
		QByteArray ba = m_Accounts->itemData(curIndex).toByteArray();
		QDataStream in(ba);
		in.setVersion(QDataStream::Qt_4_1);
		in >> curId;
	}

	m_Accounts->blockSignals(true);
	m_Accounts->clear();
	for(QMultiMap<QString, QByteArray>::const_iterator ai = accountData.cbegin(); ai != accountData.cend(); ai++)
	{
		quint32 id = 0;
		QString name;
		quint8 acctType = 0;
		QString email;
		bool loggedIn = false;
		QDateTime active;
		quint32 days = 0;
		AuId unitId[6];

		QDataStream in(*ai);
		in.setVersion(QDataStream::Qt_4_1);
		in >> id;
		in >> name;
		in >> acctType;
		in >> email;
		in >> loggedIn;
		in >> active;
		in >> days;
		for(int unit=0; unit<6; unit++)
			in >> unitId[unit];

		QIcon icon(AcctTypeIcon((ACCOUNT)acctType));
		m_Accounts->addItem(icon, name, *ai);
	}

	if(curId == 0)
	{
		m_Accounts->setCurrentIndex(-1);
		AccountChanged(-1);
	}
	else
	{
		// Find the item with the id.
		quint32 accId;
		for(int selIndex=0; selIndex<m_Accounts->count(); selIndex++)
		{
			QByteArray ba = m_Accounts->itemData(selIndex).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);
			in >> accId;
			if(accId == curId)
			{
				m_Accounts->setCurrentIndex(selIndex);
				AccountChanged(selIndex);
				break;
			}
		}
	}

	m_Accounts->blockSignals(false);
}

void AuAccountManagerDlg::AccountChanged(int acctIndex)
{
	quint32 id = 0;
	QString name;
	quint8 acctType = 0;
	QString email;
	bool loggedIn = false;
	QDateTime active;
	quint32 days = 0;
	AuId unitId[6];

	if(acctIndex >= 0)
	{
		// Create a scope so the database is destructed before we call removeDatabase.
		QString unitName[6];
		quint8 unitSex[6];
		{
			QSqlDatabase dbUniverse = QSqlDatabase::addDatabase("QPSQL7", "AuUniverse");
			if(!dbUniverse.isValid())
			{
				qWarning() << "error: AuAccountManagerDlg::AccountChanged unable to addDatabase AuUniverse";
				return;
			}

			dbUniverse.setDatabaseName("auUniverse");			// NOTE: Name is case sensitive.
			//dbUniverse.setHostName("UniverseDb.Hazeron.net");
			dbUniverse.setHostName("u0.Hazeron.net");

			if(!dbUniverse.open("aumaster","Light"))
			{
				QSqlError er = dbUniverse.lastError();
				QString sdv = er.driverText();
				QString sdb = er.databaseText();
				qWarning() << "error: AuAccountManagerDlg::AccountChanged Failed to open auUniverse database" << sdv << sdb;
				return;
			}

			// Get the data from the combo box.
			QByteArray ba = m_Accounts->itemData(acctIndex).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);
			in >> id;
			in >> name;
			in >> acctType;
			in >> email;
			in >> loggedIn;
			in >> active;
			in >> days;
			for(int unit=0; unit<6; unit++)
				in >> unitId[unit];

			for(int unit=0; unit<6; unit++)
			{
				if(!unitId[unit].IsNull())
				{
					// Query universe database for avatar information.
					QString squ;
					QSqlQuery qu(QString(), dbUniverse);
					squ = QString("SELECT name, sex FROM a_Person WHERE id=%1;").arg(unitId[unit].GetID());
					qu.prepare(squ);
					if(!qu.exec())
						qWarning() << "error: AuAccountManagerDlg::AccountChanged squ" << squ << "err" << qu.lastError();
					if(qu.next())
					{
						int qv = 0;	// query value index
						unitName[unit] = qu.value(qv++).toString();
						unitSex[unit] = qu.value(qv++).toUInt();
					}
				}
			}
		}
		QSqlDatabase::removeDatabase("AuUniverse");

		m_Id->setText(QVariant(id).toString());
		if(days)
		{
			m_Active->setText(active.toLocalTime().toString());
			m_Days->setText(QVariant(days).toString());
		}
		else
		{
			m_Active->setText("");
			m_Days->setText("");
		}
		m_Logged->setText(loggedIn ? tr("Yes") : tr("No"));
		m_Email->setText(email);

		for(int unit=0; unit<6; unit++)
		{
			if(!unitId[unit].IsNull())
			{
				m_UnitId[unit]->setText(unitId[unit].StringId());
				m_UnitName[unit]->setText(unitName[unit]);
				m_UnitSex[unit]->setText(unitSex[unit] ? tr("Male") : tr("Female"));
				m_UnitAcct[unit]->setEnabled(true);
			}
			else
			{
				m_UnitId[unit]->setText("");
				m_UnitName[unit]->setText("");
				m_UnitSex[unit]->setText("");
				m_UnitAcct[unit]->setEnabled(false);
			}
		}
	}
	else
	{
		m_Id->setText("");
		m_Active->setText("");
		m_Days->setText("");
		m_Logged->setText("");
		m_Email->setText("");

		for(int unit=0; unit<6; unit++)
		{
			m_UnitId[unit]->setText("");
			m_UnitName[unit]->setText("");
			m_UnitSex[unit]->setText("");
			m_UnitAcct[unit]->setEnabled(false);
		}
	}
}

void AuAccountManagerDlg::FindAvatarName()
{
	bool ok;
	QString avatarName = QInputDialog::getText(this, tr("Find Avatar"), tr("<html>"
				"<p>"
				"Enter avatar name. "
				"It is not case sensitive. "
				"</p>"
				"<p>"
				"Search starts at current account; repeat to find the next match. "
				"Search does not wrap at the end. "
				"</p>"
				"<p>"
				"<b>Be patient. "
				"This is slow.</b>" 
				"</p>"
				"</html>"),
			QLineEdit::Normal, "", &ok);
	if(!ok || avatarName.isEmpty())
		return;

	int selIndex = -1;
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	{
		QSqlDatabase dbUniverse = QSqlDatabase::addDatabase("QPSQL7", "AuUniverse");
		if(!dbUniverse.isValid())
		{
			qWarning() << "error: AuAccountManagerDlg::FindAvatarName unable to addDatabase AuUniverse";
			return;
		}

		dbUniverse.setDatabaseName("auUniverse");			// NOTE: Name is case sensitive.
		//dbUniverse.setHostName("UniverseDb.Hazeron.net");
		dbUniverse.setHostName("u0.Hazeron.net");

		if(!dbUniverse.open("aumaster","Light"))
		{
			QSqlError er = dbUniverse.lastError();
			QString sdv = er.driverText();
			QString sdb = er.databaseText();
			qWarning() << "error: AuAccountManagerDlg::FindAvatarName Failed to open auUniverse database" << sdv << sdb;
			return;
		}

		int curIndex = m_Accounts->currentIndex();
		quint32 id = 0;
		QString name;
		quint8 acctType = 0;
		QString email;
		bool loggedIn = false;
		QDateTime active;
		quint32 days = 0;
		AuId unitId[6];
		for(int index=curIndex+1; index<m_Accounts->count() && selIndex < 0; index++)
		{
			QByteArray ba = m_Accounts->itemData(index).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);
			in >> id;
			in >> name;
			in >> acctType;
			in >> email;
			in >> loggedIn;
			in >> active;
			in >> days;
			for(int unit=0; unit<6; unit++)
				in >> unitId[unit];

			for(int unit=0; unit<6; unit++)
			{
				if(!unitId[unit].IsNull())
				{
					// Query universe database for avatar information.
					QString squ;
					QSqlQuery qu(QString(), dbUniverse);
					squ = QString("SELECT name FROM a_Person WHERE id=%1;").arg(unitId[unit].GetID());
					qu.prepare(squ);
					if(!qu.exec())
						qWarning() << "error: AuAccountManagerDlg::FindAvatarName squ" << squ << "err" << qu.lastError();
					if(qu.next())
					{
						int qv = 0;	// query value index
						QString unitName = qu.value(qv++).toString();
						if(unitName.compare(avatarName, Qt::CaseInsensitive) == 0)
						{
							selIndex = index;
							break;
						}
					}
				}
			}
		}
	}
	QSqlDatabase::removeDatabase("AuUniverse");
	QApplication::restoreOverrideCursor();

	m_Accounts->setCurrentIndex(selIndex);
}

void AuAccountManagerDlg::FindAvatarId()
{
	bool ok;
	QString avatarId = QInputDialog::getText(this, tr("Find Avatar"), tr("<html>"
				"Enter avatar id. "
				"It may be numeric or string. "
				"Search starts at current account; repeat to find the next match."
				"</html>"),
			QLineEdit::Normal, "", &ok);
	if(!ok || avatarId.isEmpty())
		return;

	AuId avid(avatarId);
	if(avid.IsNull())
		avid = AuId(avatarId.toUInt());
	if(avid.IsNull())
		return;

	quint32 id = 0;
	QString name;
	quint8 acctType = 0;
	QString email;
	bool loggedIn = false;
	QDateTime active;
	quint32 days = 0;
	AuId unitId;

	int curIndex = m_Accounts->currentIndex();
	int selIndex = -1;
	for(int index=curIndex+1; index<m_Accounts->count() && selIndex < 0; index++)
	{
		QByteArray ba = m_Accounts->itemData(index).toByteArray();
		QDataStream in(ba);
		in.setVersion(QDataStream::Qt_4_1);
		in >> id;
		in >> name;
		in >> acctType;
		in >> email;
		in >> loggedIn;
		in >> active;
		in >> days;
		for(int unit=0; unit<6; unit++)
		{
			in >> unitId;
			if(unitId == avid)
			{
				selIndex = index;
				break;
			}
		}
	}

	m_Accounts->setCurrentIndex(selIndex);
}

void AuAccountManagerDlg::FindEmail()
{
	bool ok;
	QString acctEmail = QInputDialog::getText(this, tr("Find Email"), tr("<html>"
				"Enter email address. "
				"It is not case sensitive. "
				"Search starts at current account; repeat to find the next match."
				"</html>"),
			QLineEdit::Normal, "", &ok);
	if(!ok || acctEmail.isEmpty())
		return;

	quint32 id = 0;
	QString name;
	quint8 acctType = 0;
	QString email;

	int curIndex = m_Accounts->currentIndex();
	int selIndex = -1;
	for(int index=curIndex+1; index<m_Accounts->count() && selIndex < 0; index++)
	{
		QByteArray ba = m_Accounts->itemData(index).toByteArray();
		QDataStream in(ba);
		in.setVersion(QDataStream::Qt_4_1);
		in >> id;
		in >> name;
		in >> acctType;
		in >> email;
		if(email.compare(acctEmail, Qt::CaseInsensitive) == 0)
		{
			selIndex = index;
			break;
		}
	}

	m_Accounts->setCurrentIndex(selIndex);
}

void AuAccountManagerDlg::ChangeUnitAccount(int unitIndex)
{
	if(unitIndex < 0 || unitIndex > 5)
		return;

	quint32 curAcctId = m_Id->text().toUInt();

	AuAccountChooseDlg dlg(curAcctId, this);
	if(dlg.exec() != QDialog::Accepted)
		return;

	quint32 destAcct = dlg.Account();
	int destIndex = dlg.EmptyUnitIndex();
	if(destIndex < 0 || destIndex > 5)
		return;

	// Create a scope so the database is destructed before we call removeDatabase.
	{
		QSqlDatabase dbLogin = QSqlDatabase::addDatabase("QPSQL7", "AuLogin");
		if(!dbLogin.isValid())
		{
			qWarning() << "error: AuAccountManagerDlg::ChangeUnitAccount unable to addDatabase";
			return;
		}

		dbLogin.setDatabaseName("auLogin");			// NOTE: Name is case sensitive.
		//dbLogin.setHostName("LoginDb.Hazeron.net");
		dbLogin.setHostName("u0.Hazeron.net");

		if(!dbLogin.open("aumaster","Light"))
		{
			QSqlError er = dbLogin.lastError();
			QString sdv = er.driverText();
			QString sdb = er.databaseText();
			qWarning() << "error: AuAccountManagerDlg::ChangeUnitAccount Failed to open database" << sdv << sdb;
			return;
		}

		// Select the avatar information from the source account.
		AuId unitId;
		{
			QString sq = QString("SELECT idu%1 FROM g_Player WHERE id=%2;")
				.arg(unitIndex+1).arg(curAcctId);
			QSqlQuery qry(QString(), dbLogin);
			qry.prepare(sq);
			qry.exec();
			if(qry.next())
			{
				int qv = 0;
				unitId = AuId(qry.value(qv++).toUInt());
			}
		}
		if(unitId.IsNull())
			return;

		// Add the avatar to the accepter's account.
		bool addedToNewAcct = false;
		{
			QString sq;
			QSqlQuery ql(QString(), dbLogin);
			sq = QString("UPDATE g_Player SET idu%1=? WHERE id=%2;")
				.arg(destIndex+1).arg(destAcct);
			ql.prepare(sq);
			ql.addBindValue(unitId.GetID());
			if(!ql.exec())
				qWarning() << "error: Failed to add avatar to new account. ";
			else
				addedToNewAcct = true;
		}

		// Remove the avatar from the offerer's account.
		if(addedToNewAcct)
		{
			QString sq;
			QSqlQuery ql(QString(), dbLogin);
			sq = QString("UPDATE g_Player SET idu%1=? WHERE id=%2;")
				.arg(unitIndex+1).arg(curAcctId);
			ql.prepare(sq);
			ql.addBindValue(0);
			if(!ql.exec())
				qWarning() << "error: Failed to remove avatar from old account. ";
		}
	}
	QSqlDatabase::removeDatabase("AuLogin");

	ListAccounts();
}
