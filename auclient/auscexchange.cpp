//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscexchange.cpp,v 1.15 2022/05/04 21:50:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscexchange.h"
//
#include "auapp.h"
#include "aubuyblueprint.h"
#include "auclientglobal.h"
#include "audesign.h"
#include "auscexchangewidget.h"
#include "auscpublishdlg.h"
#include "auframe.h"
#include "aupicturewidget.h"
#include "aurateblueprint.h"
#include <auroster.h>
#include "auview.h"
#include <dagalaxy.h>
#include <daperson.h>
#include <dasector.h>
#include <dgdata.h>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QTextBrowser>
#include <QTimer>

void AuScExchangeDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleScExchangeUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScExchange::AuScExchange(QWidget* parent) : QWidget(parent)
{
	const DgData& gd = g_Au.LockDataForRead();
	bool isArchitect = gd.ClientPlayerIsArchitect();
	g_Au.UnlockData();

	float scale = g_Au.Settings().readNumEntry("Widget/ScExchange/ImageScale", 1.f);

	m_Exchange = new AuScExchangeWidget(this);
	m_Exchange->setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(m_Exchange, SIGNAL(itemSelectionChanged()), SLOT(DesignSelectionChanged()));
	connect(auApp, SIGNAL(ScExchangeUpdated()), m_Exchange, SLOT(RefreshContents()));
	connect(auApp, SIGNAL(ScExchangeUpdated()), SLOT(RefreshThumb()));

	m_DesignName = new QLineEdit(this);
	m_DesignName->setReadOnly(true);
	m_DesignName->setToolTip(tr("<html>"
				"Name of the blueprint in the library. "
				"Blueprints do not have to have unique names. "
				"More than one blueprint can share the same name. "
				"</html>"));

	m_DesignId = new QLineEdit(this);
	m_DesignId->setReadOnly(true);
	m_DesignId->setToolTip(tr("<html>"
				"Number of the current blueprint. "
				"Blueprints are identified by number. "
				"</html>"));

	m_DesignThumb = new AuPictureWidget(false, this);
	m_DesignThumb->SetScale(scale);
	connect(m_DesignThumb, SIGNAL(ScaleChanged(float)), SLOT(ImageScaleChanged(float)));

	m_DesignReport = new QTextBrowser(this);
	m_DesignReport->setOpenExternalLinks(true);
	m_DesignReport->setReadOnly(true);

	m_RequestDesigns = new QPushButton(tr("Refresh Published Blueprints"), this);
	m_RequestDesigns->setToolTip(tr("<html>"
				"Requests the list of all published blueprints that are available to your avatar. "
				"</html>"));
	connect(m_RequestDesigns, SIGNAL(clicked()), SLOT(RequestDesigns()));

	m_Buy = new QPushButton(tr("Buy Blueprint..."), this);
	m_Buy->setEnabled(false);
	m_Buy->setToolTip(tr("<html>"
				"Buys the blueprint for the terms specified. "
				"Your avatar must possess blank media to receive the blueprint, such as a blank disk. "
				"</html>"));
	connect(m_Buy, SIGNAL(clicked()), SLOT(BuyDesign()));

	m_Rate = new QPushButton(tr("Rate Blueprint..."), this);
	m_Rate->setEnabled(false);
	m_Rate->setToolTip(tr("<html>"
				"Rate and review the blueprint. "
				"</html>"));
	connect(m_Rate, SIGNAL(clicked()), SLOT(RateDesign()));

	m_Ratings = new QPushButton(tr("View Blueprint Ratings"), this);
	m_Ratings->setEnabled(false);
	m_Ratings->setToolTip(tr("<html>"
				"View the reviews on the blueprint. "
				"Reviews are the basis for its rating. "
				"</html>"));
	connect(m_Ratings, SIGNAL(clicked()), SLOT(ViewRatings()));

	m_Publish = new QPushButton(tr("Publish Blueprints..."), this);
	m_Publish->setToolTip(tr("<html>"
				"Publishes blueprints to the exchange. "
				"You can only publish blueprints designed by your avatar. "
				"<br>"
				"Only <i>Final</i> and <i>Assembly</i> blueprints can be published. "
				"</html>"));
	connect(m_Publish, SIGNAL(clicked()), SLOT(PublishDesign()));

	m_Unpublish = new QPushButton(tr("Unpublish Blueprints..."), this);
	m_Unpublish->setEnabled(false);
	if(isArchitect)
		m_Unpublish->setToolTip(tr("<html>"
					"Unpublishes selected blueprints from the exchange. "
					"You can only unpublish blueprints published by your avatar. "
					"<br>"
					"<b>Architect can unpublish any blueprints, regardless of publisher.</b>"
					"</html>"));
	else
		m_Unpublish->setToolTip(tr("<html>"
					"Unpublishes selected blueprints from the exchange. "
					"You can only unpublish blueprints published by your avatar. "
					"</html>"));
	connect(m_Unpublish, SIGNAL(clicked()), SLOT(UnpublishDesign()));

	m_Terms = new QTextBrowser(this);
	m_Terms->setReadOnly(true);

	// Put it together.
	QHBoxLayout* rdlay = new QHBoxLayout;
	rdlay->addWidget(m_RequestDesigns);
	rdlay->addStretch();
	rdlay->addWidget(m_Publish);
	rdlay->addWidget(m_Unpublish);
	rdlay->addStretch();

	QVBoxLayout* liblay = new QVBoxLayout;
	liblay->addLayout(rdlay);
	liblay->addWidget(m_Exchange);
	liblay->setMargin(0);
	liblay->setSpacing(2);

	m_Splitter2 = new QSplitter(Qt::Vertical);
	m_Splitter2->addWidget(m_DesignThumb);
	m_Splitter2->addWidget(m_DesignReport);
	m_Splitter2->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_Splitter2, SIGNAL(splitterMoved(int, int)), SLOT(Splitter2Moved(int, int)));

	QWidget* lwid = new QWidget;
	lwid->setLayout(liblay);

	m_Splitter = new QSplitter;
	m_Splitter->addWidget(lwid);
	m_Splitter->addWidget(m_Splitter2);
	m_Splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	QVBoxLayout* blay = new QVBoxLayout;
	blay->addStretch();
	blay->addWidget(m_Buy);
	blay->addWidget(m_Rate);
	blay->addWidget(m_Ratings);

	QGridLayout* rowlay = new QGridLayout;
	rowlay->addWidget(m_DesignId, 0,0);
	rowlay->addWidget(m_DesignName, 0,1);
	rowlay->addWidget(m_Terms, 1,0, 1,2);
	rowlay->addLayout(blay, 0,2, 2,1);
	rowlay->setColumnStretch(1,1);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_Splitter, 10);
	lay->addLayout(rowlay);
	lay->setMargin(4);
	lay->setSpacing(2);
	setLayout(lay);

	QByteArray splitterstate;
	g_Au.Settings().readEntry("Widget/ScExchange/Splitter", splitterstate);
	m_Splitter->restoreState(splitterstate);
	g_Au.Settings().readEntry("Widget/ScExchange/Splitter2", splitterstate);
	m_Splitter2->restoreState(splitterstate);

	for(int col=0; col<m_Exchange->columnCount(); col++)
	{
		if(col == 1)
			continue;	// skip the automatically sized column
		int colwid = g_Au.Settings().readNumEntry(QString("Widget/ScExchange/ColumnWid%1").arg(col),-123);
		if(colwid != -123)
			m_Exchange->setColumnWidth(col, colwid);
	}
}

