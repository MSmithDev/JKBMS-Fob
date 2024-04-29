#ifndef JKBMS_H
#define JKBMS_H


struct JKBMSData{
    float packVoltage;
    float packPower;
    float packCurrent;
    int packPercentage;
    float avgCellVoltage;
    float deltaCellVoltage;
    float cellVoltages[24];
    float cellResistances[24];
    float balanceCurrent;
    int balanceAction;
    float cycleAh;
    int cycleCount;
    float cycleCapacity;
    float capacityRemaining;
    float nominalCapacity;
    bool canCharge;
    bool canDischarge;
    bool canBalance;
    int maxVCell;
    int minVCell;
    int runTimeSec;
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

struct BLECmd {
    uint16_t characteristic;
    uint8_t data[22];
};



#endif // JKBMS_H