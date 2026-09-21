//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausctextureunwrapdlg.h,v 1.2 2022/02/19 18:22:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScTextureUnwrapDlg_H
#define AuScTextureUnwrapDlg_H

#include "audesign.h"
#include "audialog.h"
#include "augleffects.h"
#include "auscface.h"
#include "dbcommodity.h"

class QCheckBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QRadioButton;

class AuScTextureUnwrapDlg : public AuDialog
{
	Q_OBJECT;
private:
	COMMODITYID							m_HullModule;
	AuScFaceList::UNWRAP_METHOD			m_Method;				// Only valid on accept.
	AuDesignTex							m_Palette[SCTEXI_Count];

	// Widgets
	QLineEdit*							m_AngleTol;
	QCheckBox*							m_GridAlign;
	QListWidget*						m_TextureList;
	QLabel*								m_TextureSample;
	QLineEdit*							m_TextureSize;
	QRadioButton*						m_XAxis;
	QRadioButton*						m_YAxis;
	QRadioButton*						m_ZAxis;

public:
	AuScTextureUnwrapDlg(SC_TEXTURE_INDEX currentIndex, COMMODITYID hullModule, const AuDesignTex palette[SCTEXI_Count],
			QWidget* parent=0);
	virtual ~AuScTextureUnwrapDlg();

	// 0=x, 1=y, 2=z
	int AlignAxis() const;

	float AngleTolerance() const;

	bool GridAlign() const;

	// Returns SCTEXI_Count if none.
	SC_TEXTURE_INDEX TextureIndex() const;

	// Returns the size of the texture in feet.
	double TextureSize() const;

	AuScFaceList::UNWRAP_METHOD UnwrapMethod() const;

public slots:
	void OkClicked();
	virtual void setVisible(bool visible);
	void UnwrapByFace();
	void UnwrapByFacing();
	void UnwrapEdges();
	void UnwrapHorizontal();
	void UnwrapVertical();

protected:
	void ListTextures(int curIdx);
	void RestoreGeometry();
	void SaveGeometry();
	void TextureSample(int sct);

protected slots:
	void TextureChanged(QListWidgetItem* current, QListWidgetItem* previous);
};

#endif // AuScTextureUnwrapDlg_H
