//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austarmapplaces.h,v 1.11 2022/02/19 18:01:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStarMapPlaces_H
#define AuStarMapPlaces_H

#include "auclientlib.h"
#include <auid.h>
#include <aupoint.h>
#include <QTreeView>

class AuId;
class AuPoint;
class QMouseEvent;

class AUCLIENT_DLL AuStarMapPlaces : public QTreeView
{
	Q_OBJECT;
private:
	AuId								m_Preview;				// Atom that is closest to the moving cursor, for preview.
	AuPoint								m_PreviewLoc;			// Galaxy coordinate of preview atom
	QString								m_PreviewName;			// Atom name that is closest to the moving cursor, for orders.
	quint8								m_PreviewType;			// AtomType of preview atom.

public:
	AuStarMapPlaces(QWidget* parent=0);
	~AuStarMapPlaces();

	void RestoreSettings();

signals:
	void DomainAdd(AuId systemId, QString systemName);
	void GoDirectToCoord(AuPoint galaxyLoc);
	void GoDirectToSystem(AuId systemId, QString systemName, AuPoint galaxyLoc);
	void GoViaWormholes(AuId systemId, QString systemName);
	void LandAtCity(AuId cityId, QString cityName);
	void OrbitCity(AuId cityId, QString cityName);
	void PlaceClicked(const AuId& id);
	void PlaceDoubleClicked(const AuId& id);
	void QuerySector(const AuId& id);
	void ReportCityHistory(const AuId& cityId);
	void ReportCityLast(const AuId& cityId);
	// 'scanEx' 2=scanned, 1=explored, 0=observed
	void ReportSystemResources(AuId systemId, QString systemName, quint8 galaxy, bool isCoreBlackHole, quint8 genHab,
			quint32 genSeed, AuPoint systemLoc, float starLight, int scanEx);
	void ScanSystem();
	void ScanTactical();
	void SelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

protected slots:
	virtual void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	virtual void ColumnResized(int columnIdx, int oldWidth, int nuWidth);

protected:
	virtual void mouseReleaseEvent(QMouseEvent* event);
};

#endif // AuStarMapPlaces_H
