#include "script_controller.hpp"

//neroshop::ScriptController::Script() {}

//neroshop::ScriptController::~Script() {}
    
#if defined(NEROSHOP_USE_QT)
QString neroshop::ScriptController::getString(const QString& key) const {
    return QString::fromStdString(neroshop::Script::get_string(neroshop::lua_state, key.toStdString()));
}

double neroshop::ScriptController::getNumber(const QString& key) const {
    return neroshop::Script::get_number(neroshop::lua_state, key.toStdString());
}    

bool neroshop::ScriptController::getBoolean(const QString& key) const {
    return neroshop::Script::get_boolean(neroshop::lua_state, key.toStdString());
}

QVariantList neroshop::ScriptController::getTableStrings(const QString& key) const {
    std::vector<std::string> table_strings = neroshop::Script::get_table_string(neroshop::lua_state, key.toStdString());
    QVariantList result;
    for(auto strings : table_strings) {
        result << QString::fromStdString(strings);//std::cout << strings << std::endl;
    }
    return result;
}

// neroshop::ScriptController::get_(const QString& key) const {
    //neroshop::Script::get_(neroshop::lua_state, key.toStdString());
//}
#endif
