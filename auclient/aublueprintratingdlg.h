//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aublueprintratingdlg.h,v 1.2 2022/02/19 17:52:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBlueprintRatingDlg_H
#define AuBlueprintRatingDlg_H

#include "audialog.h"

class AuExchangeBlueprintRating;
class QVBoxLayout;

class AuBlueprintRatingDlg : public AuDialog
{
	Q_OBJECT;
public:
	AuBlueprintRatingDlg(quint32 designId, const QString& designName, const AuExchangeBlueprintRating& rating, QWidget* parent=0);
	virtual ~AuBlueprintRatingDlg();

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListReviews(QVBoxLayout& lay, const AuExchangeBlueprintRating& rating);
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuBlueprintRatingDlg_H
