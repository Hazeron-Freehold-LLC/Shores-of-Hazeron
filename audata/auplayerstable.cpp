//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auplayerstable.cpp,v 1.25 2022/03/14 17:56:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auplayerstable.h"
//
#include "auglobal.h"
#include "dgdata.h"
#include <QHeaderView>
#include <QTableWidget>

AuPlayersTable::AuPlayersTable(bool empiresAvailable, QWidget* parent) : QTableWidget(0,4, parent)
{
	m_AvatarsOnline = 0;
	m_EmpiresAvailable = empiresAvailable;
	m_IsArchitect = false;
	m_UpdateResult = 0;

	verticalHeader()->hide();
	verticalHeader()->setDefaultSectionSize(18);
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);

	QStringList headerLabels;
	//headerLabels << tr("👤");		fail
	//headerLabels << tr("👥");		fail
	//headerLabels << tr("🚀");		fail
	//headerLabels << tr("😀");
	//headerLabels << tr("☺");
	headerLabels << tr("🕹");
	headerLabels << tr("🏳️");
	headerLabels << tr("⚙");
	headerLabels << tr("💻");
	setHorizontalHeaderLabels(headerLabels);

	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setShowGrid(false);
	setSortingEnabled(true);
	sortByColumn(0, Qt::AscendingOrder);
}

void AuPlayersTable::mousePressEvent(QMouseEvent* event)
{
	QTableWidget::mousePressEvent(event);
	m_MousePressItem = itemAt(event->pos());
}

void AuPlayersTable::mouseReleaseEvent(QMouseEvent* event)
{
	QTableWidget::mouseReleaseEvent(event);

	if(event->button() == Qt::RightButton)
	{
		QTableWidgetItem* item = itemAt(event->pos());
		if(item && item == m_MousePressItem)
			emit ItemRightClicked(item);
	}
}

int AuPlayersTable::Update(const AuPlayerOnlineStatusVector& players, bool isArchitect)
{
	// Refresh the contents of the table widget.
	if(m_IsArchitect == isArchitect && m_UpdateResult != 2 && m_Contents == players)
	{
		m_UpdateResult = 0;
		return 0;
	}
	m_Contents = players;
	m_IsArchitect = isArchitect;
	clearContents();
	while(rowCount())
		removeRow(0);

	m_UpdateResult = 1;
	m_AvatarsOnline = 0;
	setSortingEnabled(false);
	for(AuPlayerOnlineStatusVector::const_iterator posi = players.constBegin(); posi != players.constEnd(); posi++)
	{
		if(!isArchitect && (*posi).IsIncognito() && (*posi).m_Id == AuId::Haxus())
			continue;	// Skip Haxus when incognito, except to architects.

		// The citizen is not in the table so insert a new record.
		m_AvatarsOnline++;
		insertRow(0);

		// Empire
		QString empireName;
		QString empireTip;
		QIcon flagIcon;
		QString nameWithTitle;
		if((*posi).m_EmpireId)
		{
			if(m_EmpiresAvailable)
			{
				QImage empireFlag;
				const DgData& gd = g_Au.LockDataForRead();
				bool hasEmpire = gd.EmpireName((*posi).m_EmpireId, empireName);
				gd.EmpireFlag((*posi).m_EmpireId, empireFlag);
				nameWithTitle = gd.EmpireCitizenNameWithTitle((*posi).m_EmpireId, (*posi).m_Id);
				g_Au.UnlockData();
				if(!hasEmpire)
				{
					emit EmpireRequested((*posi).m_EmpireId);	// Request the empire record from the server.
					m_UpdateResult = 2;							// Empire requested
				}

				if(!empireFlag.isNull())
					flagIcon = QIcon(QPixmap::fromImage(empireFlag));

				if(isArchitect && hasEmpire)
					empireTip = QString("%2 <i>%1</i>").arg((*posi).m_EmpireId).arg(empireName);
				else
					empireTip = empireName;
			}
			else
			{
				empireName = QString("%1").arg((*posi).m_EmpireId);
				empireTip = empireName;
			}
		}

		QIcon plico;
		if((*posi).IsAFK())
			plico = QIcon(":data/ComputerEye.png");
		else if((*posi).IsAgent() && !(*posi).IsIncognito())
			plico = QIcon(":data/SoHLogo32.png");
		else if((*posi).IsChatClient())
			plico = QIcon(":data/Microphone.png");
		else if((*posi).IsNewEmperor())
			plico = QIcon(":data/SurrenderFlag.png");
		else
			plico = QIcon(":data/c_VacSuitHelmet.png");

		// Player Name
		int col=0;
		QTableWidgetItem* twname = new QTableWidgetItem(plico, (*posi).m_Name);
		twname->setData(Qt::UserRole, (*posi).m_Id.GetID());
		if(!nameWithTitle.isEmpty())
			twname->setToolTip(QString("%1 <i>%2</i>").arg(nameWithTitle).arg((*posi).m_Id.StringId()));
		setItem(0,col++, twname);

		QTableWidgetItem* twempire = new QTableWidgetItem(flagIcon, empireName);
		twempire->setData(Qt::UserRole, (*posi).m_EmpireId);
		twempire->setToolTip(empireTip);
		setItem(0,col++, twempire);

		// Status
		QString status;
		QString statusTip;
		if((*posi).IsNewEmperor())
		{
			status += tr("🗡️");
			statusTip = tr("New Emperor");
		}
		else if((*posi).IsAgentPowerEnabled())
		{
			status += tr("😎");
			//status += tr("🧙");
			//status += tr("🦸");
			//status += tr("🤵");
			statusTip = tr("Agent Powers Enabled");
		}
		else
		{
			//status += tr("🙂");
			//status += tr("🧑");
			//status += tr("😀");
			status += tr("☺");
			statusTip = tr("Player");
		}
		if(isArchitect)
		{
			// Architect gets to see the status of other agents' bits.
			if((*posi).IsInvisible())
			{
				status += tr("👻");
				statusTip += ", Invisible";
			}
			if((*posi).IsInvincible())
			{
				status += tr("🛡️");
				statusTip += ", Invincible";
			}
			if((*posi).IsAnimal())
			{
				status += tr("🦊");
				statusTip += ", Animal";
			}
			if((*posi).IsEntity())
			{
				//status += tr("🎆");
				//status += tr("✴");
				//status += tr("❇");
				status += tr("💥");
				statusTip += ", Entity";
			}
			if((*posi).IsIncognito())
			{
				status += tr("🕵️");
				statusTip += ", Incognito";
			}
		}

		QTableWidgetItem* twstat = new QTableWidgetItem(status);
		twstat->setData(Qt::UserRole, (*posi).m_StatusBits);
		twstat->setToolTip(statusTip);
		setItem(0,col++, twstat);

		// OS
		QString os;
		if((*posi).WordSize() == 64)
			os = QString("%1 %2")
				.arg((*posi).OsTypeName())
				.arg((*posi).OsVersionName())
				.simplified();
		else
			os = QString("%1 %2 ×%3")
				.arg((*posi).OsTypeName())
				.arg((*posi).OsVersionName())
				.arg((*posi).WordSize())
				.simplified();
		QIcon osico((*posi).OsVersionIcon());
		QTableWidgetItem* twos = new QTableWidgetItem(osico, os);
		twos->setToolTip(os);
		setItem(0,col++, twos);
	}
	setSortingEnabled(true);
	//qDebug() << "AuPlayersTable::Update result" << m_UpdateResult;
	return m_UpdateResult;
}

