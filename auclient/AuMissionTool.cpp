//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AuMissionTool.cpp,v 1.14 2022/05/04 19:39:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "AuMissionTool.h"
//
#include <auapp.h>
#include <auglobal.h>
#include <auframe.h>
#include "AuMissionDiskUi.h"
#include "AuMissionNameUi.h"
#include "aumissionorderui.h"
#include "autoolbutton.h"
#include <daperson.h>
#include <dasolarsystem.h>
#include <auview.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

void AuSpacecraftMissionDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftMissionUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionTool::AuMissionTool(QWidget* parent) : QWidget(parent)
{
	m_timer = 0;

	twmission = new QTableWidget;
	{
		twmission->setEditTriggers(QAbstractItemView::NoEditTriggers);
		twmission->setSelectionBehavior(QAbstractItemView::SelectRows);
		twmission->setSelectionMode(QAbstractItemView::SingleSelection);
		twmission->setShowGrid(false);
		twmission->setColumnCount(2);
		twmission->horizontalHeader()->setDefaultSectionSize(24);
		twmission->horizontalHeader()->setMinimumSectionSize(24);
		twmission->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
		twmission->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
		QHeaderView* hhdr = twmission->horizontalHeader();
		hhdr->hide();
		QHeaderView* vhdr = twmission->verticalHeader();
		vhdr->hide();
		twmission->setToolTip(tr("List of orders in the mission of the ship you are aboard. "
					"The first order in the mission is the <i>current</i> order.<br>"
					"Only crew members and people in the ship's chain-of-command may view the mission."));
		connect(twmission, SIGNAL(itemSelectionChanged()), SLOT(EnableControls()));
		connect(twmission, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), SLOT(MissionOrderDoubleClicked(QTableWidgetItem*)));
	}

	tborderfin = new AuIconButton(QIcon(":data/CommRecruit.png"));
	tborderfin->setToolTip(tr("<b>Mission Order Complete</b><br>"
				"Declares the current mission order to be complete. "
				"The first order in the mission is the <i>current</i> order. "
				"Anyone in the ship's chain-of-command may declare the current order complete. "
				"An AI-controlled officer in command of a ship will make that decision, "
				"executing each order in sequence."));
	connect(tborderfin, SIGNAL(clicked()), SLOT(MissionOrderComplete()));

	pmadd = new AuIconButton(QIcon(":data/Plus.png"));
	pmadd->setToolTip(tr("<b>Add Mission Order</b><br>"
				"Adds an order to the end of the mission.<br>"
				"Only a person in the ship's chain-of-command may add a mission order."));
	connect(pmadd, SIGNAL(clicked()), SLOT(MissionAdd()));

	pmadddisk = new AuIconButton(QIcon(":data/MissionDiskAdd.png"));
	pmadddisk->setToolTip(tr("<b>Add Mission</b><br>"
				"Adds a mission loaded from a mission disk to the end of the mission.<br>"
				"Only a person in the ship's chain-of-command may add a mission."));
	connect(pmadddisk, SIGNAL(clicked()), SLOT(MissionAddMission()));

	pmclear = new AuIconButton(QIcon(":data/ExitGame.png"));
	pmclear->setToolTip(tr("<b>Clear Mission</b><br>"
				"Clears all orders from the mission.<br>"
				"Only a person in the ship's chain-of-command may clear the mission."));
	connect(pmclear, SIGNAL(clicked()), SLOT(MissionClear()));

	pmdelete = new AuIconButton(QIcon(":data/Minus.png"));
	pmdelete->setToolTip(tr("<b>Delete Mission Order</b><br>"
				"Deletes the selected order from the mission.<br>"
				"Only a person in the ship's chain-of-command may delete a mission order."));
	connect(pmdelete, SIGNAL(clicked()), SLOT(MissionDelete()));

	pminsert = new AuIconButton(QIcon(":data/I.png"));
	pminsert->setToolTip(tr("<b>Insert Mission Order</b><br>"
				"Inserts an order before the selected order in the mission.<br>"
				"Only a person in the ship's chain-of-command may insert a mission order."));
	connect(pminsert, SIGNAL(clicked()), SLOT(MissionInsert()));

	pmprop = new AuIconButton(QIcon(":data/Properties.png"));
	pmprop->setToolTip(tr("<b>Change Mission Order</b><br>"
				"Changes the mission order that is selected.<br>"
				"Only a person in the ship's chain-of-command may change a mission order."));
	connect(pmprop, SIGNAL(clicked()), SLOT(MissionOrderProperties()));

	pmnew = new AuIconButton(":data/NarrateChosen.png");
	connect(pmnew, SIGNAL(clicked()), SLOT(MissionNew()));
	pmnew->setToolTip(tr("<b>New Mission</b><br>"
				"Replaces the mission with a single order.<br>"
				"Only a person in the ship's chain-of-command may start a new mission."));

	tbburn = new AuIconButton(QIcon(":data/c_MissionDisk.png"));
	tbburn->setToolTip(tr("<b>Burn Mission Disk</b><br>"
				"Stores the mission onto a blank disk. Your inventory must contain a blank disk.<br>"
				"Only crew members or people in the ship's chain-of-command may store the mission onto a disk."));
	connect(tbburn, SIGNAL(clicked()), SLOT(MissionBurnCD()));

	tbload = new AuIconButton(QIcon(":data/dp_Load.png"));
	tbload->setToolTip(tr("<b>Load Mission</b><br>"
				"Loads the mission from a mission disk in your inventory.<br>"
				"Only people in the ship's chain-of-command may load the mission from a disk."));
	connect(tbload, SIGNAL(clicked()), SLOT(MissionLoad()));

	pbactivate = new AuIconButton(":data/mpReady.png");
	connect(pbactivate, SIGNAL(clicked()), SLOT(MissionActivate()));
	pbactivate->setToolTip(tr("<b>Activate Mission</b><br>"
				"Activates the mission. Officers and crew will execute the orders of the mission.<br>"
				"Only a person in the ship's chain-of-command may activate the mission."));

	pbdeactivate = new AuIconButton(":data/mpNotReady.png");
	connect(pbdeactivate, SIGNAL(clicked()), SLOT(MissionDeactivate()));
	pbdeactivate->setToolTip(tr("<b>Hold Mission</b><br>"
				"Deactivates the mission. "
				"Orders will not be executed until the mission is activated.<br>"
				"Only a person in the ship's chain-of-command may deactivate the mission."));

	pbcycle = new AuIconButton(":data/Cycle.png");
	connect(pbcycle, SIGNAL(clicked()), SLOT(MissionCycle()));
	pbcycle->setToolTip(tr("<b>Cycle Mission</b><br>"
				"Sets the mission to cycle. "
				"Mission orders are rotated to the end of the mission as they are completed.<br>"
				"Only a person in the ship's chain-of-command may change the mission."));

	pbnocycle = new AuIconButton(":data/DoOnce.png");
	connect(pbnocycle, SIGNAL(clicked()), SLOT(MissionNoCycle()));
	pbnocycle->setToolTip(tr("<b>Do Mission Once</b><br>"
				"Sets the mission to execute only once. "
				"Mission orders are removed from the mission as they are completed.<br>"
				"Only a person in the ship's chain-of-command may change the mission."));

	pbsensors = new AuIconButton(":data/SensorSweep0.png");
	connect(pbsensors, SIGNAL(clicked()), SLOT(RequestSensorContacts()));
	pbsensors->setToolTip(tr("<b>Sensor Contacts</b><br>"
				"Requests a snapshot of sensor scan contacts from the spacecraft you are aboard. "
				"Sensor contacts are useful for issuing certain orders, such as <i>follow</i> and <i>attack</i> orders.<p>"
				"Sensor contacts expire after three minutes. "
				"The icon on this button shows activity while sensor contacts are available."));

	// Create a layout for the row of mission button.
	QHBoxLayout* mlay = new QHBoxLayout;
	mlay->addWidget(tborderfin);
	mlay->addWidget(pbactivate);
	mlay->addWidget(pbdeactivate);
	mlay->addWidget(pbcycle);
	mlay->addWidget(pbnocycle);
	mlay->addStretch(1);
	mlay->addWidget(pmnew);
	mlay->addWidget(pmadddisk);
	mlay->addWidget(pmadd);
	mlay->addWidget(pminsert);
	mlay->addWidget(pmdelete);
	mlay->addWidget(pmprop);
	mlay->addWidget(pmclear);
	mlay->addWidget(tbload);
	mlay->addStretch(2);
	mlay->addWidget(tbburn);
	mlay->addWidget(pbsensors);
	mlay->setMargin(2);
	mlay->setSpacing(0);

	// Final main grid.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(mlay);
	lay->addWidget(twmission);
	lay->setMargin(2);
	lay->setSpacing(0);
	setLayout(lay);
}

