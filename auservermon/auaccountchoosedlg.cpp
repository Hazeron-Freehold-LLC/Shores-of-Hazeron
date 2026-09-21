//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auaccountchoosedlg.cpp,v 1.11 2022/02/19 18:40:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auaccountchoosedlg.h"
//
#include "audnabase.h"
#include "auid.h"
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

AuAccountChooseDlg::AuAccountChooseDlg(quint32 hideAccount, QWidget* parent) : QDialog(parent)
{
	setWindowIcon(QIcon(":data/ServerMon.png"));
	setWindowTitle("Shores of Hazeron Account Chooser");

	m_HideAccount = hideAccount;

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

		unitlay->addWidget(m_UnitId[unit], unit,0);
		unitlay->addWidget(m_UnitName[unit], unit,1);
		unitlay->addWidget(m_UnitSex[unit], unit,2);
		unitlay->setColumnStretch(1, 10);
		unitlay->setMargin(0);
		unitlay->setSpacing(2);
	}

	QHBoxLayout* blay = new QHBoxLayout;
	{
		m_Ok = new QPushButton(tr("Ok"), this);
		connect(m_Ok, SIGNAL(clicked()), SLOT(accept()));

		blay->addStretch();
		blay->addWidget(m_Ok);
	}

	QVBoxLayout* mainLay = new QVBoxLayout;
	mainLay->addLayout(acctlay);
	mainLay->addLayout(unitlay);
	mainLay->addLayout(blay);
	mainLay->setMargin(4);
	mainLay->setSpacing(2);
	setLayout(mainLay);
}

AuAccountChooseDlg::~AuAccountChooseDlg()
{
}

void AuAccountChooseDlg::RestoreGeometry()
{
	int posX = auServerMonApp->Settings().value("AccountChooserDlg/PosX", -123).toInt();
	int posY = auServerMonApp->Settings().value("AccountChooserDlg/PosY", -123).toInt();
	QSize szdef = sizeHint();
	int sizeX = auServerMonApp->Settings().value("AccountChooserDlg/SizeX", szdef.width()).toInt();
	int sizeY = auServerMonApp->Settings().value("AccountChooserDlg/SizeY", szdef.height()).toInt();
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuAccountChooseDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	auServerMonApp->Settings().setValue("AccountChooserDlg/PosX", ps.x());
	auServerMonApp->Settings().setValue("AccountChooserDlg/PosY", ps.y());
	auServerMonApp->Settings().setValue("AccountChooserDlg/SizeX", sz.width());
	auServerMonApp->Settings().setValue("AccountChooserDlg/SizeY", sz.height());
}

void AuAccountChooseDlg::setVisible(bool visible)
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

void AuAccountChooseDlg::ListAccounts()
{
	// Create a scope so the database is destructed before we call removeDatabase.
	{
		QSqlDatabase dbLogin = QSqlDatabase::addDatabase("QPSQL7", "AuLogin");
		if(!dbLogin.isValid())
		{
			qWarning() << "error: AuAccountChooseDlg::ListAccounts unable to addDatabase AuLogin";
			return;
		}

		QSqlDatabase dbUniverse = QSqlDatabase::addDatabase("QPSQL7", "AuUniverse");
		if(!dbUniverse.isValid())
		{
			qWarning() << "error: AuAccountChooseDlg::ListAccounts unable to addDatabase AuUniverse";
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
			qWarning() << "error: AuAccountChooseDlg::ListAccounts Failed to open login database" << sdv << sdb;
			return;
		}

		dbUniverse.setDatabaseName("auUniverse");			// NOTE: Name is case sensitive.
		//dbUniverse.setHostName("LoginDb.Hazeron.net");
		dbUniverse.setHostName("u0.Hazeron.net");

		if(!dbUniverse.open("aumaster","Light"))
		{
			QSqlError er = dbUniverse.lastError();
			QString sdv = er.driverText();
			QString sdb = er.databaseText();
			qWarning() << "error: AuAccountChooseDlg::ListAccounts Failed to open universe database" << sdv << sdb;
			return;
		}

		ListAccounts(dbLogin, dbUniverse);
	}
	QSqlDatabase::removeDatabase("AuLogin");
}

