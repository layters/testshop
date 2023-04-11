#include "script_controller.hpp"

#include "../core/script.hpp"
#include "../core/config.hpp" // neroshop::lua_state

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>

neroshop::ScriptController::ScriptController(QObject *parent) : QObject(parent) {
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

//neroshop::ScriptController::~Script() {}

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

QString neroshop::ScriptController::getJsonString(const QString& key) {
    QJsonValue json_value = json_object.value(key);
    assert(json_value.isString());
    return json_value.toString();
}

int neroshop::ScriptController::getJsonInt(const QString& key) {
    QJsonValue json_value = json_object.value(key);
    //assert(json_value.is?());
    return json_value.toInt();
}

bool neroshop::ScriptController::getJsonBool(const QString& key) {
    QJsonValue json_value = json_object.value(key);
    assert(json_value.isBool());
    return json_value.toBool();
}

double neroshop::ScriptController::getJsonDouble(const QString& key) {
    QJsonValue json_value = json_object.value(key);
    assert(json_value.isDouble());
    return json_value.toDouble();
}

QVariantList neroshop::ScriptController::getJsonArray(const QString& key) {
    QJsonValue json_value = json_object.value(key);
    assert(json_value.isArray());
    QJsonArray json_array = json_value.toArray();
    return json_array.toVariantList();
}

QVariantMap neroshop::ScriptController::getJsonObject(const QString& key) {
    QJsonValue json_value = json_object.value(key);
    assert(json_value.isObject());
    QJsonObject json_obj = json_value.toObject();
    return json_obj.toVariantMap();
}

QVariantMap neroshop::ScriptController::getJsonObject() const {
    return json_object.toVariantMap();
}

QString neroshop::ScriptController::getJsonLiteral() {
    QJsonDocument json_doc(json_object);
    return QString(json_doc.toJson(QJsonDocument::Compact));
}

void neroshop::ScriptController::saveJson(const QString& settings) {
    neroshop::modify_json(settings.toStdString());
}
