#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

// C-compatible declarations including the struct
struct GlobalState {
    int CurrentScreen;
    int CurrentPage;
    bool inSettings;
    int settingsPage;
    bool inControl;
    int controlOption;
    int bleSetup;
    int bleSelectedDevice;
    bool bleConnected;
    bool upKey;
    bool downKey;
    bool selectKey;
    int batteryPercentage;
};

// Function prototypes that will be implemented in C
const char *intToString(int value);
int map_float_to_int(float input);
unsigned int getBatteryColor(float voltage, float minVoltage, float maxVoltage);
float randFloat(float min, float max);

#ifdef __cplusplus
}
#endif // __cplusplus

// C++ specific includes and declarations outside `extern "C"`
#ifdef __cplusplus
#include <iostream>
#include <string>
#include <sstream>

// Function to convert float to string with specified precision
std::string floatToString(float num, int precision);

#endif // __cplusplus

#endif // UTILS_H