void AuAccountChooseDlg::ListAccounts(QSqlDatabase& dbLogin, QSqlDatabase& dbUniverse)
{
	// Login database is only opened in response to certain queries.
	QMultiMap<QString, QByteArray> accountData;			// key is account name in upper case.

	QHash<AuId, QString> personNames;
	QHash<AuId, AuDNABase::CHROMATID> personChromas;
	{
		QString sq("SELECT id, name, sex FROM a_Person;");
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		while(qry.next())
		{
			int qv=0;
			AuId id = AuId(qry.value(qv++).toUInt());
			QString name = qry.value(qv++).toString();
			AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)qry.value(qv++).toInt();
			personNames.insert(id, name);
			personChromas.insert(id, chroma);
		}
	}

	//QString sq("SELECT id, name, status FROM g_Player;");
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

		// Do not list the hidden account.
		if(id == m_HideAccount)
			continue;

		QString name = qry.value(qv++).toString();
		ACCOUNT status = (ACCOUNT)qry.value(qv++).toInt();
		QString email = qry.value(qv++).toString();
		bool loggedIn = qry.value(qv++).toBool();
		QDateTime active = qry.value(qv++).toDateTime();
		active.setTimeSpec(Qt::UTC);
		quint32 days = qry.value(qv++).toUInt();
		AuId unitId[6];
		QString unitName[6];
		int unitSex[6];
		bool anyEmptyUnitSlots = false;
		for(int unit=0; unit<6; unit++)
		{
			unitId[unit] = AuId(qry.value(qv++).toUInt());
			if(unitId[unit].IsNull())
				anyEmptyUnitSlots = true;
			else
			{
				// Get unit name and gender from universe query results.
				QHash<AuId, QString>::const_iterator ni = personNames.find(unitId[unit]);
				if(ni != personNames.cend())
					unitName[unit] = *ni;
				else
					qWarning() << "error: AuAccountChooseDlg::ListAccounts person name not found for id" << unitId[unit];

				QHash<AuId, AuDNABase::CHROMATID>::const_iterator ci = personChromas.find(unitId[unit]);
				if(ci != personChromas.cend())
					unitSex[unit] = *ci;
				else
					qWarning() << "error: AuAccountChooseDlg::ListAccounts person chromatid not found for id" << unitId[unit];
			}
		}

		// Do not list accounts that have no empty unit slots.
		if(!anyEmptyUnitSlots)
			continue;

		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << id;
		out << name;
		out << (quint8)status;
		out << email;
		out << loggedIn;
		out << active;
		out << days;
		for(int unit=0; unit<6; unit++)
		{
			out << unitId[unit];
			out << unitName[unit];
			out << unitSex[unit];
		}

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
		quint8 status = 0;
		QString email;
		bool loggedIn = false;
		QDateTime active;
		quint32 days = 0;
		AuId unitId[6];
		QString unitName[6];
		int unitSex[6] = {0,0,0,0,0,0};;

		QDataStream in(*ai);
		in.setVersion(QDataStream::Qt_4_1);
		in >> id;
		in >> name;
		in >> status;
		in >> email;
		in >> loggedIn;
		in >> active;
		in >> days;
		for(int unit=0; unit<6; unit++)
		{
			in >> unitId[unit];
			in >> unitName[unit];
			in >> unitSex[unit];
		}

		QIcon icon(AcctTypeIcon((ACCOUNT)status));
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

void AuAccountChooseDlg::AccountChanged(int acctIndex)
{
	quint32 id = 0;
	QString name;
	quint8 status = 0;
	QString email;
	bool loggedIn = false;
	QDateTime active;
	quint32 days = 0;
	AuId unitId[6];
	QString unitName[6];
	int unitSex[6] = {0,0,0,0,0,0};;

	if(acctIndex >= 0)
	{
		// Get the data from the combo box.
		QByteArray ba = m_Accounts->itemData(acctIndex).toByteArray();
		QDataStream in(ba);
		in.setVersion(QDataStream::Qt_4_1);
		in >> id;
		in >> name;
		in >> status;
		in >> email;
		in >> loggedIn;
		in >> active;
		in >> days;
		for(int unit=0; unit<6; unit++)
		{
			in >> unitId[unit];
			in >> unitName[unit];
			in >> unitSex[unit];
		}

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
				m_UnitSex[unit]->setText(unitSex[unit] == AuDNABase::Alpha ? tr("Alpha|Male") : tr("Beta|Female"));
			}
			else
			{
				m_UnitId[unit]->setText("");
				m_UnitName[unit]->setText("");
				m_UnitSex[unit]->setText("");
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
		}
	}

	m_Ok->setEnabled(id != 0);
}

quint32 AuAccountChooseDlg::Account() const
{
	return m_Id->text().toUInt();
}

int AuAccountChooseDlg::EmptyUnitIndex() const
{
	for(int unit=0; unit<6; unit++)
		if(m_UnitId[unit]->text().isEmpty())
			return unit;
	return -1;
}

