#if defined(_AMBARELLA_A5S)
#elif defined(_AMBARELLA_S2)
#else
    #include <Device/AdaptiveDN/DayNightDetector.h>
    #include <Util/AppRuntimeSelection.h>
    #include <App/NCG2ApplicationDelegate.h>
#endif

#if defined(_IDIS_G2_APPLICATION_SUPPORT_LOGGING)
    #include <G2Application/G2DebugLogger.h>
    #include <G2Log/G2DebugLog.h>
    #include <G2Application/G2SystemLogger.h>
    #include <G2Log/G2SystemLog.h>
    #include <G2Log/G2LogDefs.h>
#endif

#define CONSOLE_LOG
#define SHOW_HTML_LOG

using namespace Idis;

DayNightDetector::DayNightDetector()
    : _preDay(true), _irCond(false), _checkICR(false), _checkHistLux(false), _isNightStable(false), _checkStableAdc(true)
    , _checkHist(false)
    , _stamp(0), _reassignValCnt(0), _adcDiagnosisDN(0), _adcDiagnosisND(0), _stableCount(0)
    , _histLux(0.0)
{
    _useLightSensor = false;
#if defined(_LIGHT_SENSOR)
    _useLightSensor = true;
#endif

    _useDnLevel = false;
#if defined(_DN_LEVEL_SUPPORTED)
    _useDnLevel = true;
#endif
    
    _checkDay.init();
    _checkDelay.init();
    _newInfo.init();
    _unstableInfo.init();
    _stableInfo.init();
    _currInfo.init();
    _checkStableBaseInfo.init();
    _icrOffInfo.init();
}

DNChangingInfo& DayNightDetector::resultOfDetection(DNChangingInfo& info)
{
    _stamp = 0;

    bool detectedDay = true;
    _detectionInfo = info;

    if (_useLightSensor) {
        if (_adcDiagnosisDN) {
            detectedDay = processDiagnosisMode(info.getAdc());
            _preDay = detectedDay;
            _detectionInfo.setStamp(_stamp);
            _detectionInfo.setResultDay(detectedDay);
            _detectionInfo.setDetectedDay(detectedDay);
        }
    }

    if ((_preDay != _checkDay.day) && _checkDay.check) {
        int32 limit = _preDay ? _dnDetectorInfo.notCheckNightLimit() : _dnDetectorInfo.notCheckDayLimit();
        if (_checkDay.count >= limit) {
            _checkDay.check = false;
            _checkDay.count = 0;
        }
        else {
            _checkDay.count++;
            detectedDay = _preDay;
            _detectionInfo.setDetectedDay(detectedDay);
            return _detectionInfo;
        }
    }

    _currInfo.irCond = _detectionInfo.getIrCond();
    _currInfo.lux    = _detectionInfo.getPredLuxByLuma();
    _currInfo.adc    = _detectionInfo.getAdc();

    changingUnstableValues();

    detectedDay = dnDetectionProcess();

    _prevInfo.irCond = _currInfo.irCond;
    _prevInfo.lux    = _currInfo.lux;
    _prevInfo.adc    = _currInfo.adc;

#if 0
    printf("DN Changing INFO -> adc(%d), irCond(%s), lux(%.2f)", _currInfo.adc, _irCond ? "IR" : "VR", _currInfo.lux);
    if (_isNightStable) {
        printf(" stable irCond(%s), adc(%d), lux(%.2f)\n", _stableInfo.irCond ? "IR" : "VR", _stableInfo.adc, _stableInfo.lux);
    }
    else {
        printf(" usstable irCond(%s), adc(%d), lux(%.2f)\n", _unstableInfo.irCond ? "IR" : "VR", _unstableInfo.adc, _stableInfo.lux);
    }
#endif
    _detectionInfo.setCurDay(_preDay);
    _detectionInfo.setIsNightStable(_isNightStable);
    _detectionInfo.setBaseIrCond(_isNightStable ? _stableInfo.irCond : _unstableInfo.irCond);
    _detectionInfo.setBaseAdc(_isNightStable ? _stableInfo.adc : _unstableInfo.adc);
    _detectionInfo.setBaseLux(_isNightStable ? _stableInfo.lux : _unstableInfo.lux);
    _detectionInfo.setCount(_checkDelay.count);
    _detectionInfo.setLimit(_checkDelay.limit);
    _detectionInfo.setStamp(_stamp);
    _detectionInfo.setDetectedDay(detectedDay);
    
    detectedDay = checkChangingDelay(detectedDay);
    
    _detectionInfo.setResultDay(detectedDay);

    return _detectionInfo;
}

