#include "utils.h"


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