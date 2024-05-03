#ifndef JKBMS_H
#define JKBMS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
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
} JKBMSData;

typedef struct {
    bool connected;
    bool advertising;
    bool scanning;
    char deviceName[20];
    char deviceAddress[20];
    int rssi;
} BLEState;

typedef struct {
    bool startScan;
    bool stopScan;
    bool isConnected;
    bool connect;
    bool disconnect;
} BLEControl;

typedef struct {
    char deviceName[20];
    unsigned char deviceAddress[6];
    int rssi;
} BLEScan;

typedef struct {
    uint16_t characteristic;
    uint8_t data[20];
} BLECmd;

typedef struct {
    uint8_t enableCharge[20];
    uint8_t disableCharge[20];
    uint8_t enableDischarge[20];
    uint8_t disableDischarge[20];
    uint8_t enableBalance[20];
    uint8_t disableBalance[20];
} JKBMSCommands;



extern JKBMSCommands jkbmsCommands;

#endif // JKBMS_H
