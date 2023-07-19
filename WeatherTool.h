#ifndef WEATHERTOOL_H
#define WEATHERTOOL_H
#include <QString>
#include <QMap>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QWidget>
class WeatherTool{
public:
    static QString getCityCode(QString cityName);
private:
    static QMap<QString,QString> m_CityMap;
    static void initCityMap(){
        QString filePath = ":/res/citycode.json";
        QFile file(filePath);
        file.open(QIODevice::ReadOnly|QIODevice::Text);
        QByteArray json=file.readAll();
        file.close();
        //
        QJsonParseError err;
        QJsonDocument doc=QJsonDocument::fromJson(json,&err);
        if(err.error!=QJsonParseError::NoError)
        {
            return;
        }
        if(!doc.isArray())
        {
             return;
        }
        //parsing
        QJsonArray cities=doc.array();
        for(int i=0;i<cities.size();i++)
        {
            QString city_name=cities[i].toObject().value("city_name").toString();
            QString city_code = cities[i].toObject().value("city_code").toString();
            if(city_code.size()>0)
            {
                m_CityMap.insert(city_name,city_code);
            }
        }

    }

};


QMap<QString,QString> WeatherTool::m_CityMap = {};
#endif // WEATHERTOOL_H

QString WeatherTool::getCityCode(QString cityName)
{
    if(m_CityMap.isEmpty())
    {
        initCityMap();


    }
    QMap<QString,QString>::iterator it = m_CityMap.find(cityName);
    if(it==m_CityMap.end())
    {
       it = m_CityMap.find(cityName+"市");
    }
    if(it!=m_CityMap.end())
    {
        return it.value();
    }
    return "";
}
