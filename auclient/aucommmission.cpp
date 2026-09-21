//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommmission.cpp,v 1.12 2022/04/15 17:01:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommmission.h"
//
#include "AuMissionDiskUi.h"
#include "auapp.h"
#include "aucommchannel.h"
#include "auglobal.h"
#include "auframe.h"
#include "aumissionorderwidget.h"
#include "ausensorblip.h"
#include "autoolbutton.h"
#include "auview.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTimer>

AuCommMission::AuCommMission(const AuId& authorization, const QString& toName, const AuId& toId, const AuId& toSystemId,
		AuCommChannel* parent) : AuDialog(parent)
{
	m_timer = 0;

	// Get info about the scene atom.
	AuView* view = auApp->Frame()->View();
	QString worldName, slonglat, scity, sdev, shipname, shipinfo, sysNameDesc, secDesc, secName, oat, wx, wind;
	QString sysPos, secPos, galPos, galName;
	INDUSTRYID iid;
	SPACECRAFT_STRUCTURE shipStruct;
	BODY_TYPE bodyType;
	view->GetLocationInfo(worldName, bodyType, slonglat, scity, sdev, iid, shipname, shipinfo, shipStruct, sysNameDesc, secDesc,
			secName, oat, wx, wind, sysPos, secPos, galPos, galName);

	m_PersonEmpire = view->SceneAtomEmpire();

	QString recipient;
	QString title;
	if(toId.IsNull() || toName.isEmpty())
	{
		title = QObject::tr("<b>Mission Transmitter</b>");
		recipient = tr("recipient");
	}
	else
	{
		m_ToId = toId;
		title = QObject::tr("<b>Mission Transmitter to %1</b>").arg(toName);
		recipient = toName;
	}
	setAuDlgIcon(QIcon(":data/GovSpacecraft.png"));
	setAuDlgTitle(title);
	
	AuIconButton* pbshow = new AuIconButton(":data/MissionRefresh.png");
	connect(pbshow, SIGNAL(clicked()), SLOT(ShowMission()));
	pbshow->setToolTip(tr("<b>Report Mission</b><br>"
				"Requests %1 to report their mission. "
				"%1 calls out their mission orders on the comm channel.").arg(recipient));

	AuIconButton* pbsend = new AuIconButton(":data/c_MissionDisk.png");
	connect(pbsend, SIGNAL(clicked()), SLOT(SendMission()));
	pbsend->setToolTip(tr("<b>Send Mission</b><br>"
				"Requests %1 to send their mission.<br>"
				"You can accept the reply message to make it your ship's mission. "
				"This can be used to modify a ship's mission then send it back to them.").arg(recipient));

	AuIconButton* pbactivate = new AuIconButton(":data/mpReady.png");
	connect(pbactivate, SIGNAL(clicked()), SLOT(ActivateMission()));
	pbactivate->setToolTip(tr("<b>Activate Mission</b><br>"
				"Activates %1's mission.").arg(recipient));

	AuIconButton* pbdeactivate = new AuIconButton(":data/mpNotReady.png");
	connect(pbdeactivate, SIGNAL(clicked()), SLOT(DeactivateMission()));
	pbdeactivate->setToolTip(tr("<b>Hold Mission</b><br>"
				"Deactivates %1's mission. "
				"Orders will not be executed until the mission is activated.").arg(recipient));

	AuIconButton* pbcycle = new AuIconButton(":data/Cycle.png");
	connect(pbcycle, SIGNAL(clicked()), SLOT(CycleMission()));
	pbcycle->setToolTip(tr("<b>Cycle Mission</b><br>"
				"Sets %1's mission to cycle. "
				"Mission orders are rotated to the end of the mission as they are completed.").arg(recipient));

	AuIconButton* pbnocycle = new AuIconButton(":data/DoOnce.png");
	connect(pbnocycle, SIGNAL(clicked()), SLOT(NoCycleMission()));
	pbnocycle->setToolTip(tr("<b>Do Mission Once</b><br>"
				"Sets %1's mission to execute only once. "
				"Mission orders are removed from the mission as they are completed.").arg(recipient));

	m_AddOrder = new AuIconButton(":data/Plus.png");
	connect(m_AddOrder, SIGNAL(clicked()), SLOT(AddMissionOrder()));
	m_AddOrder->setToolTip(tr("<b>Add Mission Order</b><br>"
				"Appends an order to the end of %1's mission.").arg(recipient));

	m_NewOrder = new AuIconButton(":data/NarrateChosen.png");
	connect(m_NewOrder, SIGNAL(clicked()), SLOT(NewMission()));
	m_NewOrder->setToolTip(tr("<b>New Mission</b><br>"
				"Replaces %1's mission with a single order.").arg(recipient));

	m_InsertOrder = new AuIconButton(":data/I.png");
	connect(m_InsertOrder, SIGNAL(clicked()), SLOT(InsertMissionOrder()));
	m_InsertOrder->setToolTip(tr("<b>Insert Mission Order</b><br>"
				"Inserts an order at the beginning of %1's mission. The new order becomes the current order.").arg(recipient));

	m_LoadMission = new AuIconButton(":data/dp_Load.png");
	connect(m_LoadMission, SIGNAL(clicked()), SLOT(LoadMission()));
	m_LoadMission->setToolTip(tr("<b>Load Mission</b><br>"
				"Loads %1's mission from a mission disk in your inventory or from your ship's mission.<br>"
				"Your ship's mission can be loaded if the <b>Spacecraft Mission</b> tool is used to query the mission.")
			.arg(recipient));

	m_SensorContacts = new AuIconButton(":data/SensorSweep0.png");
	connect(m_SensorContacts, SIGNAL(clicked()), SLOT(RequestSensorContacts()));
	m_SensorContacts->setToolTip(tr("<b>Sensor Contacts</b><br>"
				"Requests a snapshot of sensor scan contacts from the spacecraft you are aboard. "
				"Sensor contacts are useful for issuing certain orders, such as <i>follow</i> and <i>attack</i> orders.<p>"
				"Sensor contacts expire after three minutes or when the Mission Transmitter is closed. "
				"The icon on this button shows activity while sensor contacts are available."));

	AuSensorBlipList blips;
	m_MO = new AuMissionOrderWidget(authorization, toSystemId, blips, AuPoint(galPos), AuPoint(secPos), AuPoint(sysPos));
	connect(m_MO, SIGNAL(ValidityChanged()), SLOT(EnableControls()));

	QHBoxLayout* hlay = new QHBoxLayout;
	hlay->addWidget(pbactivate);
	hlay->addWidget(pbdeactivate);
	hlay->addWidget(pbcycle);
	hlay->addWidget(pbnocycle);
	hlay->addStretch(1);
	hlay->addWidget(m_NewOrder);
	hlay->addWidget(m_AddOrder);
	hlay->addWidget(m_InsertOrder);
	hlay->addWidget(m_LoadMission);
	hlay->addStretch(2);
	hlay->addWidget(pbsend);
	hlay->addWidget(pbshow);
	hlay->addWidget(m_SensorContacts);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addLayout(hlay, row,0);
	row++;
	lay->addWidget(m_MO, row,0);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	EnableControls();

	// Create a timer to keep this page updated.
	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	m_timer->start(1000);
}