AuMissionTool::~AuMissionTool()
{
}

void AuMissionTool::EnableControls()
{
	// Determine if any of our lists have selections.
	bool selMission = false;
	QItemSelectionModel* sel = twmission->selectionModel();
	if(sel && sel->hasSelection())
		selMission = true;

	// Determine if the person is in the chain-of-command of the ship.
	AuView* pView = auApp->Frame()->View();
	bool inCommand = pView->IsPersonInCommand();

	// Enable/disable controls.
	pbactivate->setEnabled(inCommand);
	pbdeactivate->setEnabled(inCommand);
	pbcycle->setEnabled(inCommand);
	pbnocycle->setEnabled(inCommand);
	pbsensors->setEnabled(inCommand);
	pmnew->setEnabled(inCommand);
	pmadd->setEnabled(inCommand);
	pmadddisk->setEnabled(inCommand);
	pmclear->setEnabled(inCommand);
	pmdelete->setEnabled(selMission && inCommand);
	pminsert->setEnabled(selMission && inCommand);
	pmprop->setEnabled(selMission && inCommand);
	tbburn->setEnabled(inCommand);
	tbload->setEnabled(inCommand);
	tborderfin->setEnabled(inCommand);
}

void AuMissionTool::MissionNew()
{
	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();

	QString worldName, slonglat, scity, sdev, shipname, shipinfo, sysNameDesc, secDesc, secName, oat, wx, wind;
	QString sysPos, secPos, galPos, galName;
	AuView* pView = pFrame->View();
	INDUSTRYID iid;
	SPACECRAFT_STRUCTURE shipStruct;
	BODY_TYPE bodyType;
	pView->GetLocationInfo(worldName, bodyType, slonglat, scity, sdev, iid, shipname, shipinfo, shipStruct, sysNameDesc, secDesc,
			secName, oat, wx, wind, sysPos, secPos, galPos, galName);

	AuMissionOrderUi modlg(view->SceneAtomId(), QObject::tr("<b>New Mission</b>"), AuId(0), m_SensorBlips, AuPoint(galPos),
			AuPoint(secPos), AuPoint(sysPos), 0, pFrame);
	if(pFrame->ExecDialog(modlg) != QDialog::Accepted)
		return;

	AuMissionOrder* order = modlg.Order();
	if(!order)
		return;
	if(!order->IsValid())
	{
		delete order;
		return;
	}

	auApp->MissionNew(*order);
	delete order;
}

