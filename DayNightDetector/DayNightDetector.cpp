#include <Step/Time.h>

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

//#define CONSOLE_LOG
#define SHOW_HTML_LOG

using namespace Idis;

DayNightDetector::DayNightDetector()
    : _useLightSensor(false), _useDnLevel(false)
    , _preDay(true)
    , _isAeAuto(true), _isIrisFullOpen(false), _aeTarget(0), _irisType(0), _dnSwitchLevel(0), _ndSwitchLevel(0)
    , _adc(-1), _adcDtoN(-1), _newAdcDtoN(-1), _preAdc(-1), _stabledAdc(-1)
    , _predLux(0.0), _preLux(0.0), _histLux(0.0), _stabledLux(0.0), _luxDtoN(0.0), _newLuxDtoN(0.0)
    , _preEnergy(0), _currEnergy(0)
    , _irCond(false), _isWdrMode(false), _stabledIrCond(false)
    , _histGrad(0), _preHistGrad(-51), _checkHist(false), _checkHistLux(false)
    , _dnChangeDelayCnt(0), _dnChangeDelayLimit(5)
    , _reassignValCnt(0)
    , _isNightStable(false), _nightStableCnt(0), _checkStableAdc(true)
    , _notCheckNight(false), _notCheckNightCnt(0)
    , _notCheckDay(false), _notCheckDayCnt(0)
    , _dayToNightStatus(0)
    , _logTick(0), _print(false)
    , _prevIrCond(false)
    , _stamp(0), _aveLuma(0)
    , _adcDiagnosisDN(0)
    , _adcDiagnosisND(0)
{
}

bool DayNightDetector::getDetectedDay(int32  adc,
                                      bool   irCond,
                                      bool   isWdrMode,
                                      bool   isIrisFullOpen,
                                      uint32 rawEnergy,
                                      int32  aveLuma, 
                                      int32  histY0, int32 histY1, int32 histY2, int32 histY3, int32 histY4)
{
    bool detectedDay = true;

    _stamp = 0;

    if (_useLightSensor) {
        if (_adcDiagnosisDN) {
            return processDiagnosisMode(adc);
        }
    }

    
    // After changed NIGHT to DAY, DNDetector rest during "_dnDetectorInfo.notCheckNightLimit()" second
    if (_notCheckNight) {
        int32 limit = _dnDetectorInfo.notCheckNightLimit();
        if (_notCheckNightCnt >= limit) {
            _notCheckNight = false;
            _notCheckNightCnt = 0;
        }
        else {
            _notCheckNightCnt++;
            return detectedDay;
        }
    }
    
    if (_notCheckDay) {
        int32 limit = _dnDetectorInfo.notCheckDayLimit();
        if (_notCheckDayCnt >= limit) {
            _notCheckDay = false;
            _notCheckDayCnt = 0;
        }
        else {
            _notCheckDayCnt++;
            detectedDay = false;
            return detectedDay;
        }
    }


    _histGrad = calcCurrHistGrad(histY0, histY1, histY2, histY3, histY4);

    readyToDetectDN(irCond, isWdrMode, adc, aveLuma, isIrisFullOpen, rawEnergy);

    printConsolePostDetection();

    detectedDay = dnDetectionProcess();

    _prevIrCond = irCond;
    
    printConsoleAfterDetection(detectedDay);

    detectedDay = checkDNDelay(detectedDay);

    _dayToNightStatus = NORMAL;
    _preHistGrad = _histGrad;

    return detectedDay;
}

bool DayNightDetector::checkDNDelay(bool detectedDay)
{
    if (_preDay != detectedDay) {
        if (_dnChangeDelayCnt < _dnChangeDelayLimit) {
            detectedDay = _preDay;
        }
        else {
            printDebugLog(detectedDay);
            _preDay = detectedDay;

            if (!detectedDay) {
                _adcDtoN        = _adc;
                _luxDtoN        = _luxByLuma;
                if (_luxDtoN < _dnDetectorInfo.luxThreshDtoNByIdx(_dnSwitchLevel)) {
                    _luxDtoN = _dnDetectorInfo.luxThreshDtoNByIdx(_dnSwitchLevel);
                }
                _reassignValCnt = 0;
                _dayToNightInfo.setDayToNightInfo(_adc, _predLux, _histGrad, _irCond, currentTick());
                _notCheckDay = true;
                _notCheckDayCnt = 0;
            }
            else {
                _notCheckNight    = true;
                _notCheckNightCnt = 0;
                _isNightStable    = false;
                _checkStableAdc   = true;
                _stabledIrCond    = false;
                _stabledAdc       = -1;
                _stabledLux       = 0.0;
                _nightToDayInfo.setDayToNightInfo(_adc, _predLux, _histGrad, _irCond, currentTick());
            }
            adjustDelay(true);
        }
    }
    else {
        adjustDelay(true);
    }

    return detectedDay;
}