AuCommMission::~AuCommMission()
{
}

void AuCommMission::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		QTimer::singleShot(0, [=](){RestoreGeometry();});
	}
	else
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

QSize AuCommMission::sizeHint() const
{
	return QSize(400,200);
}

void AuCommMission::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/CommMission/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/CommMission/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/CommMission/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/CommMission/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuCommMission::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/CommMission/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/CommMission/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/CommMission/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/CommMission/SizeY",sz.height());
}

void AuCommMission::ActivateMission()
{
	AuView* view = auApp->Frame()->View();
	AuCommChannel* channel = (AuCommChannel*)parent();

	AuCommMissionProperty msg(m_ToId, 1,1, m_PersonEmpire,
			channel->objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), channel->Range(),
			channel->Scope());
	channel->SendChat(msg);
}

void AuCommMission::AddMissionOrder()
{
	AuMissionOrder* order = m_MO->Order();
	if(!order)
		return;
	if(!order->IsValid())
	{
		delete order;
		return;
	}
	
	AuView* view = auApp->Frame()->View();
	AuCommChannel* channel = (AuCommChannel*)parent();

	AuCommMissionOrderAdd msg(m_ToId, *order, m_PersonEmpire,
			channel->objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), channel->Range(),
			channel->Scope());
	channel->SendChat(msg);
	delete order;
}

void AuCommMission::CycleMission()
{
	AuView* view = auApp->Frame()->View();
	AuCommChannel* channel = (AuCommChannel*)parent();

	AuCommMissionProperty msg(m_ToId, 2,1, m_PersonEmpire,
			channel->objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), channel->Range(),
			channel->Scope());
	channel->SendChat(msg);
}

