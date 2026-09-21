//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucontactrec.h,v 1.6 2022/02/19 18:35:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUCONTACTREC_H
#define AUCONTACTREC_H

#include <auid.h>
#include <aucontact.h>
#include <QString>
#include <QVector>

class AuContactRec
{
friend class AuContactRecArray;
public:
	quint32		m_idp;		// g_Player id of owner
	AuId		m_idu;		// a_Person unit of contact
	QString		m_name;		// a_Person unit name
	quint8		m_dispo;	// disposition to contact, 1=blocked

public:
	AuContactRec();
	AuContactRec(const AuContactRec& c);
	AuContactRec& operator=(const AuContactRec& c);

	void Rx(QDataStream& in);
	void Store();
	void Tx(QDataStream& out) const;

	void Select(quint32 idp, QList<AuContact>& contacts);
	void Update(quint32 idp, QList<AuContact>& contacts);
	void Add(quint32 idp, QList<AuContact>& contacts);
	void Delete(quint32 idp, QList<AuContact>& contacts);
	void Add(quint32 idp, AuContact& contact);
	void Delete(quint32 idp, const AuId& idContact);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuContactRecArray : public QVector<AuContactRec>
{
public:
	void Load();
};

#endif // AUCONTACTREC_H
