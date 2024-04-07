#include "script_manager.hpp"

#include "../core/tools/script.hpp"
#include "../core/settings.hpp" // neroshop::lua_state

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>

neroshop::ScriptManager::ScriptManager(QObject *parent) : QObject(parent) {
    neroshop::create_json(); // Create settings.json if does not yet exist
    QString json = QString::fromStdString(neroshop::load_json());
    QJsonParseError json_error;
    const auto json_doc = QJsonDocument::fromJson(json.toUtf8(), &json_error);
    if(json_error.error != QJsonParseError::NoError) {
        throw std::runtime_error("Error parsing settings.json");
    }
    assert(json_doc.isObject());
    json_object = json_doc.object();
}

//neroshop::ScriptManager::~Script() {}

QString neroshop::ScriptManager::getString(const QString& key) const {
    return QString::fromStdString(neroshop::Script::get_string(neroshop::lua_state, key.toStdString()));
}

double neroshop::ScriptManager::getNumber(const QString& key) const {
    return neroshop::Script::get_number(neroshop::lua_state, key.toStdString());
}    

bool neroshop::ScriptManager::getBoolean(const QString& key) const {
    return neroshop::Script::get_boolean(neroshop::lua_state, key.toStdString());
}

QVariantList neroshop::ScriptManager::getTableStrings(const QString& key) const {
    std::vector<std::string> table_strings = neroshop::Script::get_table_string(neroshop::lua_state, key.toStdString());
    QVariantList result;
    for(auto strings : table_strings) {
        result << QString::fromStdString(strings);//std::cout << strings << std::endl;
    }
    return result;
}

// neroshop::ScriptManager::get_(const QString& key) const {
    //neroshop::Script::get_(neroshop::lua_state, key.toStdString());
//}

QString neroshop::ScriptManager::getJsonString(const QString& key) {
    QJsonValue json_value = json_object.value(key);
    assert(json_value.isString());
    return json_value.toString();
}

int neroshop::ScriptManager::getJsonInt(const QString& key) {
    QJsonValue json_value = json_object.value(key);
    //assert(json_value.is?());
    return json_value.toInt();
}

bool neroshop::ScriptManager::getJsonBool(const QString& key) {
    QJsonValue json_value = json_object.value(key);
    assert(json_value.isBool());
    return json_value.toBool();
}

double neroshop::ScriptManager::getJsonDouble(const QString& key) {
    QJsonValue json_value = json_object.value(key);
    assert(json_value.isDouble());
    return json_value.toDouble();
}

QVariantList neroshop::ScriptManager::getJsonArray(const QString& key) {
    QJsonValue json_value = json_object.value(key);
    assert(json_value.isArray());
    QJsonArray json_array = json_value.toArray();
    return json_array.toVariantList();
}

QVariantMap neroshop::ScriptManager::getJsonObject(const QString& key) {
    QJsonValue json_value = json_object.value(key);
    assert(json_value.isObject());
    QJsonObject json_obj = json_value.toObject();
    return json_obj.toVariantMap();
}

QVariantMap neroshop::ScriptManager::getJsonRootObject() const {
    return json_object.toVariantMap();
}

QString neroshop::ScriptManager::getJsonLiteral() {
    QJsonDocument json_doc(json_object);
    return QString(json_doc.toJson(QJsonDocument::Compact));
}

void neroshop::ScriptManager::saveJson(const QString& settings) {
    neroshop::modify_json(settings.toStdString());
}

QJsonObject neroshop::ScriptManager::getJsonRootObjectCpp() const {
    return json_object;
}
