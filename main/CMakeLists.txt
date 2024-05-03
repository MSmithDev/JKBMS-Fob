idf_component_register(SRCS "main.cpp"
"gui/screens/main_screen/main_screen.cpp"
"gui/screens/settings_screen/settings_screen.cpp"
"gui/screens/control_screen/control_screen.cpp"
"gui/screens/info_screen/info_screen.cpp"
"helpers/utils.cpp"
"helpers/jkbms.c"
"gui/widgets/widgets.cpp"
"tasks/fob_battery_monitor.c"
"tasks/ble_task.c"
"tasks/gui_task.cpp"

                    INCLUDE_DIRS "."
                    
                    )
