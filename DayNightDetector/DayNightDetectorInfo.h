#ifndef _DAY_NIGHT_DETECTOR_INFO_H_
#define _DAY_NIGHT_DETECTOR_INFO_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Step/Type.h>

#if defined(_IDIS_G2_APPLICATION_SUPPORT_LOGGING)
    #include <G2Application/G2DebugLogger.h>
    #include <G2Log/G2DebugLog.h>
    #include <G2Application/G2SystemLogger.h>
    #include <G2Log/G2SystemLog.h>
    #include <G2Log/G2LogDefs.h>
#endif

namespace Idis {

class DNDetectorInfo {
public :
    DNDetectorInfo();
    virtual ~DNDetectorInfo() {}

private :
    int32 _dnChangeDelayNightToDay;
    int32 _dnChangeDelayDayToNight;
    int32 _reassigneNewRefValOnNightLimit;
    int32 _maxHistSeg;
    int32 _maxAdcByIrLight;
    int32 _nightStableCntLimit;
    int32 _notCheckNightLimit;
    int32 _notCheckDayLimit;
    int32 _dnLevelCnt;
    int32 _maxLightSensorValue;

    float _luxThreshNightToDay;
    float _shutterIdx1_30;
    float _standardLuxForLP;

    int32 _adcThreshNtoD [16];
    float _luxThreshNtoD [16];
    int32 _histThreshDtoN[16];
    float _luxThreshDtoN [16];

public :
    void setAllValue(int32  dnChangeDelayNightToDay,
                     int32  dnChangeDelayDayToNight,
                     int32  reassigneNewRefValOnNightLimit,
                     int32  maxHistSeg,
                     int32  maxAdcByIrLight,
                     int32  nightStableCntLimit,
                     int32  notCheckNightLimit,
                     int32  notCheckDayLimit,
                     int32  dnLevelCnt,
                     int32  maxLightSensorValue,
                     float  shutterIdx1_30,
                     float  standardLuxForLP,
                     int32* adcThreshNtoD,
                     float* luxThreshNtoD,
                     int32* histThreshDtoN,
                     float* luxThreshDtoN);

    int32 dnChangeDelayNightToDay()        { return _dnChangeDelayNightToDay; }
    int32 dnChangeDelayDayToNight()        { return _dnChangeDelayDayToNight; }
    int32 reassigneNewRefValOnNightLimit() { return _reassigneNewRefValOnNightLimit; }
    int32 maxHistSeg()                     { return _maxHistSeg; }
    int32 maxAdcByIrLight()                { return _maxAdcByIrLight; }
    int32 nightStableCntLimit()            { return _nightStableCntLimit; }
    int32 notCheckNightLimit()             { return _notCheckNightLimit; }
    int32 notCheckDayLimit()               { return _notCheckDayLimit; }
    int32 dnLevelCnt()                     { return _dnLevelCnt; }
    int32 maxLightSensorValue()            { return _maxLightSensorValue; }
    float shutterIdx1_30()                 { return _shutterIdx1_30; }
    float standardLuxForLP()               { return _standardLuxForLP; }

    int32* adcThreshNtoD()                 { return _adcThreshNtoD; }
    float* luxThreshNtoD()                 { return _luxThreshNtoD; }
    int32* histThreshDtoN()                { return _histThreshDtoN; }
    float* luxThreshDtoN()                 { return _luxThreshDtoN; }

    int32 adcThreshNtoDByIdx (int32 idx)   { return _adcThreshNtoD[idx]; }
    float luxThreshNtoDByIdx (int32 idx)   { return _luxThreshNtoD[idx]; }
    int32 histThreshDtoNByIdx(int32 idx)   { return _histThreshDtoN[idx]; }
    float luxThreshDtoNByIdx (int32 idx)   { return _luxThreshDtoN[idx]; }

