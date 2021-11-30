#include "config.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

Config::Config(QObject *parent)
    : QObject(parent) {

}

Config::~Config() {

}

bool Config::load(const QString& fileName) {
    value_map.clear();
    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray byte_array =  file.readAll();
        file.close();
        QJsonParseError json_error;
        QJsonDocument document = QJsonDocument::fromJson(byte_array,&json_error);
        if(json_error.error == QJsonParseError::NoError) {
            if(document.isObject()) {
                QJsonObject json = document.object();
                QJsonObject::ConstIterator it = json.begin();
                while(it != json.end()) {
                    value_map.insert(it.key(), it.value().toVariant());
                    it++;
                }
             }
         }
    }
    return value_map.size() > 0 ? true : false;
}

bool Config::save(const QString& file_name) {
    if(!value_changed) {
        return true;
    }

    QJsonObject json;
    ConfigMap::Iterator it = value_map.begin();
    QJsonValue value;
    while(it != value_map.end()) {
        QJsonValue value = QJsonValue::fromVariant(it.value());
        json.insert(it.key(),value);
        it++;
    }

    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Indented);
    QFile file(file_name);
    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(byte_array);
        file.close();
        file.flush();
        return true;
    }
    return false;
}

QVariant Config::get(const QString& key) const {
    ConfigMap::ConstIterator it = value_map.find(key);
    if(it != value_map.end()) {
        return it.value();
    } else {
        return QVariant();
    }
}

void Config::set(const QString& key, const QVariant& value) {
    value_changed = true;
    value_map.remove(key);
    value_map.insert(key, value);
}
