//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausctextureprojectdlg.cpp,v 1.7 2022/02/19 18:26:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausctextureprojectdlg.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "audesign.h"
#include "auframe.h"
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

AuScTextureProjectDlg::AuScTextureProjectDlg(COMMODITYID hullModule, const AuDesignTex palette[SCTEXI_Count], QWidget* parent) :
	AuDialog(parent)
{
	setAuDlgTitle(tr("Project Texture"));
	setAuDlgIcon(QIcon(":data/ScUnwrapProject.png"));

	m_HullModule = hullModule;
	for(int t=0; t<SCTEXI_Count; t++)
		m_Palette[t] = palette[t];

	QLabel* explain = new QLabel(tr("<html>"
				"<p>"
				"Choose a palette texture to project onto the selected faces. "
				"</p>"
				"</html"), this);
	explain->setWordWrap(true);

	double lastSize = g_Au.Settings().readNumEntry("Widget/TextureProjectDlg/TextureSize", 2.);

	int lastIdx = g_Au.Settings().readNumEntry("Widget/TextureProjectDlg/Texture", 0);
	if(lastIdx >= SCTEXI_Count)
		lastIdx = 0;

	m_TextureList = new QListWidget(this);
	m_TextureList->setSpacing(2);
	ListTextures(lastIdx);
	connect(m_TextureList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
			SLOT(TextureChanged(QListWidgetItem*, QListWidgetItem*)));

	m_TextureSample = new QLabel(this);
	m_TextureSample->setMinimumSize(256,256);
	TextureSample(lastIdx);

	QLabel* lbsize = new QLabel(tr("Size of texture, in units depending upon your <i>unit of measure</i> setting. "), this);

	m_TextureSize = new QLineEdit(QVariant(lastSize).toString(), this);

	QPushButton* okbutton = new QPushButton(tr("Ok"), this);
	okbutton->setToolTip(tr("<html>"
				"Projects the selected palette texture onto the selected faces. "
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

AuScTextureProjectDlg::~AuScTextureProjectDlg()
{
}

void AuScTextureProjectDlg::setVisible(bool visible)
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

void AuScTextureProjectDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/TextureProjectDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/TextureProjectDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/TextureProjectDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/TextureProjectDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScTextureProjectDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/TextureProjectDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/TextureProjectDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/TextureProjectDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/TextureProjectDlg/SizeY",sz.height());
}

SC_TEXTURE_INDEX AuScTextureProjectDlg::TextureIndex() const
{
	QListWidgetItem* curItem = m_TextureList->currentItem();
	if(!curItem)
		return SCTEXI_Count;

	SC_TEXTURE_INDEX texIdx = (SC_TEXTURE_INDEX)curItem->data(Qt::UserRole).toUInt();
	if(texIdx >= SCTEXI_Count)
		return SCTEXI_Count;

	return texIdx;
}

void AuScTextureProjectDlg::OkClicked()
{
	SC_TEXTURE_INDEX curIdx = TextureIndex();
	if(curIdx < SCTEXI_Count)
		g_Au.Settings().writeEntry("Widget/TextureProjectDlg/Texture", curIdx);

	double texSize = m_TextureSize->text().toDouble();
	g_Au.Settings().writeEntry("Widget/TextureProjectDlg/TextureSize", texSize);

	accept();
}

void AuScTextureProjectDlg::ListTextures(int curIdx)
{
	QListWidgetItem* curItem = 0;

	Q_ASSERT(m_TextureList);
	m_TextureList->clear();
	int iconSize = 64;
	for(int texIdx=0; texIdx<SCTEXI_Count; texIdx++)
	{
		QPixmap imgTile;
		QString name;
		if(!m_Palette[texIdx].Image().isNull())
		{
			name = m_Palette[texIdx].ImageName();
			imgTile = QPixmap::fromImage(m_Palette[texIdx].Image());
		}
		else if(texIdx == SCTEXI_Material)
		{
			name = QObject::tr("Material");
			switch(m_HullModule)
			{
			default:
			case CID_WoodHullModule:
				imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullWood()));
				break;
			case CID_LumberHullModule:
				imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullLumber()));
				break;
			case CID_StoneHullModule:
				imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullStone()));
				break;
			case CID_MetalHullModule:
				imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullMetal()));
				break;
			case CID_MagmiumHullModule:
				imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullMagmium()));
				break;
			case CID_VulciumHullModule:
				imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullVulcium()));
				break;
			case CID_AdamantineHullModule:
				imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullAdamantine()));
				break;
			}
		}
		else
		{
			name = AuTextureLibItem::LibraryTextureName(m_Palette[texIdx].TexLibId());
			imgTile.load(AuTextureLibItem::LibraryTextureFile(m_Palette[texIdx].TexLibId()));
		}

		AuListWidgetSortedItem* item = new AuListWidgetSortedItem(texIdx);
		item->setData(Qt::UserRole, texIdx);

		QString itemName = QString("%1. %2").arg(texIdx+1).arg(name);
		item->setText(itemName);

		if(!imgTile.isNull())
		{
			QPixmap imgSmall = imgTile.scaled(iconSize,iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			item->setIcon(QIcon(imgSmall));
		}

		m_TextureList->addItem(item);

		if(texIdx == curIdx)
			curItem = item;
	}

	m_TextureList->sortItems();

	if(curItem)
		m_TextureList->setCurrentItem(curItem);
}

void AuScTextureProjectDlg::TextureChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
	int curIdx;
	if(current)
		curIdx = current->data(Qt::UserRole).toUInt();
	else
		curIdx = SCTEXI_Count;

	TextureSample(curIdx);
}

void AuScTextureProjectDlg::TextureSample(int texIdx)
{
	if(texIdx < 0 || texIdx >= SCTEXI_Count)
	{
		m_TextureSample->setPixmap(QPixmap());
		return;
	}

	QPixmap imgTile;
	if(!m_Palette[texIdx].Image().isNull())
		imgTile = QPixmap::fromImage(m_Palette[texIdx].Image());
	else if(texIdx == SCTEXI_Material)
	{
		switch(m_HullModule)
		{
		default:
		case CID_WoodHullModule:
			imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullWood()));
			break;
		case CID_LumberHullModule:
			imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullLumber()));
			break;
		case CID_StoneHullModule:
			imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullStone()));
			break;
		case CID_MetalHullModule:
			imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullMetal()));
			break;
		case CID_MagmiumHullModule:
			imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullMagmium()));
			break;
		case CID_VulciumHullModule:
			imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullVulcium()));
			break;
		case CID_AdamantineHullModule:
			imgTile.load(AuTextureLibItem::LibraryTextureFile(AuTextureLibItem::LibraryTextureHullAdamantine()));
			break;
		}
	}
	else
	{
		imgTile.load(AuTextureLibItem::LibraryTextureFile(m_Palette[texIdx].TexLibId()));
		imgTile = imgTile.scaled(AuDesignTex::MaxImageDimension(),AuDesignTex::MaxImageDimension(), Qt::KeepAspectRatio,
				Qt::SmoothTransformation);
	}

	m_TextureSample->setPixmap(imgTile);
}

double AuScTextureProjectDlg::TextureSize() const
{
	double texSize = m_TextureSize->text().toDouble();
	return texSize * g_AuClient.DesignerUnitScale();
}
