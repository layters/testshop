#include "script_proxy.hpp"

//neroshop::gui::Script::Script() {}

//neroshop::gui::Script::~Script() {}
    
#if defined(NEROSHOP_USE_QT)
QString neroshop::gui::Script::getString(const QString& key) const {
    return QString::fromStdString(neroshop::Script::get_string(neroshop::lua_state, key.toStdString()));
}

double neroshop::gui::Script::getNumber(const QString& key) const {
    return neroshop::Script::get_number(neroshop::lua_state, key.toStdString());
}    

bool neroshop::gui::Script::getBoolean(const QString& key) const {
    return neroshop::Script::get_boolean(neroshop::lua_state, key.toStdString());
}

QVariantList neroshop::gui::Script::getTableStrings(const QString& key) const {
    std::vector<std::string> table_strings = neroshop::Script::get_table_string(neroshop::lua_state, key.toStdString());
    QVariantList result;
    for(auto strings : table_strings) {
        //std::cout << strings << std::endl;
        result << QString::fromStdString(strings);
    }
    return result;
}

// neroshop::gui::Script::get_(const QString& key) const {
    //neroshop::Script::get_(neroshop::lua_state, key.toStdString());
//}
#endif