void DayNightDetector::printDebugLog(bool detectedDay)
{
    Btring32 s, t, u;
    if (detectedDay) {
        s.printf("%s|%s|%d|%d|\n", _irCond ? "I" : "V", _isWdrMode ? "W" : "NW", _adc, _aveLuma);
        t.printf("%.2f|%.2f|->D(%d)\n", _predLux, _luxByLuma, _stamp);
        if (_isNightStable) {
            u.printf("S|%s|%d|%.2f|\n", _stabledIrCond ? "I" : "V", _stabledAdc, _stabledLux);
        }
        else {
            u.printf("U|%d|%.2f|\n", _adcDtoN, _luxDtoN);
        }
        ::appendDebugLog(s);
        ::appendDebugLog(t);
        ::appendDebugLog(u);
    }
    else {
        s.printf("%s|%s|%d|%d|\n", _irCond ? "I" : "V", _isWdrMode ? "W" : "NW", _adc, _aveLuma);
        t.printf("%.2f|%.2f|->N(%d)\n", _predLux, _luxByLuma, _stamp);
        ::appendDebugLog(s);
        ::appendDebugLog(t);
    }
}
int32 DayNightDetector::calcCurrHistGrad(int32 histY0, int32 histY1, int32 histY2, int32 histY3, int32 histY4)
{
    int32 coeff[21][5] = { {  2,  7, 4, 10, 22},  // -10 
                           {  2,  7, 4, 10, 19},  //  -9
                           {  3,  7, 4, 10, 17},  //  -8
                           {  3,  7, 4,  9, 15},  //  -7
                           {  3,  7, 3,  9, 14},  //  -6
                           {  3,  7, 3,  9, 12},  //  -5
                           {  3,  7, 3,  9, 12},  //  -4
                           {  3,  7, 3,  8, 11},  //  -3
                           {  3,  7, 3,  8, 10},  //  -2
                           {  3,  7, 3,  8 , 9},  //   -1
                           {  3,  7, 3,  8 , 8},  //   0
                           {  4,  7, 3,  7,  7},  //   1
                           {  4,  7, 3,  7,  6},  //   2
                           {  4,  7, 3,  7,  6},  //   3
                           {  5,  8, 3,  6,  5},  //   4
                           {  5,  8, 3,  6,  5},  //   5
                           {  6,  8, 3,  6,  4},  //   6
                           {  7,  9, 3,  6,  4},  //   7
                           {  8,  9, 3,  6,  3},  //   8
                           {  9, 13, 3,  6,  3},  //   9
                           { 11, 13, 3,  6,  3}}; //  10

    int32 histGrad = 50;
    int32 aeTarget = _aeTarget + 10;

    int32 hY0 = histY0 * coeff[aeTarget][0];
    int32 hY1 = histY1 * coeff[aeTarget][1];
    int32 hY2 = histY2 * coeff[aeTarget][2];
    int32 hY3 = histY3 * coeff[aeTarget][3];
    int32 hY4 = histY4 * coeff[aeTarget][4];

    int32 histSum = hY0 + hY1 + hY2 + hY3 + hY4;
    float luma0R  = 0.0;
    float luma1R  = 0.0;
    int32 luma0   = hY0 + hY1;
    int32 luma1   = hY2 + hY3 + hY4;

    if (luma0 == 0) {
        luma0R = 0.0;
    }
    else {
        luma0R = ((float)luma0 * 100.0) / (float)histSum;
    }

    if (luma1 == 0) {
        luma1R = 0.0;
    }
    else {
        luma1R = ((float)luma1 * 100.0) / (float)histSum;
    }
    
    histGrad = (int32)(((luma1R - luma0R) / 2.0)+0.5);

    return histGrad;
}