void AuMissionTool::MissionAdd()
{
	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();

	QString worldName, slonglat, scity, sdev, shipname, shipinfo, sysNameDesc, secDesc, secName, oat, wx, wind;
	QString sysPos, secPos, galPos, galName;
	AuView* pView = pFrame->View();
	INDUSTRYID iid;
	SPACECRAFT_STRUCTURE shipStruct;
	BODY_TYPE bodyType;
	pView->GetLocationInfo(worldName, bodyType, slonglat, scity, sdev, iid, shipname, shipinfo, shipStruct, sysNameDesc, secDesc,
			secName, oat, wx, wind, sysPos, secPos, galPos, galName);

	// Determine system relative to last mission order.
	AuId systemId = m_Mission.SystemIdFinal();	// If NULL, the dialog will use the current system, which is appropriate.
	AuMissionOrderUi modlg(view->SceneAtomId(), QObject::tr("<b>Add Mission Order</b>"), systemId, m_SensorBlips, AuPoint(galPos),
			AuPoint(secPos), AuPoint(sysPos), 0, pFrame);
	if(pFrame->ExecDialog(modlg) != QDialog::Accepted)
		return;

	AuMissionOrder* order = modlg.Order();
	if(!order)
		return;
	if(!order->IsValid())
	{
		delete order;
		return;
	}

	// Send the mission add command request to the server.
	auApp->MissionAppendOrder(*order);
	delete order;
}

void AuMissionTool::MissionAddMission()
{
	// Query for the mission disk CD.
	AuFrame* pFrame = auApp->Frame();
	AuMissionDiskUi dlg(false, false, pFrame);	// Append mission.
	int result = pFrame->ExecDialog(dlg);
	if(result == QDialog::Accepted)
		auApp->MissionAppendMission(dlg.Mission());
}

