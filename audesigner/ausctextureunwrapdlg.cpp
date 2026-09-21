//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausctextureunwrapdlg.cpp,v 1.8 2022/02/19 18:26:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausctextureunwrapdlg.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "audesign.h"
#include "auframe.h"
#include "ausctexturechoose.h"
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>

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

AuScTextureUnwrapDlg::AuScTextureUnwrapDlg(SC_TEXTURE_INDEX currentIndex, COMMODITYID hullModule,
		const AuDesignTex palette[SCTEXI_Count], QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Unwrap Faces"));
	setAuDlgIcon(QIcon(":data/ScUnwrapVert.png"));

	if(currentIndex >= SCTEXI_Count)
		currentIndex = SCTEXI_Material;

	m_HullModule = hullModule;
	for(int t=0; t<SCTEXI_Count; t++)
		m_Palette[t] = palette[t];
	m_Method = AuScFaceList::UNWRAP_ByFace;

	// Read last settings.
	double lastSize = g_Au.Settings().readNumEntry("Widget/TextureUnwrapDlg/TextureSize", 2.);	// Stored in user units.
	float lastTol = g_Au.Settings().readNumEntry("Widget/TextureUnwrapDlg/AngleTol", 2.);		// Stored in degrees.
	if(lastTol < 0)
		lastTol = 0;
	bool lastAlign = g_Au.Settings().readBoolEntry("Widget/TextureUnwrapDlg/GridAlign", true);
	int lastAxis = g_Au.Settings().readNumEntry(QString("Widget/TextureUnwrapDlg/AlignAxis"), 1);

	// Build widgets.
	QLabel* explain = new QLabel(tr("<html>"
				"<p>"
				"Choose the palette texture that will be applied to the unwrapped faces. "
				"</p>"
				"</html"), this);
	explain->setWordWrap(true);

	m_TextureList = new QListWidget(this);
	m_TextureList->setSpacing(2);
	ListTextures(currentIndex);
	connect(m_TextureList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
			SLOT(TextureChanged(QListWidgetItem*, QListWidgetItem*)));

	m_TextureSample = new QLabel(this);
	m_TextureSample->setMinimumSize(256,256);
	TextureSample(currentIndex);

	QLabel* lbsize = new QLabel(tr("Size of texture, in units depending upon your <i>unit of measure</i> setting. "), this);

	m_TextureSize = new QLineEdit(QVariant(lastSize).toString(), this);

	QLabel* lbtol = new QLabel(tr("Angle tolerance when deciding if faces face the same direction, in degrees. "), this);

	m_AngleTol = new QLineEdit(QVariant(lastTol).toString(), this);

	m_GridAlign = new QCheckBox(tr("Align texture up direction to grid axis."), this);
	m_GridAlign->setChecked(lastAlign);

	m_XAxis = new QRadioButton(tr("X Axis"), this);
	m_XAxis->setChecked(lastAxis == 0);

	m_YAxis = new QRadioButton(tr("Y Axis"), this);
	m_YAxis->setChecked(lastAxis == 1);

	m_ZAxis = new QRadioButton(tr("Z Axis"), this);
	m_ZAxis->setChecked(lastAxis == 2);

	QPushButton* unwrapByFace = new QPushButton(tr("Unwrap by Face"), this);
	unwrapByFace->setToolTip(tr("<html>"
				"Unwraps the selected faces onto the texture that is chosen. "
				"Each face is unwrapped without considering other faces. "
				"</html>"));
	connect(unwrapByFace, SIGNAL(clicked()), SLOT(UnwrapByFace()));

	QPushButton* unwrapByFacing = new QPushButton(tr("Unwrap by Facing"), this);
	unwrapByFacing->setToolTip(tr("<html>"
				"Unwraps the selected faces onto the texture that is chosen. "
				"Faces that face the same direction are unwrapped together. "
				"Edges at turns are not wrapped. "
				"</html>"));
	connect(unwrapByFacing, SIGNAL(clicked()), SLOT(UnwrapByFacing()));

	QPushButton* unwrapEdges = new QPushButton(tr("Unwrap by Edges"), this);
	unwrapEdges->setToolTip(tr("<html>"
				"Unwraps the selected faces onto the texture that is chosen. "
				"Faces that face the same direction are unwrapped together. "
				"Edges at turns are wrapped. "
				"</html>"));
	connect(unwrapEdges, SIGNAL(clicked()), SLOT(UnwrapEdges()));

	QPushButton* unwrapHorizontal = new QPushButton(tr("Unwrap by Edges Horizontally"), this);
	unwrapHorizontal->setToolTip(tr("<html>"
				"Unwraps the selected faces onto the texture that is chosen. "
				"Faces that face the same direction are unwrapped together. "
				"Edges at turns are wrapped horizontally. "
				"</html>"));
	connect(unwrapHorizontal, SIGNAL(clicked()), SLOT(UnwrapHorizontal()));

	QPushButton* unwrapVertical = new QPushButton(tr("Unwrap by Edges Vertically"), this);
	unwrapVertical->setToolTip(tr("<html>"
				"Unwraps the selected faces onto the texture that is chosen. "
				"Faces that face the same direction are unwrapped together. "
				"Edges at turns are wrapped vertically. "
				"</html>"));
	connect(unwrapVertical, SIGNAL(clicked()), SLOT(UnwrapVertical()));

	QGridLayout* blay = new QGridLayout;
	blay->addWidget(unwrapByFace, 0,0, 1,2);
	blay->addWidget(unwrapByFacing, 0,2, 1,2);
	blay->addWidget(unwrapEdges, 0,4, 1,2);
	blay->addWidget(unwrapHorizontal, 1,0, 1,3);
	blay->addWidget(unwrapVertical, 1,3, 1,3);
	blay->setMargin(0);
	blay->setSpacing(2);

	QHBoxLayout* clay = new QHBoxLayout;
	clay->addWidget(m_TextureList);
	clay->addWidget(m_TextureSample);
	clay->setMargin(0);
	clay->setSpacing(2);

	QGridLayout* slay = new QGridLayout;
	int row=0;
	slay->addWidget(lbsize, row,0, 1,1);
	slay->addWidget(m_TextureSize, row,1, 1,1);
	row++;
	slay->addWidget(lbtol, row,0, 1,1);
	slay->addWidget(m_AngleTol, row,1, 1,1);
	row++;
	slay->addWidget(m_GridAlign, row,0, 1,2);
	row++;
	slay->addWidget(m_XAxis, row,0, 1,1);
	row++;
	slay->addWidget(m_YAxis, row,0, 1,1);
	row++;
	slay->addWidget(m_ZAxis, row,0, 1,1);
	row++;
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

AuScTextureUnwrapDlg::~AuScTextureUnwrapDlg()
{
}

void AuScTextureUnwrapDlg::setVisible(bool visible)
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

void AuScTextureUnwrapDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/TextureUnwrapDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/TextureUnwrapDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/TextureUnwrapDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/TextureUnwrapDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScTextureUnwrapDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/TextureUnwrapDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/TextureUnwrapDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/TextureUnwrapDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/TextureUnwrapDlg/SizeY",sz.height());
}

