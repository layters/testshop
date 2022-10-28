#pragma once

#ifndef BACKEND_HPP_NEROSHOP
#define BACKEND_HPP_NEROSHOP

#include <QObject>
#include <QUrl>
#include <QString>
#include <QClipboard>
#include <QGuiApplication>

#include <iostream>

namespace neroshop {
class Backend : public QObject { // This class was created for storing utility functions and backend implementations // Maybe I should rename this to BackendTools?
    Q_OBJECT 
public:
    Q_INVOKABLE QString urlToLocalFile(const QUrl& url) const;
    Q_INVOKABLE void copyTextToClipboard(const QString& text);
private:
};
}
#endif
