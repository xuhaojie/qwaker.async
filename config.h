#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <QObject>
#include <QVariant>
#include <qmap.h>

class Config : public QObject {
    Q_OBJECT
//    Q_ENUMS(GenerateAlgorithm)
    //Q_PROPERTY(float Speed)


public:
    explicit Config(QObject *parent = 0);
    ~Config();
    Q_INVOKABLE bool load(const QString& fileName);
    Q_INVOKABLE bool save(const QString& fileName);
    Q_INVOKABLE QVariant get(const QString& key)const;
    Q_INVOKABLE void set(const QString& key, const QVariant& value);

protected:
    typedef QMap<QString,QVariant> ConfigMap;
    ConfigMap value_map;
    bool value_changed;
};

#endif // __CONFIG_H__
