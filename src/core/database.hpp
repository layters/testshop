#ifndef DATABASE_HPP_NEROSHOP
#define DATABASE_HPP_NEROSHOP

#if defined(NEROSHOP_USE_QT)
#include <QStandardPaths>
#define NEROSHOP_DEFAULT_DATABASE_PATH QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) //(QStandardPaths::AppLocalDataLocation)
#endif

#if !defined(NEROSHOP_USE_QT)
#if defined(__linux__) && !defined(__ANDROID__)
#define NEROSHOP_DEFAULT_DATABASE_PATH "/home/" + neroshop::device::get_user() + "/.config/neroshop" //"/home/" + neroshop::device::get_user() + "/.local/share/neroshop"
#endif
#if defined(_WIN32)
#define NEROSHOP_DEFAULT_DATABASE_PATH "C:/Users/" + neroshop::device::get_user() + "/AppData/Local/neroshop"
#endif
#endif // endif !defined(NEROSHOP_USE_QT)

#define NEROSHOP_DATABASE_NAME "data"
#define NEROSHOP_DATABASE_EXTENSION "sqlite3"
#define NEROSHOP_DATABASE_FILE NEROSHOP_DATABASE_NAME "." NEROSHOP_DATABASE_EXTENSION

#include "database/sqlite.hpp"

#endif