void DayNightDetector::readyToDetectDN(bool irCond, bool isWdrMode, int32 adc, int32 aveLuma, bool isIrisFullOpen, uint32 rawEnergy)
{
    _adc            = adc;
    _irCond         = irCond;
    _isWdrMode      = isWdrMode;
    _isIrisFullOpen = isIrisFullOpen;
    _currEnergy     = rawEnergy;
    _aveLuma        = aveLuma;
    _predLux        = predCurrLux(_irisType, _aeTarget);
    _luxByLuma      = currLuxByLuma(_predLux, aveLuma);
    
    bool stateA = _reassignValCnt < _dnDetectorInfo.reassigneNewRefValOnNightLimit();
    if (!_preDay && !_isNightStable && stateA) {
        int32 maxIrAdc = _adcDtoN + _dnDetectorInfo.maxAdcByIrLight();
        bool  expectedIR = false;

        if ((adc <= maxIrAdc)) {
            expectedIR = true;
            if (_newAdcDtoN == -1) {
                if (adc < _adcDtoN) {
                    _newAdcDtoN = _adcDtoN;
                }
                else {
                    _newAdcDtoN = adc; 
                }
            }
            else {
                int32 tAdc = adc;
                if (adc < _adcDtoN) {
                    tAdc = _adcDtoN;
                }
                    
                _newAdcDtoN = ((tAdc * 7) + (_newAdcDtoN)) + 4 >> 3;
            }
        }

        // PhotoTR of ZIRCON is not increased by IR LED 
        if (!_useLightSensor) {
#if defined(_HYDRA)
            if (_luxByLuma - _luxDtoN < 20.0) {
#else
            if (_luxByLuma < _luxDtoN * 100.0) {
#endif

                if (_newLuxDtoN == 0.0) {
                    _newLuxDtoN = _luxByLuma; 
                }
                else {
                    _newLuxDtoN = ((_newLuxDtoN) + (9.0*_luxByLuma)) / 10.0;
                }
            }
        }
        else {
            if (expectedIR) {
                if (_newLuxDtoN == 0.0) {
                    _newLuxDtoN = _luxByLuma; 
                }
                else {
                    _newLuxDtoN = ((_newLuxDtoN) + (9.0*_luxByLuma)) / 10.0;
                }
            }
        }

        _reassignValCnt++;

        if (_newAdcDtoN != -1) {
            _irCond = true;
        }

        if (!_useLightSensor && _newLuxDtoN != 0.0) {
            _irCond = true;
        }
    }

    bool stateB = _reassignValCnt >= _dnDetectorInfo.reassigneNewRefValOnNightLimit();
    if (stateB && !_preDay && !_isNightStable) {
        if (_newAdcDtoN != -1) {
            _adcDtoN    = _newAdcDtoN;
            _newAdcDtoN = -1;
            if (_useLightSensor) {
                _luxDtoN    = _newLuxDtoN;
                _newLuxDtoN = 0.0;
            }
        }
        
        if (!_useLightSensor && (_newLuxDtoN != 0.0)) {
            _luxDtoN    = _newLuxDtoN;
            _newLuxDtoN = 0.0;
        }
    }
}

bool DayNightDetector::dnDetectionProcess()
{
    bool detectedDay     = true;
    bool isLowLightLevel = checkLowLightLevel();

    if (_preDay) {
        if (isLowLightLevel) {
            detectedDay = false;
            if (_dayToNightStatus == WAIT_TO_CHANGE_NIGHT_BY_IRIS) {
                adjustDelay(false, 5);
            }
            else {
                adjustDelay(false, _dnDetectorInfo.dnChangeDelayDayToNight());
            }

            return detectedDay;
        }
        else {
            adjustDelay(true);
            return detectedDay;
        }
    }
    else {
        bool stateA = (_stabledIrCond && !_irCond && _useLightSensor && _stabledAdc != -1);
        bool stateB = (_prevIrCond && !_irCond);
        bool stateC = (!_useLightSensor && _stabledIrCond && !_irCond && _stabledLux != 0.0);
        if ( stateA || stateB || stateC) {
            _luxDtoN = _dnDetectorInfo.luxThreshDtoNByIdx(_dnSwitchLevel);
        }
        
        if (_isNightStable) {
            detectedDay = dnDetectionProcessStableNight();
        }
        else {
            detectedDay = dnDetectionProcessUnstableNight();
        }

        if (detectedDay) {
            adjustDelay(false, _dnDetectorInfo.dnChangeDelayNightToDay());
            
            return detectedDay;
        }
        else {
            adjustDelay(true);
            
            return detectedDay;
        }
    }

    return detectedDay;
}

bool DayNightDetector::dnDetectionProcessStableNight()
{
    bool detectedDay = dnDetectionOnNight(_stabledAdc, _stabledLux);

    _preAdc = _adc;
    _preLux = _luxByLuma;

    _isNightStable = checkUnstableCond(detectedDay, _stabledLux);
    
    return detectedDay;
}

bool DayNightDetector::dnDetectionProcessUnstableNight()
{
    bool detectedDay = dnDetectionOnNight(_adcDtoN, _luxDtoN);

    if (!detectedDay) {
        checkNightStable();
    }

    return detectedDay;
}

bool DayNightDetector::dnDetectionOnNight(int32 standardAdc, float standardLux)
{
    bool  detectedDay  = false;
    bool  bMaxAdc = false;
    float luxW = _isNightStable ? 1.0 : 2.0;
    float luxThreshold = standardLux + (getLuxThreshold(standardLux) * luxW);

    if (_useLightSensor) {
        int32 adcTh = standardAdc + _dnDetectorInfo.adcThreshNtoDByIdx(_ndSwitchLevel);
        
        if (adcTh > _dnDetectorInfo.maxLightSensorValue()) {
            bMaxAdc = true;
        }

        if (_adc > adcTh) {
            if (!_irCond && (_luxByLuma > luxThreshold)) {
                _stamp = 200;
                detectedDay = true;
            }
            else if (_irCond && (_adc > (adcTh + _dnDetectorInfo.adcThreshNtoDByIdx(_ndSwitchLevel)))) {
                _stamp = 201;
                detectedDay = true;
            }
            else if (!_irCond && (_luxByLuma > 100.0)) {
                _stamp = 202;
                detectedDay = true;
            }
            else {
                _stamp = 203;
                detectedDay = false;
            }
        }
        else if (bMaxAdc && !_irCond && (_luxByLuma > (luxThreshold + standardLux))) {
            _stamp = 204;
            detectedDay = true;
        }
        else if (_irCond && (_luxByLuma > 100.0) && _adc > 200) {
            _stamp = 205;
            detectedDay = true;
        }
        else {
            _stamp = 206;
            detectedDay = false;
        }
    }
    else {
        if (!_irCond && (_luxByLuma > luxThreshold)) {
            _stamp = 207;
            detectedDay = true;
        }
        else if (!_irCond && (_luxByLuma > 100.0)) {
            _stamp = 208;
            detectedDay = true;
        }
        else {
            _stamp = 209;
            detectedDay = false;
        }
    }
    
    float minNightLux =_dnDetectorInfo.luxThreshNtoDByIdx(_ndSwitchLevel);
    
    if ((_luxByLuma < minNightLux)) {
        _stamp = 210;
        detectedDay = false;
    }
    
    if (!detectedDay) {
        _checkHist = false;
    }
    
    return detectedDay;
}

bool DayNightDetector::checkUnstableCond(bool detectedDay, float lux)
{
    bool  isNightStable = true;
    float luxThreshold  = getLuxThreshold(lux);
    float diffLux       = _luxByLuma - luxThreshold;
    
    bool stateA = _adc < (_stabledAdc - _dnDetectorInfo.adcThreshNtoDByIdx(_ndSwitchLevel));
    bool stateB = _adc < (_preAdc - _dnDetectorInfo.adcThreshNtoDByIdx(_ndSwitchLevel));
    
    if (_stabledIrCond != _irCond) {
        isNightStable = false;
    }
    else if (_luxByLuma < (_stabledLux * 0.9)) {
        isNightStable = false;
    }
    
    if (!isNightStable) {
        _checkStableAdc = false;
        _luxDtoN = (_stabledLux + _luxDtoN) / 2.0;
        _print = true;
    }

    return isNightStable;
}

bool DayNightDetector::checkLowLightLevel()
{
    bool  isLowLightLevel = false;
    float luxThreshold    = _dnDetectorInfo.luxThreshDtoNByIdx(_dnSwitchLevel);
    int32 histThreshold   = _dnDetectorInfo.histThreshDtoNByIdx(_dnSwitchLevel);

    bool stateA = _luxByLuma < luxThreshold;
    if (stateA) {
        isLowLightLevel   = true;
        _dayToNightStatus = NORMAL;
        _stamp = 101;
    }

    if (_histGrad < histThreshold) {
        if (!_checkHistLux) {
            _stamp = 102;
            isLowLightLevel = true;
            if (_irisType == 3 || _irisType == 1) {
                _dayToNightStatus = WAIT_TO_CHANGE_NIGHT_BY_IRIS;
            }
            _histLux = _luxByLuma;
            _checkHistLux = true;
        }
        else {
            if (_luxByLuma != _histLux) {
                _stamp = 103;
                isLowLightLevel = false;
                _checkHistLux = false;
                _histLux = 0.0;
            }
            else {
                _stamp = 104;
                isLowLightLevel = true;
            }
        }
    }

    return isLowLightLevel;
}

void DayNightDetector::adjustDelay(bool init, int32 limit)
{
    if (init) {
        _dnChangeDelayCnt   = 0;
        _dnChangeDelayLimit = -1;
    }
    else {
        _dnChangeDelayCnt++;
        _dnChangeDelayLimit = limit;
    }
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
        _nightStableCnt++;
    }
    else {
        _nightStableCnt = 0;
    }

    if (_nightStableCnt > _dnDetectorInfo.nightStableCntLimit()) {
        setStableState();
    }
}

