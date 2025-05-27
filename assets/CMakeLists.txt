install(
    FILES neroshopd.desktop
    DESTINATION share/applications/
)

if(NEROSHOP_BUILD_GUI)
    install(
        FILES neroshop.desktop
        DESTINATION share/applications/
    )
endif()

if(NEROSHOP_BUILD_CLI)
    install(
        FILES neroshop-console.desktop
        DESTINATION share/applications/
    )
endif()

install(
    FILES neroshop.png
    DESTINATION share/icons/hicolor/256x256/apps
)
