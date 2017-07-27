#include <stdio.h>
#include <math.h>
#if defined(_AMBARELLA_A5S)
#elif defined(_AMBARELLA_S2)
#else
    #include <Device/AdaptiveDN/DayNightDetectorInfo.h>
    #include <Util/AppRuntimeSelection.h>
    #include <App/NCG2ApplicationDelegate.h>
#endif
#include <App/Const.h>

using namespace Idis;

/////////////////////////////////////////////////////////////////////////////////////////
DNDetectorInfo::DNDetectorInfo()
{
    _dnChangeDelayNightToDay        = 7;
    _dnChangeDelayDayToNight        = 3;
    _reassigneNewRefValOnNightLimit = 3;
    _maxHistSeg                     = 5;
    _maxAdcByIrLight                = 13;
    _nightStableCntLimit            = 7;
    _notCheckNightLimit             = 5;
    _dnLevelCnt                     = 8;
    _maxLightSensorValue            = 0;

    _shutterIdx1_30                 = 33274.0;
    _standardLuxForLP               = 32.176;
}

void DNDetectorInfo::setAllValue(int32 dnChangeDelayNightToDay,
                                 int32 dnChangeDelayDayToNight,
                                 int32 reassigneNewRefValOnNightLimit,
                                 int32 maxHistSeg,
                                 int32 maxAdcByIrLight,
                                 int32 nightStableCntLimit,
                                 int32 notCheckNightLimit,
                                 int32 notCheckDayLimit,
                                 int32 dnLevelCnt,
                                 int32 maxLightSensorValue,
                                 float shutterIdx1_30,
                                 float standardLuxForLP,
                                 int32* adcThreshNtoD,
                                 float* luxThreshNtoD,
                                 int32* histThreshDtoN,
                                 float* luxThreshDtoN)
{
    _dnChangeDelayNightToDay        = dnChangeDelayNightToDay;
    _dnChangeDelayDayToNight        = dnChangeDelayDayToNight;
    _reassigneNewRefValOnNightLimit = reassigneNewRefValOnNightLimit;
    _maxHistSeg                     = maxHistSeg;
    _maxAdcByIrLight                = maxAdcByIrLight;
    _nightStableCntLimit            = nightStableCntLimit;
    _notCheckNightLimit             = notCheckNightLimit;
    _notCheckDayLimit               = notCheckDayLimit;
    _dnLevelCnt                     = dnLevelCnt;
    _maxLightSensorValue            = maxLightSensorValue;

    _shutterIdx1_30                 = shutterIdx1_30;
    _standardLuxForLP               = standardLuxForLP;
    
    for (int32 i = 0; i < _dnLevelCnt; i++) {
        _adcThreshNtoD [i] = adcThreshNtoD [i];
        _luxThreshNtoD [i] = luxThreshNtoD [i];
        _histThreshDtoN[i] = histThreshDtoN[i];
        _luxThreshDtoN [i] = luxThreshDtoN [i];
    }
}
/////////////////////////////////////////////////////////////////////////////////////////
DNChangedInfo::DNChangedInfo()
{
    _adc         = 0;
    _lux         = 0.0;
    _histGrad    = 0;
    _irCond      = false;
    _changedTick = 0;
}

void DNChangedInfo::setDayToNightInfo(int32 adc, float lux, int32 histGrad, bool irCond, uint32 changedTick)
{
    _adc         = adc;
    _lux         = lux;
    _histGrad    = histGrad;
    _irCond      = irCond;
    _changedTick = changedTick;
}

/////////////////////////////////////////////////////////////////////////////////////////
DNStateInfo::DNStateInfo()
{
    _adc         = 0;
    _lux         = 0.0;
    _luxByRaw    = 0.0;
    _irCond      = false;
}

void DNStateInfo::setStateInfo(int32 adc, float lux, float luxByRaw, bool irCond)
{
    _adc         = adc;
    _lux         = lux;
    _luxByRaw    = luxByRaw;
    _irCond      = irCond;
}