AuScExchange::~AuScExchange()
{
}

void AuScExchange::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	RequestDesigns();
}

void AuScExchange::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Save the sizes of the table widget's columns.
	for(int col=0; col<m_Exchange->columnCount(); col++)
		g_Au.Settings().writeEntry(QString("Widget/ScExchange/ColumnWid%1").arg(col), m_Exchange->columnWidth(col));
}

quint32 AuScExchange::DesignId() const
{
	return m_DesignId->text().toUInt();
}

QString AuScExchange::DesignName() const
{
	return m_DesignName->text().trimmed();
}

void AuScExchange::SetDesign(quint32 id, const QString& name, const QString& report)
{
	if(id)
		m_DesignId->setText(QVariant(id).toString());
	else
		m_DesignId->setText("");

	m_DesignName->setText(name.trimmed());

	m_DesignReport->setHtml(report);

	const DgData& gd = g_Au.LockDataForRead();
	AuId unitId = gd.ClientUnitId();
	bool isArchitect = gd.ClientPlayerIsArchitect();
	g_Au.UnlockData();

	QHash<quint32, QString> selDesigns;
	m_Exchange->SelectedDesigns(selDesigns);
	bool unpubOk = !selDesigns.empty();
	if(unpubOk && !isArchitect)
	{
		QHash<quint32, AuScDesignExchangeEntryClient>& designs = g_AuClient.LockScDesignExchange();
		for(QHash<quint32, QString>::const_iterator si = selDesigns.cbegin(); si != selDesigns.cend(); si++)
		{
			QHash<quint32, AuScDesignExchangeEntryClient>::const_iterator di = designs.find(si.key());
			if(di == designs.cend() || (*di).DesignArchitectId() != unitId)
			{
				// Design not found or different architect than local player.
				unpubOk = false;
				break;
			}
		}
		g_AuClient.UnlockScDesignExchange();
	}

	// Get the terms for purchase and the thumb of the design.
	QString terms;
	bool isRated = false;
	if(id)
	{
		QHash<quint32, AuScDesignExchangeEntryClient>& designs = g_AuClient.LockScDesignExchange();
		QHash<quint32, AuScDesignExchangeEntryClient>::const_iterator di = designs.find(id);
		if(di != designs.end())
		{
			isRated = (*di).Raters();
			terms = tr("<p><big><b>Terms of Sale</b></big></p>");

			m_DesignThumb->SetImage((*di).Thumb());

			if((*di).Price())
				terms += tr("<p><b>%L1%2</b> per copy of the blueprint.</p>")
					.arg((*di).Price()).arg(g_Au.CurrencySymbol());
			else
				terms += tr("<p><b>Free.</b> Copies of the blueprint cost nothing.</p>");

			if((*di).CanCopy())
				terms += tr("<p><b>Copy.</b> "
						"The blueprint can be opened in a design studio. "
						"Parts can be copied to the clipboard and the blueprint can be saved to a file or to the library as a "
						"new blueprint. "
						"<i>Final</i> blueprints can be built at a factory. "
						"</p>");
			else if((*di).CanView())
				terms += tr("<p><b>View.</b> "
						"The blueprint can be opened in a design studio. "
						"Parts cannot be copied to the clipboard and the blueprint cannot be saved to a file or to the library. "
						"<i>Final</i> blueprints can be built at a factory. "
						"</p>");
			else
				terms += tr("<p><b>Make.</b> "
						"<i>Final</i> blueprints can be built at a factory. "
						"The blueprint cannot be opened in a design studio. "
						"<br>"
						"<i>Assembly</i> and <i>Unfinished</i> blueprints cannot be built at a factory. "
						"</p>");

			QString empireName;
			const DgData& gd = g_Au.LockDataForRead();
			gd.EmpireName((*di).PublisherEmpire(), empireName);
			g_Au.UnlockData();
			switch((*di).MinStanceToBuyer())
			{
			case POL_Vassal:
			case POL_Enemy:
				terms += tr("<p><b>All Access.</b> All empires can access this blueprint."
						"</p>");
				break;
			case POL_Neutral:
				terms += tr("<p><b>Neutral Access.</b> Enemy empires of %1 may not access this blueprint. "
						"This restriction is preserved with the blueprint. "
						"It remains in effect even if the blueprint is transferred."
						"</p>").arg(empireName);
				break;
			case POL_Friend:
				terms += tr("<p><b>Friend Access.</b> Enemy and neutral empires of %1 may not access this blueprint. "
						"This restriction is preserved with the blueprint. "
						"It remains in effect even if the blueprint is transferred."
						"</p>").arg(empireName);
				break;
			case POL_Empire:
				terms += tr("<p><b>Empire Access</b> Only members of %1 may access this blueprint. "
						"This restriction is preserved with the blueprint. "
						"It remains in effect even if the blueprint is transferred."
						"</p>").arg(empireName);
				break;
			}

			if((*di).DesignState() == AuDesign::Final)
			{
				// UseOnce only applies to (*di) that are final.
				if((*di).UseOnce())
					terms += tr("<p><b>Make One.</b> "
							"The blueprint can be used at a factory only once. "
							"It can be used to purchase a company spacecraft or it can be used to commission a fleet spacecraft. "
							"After purchasing or commissioning a spacecraft, the blueprint is destroyed. "
							"</p>");
				else
					terms += tr("<p><b>Make Unlimited.</b> "
							"The blueprint can be used to purchase or manufacture spacecraft in unlimited quantities. "
							"</p>");
			}
		}
		g_AuClient.UnlockScDesignExchange();
	}

	if(terms.isEmpty())
		m_DesignThumb->SetImage(QImage());
	m_Unpublish->setEnabled(unpubOk);

	m_Terms->setHtml(terms);

	m_Buy->setEnabled(id);
	m_Rate->setEnabled(id);
	m_Ratings->setEnabled(id && isRated);
}

