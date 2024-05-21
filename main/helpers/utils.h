#ifndef UTILS_H
#define UTILS_H

// Include C++ headers outside of the extern "C" block
#ifdef __cplusplus
#include <iostream>
#include <string>
#include <sstream>
#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif

// C-compatible declarations including the struct
struct GlobalState {
    int CurrentScreen;
    int CurrentPage;
    bool inSettings;
    bool inControl;
    int settingsPage;
    int controlOption;
    int bleSetup;
    bool bleConnected;
    bool upKey;
    bool downKey;
    bool selectKey;
    float batteryPercentage;
    int16_t screenBrightness;
    bool hasSavedDevice;
    int32_t sleepMode;
    bool resetSleepTimer;
};

// Function prototypes that will be implemented in C
const char *intToString(int value);
int map_float_to_int(float input);
unsigned int getBatteryColor(float voltage, float minVoltage, float maxVoltage);
float randFloat(float min, float max);

#ifdef __cplusplus
}
#endif // __cplusplus

// C++ specific declarations can be placed outside `extern "C"`
#ifdef __cplusplus
// Function to convert float to string with specified precision
std::string floatToString(float num, int precision);

class Utils {
    public:
        static float mapFloat(float input, float in_min, float in_max, float out_min, float out_max);
        static int mapFloatToInt(float value, float inputMin, float inputMax, int outputMin, int outputMax);
        static unsigned int getColorRedGreen(float input, float minInput, float maxInput);
        static unsigned int getColorGreenRed(float input, float minInput, float maxInput);
        static unsigned int getColorBlueRed(float input, float minInput, float maxInput);
};

#endif // __cplusplus

#endif // UTILS_H
