#ifndef JKBMS_H
#define JKBMS_H


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

struct BLEControl{
    bool startScan;
    bool stopScan;
    bool isConnected;
    bool connect;
    bool disconnect;


};

struct BLEScan{
    char deviceName[20];
    unsigned char deviceAddress[6];
    int rssi;
};



#endif // JKBMS_H