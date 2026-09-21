//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aureportwidget.cpp,v 1.17 2022/02/19 18:40:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aureportwidget.h"
//
#include "auhistopair.h"
#include "autablewidget.h"
#include <auplayerrec.h>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QSplitter>
#include <QTimer>
#include <QVBoxLayout>

AuReportWidget::AuReportWidget(QWidget* parent) : QWidget(parent)
{
	m_TMinus = 0;
	m_TMinusCountDown = 0;
	m_HistoPair = new AuHistoPair(&m_Status);
	connect(m_HistoPair, SIGNAL(SystemRightClicked(AuId)), SLOT(SystemRightClicked(AuId)));
	connect(m_HistoPair, SIGNAL(TMinus(int)), SLOT(SetTMinus(int)));
	connect(this, SIGNAL(TMinus(int)), m_HistoPair, SLOT(SetTMinus(int)));

	m_Clients = new AuTableWidget;
	{
		//m_Clients->setEditTriggers(QAbstractItemView::NoEditTriggers);
		//m_Clients->setSelectionBehavior(QAbstractItemView::SelectRows);
		//m_Clients->setSelectionMode(QAbstractItemView::SingleSelection);
		//m_Clients->setShowGrid(false);

		// Horizontal header for columns.
		QStringList hls;
		hls << tr("Player") << tr("Unit") << tr("Empire") << tr("Solar System") << tr("Connected") << tr("Bytes Tx")
			<< tr("Bytes Rx") << tr("Bad Packets") << tr("Syncs") << tr("Avg Atoms") << tr("Avg Bytes") << tr("Avg Ping")
			<< tr("Last Atoms") << tr("Last Bytes") << tr("Last Ping");
		m_Clients->setColumnCount(hls.count());
		m_Clients->setHorizontalHeaderLabels(hls);
		//m_Clients->horizontalHeader()->setResizeMode(0, QHeaderView::Interactive);
		//m_Clients->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);

		// Vertical header for rows hidden.
		QHeaderView* vhdr = m_Clients->verticalHeader();
		vhdr->hide();

		connect(m_Clients, SIGNAL(itemPressed(QTableWidgetItem*)), SLOT(ClientClicked(QTableWidgetItem*)));
	}

	// Assemble the histogram pair and clients list into a splitter.
	QSplitter* split = new QSplitter;
	split->setOrientation(Qt::Vertical);
	split->addWidget(m_HistoPair);
	split->addWidget(m_Clients);
	split->setStretchFactor(1, 1);

	QVBoxLayout* mainLay = new QVBoxLayout;
	{
		mainLay->addWidget(split);
		mainLay->setMargin(0);
		mainLay->setSpacing(2);
		setLayout(mainLay);
	}

	// Get an update timer started.
	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), SLOT(TimerUpdate()));
	timer->start(1000);
}

AuReportWidget::~AuReportWidget()
{
}

