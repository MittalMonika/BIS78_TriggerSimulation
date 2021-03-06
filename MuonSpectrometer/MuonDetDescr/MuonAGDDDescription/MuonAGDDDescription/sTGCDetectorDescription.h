/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef sTGCDetectorDescription_H
#define sTGCDetectorDescription_H

#include "AGDDKernel/AGDDDetector.h"
#include "MuonAGDDDescription/sTGC_Technology.h"
#include <string>
#include <vector>
#include <iostream>

class GeoMaterial;

using MuonGM::sTGC_Technology;

struct sTGCReadoutParameters {
    double sPadWidth;
    double lPadWidth;
    double sStripWidth;
    double lStripWidth;
    std::vector<double> padH;
    std::vector<int> nPadPhi;
    double anglePadPhi;
    std::vector<double> firstPadPhiDivision_A;
    std::vector<double> firstPadPhiDivision_C;
    std::vector<double> PadPhiShift_A;
    std::vector<double> PadPhiShift_C;
    std::vector<double> nPadH;
    std::vector<double> firstPadH;
    std::vector<int> firstPadRow;
    std::vector<int> nWires;
    std::vector<double> wireCutout;
    std::vector<double> firstWire;
    int wireGroupWidth;
    int nStrips;
    std::vector<int> firstTriggerBand;
    std::vector<int> nTriggerBands;
    std::vector<int> firstStripInTrigger;
    std::vector<double> firstStripWidth;
    std::vector<int> StripsInBandsLayer1;
    std::vector<int> StripsInBandsLayer2;
    std::vector<int> StripsInBandsLayer3;
    std::vector<int> StripsInBandsLayer4;
    std::vector<int> nWireGroups;
    std::vector<int> firstWireGroup;
	
};

class sTGCDetectorDescription: public AGDDDetector {
public:
    sTGCDetectorDescription(std::string s);
    void Register();
	
        virtual void SetXYZ(std::vector<double> v) override
	{
                small_x(v[0]);
		large_x(v[1]);
		y(v[2]);
		z(v[3]);
		_yCutout=v[4];
		_stripPitch=v[5];
		_wirePitch=v[6];
		_stripWidth=v[7];
		_yCutoutCathode=v[8];
	}
	
	double sWidth() {return small_x();}
	double lWidth() {return large_x();}
	double Length() {return y();}
	double Tck()    {return z();}

	void yCutout(double y) {_yCutout=y;}
	double yCutout() {return _yCutout;}
	
	void stripPitch(double y) {_stripPitch=y;}
	double stripPitch() {return _stripPitch;}
	
	void wirePitch(double y) {_wirePitch=y;}
	double wirePitch() {return _wirePitch;}
	
	void stripWidth(double y) {_stripWidth=y;}
	double stripWidth() {return _stripWidth;}
	
	void yCutoutCathode(double y) {_yCutoutCathode=y;}
	double yCutoutCathode() {return _yCutoutCathode;}
	
	void xFrame(double y) {_xFrame=y;}
	double xFrame() {return _xFrame;}
	
	void ysFrame(double y) {_ysFrame=y;}
	double ysFrame() {return _ysFrame;}
	
	void ylFrame(double y) {_ylFrame=y;}
	double ylFrame() {return _ylFrame;}

	sTGCReadoutParameters roParameters;
	
	sTGCReadoutParameters& GetReadoutParameters() {return roParameters;}

	static sTGCDetectorDescription* GetCurrent() {return current;}
	
	sTGC_Technology* GetTechnology();

protected:

	double _yCutout;
	double _stripPitch;
	double _wirePitch;
	double _stripWidth;
	double _yCutoutCathode;
	
	double _xFrame;
	double _ysFrame;
	double _ylFrame;
	
	static sTGCDetectorDescription* current;
	void SetDetectorAddress(AGDDDetectorPositioner*);
};

#endif
