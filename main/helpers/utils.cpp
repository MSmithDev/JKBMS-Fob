#include "utils.h"
#include <cmath>

// Function to convert float to string with specified precision
std::string floatToString(float num, int precision) {
    std::ostringstream out;
    out.precision(precision);  // Set the decimal precision
    out << std::fixed << num;  // Ensure fixed point notation for float
    return out.str();
}

// func to convert int to string
const char *intToString(int value)
{
    static std::string str; // Made static to prolong its lifetime beyond the function scope
    std::ostringstream out;
    out << value;
    str = out.str();
    return str.c_str();
}

// Function to map a float value to an integer value

int map_float_to_int(float input)
{
    if (input < 68.0 || input > 84.0)
    {
        // printf("Input out of range. Please enter a value between 68 and 84.\n");
        return 0; // Return an error code if input is out of range
    }

    // Linear mapping from range 68 to 84 to range 0 to 67
    return (int)((input - 68.0) * (67.0 / (84.0 - 68.0)));
}

/**
 * Function to get the battery color based on voltage.
 * @param voltage The current voltage.
 * @param minVoltage The minimum voltage threshold.
 * @param maxVoltage The maximum voltage threshold.
 * @return The color as an unsigned integer in 0xRRGGBB format.
 */
unsigned int getBatteryColor(float voltage, float minVoltage, float maxVoltage)
{
    // Clamp the voltage to the range [minVoltage, maxVoltage]
    voltage = std::max(minVoltage, std::min(maxVoltage, voltage));

    // Normalize the voltage to a [0, 1] range
    float range = maxVoltage - minVoltage;
    float normalized = (voltage - minVoltage) / range;

    // Calculate the red and green components based on the voltage
    // Red decreases as the voltage increases, green increases as the voltage increases
    uint8_t red = static_cast<uint8_t>((1 - normalized) * 255);
    uint8_t green = static_cast<uint8_t>(normalized * 255);

    // Combine the components into a single unsigned integer
    // Format: 0xRRGGBB
    return (red << 16) | (green << 8);
}

// Function to generate a random float in the range [min_val, max_val]
float randFloat(float min_val, float max_val)
{
    // Initialize the random number generator only once
    static int seed_initialized = 0;
    if (!seed_initialized)
    {
        srand(time(NULL)); // Use current time as seed for random generator
        seed_initialized = 1;
    }

    // Generate a random float in the range [0, 1]
    float scale = rand() / (float)RAND_MAX;

    // Adjust the scale to the desired range and return
    return min_val + scale * (max_val - min_val);
}

//New utils go here

float Utils::mapFloat(float input, float in_min, float in_max, float out_min, float out_max)
{
    // Ensure input is within the expected range
    if (input < in_min) input = in_min;
    if (input > in_max) input = in_max;

    // Map the input value to the output range
    return out_min + (out_max - out_min) * ((input - in_min) / (in_max - in_min));
}

// Function to map a float value to an int within a specified range
int Utils::mapFloatToInt(float value, float inputMin, float inputMax, int outputMin, int outputMax) {
    // Ensure the value is within the expected range
    if (value < inputMin) value = inputMin;
    if (value > inputMax) value = inputMax;

    // Map the float value to the integer range and round to nearest integer
    return static_cast<int>(round(outputMin + (outputMax - outputMin) * ((value - inputMin) / (inputMax - inputMin))));
}

// Get the color from red - green based on input values

unsigned int Utils::getColorRedGreen(float input, float minInput, float maxInput)
{
    // Clamp the input to the range [minInput, maxInput]
    input = std::max(minInput, std::min(maxInput, input));

    // Normalize the input to a [0, 1] range
    float range = maxInput - minInput;
    float normalized = (input - minInput) / range;

    // Define colors for low (red), mid (orange), and high (green) levels
    uint8_t red, green, blue;
    if (normalized < 0.5) {
        // Transition from red to orange
        float midNormalized = normalized * 2; // Scale to [0, 1] for half the range
        red = 255;
        green = static_cast<uint8_t>(midNormalized * 165); // Orange at mid-point (165 is the green value in orange color)
        blue = 0;
    } else {
        // Transition from orange to green
        float midNormalized = (normalized - 0.5) * 2; // Adjust to start from 0 again and scale to [0, 1]
        red = static_cast<uint8_t>((1 - midNormalized) * 255); // Decrease red
        green = 165 + static_cast<uint8_t>(midNormalized * (255 - 165)); // Start from orange and go to full green
        blue = 0;
    }

    // Combine the components into a single unsigned integer
    // Format: 0xRRGGBB
    return (red << 16) | (green << 8) | blue;
}

unsigned int Utils::getColorGreenRed(float input, float minInput, float maxInput)
{
    // Clamp the input to the range [minInput, maxInput]
    input = std::max(minInput, std::min(maxInput, input));

    // Normalize the input to a [0, 1] range
    float range = maxInput - minInput;
    float normalized = (input - minInput) / range;

    // Define colors for low (green), mid (orange), and high (red) levels
    uint8_t red, green, blue;
    if (normalized < 0.5) {
        // Transition from green to orange
        float midNormalized = normalized * 2; // Scale to [0, 1] for half the range
        red = static_cast<uint8_t>(midNormalized * 255); // Increase red
        green = 255 - static_cast<uint8_t>(midNormalized * (255 - 165)); // Decrease green starting from full
        blue = 0;
    } else {
        // Transition from orange to red
        float midNormalized = (normalized - 0.5) * 2; // Adjust to start from 0 again and scale to [0, 1]
        red = 255; // Full red
        green = static_cast<uint8_t>((1 - midNormalized) * 165); // Decrease green to 0 from orange
        blue = 0;
    }

    // Combine the components into a single unsigned integer
    // Format: 0xRRGGBB
    return (red << 16) | (green << 8) | blue;
}

unsigned int Utils::getColorBlueRed(float input, float minInput, float maxInput)
{
    // Clamp the input to the range [minInput, maxInput]
    input = std::max(minInput, std::min(maxInput, input));

    // Normalize the input to a [0, 1] range
    float range = maxInput - minInput;
    float normalized = (input - minInput) / range;

    // Define colors for blue at low and red at high
    uint8_t red, green, blue;
    red = static_cast<uint8_t>(normalized * 255); // Red increases as input increases
    green = 0; // No green component
    blue = static_cast<uint8_t>((1 - normalized) * 255); // Blue decreases as input increases

    // Combine the components into a single unsigned integer
    // Format: 0xRRGGBB
    return (red << 16) | (green << 8) | blue;
}