void AuScExchange::RequestDesigns()
{
	auApp->ScExchangeRequestDesigns();
}

void AuScExchange::DesignSelectionChanged()
{
	quint32 designId = m_Exchange->CurrentDesignId();
	QString designName = m_Exchange->CurrentDesignName();
	QString designReport = m_Exchange->CurrentDesignReport();

	SetDesign(designId, designName, designReport);
}

void AuScExchange::SplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/ScExchange/Splitter", m_Splitter->saveState());
}

void AuScExchange::Splitter2Moved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/ScExchange/Splitter2", m_Splitter2->saveState());
}

void AuScExchange::RefreshThumb()
{
	quint32 designId = DesignId();
	if(designId)
	{
		QHash<quint32, AuScDesignExchangeEntryClient>& designs = g_AuClient.LockScDesignExchange();
		QHash<quint32, AuScDesignExchangeEntryClient>::const_iterator di = designs.find(designId);
		if(di != designs.end())
			m_DesignThumb->SetImage((*di).Thumb());
		g_AuClient.UnlockScDesignExchange();
	}
	else
		m_DesignThumb->SetImage(QImage());
}

void AuScExchange::BuyDesign()
{
	quint32 designId = DesignId();
	if(!designId)
		return;

	AuBuyBlueprint dlg(designId, DesignName(), m_Terms->toHtml());
	if(auApp->Frame()->ExecDialog(dlg) != QDialog::Accepted)
		return;
	auApp->ScExchangeBuyDesign(designId, dlg.MediaPos());
}

