//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausceffectsphericaldlg.h,v 1.2 2022/02/19 18:22:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScEffectSphericalDlg_H
#define AuScEffectSphericalDlg_H

#include "audialog.h"
#include "augleffects.h"
#include "dbcommodity.h"

class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;

class AuScEffectSphericalDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QListWidget*						m_TextureList;
	QLabel*								m_TextureSample;
	QLineEdit*							m_TextureReps;

public:
	AuScEffectSphericalDlg(QWidget* parent=0);
	virtual ~AuScEffectSphericalDlg();

	SURFACE_EFFECT Effect() const;

	quint16 TextureReps() const;

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

#endif // AuScEffectSphericalDlg_H