SC_TEXTURE_INDEX AuScTextureUnwrapDlg::TextureIndex() const
{
	QListWidgetItem* curItem = m_TextureList->currentItem();
	if(!curItem)
		return SCTEXI_Count;

	SC_TEXTURE_INDEX texIdx = (SC_TEXTURE_INDEX)curItem->data(Qt::UserRole).toUInt();
	if(texIdx >= SCTEXI_Count)
		return SCTEXI_Count;

	return texIdx;
}

void AuScTextureUnwrapDlg::ListTextures(int curIdx)
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

void AuScTextureUnwrapDlg::TextureChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
	int curIdx;
	if(current)
		curIdx = current->data(Qt::UserRole).toUInt();
	else
		curIdx = SCTEXI_Count;

	TextureSample(curIdx);
}

void AuScTextureUnwrapDlg::TextureSample(int texIdx)
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

bool AuScTextureUnwrapDlg::GridAlign() const
{
	return m_GridAlign->isChecked();
}

float AuScTextureUnwrapDlg::AngleTolerance() const
{
	float angleTol = m_AngleTol->text().toFloat();
	if(angleTol < 0)
		angleTol = 0;
	return angleTol * DEG2RADf;
}

double AuScTextureUnwrapDlg::TextureSize() const
{
	double texSize = m_TextureSize->text().toDouble();
	return texSize * g_AuClient.DesignerUnitScale();
}

AuScFaceList::UNWRAP_METHOD AuScTextureUnwrapDlg::UnwrapMethod() const
{
	return m_Method;
}

void AuScTextureUnwrapDlg::OkClicked()
{
	double texSize = m_TextureSize->text().toDouble();
	g_Au.Settings().writeEntry("Widget/TextureUnwrapDlg/TextureSize", texSize);

	float angleTol = m_AngleTol->text().toFloat();
	g_Au.Settings().writeEntry("Widget/TextureUnwrapDlg/AngleTol", angleTol);

	bool gridAlign = m_GridAlign->isChecked();
	g_Au.Settings().writeEntry("Widget/TextureUnwrapDlg/GridAlign", gridAlign);

	int axis = AlignAxis();
	g_Au.Settings().writeEntry(QString("Widget/TextureUnwrapDlg/AlignAxis"), axis);

	accept();
}

void AuScTextureUnwrapDlg::UnwrapByFace()
{
	m_Method = AuScFaceList::UNWRAP_ByFace;
	OkClicked();
}

void AuScTextureUnwrapDlg::UnwrapByFacing()
{
	m_Method = AuScFaceList::UNWRAP_ByFacing;
	OkClicked();
}

void AuScTextureUnwrapDlg::UnwrapEdges()
{
	m_Method = AuScFaceList::UNWRAP_Edges;
	OkClicked();
}

void AuScTextureUnwrapDlg::UnwrapHorizontal()
{
	m_Method = AuScFaceList::UNWRAP_Horizontal;
	OkClicked();
}

void AuScTextureUnwrapDlg::UnwrapVertical()
{
	m_Method = AuScFaceList::UNWRAP_Vertical;
	OkClicked();
}

int AuScTextureUnwrapDlg::AlignAxis() const
{
	int axis;
	if(m_XAxis->isChecked())
		axis = 0;
	else if(m_YAxis->isChecked())
		axis = 1;
	else
		axis = 2;
	return axis;
}
