//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscpalettedlg.cpp,v 1.8 2022/02/19 18:25:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscpalettedlg.h"
//
#include "auapp.h"
#include "audesign.h"
#include "auglobal.h"
#include "auframe.h"
#include "ausctexturechoose.h"
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
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

AuScPaletteDlg::AuScPaletteDlg(COMMODITYID hullModule, const AuDesignTex palette[SCTEXI_Count], QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Texture Palette"));
	setAuDlgIcon(QIcon(":data/ScTexPalette.png"));

	m_HullModule = hullModule;
	for(int t=0; t<SCTEXI_Count; t++)
		m_Palette[t] = palette[t];
	m_PaletteChanged = false;

	QLabel* explain = new QLabel(tr("<html>"
				"<p>"
				"Changes to a texture palette item will affect all existing faces that refer to that texture palette item. "
				"</p>"
				"</html"), this);
	explain->setWordWrap(true);

	int lastIdx = g_Au.Settings().readNumEntry("Widget/PaletteDlg/Texture", 0);
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

	QPushButton* chooseImg = new QPushButton(tr("Choose Image..."), this);
	chooseImg->setToolTip(tr("<html>"
				"Displays a window to choose an image file for the palette entry. "
				"</html>"));
	connect(chooseImg, SIGNAL(clicked()), SLOT(ChooseCustomTexture()));

	QPushButton* chooseLib = new QPushButton(tr("Choose Library Texture..."), this);
	chooseLib->setToolTip(tr("<html>"
				"Displays a window to choose a built-in texture for the palette entry. "
				"</html>"));
	connect(chooseLib, SIGNAL(clicked()), SLOT(ChooseLibraryTexture()));

	QPushButton* okbutton = new QPushButton(tr("Ok"), this);
	okbutton->setToolTip(tr("<html>"
				"Accepts changes to the design's texture palette. "
				"</html>"));
	connect(okbutton, SIGNAL(clicked()), SLOT(OkClicked()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(chooseImg);
	blay->addWidget(chooseLib);
	blay->addWidget(okbutton);
	blay->setMargin(0);
	blay->setSpacing(2);

	QHBoxLayout* clay = new QHBoxLayout;
	clay->addWidget(m_TextureList);
	clay->addWidget(m_TextureSample);
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

AuScPaletteDlg::~AuScPaletteDlg()
{
}

void AuScPaletteDlg::setVisible(bool visible)
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

void AuScPaletteDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/PaletteDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/PaletteDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/PaletteDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/PaletteDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScPaletteDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/PaletteDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/PaletteDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/PaletteDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/PaletteDlg/SizeY",sz.height());
}

SC_TEXTURE_INDEX AuScPaletteDlg::CurrentTextureIndex() const
{
	QListWidgetItem* curItem = m_TextureList->currentItem();
	if(!curItem)
		return SCTEXI_Count;

	SC_TEXTURE_INDEX texIdx = (SC_TEXTURE_INDEX)curItem->data(Qt::UserRole).toUInt();
	if(texIdx >= SCTEXI_Count)
		return SCTEXI_Count;

	return texIdx;
}

void AuScPaletteDlg::OkClicked()
{
	SC_TEXTURE_INDEX curIdx = CurrentTextureIndex();
	if(curIdx < SCTEXI_Count)
		g_Au.Settings().writeEntry("Widget/PaletteDlg/Texture", curIdx);

	accept();
}

void AuScPaletteDlg::ListTextures(int curIdx)
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

void AuScPaletteDlg::TextureChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
	int curIdx;
	if(current)
		curIdx = current->data(Qt::UserRole).toUInt();
	else
		curIdx = SCTEXI_Count;

	TextureSample(curIdx);
}

void AuScPaletteDlg::TextureSample(int texIdx)
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

// Load a picture from a file.
void AuScPaletteDlg::ChooseCustomTexture()
{
	SC_TEXTURE_INDEX curIdx = CurrentTextureIndex();
	if(curIdx == SCTEXI_Material)
	{
		// Tell them what is going to happen.
		QMessageBox::information(this, g_Au.ApplicationName(), tr("<html>"
					"<p>"
					"The <i>material</i> texture can be overridden using a texture image. "
					"When a custom texture image is selected, the material texture will no longer change based on the "
					"manufacturing material. "
					"</p>"
					"</html>"),
				QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
	}

	// Load last known file folder.
	QString lastFile = g_Au.Settings().readEntry("/Settings/LastTextureFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();
	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and use the last folder.

	QString file = QFileDialog::getOpenFileName(this, tr("Select Image"), lastFile, "", 0,
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(!file.isEmpty())
	{
		// Store last known file folder.
		g_Au.Settings().writeEntry("/Settings/LastTextureFile", file);

		QImage imTex;
		if(imTex.load(file))
		{
			QString name = QFileInfo(file).fileName();
			QImage nonScaledImg = imTex;
			if(imTex.width() > AuDesignTex::MaxImageDimension() || imTex.height() > AuDesignTex::MaxImageDimension())
			{
				QMessageBox::warning(this, g_Au.ApplicationName(), tr("<html>"
							"The maximum image dimension is %1 pixels. "
							"The image will be resized to fit within this limit. "
							"</html>").arg(AuDesignTex::MaxImageDimension()),
						QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
				QImage custom = imTex.scaled(AuDesignTex::MaxImageDimension(),AuDesignTex::MaxImageDimension(), Qt::KeepAspectRatio,
						Qt::SmoothTransformation);
				m_Palette[curIdx].SetImage(name, custom);
				m_PaletteChanged = true;

				QListWidgetItem* curItem = m_TextureList->currentItem();
				if(curItem)
				{
					int iconSize = 64;
					QPixmap imgSmall = QPixmap::fromImage(imTex.scaled(iconSize,iconSize, Qt::IgnoreAspectRatio,
								Qt::SmoothTransformation));
					curItem->setIcon(QIcon(imgSmall));
					curItem->setText(name);
				}

				TextureSample(curIdx);
			}
			else
			{
				if(imTex.width() < 1 || imTex.height() < 1)
				{
					QMessageBox::critical(this, g_Au.ApplicationName(), tr("<html>"
								"The image is too small. "
								"Please select an image that is at least 1 pixel in width and height."),
							QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
				}
				else
				{
					m_Palette[curIdx].SetImage(name, imTex);
					m_PaletteChanged = true;

					QListWidgetItem* curItem = m_TextureList->currentItem();
					if(curItem)
					{
						int iconSize = 64;
						QPixmap imgSmall = QPixmap::fromImage(imTex.scaled(iconSize,iconSize, Qt::IgnoreAspectRatio,
									Qt::SmoothTransformation));
						curItem->setIcon(QIcon(imgSmall));
						curItem->setText(name);
					}

					TextureSample(curIdx);
				}
			}
		}
		else
		{
			QMessageBox::critical(this, g_Au.ApplicationName(), tr("<html>"
						"The image could not be loaded. "
						"Many image formats are supported but some formats give us trouble. "
						"If the image can be loaded with a paint program, "
						"try saving it as a different format such as PNG or BMP."
						"</html>"),
					QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		}
	}
}

void AuScPaletteDlg::ChooseLibraryTexture()
{
	SC_TEXTURE_INDEX curIdx = CurrentTextureIndex();
	switch(curIdx)
	{
	case SCTEXI_Material:
		// Tell them why this one cannot be configured.
		{
			int result = QMessageBox::information(this, g_Au.ApplicationName(), tr("<html>"
					"<p>"
					"The <i>material</i> texture cannot be changed to use a texture from the library. "
					"It changes based on the manufacturing material. "
					"Material is configured in the design properties. "
					"</p>"
					"<p>"
					"Press 'Ok' to remove any custom texture that was loaded into the palette entry. "
					"The material entry will then resume texturing based on manufacturing material. "
					"</p>"
					"</html>"),
				QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton);
			if(result != QMessageBox::Ok)
				return;
		}
		break;
	case SCTEXI_User0:
	case SCTEXI_User1:
	case SCTEXI_User2:
	case SCTEXI_User3:
	case SCTEXI_User4:
	case SCTEXI_User5:
	case SCTEXI_User6:
	case SCTEXI_User7:
	case SCTEXI_User8:
	case SCTEXI_User9:
	case SCTEXI_User10:
	case SCTEXI_User11:
	case SCTEXI_User12:
	case SCTEXI_User13:
		{
			AuScTextureChoose dlg(m_Palette[curIdx].TexLibId(), this);
			if(dlg.exec() == QDialog::Accepted)
			{
				quint16 libTexId = dlg.Texture();

				m_Palette[curIdx].SetTexLibId(dlg.Texture());
				m_PaletteChanged = true;

				QListWidgetItem* curItem = m_TextureList->currentItem();
				if(curItem)
				{
					int iconSize = 64;
					QImage imTex(AuTextureLibItem::LibraryTextureFile(libTexId));
					QPixmap imgSmall = QPixmap::fromImage(imTex.scaled(iconSize,iconSize, Qt::IgnoreAspectRatio,
								Qt::SmoothTransformation));
					curItem->setIcon(QIcon(imgSmall));

					QString name(AuTextureLibItem::LibraryTextureName(libTexId));
					QString itemName = QString("%1. %2").arg(curIdx+1).arg(name);
					curItem->setText(itemName);
				}
				TextureSample(curIdx);
			}
		}
		break;
	case SCTEXI_Count:
		break;
	}
}
