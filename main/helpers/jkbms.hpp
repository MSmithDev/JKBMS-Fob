#ifndef JKBMS_HPP
#define JKBMS_HPP


struct JKBMSData{
    float packVoltage;
    float packPower;
    float avgCellVoltage;
    float deltaCellVoltage;
    float cellVoltages[24];
    float cellResistances[24];
};

struct BLEState{
    bool connected;
    bool advertising;
    bool scanning;
    char deviceName[20];
    char deviceAddress[20];
    int rssi;
};


#endif // JKBMS_HPP