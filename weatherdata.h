#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#endif // WEATHERDATA_H
#include <QString>
#include <QDate>
class Today{
public:
    Today(){
        date=QDate::currentDate().toString("yyyy-MM-dd");
        city="广州";
        fever = "感冒指数";
        temperature = 0;
        pm2_5 = 0;
        humidity="0%";
        quality="none";
        type = "多云";
        speed = "2级";
        direction = "南风";
        high = 30;
        low = 18;

    }

    QString date;
    QString city;
    QString fever;
    QString humidity;
    QString quality;
    QString type;
    QString speed;
    QString direction;
    int temperature;
    double pm2_5;
    int high;
    int low;


};

class Day{
public:
    Day(){
        date=QDate::currentDate().toString("yyyy-MM-dd");
        week="周五";
        type="多云";
        high=0;
        low=0;
        speed="2级";
        direction="南风";
        aqi=0;
    }

    QString date;
    QString week;
    QString type;
    int high;
    int low;
    QString speed;
    QString direction;
    double aqi;
};