void AuReportWidget::TimerUpdate()
{
	// Update client connections status.
	// List clients that are currently in the table.
	// Must identify them by both player id and unit id.
	QList<quint32> playerIds;
	QList<quint32> unitIds;
	for(int row=0; row<m_Clients->rowCount(); row++)
	{
		playerIds.append(m_Clients->item(row,0)->data(Qt::UserRole).toInt());
		unitIds.append(m_Clients->item(row,1)->data(Qt::UserRole).toInt());
	}

	// Count down m_TMinus.
	if(m_TMinusCountDown)
	{
		m_TMinusCountDown--;
		if(!m_TMinusCountDown)
		{
			m_TMinus = 0;			// Time to reset m_TMinus.
			emit TMinus(0);
		}
	}

	// Add/update table items.
	int rpt = m_Status.count()-1 - m_TMinus;	// Report about the clients in the last status report.
	if(rpt >= 0)
	{
		for(QList<AuStatusClient>::const_iterator cit = m_Status[rpt].m_Clients.constBegin();
				cit != m_Status[rpt].m_Clients.constEnd(); cit++)
		{
			// Look for the existing entry in the table.
			bool entryExists = false;
			for(int row=0; row<m_Clients->rowCount(); row++)
			{
				if((quint32)m_Clients->item(row,0)->data(Qt::UserRole).toInt() == (*cit).m_PlayerId &&
						(quint32)m_Clients->item(row,1)->data(Qt::UserRole).toInt() == (*cit).m_UnitId)
				{
					// Update existing entry in the table.
					entryExists = true;

					QString unit = QString("%1 %2").arg((*cit).m_UnitId).arg((*cit).m_UnitName);
					m_Clients->item(row,1)->setText(unit);

					m_Clients->item(row,2)->setText(QVariant((*cit).m_UnitEmpire).toString());
					m_Clients->item(row,2)->setData(Qt::UserRole, (*cit).m_UnitEmpire);

					m_Clients->item(row,3)->setText(QVariant((*cit).m_SolarSystem).toString());
					m_Clients->item(row,3)->setData(Qt::UserRole, (*cit).m_SolarSystem);

					m_Clients->item(row,4)->setText((*cit).m_ConnectTime.toString());
					m_Clients->item(row,5)->setText(QVariant((*cit).m_BytesTx).toString());
					m_Clients->item(row,6)->setText(QVariant((*cit).m_BytesRx).toString());
					m_Clients->item(row,7)->setText(QVariant((*cit).m_BadPackets).toString());
					m_Clients->item(row,8)->setText(QVariant((*cit).m_SyncCount).toString());
					m_Clients->item(row,9)->setText(QVariant((*cit).m_SyncAvgAtoms).toString());
					m_Clients->item(row,10)->setText(QVariant((*cit).m_SyncAvgBytes).toString());
					m_Clients->item(row,11)->setText(QString("%1ms").arg((*cit).m_SyncAvgPing));
					m_Clients->item(row,12)->setText(QVariant((*cit).m_SyncLastAtoms).toString());
					m_Clients->item(row,13)->setText(QVariant((*cit).m_SyncLastBytes).toString());
					m_Clients->item(row,14)->setText(QString("%1ms").arg((*cit).m_SyncLastPing));

					// Remove the entry from the playerIds/unitIds lists so it won't be deleted when we're done.
					QList<quint32>::iterator pit = playerIds.begin();
					QList<quint32>::iterator uit = unitIds.begin();
					while(pit != playerIds.end() && uit != unitIds.end())
					{
						if(*pit == (*cit).m_PlayerId && *uit == (*cit).m_UnitId)
						{
							playerIds.erase(pit);
							unitIds.erase(uit);
							break;
						}
						pit++;
						uit++;
					}
				}
			}

			if(!entryExists)
			{
				// Add new entry to the table.
				int row = m_Clients->rowCount();
				m_Clients->insertRow(row);			// Add a row at the end.

				int col=0;
				QTableWidgetItem* twPlayer = new QTableWidgetItem(QIcon(AcctTypeIcon((*cit).m_AccountType)),
						QVariant((*cit).m_PlayerId).toString());
				twPlayer->setToolTip(AcctTypeName((*cit).m_AccountType));
				twPlayer->setData(Qt::UserRole, (*cit).m_PlayerId);
				m_Clients->setItem(row,col++, twPlayer);

				QString unit = QString("%1 %2").arg((*cit).m_UnitId).arg((*cit).m_UnitName);
				QTableWidgetItem* twUnit = new QTableWidgetItem(unit);
				twUnit->setData(Qt::UserRole, (*cit).m_UnitId);
				m_Clients->setItem(row,col++, twUnit);

				QTableWidgetItem* twEmpire = new QTableWidgetItem(QVariant((*cit).m_UnitEmpire).toString());
				twEmpire->setData(Qt::UserRole, (*cit).m_UnitEmpire);
				m_Clients->setItem(row,col++, twEmpire);

				QTableWidgetItem* twSystem = new QTableWidgetItem(QVariant((*cit).m_SolarSystem).toString());
				twSystem->setData(Qt::UserRole, (*cit).m_SolarSystem);
				m_Clients->setItem(row,col++, twSystem);

				QTableWidgetItem* twConnect = new QTableWidgetItem((*cit).m_ConnectTime.toString());
				m_Clients->setItem(row,col++, twConnect);

				QTableWidgetItem* twBytesTx = new QTableWidgetItem(QVariant((*cit).m_BytesTx).toString());
				m_Clients->setItem(row,col++, twBytesTx);

				QTableWidgetItem* twBytesRx = new QTableWidgetItem(QVariant((*cit).m_BytesRx).toString());
				m_Clients->setItem(row,col++, twBytesRx);

				QTableWidgetItem* twBadPackets = new QTableWidgetItem(QVariant((*cit).m_BadPackets).toString());
				m_Clients->setItem(row,col++, twBadPackets);

				QTableWidgetItem* twSyncCount = new QTableWidgetItem(QVariant((*cit).m_SyncCount).toString());
				m_Clients->setItem(row,col++, twSyncCount);

				QTableWidgetItem* twSyncAvgAtoms = new QTableWidgetItem(QVariant((*cit).m_SyncAvgAtoms).toString());
				m_Clients->setItem(row,col++, twSyncAvgAtoms);

				QTableWidgetItem* twSyncAvgBytes = new QTableWidgetItem(QVariant((*cit).m_SyncAvgBytes).toString());
				m_Clients->setItem(row,col++, twSyncAvgBytes);

				QTableWidgetItem* twSyncAvgPing = new QTableWidgetItem(QString("%1ms").arg((*cit).m_SyncAvgPing));
				m_Clients->setItem(row,col++, twSyncAvgPing);

				QTableWidgetItem* twSyncLastAtoms = new QTableWidgetItem(QVariant((*cit).m_SyncLastAtoms).toString());
				m_Clients->setItem(row,col++, twSyncLastAtoms);

				QTableWidgetItem* twSyncLastBytes = new QTableWidgetItem(QVariant((*cit).m_SyncLastBytes).toString());
				m_Clients->setItem(row,col++, twSyncLastBytes);

				QTableWidgetItem* twSyncLastPing = new QTableWidgetItem(QString("%1ms").arg((*cit).m_SyncLastPing));
				m_Clients->setItem(row,col++, twSyncLastPing);
			}
		}
	}

	// Remove table items that no longer belong there.
	// Anything left in the playerIds/unitIds lists must be removed.
	while(!playerIds.isEmpty() && !unitIds.isEmpty())
	{
		quint32 playerId = playerIds.takeFirst();
		quint32 unitId = unitIds.takeFirst();
		for(int row=0; row<m_Clients->rowCount(); row++)
		{
			if((quint32)m_Clients->item(row,0)->data(Qt::UserRole).toInt() == playerId &&
					(quint32)m_Clients->item(row,1)->data(Qt::UserRole).toInt() == unitId)
			{
				m_Clients->removeRow(row);
				break;
			}
		}
	}
}

void AuReportWidget::ClientClicked(QTableWidgetItem* item)
{
	if(m_Clients->LastButtonDown() != Qt::RightButton)
		return;

	// Show a pop-up menu of options.
	QMenu menu(this);
	menu.addAction("Kick Unit");
	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;

	int row = item->row();
	quint32 unitId = m_Clients->item(row,1)->data(Qt::UserRole).toInt();

	if(action->text() == "Kick Unit")
		emit KickUnit(m_ServerName, m_ServerPort, unitId);
}

void AuReportWidget::RestoreSettings(const QString& key, QSettings& settings)
{
	m_Clients->RestoreColumnPositions(key + "/Clients", settings);
}

void AuReportWidget::SaveSettings(const QString& key, QSettings& settings)
{
	m_Clients->SaveColumnPositions(key + "/Clients", settings);
}

void AuReportWidget::SetTMinus(int ms)
{
	m_TMinus = ms;
	m_TMinusCountDown = 2;	// seconds to reset m_TMinus to 0
}

void AuReportWidget::SystemRightClicked(AuId systemId)
{
	QString text = tr("%1\n%2").arg(systemId.GetID()).arg(systemId.StringId());
	QInputDialog::getMultiLineText(this, tr("System Id"), tr("Id of system."), text);
}