DNChangingInfo::DNChangingInfo()
    : _curDay(true), _irCond(false), _isWdrMode(false), _detectedDay(true), _isNightStable(false), _icrOn(false)
    , _adc(-1), _shutterIdx(0), _ispGain(0), _aGain(0), _dGain(0), _irisType(0), _aeTarget(0), _aveLuma(0), _histGrad(0)
    , _dnSwitchLevel(0), _ndSwitchLevel(0)
    , _irisF(1.0), _gain(0.0), _predLux(0.0), _predLuxByLuma(0.0)
{
#if defined(_AMBARELLA_A5S)
    _platform = AMBARELLA_A5S;
#elif defined(_AMBARELLA_S2)
    _platform = AMBARELLA_S2;
#else
    _platform = HISILICON;
#endif
}
// For Ambarella
void DNChangingInfo::setAllValues(int32 adc, bool irCond, bool isWdrMode, int32 shutterIdx, float gain, int32 aeTarget,
                                  int32 irisType, float irisF, int32 aveLuma, bool icrOn, unsigned short* histInfo)
{
    _adc           = adc;
    _irCond        = irCond;
    _isWdrMode     = isWdrMode;
    _shutterIdx    = shutterIdx;
    _gain          = gain;
    _irisType      = irisType;
    _aeTarget      = aeTarget;
    _irisF         = irisF;
    _icrOn         = icrOn;
    _histInfo      = histInfo;
    _predLux       = calcPredLux(shutterIdx, gain, irisType, irisF, aeTarget, 0, 0);
    _predLuxByLuma = calcPredLuxByLuma(_predLux, aveLuma);
}

// For Hisilicon
void DNChangingInfo::setAllValues(int32 adc, bool irCond, bool isWdrMode, int32 shutterIdx, int32 ispGain, int32 aGain, int32 dGain,
                                  int32 aeTarget, int32 irisType, float irisF, int32 aveLuma, bool icrOn, unsigned short* histInfo,
                                  int32 dnSwitchLevel, int32 ndSwitchLevel, float shutterIdx1_30, float constK)
{
    _adc           = adc;
    _irCond        = irCond;
    _isWdrMode     = isWdrMode;
    _shutterIdx    = shutterIdx;
    _ispGain       = ispGain;
    _aGain         = aGain;
    _dGain         = dGain;
    _irisType      = irisType;
    _aeTarget      = aeTarget;
    _aveLuma       = aveLuma;
    _irisF         = irisF;
    _icrOn         = icrOn;
    _histInfo      = histInfo;
    _dnSwitchLevel = dnSwitchLevel;
    _ndSwitchLevel = ndSwitchLevel;
    _gain          = calcCurrGain(ispGain, dGain, aGain);
    _predLux       = calcPredLux(shutterIdx, _gain, irisType, irisF, aeTarget, shutterIdx1_30, constK);
    _predLuxByLuma = calcPredLuxByLuma(_predLux, aveLuma);
    _histGrad      = calcHistGrad(histInfo[0], histInfo[1], histInfo[2], histInfo[3], histInfo[4], aeTarget);

#if 0
    printf("<%s:%s(%d)> ** adc(%d), irCond(%s), wdrMode(%s), shutter(%d), ispGain(%d), aGain(%d), dGain(%d)\n", \
            __FILE__, __FUNCTION__, __LINE__, _adc, _irCond ? "IR" : "VR", _isWdrMode ? "ON" : "OFF", \
            _shutterIdx, _ispGain, _aGain, _dGain);
 
    printf("<%s:%s(%d)> ** irisType(%d), aeTarget(%d), aveLuma(%d), irisF(%.2f), icrOn(%s), switchLevel(%d, %d)\n", \
            __FILE__, __FUNCTION__, __LINE__, _irisType, _aeTarget, _aveLuma, _irisF, _icrOn ? "ON" : "OFF", \
            _dnSwitchLevel, _ndSwitchLevel);
   
    printf("<%s:%s(%d)> ** gain(%.2f), _predLux(%.2f), _predLuxByLuma(%.2f), _histGrad(%d)\n", \
            __FILE__, __FUNCTION__, __LINE__, _gain, _predLux, _predLuxByLuma, _histGrad);
#endif

}

// For Hisilicon
void DNChangingInfo::setLuxPredictionInfo(int32 shutterIdx, int32 ispGain, int32 aGain, int32 dGain, int32 aeTarget,\
                                          int32 aveLuma, int32 irisType, float irisF, float shutterIdx1_30, float constK)
{
    _shutterIdx    = shutterIdx;
    _ispGain       = ispGain;
    _aGain         = aGain;
    _dGain         = dGain;
    _aGain         = aGain;
    _irisType      = irisType;
    _aeTarget      = aeTarget;
    _aveLuma       = aveLuma;
    _irisF         = irisF;
    _gain          = calcCurrGain(ispGain, dGain, aGain);
    _predLux       = calcPredLux(shutterIdx, _gain, irisType, irisF, aeTarget, shutterIdx1_30, constK);
    _predLuxByLuma = calcPredLuxByLuma(_predLux, aveLuma);
}

