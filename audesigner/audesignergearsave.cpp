//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignergearsave.cpp,v 1.8 2022/02/19 18:25:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audesignergearsave.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "audesign.h"
#include "auglobal.h"
#include "auframe.h"
#include "auscwidget.h"
#include "auview.h"
#include "daperson.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSplitter>
#include <QTextBrowser>

AuDesignerGearSave::AuDesignerGearSave(const AuId& architectId, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Save Spacecraft Blueprint to Gear"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	m_Designs = new AuScWidget(architectId, true, this);
	connect(m_Designs, SIGNAL(itemSelectionChanged()), SLOT(DesignSelectionChanged()));
	connect(auApp, SIGNAL(ScLibraryUpdated()), m_Designs, SLOT(RefreshContents()));
	connect(auApp, SIGNAL(ScLibraryUpdated()), SLOT(RefreshThumb()));

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

	m_DesignThumb = new QLabel(this);
	m_DesignThumb->setAlignment(Qt::AlignCenter);

	m_DesignReport = new QTextBrowser(this);
	m_DesignReport->setOpenExternalLinks(true);
	m_DesignReport->setReadOnly(true);

	m_RequestDesigns = new QPushButton(tr("Refresh Blueprints"), this);
	m_RequestDesigns->setToolTip(tr("<html>"
				"Requests all blueprints that were designed by your avatar. "
				"</html>"));
	connect(m_RequestDesigns, SIGNAL(clicked()), SLOT(RequestDesigns()));

	m_CanCopy = new QRadioButton(tr("Blueprint can be copied."), this);
	m_CanCopy->setToolTip(tr("<html>"
				"The blueprint can be opened in a design studio. "
				"Parts can be copied to the clipboard and the blueprint can be saved to a file or to the library as a new design. "
				"<br>"
				"<i>Final</i> blueprints can be built at a factory. "
				"</html>"));
	m_CanCopy->setChecked(true);
	connect(m_CanCopy, SIGNAL(clicked()), SLOT(EnableControls()));

	m_CanView = new QRadioButton(tr("Blueprint can be viewed."), this);
	m_CanView->setToolTip(tr("<html>"
				"The blueprint can be opened in a design studio. "
				"Parts cannot be copied to the clipboard and the blueprint cannot be saved to a file or to the library. "
				"<br>"
				"<i>Final</i> blueprints can be built at a factory. "
				"</html>"));
	connect(m_CanView, SIGNAL(clicked()), SLOT(EnableControls()));

	m_CanMfg = new QRadioButton(tr("Blueprint can be manufactured."), this);
	m_CanMfg->setToolTip(tr("<html>"
				"Only <i>Final</i> blueprints can be built at a factory. "
				"The blueprint cannot be opened in a design studio. "
				"<br>"
				"This control is disabled if the selected blueprint is an assembly. "
				"</html>"));
	connect(m_CanMfg, SIGNAL(clicked()), SLOT(EnableControls()));

	m_UseOnce = new QCheckBox(tr("Use Once"), this);
	m_UseOnce->setToolTip(tr("<html>"
				"The media can be used at a factory only once. "
				"It can be used to purchase a company spacecraft or it can be used to commission a fleet spacecraft. "
				"After one use, the blueprint is destroyed. "
				"<br>"
				"This control is disabled if the selected blueprint is an assembly. "
				"</html>"));

	m_MinStance = new QComboBox(this);
	m_MinStance->addItem(tr("All Access. All empires can access this media."), POL_Enemy);
	m_MinStance->addItem(tr("Neutral Access. Enemy empires may not access this media."), POL_Neutral);
	m_MinStance->addItem(tr("Friend Access. Enemy and neutral empires may not access this media."), POL_Friend);
	m_MinStance->addItem(tr("Empire Access. Only the publisher's empire may access this media."), POL_Empire);
	m_MinStance->setToolTip(tr("<html>"
				"Usage of this media can be limited based upon the political stance of your empire toward the holder's empire. "
				"This restriction is preserved with the media and survives transfer of the media to another avatar. "
				"e.g. If enemy is excluded, no enemy empire may use the media to buy or commission a spacecraft. "
				"<br>"
				"Your current empire is stored on the media. "
				"<b>If you change empires, empire restrictions remain in the context of the stored empire.</b>"
				"</html>"));

	m_GearMedia = new QComboBox(this);
	m_GearMedia->setToolTip(tr("<html>Shows media in your gear that can be used to store a blueprint. </html>"));
	connect(m_GearMedia, SIGNAL(currentIndexChanged(int)), SLOT(GearMediaChanged(int)));

	m_Ok = new QPushButton(tr("Save Blueprint"), this);
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	m_Ok->setEnabled(false);
	m_Ok->setToolTip(tr("<html>"
				"Saves the blueprint with the terms specified onto the selected media. "
				"</html>"));
	connect(m_Ok, SIGNAL(clicked()), SLOT(SaveDesign()));

	// Put it together.
	QHBoxLayout* rdlay = new QHBoxLayout;
	rdlay->addWidget(m_RequestDesigns);
	rdlay->addStretch();
	rdlay->setMargin(0);
	rdlay->setSpacing(2);

	QVBoxLayout* liblay = new QVBoxLayout;
	liblay->addLayout(rdlay);
	liblay->addWidget(m_Designs);
	liblay->setMargin(0);
	liblay->setSpacing(2);

	QVBoxLayout* rlay = new QVBoxLayout;
	rlay->addWidget(m_DesignThumb);
	rlay->addWidget(m_DesignReport);
	rlay->setMargin(0);
	rlay->setSpacing(2);

	QWidget* lwid = new QWidget;
	lwid->setLayout(liblay);

	QWidget* rwid = new QWidget;
	rwid->setLayout(rlay);

	m_Splitter = new QSplitter;
	m_Splitter->addWidget(lwid);
	m_Splitter->addWidget(rwid);
	m_Splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	QHBoxLayout* didlay = new QHBoxLayout;
	didlay->addWidget(m_DesignId);
	didlay->addWidget(m_DesignName, 10);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(m_GearMedia, 1);
	blay->addWidget(m_Ok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_Splitter, 10);
	lay->addLayout(didlay);
	lay->addWidget(m_CanCopy);
	lay->addWidget(m_CanView);
	lay->addWidget(m_CanMfg);
	lay->addWidget(m_MinStance);
	lay->addWidget(m_UseOnce);
	lay->addLayout(blay);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	QByteArray splitterstate;
	g_Au.Settings().readEntry("Widget/DesignerGearSave/Splitter", splitterstate);
	m_Splitter->restoreState(splitterstate);
}

