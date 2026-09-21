//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucontactrec.cpp,v 1.14 2022/02/19 18:35:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucontactrec.h"
//
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

AuContactRec::AuContactRec()
{
	m_idp = 0;		// g_Player id of owner
	m_dispo = 0;	// disposition to contact
}

AuContactRec::AuContactRec(const AuContactRec& c)
{
	m_idp = c.m_idp;		// g_Player id of owner
	m_idu = c.m_idu;		// a_Person unit of contact
	m_name = c.m_name;		// a_Person unit name
	m_dispo = c.m_dispo;	// disposition to contact
}

AuContactRec& AuContactRec::operator=(const AuContactRec& c)
{
	m_idp = c.m_idp;		// g_Player id of owner
	m_idu = c.m_idu;		// a_Person unit of contact
	m_name = c.m_name;		// a_Person unit name
	m_dispo = c.m_dispo;	// disposition to contact
	return *this;
}

void AuContactRec::Store()
{
	QSqlDatabase db = QSqlDatabase::database("AuLogin", false);
	if(!db.isOpen())
		return;

	const char* sTableName = "g_Contact";
	QSqlQuery qa(QString(), db);
	QString sq;
	sq = QString("INSERT INTO %1 (idp,idu,name,dispo) VALUES (?,?,?,?);").arg(sTableName);
	qa.prepare(sq);
	qa.addBindValue(m_idp);
	qa.addBindValue(m_idu.GetID());
	qa.addBindValue(m_name);
	qa.addBindValue(m_dispo);

	if(!qa.exec())
	{
		QString svals;
		svals = QString("%1 %2 %3 %4").arg(m_idp).arg(m_idu.GetID()).arg(m_name).arg(m_dispo);
		qDebug("AuContactRec::Store %s - INSERT FAILED. %s %s",
				(const char*)m_name.toLatin1(), (const char*)sq.toLatin1(), (const char*)svals.toLatin1());
	}
}

void AuContactRec::Rx(QDataStream& in)
{
	in >> m_idp;
	in >> m_idu;
	in >> m_name;
	in >> m_dispo;
}

void AuContactRec::Tx(QDataStream& out) const
{
	out << m_idp;
	out << m_idu;
	out << m_name;
	out << m_dispo;
}

void AuContactRec::Select(quint32 idp, QList<AuContact>& contacts)
{
	QSqlDatabase db = QSqlDatabase::database("AuLogin", false);
	if(!db.isOpen())
		return;

	QSqlQuery qry(QString(), db);
	QString sql("SELECT idu, name, dispo FROM g_Contact WHERE idp=?;");
	qry.prepare(sql);
	qry.addBindValue(idp);
	if(!qry.exec())
		qDebug("AuContactRec::Select %d - SELECT FAILED.", idp);
	while(qry.next())
	{
		AuContact contact;
		contact.m_id = qry.value(0).toUInt();
		contact.m_name = qry.value(1).toString();
		contact.m_bIgnore = qry.value(2).toBool();

		contacts.append(contact);
	}
}

void AuContactRec::Add(quint32 idp, QList<AuContact>& contacts)
{
	m_idp = idp;
	for(QList<AuContact>::iterator it = contacts.begin(); it != contacts.end(); it++)
	{
		m_idu = (*it).m_id;
		m_name = (*it).m_name;
		m_dispo = (*it).m_bIgnore;
		Store();
	}
}

void AuContactRec::Update(quint32 idp, QList<AuContact>& contacts)
{
	QSqlDatabase db = QSqlDatabase::database("AuLogin", false);
	if(!db.isOpen())
		return;

	QSqlQuery qry(QString(), db);
	QString sql("DELETE FROM g_Contact WHERE idp=?;");
	qry.prepare(sql);
	qry.addBindValue(idp);
	if(!qry.exec())
		qDebug("AuContactRec::Update %d - DELETE PRIOR TO UPDATE FAILED.",	idp);
	else
	{
		m_idp = idp;
		for(QList<AuContact>::iterator it = contacts.begin(); it != contacts.end(); it++)
		{
			m_idu = (*it).m_id;
			m_name = (*it).m_name;
			m_dispo = (*it).m_bIgnore;
			Store();
		}
	}
}

void AuContactRec::Delete(quint32 idp, QList<AuContact>& contacts)
{
	QSqlDatabase db = QSqlDatabase::database("AuLogin", false);
	if(!db.isOpen())
		return;
		
	QSqlQuery qry(QString(), db);
	for(QList<AuContact>::iterator it = contacts.begin(); it != contacts.end(); it++)
	{
		QString sql("DELETE FROM g_Contact WHERE idp=? AND idu=?;");
		qry.prepare(sql);
		qry.addBindValue(idp);
		qry.addBindValue((*it).m_id.GetID());

		if(!qry.exec())
			qDebug("AuContactRec::Delete %d - %d - LIST DELETE FAILED.",
					idp, (*it).m_id.GetID());
	}
}

void AuContactRec::Add(quint32 idp, AuContact& contact)
{
	m_idp = idp;
	m_idu = contact.m_id;
	m_name = contact.m_name;
	m_dispo = contact.m_bIgnore;
	Store();
}

void AuContactRec::Delete(quint32 idp, const AuId& idContact)
{
	QSqlDatabase db = QSqlDatabase::database("AuLogin", false);
	if(!db.isOpen())
		return;
		
	QSqlQuery qry(QString(), db);
	QString sql("DELETE FROM g_Contact WHERE idp=? AND idu=?;");
	qry.prepare(sql);
	qry.addBindValue(idp);
	qry.addBindValue(idContact.GetID());

	if(!qry.exec())
		qDebug("AuContactRec::Delete - SINGLE DELETE FAILED. %s %d %d",
				(const char*)sql.toLatin1(), idp, idContact.GetID());
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

void AuContactRecArray::Load()
{
	clear();

	QSqlDatabase db = QSqlDatabase::database("AuLogin", false);
	if(!db.isOpen())
		return;

	AuContactRec ob;
	QSqlQuery qry("SELECT idp, idu, name, dispo FROM g_Contact;", db);
	while(qry.next())
	{
		ob.m_idp = qry.value(0).toInt();
		ob.m_idu = AuId(qry.value(1).toInt());
		ob.m_name = qry.value(2).toString();
		ob.m_dispo = qry.value(3).toInt();
		append(ob);
	}
}
