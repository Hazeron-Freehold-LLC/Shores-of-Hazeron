//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: au3dsglsldna.h,v 1.2 2022/02/19 18:29:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Coded using the following "specifications".
// "3D-Studio File Format (.3ds)"
//   Autodesk Ltd., Document Revision 0.91. June 1996
//   Original by Jim Pitts. Edited by Paul Bourke.
//   http://local.wasp.uwa.edu.au/~pbourke/dataformats/3ds/
// "3D-Studio Material-Library File Format (.mli)"
//   Autodesk Ltd., Document Revision 0.25. May 1996, December 1996
//   Robin Fercoq, Martin van Velsen, James Fleming
//   http://www.martinreddy.net/gfx/3d/MLI.spec
//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef Au3dsGLSLDNA_H
#define Au3dsGLSLDNA_H

#include "augl.h"

// This class takes all the properties of a players DNA that effect the the model outcome.
// It will be used to determine if a display list has already been created from the model
// pieces that form the player.
class AUGL_DLL Au3dsGLSLDNA
{
public:
	Au3dsGLSLDNA();
	//Au3dsGLSLDNA(unsigned int iOverallSize, unsigned int iTorsoVertebrae, unsigned int iNeckVertebrae,
	//	unsigned int TailVertebrae, unsigned int iHeads, unsigned int iArmPairs, unsigned int iLegPairs,
	//	unsigned int iArmType, unsigned int iLegType, unsigned int iTorsoSegmentLength, unsigned int iTorsoThickness,
	//	unsigned int iTailType, unsigned int iTopSkinThickness, unsigned int iTopSkinSubtype, unsigned int iSkinType,
	//	unsigned int iHandLength, unsigned int iHandType, unsigned int iFootLength, unsigned int iFootType,
	//	unsigned int iWingSubType, unsigned int iUpperArmLength, unsigned int iUpperArmThickness, unsigned int iForeArmLength,
	//	unsigned int iForeArmThickness, unsigned int iThighLength, unsigned int iThighThickness, unsigned int iCalfLength,
	//	unsigned int iCalfThickness, unsigned int iMouthType, unsigned int iMouthSize, unsigned int iNoseType,
	//	unsigned int iNoseSize, unsigned int iHornBranches, unsigned int iHornSize, unsigned int iEyeSize,
	//	unsigned int iEyeType, unsigned int iEarSize, unsigned int iEarType, unsigned int iHeadCrestSize,
	//	unsigned int iTorsoCrestSize, unsigned int iBodyCarriage, unsigned int iCrestSubtype, unsigned int iWeaponType,
	//	unsigned int iWeaponMode, unsigned int iWeaponSize, unsigned int iBottomSkinSubtype, unsigned int iBottomSkinThickness);
	~Au3dsGLSLDNA();

	unsigned int OverallSize;
	unsigned int TorsoVertebrae;
	unsigned int NeckVertebrae;
	unsigned int TailVertebrae;
	unsigned int Heads;
	unsigned int ArmPairs;
	unsigned int LegPairs;
	unsigned int ArmType;
	unsigned int LegType;
	unsigned int TorsoSegmentLength;
	unsigned int TorsoThickness;
	unsigned int TailType;
	unsigned int TopSkinThickness;
	unsigned int TopSkinSubtype;
	unsigned int SkinType;
	unsigned int HandLength;
	unsigned int HandType;
	unsigned int FootLength;
	unsigned int FootType;
	unsigned int WingSubType;
	unsigned int UpperArmLength;
	unsigned int UpperArmThickness;
	unsigned int ForeArmLength;
	unsigned int ForeArmThickness;
	unsigned int ThighLength;
	unsigned int ThighThickness;
	unsigned int CalfLength;
	unsigned int CalfThickness;
	unsigned int MouthType;
	unsigned int MouthSize;
	unsigned int NoseType;
	unsigned int NoseSize;
	unsigned int HornBranches;
	unsigned int HornSize;
	unsigned int EyeSize;
	unsigned int EyeType;
	unsigned int EarSize;
	unsigned int EarType;
	unsigned int HeadCrestSize;
	unsigned int TorsoCrestSize;
	unsigned int BodyCarriage;
	unsigned int CrestSubtype;
	unsigned int WeaponType;
	unsigned int WeaponMode;
	unsigned int WeaponSize;
	unsigned int BottomSkinSubtype;
	unsigned int BottomSkinThickness;
	unsigned int ModelType;				// 0-Skin, 1-PowerArmor