AuDesignerGearSave::~AuDesignerGearSave()
{
}

void AuDesignerGearSave::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
		ListMedia();
		EnableControls();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuDesignerGearSave::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/DesignerGearSave/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/DesignerGearSave/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/DesignerGearSave/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/DesignerGearSave/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuDesignerGearSave::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/DesignerGearSave/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/DesignerGearSave/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/DesignerGearSave/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/DesignerGearSave/SizeY",sz.height());
}

quint32 AuDesignerGearSave::DesignId() const
{
	return m_DesignId->text().toUInt();
}

QString AuDesignerGearSave::DesignName() const
{
	return m_DesignName->text().trimmed();
}

void AuDesignerGearSave::SetDesign(quint32 id, const QString& name, const QString& report)
{
	if(id)
		m_DesignId->setText(QVariant(id).toString());
	else
		m_DesignId->setText("");

	m_DesignName->setText(name.trimmed());

	m_DesignReport->setHtml(report);

	// See if we have a thumb of the design available.
	if(id)
	{
		QHash<quint32, AuScDesignListEntry>& designs = g_AuClient.LockScDesignList();
		QHash<quint32, AuScDesignListEntry>::const_iterator di = designs.find(id);
		if(di != designs.end())
			m_DesignThumb->setPixmap(QPixmap::fromImage((*di).Thumb()));
		g_AuClient.UnlockScDesignList();
	}

	m_Designs->blockSignals(true);
	m_Designs->SetCurrent(id);
	m_Designs->blockSignals(false);

	m_Ok->setEnabled(IsNameValid(name.trimmed()));
}

void AuDesignerGearSave::RequestDesigns()
{
	auApp->ScLibraryRequestDesigns();
}

