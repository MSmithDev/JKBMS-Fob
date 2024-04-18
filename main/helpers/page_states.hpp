#ifndef PAGE_STATES_HPP
#define PAGE_STATES_HPP

struct mainScreenState {
    int currentScreen;
    int selectedOption;
    bool scrollable;
};

struct settingsPageState {
    int currentSelection;
    int selectedSetting;
    bool isActivePage;
    bool showOptions;

};

struct infoPageState {
    bool isActivePage;

};

struct controlPageState {
    int selectedOption;
    int selectedSubOption;
    bool isActivePage;

};

struct GlobalState {
    int CurrentScreen;
    int CurrentPage;
    bool inSettings;
    int settingsPage;
    int bleSetup;
    int bleSelectedDevice;
    bool bleConnected;
    bool upKey;
    bool downKey;
    bool selectKey;
    int batteryPercentage;
};

#endif // PAGE_STATES_HPP