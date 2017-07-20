#ifndef _DAY_NIGHT_DETECTOR_H_
#define _DAY_NIGHT_DETECTOR_H_
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Step/Type.h>
#include <Device/AdaptiveDN/DayNightDetectorInfo.h>

namespace Idis {

class DayNightDetector {
public :
    DayNightDetector();
    virtual ~DayNightDetector() {}

public :
    DNDetectorInfo _dnDetectorInfo;

private :
    // Device별로 한번만 설정해주는 변수
    bool _useLightSensor;
    bool _useDnLevel;

    // About Video Setup
    bool  _isAeAuto;
    bool  _isIrisFullOpen;
    int32 _aeTarget;
    int32 _irisType;
    int32 _dnSwitchLevel;
    int32 _ndSwitchLevel;
    int32 _stamp;
    int32 _aveLuma;

    // Variable about detection DN
    bool _preDay;

    // Variable about PhotoTR
    int32 _adc;
    int32 _adcDtoN;
    int32 _newAdcDtoN;
    int32 _preAdc;
    int32 _stabledAdc;
    
    // Variable about Lux
    float _predLux;
    float _preLux;
    float _histLux;
    float _stabledLux;
    float _luxDtoN;
    float _newLuxDtoN;
    float _luxByLuma;

    // Variable about Raw Energy
    uint32 _preEnergy;
    uint32 _currEnergy;

    // Variable about IR Condition
    bool _irCond;
    bool _isWdrMode;
    bool _stabledIrCond;

    // Variable about Histogram Gradient
    int32 _histGrad;
    int32 _preHistGrad;
    bool  _checkHist;
    bool  _checkHistLux;

    // Variable about DN Change Delay
    int32 _dnChangeDelayCnt;
    int32 _dnChangeDelayLimit;

    // Variable about Force-IR Detection
    int32 _reassignValCnt;

    // Variable about Night Stable Statement
    bool  _isNightStable;
    bool  _checkStableAdc;
    int32 _nightStableCnt;

    // Variable about waiting until the AE is stabled
    bool  _notCheckNight;
    int32 _notCheckNightCnt;

    bool  _notCheckDay;
    int32 _notCheckDayCnt;

    bool _prevIrCond;

    int32    _adcDiagnosisDN;
    int32    _adcDiagnosisND;
#if 1
    // They are being consentrated add on ASAAD2
    int32 _dayToNightStatus;
    enum {
        NORMAL  = 0,
        WAIT_TO_CHANGE_NIGHT_BY_IRIS,
    };

    DNChangedInfo _dayToNightInfo;
    DNChangedInfo _nightToDayInfo;
#endif

    uint32 _logTick;
    bool   _print;
public :
    void setUseLightSensor(bool   useLightSensor) { _useLightSensor = useLightSensor; }
    void setIrCond        (bool   irCond)         { _irCond         = irCond; }
    void setIsIrisFullOpen(bool   isIrisFullOpen) { _isIrisFullOpen = isIrisFullOpen; }
    void setRawEnergy     (uint32 rawEnergy)      { _currEnergy     = rawEnergy; }
    void setIrisType      (int32  irisType)       { _irisType       = irisType; }
    void setDnSwitchLevel (int32  dnSwitchLevel)  { _dnSwitchLevel  = dnSwitchLevel; }
    void setNdSwitchLevel (int32  ndSwitchLevel)  { _ndSwitchLevel  = ndSwitchLevel; }
    void setAeTarget      (int32  aeTarget)       { _aeTarget       = aeTarget; }

    void setDNDetectorInfo(DNDetectorInfo dnDetectorInfo);

    void setAdcAndHistInfo(int32 adc, int32 histY0, int32 histY1, int32 histY2, int32 histY3, int32 histY4);

    bool getDetectedDay(int32  adc, 
                        bool   irCond,
                        bool   isWdrMode,
                        bool   isIrisFullOpen,
                        uint32 rawEnergy,
                        int32  aveLuma,
                        int32  histY0, int32 histY1, int32 histY2, int32 histY3, int32 histY4);
    bool checkDNDelay(bool detectedDay);

    void printConsolePostDetection ();
    void printConsoleAfterDetection(bool detectedDay);
    void printDebugLog(bool detectedDay);

    void setDiagnosis(int32 dnThresh, int32 ndThresh) { _adcDiagnosisDN=dnThresh; _adcDiagnosisND=ndThresh; }

private :
    bool dnDetectionProcess();
    bool dnDetectionProcessStableNight();
    bool dnDetectionProcessUnstableNight();
    bool checkLowLightLevel();
    void checkNightStable();
    bool checkAdcStable();
    bool checkLuxStable();
    void setStableState();
    
    int32 calcCurrHistGrad  (int32 histY0, int32 histY1, int32 histY2, int32 histY3, int32 histY4);
    void  readyToDetectDN   (bool irCond, bool isWdrMode, int32 adc, int32 aveLuma, 
                             bool isIrisFullOpen, uint32 rawEnergy);
    bool  dnDetectionOnNight(int32 standardAdc, float standardLux); 
    bool  checkUnstableCond (bool  detectedDay, float lux);
    void  adjustDelay       (bool init, int32 limit = -1);
    float getLuxThreshold   (float lux);

    bool processDiagnosisMode(int32  adc);
private :
    virtual float predCurrLux(int32 irisType, int32 aeTarget) = 0;
    virtual float currLuxByLuma(float lux, int32 aveLuma) = 0;
    virtual int32 getShutterIdx() = 0;
    virtual float getGain()       = 0;
};

class DayNightDetectorHi : public DayNightDetector {
public :
    DayNightDetectorHi();
    virtual ~DayNightDetectorHi() {}

private :
    int32 _shutterIdx;
    float _gain;
    float _pIrisF;

    int32 _ispGain;
    int32 _aGain;
    int32 _dGain;

public :
    bool isDnByDetector(int32  adc, 
                        bool   irCond, 
                        bool   isWdrMode,
                        uint32 rawEnergy, 
                        int32  shutterIdx, 
                        int32  ispGain, 
                        int32  aGain, 
                        int32  dGain, 
                        int32  irisPos, 
                        float  pIrisF, 
                        int32  aveLuma,
                        unsigned short* histInfo);
    

private :
    float predCurrLux(int32 irisType, int32 aeTarget);
    float currLuxByLuma(float lux, int32 aveLuma);

    int32 getShutterIdx() { return _shutterIdx; }
    float getGain()       { return _gain; }
    float getPIrisF()     { return _pIrisF; }
    
    void setPIrisF(float pIrisF) { _pIrisF = pIrisF; }

    void setAeInfo(int32 shutterIdx, int32 ispGain, int32 aGain, int32 dGain, float pIrisF = 1.0);
};
   
class DayNightDetectorAmb : public DayNightDetector {
public :
    DayNightDetectorAmb();
    virtual ~DayNightDetectorAmb() {}

private :
    int32 _shutterIdx;
    float _gain;
    float _pIrisF;

public :
    void setPIrisF(float pIrisF) { _pIrisF = pIrisF; }
    
    void setAeInfo(int32 shutterIdx, int32 gain, float pIrisF = 1.0);

private :
    float predCurrLux(int32 irisType, int32 aeTarget);
    int32 getShutterIdx() { return _shutterIdx; }
    float getGain()       { return _gain; }
    float getPIrisF()     { return _pIrisF; }
};
}
#endif