    void setDnChangeDelayNightToDay       (int32 dnChangeDelayNightToDay)        { _dnChangeDelayNightToDay        = dnChangeDelayNightToDay; }
    void setDnChangeDelayDayToNight       (int32 dnChangeDelayDayToNight)        { _dnChangeDelayDayToNight        = dnChangeDelayDayToNight; }
    void setReassigneNewRefValOnNightLimit(int32 reassigneNewRefValOnNightLimit) { _reassigneNewRefValOnNightLimit = reassigneNewRefValOnNightLimit; }
    void setMaxHistSeg                    (int32 maxHistSeg)                     { _maxHistSeg                     = maxHistSeg; }
    void setMaxAdcByIrLight               (int32 maxAdcByIrLight)                { _maxAdcByIrLight                = maxAdcByIrLight; }
    void setNightStableCntLimit           (int32 nightStableCntLimit)            { _nightStableCntLimit            = nightStableCntLimit; }
    void setNotCheckNightLimit            (int32 notCheckNightLimit)             { _notCheckNightLimit             = notCheckNightLimit; }
    void setNotCheckDayLimit              (int32 notCheckDayLimit)               { _notCheckNightLimit             = notCheckDayLimit; }
    void setDnLevelCnt                    (int32 dnLevelCnt)                     { _dnLevelCnt                     = dnLevelCnt; }
    void setMaxLightSensorValue           (int32 maxLightSensorValue)            { _maxLightSensorValue            = maxLightSensorValue; }
    void setShutterIdx1_30                (float shutterIdx1_30)                 { _shutterIdx1_30                 = shutterIdx1_30; }
    void setStandardLuxForLP              (float standardLuxForLP)               { _standardLuxForLP               = standardLuxForLP; }
};

class DNChangedInfo {
public :
    DNChangedInfo();
    virtual ~DNChangedInfo() {}

private :
    int32  _adc;
    float  _lux;
    int32  _histGrad;
    bool   _irCond;
    uint32 _changedTick;

public :
    void setDayToNightInfo(int32 adc, float lux, int32 histGrad, bool irCond, uint32 changedTick);

    int32  getAdc()         { return _adc; }
    float  getLux()         { return _lux; }
    int32  getHistGrad()    { return _histGrad; }
    bool   getIrCond()      { return _irCond; }
    uint32 getChangedTick() { return _changedTick; }
};

class DNStateInfo {
public :
    DNStateInfo();
    virtual ~DNStateInfo() {}

private :
    int32 _adc;
    float _lux;
    float _luxByRaw;
    bool  _irCond;
    
public :
    void setStateInfo(int32 adc, float lux, float luxByRaw, bool irCond);

    int32  getAdc()      { return _adc; }
    float  getLux()      { return _lux; }
    float  getLuxByRaw() { return _luxByRaw; }
    bool   getIrCond()      { return _irCond; }
};

class DNChangingInfo {
public :
    DNChangingInfo();
    virtual ~DNChangingInfo() {}

private :
    int32 _platform;

    bool _curDay;
    bool _irCond;
    bool _isWdrMode;
    bool _detectedDay;
    bool _isNightStable;
    bool _icrOn;
    bool _baseIrCond;
    bool _resultDay;

    int32 _adc;
    int32 _shutterIdx;
    int32 _ispGain;
    int32 _aGain;
    int32 _dGain;
    int32 _irisType;
    int32 _aeTarget;
    int32 _aveLuma;
    int32 _histGrad;
    int32 _stamp;
    int32 _dnSwitchLevel;
    int32 _ndSwitchLevel;
    int32 _baseAdc;
    int32 _count;
    int32 _limit;

    float _irisF;
    float _gain;
    float _predLux;
    float _predLuxByLuma;
    float _baseLux;

    unsigned short* _histInfo;

public :
    enum {
        HISILICON = 0,
        AMBARELLA_A5S,
        AMBARELLA_S2,
    };

    enum {
        NOT_SHOW = 0,
        CONSOLE, 
        HTML,
        CONSOLE_AND_HTML,
    };

public :
    DNChangingInfo& operator=(DNChangingInfo& dnChangingInfo);