	bool operator==(const Au3dsGLSLDNA& c) const;

public:
	// For QMap to work properly we must include this
	// Sort the checks by greatest to smallest number difference
	// so that this comparison can complete quicker
	inline bool operator<(const Au3dsGLSLDNA &p) const
	{
		if(OverallSize != p.OverallSize)						// 0..31
		{
			if(OverallSize < p.OverallSize)
				return true;
			else if(OverallSize > p.OverallSize)
				return false;
		}
		else if(TorsoSegmentLength != p.TorsoSegmentLength)		// 0..31
		{
			if(TorsoSegmentLength < p.TorsoSegmentLength)
				return true;
			else if(TorsoSegmentLength > p.TorsoSegmentLength)
				return false;
		}
		else if(TorsoThickness != p.TorsoThickness)				// 0..31
		{
			if(TorsoThickness < p.TorsoThickness)
				return true;
			else if(TorsoThickness > p.TorsoThickness)
				return false;
		}
		else if(HandLength != p.HandLength)						// 0..31
		{
			if(HandLength < p.HandLength)
				return true;
			else if(HandLength > p.HandLength)
				return false;
		}
		else if(FootLength != p.FootLength)						// 0..31
		{
			if(FootLength < p.FootLength)
				return true;
			else if(FootLength > p.FootLength)
				return false;
		}
		else if(UpperArmLength != p.UpperArmLength)				// 0..31
		{
			if(UpperArmLength < p.UpperArmLength)
				return true;
			else if(UpperArmLength > p.UpperArmLength)
				return false;
		}
		else if(UpperArmThickness != p.UpperArmThickness)		// 0..31
		{
			if(UpperArmThickness < p.UpperArmThickness)
				return true;
			else if(UpperArmThickness > p.UpperArmThickness)
				return false;
		}
		else if(ForeArmLength != p.ForeArmLength)				// 0..31
		{
			if(ForeArmLength < p.ForeArmLength)
				return true;
			else if(ForeArmLength > p.ForeArmLength)
				return false;
		}
		else if(ForeArmThickness != p.ForeArmThickness)			// 0..31
		{
			if(ForeArmThickness < p.ForeArmThickness)
				return true;
			else if(ForeArmThickness > p.ForeArmThickness)
				return false;
		}
		else if(ThighLength != p.ThighLength)					// 0..31
		{
			if(ThighLength < p.ThighLength)
				return true;
			else if(ThighLength > p.ThighLength)
				return false;
		}
		else if(ThighThickness != p.ThighThickness)				// 0..31
		{
			if(ThighThickness < p.ThighThickness)
				return true;
			else if(ThighThickness > p.ThighThickness)
				return false;
		}
		else if(CalfLength != p.CalfLength)						// 0..31
		{
			if(CalfLength < p.CalfLength)
				return true;
			else if(CalfLength > p.CalfLength)
				return false;
		}
		else if(CalfThickness != p.CalfThickness)				// 0..31
		{
			if(CalfThickness < p.CalfThickness)
				return true;
			else if(CalfThickness > p.CalfThickness)
				return false;
		}
		else if(WeaponType != p.WeaponType)						// 0..15
		{
			if(WeaponType < p.WeaponType)
				return true;
			else if(WeaponType > p.WeaponType)
				return false;
		}
		else if(WeaponSize != p.WeaponSize)						// 0..7
		{
			if(WeaponSize < p.WeaponSize)
				return true;
			else if(WeaponSize > p.WeaponSize)
				return false;
		}
		else if(TopSkinSubtype != p.TopSkinSubtype)				// 0..7
		{
			if(TopSkinSubtype < p.TopSkinSubtype)
				return true;
			else if(TopSkinSubtype > p.TopSkinSubtype)
				return false;
		}
		else if(BottomSkinSubtype != p.BottomSkinSubtype)		// 0..7
		{
			if(BottomSkinSubtype < p.BottomSkinSubtype)
				return true;
			else if(BottomSkinSubtype > p.BottomSkinSubtype)
				return false;
		}
		else if(TorsoVertebrae != p.TorsoVertebrae)				// 0..7
		{
			if(TorsoVertebrae < p.TorsoVertebrae)
				return true;
			else if(TorsoVertebrae > p.TorsoVertebrae)
				return false;
		}
		else if(ArmType != p.ArmType)							// 0..7
		{
			if(ArmType < p.ArmType)
				return true;
			else if(ArmType > p.ArmType)
				return false;
		}
		else if(SkinType != p.SkinType)							// 0..7
		{
			if(SkinType < p.SkinType)
				return true;
			else if(SkinType > p.SkinType)
				return false;
		}
		else if(HandType != p.HandType)							// 0..7
		{
			if(HandType < p.HandType)
				return true;
			else if(HandType > p.HandType)
				return false;
		}
		else if(FootType != p.FootType)							// 0..7
		{
			if(FootType < p.FootType)
				return true;
			else if(FootType > p.FootType)
				return false;
		}
		else if(WingSubType != p.WingSubType)					// 0..7
		{
			if(WingSubType < p.WingSubType)
				return true;
			else if(WingSubType > p.WingSubType)
				return false;
		}
		else if(MouthType != p.MouthType)						// 0..7
		{
			if(MouthType < p.MouthType)
				return true;
			else if(MouthType > p.MouthType)
				return false;
		}
		else if(EyeType != p.EyeType)							// 0..7
		{
			if(EyeType < p.EyeType)
				return true;
			else if(EyeType > p.EyeType)
				return false;
		}
		else if(EarType != p.EarType)							// 0..7
		{
			if(EarType < p.EarType)
				return true;
			else if(EarType > p.EarType)
				return false;
		}
		else if(NoseType != p.NoseType)							// 0..7
		{
			if(NoseType < p.NoseType)
				return true;
			else if(NoseType > p.NoseType)
				return false;
		}
		else if(NeckVertebrae != p.NeckVertebrae)				// 0..3
		{
			if(NeckVertebrae < p.NeckVertebrae)
				return true;
			else if(NeckVertebrae > p.NeckVertebrae)
				return false;
		}
		else if(TailVertebrae != p.TailVertebrae)				// 0..3
		{
			if(TailVertebrae < p.TailVertebrae)
				return true;
			else if(TailVertebrae > p.TailVertebrae)
				return false;
		}
		else if(ArmPairs != p.ArmPairs)							// 0..3
		{
			if(ArmPairs < p.ArmPairs)
				return true;
			else if(ArmPairs > p.ArmPairs)
				return false;
		}
		else if(LegPairs != p.LegPairs)							// 0..3
		{
			if(LegPairs < p.LegPairs)
				return true;
			else if(LegPairs > p.LegPairs)
				return false;
		}
		else if(TorsoVertebrae != p.TorsoVertebrae)				// 0..3
		{
			if(TorsoVertebrae < p.TorsoVertebrae)
				return true;
			else if(TorsoVertebrae > p.TorsoVertebrae)
				return false;
		}
		else if(LegType != p.LegType)							// 0..3
		{
			if(LegType < p.LegType)
				return true;
			else if(LegType > p.LegType)
				return false;
		}
		else if(TailType != p.TailType)							// 0..3
		{
			if(TailType < p.TailType)
				return true;
			else if(TailType > p.TailType)
				return false;
		}
		else if(TopSkinThickness != p.TopSkinThickness)			// 0..3
		{
			if(TopSkinThickness < p.TopSkinThickness)
				return true;
			else if(TopSkinThickness > p.TopSkinThickness)
				return false;
		}
		else if(MouthSize != p.MouthSize)						// 0..3
		{
			if(MouthSize < p.MouthSize)
				return true;
			else if(MouthSize > p.MouthSize)
				return false;
		}
		else if(NoseSize != p.NoseSize)							// 0..3
		{
			if(NoseSize < p.NoseSize)
				return true;
			else if(NoseSize > p.NoseSize)
				return false;
		}
		else if(HornBranches != p.HornBranches)					// 0..3
		{
			if(HornBranches < p.HornBranches)
				return true;
			else if(HornBranches > p.HornBranches)
				return false;
		}
		else if(HornSize != p.HornSize)							// 0..3
		{
			if(HornSize < p.HornSize)
				return true;
			else if(HornSize > p.HornSize)
				return false;
		}
		else if(EyeSize != p.EyeSize)							// 0..3
		{
			if(EyeSize < p.EyeSize)
				return true;
			else if(EyeSize > p.EyeSize)
				return false;
		}
		else if(EarSize != p.EarSize)							// 0..3
		{
			if(EarSize < p.EarSize)
				return true;
			else if(EarSize > p.EarSize)
				return false;
		}
		else if(HeadCrestSize != p.HeadCrestSize)				// 0..3
		{
			if(HeadCrestSize < p.HeadCrestSize)
				return true;
			else if(HeadCrestSize > p.HeadCrestSize)
				return false;
		}
		else if(TorsoCrestSize != p.TorsoCrestSize)				// 0..3
		{
			if(TorsoCrestSize < p.TorsoCrestSize)
				return true;
			else if(TorsoCrestSize > p.TorsoCrestSize)
				return false;
		}
		else if(BodyCarriage != p.BodyCarriage)					// 0..3
		{
			if(BodyCarriage < p.BodyCarriage)
				return true;
			else if(BodyCarriage > p.BodyCarriage)
				return false;
		}
		else if(CrestSubtype != p.CrestSubtype)					// 0..3
		{
			if(CrestSubtype < p.CrestSubtype)
				return true;
			else if(CrestSubtype > p.CrestSubtype)
				return false;
		}
		else if(WeaponMode != p.WeaponMode)						// 0..3
		{
			if(WeaponMode < p.WeaponMode)
				return true;
			else if(WeaponMode > p.WeaponMode)
				return false;
		}
		else if(BottomSkinThickness != p.BottomSkinThickness)	// 0..3
		{
			if(BottomSkinThickness < p.BottomSkinThickness)
				return true;
			else if(BottomSkinThickness > p.BottomSkinThickness)
				return false;
		}
		else if(Heads != p.Heads)								// 0..1
		{
			if(Heads < p.Heads)
				return true;
			else if(Heads > p.Heads)
				return false;
		}
		else if(ModelType != p.ModelType)						// 0 - Skin, 1 - PowerArmor
		{
			if(ModelType < p.ModelType)
				return true;
			else if(ModelType > p.ModelType)
				return false;
		}
		return false;
	}

	inline quint64 MemoryUsage() const
	{
		return sizeof(Au3dsGLSLDNA);
	}
};

#endif // Au3dsGLSLDNA_H