void AuDesignerGearSave::DesignSelectionChanged()
{
	quint32 designId = m_Designs->CurrentDesignId();
	QString designName = m_Designs->CurrentDesignName();
	QString designReport = m_Designs->CurrentDesignReport();

	SetDesign(designId, designName, designReport);
	EnableControls();
}

void AuDesignerGearSave::SplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/DesignerGearSave/Splitter", m_Splitter->saveState());
}

void AuDesignerGearSave::RefreshThumb()
{
	quint32 designId = DesignId();
	if(designId)
	{
		QHash<quint32, AuScDesignListEntry>& designs = g_AuClient.LockScDesignList();
		QHash<quint32, AuScDesignListEntry>::const_iterator di = designs.find(designId);
		if(di != designs.end())
			m_DesignThumb->setPixmap(QPixmap::fromImage((*di).Thumb()));
		g_AuClient.UnlockScDesignList();
	}
}

void AuDesignerGearSave::SaveDesign()
{
	if(!m_Media.IsValid())
		return;

	quint32 designId = DesignId();
	if(!designId)
		return;

	QString smsg = tr("<html>"
			"Confirm save of spacecraft blueprint <b>%1 %2</b> onto the <b>%3</b>. "
			"</html>").arg(designId).arg(DesignName()).arg(m_MediaName);
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	accept();
}

void AuDesignerGearSave::EnableControls()
{
	bool canMake = m_Designs->CurrentDesignState() == AuDesign::Final;

	m_CanMfg->setEnabled(canMake);

	m_UseOnce->setEnabled(canMake);

	bool enableOk = true;

	quint32 designId = DesignId();
	if(!designId)
		enableOk = false;
	else if(m_Designs->CurrentDesignState() != AuDesign::Final && m_CanMfg->isChecked())
		enableOk = false;

	if(!m_Media.IsValid())
		enableOk = false;

	m_Ok->setEnabled(enableOk);
}

bool AuDesignerGearSave::CanCopy() const
{
	return m_CanCopy->isChecked();
}

bool AuDesignerGearSave::CanView() const
{
	return m_CanCopy->isChecked() || m_CanView->isChecked();
}

POLITICAL_STANCE AuDesignerGearSave::MinStance() const
{
	return (POLITICAL_STANCE)m_MinStance->itemData(m_MinStance->currentIndex()).toInt();
}

bool AuDesignerGearSave::UseOnce() const
{
	return m_UseOnce->isChecked();
}

void AuDesignerGearSave::ListMedia()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	m_GearMedia->blockSignals(true);

	DaPerson* sceneAtom;
	view->LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		// Add designs in my possession.
		const AuGear* inv = sceneAtom->LockGearForRead();
		AuGearPosList allGearPos;
		inv->ListAll(allGearPos);
		for(AuGearPosList::const_iterator gi = allGearPos.cbegin(); gi != allGearPos.cend(); gi++)
		{
			const AuItem* gitem = inv->ItemAt(*gi);
			if(gitem->CommodityId() == CID_BlankDisk)
			{
				m_GearMedia->addItem(QIcon(gitem->BitmapName()), gitem->ListDesc(), QVariant::fromValue(*gi));
				continue;
			}
			else if(gitem->CommodityId() == CID_Paper && gitem->Inscription().isEmpty())
			{
				m_GearMedia->addItem(QIcon(gitem->BitmapName()), gitem->ListDesc(), QVariant::fromValue(*gi));
				continue;
			}
			const AuItemStorageMedia* media = dynamic_cast<const AuItemStorageMedia*>(gitem);
			if(media)
			{
				if(media->StorageSpaceAvailable())
					m_GearMedia->addItem(QIcon(gitem->BitmapName()), gitem->ListDesc(), QVariant::fromValue(*gi));
				continue;
			}
		}
		sceneAtom->UnlockGear();
	}
	view->UnlockSceneAtom();

	m_GearMedia->setCurrentIndex(-1);
	m_GearMedia->blockSignals(false);
}

void AuDesignerGearSave::GearMediaChanged(int index)
{
	m_Media = m_GearMedia->itemData(index).value<AuGearPos>();
	m_MediaName = m_GearMedia->itemText(index);
	EnableControls();
}
