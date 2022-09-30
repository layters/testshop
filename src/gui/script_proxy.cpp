#include "script_proxy.hpp"

//neroshop::gui::Script::Script() {}

//neroshop::gui::Script::~Script() {}
    
#if defined(NEROSHOP_USE_QT)
QString neroshop::gui::Script::get_string(const QString& key) const {
    return QString::fromStdString(neroshop::Script::get_string(neroshop::lua_state, key.toStdString()));
}

double neroshop::gui::Script::get_number(const QString& key) const {
    return neroshop::Script::get_number(neroshop::lua_state, key.toStdString());
}    

bool neroshop::gui::Script::get_boolean(const QString& key) const {
    return neroshop::Script::get_boolean(neroshop::lua_state, key.toStdString());
}

// neroshop::gui::Script::get_(const QString& key) const {
    //neroshop::Script::get_(neroshop::lua_state, key.toStdString());
//}
#endif
