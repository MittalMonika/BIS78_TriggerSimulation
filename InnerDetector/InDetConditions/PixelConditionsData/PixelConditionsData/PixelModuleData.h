/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELMODULEDATA_H
#define PIXELMODULEDATA_H

#include "AthenaKernel/CLASS_DEF.h"
#include <map>

class PixelModuleData {
  public:
    PixelModuleData();
    virtual ~PixelModuleData();

    void setBiasVoltage(const int chanNum, const float value);
    float getBiasVoltage(const int chanNum) const;

    void setTemperature(const int chanNum, const float value);
    float getTemperature(const int chanNum) const;

    void setModuleStatus(const int chanNum, const int value);
    int getModuleStatus(const int chanNum) const;

    void setChipStatus(const int chanNum, const int value);
    int getChipStatus(const int chanNum) const;

    enum DCSModuleStatus{OK,WARNING,ERROR,FATAL,NOSTATUS};
    enum DCSModuleState{READY,ON,UNKNOWN,TRANSITION,UNDEFINED,NOSTATE};

    // set via job option
    void setBunchSpace(double bunchSpace);
    double getBunchSpace() const;

    void setUseComTime(bool UseComTime);
    bool getUseComTime() const;

    void setComTime(double ComTime);
    double getComTime() const;

    void setBarrelNumberOfBCID(std::vector<int> BarrelNumberOfBCID);
    void setEndcapNumberOfBCID(std::vector<int> EndcapNumberOfBCID);
    void setDBMNumberOfBCID(std::vector<int>    DBMNumberOfBCID);
    int getNumberOfBCID(int bec, int layer) const;

    void setBarrelTimeOffset(std::vector<double> BarrelTimeOffset);
    void setEndcapTimeOffset(std::vector<double> EndcapTimeOffset);
    void setDBMTimeOffset(std::vector<double>    DBMTimeOffset);
    double getTimeOffset(int bec, int layer) const;

    void setBarrelTimeJitter(std::vector<double> BarrelTimeJitter);
    void setEndcapTimeJitter(std::vector<double> EndcapTimeJitter);
    void setDBMTimeJitter(std::vector<double>    DBMTimeJitter);
    double getTimeJitter(int bec, int layer) const;

    void setUseCalibConditions(bool UseCalibConditions);
    bool getUseCalibConditions() const;

    void setDefaultBarrelAnalogThreshold(std::vector<int> BarrelAnalogThreshold);
    void setDefaultEndcapAnalogThreshold(std::vector<int> EndcapAnalogThreshold);
    void setDefaultDBMAnalogThreshold(std::vector<int>    DBMAnalogThreshold);
    int getDefaultAnalogThreshold(int bec, int layer) const;

    void setDefaultBarrelAnalogThresholdSigma(std::vector<int> BarrelAnalogThresholdSigma);
    void setDefaultEndcapAnalogThresholdSigma(std::vector<int> EndcapAnalogThresholdSigma);
    void setDefaultDBMAnalogThresholdSigma(std::vector<int>    DBMAnalogThresholdSigma);
    int getDefaultAnalogThresholdSigma(int bec, int layer) const;

    void setDefaultBarrelAnalogThresholdNoise(std::vector<int> BarrelAnalogThresholdNoise);
    void setDefaultEndcapAnalogThresholdNoise(std::vector<int> EndcapAnalogThresholdNoise);
    void setDefaultDBMAnalogThresholdNoise(std::vector<int>    DBMAnalogThresholdNoise);
    int getDefaultAnalogThresholdNoise(int bec, int layer) const;

    void setDefaultBarrelInTimeThreshold(std::vector<int> BarrelInTimeThreshold);
    void setDefaultEndcapInTimeThreshold(std::vector<int> EndcapInTimeThreshold);
    void setDefaultDBMInTimeThreshold(std::vector<int>    DBMInTimeThreshold);
    int getDefaultInTimeThreshold(int bec, int layer) const;

    void setDefaultQ2TotA(float paramA);
    void setDefaultQ2TotE(float paramE);
    void setDefaultQ2TotC(float paramC);
    float getDefaultQ2TotA() const;
    float getDefaultQ2TotE() const;
    float getDefaultQ2TotC() const;

    void setBarrelToTThreshold(std::vector<int> BarrelToTThreshold);
    void setEndcapToTThreshold(std::vector<int> EndcapToTThreshold);
    void setDBMToTThreshold(std::vector<int>    DBMToTThreshold);
    int getToTThreshold(int bec, int layer) const;

    void setBarrelCrossTalk(std::vector<double> BarrelCrossTalk);
    void setEndcapCrossTalk(std::vector<double> EndcapCrossTalk);
    void setDBMCrossTalk(std::vector<double>    DBMCrossTalk);
    double getCrossTalk(int bec, int layer) const;

    void setBarrelThermalNoise(std::vector<double> BarrelThermalNoise);
    void setEndcapThermalNoise(std::vector<double> EndcapThermalNoise);
    void setDBMThermalNoise(std::vector<double>    DBMThermalNoise);
    double getThermalNoise(int bec, int layer) const;

    void setBarrelNoiseOccupancy(std::vector<double> BarrelNoiseOccupancy);
    void setEndcapNoiseOccupancy(std::vector<double> EndcapNoiseOccupancy);
    void setDBMNoiseOccupancy(std::vector<double>    DBMNoiseOccupancy);
    double getNoiseOccupancy(int bec, int layer) const;

    void setBarrelDisableProbability(std::vector<double> BarrelDisableProbability);
    void setEndcapDisableProbability(std::vector<double> EndcapDisableProbability);
    void setDBMDisableProbability(std::vector<double>    DBMDisableProbability);
    double getDisableProbability(int bec, int layer) const;