bool DayNightDetector::checkAdcStable()
{
    int32 preAdc = _preAdc;
    bool  stable = false;
    int32 adc    = _adc;
    int32 limit  = _dnDetectorInfo.adcThreshNtoDByIdx(_ndSwitchLevel);
    
    if (preAdc == -1) {
        preAdc = adc;
        stable = true;
    }
    else {
        int32 diffAdc    = preAdc - adc;
        int32 minusLimit = -1 * limit;
        if ((diffAdc > minusLimit) && (diffAdc < limit)) {
            preAdc = (preAdc + adc + 1) >> 1;
            stable = true;
        }
        else {
            preAdc = adc;
            stable = false;
        }
    }
    _preAdc = preAdc;

    return stable;
}

bool DayNightDetector::checkLuxStable()
{
    float preLux = _preLux;
    bool  stable = false;

    if (preLux == 0.0) {
        preLux = _luxByLuma; 
        stable = true;
    }
    else {
        if (preLux == _luxByLuma) {
            stable = true;
        }
        else {
            preLux = _luxByLuma;
            stable = false;
        }
    }

    _preLux = preLux;

    return stable;
}

void DayNightDetector::setStableState()
{
    _stabledIrCond  = _irCond;
    
    if (_checkStableAdc) {
        if (_luxByLuma < _dnDetectorInfo.luxThreshDtoNByIdx(_dnSwitchLevel)) {
            _stabledLux = _dnDetectorInfo.luxThreshDtoNByIdx(_dnSwitchLevel);
        }
        else {
            _stabledLux = _luxByLuma;
        }

        if (_useLightSensor) {
            if (_adc <= _adcDtoN) {
                _stabledAdc = _adcDtoN;
            }
            else {
                _stabledAdc = _adc;
            }
        }
    }
    else {
        if (_luxByLuma < _stabledLux && _luxByLuma > _dnDetectorInfo.luxThreshDtoNByIdx(_dnSwitchLevel)) {
            _stabledLux = _luxByLuma;
        }
    }

    _preAdc    = _stabledAdc;
    _preLux    = _stabledLux;
    _preEnergy = _currEnergy;

    _nightStableCnt = 0;
    _isNightStable = true;
}

