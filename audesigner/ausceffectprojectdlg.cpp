//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausceffectprojectdlg.cpp,v 1.5 2022/02/19 18:25:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausceffectprojectdlg.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "audesign.h"
#include "auframe.h"
#include "auscface.h"
#include "ausctexturechoose.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>

class AuListWidgetSortedItem : public QListWidgetItem
{
public:
	inline AuListWidgetSortedItem(int sortOrder)
	{
		setData(Qt::UserRole+10, sortOrder);
	}

	inline bool operator<(const QListWidgetItem& other) const
	{
		int mySortOrder = QVariant(data(Qt::UserRole+10)).toInt();
		int otherSortOrder = QVariant(other.data(Qt::UserRole+10)).toInt();
		return (mySortOrder < otherSortOrder) ? true : false;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScEffectProjectDlg::AuScEffectProjectDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Project Effect Texture"));
	setAuDlgIcon(QIcon(":data/ScUnwrapProject.png"));

	QLabel* explain = new QLabel(tr("<html>"
				"<p>"
				"Choose an effect texture to project onto the selected faces. "
				"</p>"
				"</html"), this);
	explain->setWordWrap(true);

	double lastSize = g_Au.Settings().readNumEntry("Widget/EffectProjectDlg/TextureSize", 2.);

	int lastIdx = g_Au.Settings().readNumEntry("Widget/EffectProjectDlg/Texture", 0);
	if(lastIdx >= EFF_Count)
		lastIdx = 0;

	m_TextureList = new QListWidget(this);
	m_TextureList->setSpacing(2);
	ListEffects(lastIdx);
	connect(m_TextureList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
			SLOT(TextureChanged(QListWidgetItem*, QListWidgetItem*)));

	m_TextureSample = new QLabel(this);
	m_TextureSample->setMinimumSize(256,256);
	TextureSample(lastIdx);

	QLabel* lbsize = new QLabel(tr("Size of texture, in units depending upon your <i>unit of measure</i> setting. "), this);

	m_TextureSize = new QLineEdit(QVariant(lastSize).toString(), this);

	QPushButton* okbutton = new QPushButton(tr("Ok"), this);
	okbutton->setToolTip(tr("<html>"
				"Projects the selected effect texture onto the selected faces. "
				"</html>"));
	connect(okbutton, SIGNAL(clicked()), SLOT(OkClicked()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(okbutton);
	blay->setMargin(0);
	blay->setSpacing(2);

	QHBoxLayout* clay = new QHBoxLayout;
	clay->addWidget(m_TextureList);
	clay->addWidget(m_TextureSample);
	clay->setMargin(0);
	clay->setSpacing(2);

	QHBoxLayout* slay = new QHBoxLayout;
	slay->addWidget(lbsize);
	slay->addWidget(m_TextureSize);
	slay->setMargin(0);
	slay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(explain);
	lay->addLayout(clay);
	lay->addLayout(slay);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScEffectProjectDlg::~AuScEffectProjectDlg()
{
}

void AuScEffectProjectDlg::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuScEffectProjectDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/EffectProjectDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/EffectProjectDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/EffectProjectDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/EffectProjectDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScEffectProjectDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/EffectProjectDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/EffectProjectDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/EffectProjectDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/EffectProjectDlg/SizeY",sz.height());
}

SURFACE_EFFECT AuScEffectProjectDlg::Effect() const
{
	QListWidgetItem* curItem = m_TextureList->currentItem();
	if(!curItem)
		return EFF_Count;

	SURFACE_EFFECT effect = (SURFACE_EFFECT)curItem->data(Qt::UserRole).toUInt();
	if(effect >= EFF_Count)
		return EFF_Count;

	return effect;
}

void AuScEffectProjectDlg::OkClicked()
{
	SURFACE_EFFECT curIdx = Effect();
	if(curIdx < EFF_Count)
		g_Au.Settings().writeEntry("Widget/EffectProjectDlg/Texture", curIdx);

	double texSize = m_TextureSize->text().toDouble();
	g_Au.Settings().writeEntry("Widget/EffectProjectDlg/TextureSize", texSize);

	accept();
}

void AuScEffectProjectDlg::ListEffects(int curIdx)
{
	QListWidgetItem* curItem = 0;

	Q_ASSERT(m_TextureList);
	m_TextureList->clear();
	int iconSize = 64;
	for(int effect=1; effect<EFF_Count; effect++)
	{
		if(SurfaceEffectSort((SURFACE_EFFECT)effect) == 0)
			continue;
		QString itemName = SurfaceEffectName((SURFACE_EFFECT)effect);
		QPixmap imgTile(AuTextureLibItem::EffectTextureItem((SURFACE_EFFECT)effect).FileName());

		AuListWidgetSortedItem* item = new AuListWidgetSortedItem(SurfaceEffectSort((SURFACE_EFFECT)effect));
		item->setData(Qt::UserRole, effect);

		item->setText(itemName);

		if(!imgTile.isNull())
		{
			QPixmap imgSmall = imgTile.scaled(iconSize,iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			item->setIcon(QIcon(imgSmall));
		}

		m_TextureList->addItem(item);

		if(effect == curIdx)
			curItem = item;
	}

	m_TextureList->sortItems();

	if(curItem)
		m_TextureList->setCurrentItem(curItem);
}

void AuScEffectProjectDlg::TextureChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
	int curIdx;
	if(current)
		curIdx = current->data(Qt::UserRole).toUInt();
	else
		curIdx = EFF_Count;

	TextureSample(curIdx);
}

void AuScEffectProjectDlg::TextureSample(int effect)
{
	if(effect <= EFF_None || effect >= EFF_Count)
	{
		m_TextureSample->setPixmap(QPixmap());
		return;
	}

	QPixmap imgTile(AuTextureLibItem::EffectTextureItem((SURFACE_EFFECT)effect).FileName());
	imgTile = imgTile.scaled(256,256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	m_TextureSample->setPixmap(imgTile);
}

double AuScEffectProjectDlg::TextureSize() const
{
	double texSize = m_TextureSize->text().toDouble();
	return texSize * g_AuClient.DesignerUnitScale();
}