void DayNightDetector::changingUnstableValues()
{
    bool stateA = _reassignValCnt < _dnDetectorInfo.reassigneNewRefValOnNightLimit();
    int32 adc = _currInfo.adc;
    float lux = _currInfo.lux;
    int32 dnSwitchLevel = _detectionInfo.getDnSwitchLevel();
    float minLux        = _dnDetectorInfo.luxThreshDtoNByIdx(dnSwitchLevel);
    
    _irCond = _currInfo.irCond;
    
    if (!_preDay && !_isNightStable && stateA) {
        int32 maxIrAdc = _unstableInfo.adc + _dnDetectorInfo.maxAdcByIrLight();
        bool  expectedIR = false;

        if ((adc <= maxIrAdc)) {
            expectedIR = true;
            if (_newInfo.adc == -1) {
                if (adc < _unstableInfo.adc) {
                    _newInfo.adc = _unstableInfo.adc;
                }
                else {
                    _newInfo.adc = adc; 
                }
            }
            else {
                int32 tAdc = adc;
                if (adc < _unstableInfo.adc) {
                    tAdc = _unstableInfo.adc;
                }
                    
                _newInfo.adc = ((tAdc * 7) + (_newInfo.adc)) + 4 >> 3;
            }
        }

        // PhotoTR of ZIRCON is not increased by IR LED 
        if (!_useLightSensor) {
#if defined(_HYDRA)
            if (lux - _unstableInfo.lux < 20.0) {
#else
            if (lux < _unstableInfo.lux * 100.0) {
#endif

                if (_newInfo.lux == 0.0) {
                    _newInfo.lux = lux; 
                }
                else {
                    _newInfo.lux = ((_newInfo.lux) + (9.0 * lux)) / 10.0;
                }
            }
        }
        else {
            if (expectedIR) {
                if (_newInfo.lux == 0.0) {
                    _newInfo.lux = lux; 
                }
                else {
                    _newInfo.lux = ((_newInfo.lux) + (9.0 * lux)) / 10.0;
                }
            }
        }

        _reassignValCnt++;

        if (_newInfo.adc != -1) {
            _irCond = true;
        }

        if (!_useLightSensor && _newInfo.lux != 0.0) {
            _irCond = true;
        }
    }
    
    bool stateB = _reassignValCnt >= _dnDetectorInfo.reassigneNewRefValOnNightLimit();
    if (stateB && !_preDay && !_isNightStable) {
        if (_newInfo.adc != -1) {
            _unstableInfo.adc = _newInfo.adc;
            _newInfo.adc = -1;
            if (_useLightSensor) {
                if (_newInfo.lux < minLux) {
                    _unstableInfo.lux = minLux;
                }
                else {
                    _unstableInfo.lux = _newInfo.lux;
                }
                _newInfo.lux = 0.0;
            }
        }
        
        if (!_useLightSensor && (_newInfo.lux != 0.0)) {
            if (_newInfo.lux < minLux) {
                _unstableInfo.lux = minLux;
            }
            else {
                _unstableInfo.lux = _newInfo.lux;
            }
            _newInfo.lux = 0.0;
        }
    }
}

bool DayNightDetector::dnDetectionProcess()
{
    bool  detectedDay   = true;
    bool  isLLL         = checkLowLightLevel();
    float currLux       = _currInfo.lux;
    int32 ndSwitchLevel = _detectionInfo.getNdSwitchLevel();
    int32 dnSwitchLevel = _detectionInfo.getDnSwitchLevel();
    int32 dnLimit       = _dnDetectorInfo.dnChangeDelayDayToNight();
    int32 ndLimit       = _dnDetectorInfo.dnChangeDelayNightToDay();
    float luxThreshDN   = _dnDetectorInfo.luxThreshDtoNByIdx(dnSwitchLevel); 
    float luxThreshND   = _dnDetectorInfo.luxThreshNtoDByIdx(ndSwitchLevel);

    if (_preDay) {
        if (isLLL) {
            detectedDay = false;
            int32 count = _checkDelay.count + 1;
            _checkDelay.setValues(detectedDay, false, count, dnLimit);
            return detectedDay;
        }
        else {
            _checkDelay.init();
            return detectedDay;
        }
    }
    else {
        if (_checkICR) {
            if (currLux > luxThreshND) {
                detectedDay = true;
                int32 count = _checkDelay.count + 1;
                _checkDelay.setValues(detectedDay, false, count, ndLimit);
                return detectedDay;
            }
            else {
                detectedDay = false;
                _checkICR = false;
                _isNightStable = false;
                _unstableInfo.lux = _icrOffInfo.lux;
                _unstableInfo.adc = _icrOffInfo.adc;
                _checkDelay.init();
                _icrOffInfo.init();
                return detectedDay;
            }
        }
        else {
            bool stateA = (_stableInfo.irCond && !_irCond && _useLightSensor && (_stableInfo.adc != -1));
            bool stateB = (_prevInfo.irCond && !_irCond);
            bool stateC = (!_useLightSensor && _stableInfo.irCond && !_irCond && (_stableInfo.lux != 0.0));
            if (stateA || stateB || stateC) {
                _unstableInfo.lux = luxThreshDN;
            }

            if (_isNightStable) {
                detectedDay = dnDetectionProcessStableNight();
            }
            else {
                detectedDay = dnDetectionProcessUnstableNight();
            }

            if (detectedDay) {
                int32 count = _checkDelay.count + 1;
                _checkDelay.setValues(detectedDay, false, count, ndLimit);
                return detectedDay;
            }
            else {
                _checkDelay.init();
                return detectedDay;
            }
        }
    }

    _checkDelay.init();
    return detectedDay;
}

bool DayNightDetector::checkLowLightLevel()
{
    bool isLowLightLevel = false;
    int32 dnSwitchLevel  = _detectionInfo.getDnSwitchLevel();
    int32 histGrad       = _detectionInfo.getHistGrad();
    int32 irisType       = _detectionInfo.getIrisType();
    float currLux        = _currInfo.lux;
    float luxThreshDN    = _dnDetectorInfo.luxThreshDtoNByIdx(dnSwitchLevel);
    float histThreshDN   = _dnDetectorInfo.histThreshDtoNByIdx(dnSwitchLevel);

    bool stateA = currLux < luxThreshDN;
    if (stateA) {
        isLowLightLevel = true;
        _stamp = 1001;
    }
    
    if (histGrad < histThreshDN) {
        if (!_checkHistLux) {
            isLowLightLevel = true;
            _histLux        = currLux;
            _checkHistLux   = true;
            _stamp          = 1002;
        }
        else {
            if (currLux != _histLux) {
                isLowLightLevel = false;
                _checkHistLux   = false;
                _histLux        = 0.0;
                _stamp          = 1003;
            }
            else {
                isLowLightLevel = true;
                _stamp          = 1004;
            }
        }
    }

    return isLowLightLevel;
}

bool DayNightDetector::dnDetectionProcessStableNight()
{
    bool detectedDay = dnDetectionOnNight(_stableInfo);

    _isNightStable = checkUnstableCond();
    
    return detectedDay;
}

bool DayNightDetector::dnDetectionProcessUnstableNight()
{
    bool detectedDay = dnDetectionOnNight(_unstableInfo);
    
    if (!detectedDay) {
        checkNightStable();
    }

    return detectedDay;
}

bool DayNightDetector::dnDetectionOnNight(stSavingInfo& info)
{
    bool  detectedDay   = false;
    bool  bMaxAdc       = false;
    bool  currIrCond    = _irCond;
    float luxW          = _isNightStable ? 1.0 : 2.0;
    float luxThreshND   = info.lux + (getLuxThreshold(info.lux) * luxW);
    float currLux       = _currInfo.lux;
    int32 currAdc       = _currInfo.adc;
    int32 dnSwitchLevel = _detectionInfo.getDnSwitchLevel();
    int32 ndSwitchLevel = _detectionInfo.getNdSwitchLevel();
    int32 maxAdcVal     = _dnDetectorInfo.maxLightSensorValue();
    int32 adcDiff       = _dnDetectorInfo.adcThreshNtoDByIdx(ndSwitchLevel);
    int32 highAdc       = 200;
    
    float minLuxLimit   = _dnDetectorInfo.luxThreshNtoDByIdx(dnSwitchLevel);
    float maxLuxLimit   = 100.0;
    if (dnSwitchLevel != 0) {
        maxLuxLimit = 100.0 * (1.0 + ((float)dnSwitchLevel / 10.0));
        if (maxLuxLimit > 200.0) {
            maxLuxLimit = 200.0;
        }
    }

    if (_useLightSensor) {
        int32 adcThreshND = info.adc + adcDiff;

        if (adcThreshND > maxAdcVal) {
            bMaxAdc = true;
        }

        if (currAdc > adcThreshND) {
            if (!currIrCond && (currLux > luxThreshND)) {
                _stamp = 2000;
                detectedDay = true;
            }
            else if (currIrCond && (currAdc > (adcThreshND + adcDiff))) {
                _stamp = 2001;
                detectedDay = true;
            }
            else if (!currIrCond && (currLux > maxLuxLimit)) {
                _stamp = 2002;
                detectedDay = true;
            }
            else {
                _stamp = 2003;
                detectedDay = false;
            }
        }
        else if (bMaxAdc && !currIrCond && (currLux > (luxThreshND + info.lux))) {
            _stamp = 2004;
            detectedDay = true;
        }
        else if (currIrCond && (currLux > maxLuxLimit) && (currAdc > highAdc)) {
            _stamp = 2005;
            detectedDay = true;
        }
        else {
            _stamp = 2006;
            detectedDay = false;
        }
    }
    else {
        if (!currIrCond && (currLux > luxThreshND)) {
            _stamp = 2007;
            detectedDay = true;
        }
        else if (!currIrCond && (currLux > maxLuxLimit)) {
            _stamp = 2008;
            detectedDay = true;
        }
        else {
            _stamp = 2009;
            detectedDay = false;
        }
    }

    if (currLux < minLuxLimit) {
        _stamp = _stamp + 100;
        detectedDay = false;
    }

    if (!detectedDay) {
        _checkHist = false;
    }

    return detectedDay;
}

bool DayNightDetector::checkUnstableCond()
{
    bool  isNightStable = true;
    int32 dnSwitchLevel = _detectionInfo.getDnSwitchLevel();
    float luxThreshDN   = _dnDetectorInfo.luxThreshDtoNByIdx(dnSwitchLevel);
    float of90pro       = 0.9;

    if (_stableInfo.irCond != _irCond) {
        isNightStable = false;
    }
    else if (_currInfo.lux < (_stableInfo.lux * of90pro)) {
        if (_currInfo.lux > luxThreshDN) {
            isNightStable = false;
        }
    }

    if (!isNightStable) {
        _checkStableAdc = false;
        float newLux = ((_stableInfo.lux + _unstableInfo.lux + (2.0 * _currInfo.lux)) / 4.0);
        if (newLux < luxThreshDN) {
            newLux = luxThreshDN;
        }
        _unstableInfo.lux = newLux;
    }

    return isNightStable;
}

void DayNightDetector::checkNightStable()
{
    bool isAdcStable = false;
    bool isLuxStable = false;
    bool isStable    = false;

    if (_useLightSensor) {
        isAdcStable = checkAdcStable();
    }
    else {
        isAdcStable = true;
    }

    isLuxStable = checkLuxStable();
    isStable    = isAdcStable && isLuxStable;

    if (isStable) {
        _stableCount++;
    }
    else {
        _stableCount = 0;
    }

    if (_stableCount > _dnDetectorInfo.nightStableCntLimit()) {
        setStableState();
    }
}

bool DayNightDetector::checkAdcStable()
{
    bool  stable        = false;
    int32 baseAdc       = _checkStableBaseInfo.adc;
    int32 currAdc       = _currInfo.adc;
    int32 ndSwitchLevel = _detectionInfo.getNdSwitchLevel();
    int32 limit         = _dnDetectorInfo.adcThreshNtoDByIdx(ndSwitchLevel);

    if (baseAdc == -1) {
        stable = true;
        _checkStableBaseInfo.adc = currAdc;
    }
    else {
        int32 diffAdc = baseAdc - currAdc;

        if ((diffAdc > -1 * limit) && (diffAdc < limit)) {
            stable = true;
        }
        else {
            stable = false;
            _checkStableBaseInfo.adc = -1;
        }
    }

    return stable;
}

bool DayNightDetector::checkLuxStable()
{
    bool stable   = false;
    float baseLux = _checkStableBaseInfo.lux;
    float currLux = _currInfo.lux;

    if (baseLux == 0.0) {
        stable = true;
        _checkStableBaseInfo.lux = currLux;
    }
    else {
        if (baseLux == currLux) {
            stable = true;
        }
        else {
            stable = false;
            _checkStableBaseInfo.lux = 0.0;
        }
    }

    return stable;
}

void DayNightDetector::setStableState()
{
    int32 currAdc       = _currInfo.adc;
    float currLux       = _currInfo.lux;
    int32 dnSwitchLevel = _detectionInfo.getDnSwitchLevel();
    float minLux        = _dnDetectorInfo.luxThreshDtoNByIdx(dnSwitchLevel);
    _stableInfo.irCond  = _irCond;

    if (_checkStableAdc) {
        if (currLux < minLux) {
            _stableInfo.lux = minLux;
        }
        else {
            _stableInfo.lux = currLux;
        }

        if (_useLightSensor) {
            if (currAdc <= _unstableInfo.adc) {
                _stableInfo.adc = _unstableInfo.adc;
            }
            else {
                _stableInfo.adc = currAdc;
            }
        }
    }
    else {
        if (currLux < minLux) {
            _stableInfo.lux = minLux;
        }
        else {
            _stableInfo.lux = currLux;
        }
    }

    _stableCount   = 0;
    _isNightStable = true;
}

float DayNightDetector::getLuxThreshold(float lux)
{
    int32 dnSwitchLevel = _detectionInfo.getDnSwitchLevel();
    int32 ndSwitchLevel = _detectionInfo.getNdSwitchLevel();
    float luxThreshDN   = _dnDetectorInfo.luxThreshDtoNByIdx(dnSwitchLevel);
    float luxThreshND   = _dnDetectorInfo.luxThreshNtoDByIdx(ndSwitchLevel);
    float diffThresh    = luxThreshND - luxThreshDN;
    float luxThresh     = 0.0;

    float threshW = lux / 10.0;
    if (threshW > 20.0) {
        threshW = 20;
    }

    luxThresh = diffThresh + threshW;

    return luxThresh;
}

bool DayNightDetector::checkChangingDelay(bool detectedDay)
{
    int32 dnSwitchLevel = _detectionInfo.getDnSwitchLevel();
    float minLux        = _dnDetectorInfo.luxThreshDtoNByIdx(dnSwitchLevel);

#if 0
    printf("<%s:%s(%d)> ** _preDay(%s), detectedDay(%s,%d), count(%d), limit(%d)\n", 
            __FILE__, __FUNCTION__, __LINE__, _preDay ? "DAY" : "NIGHT", detectedDay ? "DAY" : "NIGHT", _stamp, _checkDelay.count, _checkDelay.limit);
#endif
    if (_preDay != detectedDay) {
        if (_checkDelay.count < _checkDelay.limit) {
            if (detectedDay) {
#if defined(_FORCE_ICR_ONOFF)
                _checkICR = true;
#endif
                _icrOffInfo.adc = _currInfo.adc;
                _icrOffInfo.lux = _currInfo.lux;
                _icrOffInfo.irCond = _currInfo.irCond;
            }
            detectedDay = _preDay;
        }
        else {
            _preDay = detectedDay;

            if (!detectedDay) {
                _unstableInfo.adc = _currInfo.adc;
                _unstableInfo.lux = _currInfo.lux;
                if (_unstableInfo.lux < minLux) {
                    _unstableInfo.lux = minLux;
                }
                _reassignValCnt = 0;
                _checkDay.check = true;
                _checkDay.day = false;
                _checkDay.count = 0;
                _checkDay.limit = 0;
                _checkICR = false;
            }
            else {
                _checkDay.day   = true;
                _checkDay.check = true;
                _checkDay.count = 0;
                _checkDay.limit = 0;
                _isNightStable = false;
                _checkStableAdc = true;
                _checkICR = false;
                _stableInfo.init();
                _unstableInfo.init();
                _icrOffInfo.init();
                _newInfo.init();
                _checkStableBaseInfo.init();
            }

            _checkDelay.init();
        }
    }
    else {
        _checkDelay.init();
    }

    return detectedDay;
}

void DayNightDetector::setDNDetectorInfo(DNDetectorInfo dnDetectorInfo)
{
    _dnDetectorInfo.setAllValue(dnDetectorInfo.dnChangeDelayNightToDay(),
                                dnDetectorInfo.dnChangeDelayDayToNight(),
                                dnDetectorInfo.reassigneNewRefValOnNightLimit(),
                                dnDetectorInfo.maxHistSeg(),
                                dnDetectorInfo.maxAdcByIrLight(),
                                dnDetectorInfo.nightStableCntLimit(),
                                dnDetectorInfo.notCheckNightLimit(),
                                dnDetectorInfo.notCheckDayLimit(),
                                dnDetectorInfo.dnLevelCnt(),
                                dnDetectorInfo.maxLightSensorValue(),
                                dnDetectorInfo.shutterIdx1_30(),
                                dnDetectorInfo.standardLuxForLP(),
                                dnDetectorInfo.adcThreshNtoD(),
                                dnDetectorInfo.luxThreshNtoD(),
                                dnDetectorInfo.histThreshDtoN(),
                                dnDetectorInfo.luxThreshDtoN());
}

bool DayNightDetector::processDiagnosisMode(int32 adc)
{
    int32 limit = _dnDetectorInfo.maxAdcByIrLight() * 2;

    if (_preDay) {
        limit = _adcDiagnosisDN;
        if (adc < limit) {
            return false;
        }
    }
    else {
        limit = _adcDiagnosisND;
        if (adc > limit) {
            return true;
        }
    }

    return _preDay;
}
