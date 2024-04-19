#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>
#include <sstream>

// Function to convert float to string with specified precision
std::string floatToString(float num, int precision);

// func to convert int to string
const char *intToString(int value);

// Function to map a float value to an integer value
int map_float_to_int(float input);

// Function to get the battery color based on voltage.
unsigned int getBatteryColor(float voltage, float minVoltage, float maxVoltage);

// Function to generate a random float value in the range [min, max]
float randFloat(float min, float max);

#endif // UTILS_HPP