void AuCommMission::DeactivateMission()
{
	AuView* view = auApp->Frame()->View();
	AuCommChannel* channel = (AuCommChannel*)parent();

	AuCommMissionProperty msg(m_ToId, 1,0, m_PersonEmpire,
			channel->objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), channel->Range(),
			channel->Scope());
	channel->SendChat(msg);
}

void AuCommMission::EnableControls()
{
	m_AddOrder->setEnabled(m_MO->IsContentValid());
	m_InsertOrder->setEnabled(m_MO->IsContentValid());
	m_NewOrder->setEnabled(m_MO->IsContentValid());
}

void AuCommMission::InsertMissionOrder()
{
	AuMissionOrder* order = m_MO->Order();
	if(!order)
		return;
	if(!order->IsValid())
	{
		delete order;
		return;
	}
	
	AuView* view = auApp->Frame()->View();
	AuCommChannel* channel = (AuCommChannel*)parent();

	AuCommMissionOrderInsert msg(m_ToId, *order, m_PersonEmpire,
			channel->objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), channel->Range(),
			channel->Scope());
	channel->SendChat(msg);
	delete order;
}

void AuCommMission::LoadMission()
{
	AuFrame* pFrame = auApp->Frame();
	AuMissionDiskUi dlg(true, true, pFrame);	// List mission disks and ship's current mission.
	int result = auApp->Frame()->ExecDialog(dlg);
	if(result != QDialog::Accepted)
		return;

	AuView* view = auApp->Frame()->View();
	AuCommChannel* channel = (AuCommChannel*)parent();

	AuCommMissionNew msg(m_ToId, dlg.MissionName(), dlg.Mission(), m_PersonEmpire, 
			channel->objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), channel->Range(),
			channel->Scope());
	channel->SendChat(msg);
}

void AuCommMission::NewMission()
{
	const AuMissionOrder* order = m_MO->Order();
	if(!order)
		return;
	if(!order->IsValid())
	{
		delete order;
		return;
	}
	
	AuView* view = auApp->Frame()->View();
	AuCommChannel* channel = (AuCommChannel*)parent();

	AuCommMissionOrderNew msg(m_ToId, *order, m_PersonEmpire, 
			channel->objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), channel->Range(),
			channel->Scope());
	channel->SendChat(msg);
	delete order;
}

void AuCommMission::NoCycleMission()
{
	AuView* view = auApp->Frame()->View();
	AuCommChannel* channel = (AuCommChannel*)parent();

	AuCommMissionProperty msg(m_ToId, 2,0, m_PersonEmpire, 
			channel->objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), channel->Range(),
			channel->Scope());
	channel->SendChat(msg);
}

void AuCommMission::ShowMission()
{
	AuView* view = auApp->Frame()->View();
	AuCommChannel* channel = (AuCommChannel*)parent();

	AuCommMissionProperty msg(m_ToId, 0,0, m_PersonEmpire, 
			channel->objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), channel->Range(),
			channel->Scope());
	channel->SendChat(msg);
}

void AuCommMission::SendMission()
{
	AuView* view = auApp->Frame()->View();
	AuCommChannel* channel = (AuCommChannel*)parent();

	AuCommMissionProperty msg(m_ToId, 3,0, m_PersonEmpire, 
			channel->objectName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), channel->Range(),
			channel->Scope());
	channel->SendChat(msg);
}

void AuCommMission::UpdateContents()
{
	if(m_SensorBlipTime.isValid())
	{
		//qDebug() << "AuMissionTool::UpdateContents elapsed" << m_SensorBlipTime.elapsed();
		// Sensor contacts are discarded when they are too old.
		if(m_SensorBlipTime.elapsed()/d_MSecPerMin > 3)
		{
			m_SensorBlipTime = AuElapsedTimer();
			m_SensorContacts->setIcon(QIcon(":data/SensorSweep0.png"));
		}
		else
		{
			int icon = (m_SensorBlipTime.elapsed() / d_MSecPerSec) % 3 + 1;
			m_SensorContacts->setIcon(QIcon(QString(":data/SensorSweep%1.png").arg(icon)));
		}
	}
}

void AuCommMission::RequestSensorContacts()
{
	auApp->MissionRequestSensorContacts();
}

void AuCommMission::SensorContactsReceived(QByteArray bablips)
{
	// Extract the sensor blips from the byte array.
	AuSensorBlipList sensorBlips;
	QDataStream in(bablips);
	in.setVersion(QDataStream::Qt_4_1);
	in >> sensorBlips;

	m_SensorBlipTime.restart();
	if(m_MO)
		m_MO->SetSensorBlips(sensorBlips);
}