float DayNightDetector::getLuxThreshold(float lux)
{
    float luxThreshold = 0.0;
    float luxLevel     = _dnDetectorInfo.luxThreshNtoDByIdx(_ndSwitchLevel) -\
                         _dnDetectorInfo.luxThreshDtoNByIdx(_dnSwitchLevel);

    if (lux < 10.1) {
        luxThreshold = luxLevel;
    }
    else if (lux < 20.1) {
        luxThreshold = luxLevel + 2.0;
    }
    else if (lux < 50.1) {
        luxThreshold = luxLevel + 3.0;
    }
    else if (lux < 100.1) {
        luxThreshold = luxLevel + 5.0;
    }
    else if (lux < 500.1) {
        luxThreshold = luxLevel + 8.0;
    }
    else if (lux < 1000.0) {
        luxThreshold = luxLevel + 15.0;
    }
    else {
        luxThreshold = luxLevel + 20.0;
    }
    
    return luxThreshold;
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

void DayNightDetector::printConsolePostDetection()
{
#if defined(SHOW_HTML_LOG)
    if (!_preDay) {
        if (_isNightStable) {
            HTML_LOG(40, "%s | %s | %s | %d | %d | %d | %.2f | %.2f |  %s | %s | %d | %.2f\n", \
                    _preDay ? "D" : "N", _irCond ? "I" : "V", _isWdrMode ? "W" : "NW", _adc, _histGrad, _aveLuma, _predLux, _luxByLuma, "S",\
                    _stabledIrCond ? "I" : "V", _stabledAdc, _stabledLux);
        }
        else {
            HTML_LOG(40, "%s | %s | %s | %d | %d | %d | %.2f | %.2f | %s | %d | %.2f\n", \
                    _preDay ? "D" : "N", _irCond ? "I" : "V", _isWdrMode ? "W" : "NW", _adc, _histGrad, _aveLuma, _predLux, _luxByLuma,  "U",\
                    _adcDtoN, _luxDtoN);
        }
    }
    else {
        HTML_LOG(40, "%s | %s | %s | %d | %d | %d | %.2f | %.2f\n", \
                _preDay ? "D" : "N", _irCond ? "I" : "V", _isWdrMode ? "W" : "NW", _adc, _histGrad, _aveLuma, _predLux, _luxByLuma);
    }
#endif

#if defined(CONSOLE_LOG)
    if (!_preDay) {
        if (_isNightStable) {
            printf("%s | %s | %s | %d | %d | %d | %.2f | %.2f |  %s | %s | %d | %.2f\n", \
                    _preDay ? "D" : "N", _irCond ? "I" : "V", _isWdrMode ? "W" : "NW", _adc, _histGrad, _aveLuma, _predLux, _luxByLuma, "S",\
                    _stabledIrCond ? "I" : "V", _stabledAdc, _stabledLux);
        }
        else {
            printf("%s | %s | %s | %d | %d | %d | %.2f | %.2f | %s | %d | %.2f\n", \
                    _preDay ? "D" : "N", _irCond ? "I" : "V", _isWdrMode ? "W" : "NW", _adc, _histGrad, _aveLuma, _predLux, _luxByLuma,  "U",\
                    _adcDtoN, _luxDtoN);
        }
    }
    else {
        printf("%s | %s | %s | %d | %d | %d | %.2f | %.2f\n", \
                _preDay ? "D" : "N", _irCond ? "I" : "V", _isWdrMode ? "W" : "NW", _adc, _histGrad, _aveLuma, _predLux, _luxByLuma);
    }
#endif
}

void DayNightDetector::printConsoleAfterDetection(bool detectedDay)
{
#if defined(SHOW_HTML_LOG)
    HTML_LOG(40,"%s(%d)\n", detectedDay ? "DAY" : "NIGHT", _stamp);
#endif
#if defined(CONSOLE_LOG)
    printf("%s(%d)\n", detectedDay ? "DAY" : "NIGHT", _stamp);
#endif

}

bool DayNightDetector::processDiagnosisMode(int32  adc)
{

    int32 limit  = _dnDetectorInfo.maxAdcByIrLight()*2;

    if( _preDay ){
        limit = _adcDiagnosisDN;
        if(adc < limit)
            return false;
    }
    else {
        limit = _adcDiagnosisND;
        if(adc > limit)
            return true;
    }

    return _preDay;
}


///////////////////////////////////////////////////////////////////////////////
DayNightDetectorHi::DayNightDetectorHi()
    : _shutterIdx(0), _gain(0.0), _pIrisF(1.0)
    , _ispGain(0), _aGain(0), _dGain(0)
{
}

#define VAL_TO_GAIN(val)    ((float)val / 1024)
void DayNightDetectorHi::setAeInfo(int32 shutterIdx, int32 ispGain, int32 aGain, int32 dGain, float pIrisF)
{
    _shutterIdx = shutterIdx;
    _ispGain    = ispGain;
    _aGain      = aGain;
    _dGain      = dGain;
    _gain       = VAL_TO_GAIN(ispGain) * VAL_TO_GAIN(aGain) * VAL_TO_GAIN(dGain);
    _pIrisF     = pIrisF;
}

bool DayNightDetectorHi::isDnByDetector(int32  adc,        \ 
                                        bool   irCond,     \ 
                                        bool   isWdrMode,  \
                                        uint32 rawEnergy,  \ 
                                        int32  shutterIdx, \
                                        int32  ispGain,    \
                                        int32  aGain,      \
                                        int32  dGain,      \
                                        int32  irisPos,    \ 
                                        float  pIrisF,     \
                                        int32  aveLuma,    \
                                        unsigned short* histInfo)
{
    bool isIrisFullOpen = false;
    int32 histo[_dnDetectorInfo.maxHistSeg()];
 
    for(int32 i = 0; i < _dnDetectorInfo.maxHistSeg(); i++) {
        histo[i] = (int32)histInfo[i];
    }

    if (irisPos == 0) {
        isIrisFullOpen = true;
    }
    
    setAeInfo(shutterIdx, ispGain, aGain, dGain, pIrisF);

    //printf("shutter : %d gain : %.2f pIrisf : %.2f\n", _shutterIdx, _gain, _pIrisF);
    return getDetectedDay(adc, 
                          irCond,
                          isWdrMode,
                          isIrisFullOpen, 
                          rawEnergy,
                          aveLuma, 
                          histo[0], 
                          histo[1], 
                          histo[2], 
                          histo[3], 
                          histo[4]);
}

float DayNightDetectorHi::predCurrLux(int32 irisType, int32 aeTarget)
{
    float aeTargetW = 1.0;
    if (aeTarget < 0) {
        aeTargetW = -1.0 / (2.0 * aeTarget);
    }
    else if (aeTarget > 0) {
        aeTargetW = 2.0 * aeTarget;
    }
    else {
        aeTargetW = 1.0;
    }
    
    float shutterFactor = _dnDetectorInfo.shutterIdx1_30() / (float)_shutterIdx; // IRON
    float irisFactor    = _pIrisF * _pIrisF;

    float gainFactor    = 1.0;
    if (_gain)
        gainFactor = 1.0 / _gain;
    float constK        = _dnDetectorInfo.standardLuxForLP();

    float lux = 0.0;
    if (irisType == 2 || irisType == 1 || irisType == 0) {
        lux = constK * shutterFactor * gainFactor * aeTargetW;
    }
    else {
        lux = constK * shutterFactor * gainFactor * irisFactor * aeTargetW;
    }

    HTML_LOG(40, "%d | %.2f | %.2f | %.2f | %.2f\n", _shutterIdx, _gain, _pIrisF, constK, aeTarget);
    return lux;
}

float DayNightDetectorHi::currLuxByLuma(float lux, int32 aveLuma)
{
    float luxByLuma = 0.0;
    float lumaF = (float)aveLuma / 50.0;

    if (aveLuma == 255) {
        lumaF = 100;
    }
    else if (aveLuma > 200) {
        lumaF = 2 * lumaF;
    }
    else if (aveLuma > 100) {
        lumaF = 1.5 * lumaF;
    }

    luxByLuma = lux * lumaF;

    return luxByLuma;
}

/////////////////////////////////////////////////////////////////////////////////////
DayNightDetectorAmb::DayNightDetectorAmb()
    : _shutterIdx(0), _gain(0.0), _pIrisF(1.0)
{
}

float DayNightDetectorAmb::predCurrLux(int32 irisType, int32 aeTarget)
{
    int32 cost = _shutterIdx - 1012 - (int32)_gain;
    float lux  = pow(2.0, (float)cost / 128.0) * 12.0;

    return lux;
}

void DayNightDetectorAmb::setAeInfo(int32 shutterIdx, int32 gain, float pIrisF)
{
    _shutterIdx = shutterIdx;
    _gain       = (float)gain;
    _pIrisF     = pIrisF;
}
