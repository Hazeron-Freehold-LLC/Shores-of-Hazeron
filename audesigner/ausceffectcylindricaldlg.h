//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausceffectcylindricaldlg.h,v 1.2 2022/02/19 18:22:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScEffectCylindricalDlg_H
#define AuScEffectCylindricalDlg_H

#include "audialog.h"
#include "augleffects.h"
#include "dbcommodity.h"

class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;

class AuScEffectCylindricalDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QListWidget*						m_TextureList;
	QLabel*								m_TextureSample;
	QLineEdit*							m_TextureReps;
	QLineEdit*							m_TextureSize;

public:
	AuScEffectCylindricalDlg(QWidget* parent=0);
	virtual ~AuScEffectCylindricalDlg();

	SURFACE_EFFECT Effect() const;

	quint16 TextureReps() const;

	// Returns the size of the texture in feet.
	double TextureSize() const;

public slots:
	void OkClicked();
	virtual void setVisible(bool visible);

protected:
	void ListEffects(int curIdx);
	void RestoreGeometry();
	void SaveGeometry();
	void TextureSample(int sct);

protected slots:
	void TextureChanged(QListWidgetItem* current, QListWidgetItem* previous);
};

#endif // AuScEffectCylindricalDlg_H