void AuScExchange::PublishDesign()
{
	const DgData& gd = g_Au.LockDataForRead();
	AuId unitId = gd.ClientUnitId();
	g_Au.UnlockData();

	AuScPublishDlg dlg(unitId, this);
	if(dlg.exec() == QDialog::Accepted)
	{
		QList<quint32> designs;
		dlg.DesignIds(designs);
		for(QList<quint32>::const_iterator pi = designs.cbegin(); pi != designs.cend(); pi++)
		{
			quint32 designId = *pi;
			if(!designId)
				continue;

			const QHash<quint32, AuScDesignListEntry>& designs = g_AuClient.LockScDesignList();
			QHash<quint32, AuScDesignListEntry>::const_iterator di = designs.find(designId);
			if(di == designs.cend() ||
					((*di).State() != AuDesign::Final && (*di).State() != AuDesign::FinalAssembly))
			{
				g_AuClient.UnlockScDesignList();
				continue;
			}

			// Publisher empire is filled in by the server so I can pass a 0.
			AuScDesignExchangeEntry libEntry((*di).Report(), (*di).State(), dlg.Price(), dlg.CanCopy(), dlg.CanView(),
					dlg.UseOnce(), 0, dlg.MinStance());
			g_AuClient.UnlockScDesignList();

			// Publish the design.
			auApp->ScExchangePublish(libEntry);
		}
	}
}

void AuScExchange::UnpublishDesign()
{
	QHash<quint32, QString> designs;
	m_Exchange->SelectedDesigns(designs);
	if(designs.empty())
		return;

	QString smsg;
	if(designs.size() == 1)
		smsg = tr("<html>"
				"This action will remove the blueprint from the exchange. "
				"<br>"
				"Confirm unpublish of blueprint <b>%1 %2</b>. "
				"</html>").arg(designs.cbegin().key()).arg(*designs.cbegin());
	else
		smsg = tr("<html>"
				"This action will remove the blueprints from the exchange. "
				"<br>"
				"Confirm unpublish of %L1 blueprints. "
				"</html>").arg(designs.size());

	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	//qDebug() << "AuScExchange::UnpublishDesign";

	// Unpublish the blueprints.
	for(QHash<quint32, QString>::const_iterator di = designs.cbegin(); di != designs.cend(); di++)
		auApp->ScExchangeUnpublish(di.key());
}

void AuScExchange::ImageScaleChanged(float scale)
{
	g_Au.Settings().writeEntry("Widget/ScExchange/ImageScale", scale);
}

void AuScExchange::RateDesign()
{
	quint32 designId = DesignId();
	if(!designId)
		return;

	AuRateBlueprint dlg(designId, DesignName(), m_DesignReport->toHtml());
	if(auApp->Frame()->ExecDialog(dlg) != QDialog::Accepted)
		return;

	// Rate the design.
	auApp->ScExchangeRate(designId, dlg.Rating(), dlg.Comment());
}

void AuScExchange::ViewRatings()
{
	quint32 designId = DesignId();
	auApp->ScExchangeViewRatings(designId);
}

QSize AuScExchange::sizeHint() const
{
	return QSize(1000,600);
}