#define VAL_TO_GAIN(val)    ((float)val / 1024)
float DNChangingInfo::calcCurrGain(int32 ispGain, int32 dGain, int32 aGain)
{
    return (VAL_TO_GAIN(ispGain) * VAL_TO_GAIN(aGain) * VAL_TO_GAIN(dGain));

}

float DNChangingInfo::calcPredLux(int32 shutterIdx, float gain, int32 irisType, float irisF, int32 aeTarget, float shutterIdx1_30, float constK)
{
    if (_platform == HISILICON) {
        return calcPredLuxHi(shutterIdx, gain, irisType, irisF, aeTarget, shutterIdx1_30, constK);
    }
    else {
        return calcPredLuxAm(shutterIdx, gain, irisType, irisF, aeTarget);
    }
}

float DNChangingInfo::calcPredLuxAm(int32 shutterIdx, float gain, int32 irisType, float irisF, int32 aeTarget)
{
    int32 cost = _shutterIdx - 1012 - (int32)gain;
    float lux = pow(2.0, (float)cost / 128.0) * 12.0;

    return lux;
}


float DNChangingInfo::calcPredLuxHi(int32 shutterIdx, float gain, int32 irisType, float irisF,\
                                    int32 aeTarget, float shutterIdx1_30, float constK)
{
    float aeW = 1.0;
    if (aeTarget < 0) {
        aeW = -1.0 / (2.0 * aeTarget);
    }
    else if (aeTarget > 0) {
        aeW = 2.0 * aeTarget;
    }

    float gainFactor = 1.0 / gain;
    float shutterFactor = shutterIdx1_30 / (float)_shutterIdx;
    float irisFactor = 1.0;
    if (irisType == 2 || irisType == 1 || irisType == 0) {
        irisFactor = 1.0;
    }
    else {
        irisFactor = irisF * irisF;
    }

    float lux = constK * shutterFactor * gainFactor * irisFactor * aeW;

    return lux;
}

float DNChangingInfo::calcPredLuxByLuma(float predLux, int32 aveLuma)
{
    float luxByLuma = 0.0;
    float lumaW = (float)aveLuma / 50.0;

    if (aveLuma == 255) {
        lumaW = 100.0;
    }
    else if (aveLuma > 200) {
        lumaW = 2.0 * lumaW;
    }
    else if (aveLuma > 100) {
        lumaW = 1.5 * lumaW;
    }
    
    luxByLuma = predLux * lumaW;

    return luxByLuma;
}

int32 DNChangingInfo::calcHistGrad(int32 histY0, int32 histY1, int32 histY2, int32 histY3, int32 histY4, int32 aeTarget)
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
    int32 aeTargetW = _aeTarget + 10;

    int32 hY0 = histY0 * coeff[aeTargetW][0];
    int32 hY1 = histY1 * coeff[aeTargetW][1];
    int32 hY2 = histY2 * coeff[aeTargetW][2];
    int32 hY3 = histY3 * coeff[aeTargetW][3];
    int32 hY4 = histY4 * coeff[aeTargetW][4];

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

DNChangingInfo& DNChangingInfo::operator=(DNChangingInfo& info)
{
    copy(info);

    return *this;
}

