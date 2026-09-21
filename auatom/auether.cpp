//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auether.cpp,v 1.81 2022/04/15 16:56:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auether.h"
//
#include "auatomrenderinfo.h"
#include "auatomset.h"
#include "auglobal.h"
#include "daatomlist.h"
#include "dagalaxy.h"
#include "danpc.h"
#include "daperson.h"
#include "dasolarsystem.h"
#include "dauniverse.h"

AuEther::AuEther(const QString& name)
{
	m_Name = name;
}

AuEther::AuEther(const char* file, int line)
{
	m_Name = QObject::tr("%1:%2").arg(file).arg(line);
}

AuEther::~AuEther()
{
	if(g_Au.m_LogMessages & LOG_Etherium)
	{
		m_HashLock.lockForRead();
		if(!m_AtomHash.empty())
		{
			QString logmsg = QString("0x%1 %2 AuEther::~AuEther deleting %3 atoms")
				.arg((size_t)this, 12,16,QChar('0'))
				.arg(m_Name)
				.arg(m_AtomHash.size());
			g_Au.Log(logmsg);
			for(QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomHash.cbegin(); ahi != m_AtomHash.cend(); ahi++)
			{
				logmsg = QString("0x%1 %2 AuEther::~AuEther  0x%3 %4 %5 %6")
					.arg((size_t)this, 12,16,QChar('0'))
					.arg(m_Name)
					.arg((unsigned long long)(*ahi), 12,16,QChar('0'))
					.arg(ahi.key().GetID())
					.arg((*ahi)->Name())
					.arg((*ahi)->AtomTypeName());
				g_Au.Log(logmsg);
			}
		}
		if(!m_AtomSFHash.empty())
		{
			QString logmsg = QString("0x%1 %2 AuEther::~AuEther deleting %3 star field atoms")
				.arg((size_t)this, 12,16,QChar('0'))
				.arg(m_Name)
				.arg(m_AtomSFHash.size());
			g_Au.Log(logmsg);
			for(QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomSFHash.cbegin(); ahi != m_AtomSFHash.cend(); ahi++)
			{
				logmsg = QString("0x%1 %2 AuEther::~AuEther  0x%3 %4 %5 %6")
					.arg((size_t)this, 12,16,QChar('0'))
					.arg(m_Name)
					.arg((unsigned long long)(*ahi), 12,16,QChar('0'))
					.arg(ahi.key().GetID())
					.arg((*ahi)->Name())
					.arg((*ahi)->AtomTypeName());
				g_Au.Log(logmsg);
			}
		}
		m_HashLock.unlock();
	}

	// Delete my atoms.
	DeleteEther();
}

void AuEther::operator>>(QDataStream& out) const
{
	out << m_Name;
	m_HashLock.lockForRead();
	out << (quint32)m_AtomHash.size() + (quint32)m_AtomSFHash.size();
	for(QHash<AuId, DaAtom*>::const_iterator ai = m_AtomHash.cbegin(); ai != m_AtomHash.cend(); ai++)
	{
		out << ai.key();
		out << (quint8)(*ai)->AtomType();
		out << **ai;
	}
	for(QHash<AuId, DaAtom*>::const_iterator ai = m_AtomSFHash.cbegin(); ai != m_AtomSFHash.cend(); ai++)
	{
		out << ai.key();
		out << (quint8)(*ai)->AtomType();
		out << **ai;
	}
	m_HashLock.unlock();
}

