//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausceffectunwrapdlg.cpp,v 1.6 2022/02/19 18:25:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausceffectunwrapdlg.h"
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

AuScEffectUnwrapDlg::AuScEffectUnwrapDlg(SURFACE_EFFECT currentIndex, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Unwrap Effect Faces"));
	setAuDlgIcon(QIcon(":data/ScUnwrapVert.png"));

	if(currentIndex >= EFF_Count)
		currentIndex = EFF_FTLDrive1;

	m_Method = AuScFaceList::UNWRAP_ByFace;

	// Read last settings.
	double lastSize = g_Au.Settings().readNumEntry("Widget/EffectUnwrapDlg/TextureSize", 2.);	// Stored in user units.
	float lastTol = g_Au.Settings().readNumEntry("Widget/EffectUnwrapDlg/AngleTol", 2.);		// Stored in degrees.
	if(lastTol < 0)
		lastTol = 0;
	bool lastAlign = g_Au.Settings().readBoolEntry("Widget/EffectUnwrapDlg/GridAlign", true);
	int lastAxis = g_Au.Settings().readNumEntry(QString("Widget/EffectUnwrapDlg/AlignAxis"), 1);

	// Build widgets.
	QLabel* explain = new QLabel(tr("<html>"
				"<p>"
				"Choose the effect texture that will be applied to the unwrapped faces. "
				"</p>"
				"</html"), this);
	explain->setWordWrap(true);

	m_TextureList = new QListWidget(this);
	m_TextureList->setSpacing(2);
	ListEffects(currentIndex);
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

AuScEffectUnwrapDlg::~AuScEffectUnwrapDlg()
{
}

void AuScEffectUnwrapDlg::setVisible(bool visible)
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

void AuScEffectUnwrapDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/EffectUnwrapDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/EffectUnwrapDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/EffectUnwrapDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/EffectUnwrapDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScEffectUnwrapDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/EffectUnwrapDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/EffectUnwrapDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/EffectUnwrapDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/EffectUnwrapDlg/SizeY",sz.height());
}

SURFACE_EFFECT AuScEffectUnwrapDlg::Effect() const
{
	QListWidgetItem* curItem = m_TextureList->currentItem();
	if(!curItem)
		return EFF_Count;

	SURFACE_EFFECT effect = (SURFACE_EFFECT)curItem->data(Qt::UserRole).toUInt();
	if(effect >= EFF_Count)
		return EFF_Count;

	return effect;
}

void AuScEffectUnwrapDlg::ListEffects(int curIdx)
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

void AuScEffectUnwrapDlg::TextureChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
	int curIdx;
	if(current)
		curIdx = current->data(Qt::UserRole).toUInt();
	else
		curIdx = EFF_Count;

	TextureSample(curIdx);
}

void AuScEffectUnwrapDlg::TextureSample(int effect)
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

bool AuScEffectUnwrapDlg::GridAlign() const
{
	return m_GridAlign->isChecked();
}

float AuScEffectUnwrapDlg::AngleTolerance() const
{
	float angleTol = m_AngleTol->text().toFloat();
	if(angleTol < 0)
		angleTol = 0;
	return angleTol * DEG2RADf;
}

double AuScEffectUnwrapDlg::TextureSize() const
{
	double texSize = m_TextureSize->text().toDouble();
	return texSize * g_AuClient.DesignerUnitScale();
}

AuScFaceList::UNWRAP_METHOD AuScEffectUnwrapDlg::UnwrapMethod() const
{
	return m_Method;
}

void AuScEffectUnwrapDlg::OkClicked()
{
	double texSize = m_TextureSize->text().toDouble();
	g_Au.Settings().writeEntry("Widget/EffectUnwrapDlg/TextureSize", texSize);

	float angleTol = m_AngleTol->text().toFloat();
	g_Au.Settings().writeEntry("Widget/EffectUnwrapDlg/AngleTol", angleTol);

	bool gridAlign = m_GridAlign->isChecked();
	g_Au.Settings().writeEntry("Widget/EffectUnwrapDlg/GridAlign", gridAlign);

	int axis = AlignAxis();
	g_Au.Settings().writeEntry(QString("Widget/EffectUnwrapDlg/AlignAxis"), axis);

	accept();
}

void AuScEffectUnwrapDlg::UnwrapByFace()
{
	m_Method = AuScFaceList::UNWRAP_ByFace;
	OkClicked();
}

void AuScEffectUnwrapDlg::UnwrapByFacing()
{
	m_Method = AuScFaceList::UNWRAP_ByFacing;
	OkClicked();
}

void AuScEffectUnwrapDlg::UnwrapEdges()
{
	m_Method = AuScFaceList::UNWRAP_Edges;
	OkClicked();
}

void AuScEffectUnwrapDlg::UnwrapHorizontal()
{
	m_Method = AuScFaceList::UNWRAP_Horizontal;
	OkClicked();
}

void AuScEffectUnwrapDlg::UnwrapVertical()
{
	m_Method = AuScFaceList::UNWRAP_Vertical;
	OkClicked();
}

int AuScEffectUnwrapDlg::AlignAxis() const
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
