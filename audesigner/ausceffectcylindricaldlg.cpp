//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausceffectcylindricaldlg.cpp,v 1.5 2022/02/19 18:25:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausceffectcylindricaldlg.h"
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

AuScEffectCylindricalDlg::AuScEffectCylindricalDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Cylindrical Effect Texture"));
	setAuDlgIcon(QIcon(":data/ScUnwrapCylinder.png"));

	QLabel* explain = new QLabel(tr("<html>"
				"<p>"
				"Choose an effect texture to cylindrically wrap around the selected faces. "
				"</p>"
				"</html"), this);
	explain->setWordWrap(true);

	int lastReps = g_Au.Settings().readNumEntry("Widget/EffectCylindricalDlg/TextureReps", 10);

	double lastSize = g_Au.Settings().readNumEntry("Widget/EffectCylindricalDlg/TextureSize", 2.);

	int lastIdx = g_Au.Settings().readNumEntry("Widget/EffectCylindricalDlg/Texture", 0);
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

	QLabel* lbreps = new QLabel(tr("Number of times the texture repeats around the cylinder. "), this);

	m_TextureReps = new QLineEdit(QVariant(lastReps).toString(), this);

	QLabel* lbsize = new QLabel(tr("Size of texture, in units depending upon your <i>unit of measure</i> setting. "), this);

	m_TextureSize = new QLineEdit(QVariant(lastSize).toString(), this);

	QPushButton* okbutton = new QPushButton(tr("Ok"), this);
	okbutton->setToolTip(tr("<html>"
				"Cylindrically wraps the selected effect texture around the selected faces. "
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

	QHBoxLayout* rlay = new QHBoxLayout;
	rlay->addWidget(lbreps);
	rlay->addWidget(m_TextureReps);
	rlay->setMargin(0);
	rlay->setSpacing(2);

	QHBoxLayout* slay = new QHBoxLayout;
	slay->addWidget(lbsize);
	slay->addWidget(m_TextureSize);
	slay->setMargin(0);
	slay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(explain);
	lay->addLayout(clay);
	lay->addLayout(slay);
	lay->addLayout(rlay);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScEffectCylindricalDlg::~AuScEffectCylindricalDlg()
{
}

void AuScEffectCylindricalDlg::setVisible(bool visible)
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

void AuScEffectCylindricalDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/EffectCylindricalDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/EffectCylindricalDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/EffectCylindricalDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/EffectCylindricalDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScEffectCylindricalDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/EffectCylindricalDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/EffectCylindricalDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/EffectCylindricalDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/EffectCylindricalDlg/SizeY",sz.height());
}

SURFACE_EFFECT AuScEffectCylindricalDlg::Effect() const
{
	QListWidgetItem* curItem = m_TextureList->currentItem();
	if(!curItem)
		return EFF_Count;

	SURFACE_EFFECT effect = (SURFACE_EFFECT)curItem->data(Qt::UserRole).toUInt();
	if(effect >= EFF_Count)
		return EFF_Count;

	return effect;
}

void AuScEffectCylindricalDlg::OkClicked()
{
	SURFACE_EFFECT curIdx = Effect();
	if(curIdx < EFF_Count)
		g_Au.Settings().writeEntry("Widget/EffectCylindricalDlg/Texture", curIdx);

	g_Au.Settings().writeEntry("Widget/EffectCylindricalDlg/TextureReps", TextureReps());

	double texSize = m_TextureSize->text().toDouble();
	g_Au.Settings().writeEntry("Widget/EffectCylindricalDlg/TextureSize", texSize);

	accept();
}

void AuScEffectCylindricalDlg::ListEffects(int curIdx)
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

void AuScEffectCylindricalDlg::TextureChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
	int curIdx;
	if(current)
		curIdx = current->data(Qt::UserRole).toUInt();
	else
		curIdx = EFF_Count;

	TextureSample(curIdx);
}

void AuScEffectCylindricalDlg::TextureSample(int effect)
{
	if(effect < 0 || effect >= EFF_Count)
	{
		m_TextureSample->setPixmap(QPixmap());
		return;
	}

	QPixmap imgTile(AuTextureLibItem::EffectTextureItem((SURFACE_EFFECT)effect).FileName());
	imgTile = imgTile.scaled(256,256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	m_TextureSample->setPixmap(imgTile);
}

quint16 AuScEffectCylindricalDlg::TextureReps() const
{
	quint16 texReps = m_TextureReps->text().toUInt();
	return texReps;
}

double AuScEffectCylindricalDlg::TextureSize() const
{
	double texSize = m_TextureSize->text().toDouble();
	return texSize * g_AuClient.DesignerUnitScale();
}
