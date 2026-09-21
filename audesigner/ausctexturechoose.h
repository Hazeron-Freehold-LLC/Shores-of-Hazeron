//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausctexturechoose.h,v 1.2 2022/02/19 18:22:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScTextureChoose_H
#define AuScTextureChoose_H

#include <audialog.h>

class QLabel;
class QListWidget;
class QListWidgetItem;

class AuScTextureChoose : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QListWidget*						m_TextureList;
	QLabel*								m_TextureSample;

public:
	AuScTextureChoose(quint16 libTexId, QWidget* parent=0);
	virtual ~AuScTextureChoose();

	// Returns an AuTextureLibItem::Id.
	quint16 Texture() const;

public slots:
	void OkClicked();
	virtual void setVisible(bool visible);

protected:
	void ListTextures(int curItemId);
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void TextureChanged(QListWidgetItem* current, QListWidgetItem* previous);
};

#endif // AuScTextureChoose_H