void DNChangingInfo::showDetectionResult(int32 mode)
{
    if (mode == NOT_SHOW) {
        return;
    }
    else if (mode == CONSOLE) {
        // irCond | ADC | WDR | histGrad | LUX | STABLE/UNSTABLE | irCond | ADC | LUX | CurDAY -> RESULT(STAMP)
        printf("%s | %d | %s | %d | %.2f | %s | %s | %d | %.2f | %s -> %s(%d)(%d/%d)\n", 
                _irCond ? "IR" : "VR", _adc, _isWdrMode ? "W" : "NW", _histGrad, _predLuxByLuma, _isNightStable ? "S" : "US", \
                _baseIrCond ? "IR" : "VR", _baseAdc, _baseLux, _curDay ? "DAY" : "NIGHT", _detectedDay ? "DAY" : "NIGHT",\
                _stamp, _count, _limit);
    }
    else if (mode == HTML) {
        // irCond | ADC | WDR | LUX | STABLE/UNSTABLE | irCond | ADC | LUX | CurDAY -> RESULT(STAMP)
        HTML_LOG(80, "%s | %d | %s | %d | %.2f | %s | %s | %d | %.2f | %s -> %s(%d)(%d/%d)\n", 
                _irCond ? "IR" : "VR", _adc, _isWdrMode ? "W" : "NW", _histGrad, _predLuxByLuma, _isNightStable ? "S" : "US", \
                _baseIrCond ? "IR" : "VR", _baseAdc, _baseLux, _curDay ? "DAY" : "NIGHT", _detectedDay ? "DAY" : "NIGHT",\
                _stamp, _count, _limit);
    }
    else {
        // irCond | ADC | WDR | histGrad | LUX | STABLE/UNSTABLE | irCond | ADC | LUX | CurDAY -> RESULT(STAMP)
        printf("%s | %d | %s | %d | %.2f | %s | %s | %d | %.2f | %s -> %s(%d)(%d/%d)\n", 
                _irCond ? "IR" : "VR", _adc, _isWdrMode ? "W" : "NW", _histGrad, _predLuxByLuma, _isNightStable ? "S" : "US", \
                _baseIrCond ? "IR" : "VR", _baseAdc, _baseLux, _curDay ? "DAY" : "NIGHT", _detectedDay ? "DAY" : "NIGHT",\
                _stamp, _count, _limit);
        
        HTML_LOG(80, "%s | %d | %s | %d | %.2f | %s | %s | %d | %.2f | %s -> %s(%d)(%d/%d)\n", 
                _irCond ? "IR" : "VR", _adc, _isWdrMode ? "W" : "NW", _histGrad, _predLuxByLuma, _isNightStable ? "S" : "US", \
                _baseIrCond ? "IR" : "VR", _baseAdc, _baseLux, _curDay ? "DAY" : "NIGHT", _detectedDay ? "DAY" : "NIGHT",\
                _stamp, _count, _limit);
    }
}

void DNChangingInfo::showLuxPredictionInfo(int32 mode)
{
    if (mode == NOT_SHOW) {
        return;
    }
    else if (mode == CONSOLE) {
        // shutter | gain | aveLuma | irisType | irisF | predLux | predLuxByLuma
        printf("%d | %.2f | %d | %d | %.2f | %.2f | %.2f\n", \
                _shutterIdx, _gain, _aveLuma, _irisType, _irisF, _predLux, _predLuxByLuma);
    }
    else if (mode == HTML) {
        // shutter | gain | aveLuma | irisType | irisF | predLux | predLuxByLuma
        HTML_LOG(50, "%d | %.2f | %d | %d | %.2f | %.2f | %.2f\n", \
                _shutterIdx, _gain, _aveLuma, _irisType, _irisF, _predLux, _predLuxByLuma);
    }
    else {
        // shutter | gain | aveLuma | irisType | irisF | predLux | predLuxByLuma
        printf("%d | %.2f | %d | %d | %.2f | %.2f | %.2f\n", \
                _shutterIdx, _gain, _aveLuma, _irisType, _irisF, _predLux, _predLuxByLuma);
        
        HTML_LOG(50, "%d | %.2f | %d | %d | %.2f | %.2f | %.2f\n", \
                _shutterIdx, _gain, _aveLuma, _irisType, _irisF, _predLux, _predLuxByLuma);
    }
}

void DNChangingInfo::debugLogDnResult()
{
    Btring32 s, t, u;
    if (_resultDay) {
        s.printf("%s|%s|%d|%d|\n", _irCond ? "I" : "V", _isWdrMode ? "W" : "NW", _adc, _aveLuma);
        t.printf("%.2f|%.2f|->D(%d)\n", _predLux, _predLuxByLuma, _stamp);
        if (_isNightStable) {
            u.printf("S|%s|%d|%.2f|\n", _baseIrCond ? "I" : "V", _baseAdc, _baseLux);
        }
        else {
            u.printf("U|%s|%d|%.2f|\n", _baseIrCond ? "I" : "V", _baseAdc, _baseLux);
        }

        ::appendDebugLog(s);
        ::appendDebugLog(t);
        ::appendDebugLog(u);
    }
    else {
        s.printf("%s|%s|%d|%d|\n", _irCond ? "I" : "V", _isWdrMode ? "W" : "NW", _adc, _aveLuma);
        t.printf("%.2f|%.2f|->N(%d)\n", _predLux, _predLuxByLuma, _stamp);

        ::appendDebugLog(s);
        ::appendDebugLog(t);
    }
}
