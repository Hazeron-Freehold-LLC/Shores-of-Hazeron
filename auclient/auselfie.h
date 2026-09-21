//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auselfie.h,v 1.6 2022/02/19 18:00:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSelfie_H
#define AuSelfie_H

#include "auclientlib.h"
#include "audialog.h"
#include <QImage>

class AUCLIENT_DLL AuSelfie : public AuDialog
{
	Q_OBJECT;
	QImage								m_Selfie;

public:
	AuSelfie(const QString& selfieName, const QImage& selfieImg, QWidget* parent=0);
	virtual ~AuSelfie();

public slots:
	virtual void setVisible(bool visible);
	void SendSelfie();

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuSelfie_H
