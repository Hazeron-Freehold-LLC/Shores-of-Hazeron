//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausceffectunwrapdlg.h,v 1.2 2022/02/19 18:22:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScEffectUnwrapDlg_H
#define AuScEffectUnwrapDlg_H

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

class AuScEffectUnwrapDlg : public AuDialog
{
	Q_OBJECT;
private:
	AuScFaceList::UNWRAP_METHOD			m_Method;				// Only valid on accept.

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
	AuScEffectUnwrapDlg(SURFACE_EFFECT currentIndex, QWidget* parent=0);
	virtual ~AuScEffectUnwrapDlg();

	// 0=x, 1=y, 2=z
	int AlignAxis() const;

	float AngleTolerance() const;

	bool GridAlign() const;

	// Returns EFF_Count if none.
	SURFACE_EFFECT Effect() const;

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
	void ListEffects(int curIdx);
	void RestoreGeometry();
	void SaveGeometry();
	void TextureSample(int sct);

protected slots:
	void TextureChanged(QListWidgetItem* current, QListWidgetItem* previous);
};

#endif // AuScEffectUnwrapDlg_H
