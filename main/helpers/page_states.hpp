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

#endif // PAGE_STATES_HPP