    void setBarrelNoiseShape(const int layer, const float BarrelNoiseShape);
    void setEndcapNoiseShape(const int layer, const float EndcapNoiseShape);
    void setDBMNoiseShape(const int layer,    const float DBMNoiseShape);
    std::vector<float> getNoiseShape(const int bec, const int layer) const;

    void setFEI3BarrelLatency(std::vector<int> FEI3BarrelLatency);
    void setFEI3EndcapLatency(std::vector<int> FEI3EndcapLatency);
    int getFEI3Latency(int bec, int layer) const;

    void setFEI3BarrelHitDuplication(std::vector<bool> FEI3BarrelHitDuplication);
    void setFEI3EndcapHitDuplication(std::vector<bool> FEI3EndcapHitDuplication);
    bool getFEI3HitDuplication(int bec, int layer) const;

    void setFEI3BarrelSmallHitToT(std::vector<int> FEI3BarrelSmallHitToT);
    void setFEI3EndcapSmallHitToT(std::vector<int> FEI3EndcapSmallHitToT);
    int getFEI3SmallHitToT(int bec, int layer) const;

    void setFEI3BarrelTimingSimTune(std::vector<int> FEI3BarrelTimingSimTune);
    void setFEI3EndcapTimingSimTune(std::vector<int> FEI3EndcapTimingSimTune);
    int getFEI3TimingSimTune(int bec, int layer) const;

    void setFEI4BarrelHitDiscConfig(std::vector<int> FEI4BarrelHitDiscConfig);
    void setFEI4EndcapHitDiscConfig(std::vector<int> FEI4EndcapHitDiscConfig);
    int getFEI4HitDiscConfig(int bec, int layer) const;
    int getFEI4OverflowToT(int bec, int layer) const;

    void clear();

  private:
    typedef std::map<int, float> FloatConditions;
    typedef std::map<int, int> IntConditions;
    FloatConditions  m_biasVoltage;
    FloatConditions  m_temperature;
    IntConditions  m_moduleStatus;
    IntConditions  m_chipStatus;

    typedef std::map<int, std::vector<float>> chipCharge;

    double m_bunchSpace;
    bool m_UseComTime;
    double m_ComTime;

    std::vector<int> m_BarrelNumberOfBCID;
    std::vector<int> m_EndcapNumberOfBCID;
    std::vector<int> m_DBMNumberOfBCID;
    std::vector<double> m_BarrelTimeOffset;
    std::vector<double> m_EndcapTimeOffset;
    std::vector<double> m_DBMTimeOffset;
    std::vector<double> m_BarrelTimeJitter;
    std::vector<double> m_EndcapTimeJitter;
    std::vector<double> m_DBMTimeJitter;

    bool m_useCalibConditions;

    std::vector<int> m_defaultBarrelAnalogThreshold;
    std::vector<int> m_defaultEndcapAnalogThreshold;
    std::vector<int> m_defaultDBMAnalogThreshold;
    std::vector<int> m_defaultBarrelAnalogThresholdSigma;
    std::vector<int> m_defaultEndcapAnalogThresholdSigma;
    std::vector<int> m_defaultDBMAnalogThresholdSigma;
    std::vector<int> m_defaultBarrelAnalogThresholdNoise;
    std::vector<int> m_defaultEndcapAnalogThresholdNoise;
    std::vector<int> m_defaultDBMAnalogThresholdNoise;
    std::vector<int> m_defaultBarrelInTimeThreshold;
    std::vector<int> m_defaultEndcapInTimeThreshold;
    std::vector<int> m_defaultDBMInTimeThreshold;

    float m_paramA;
    float m_paramE;
    float m_paramC;

    std::vector<int> m_BarrelToTThreshold;
    std::vector<int> m_EndcapToTThreshold;
    std::vector<int> m_DBMToTThreshold;
    std::vector<double> m_BarrelCrossTalk; 
    std::vector<double> m_EndcapCrossTalk; 
    std::vector<double> m_DBMCrossTalk; 
    std::vector<double> m_BarrelThermalNoise;
    std::vector<double> m_EndcapThermalNoise;
    std::vector<double> m_DBMThermalNoise;
    std::vector<double> m_BarrelNoiseOccupancy;
    std::vector<double> m_EndcapNoiseOccupancy;
    std::vector<double> m_DBMNoiseOccupancy;
    std::vector<double> m_BarrelDisableProbability;
    std::vector<double> m_EndcapDisableProbability;
    std::vector<double> m_DBMDisableProbability;

    chipCharge m_BarrelNoiseShape;
    chipCharge m_EndcapNoiseShape;
    chipCharge m_DBMNoiseShape;

    std::vector<int> m_FEI3BarrelLatency;
    std::vector<int> m_FEI3EndcapLatency;
    std::vector<bool> m_FEI3BarrelHitDuplication;
    std::vector<bool> m_FEI3EndcapHitDuplication;
    std::vector<int>  m_FEI3BarrelSmallHitToT;
    std::vector<int>  m_FEI3EndcapSmallHitToT;
    std::vector<int>  m_FEI3BarrelTimingSimTune;
    std::vector<int>  m_FEI3EndcapTimingSimTune;
    std::vector<int>  m_FEI4BarrelHitDiscConfig;
    std::vector<int>  m_FEI4EndcapHitDiscConfig;

};

CLASS_DEF( PixelModuleData , 345932873 , 1 )

#include "AthenaKernel/CondCont.h"
CONDCONT_DEF( PixelModuleData, 578988393 );

#endif