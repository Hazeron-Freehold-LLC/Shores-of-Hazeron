//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audamagereport.cpp,v 1.13 2022/05/04 21:11:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audamagereport.h"
//
#include "auapp.h"
#include "audamage.h"
#include "auframe.h"
#include "autoolbutton.h"
#include "auview.h"
#include "daperson.h"
#include "davessel.h"
#include <QLabel>
#include <QProgressBar>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>

void AuSpacecraftDamageDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftDamageUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDamageReportSystem::AuDamageReportSystem(QWidget* parent) : QWidget(parent)
{
	m_Icon = new QLabel(this);
	m_Icon->setAlignment(Qt::AlignLeft|Qt::AlignTop);
	m_Icon->setToolTip(tr("<html>"
				"System Type"
				"</html>"));

	m_Name = new QLabel(this);

	m_Bar = new QProgressBar(this);
	m_Bar->setRange(0,100);
	m_Bar->setFormat(QString());
	m_Bar->setMaximumHeight(6);
	m_Bar->setToolTip(tr("<html>"
				"System Health Percentage"
				"</html>"));

	m_PercentIco = new QLabel(this);
	m_PercentIco->setToolTip(tr("<html>"
				"System Status"
				"<br>Grey = No Modules"
				"<br>Red = 10% or Less"
				"<br>Amber = 11% to 50%"
				"<br>Green = 51% to 100%"
				"</html>"));

	m_Percent = new QLabel(this);

	m_Parts = new QLabel(this);
	m_Parts->setAlignment(Qt::AlignLeft|Qt::AlignTop);
	m_Parts->setWordWrap(true);
	m_Parts->setMinimumWidth(32);

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addWidget(m_Name, 1);
	blay->addWidget(m_Percent);
	blay->addWidget(m_PercentIco);
	blay->setSpacing(2);
	blay->setMargin(0);

	QVBoxLayout* nlay = new QVBoxLayout();
	nlay->addLayout(blay);
	nlay->addWidget(m_Bar);
	nlay->addStretch();
	nlay->setSpacing(2);
	nlay->setMargin(0);

	QHBoxLayout* lay = new QHBoxLayout();
	lay->addWidget(m_Icon);
	lay->addLayout(nlay, 1);
	lay->addWidget(m_Parts);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuDamageReportSystem::~AuDamageReportSystem()
{
}

void AuDamageReportSystem::Update(const AuSysId& sysId, COMMODITYID curModule, const QString& sysName, const AuDamage& damage,
		quint32 maxHits, quint32 maxComponents)
{
	m_Icon->setPixmap(QPixmap(sysId.Icon()).scaled(22,22, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	m_Name->setText(QString("<html><b>%1</b></html>").arg(sysName));

	bool noModule = maxComponents && DbCommodity::IsModule(curModule) && damage.PartsDamaged(sysId, curModule) >= maxComponents;

	quint32 dmg = damage.HitsDamageAddParts(sysId);	// This is in hit points.
	if(dmg > maxHits)
		dmg = maxHits;
	int percDmg;
	if(dmg >= maxHits || maxHits == 0)
		percDmg = 100;
	else
	{
		percDmg = Percent(dmg, maxHits);
		if(!percDmg && dmg)
			percDmg = 1;
		else if(percDmg == 100)
			percDmg = 99;
	}

	if(noModule)
		m_PercentIco->setPixmap(QPixmap(":data/AlertNeutral.png").scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	else if(percDmg >= 90)
		m_PercentIco->setPixmap(QPixmap(":data/AlertRedBright.png").scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	else if(percDmg >= 50)
		m_PercentIco->setPixmap(QPixmap(":data/AlertYellowBright.png").scaled(16,16, Qt::KeepAspectRatio,Qt::SmoothTransformation));
	else
		m_PercentIco->setPixmap(QPixmap(":data/AlertGreen.png").scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));

	int health = 100 - percDmg;
	m_Bar->setValue(health);
	if(!dmg)
		m_Percent->setText(QString("%L1hp 100%")
				.arg(maxHits));
	else if(noModule)
		m_Percent->setText(QString("<b><i>No Module</i></b> %L1/%L2hp %3%")
				.arg(maxHits - dmg)
				.arg(maxHits)
				.arg(health));
	else
		m_Percent->setText(QString("%L1/%L2hp %3%")
				.arg(maxHits - dmg)
				.arg(maxHits)
				.arg(health));

	AuRepairParts parts;
	damage.PartsDamage(sysId, parts);
	QString partsList;
	QString toolTip;
	if(!parts.empty())
	{
		// Make an html formatted list of the parts with icons.
		// Names make them too big; we'll put the names in a tool tip.
		QString part;
		toolTip = tr("<p>Parts Needed");
		for(AuRepairParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
		{
			if(pi == parts.cbegin())
				part = QString("<img height=\"16\" src=\"qrc%1\"> %L2")
					.arg(g_Au.CommodityBitmapName(pi.key()))
					.arg(*pi);
			else
				part = QString("<br><img height=\"16\" src=\"qrc%1\"> %L2")
					.arg(g_Au.CommodityBitmapName(pi.key()))
					.arg(*pi);
			partsList += part;
			toolTip += QString("<br>%L1 %2")
				.arg(*pi)
				.arg(g_Au.CommodityName(pi.key()));
		}
		toolTip += "</p>";

		if(noModule)
			toolTip += tr("<p>"
					"System needs a complete set of modules. "
					"The module type can be changed by installing any module compatible with the system. "
					"</p>");
	}
	else
		toolTip = tr("No Parts Needed");
	m_Parts->setText(partsList);
	m_Parts->setToolTip(toolTip);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDamageReport::AuDamageReport(QWidget* parent) : QWidget(parent)
{
	m_LastAllOrDamaged = g_Au.Settings().readBoolEntry("Widget/DamageReport/AllOrDamaged", false);

	m_Name = new QLabel(this);

	m_Conditions = new QLabel(this);
	m_Conditions->setWordWrap(true);

	m_AllOrDamaged = new AuIconButton(QIcon(":data/MsgDamageReport.png"), this);
	m_AllOrDamaged->setCheckable(true);
	m_AllOrDamaged->setToolTip(tr("<html>"
				"Shows All Systems or Damaged Systems"
				"</html>"));
	m_AllOrDamaged->setChecked(m_LastAllOrDamaged);
	connect(m_AllOrDamaged, SIGNAL(clicked()), SLOT(AllOrDamaged()));

	// Build the layout.
	QWidget* epanels = new QWidget;
	m_SystemLay = new QVBoxLayout(epanels);
	epanels->setLayout(m_SystemLay);

	m_SystemScroller = new QScrollArea;
	m_SystemScroller->setWidget(epanels);
	m_SystemScroller->setWidgetResizable(true);

	QHBoxLayout* nlay = new QHBoxLayout;
	nlay->addWidget(m_Name, 1);
	nlay->addWidget(m_AllOrDamaged);
	nlay->setSpacing(2);
	nlay->setMargin(0);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(nlay);
	lay->addWidget(m_Conditions);
	lay->addWidget(m_SystemScroller,1);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);

	m_timer = 0;
}

AuDamageReport::~AuDamageReport()
{
}

// Update contents of dialog that might change while the dialog is up.
void AuDamageReport::UpdateContents()
{
	// Lock the root and find the scene atom.
	AuDamage damage;
	bool allOrDamaged = m_AllOrDamaged->isChecked();
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			ship->DamageReportHp(damage);		// Damaged systems only
			if(m_LastDamage == damage && m_LastAllOrDamaged == allOrDamaged)
			{
				pView->UnlockSceneAtom();
				return;
			}

			m_Name->setText(ship->NameWithEmpire());

			if(ship->IsAnyConditionVisible())
			{
				QString conds;
				const AuConditionList& cond = ship->LockConditions();
				for(AuConditionList::const_iterator ci = cond.cbegin(); ci != cond.cend(); ci++)
				{
					if(!(*ci)->IsVisible())
						continue;
					if(conds.isEmpty())
						conds += QString("<img height=\"16\" src=\"qrc%1\"> %2")
							.arg((*ci)->Bitmap())
							.arg((*ci)->Name());
					else
						conds += QString("<br><img height=\"16\" src=\"qrc%1\"> %2")
							.arg((*ci)->Bitmap())
							.arg((*ci)->Name());
				}
				ship->UnlockConditions();
				m_Conditions->setText(QString("<html>%1</html>").arg(conds));
				m_Conditions->setVisible(true);
			}
			else
				m_Conditions->setVisible(false);

			// Get the names of those systems.
			QMultiMap<QString, AuSysId> names;	// System names sorted alphabetically. Names should not duplicate but multi is safe.
			QSet<AuSysId> sysIds;
			damage.SystemIds(sysIds);
			for(QSet<AuSysId>::const_iterator di = sysIds.cbegin(); di != sysIds.cend(); di++)
			{
				QString name = ship->SystemName(*di);
				names.insert(name, *di);
			}

			// Damage information changed.
			// Insure we have enough systems widgets.
			while(m_SystemWidgets.size() < names.size())
			{
				AuDamageReportSystem* sys = new AuDamageReportSystem();
				m_SystemWidgets.append(sys);
				m_SystemLay->addWidget(sys);
			}

			// Update systems info. Show/hide system widgets as needed.
			QList<AuDamageReportSystem*>::iterator si = m_SystemWidgets.begin();
			for(QMultiMap<QString, AuSysId>::const_iterator ni = names.cbegin(); ni != names.cend(); ni++)
			{
				if(!(*ni).IsStructural())
					continue;	// List structural first.
				if(allOrDamaged && !damage.HitsDamageAddParts(*ni))
					continue;	// System is undamaged.

				COMMODITYID curModule = ship->SystemModule(*ni);

				(*si)->show();
				(*si)->Update(*ni, curModule, ni.key(), damage, ship->SystemHits(*ni), ship->SystemComponents(*ni, curModule));
				si++;
			}
			for(QMultiMap<QString, AuSysId>::const_iterator ni = names.cbegin(); ni != names.cend(); ni++)
			{
				if((*ni).IsStructural())
					continue;	// List equipment/nonstructural after structural.
				if(allOrDamaged && !damage.HitsDamageAddParts(*ni))
					continue;	// System is undamaged.

				COMMODITYID curModule = ship->SystemModule(*ni);

				(*si)->show();
				(*si)->Update(*ni, curModule, ni.key(), damage, ship->SystemHits(*ni), ship->SystemComponents(*ni, curModule));
				si++;
			}

			// Hide excess systems widgets, if any.
			while(si != m_SystemWidgets.end())
			{
				(*si)->hide();
				si++;
			}
		}
	}
	pView->UnlockSceneAtom();
	m_LastDamage = damage;
	m_LastAllOrDamaged = allOrDamaged;
}

void AuDamageReport::showEvent(QShowEvent* event)
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

void AuDamageReport::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_timer && m_timer->isActive())
		m_timer->stop();
}

void AuDamageReport::AllOrDamaged()
{
	bool isChecked = m_AllOrDamaged->isChecked();
	g_Au.Settings().writeEntry("Widget/DamageReport/AllOrDamaged", isChecked);
}

QSize AuDamageReport::sizeHint() const
{
	return QSize(500,600);
}