    void setLuxPredictionInfo(int32 shutterIdx, int32 ispGain, int32 aGain, int32 dGain, int32 aeTarget, 
                              int32 aveLuma, int32 irisType, float irisF, float shutterIdx1_30, float constK);

    void setLuxPredictionInfo(int32 shutterIdx, float gain, int32 irisType, float irisF) 
    {
        _shutterIdx = shutterIdx;
        _gain       = gain;
        _irisType   = irisType;
        _irisF      = irisF;
    }

    void setIrisInfo(int32 irisType, float irisF)
    {
        _irisType = irisType;
        _irisF    = irisF;
    }

    // For Hisilicon
    void setAllValues(int32 adc, bool irCond, bool isWdrMode, int32 shutterIdx, int32 ispGain, int32 aGain, int32 dGain,
                      int32 aeTarget, int32 irisType, float irisF, int32 aveLuma, bool icrOn, unsigned short* histInfo,
                      int32 dnSwitchLevel, int32 ndSwitchLevel, float shutterIdx1_30, float constK);

    // For Ambarella
    void setAllValues(int32 adc, bool irCond, bool isWdrMode, int32 shutterIdx, float gain, int32 aeTarget,
                      int32 irisType, float irisF, int32 aveLuma, bool icrOn, unsigned short* histInfo);

    void setCurDay(bool curDay)                { _curDay = curDay; }
    void setIrCond(bool irCond)                { _irCond = irCond; }
    void setWdrMode(bool wdrMode)              { _isWdrMode = wdrMode; }
    void setDetectedDay(bool detectedDay)      { _detectedDay = detectedDay; }
    void setIsNightStable(bool isNightStable)  { _isNightStable = isNightStable; }
    void setIcrOn(bool icrOn)                  { _icrOn = icrOn; }
    void setBaseIrCond(bool baseIrCond)        { _baseIrCond = baseIrCond; }
    void setResultDay(bool resultDay)          { _resultDay = resultDay; }
    void setAdc(int32 adc)                     { _adc = adc; }
    void setShutterIdx(int32 shutterIdx)       { _shutterIdx = shutterIdx; }
    void setIspGain(int32 ispGain)             { _ispGain = ispGain; }
    void setAGain(int32 aGain)                 { _aGain = aGain; }
    void setDGain(int32 dGain)                 { _dGain = dGain; }
    void setIrisType(int32 irisType)           { _irisType = irisType; }
    void setAeTarget(int32 aeTarget)           { _aeTarget = aeTarget; }
    void setAveLuma(int32 aveLuma)             { _aveLuma = aveLuma; }
    void setHistGrad(int32 histGrad)           { _histGrad = histGrad; }
    void setStamp(int32 stamp)                 { _stamp = stamp; }
    void setDnSwitchLevel(int32 dnSwitchLevel) { _dnSwitchLevel = dnSwitchLevel; }
    void setNdSwitchLevel(int32 ndSwitchLevel) { _ndSwitchLevel = ndSwitchLevel; }
    void setBaseAdc(int32 baseAdc)             { _baseAdc = baseAdc; }
    void setCount(int32 count)                 { _count = count; }
    void setLimit(int32 limit)                 { _limit = limit; }
    void setIrisF(float irisF)                 { _irisF = irisF; }
    void setGain(float gain)                   { _gain = gain; }
    void setPredLux(float predLux)             { _predLux = predLux; }
    void setPredLuxByLuma(float predLuxByLuma) { _predLuxByLuma = predLuxByLuma; }
    void setBaseLux(float baseLux)             { _baseLux = baseLux; }
    void setHistInfo(unsigned short* histInfo) { _histInfo = histInfo; }

