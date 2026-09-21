//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausctexturechoose.cpp,v 1.4 2022/02/19 18:26:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausctexturechoose.h"
//
#include "auapp.h"
#include "audesign.h"
#include "auglobal.h"
#include "auframe.h"
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

AuScTextureChoose::AuScTextureChoose(quint16 libTexId, QWidget* parent) : AuDialog(parent)
{
	if(libTexId >= AuTextureLibItem::LibraryTextureCount())
		libTexId = 0;

	setAuDlgTitle(tr("Choose Texture"));
	setAuDlgIcon(QIcon(":data/ScTexPalette.png"));

	QLabel* explain = new QLabel(tr("<html>"
				"<p>"
				"Texture names are for reference only. "
				"Textures may be used for any purpose. "
				"</p>"
				"</html"), this);
	explain->setWordWrap(true);

	m_TextureList = new QListWidget(this);
	m_TextureList->setSpacing(2);
	ListTextures(libTexId);
	connect(m_TextureList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
			SLOT(TextureChanged(QListWidgetItem*, QListWidgetItem*)));

	m_TextureSample = new QLabel(this);
	m_TextureSample->setMinimumSize(256,256);
	QPixmap imgTile(AuTextureLibItem::LibraryTextureFile(libTexId));
	m_TextureSample->setPixmap(imgTile);

	QPushButton* okbutton = new QPushButton(tr("Ok"), this);
	connect(okbutton, SIGNAL(clicked()), SLOT(OkClicked()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(okbutton);
	blay->setMargin(0);
	blay->setSpacing(2);

	QHBoxLayout* clay = new QHBoxLayout;
	clay->addWidget(m_TextureSample);
	clay->addWidget(m_TextureList);
	clay->setMargin(0);
	clay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(explain);
	lay->addLayout(clay);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScTextureChoose::~AuScTextureChoose()
{
}

void AuScTextureChoose::setVisible(bool visible)
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

void AuScTextureChoose::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/TextureChoose/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/TextureChoose/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/TextureChoose/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/TextureChoose/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScTextureChoose::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/TextureChoose/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/TextureChoose/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/TextureChoose/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/TextureChoose/SizeY",sz.height());
}

quint16 AuScTextureChoose::Texture() const
{
	QListWidgetItem* curItem = m_TextureList->currentItem();
	if(!curItem)
		return 0;

	quint16 libTexId = curItem->data(Qt::UserRole).toUInt();
	if(libTexId >= AuTextureLibItem::LibraryTextureCount())
		return 0;

	return libTexId;
}

void AuScTextureChoose::OkClicked()
{
	quint16 libTexId = Texture();
	if(!libTexId)
		return;

	g_Au.Settings().writeEntry("Widget/TextureChoose/Texture", libTexId);

	accept();
}

void AuScTextureChoose::ListTextures(int curItemId)
{
	QListWidgetItem* curItem = 0;

	Q_ASSERT(m_TextureList);
	m_TextureList->clear();
	int iconSize = 64;
	for(int sct=0; sct<AuTextureLibItem::LibraryTextureCount(); sct++)
	{
		const AuTextureLibItem& tli = AuTextureLibItem::LibraryTextureItem(sct);

		QPixmap imgTile(tli.FileName());
		if(imgTile.isNull())
		{
			qWarning() << "ERROR: AuScTextureChoose::ListTextures failed to load image" << tli.FileName();
			continue;
		}

		QPixmap imgSmall = imgTile.scaled(iconSize,iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

		QListWidgetItem* item = new QListWidgetItem;
		item->setText(tli.Name());
		item->setIcon(QIcon(imgSmall));
		item->setData(Qt::UserRole, tli.Id());
		m_TextureList->addItem(item);

		if(tli.Id() == curItemId)
			curItem = item;
	}

	m_TextureList->sortItems();

	if(curItem)
		m_TextureList->setCurrentItem(curItem);
}

void AuScTextureChoose::TextureChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
	if(!current)
	{
		m_TextureSample->setPixmap(QPixmap());
		return;
	}

	quint16 libTexId = current->data(Qt::UserRole).toUInt();
	if(libTexId >= AuTextureLibItem::LibraryTextureCount())
	{
		m_TextureSample->setPixmap(QPixmap());
		return;
	}

	QPixmap imgTile(AuTextureLibItem::LibraryTextureFile(libTexId));
	imgTile = imgTile.scaled(256,256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	m_TextureSample->setPixmap(imgTile);
}
