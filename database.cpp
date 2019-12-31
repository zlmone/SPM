#include "database.h"

#include <QSettings>
#include <QCoreApplication>
#include <QDebug>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>

Database::Database(QObject *parent) : QObject(parent)
{
    QString writablePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if(writablePath.isEmpty()) qFatal("Cannot determine settings storage location");

    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if(!dir.exists())
    {
        dir.mkpath(".");
    }

    loadConfig();
    loadScreenParams();
    loadFilterList();
}

void Database::loadConfig()
{
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + CONFIGFILE, QSettings::IniFormat);
    setting.degiroCSV = settings.value("DeGiro/path", "").toString();
    setting.CSVdelimeter = static_cast<eDELIMETER>(settings.value("DeGiro/delimeter", 0).toInt());
    setting.degiroAutoLoad = settings.value("DeGiro/Dautoload", false).toBool();
    setting.lastScreenerIndex = settings.value("Screener/lastScreenerIndex", -1).toInt();
    setting.filterON = settings.value("Screener/filterON", false).toBool();
    setting.screenerAutoLoad = settings.value("Screener/Sautoload", false).toBool();
    setting.width = settings.value("General/width", 0).toInt();
    setting.height = settings.value("General/height", 0).toInt();
    setting.xPos = settings.value("General/xPos", 0).toInt();
    setting.yPos = settings.value("General/yPos", 0).toInt();
    setting.lastOpenedTab = settings.value("General/lastOpenedTab", 0).toInt();
    setting.currency = static_cast<eCURRENCY>(settings.value("General/currency", 0).toInt());

    QDate currentDate = QDate::currentDate();
    currentDate = currentDate.addDays(-1);
    qDebug() << currentDate.toString("dd.MM.yyyy");
    setting.lastExchangeRatesUpdate = QDate::fromString(settings.value("Exchange/lastExchangeRatesUpdate", currentDate.toString("dd.MM.yyyy")).toString(), "dd.MM.yyyy");
    setting.EUR2USD = settings.value("Exchange/EUR2USD", 1.12).toDouble();
    setting.CZK2USD = settings.value("Exchange/CZK2USD", 0.044).toDouble();
    setting.USD2CZK = settings.value("Exchange/USD2CZK", 22.68).toDouble();
    setting.USD2EUR = settings.value("Exchange/USD2EUR", 0.89).toDouble();
}

void Database::saveConfig()
{
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + CONFIGFILE, QSettings::IniFormat);
    settings.setValue("DeGiro/path", setting.degiroCSV);
    settings.setValue("DeGiro/delimeter", setting.CSVdelimeter);
    settings.setValue("DeGiro/Dautoload", setting.degiroAutoLoad);

    settings.setValue("Screener/lastScreenerIndex", setting.lastScreenerIndex);
    settings.setValue("Screener/filterON", setting.filterON);
    settings.setValue("Screener/Sautoload", setting.screenerAutoLoad);

    settings.setValue("General/width", setting.width);
    settings.setValue("General/height", setting.height);
    settings.setValue("General/xPos", setting.xPos);
    settings.setValue("General/yPos", setting.yPos);
    settings.setValue("General/lastOpenedTab", setting.lastOpenedTab);
    settings.setValue("General/currency", setting.currency);

    settings.setValue("Exchange/lastExchangeRatesUpdate", setting.lastExchangeRatesUpdate.toString("dd.MM.yyyy"));
    settings.setValue("Exchange/USD2EUR", setting.USD2EUR);
    settings.setValue("Exchange/CZK2USD", setting.CZK2USD);
    settings.setValue("Exchange/USD2CZK", setting.USD2CZK);
    settings.setValue("Exchange/USD2EUR", setting.USD2EUR);
}

void Database::loadScreenParams()
{
    QFile qFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + SCREENERPARAMSFILE);

    if(qFile.exists())
    {
        if (qFile.open(QIODevice::ReadOnly))
        {
            QDataStream in(&qFile);
            in >> setting.screenerParams;
            qFile.close();
        }
    }

    setEnabledScreenerParams();
}

void Database::saveScreenerParams()
{
    QFile qFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + SCREENERPARAMSFILE);
    if (qFile.open(QIODevice::WriteOnly))
    {
        QDataStream out(&qFile);
        out << setting.screenerParams;
        qFile.close();
    }
}

QDataStream &operator<<(QDataStream &out, const sSCREENERPARAM &param)
{
    out << param.name;
    out << param.enabled;

    return out;
}

QDataStream &operator>>(QDataStream &in, sSCREENERPARAM &param)
{
    in >> param.name;
    in >> param.enabled;

    return in;
}

QString Database::getCurrencyText(eCURRENCY currency)
{
    switch (currency)
    {
        case CZK: return "CZK";
        case EUR: return "EUR";
        case USD: return "USD";
    }

    return "";
}

// Getters and Setters
sSETTINGS Database::getSetting() const
{
    return setting;
}

void Database::setSetting(const sSETTINGS &value)
{
    setting = value;
    saveConfig();
}

QString Database::getDegiroCSV() const
{
    return setting.degiroCSV;
}

void Database::setDegiroCSV(const QString &value)
{
    setting.degiroCSV = value;
    saveConfig();
}

int Database::getLastScreenerIndex() const
{
    return setting.lastScreenerIndex;
}

void Database::setLastScreenerIndex(const int &value)
{
    setting.lastScreenerIndex = value;
    saveConfig();
}

QVector<sSCREENERPARAM> Database::getScreenerParams() const
{
    return setting.screenerParams;
}

void Database::setScreenerParams(const QVector<sSCREENERPARAM> &value)
{
    setting.screenerParams = value;
    saveScreenerParams();
    setEnabledScreenerParams();
}

void Database::setEnabledScreenerParams()
{
    enabledScreenerParams.clear();

    for(const sSCREENERPARAM &param : setting.screenerParams)
    {
        if(param.enabled)
        {
            enabledScreenerParams << param.name;
        }
    }
}

QStringList Database::getEnabledScreenerParams() const
{
    return enabledScreenerParams;
}


QVector<sFILTER> Database::getFilterList() const
{
    return filterList;
}

void Database::setFilterList(const QVector<sFILTER> &value)
{
    filterList = value;
    saveFilterList();
}

void Database::loadFilterList()
{
    QFile qFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + FILTERLISTFILE);

    if(qFile.exists())
    {
        if (qFile.open(QIODevice::ReadOnly))
        {
            QDataStream in(&qFile);
            in >> filterList;
            qFile.close();
        }
    }
}

void Database::saveFilterList()
{
    QFile qFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + FILTERLISTFILE);
    if (qFile.open(QIODevice::WriteOnly))
    {
        QDataStream out(&qFile);
        out << filterList;
        qFile.close();
    }
}

QDataStream &operator<<(QDataStream &out, const sFILTER &param)
{
    out << param.param;
    out << static_cast<int>(param.filter);
    out << param.color;
    out << param.val1;
    out << param.val2;

    return out;
}

QDataStream &operator>>(QDataStream &in, sFILTER &param)
{
    in >> param.param;

    int buffer;
    in >> buffer;
    param.filter = static_cast<eFILTER>(buffer);

    in >> param.color;
    in >> param.val1;
    in >> param.val2;

    return in;
}