    bool  getCurDay()        { return _curDay; }
    bool  getIrCond()        { return _irCond; }
    bool  getWdrMode()       { return _isWdrMode; }
    bool  getDetectedDay()   { return _detectedDay; }
    bool  getIsNightStable() { return _isNightStable; }
    bool  getIcrOn()         { return _icrOn; }
    bool  getBaseIrCond()    { return _baseIrCond; }
    bool  getResultDay()     { return _resultDay; }
    int32 getAdc()           { return _adc; }
    int32 getShutterIdx()    { return _shutterIdx; }
    int32 getIspGain()       { return _ispGain; }
    int32 getAGain()         { return _aGain; }
    int32 getDGain()         { return _dGain; }
    int32 getIrisType()      { return _irisType; }
    int32 getAeTarget()      { return _aeTarget; }
    int32 getAveLuma()       { return _aveLuma; }
    int32 getHistGrad()      { return _histGrad; }
    int32 getStamp()         { return _stamp; }
    int32 getDnSwitchLevel() { return _dnSwitchLevel; }
    int32 getNdSwitchLevel() { return _ndSwitchLevel; }
    int32 getBaseAdc()       { return _baseAdc; }
    int32 getCount()         { return _count; }
    int32 getLimit()         { return _limit; }
    float getIrisF()         { return _irisF; }
    float getGain()          { return _gain; }
    float getPredLux()       { return _predLux; }
    float getPredLuxByLuma() { return _predLuxByLuma; }
    float getBaseLux()       { return _baseLux; }

    unsigned short* getHistInfo() { return _histInfo; }

    void showDetectionResult(int32 mode);
    void showLuxPredictionInfo(int32 mode);
    void debugLogDnResult();
    
private :
   void copy(DNChangingInfo& dnChangingInfo)
   {
       _curDay        = dnChangingInfo.getCurDay();
       _irCond        = dnChangingInfo.getIrCond();
       _isWdrMode     = dnChangingInfo.getWdrMode();
       _detectedDay   = dnChangingInfo.getDetectedDay();
       _isNightStable = dnChangingInfo.getIsNightStable();
       _icrOn         = dnChangingInfo.getIcrOn();
       _baseIrCond    = dnChangingInfo.getBaseIrCond();
       _resultDay     = dnChangingInfo.getResultDay();

       _adc           = dnChangingInfo.getAdc();
       _shutterIdx    = dnChangingInfo.getShutterIdx();
       _ispGain       = dnChangingInfo.getIspGain();
       _aGain         = dnChangingInfo.getAGain();
       _dGain         = dnChangingInfo.getDGain();
       _irisType      = dnChangingInfo.getIrisType();
       _aeTarget      = dnChangingInfo.getAeTarget();
       _aveLuma       = dnChangingInfo.getAveLuma();
       _histGrad      = dnChangingInfo.getHistGrad();
       _stamp         = dnChangingInfo.getStamp();
       _dnSwitchLevel = dnChangingInfo.getDnSwitchLevel();
       _ndSwitchLevel = dnChangingInfo.getNdSwitchLevel();
       _baseAdc       = dnChangingInfo.getBaseAdc();
       _count         = dnChangingInfo.getCount();
       _limit         = dnChangingInfo.getLimit();

       _irisF         = dnChangingInfo.getIrisF();
       _gain          = dnChangingInfo.getGain();
       _predLux       = dnChangingInfo.getPredLux();
       _predLuxByLuma = dnChangingInfo.getPredLuxByLuma();
       _baseLux       = dnChangingInfo.getBaseLux();
       _histInfo      = dnChangingInfo.getHistInfo();
   }

   float calcCurrGain(int32 ispGain, int32 dGain, int32 aGain);
   float calcPredLux(int32 shutterIdx, float gain, int32 irisType, float irisF, int32 aeTarget, float shutterIdx1_30, float constK);
   float calcPredLuxHi(int32 shutterIdx, float gain, int32 irisType, float irisF, int32 aeTarget, float shutterIdx1_30, float constK);
   float calcPredLuxAm(int32 shutterIdx, float gain, int32 irisType, float irisF, int32 aeTarget);
   float calcPredLuxByLuma(float predLux, int32 aveLuma);

   int32 calcHistGrad(int32 histY0, int32 histY1, int32 histY2, int32 histY3, int32 histY4, int32 aeTarget);
}; // class DNChanginInfo
}
#endif