void AuEther::operator<<(QDataStream& in)
{
	DeleteEther();
	in >> m_Name;
	quint32 cnt;
	in >> cnt;
	while(cnt--)
	{
		AuId id;
		quint8 atomType;
		in >> id;
		in >> atomType;
		DaAtom* atom = DaAtom::Create((DaAtom::ATOM_TYPE)atomType);
		if(atom)
		{
			in >> *atom;
			atom = atom->UpgradeOrDelete();
			if(atom)
			{
				atom->ConnectSignals();
				AddAtom(atom);
			}
		}
		else
			qWarning() << "error: AuEther::operator<< atom read failure id" << id << "type" << atomType;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// This block of functions manages the contents of the ether.

void AuEther::DeleteEther()
{
	m_TrashLock.lock();
	size_t trash = m_Trash.size();
	m_TrashLock.unlock();

	m_DeleteLock.lockForWrite();
	m_HashLock.lockForWrite();

	if(g_Au.m_LogMessages & LOG_Etherium)
	{
		if(m_AtomHash.size() || m_AtomSFHash.size() || trash)
			g_Au.Log(QObject::tr("0x%1 %2 AuEther::DeleteEther %3 atoms, %4 star field atoms, %5 trash")
					.arg((size_t)this, 12,16,QChar('0'))
					.arg(m_Name)
					.arg(m_AtomHash.size())
					.arg(m_AtomSFHash.size())
					.arg(trash));
	}

	for(QHash<AuId, DaAtom*>::iterator ahi = m_AtomHash.begin(); ahi != m_AtomHash.end(); ahi++)
		delete *ahi;
	m_AtomHash.clear();

	for(QHash<AuId, DaAtom*>::iterator ahi = m_AtomSFHash.begin(); ahi != m_AtomSFHash.end(); ahi++)
		delete *ahi;
	m_AtomSFHash.clear();

	for(QHash<AuId, QReadWriteLock*>::iterator li = m_SubatomLocks.begin(); li != m_SubatomLocks.end(); li++)
		delete *li;
	m_SubatomLocks.clear();

	for(QMultiHash<AuId, AuAtomSet*>::const_iterator si = m_Subatoms.begin(); si != m_Subatoms.end(); si++)
		delete *si;
	m_Subatoms.clear();

	m_HashLock.unlock();
	m_DeleteLock.unlock();

	m_TrashLock.lock();
	for(QHash<DaAtom*, AuElapsedTimer>::iterator ti = m_Trash.begin(); ti != m_Trash.end(); ti++)
		delete ti.key();
	m_Trash.clear();
	m_TrashLock.unlock();

	m_RunawayLock.lockForWrite();
	m_Runaways.clear();
	m_RunawayLock.unlock();
}

void AuEther::AddAtomLocked(DaAtom* atom)
{
	if(!atom)
	{
		qWarning() << "error:" << m_Name << "AuEther::AddAtomLocked null atom pointer rejected";
		return;
	}

	AuId atomId = atom->Id();
	if(atomId.IsNull())
	{
		qWarning() << "warning:" << m_Name << "AuEther::AddAtomLocked atom with null id rejected, type" << atom->AtomTypeName();
		return;
	}

	if(g_Au.m_LogMessages & LOG_Etherium)
		g_Au.Log(QObject::tr("0x%1 %2 AuEther::AddAtomLocked 0x%3 id %4 %5 %6")
				.arg((size_t)this, 12,16,QChar('0'))
				.arg(m_Name)
				.arg((size_t)atom, 12,16,QChar('0'))
				.arg(atomId.GetID())
				.arg(atom->Name())
				.arg(atom->AtomTypeName()));

	if(atom->InStarField())
	{
		// Atom is a sector or a solar system.
		// Check for existing star field atom with that id.
		QHash<AuId, DaAtom*>::iterator ahi = m_AtomSFHash.find(atomId);
		if(ahi != m_AtomSFHash.end())
		{
			// Existing star field atom id found. Remove it before adding the nu one.
			DaAtom* exAtom = *ahi;
			if(exAtom == atom)
			{
				qWarning() << "warning:" << m_Name << "AuEther::AddAtomLocked ignoring star field atom pointer already here"
					<< *atom;
				return;
			}
			if(g_Au.m_LogMessages & LOG_Etherium)
				g_Au.Log(QObject::tr("0x%1 %2 AuEther::AddAtomLocked dupicate star field id, replacing %3, "
							"trashing 0x%4 id %5 %6 %7")
						.arg((size_t)this, 12,16,QChar('0'))
						.arg(m_Name)
						.arg(atomId.GetID())
						.arg((size_t)*ahi, 12,16,QChar('0'))
						.arg(ahi.key().GetID())
						.arg(exAtom->Name())
						.arg(exAtom->AtomTypeName()));
			else
				qWarning() << "warning:" << m_Name << "AuEther::AddAtomLocked duplicate star field id, replacing" << atomId
					<< ", trashing" << *exAtom;
			RemoveAtomLocked(exAtom);

			// Move the replaced star field atom to the trash for safe delete later.
			AuElapsedTimer now(AuElapsedTimer::Now());
			m_TrashLock.lock();
			exAtom->SetTrash();
			m_Trash.insert(exAtom, now);
			m_TrashLock.unlock();
		}

		// Add the nu star field atom to my hash.
		m_AtomSFHash.insert(atomId, atom);
	}
	else
	{
		// Check for existing atom with that id.
		QHash<AuId, DaAtom*>::iterator ahi = m_AtomHash.find(atomId);
		if(ahi != m_AtomHash.end())
		{
			// Existing atom id found. Remove it before adding the nu one.
			DaAtom* exAtom = *ahi;
			if(exAtom == atom)
			{
				qWarning() << "warning:" << m_Name << "AuEther::AddAtomLocked ignoring atom pointer already here" << *atom;
				return;
			}
			if(g_Au.m_LogMessages & LOG_Etherium)
				g_Au.Log(QObject::tr("0x%1 %2 AuEther::AddAtomLocked dupicate id, replacing %3, trashing 0x%4 id %5 %6 %7")
						.arg((size_t)this, 12,16,QChar('0'))
						.arg(m_Name)
						.arg(atomId.GetID())
						.arg((size_t)*ahi, 12,16,QChar('0'))
						.arg(ahi.key().GetID())
						.arg(exAtom->Name())
						.arg(exAtom->AtomTypeName()));
			else
				qWarning() << "warning:" << m_Name << "AuEther::AddAtomLocked duplicate id, replacing" << atomId << ", trashing"
					<< *exAtom;
			RemoveAtomLocked(exAtom);

			// Move the replaced atom to the trash for safe delete later.
			AuElapsedTimer now(AuElapsedTimer::Now());
			m_TrashLock.lock();
			exAtom->SetTrash();
			m_Trash.insert(exAtom, now);
			m_TrashLock.unlock();
		}

		// Add the nu atom to my hash.
		m_AtomHash.insert(atomId, atom);
	}

	// Add the atom to the subatoms table of its nu parent.
	AuId atomParId(atom->ParentId());
	if(!atomParId.IsNull())
	{
		QMultiHash<AuId, AuAtomSet*>::iterator si = m_Subatoms.find(atomParId);
		if(si != m_Subatoms.end())
			(*si)->insert(atom);
		else
			AddRunaway(atomParId, atomId);	// My parent is not here.
	}

	// Create subatom data for the nu atom.
	AuAtomSet* list = new AuAtomSet;
	m_Subatoms.insert(atomId, list);
	QReadWriteLock* lock = new QReadWriteLock();
	m_SubatomLocks.insert(atomId, lock);

	// Insure the atom knows its ether.
	atom->SetEther(this);

	// I could be the parent of one or more runaways.
	FixRunaways(atomId);
}

void AuEther::AddAtom(DaAtom* atom)
{
	m_HashLock.lockForWrite();
	AddAtomLocked(atom);
	m_HashLock.unlock();
}

void AuEther::AddAtoms(const DaAtomList& atoms)
{
	if(atoms.empty())
		return;
	m_HashLock.lockForWrite();
	for(DaAtomList::const_iterator ai = atoms.cbegin(); ai != atoms.cend(); ai++)
		AddAtomLocked(*ai);
	m_HashLock.unlock();
}

void AuEther::RemoveAtom(const AuId& remId, DaAtomList& removed)
{
	// Locate the atom to remove.
	m_HashLock.lockForWrite();
	QHash<AuId, DaAtom*>::iterator ahi = m_AtomHash.find(remId);
	if(ahi != m_AtomHash.end())
	{
		// Make a list that contains the removed atom and all of its subatoms, recursively.
		removed.append(*ahi);			// The list always starts with the removed atom.
		ListSubatomsRecursiveLocked(remId, removed);

		// Remove the atoms from the atom hash.
		for(DaAtomList::const_iterator ri = removed.cbegin(); ri != removed.cend(); ri++)
			RemoveAtomLocked(*ri);
	}
	else
	{
		ahi = m_AtomSFHash.find(remId);
		if(ahi != m_AtomSFHash.end())
		{
			// Make a list that contains the removed atom and all of its subatoms, recursively.
			removed.append(*ahi);			// The list always starts with the removed atom.
			ListSubatomsRecursiveLocked(remId, removed);

			// Remove the atoms from the atom hash.
			for(DaAtomList::const_iterator ri = removed.cbegin(); ri != removed.cend(); ri++)
				RemoveAtomLocked(*ri);
		}
	}
	m_HashLock.unlock();
}

void AuEther::RemoveAtomLocked(DaAtom* atom)
{
	AuId atomId(atom->Id());
	if(g_Au.m_LogMessages & LOG_Etherium)
		g_Au.Log(QObject::tr("0x%1 %2 AuEther::RemoveAtomLocked 0x%3 id %4 %5 %6")
				.arg((size_t)this, 12,16,QChar('0'))
				.arg(m_Name)
				.arg((size_t)atom, 12,16,QChar('0'))
				.arg(atomId.GetID())
				.arg(atom->Name())
				.arg(atom->AtomTypeName()));

	// Null ether causes the atom's MustNotTickTock function to return true so it won't get a tick or tock thread.
	atom->SetEther(0);	
	g_Au.ReleaseTickTock(atomId);	// Release the atom from the tick/tock queue. Current threads could still be running.

	// Remove the atom from my hash tables. Hash lock is currently locked for write.
	if(atom->InStarField())
	{
		QHash<AuId, DaAtom*>::iterator ahi = m_AtomSFHash.find(atomId);
		if(ahi != m_AtomSFHash.end())
			m_AtomSFHash.erase(ahi);		// Remove the star field atom from the atom hash.
	}
	else
	{
		QHash<AuId, DaAtom*>::iterator ahi = m_AtomHash.find(atomId);
		if(ahi != m_AtomHash.end())
			m_AtomHash.erase(ahi);			// Remove the atom from the atom hash.
	}

	// Remove the atom from its former parent's subatoms list.
	AuId parentId(atom->ParentId());
	QMultiHash<AuId, AuAtomSet*>::iterator si = m_Subatoms.find(parentId);
	if(si != m_Subatoms.end())
		(*si)->remove(atom);

	if(g_Au.m_DebugSwitches & 0x00100000)
	{
		// Thorough Remove Check
		// Thoroughly remove the atom from all subatom lists.
		for(QMultiHash<AuId, AuAtomSet*>::const_iterator si = m_Subatoms.cbegin(); si != m_Subatoms.cend(); si++)
		{
			if(!(*si)->remove(atom))
				continue;
			// The atom was found in a subatom list.
			if(g_Au.m_LogMessages & LOG_Etherium)
			{
				g_Au.Log(QObject::tr("error: 0x%1 %2 AuEther::RemoveAtomLocked atom 0x%3 %4 %5 %6 found in list for parent "
							"%7 after removal from parent %8. Removing it from parent's list.")
						.arg((size_t)this, 12,16,QChar('0'))
						.arg(m_Name)
						.arg((size_t)atom, 12,16,QChar('0'))
						.arg(atom->Id().GetID())
						.arg(atom->Name())
						.arg(atom->AtomTypeName())
						.arg(si.key().GetID())
						.arg(parentId.GetID()));
			}
			else
				qWarning() << "error:" << m_Name << "AuEther::RemoveAtomLocked atom" << *atom << "found in list for parent"
					<< si.key() << "after removal from parent" << parentId << ". Removing it from parent's list.";
			Q_ASSERT(false);	// This is a slow test. Fix the bug if we ever hit this assertion. Ran for days with no hit.
		}
	}

	// Remove the atom's subatom lock.
	QHash<AuId, QReadWriteLock*>::iterator li = m_SubatomLocks.find(atomId);
	if(li != m_SubatomLocks.end())
	{
		delete *li;
		m_SubatomLocks.erase(li);
	}

	// Remove the subatom data for the existing atom.
	si = m_Subatoms.find(atomId);
	if(si != m_Subatoms.end())
	{
		delete *si;
		m_Subatoms.erase(si);
	}
}

void AuEther::ChangeParent(const AuId& chgId, const AuAtomPos& pos, bool orient)
{
	// Locate the atom that is changing.
	m_HashLock.lockForRead();
	QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomHash.find(chgId);
	if(ahi == m_AtomHash.cend())
	{
		ahi = m_AtomSFHash.find(chgId);
		if(ahi == m_AtomSFHash.cend())
		{
			m_HashLock.unlock();
			return;
		}
	}

	// Move the atom to its nu parent's hash table.
	DaAtom* atom = *ahi;
	bool nuParentOk;
	do
	{
		// Locate subatom locks for old and nu parents.
		nuParentOk = false;
		AuId oldParId(atom->ParentId());
		QHash<AuId, QReadWriteLock*>::iterator oli = m_SubatomLocks.find(oldParId);		// Old parent
		QHash<AuId, QReadWriteLock*>::iterator nli = m_SubatomLocks.find(pos.ParentId());	// New parent
		if(nli != m_SubatomLocks.end() && oli != m_SubatomLocks.end())
		{
			// We found the subatom locks for both the old and nu parents.
			if(nli != oli)
			{
				// Old and nu parents are not the same.
				// We must lock both locks for write at once. To safely do that, we must always lock them in the same sequence.
				if((*oli) < (*nli))
				{
					(*oli)->lockForWrite();
					(*nli)->lockForWrite();
				}
				else
				{
					(*nli)->lockForWrite();
					(*oli)->lockForWrite();
				}

				// We acquired the write locks.
				if(atom->ParentId() != oldParId)
				{
					// The atom's parent changed while we were waiting for the locks. Another thread got here first. Try again.
					// Unlock sequence is irrelevant.
					(*nli)->unlock();
					(*oli)->unlock();
					continue;
				}

				// Move the atom to its nu parent's subatom list.
				QMultiHash<AuId, AuAtomSet*>::iterator osi = m_Subatoms.find(oldParId);
				if(osi != m_Subatoms.end())
					(*osi)->remove(atom);	// Remove it from the old list.
				else if(g_Au.m_DebugSwitches & 0x00040000)
				{
					// Thorough Change Parent Check
					// Search for the atom in any existing subatom list. This could indicate a problem.
					for(QMultiHash<AuId, AuAtomSet*>::const_iterator si = m_Subatoms.cbegin(); si != m_Subatoms.cend(); si++)
						if((*si)->remove(atom))
						{
							if(g_Au.m_LogMessages & LOG_Etherium)
							{
								g_Au.Log(QObject::tr("0x%1 %2 AuEther::ChangeParent atom 0x%3 %4 %5, %6 parent %7 not found. "
											"Exhaustive search of %8 subatom lists found it in parent %9's list. "
											"Removing atom.")
										.arg((size_t)this, 12,16,QChar('0'))
										.arg(m_Name)
										.arg((size_t)atom, 12,16,QChar('0'))
										.arg(ahi.key().GetID())
										.arg(atom->Name())
										.arg(atom->AtomTypeName())
										.arg(oldParId.GetID())
										.arg(m_Subatoms.size())
										.arg(si.key().GetID()));
							}
							else
								qWarning() << "warning:" << m_Name << "AuEther::ChangeParent atom" << *atom
									<< "parent not found" << oldParId << ". Exhaustive search of" << m_Subatoms.size()
									<< "subatom lists found atom in the list for parent" << si.key() << ". Removing atom.";
						}
				}

				QMultiHash<AuId, AuAtomSet*>::iterator nsi = m_Subatoms.find(pos.ParentId());
				if(nsi != m_Subatoms.end())
					(*nsi)->insert(atom);	// Add it to the nu list.

				// Unlock sequence is irrelevant.
				(*nli)->unlock();
				(*oli)->unlock();
			}
		}
		else if(nli != m_SubatomLocks.end())
		{
			// We only found the subatom lock for the nu parent.
			if(g_Au.m_DebugSwitches & 0x00040000)
			{
				// Thorough Change Parent Check
				// Search for the atom in any existing subatom list. This would indicate a problem.
				for(QHash<AuId, QReadWriteLock*>::iterator li = m_SubatomLocks.begin(); li != m_SubatomLocks.end(); li++)
				{
					QMultiHash<AuId, AuAtomSet*>::const_iterator si = m_Subatoms.find(li.key());
					if(si == m_Subatoms.end())
						continue;
					(*li)->lockForRead();
					bool hasIt = (*si)->contains(atom);
					(*li)->unlock();
					if(!hasIt)
						continue;

					// Found the atom in the subatom list of an existing atom.
					if(nli == li)
					{
						// The existing atom is in the subatom list of the atom's nu parent, right where it belongs. :/
						qWarning() << "warning:" << m_Name << "AuEther::ChangeParent atom" << chgId << "parent not found"
							<< oldParId << ". Exhaustive search of" << m_Subatoms.size()
							<< "subatom lists found atom in the list for parent" << si.key()
							<< ". Leaving it there because that IS its nu parent.";
						nuParentOk = true;
					}
					else
					{
						if(g_Au.m_LogMessages & LOG_Etherium)
						{
							g_Au.Log(QObject::tr("0x%1 %2 AuEther::ChangeParent atom 0x%3 %4 %5, %6 parent %7 not found. "
										"Exhaustive search of %8 subatom lists found it in parent %9's list. "
										"Removing atom.")
									.arg((size_t)this, 12,16,QChar('0'))
									.arg(m_Name)
									.arg((size_t)atom, 12,16,QChar('0'))
									.arg(ahi.key().GetID())
									.arg(atom->Name())
									.arg(atom->AtomTypeName())
									.arg(oldParId.GetID())
									.arg(m_Subatoms.size())
									.arg(si.key().GetID()));
						}
						else
						{
							qWarning() << "warning:" << m_Name << "AuEther::ChangeParent atom" << chgId << "parent not found"
								<< oldParId << ". Exhaustive search of" << m_Subatoms.size()
								<< "subatom lists found atom in the list for parent" << si.key() << ". Removing atom.";
						}
						(*li)->lockForWrite();
						(*si)->remove(atom);	// Remove it from the old list.
						(*li)->unlock();
					}
				}
				if(nuParentOk)
					continue;
			}

			// Add the atom to its nu parent's subatom list.
			(*nli)->lockForWrite();
			QMultiHash<AuId, AuAtomSet*>::iterator nsi = m_Subatoms.find(pos.ParentId());
			if(nsi != m_Subatoms.end())
				(*nsi)->insert(atom);
			(*nli)->unlock();
		}
		else
		{
			// We may have found the lock for the old parent but the lock for the nu parent was not found.
			// Sometimes on the client an atom arrives before its parent.
			AddRunaway(pos.ParentId(), atom->Id());
		}
		nuParentOk = true;
	}
	while(!nuParentOk);

	// Change parent and position info on the atom.
	if(orient)
		atom->SetPosAndOrientByEther(pos, this);
	else
		atom->SetPosByEther(pos, this);

	m_HashLock.unlock();
	atom->InvalidateEnvTransform();
}

bool AuEther::HasSubatoms(const AuId& atomId) const
{
	bool hasSubs = false;
	m_HashLock.lockForRead();
	QHash<AuId, QReadWriteLock*>::iterator li = m_SubatomLocks.find(atomId);
	if(li != m_SubatomLocks.end())
	{
		(*li)->lockForRead();
		QMultiHash<AuId, AuAtomSet*>::const_iterator si = m_Subatoms.find(atomId);
		if(si != m_Subatoms.cend())
			hasSubs = !(*si)->empty();
		(*li)->unlock();
	}
	m_HashLock.unlock();
	return hasSubs;
}

bool AuEther::HasRunaways(const AuId& parentId) const
{
	m_RunawayLock.lockForRead();
	QMultiHash<AuId, AuId>::const_iterator ri = m_Runaways.find(parentId);
	bool hasRunaways = ri != m_Runaways.cend();
	m_RunawayLock.unlock();
	return hasRunaways;
}

void AuEther::AddRunaway(const AuId& parentId, const AuId& atomId)
{
	bool hasRunaway = false;
	m_RunawayLock.lockForWrite();
	for(QMultiHash<AuId, AuId>::const_iterator ri = m_Runaways.find(parentId);
			ri != m_Runaways.cend() && ri.key() == parentId; ri++)
		if(*ri == atomId)
		{
			hasRunaway = true;	// That runaway is already in my list.
			break;
		}
	if(!hasRunaway)
		m_Runaways.insert(parentId, atomId);	// Add a nu runaway.
	m_RunawayLock.unlock();
}

void AuEther::FixRunaways(const AuId& parentId)
{
	if(!HasRunaways(parentId))
		return;

	// Take the list of ids to fix.
	QSet<AuId> fix;					// value = runaway atom id
	m_RunawayLock.lockForWrite();
	QMultiHash<AuId, AuId>::iterator ri = m_Runaways.find(parentId);
	while(ri != m_Runaways.end() && ri.key() == parentId)
	{
		fix.insert(*ri);
		ri = m_Runaways.erase(ri);
	}
	m_RunawayLock.unlock();

	for(QSet<AuId>::const_iterator fi = fix.cbegin(); fi != fix.cend(); fi++)
	{
		QHash<AuId, DaAtom*>::iterator ahi = m_AtomHash.find(*fi);
		if(ahi == m_AtomHash.end())
		{
			ahi = m_AtomSFHash.find(*fi);
			if(ahi == m_AtomSFHash.end())
				continue;
		}

		DaAtom* atom = *ahi;
		if(atom->ParentId() != parentId)
			continue;	// The atom no longer thinks that is its parent.

		// Move the atom to its nu parent's hash table.
		QHash<AuId, QReadWriteLock*>::iterator nli = m_SubatomLocks.find(parentId);		// New parent
		if(nli == m_SubatomLocks.end())
			continue;	// New parent subatoms not found?

		// We found the lock for the nu parent's subatoms.
		(*nli)->lockForWrite();

		// Add the atom to its nu parent's subatom list.
		QMultiHash<AuId, AuAtomSet*>::iterator nsi = m_Subatoms.find(parentId);
		if(nsi != m_Subatoms.end())
			(*nsi)->insert(atom);

		(*nli)->unlock();
	}
}

quint64 AuEther::MemoryUsage() const
{
	quint64 mem = sizeof(AuEther);
	mem += (sizeof(AuId) + sizeof(DaAtom*)) * (m_AtomHash.size() + m_AtomSFHash.size());
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// This block of functions manages the trash.

void AuEther::TrashAtom(const AuId& delId)
{
	// Remove the atom and all of its subatoms from my hash tables.
	DaAtomList del;
	RemoveAtom(delId, del);
	if(del.empty())
		return;

	if(g_Au.m_LogMessages & LOG_Etherium)
		g_Au.Log(QObject::tr("0x%1 %2 AuEther::TrashAtom id %3 trashing %4 atoms")
				.arg((size_t)this, 12,16,QChar('0'))
				.arg(m_Name)
				.arg(delId.GetID())
				.arg(del.size()));

	// Move atoms to the trash, for safe delete later.
	AuElapsedTimer now(AuElapsedTimer::Now());
	m_TrashLock.lock();
	for(DaAtomList::const_iterator di = del.cbegin(); di != del.cend(); di++)
	{
		if(g_Au.m_LogMessages & LOG_Etherium)
			g_Au.Log(QObject::tr("0x%1 %2 AuEther::TrashAtom  0x%3 id %4 %5 %6")
					.arg((size_t)this, 12,16,QChar('0'))
					.arg(m_Name)
					.arg((size_t)(*di), 12,16,QChar('0'))
					.arg((*di)->Id().GetID())
					.arg((*di)->Name())
					.arg((*di)->AtomTypeName()));
		(*di)->SetTrash();
		m_Trash.insert(*di, now);
	}
	m_TrashLock.unlock();
}

void AuEther::TrashAtoms(const AuIdList& delIds)
{
	for(AuIdList::const_iterator di = delIds.cbegin(); di != delIds.cend(); di++)
		TrashAtom(*di);
}

void AuEther::TrashAtoms(const DaAtomList& trashAtoms)
{
	if(g_Au.m_LogMessages & LOG_Etherium)
		g_Au.Log(QObject::tr("0x%1 %2 AuEther::TrashAtoms %3 atoms")
				.arg((size_t)this, 12,16,QChar('0'))
				.arg(m_Name)
				.arg(trashAtoms.size()));

	// Move atoms to the trash, for safe delete later.
	AuElapsedTimer now(AuElapsedTimer::Now());
	m_TrashLock.lock();
	for(DaAtomList::const_iterator di = trashAtoms.cbegin(); di != trashAtoms.cend(); di++)
	{
		if(g_Au.m_LogMessages & LOG_Etherium)
			g_Au.Log(QObject::tr("0x%1 %2 AuEther::TrashAtoms  0x%3 id %4 %5 %6")
					.arg((size_t)this, 12,16,QChar('0'))
					.arg(m_Name)
					.arg((size_t)(*di), 12,16,QChar('0'))
					.arg((*di)->Id().GetID())
					.arg((*di)->Name())
					.arg((*di)->AtomTypeName()));
		(*di)->SetTrash();
		m_Trash.insert(*di, now);
	}
	m_TrashLock.unlock();
}

size_t AuEther::DeleteTrashAtom()
{
	AuElapsedTimer now(AuElapsedTimer::Now());			// Current time for rot time checking.
	int rotSecs = 20;					// Seconds an atom remains in the trash before being deleted.

	// Find an atom that is ready for deletion.
	DaAtom* atom = 0;
	m_TrashLock.lock();
	for(QHash<DaAtom*, AuElapsedTimer>::iterator ti = m_Trash.begin(); ti != m_Trash.end(); ti++)
	{
		if((*ti).SecsTo(now) < rotSecs)
			continue;					// Atom has not been in the trash long enough. Let it rot a little.

		if(g_Au.InTickTockQueue(ti.key()->Id()))
		{
			// Reset the atom's rot timer so we don't delet it the instant the tick/tock threads are done with it.
			*ti = now;
			continue;
		}

		// The atom has rotted in the trash for a while and it is not being processed by a tick or tock thread. We can
		// probably delet it safely from the heap.
		atom = ti.key();
		m_Trash.erase(ti);
		break;							// Break out to delet the atom.
	}
	size_t trashAtoms = m_Trash.size();	// We will return the number of atoms remaining in the trash.
	m_TrashLock.unlock();

	if(atom)
	{
		// We pulled a rotten atom out of the trash.
		if(g_Au.m_LogMessages & LOG_Etherium)
			g_Au.Log(QObject::tr("0x%1 %2 AuEther::DeleteTrashAtom 0x%3 id %4 %5 %6")
					.arg((size_t)this, 12,16,QChar('0'))
					.arg(m_Name)
					.arg((size_t)atom, 12,16,QChar('0'))
					.arg(atom->Id().GetID())
					.arg(atom->Name())
					.arg(atom->AtomTypeName()));

		bool delAtom = true;
		if(g_Au.m_DebugSwitches & 0x00080000)
		{
			// Thorough Trash Check
			AuId atomBadId;
			AuId atomSubId;
			m_DeleteLock.lockForWrite();
			bool atomInEther = FindAtom(atom->Id()) == atom;
			if(!atomInEther)
			{
				// Look for the atom by pointer, not id.
				m_HashLock.lockForRead();
				// Search for the atom in any existing subatom list. This would indicate a problem.
				for(QHash<AuId, QReadWriteLock*>::iterator li = m_SubatomLocks.begin(); li != m_SubatomLocks.end(); li++)
				{
					QMultiHash<AuId, AuAtomSet*>::const_iterator si = m_Subatoms.find(li.key());
					if(si == m_Subatoms.end())
						continue;
					(*li)->lockForRead();
					bool hasIt = (*si)->contains(atom);
					(*li)->unlock();
					if(hasIt)
					{
						atomSubId = si.key();
						break;
					}
				}
				if(atomSubId.IsNull())
				{
					// No problem so far. Check to see if the atom's pointer is still in our atom list.
					if(atom->InStarField())
					{
						for(QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomSFHash.cbegin(); ahi != m_AtomSFHash.cend(); ahi++)
							if(atom == *ahi)
							{
								atomBadId = ahi.key();
								break;
							}
					}
					else
					{
						for(QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomHash.cbegin(); ahi != m_AtomHash.cend(); ahi++)
							if(atom == *ahi)
							{
								atomBadId = ahi.key();
								break;
							}
					}
				}
				m_HashLock.unlock();
			}
			m_DeleteLock.unlock();
			if(atomInEther)
			{
				// Must not delete an atom that is in the ether.
				delAtom = false;
				if(g_Au.m_LogMessages & LOG_Etherium)
					g_Au.Log(QObject::tr("0x%1 %2 AuEther::DeleteTrashAtom cannot delete 0x%3 id %4 %5 %6 still in the ether")
							.arg((size_t)this, 12,16,QChar('0'))
							.arg(m_Name)
							.arg((size_t)atom, 12,16,QChar('0'))
							.arg(atom->Id().GetID())
							.arg(atom->Name())
							.arg(atom->AtomTypeName()));
				else
					qWarning() << "error:" << m_Name << "AuEther::DeleteTrashAtom cannot delete" << *atom << "still in the ether"; 
			}
			else if(!atomSubId.IsNull())
			{
				// Must not delete an atom that is still in a subatom list.
				delAtom = false;
				if(g_Au.m_LogMessages & LOG_Etherium)
					g_Au.Log(QObject::tr("0x%1 %2 AuEther::DeleteTrashAtom cannot delete 0x%3 id %4 %5 %6, parent %7, "
								"still in the subatom list of id %8")
							.arg((size_t)this, 12,16,QChar('0'))
							.arg(m_Name)
							.arg((size_t)atom, 12,16,QChar('0'))
							.arg(atom->Id().GetID())
							.arg(atom->Name())
							.arg(atom->AtomTypeName())
							.arg(atom->ParentId().GetID())
							.arg(atomSubId.GetID()));
				else
					qWarning() << "error:" << m_Name << "AuEther::DeleteTrashAtom cannot delete" << *atom
						<< "parentId" << atom->ParentId() << "still in the subatom list of id" << atomSubId;
			}
			else if(!atomBadId.IsNull())
			{
				// Must not delete an atom that is still in the ether.
				delAtom = false;
				if(g_Au.m_LogMessages & LOG_Etherium)
					g_Au.Log(QObject::tr("0x%1 %2 AuEther::DeleteTrashAtom cannot delete 0x%3 id %4 %5 %6 still in the ether, id %7")
							.arg((size_t)this, 12,16,QChar('0'))
							.arg(m_Name)
							.arg((size_t)atom, 12,16,QChar('0'))
							.arg(atom->Id().GetID())
							.arg(atom->Name())
							.arg(atom->AtomTypeName())
							.arg(atomBadId.GetID()));
				else
					qWarning() << "error:" << m_Name << "AuEther::DeleteTrashAtom cannot delete" << *atom
						<< "still in the ether under id" << atomBadId; 
			}
		}
		if(delAtom)
		{
			// Unless someone puts our atom back into the ether right NOW, it is safe to delete it.
			// This is done after releasing the delete lock because memory deletion can take a long time, during which everyone else
			// would have to wait. We don't care how long the trash thread takes to delete an atom if it isn't sitting on a lock.
			delete atom;				// Delete the atom from the heap.
		}
	}
	return trashAtoms;
}

qint64 AuEther::TestLock(int timeout) const
{
	AuElapsedTimer time;
	time.start();
	if(m_HashLock.tryLockForWrite(timeout))	// Lock hash for write.
	{
		m_HashLock.unlock();		// Let it go as soon as we get it.
		return time.elapsed();		// Return how long it took to acquire the lock.
	}
	return -1;						// We failed to get the lock in the timeout period.
}

bool AuEther::TestTrashLock()
{
	if(m_TrashLock.tryLock(1000))		// Attempt to acquire the lock but don't wait forever.
	{
		m_TrashLock.unlock();			// Let it go as soon as I get it.
		return true;					// I am not deadlocked.
	}
	return false;						// I failed to get the lock in the timeout period.
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// This block of functions operates on atoms in the ether.

void AuEther::DeleteDeviceObjects(AuAtomRenderInfo& ari) const
{
	DaUniverse* uni = FindUniverse();
	if(uni)
		uni->DeleteDeviceObjects(ari);
}

void AuEther::EmpireDeleted(quint32 empireId) const
{
	m_HashLock.lockForRead();
	for(QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomHash.cbegin(); ahi != m_AtomHash.cend(); ahi++)
	{
		DaAtom* atom = *ahi;
		if(atom && atom->IsNormal())
			atom->EmpireDeleted(empireId);
	}
	// m_AtomSFHash atoms do not care if an empire is deleted.
	m_HashLock.unlock();
}

void AuEther::TickTock() const
{
	// Update the tick/tock queue with the current atoms.
	DaAtomList atoms;
	ListAtoms(atoms);
	for(DaAtomList::const_iterator ai = atoms.cbegin(); ai != atoms.cend(); ai++)
		(*ai)->TickTock();
}

void AuEther::TickOnClient(const AuElapsedTimer& last)
{
	DaUniverse* uni = FindUniverse();
	if(uni)
		uni->TickOnClient(last);
}

void AuEther::ResetAtomsToNormal()
{
	m_HashLock.lockForRead();
	for(QHash<AuId, DaAtom*>::iterator ahi = m_AtomHash.begin(); ahi != m_AtomHash.end(); ahi++)
		if(!(*ahi)->IsNormal())
		{
			(*ahi)->SetNormal();
			(*ahi)->SetModified();
		}
	// m_AtomSFHash atoms are not the target of this function.
	m_HashLock.unlock();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// This block of 'Find' and 'List' functions performs verious searches.

DaAtom* AuEther::FindAtom(const AuId& id) const
{
	DaAtom* atom = 0;
	m_HashLock.lockForRead();
	QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomHash.find(id);
	if(ahi != m_AtomHash.cend())
		atom = *ahi;
	else
	{
		ahi = m_AtomSFHash.find(id);
		if(ahi != m_AtomSFHash.cend())
			atom = *ahi;
	}
	m_HashLock.unlock();
	return atom;
}

DaAtom* AuEther::FindNormalAtom(const AuId& id) const
{
	DaAtom* atom = FindAtom(id);
	if(atom && atom->IsNormal())
		return atom;
	return 0;
}

DaAtom* AuEther::FindDormantAtom(const AuId& id) const
{
	DaAtom* atom = FindAtom(id);
	if(atom && atom->IsDormant())
		return atom;
	return 0;
}

DaAtom* AuEther::FindLiveAtom(const AuId& id) const
{
	DaAtom* atom = FindAtom(id);
	if(atom && !atom->IsDead())
		return atom;
	return 0;
}

void AuEther::FindNpcsFromSector(const AuId& sectorId, DaAtomList& officers) const
{
	m_HashLock.lockForRead();
	for(QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomHash.cbegin(); ahi != m_AtomHash.cend(); ahi++)
	{
		DaNpc* off = dynamic_cast<DaNpc*>(*ahi);
		if(off && off->IsNormal() && off->HomeSectorId() == sectorId)
			officers.append(off);
	}
	// m_AtomSFHash does not contain npcs.
	m_HashLock.unlock();
}

void AuEther::FindNpcsFromWorld(const AuId& worldId, DaAtomList& officers) const
{
	m_HashLock.lockForRead();
	for(QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomHash.cbegin(); ahi != m_AtomHash.cend(); ahi++)
	{
		DaNpc* off = dynamic_cast<DaNpc*>(*ahi);
		if(off && off->IsNormal() && off->HomeWorldId() == worldId)
			officers.append(off);
	}
	// m_AtomSFHash does not contain npcs.
	m_HashLock.unlock();
}

void AuEther::FindPeopleFromSystem(const AuId& systemId, DaAtomList& people) const
{
	m_HashLock.lockForRead();
	for(QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomHash.cbegin(); ahi != m_AtomHash.cend(); ahi++)
	{
		if(!(*ahi)->IsNormal())
			continue;
		DaPerson* person = dynamic_cast<DaPerson*>(*ahi);
		if(person && person->SpawnCitySystem() == systemId)
			people.append(person);
	}
	// m_AtomSFHash does not contain people.
	m_HashLock.unlock();
}

DaGalaxy* AuEther::FindGalaxy(GALAXY galaxy) const
{
	return dynamic_cast<DaGalaxy*>(FindAtom(AuId::GalaxyId(galaxy)));
}

DaUniverse* AuEther::FindUniverse() const
{
	return dynamic_cast<DaUniverse*>(FindAtom(AuId::Universe()));
}

void AuEther::ListSubatoms(const AuId& parentId, DaAtomList& subatoms) const
{
	m_HashLock.lockForRead();
	QHash<AuId, QReadWriteLock*>::iterator li = m_SubatomLocks.find(parentId);
	if(li != m_SubatomLocks.end())
	{
		(*li)->lockForRead();
		QMultiHash<AuId, AuAtomSet*>::const_iterator si = m_Subatoms.find(parentId);
		if(si != m_Subatoms.cend())
			subatoms.Append(*(*si));
		(*li)->unlock();
	}
	m_HashLock.unlock();
}

void AuEther::ListNormalSubatoms(const AuId& parentId, DaAtomList& subatoms) const
{
	m_HashLock.lockForRead();
	QHash<AuId, QReadWriteLock*>::iterator li = m_SubatomLocks.find(parentId);
	if(li != m_SubatomLocks.end())
	{
		(*li)->lockForRead();
		QMultiHash<AuId, AuAtomSet*>::const_iterator si = m_Subatoms.find(parentId);
		if(si != m_Subatoms.cend())
			subatoms.AppendNormal(*(*si));
		(*li)->unlock();
	}
	m_HashLock.unlock();
}

void AuEther::ListSubatomsRecursiveLocked(const AuId& parentId, DaAtomList& subs) const
{
	// Must lock my subatom list because ether is only locked for read.
	QHash<AuId, QReadWriteLock*>::iterator li = m_SubatomLocks.find(parentId);
	if(li == m_SubatomLocks.end())
		return;	// wut?

	// List my subatoms.
	AuIdList subIds;
	(*li)->lockForRead();
	QMultiHash<AuId, AuAtomSet*>::const_iterator si = m_Subatoms.find(parentId);
	if(si != m_Subatoms.cend())
	{
		subs.Append(*(*si));
		(*si)->ListIds(subIds);
	}
	(*li)->unlock();

	// Recurse to list the subatoms of subIds.
	for(AuIdList::const_iterator si = subIds.cbegin(); si != subIds.cend(); si++)
		ListSubatomsRecursiveLocked(*si, subs);
}

void AuEther::ListNormalSubatomsRecursiveLocked(const AuId& parentId, DaAtomList& subs) const
{
	// Must lock my subatom list because ether is only locked for read.
	QHash<AuId, QReadWriteLock*>::iterator li = m_SubatomLocks.find(parentId);
	if(li == m_SubatomLocks.end())
		return;	// wut?

	// List my subatoms.
	AuIdList subIds;
	(*li)->lockForRead();
	QMultiHash<AuId, AuAtomSet*>::const_iterator si = m_Subatoms.find(parentId);
	if(si != m_Subatoms.cend())
	{
		subs.AppendNormal(*(*si));
		(*si)->ListNormalIds(subIds);
	}
	(*li)->unlock();

	// Recurse to list the subatoms of subIds.
	for(AuIdList::const_iterator si = subIds.cbegin(); si != subIds.cend(); si++)
		ListNormalSubatomsRecursiveLocked(*si, subs);
}

void AuEther::ListSubatomsRecursiveNoInstanceLocked(const AuId& parentId, DaAtomList& subs) const
{
	// Must lock my subatom list because ether is only locked for read.
	QHash<AuId, QReadWriteLock*>::iterator li = m_SubatomLocks.find(parentId);
	if(li == m_SubatomLocks.end())
		return;	// wut?

	// List my subatoms.
	AuIdList subIds;
	(*li)->lockForRead();
	QMultiHash<AuId, AuAtomSet*>::const_iterator si = m_Subatoms.find(parentId);
	if(si != m_Subatoms.cend())
	{
		subs.Append(*(*si));
		(*si)->ListIdsNoInstance(subIds);
	}
	(*li)->unlock();

	// Recurse to list the subatoms of subIds.
	for(AuIdList::const_iterator si = subIds.cbegin(); si != subIds.cend(); si++)
		ListSubatomsRecursiveNoInstanceLocked(*si, subs);
}

void AuEther::ListNormalSubatomsRecursiveNoInstanceLocked(const AuId& parentId, DaAtomList& subs) const
{
	// Must lock my subatom list because ether is only locked for read.
	QHash<AuId, QReadWriteLock*>::iterator li = m_SubatomLocks.find(parentId);
	if(li == m_SubatomLocks.end())
		return;	// wut?

	// List my subatoms.
	AuIdList subIds;
	(*li)->lockForRead();
	QMultiHash<AuId, AuAtomSet*>::const_iterator si = m_Subatoms.find(parentId);
	if(si != m_Subatoms.cend())
	{
		subs.AppendNormal(*(*si));
		(*si)->ListNormalIdsNoInstance(subIds);
	}
	(*li)->unlock();

	// Recurse to list the subatoms of subIds.
	for(AuIdList::const_iterator si = subIds.cbegin(); si != subIds.cend(); si++)
		ListNormalSubatomsRecursiveNoInstanceLocked(*si, subs);
}

void AuEther::ListGalaxies(DaAtomList& galaxies) const
{
	DaUniverse* uni = FindUniverse();
	if(uni)
		uni->ListGalaxies(galaxies);
}

void AuEther::ListAtoms(DaAtomList& atoms) const
{
	m_HashLock.lockForRead();
	for(QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomHash.cbegin(); ahi != m_AtomHash.cend(); ahi++)
		atoms.append(*ahi);
	for(QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomSFHash.cbegin(); ahi != m_AtomSFHash.cend(); ahi++)
		atoms.append(*ahi);
	m_HashLock.unlock();
}

void AuEther::ListNormalAtoms(DaAtomList& atoms) const
{
	m_HashLock.lockForRead();
	for(QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomHash.cbegin(); ahi != m_AtomHash.cend(); ahi++)
		if((*ahi)->IsNormal())
			atoms.append(*ahi);
	for(QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomSFHash.cbegin(); ahi != m_AtomSFHash.cend(); ahi++)
		if((*ahi)->IsNormal())
			atoms.append(*ahi);
	m_HashLock.unlock();
}

void AuEther::ListNormalPeople(DaAtomList& people) const
{
	m_HashLock.lockForRead();
	for(QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomHash.cbegin(); ahi != m_AtomHash.cend(); ahi++)
		if((*ahi)->IsNormal() && dynamic_cast<DaPerson*>(*ahi))
			people.append(*ahi);
	// m_AtomSFHash does not contain people.
	m_HashLock.unlock();
}

void AuEther::ListNormalSolarSystems(DaAtomList& systems) const
{
	m_HashLock.lockForRead();
	// Solar systems are only in the star field.
	for(QHash<AuId, DaAtom*>::const_iterator ahi = m_AtomSFHash.cbegin(); ahi != m_AtomSFHash.cend(); ahi++)
		if((*ahi)->IsNormal() && dynamic_cast<DaSolarSystem*>(*ahi))
			systems.append(*ahi);
	m_HashLock.unlock();
}

void AuEther::ListSubatomsRecursive(const AuId& parentId, DaAtomList& subatoms) const
{
	m_HashLock.lockForRead();
	ListSubatomsRecursiveLocked(parentId, subatoms);
	m_HashLock.unlock();
}

void AuEther::ListNormalSubatomsRecursive(const AuId& parentId, DaAtomList& subatoms) const
{
	m_HashLock.lockForRead();
	ListNormalSubatomsRecursiveLocked(parentId, subatoms);
	m_HashLock.unlock();
}

void AuEther::ListSubatomsRecursiveNoInstance(const AuId& parentId, DaAtomList& subatoms) const
{
	m_HashLock.lockForRead();
	ListSubatomsRecursiveNoInstanceLocked(parentId, subatoms);
	m_HashLock.unlock();
}

void AuEther::ListNormalSubatomsRecursiveNoInstance(const AuId& parentId, DaAtomList& subatoms) const
{
	m_HashLock.lockForRead();
	ListNormalSubatomsRecursiveNoInstanceLocked(parentId, subatoms);
	m_HashLock.unlock();
}

void AuEther::ListOrphans(DaAtomList& orphans) const
{
	// N/a when there is no universe.
	if(!FindUniverse())
		return;

	// Report orphans that are not subordinate to the universe.
	DaAtomList atoms;
	ListAtoms(atoms);
	if(atoms.empty())
		return;	// I have no atoms.

	// Get a list of all the atoms that are subordinate to the universe.
	DaAtomList uatoms;
	ListSubatomsRecursive(AuId::Universe(), uatoms);

	// Fill the orphans list with atoms that are not in the universe list.
	for(DaAtomList::const_iterator ai = atoms.cbegin(); ai != atoms.cend(); ai++)
	{
		if((*ai)->Is(AuId::Universe()))
			continue;
		if(!uatoms.FindId((*ai)->Id()))
			orphans.append(*ai);			// Atom is not in the universe's hierarchy.
	}
}

// Performs decay on solo game after loading.
void AuEther::DecaySolo()
{
	DaUniverse* uni = FindUniverse();
	uni->DecaySolo();
}