void AuMissionTool::MissionInsert()
{
	QItemSelectionModel* ism = twmission->selectionModel();
	if(!ism || !ism->hasSelection())
		return;
	QModelIndexList sel = ism->selectedIndexes();
	if(sel.isEmpty())
		return;
	QTableWidgetItem* item = twmission->item(sel.first().row(), 0);
	if(!item)
		return;
	quint8 orderId = item->data(Qt::UserRole).toUInt();

	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();

	QString worldName, slonglat, scity, sdev, shipname, shipinfo, sysNameDesc, secDesc, secName, oat, wx, wind;
	QString sysPos, secPos, galPos, galName;
	AuView* pView = pFrame->View();
	INDUSTRYID iid;
	SPACECRAFT_STRUCTURE shipStruct;
	BODY_TYPE bodyType;
	pView->GetLocationInfo(worldName, bodyType, slonglat, scity, sdev, iid, shipname, shipinfo, shipStruct, sysNameDesc, secDesc,
			secName, oat, wx, wind, sysPos, secPos, galPos, galName);

	// Determine system relative to selected mission order.
	AuId systemId = m_Mission.SystemIdBefore(orderId);	// If NULL, the dialog will use the current system, which is appropriate.
	AuMissionOrderUi modlg(view->SceneAtomId(), QObject::tr("<b>Insert Mission Order</b>"), systemId, m_SensorBlips,
			AuPoint(galPos), AuPoint(secPos), AuPoint(sysPos), 0, pFrame);
	if(pFrame->ExecDialog(modlg) != QDialog::Accepted)
		return;

	AuMissionOrder* order = modlg.Order();
	if(!order)
		return;
	if(!order->IsValid())
	{
		delete order;
		return;
	}

	auApp->MissionInsertOrder(orderId, *order);
	delete order;
}

void AuMissionTool::MissionClear()
{
	QMessageBox mb(g_Au.ApplicationName(), tr("The mission will be completely erased."),
		QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	if(auApp->Frame()->ExecMessageBox(mb) != QMessageBox::Ok)
		return;
	auApp->MissionClear();
}

void AuMissionTool::MissionDelete()
{
	QItemSelectionModel* ism = twmission->selectionModel();
	if(!ism || !ism->hasSelection())
		return;
	QModelIndexList sel = ism->selectedIndexes();
	if(sel.isEmpty())
		return;
	QTableWidgetItem* item = twmission->item(sel.first().row(), 0);
	if(!item)
		return;
	quint8 orderId = item->data(Qt::UserRole).toUInt();
	auApp->MissionOrderDelete(orderId);
}

void AuMissionTool::MissionActivate()
{
	auApp->MissionSetActive(true);
}

void AuMissionTool::MissionDeactivate()
{
	auApp->MissionSetActive(false);
}

void AuMissionTool::MissionCycle()
{
	auApp->MissionSetCycle(true);
}

void AuMissionTool::MissionNoCycle()
{
	auApp->MissionSetCycle(false);
}

void AuMissionTool::MissionOrderComplete()
{
	auApp->MissionOrderComplete();
}

void AuMissionTool::MissionBurnCD()
{
	// Query for the mission name to burn on the CD.
	AuFrame* pFrame = auApp->Frame();
	AuMissionNameUi dlg(pFrame);
	int result = pFrame->ExecDialog(dlg);
	if(result == QDialog::Accepted)
		auApp->MissionBurnCD(dlg.Name(), dlg.MediaPos());
}

void AuMissionTool::MissionLoad()
{
	// Query for the mission disk CD.
	AuFrame* pFrame = auApp->Frame();
	AuMissionDiskUi dlg(true, false, pFrame);	// Replace mission
	int result = pFrame->ExecDialog(dlg);
	if(result == QDialog::Accepted)
		auApp->MissionLoad(dlg.Mission());
}

QSize AuMissionTool::sizeHint() const
{
	return QSize(442,230);
}

void AuMissionTool::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(1000);
}

