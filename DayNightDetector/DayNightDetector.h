#ifndef _DAY_NIGHT_DETECTOR_H
#define _DAY_NIGHT_DETECTOR_H

#include <Step/Type.h>
#include <Device/AdaptiveDN/DayNightDetectorInfo.h>

namespace Idis {

    typedef struct checkDay {
        bool day;
        bool check;
        int32 count;
        int32 limit;

        void init()
        {
            day   = true;
            check = false;
            count = 0;
            limit = 999;
        }

        void setValues(bool d, bool c, int32 cnt, int32 l)
        {
            day   = d;
            check = c;
            count = cnt;
            limit = l;
        }
    } stCheckDay;

    typedef struct savingInfo {
        bool  irCond;
        float lux;
        int32 adc;
        
        void init()
        {
            irCond = false;
            lux    = 0.0;
            adc    = -1;
        }
    } stSavingInfo;

class DayNightDetector {
public :
    DayNightDetector();
    virtual ~DayNightDetector() {}

public :
    DNDetectorInfo _dnDetectorInfo;
    DNChangingInfo  _detectionInfo;

    enum {
        ICR_OFF_BY_CONFIG = 0,
        ICR_ON_BY_CONFIG,
        ICR_OFF_BY_DETECTOR,
        ICR_ON_BY_DETECTOR,
    };

private :
    bool _useLightSensor;
    bool _useDnLevel;
   
    bool _preDay;
    bool _irCond;
    bool _checkICR;
    bool _checkHistLux;
    bool _isNightStable;
    bool _checkStableAdc;
    bool _checkHist;

    int32 _stamp;
    int32 _reassignValCnt;
    int32 _adcDiagnosisDN;
    int32 _adcDiagnosisND;
    int32 _stableCount;

    float _histLux;

    stCheckDay _checkDay;
    stCheckDay _checkDelay;
    savingInfo _newInfo;
    savingInfo _unstableInfo;
    savingInfo _stableInfo;
    savingInfo _currInfo;
    savingInfo _prevInfo;
    savingInfo _checkStableBaseInfo;
    savingInfo _icrOffInfo;

public :
    DNChangingInfo& resultOfDetection(DNChangingInfo& info);

    void setDNDetectorInfo(DNDetectorInfo dnDetectorInfo);
    void setUseLightSensor(bool useLightSensor)            { _useLightSensor = useLightSensor; }
    bool getICR           ()                               { return _checkICR; }
    void setDiagnosis     (int32 dnThresh, int32 ndThresh) { _adcDiagnosisDN = dnThresh, _adcDiagnosisND = ndThresh; }

private :
    void  changingUnstableValues         ();
    bool  dnDetectionProcess             ();
    bool  checkLowLightLevel             ();
    bool  dnDetectionProcessStableNight  ();
    bool  dnDetectionProcessUnstableNight();
    bool  dnDetectionOnNight             (stSavingInfo& info);
    bool  checkUnstableCond              ();
    void  checkNightStable               ();
    bool  checkAdcStable                 ();
    bool  checkLuxStable                 ();
    void  setStableState                 ();
    bool  checkChangingDelay             (bool detectedDay);
    float getLuxThreshold                (float lux);
    bool  processDiagnosisMode           (int32 adc);
}; // class DayNightDetector
} // namespace Idis

#endif // _DAY_NIGHT_DETECTOR_H
