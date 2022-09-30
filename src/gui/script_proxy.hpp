#pragma once

#ifndef SCRIPT_PROXY_HPP_NEROSHOP
#define SCRIPT_PROXY_HPP_NEROSHOP

#if defined(NEROSHOP_USE_QT)
#include <QObject>
#include <QString>
#endif

#include "../core/script.hpp"
#include "../core/config.hpp" // neroshop::lua_state

namespace neroshop {

namespace gui { 

#if defined(NEROSHOP_USE_QT)
class Script : public QObject {
    Q_OBJECT 
    // properties (for use in QML)
    //Q_PROPERTY(<type> <variable_name> READ ...)
public:    
    // functions (for use in QML)
    //Q_INVOKABLE <type> get_table(const QString& key) const {}    
    Q_INVOKABLE QString get_string(const QString& key) const;// {}   
    //Q_INVOKABLE <type> get_userdata(const QString& key) const {} 
    Q_INVOKABLE double get_number(const QString& key) const;// {}    
    Q_INVOKABLE bool get_boolean(const QString& key) const;// {}    
    //Q_INVOKABLE <type> get_(const QString& key) const {}    
#else
class Script {    
#endif
public:
    //Script();
    //~Script();
};

}

}

#endif