void AuMissionTool::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	if(m_SensorBlipTime.isValid())
	{
		//qDebug() << "AuMissionTool::UpdateContents elapsed" << m_SensorBlipTime.elapsed();
		// Sensor contacts are discarded when they are too old.
		if(m_SensorBlipTime.elapsed()/d_MSecPerMin >= 3)
		{
			m_SensorBlips.Clear();
			m_SensorBlipTime = AuElapsedTimer();
			pbsensors->setIcon(QIcon(":data/SensorSweep0.png"));
		}
		else
		{
			int icon = (m_SensorBlipTime.elapsed() / d_MSecPerSec) % 3 + 1;
			pbsensors->setIcon(QIcon(QString(":data/SensorSweep%1.png").arg(icon)));
		}
	}

	// Create list view entries from spaceship atom data.
	AuId systemId;	// Current solar system id.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		// Mission orders can be seen only by crew members and people in the ship's chain-of-command.
		const DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && (pView->IsPersonInCommand() || ship->IsCrewOrTroop(person->Id())))
		{
			const DaAtom* system = ship->FindEnvironmentAtom();
			if(system)
				systemId = system->Id();

			// Show mission information.
			AuMission mission = ship->LockMission();
			ship->UnlockMission();

			if(m_Mission != mission)
			{
				m_Mission = mission;

				pbactivate->setDown(mission.IsActive());
				pbdeactivate->setDown(!mission.IsActive());

				pbcycle->setDown((mission.Type() == AuMission::Single)?false:true);
				pbnocycle->setDown((mission.Type() == AuMission::Single)?true:false);

				// List mission orders.
				twmission->blockSignals(true);
				m_Mission.AddTableWidgetEntries(twmission);
				twmission->resizeRowsToContents();
				twmission->blockSignals(false);
			}
		}
		else
		{
			// Not aboard a spacecraft.
			if(!m_Mission.IsEmpty())
			{
				m_Mission.Clear();
				twmission->setRowCount(0);
			}
		}
	}
	pView->UnlockSceneAtom();
	EnableControls();

	// Track our current solar system.
	if(!systemId.IsNull() && systemId != m_SystemId)
		m_SystemId = systemId;									// Current solar system changed.
}

void AuMissionTool::RequestSensorContacts()
{
	auApp->MissionRequestSensorContacts();
}

void AuMissionTool::SensorContactsReceived(QByteArray bablips)
{
	// Extract the sensor blips from the byte array.
	QDataStream in(bablips);
	in.setVersion(QDataStream::Qt_4_1);
	in >> m_SensorBlips;
	m_SensorBlipTime.restart();
	//qDebug() << "AuMissionTool::SensorContactsReceived" << m_SensorBlips.count();
}

void AuMissionTool::MissionOrderDoubleClicked(QTableWidgetItem* /*orderItem*/)
{
	MissionOrderProperties();
}

void AuMissionTool::MissionOrderProperties()
{
	QItemSelectionModel* ism = twmission->selectionModel();
	if(!ism || !ism->hasSelection())
		return;
	QModelIndexList sel = ism->selectedIndexes();
	if(sel.isEmpty())
		return;
	QTableWidgetItem* item = twmission->item(sel.first().row(), 0);
	if(!item)
		return;
	quint8 orderId = item->data(Qt::UserRole).toUInt();
	const AuMissionOrder* curOrder = m_Mission.FindOrder(orderId);
	if(!curOrder)
		return;

	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();

	QString worldName, slonglat, scity, sdev, shipname, shipinfo, sysNameDesc, secDesc, secName, oat, wx, wind;
	QString sysPos, secPos, galPos, galName;
	AuView* pView = pFrame->View();
	INDUSTRYID iid;
	SPACECRAFT_STRUCTURE shipStruct;
	BODY_TYPE bodyType;
	pView->GetLocationInfo(worldName, bodyType, slonglat, scity, sdev, iid, shipname, shipinfo, shipStruct, sysNameDesc, secDesc,
			secName, oat, wx, wind, sysPos, secPos, galPos, galName);

	// Determine system relative to selected mission order.
	AuId systemId = m_Mission.SystemIdBefore(orderId);	// If NULL, the dialog will use the current system, which is appropriate.
	AuMissionOrderUi modlg(view->SceneAtomId(), QObject::tr("<b>Change Mission Order</b>"), systemId, m_SensorBlips,
			AuPoint(galPos), AuPoint(secPos), AuPoint(sysPos), curOrder, pFrame);
	if(pFrame->ExecDialog(modlg) != QDialog::Accepted)
		return;

	AuMissionOrder* order = modlg.Order();
	if(!order)
		return;
	if(!order->IsValid())
	{
		delete order;
		return;
	}

	auApp->MissionOrderReplace(orderId, *order);
	delete order;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionDialog::AuMissionDialog(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Spacecraft Mission"));
	setAuDlgIcon(QIcon(":data/OrderTakeCommand.png"));

	m_Mission = new AuMissionTool(this);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_Mission);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuMissionDialog::~AuMissionDialog()
{
}

void AuMissionDialog::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		QTimer::singleShot(0, [=](){RestoreGeometry();});
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuMissionDialog::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/MissionDialog/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/MissionDialog/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/MissionDialog/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/MissionDialog/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuMissionDialog::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/MissionDialog/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/MissionDialog/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/MissionDialog/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/MissionDialog/SizeY",sz.height());
}
