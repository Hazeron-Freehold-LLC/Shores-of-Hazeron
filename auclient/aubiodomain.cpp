//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubiodomain.cpp,v 1.16 2022/02/19 18:08:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubiodomain.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <QVBoxLayout>

AuBioDomainTable::AuBioDomainTable(QWidget* parent) : QTableWidget(0,1, parent)
{
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setShowGrid(false);

	//QStringList headerLabels;
	//headerLabels << tr("Solar System");
	//setHorizontalHeaderLabels(headerLabels);

	setSortingEnabled(true);
	sortByColumn(0, Qt::AscendingOrder);

	QHeaderView* hhdr = horizontalHeader();
	hhdr->setSectionResizeMode(0, QHeaderView::Stretch);

	horizontalHeader()->hide();
	verticalHeader()->hide();
	verticalHeader()->setDefaultSectionSize(18);
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
}

void AuBioDomainTable::mousePressEvent(QMouseEvent* event)
{
	QTableWidget::mousePressEvent(event);
	m_MousePressItem = itemAt(event->pos());
}

void AuBioDomainTable::mouseReleaseEvent(QMouseEvent* event)
{
	QTableWidget::mouseReleaseEvent(event);

	if(event->button() == Qt::RightButton)
	{
		QTableWidgetItem* item = itemAt(event->pos());
		if(item == m_MousePressItem)
			emit ItemRightClicked(item);
	}
}

void AuBioDomainTable::UpdateContents(const AuDomain& domain)
{
	// Refresh the contents of the table widget.
	if(m_Contents == domain)
		return;
	m_Contents = domain;
	clearContents();
	while(rowCount())
		removeRow(0);

	setSortingEnabled(false);
	for(AuDomain::const_iterator si = domain.cbegin(); si != domain.cend(); si++)
	{
		insertRow(0);

		QIcon plico(":data/RangeSystem.png");

		// System Name
		int col=0;
		QTableWidgetItem* twname = new QTableWidgetItem(plico, *si);
		twname->setData(Qt::UserRole, QVariant::fromValue<AuId>(si.key()));
		setItem(0,col++, twname);
	}
	setSortingEnabled(true);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBioDomain::AuBioDomain(QWidget *parent) : QWidget(parent)
{
	m_timer = 0;

	QLabel* lbInfo = new QLabel(tr("<html>"
				"<p>"
				"Buildings are protected from abandonment decay when their solar system is in the <i>land</i> of any avatar. "
				"<br>"
				"%2 Buildings do not have to be part of a city. "
				"<br>"
				"%2 Empire is not a factor. "
				"</p>"
				"<p>"
				"An avatar's land includes their home solar system, even if their home building is destroyed. "
				"Up to %1 additional solar systems may be added to their land. "
				"<br>"
				"%2 Land is arbitrary; any solar systems can be added. "
				"<br>"
				"%2 Land does not claim solar systems as property. "
				"<br>"
				"%2 Any number of avatars may add the same solar system to their land. "
				"<br>"
				"%2 There is no way to know if a solar system is in another avatar's land. "
				"</p>"
				"<p>"
				"Right-click the list to add the current solar system to your land."
				"</p>"
				"</html>").arg(AuDomain::LimitPerAvatar()).arg(g_Au.MiddleDotSymbol()), this);
	lbInfo->setAlignment(Qt::AlignLeft|Qt::AlignTop);
	lbInfo->setWordWrap(true);

	m_HomeCitySysIcon = new QLabel(this);

	m_HomeCitySys = new QLabel(this);
	m_HomeCitySys->setToolTip(tr("<html>"
				"<p>"
				"Your home solar system is your land, even if your home building is destroyed. "
				"</p>"
				"<p>"
				"Use the <i>Building</i> page of the <i>Building</i> window to change your home. "
				"</p>"
				"</html>"));

	m_Domain = new AuBioDomainTable(this);
	m_Domain->setToolTip(tr("<html>"
				"<p>"
				"You may add up to %1 solar systems to your land. "
				"Right-click to add or remove a solar system. "
				"</p>"
				"<p>"
				"Solar systems can also be added to your land using the <i>Cities and Bases</i>, <i>System Map</i> and "
				"<i>Star Map</i> windows. "
				"</p>"
				"</html>").arg(AuDomain::LimitPerAvatar()));
	connect(m_Domain, SIGNAL(ItemRightClicked(QTableWidgetItem*)), SLOT(SystemRightClicked(QTableWidgetItem*)));

	m_Counts = new QLabel(this);

	QHBoxLayout* sslay = new QHBoxLayout;
	sslay->addWidget(m_HomeCitySysIcon);
	sslay->addWidget(m_HomeCitySys, 1);
	sslay->addWidget(m_Counts);

	// Layout the widgets.
	QVBoxLayout* dlay = new QVBoxLayout;
	dlay->addLayout(sslay);
	dlay->addWidget(m_Domain);

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addLayout(dlay);
	lay->addWidget(lbInfo);

	setLayout(lay);
}

AuBioDomain::~AuBioDomain()
{
}

QSize AuBioDomain::sizeHint() const
{
	return QSize(400,300);
}

void AuBioDomain::showEvent(QShowEvent* event)
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

void AuBioDomain::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	// Get the scene atom so we can show their information.
	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(!person)
	{
		view->UnlockSceneAtom();
		return;
	}

	AuDomain domain;
	person->Domain(domain);
	QString spawnSys = person->SpawnCitySystemName();
	view->UnlockSceneAtom();

	m_HomeCitySys->setText(spawnSys);
	if(!spawnSys.isEmpty())
		m_HomeCitySysIcon->setPixmap(QPixmap(":data/RangeSystem.png").scaled(16,16));
	else
		m_HomeCitySysIcon->setPixmap(QPixmap());

	m_Domain->blockSignals(true);
	m_Domain->UpdateContents(domain);
	m_Domain->blockSignals(false);

	m_Counts->setText(QString("%1/%2")
			.arg(domain.size())
			.arg(AuDomain::LimitPerAvatar()));
}

void AuBioDomain::SystemRightClicked(QTableWidgetItem* item)
{
	// Show a menu with the delete option.
	QMenu menu(this);

	QAction* add;
	if(m_Domain->rowCount() < AuDomain::LimitPerAvatar())
		add = menu.addAction(QIcon(":data/Plus.png"), tr("Add Current Solar System..."));
	else
		add = 0;

	QAction* remove;
	if(item)
		remove = menu.addAction(QIcon(":data/Minus.png"), tr("Remove Solar System..."));
	else
		remove = 0;

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;

	if(action == add)
	{
		QString smsg = tr("<html>"
				"Confirm adding current solar system to your land."
				"</html>");
		QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel,
				QMessageBox::NoButton, this);
		int result = auApp->Frame()->ExecMessageBox(mb);
		if(result != QMessageBox::Ok)
			return;

		auApp->LandAddCurrentSystem();
	}
	else if(action == remove)
	{
		AuId systemId = item->data(Qt::UserRole).value<AuId>();
		QString systemName = item->text();

		QString smsg = tr("<html>"
				"Confirm removal of %1 from your land."
				"</html>").arg(systemName);
		QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel,
				QMessageBox::NoButton, this);
		int result = auApp->Frame()->ExecMessageBox(mb);
		if(result != QMessageBox::Ok)
			return;

		auApp->LandRemoveSystem(systemId, systemName);
	}
}

