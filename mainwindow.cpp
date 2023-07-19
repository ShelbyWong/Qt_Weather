#include "mainwindow.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkReply>
#include <QPainter>
#include <QTimer>
#include "WeatherTool.h"
#include "ui_mainwindow.h"
#include <QPen>
#include <QBrush>


#define SCALE 3
#define POINT_RADIUS 3
#define TEXT_OFFSET_X 10
#define TEXT_OFFSET_Y 10

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    //设置窗口属性
    setWindowFlag(Qt::FramelessWindowHint);  // 无边框
    //    setFixedSize(height(), width());         // 固定窗口大小
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    this->setBaseSize(800,500);
    setSizeIncrement(0.2,0.2);

    // 右键菜单：退出程序
    mExitMenu = new QMenu(this);
    mExitAct = new QAction();
    mExitAct->setText(tr("退出"));
    mExitAct->setIcon(QIcon(":/res/close.png"));
    mExitMenu->addAction(mExitAct);

    connect(mExitAct, &QAction::triggered, this, [=]() { qApp->exit(0); });
    //
    m_WeekList<<ui->lblWeek0<<ui->lblWeek1<<ui->lblWeek2<<ui->lblWeek3<<ui->lblWeek4<<ui->lblWeek5;
    m_DateList<<ui->lblDate0<<ui->lblDate1<<ui->lblDate2<<ui->lblDate3<<ui->lblDate4<<ui->lblDate5;
    m_TypeList<<ui->lblType0<<ui->lblType1<<ui->lblType2<<ui->lblType3<<ui->lblType4<<ui->lblType5;
    m_TypeIconList<<ui->lblTypeIcon0<<ui->lblTypeIcon1<<ui->lblTypeIcon2<<ui->lblTypeIcon3<<ui->lblTypeIcon4<<ui->lblTypeIcon5;
    m_AqiList<<ui->lblQuality0<<ui->lblQuality1<<ui->lblQuality2<<ui->lblQuality3<<ui->lblQuality4<<ui->lblQuality5;
    m_diList<<ui->lblFx0<<ui->lblFx1<<ui->lblFx2<<ui->lblFx3<<ui->lblFx4<<ui->lblFx5;
    m_spList<<ui->lblFl0<<ui->lblFl1<<ui->lblFl2<<ui->lblFl3<<ui->lblFl4<<ui->lblFl5;

    //http request
    m_NetAccessManager = new QNetworkAccessManager(this);

    connect(m_NetAccessManager,&QNetworkAccessManager::finished,this,&MainWindow::onReply);
    getWeatherInfo("广州");//GuangZhou

    //add event filter to label
    ui->lblHighCurve->installEventFilter(this);
    ui->lblLowCurve->installEventFilter(this);


}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::onReply(QNetworkReply *reply)
{
    qDebug()<<"send request successfully!";

    int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<"operation: "<<reply->operation();
    qDebug()<<"status code: "<<status_code;
    qDebug()<<"url: "<<reply->url();
    qDebug()<<"response header: "<<reply->rawHeaderList();
    if(reply->error()!=QNetworkReply::NoError||status_code!=200)
    {
        qDebug()<<reply->errorString().toUtf8().data();
        QMessageBox::warning(this,"weather","failed to request!",QMessageBox::Ok);

    }else
    {
        QByteArray arr=reply->readAll();
        qDebug()<<arr.data();
        parseJson(arr);//after update, parse the response json data, and update UI

    }
    reply->deleteLater();
}

void MainWindow::contextMenuEvent(QContextMenuEvent* event) {
    mExitMenu->exec(QCursor::pos());
    event->accept();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    m_Offset = event->globalPos()-this->pos();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos()-m_Offset);
}

void MainWindow::getWeatherInfo(QString cityName)
{
    QString cityCode = WeatherTool::getCityCode(cityName);
    if(cityCode.isEmpty())
    {
        QMessageBox::warning(this,"天气","请检查是否输入正确",QMessageBox::Ok);
        return;
    }
    QUrl url("http://t.weather.itboy.net/api/weather/city/"+cityCode);
    m_NetAccessManager->get(QNetworkRequest(url));

}

void MainWindow::parseJson(QByteArray &byteArray)
{
    QJsonParseError err;
    QJsonDocument doc=QJsonDocument::fromJson(byteArray,&err);
    if(err.error!=QJsonParseError::NoError)
    {
        return;
    }
    QJsonObject rootObj=doc.object();
    qDebug()<<rootObj.value("message").toString();
    //1.parse date and city
    m_Today.date=rootObj.value("date").toString();
    m_Today.city=rootObj.value("cityInfo").toObject().value("city").toString();
    //2.parse yesterday
    QJsonObject data_obj=rootObj.value("data").toObject();
    QJsonObject yesterday_obj= data_obj.value("yesterday").toObject();
    m_Day[0].week = yesterday_obj.value("week").toString();
    m_Day[0].date = yesterday_obj.value("ymd").toString();
    m_Day[0].type = yesterday_obj.value("type").toString();
    QString s;
    s = yesterday_obj.value("high").toString().split(" ").at(1);
    s=s.left(s.length()-1);//substring
    m_Day[0].high    = s.toInt();

    s = yesterday_obj.value("low").toString().split(" ").at(1);
    s=s.left(s.length()-1);//substring
    m_Day[0].low    = s.toInt();

    //speed and direction
    m_Day[0].direction=yesterday_obj.value("fx").toString();
    m_Day[0].speed=yesterday_obj.value("fl").toString();

    //aqi
    m_Day[0].aqi = yesterday_obj.value("aqi").toDouble();


    //forecast, the next five days
    QJsonArray forecast_arr=data_obj.value("forecast").toArray();
    for (size_t i=1;i<=5;i++) {
        QJsonObject forecast_obj = forecast_arr[i-1].toObject();
        m_Day[i].week = forecast_obj.value("week").toString();
        m_Day[i].date = forecast_obj.value("ymd").toString();
        m_Day[i].type = forecast_obj.value("type").toString();

        //
        QString s;
        s = forecast_obj.value("high").toString().split(" ").at(1);
        s=s.left(s.length()-1);
        m_Day[i].high = s.toInt();
        s= forecast_obj.value("low").toString().split(" ").at(1);
        s=s.left(s.length()-1);
        m_Day[i].low  = s.toInt();
        m_Day[i].aqi = forecast_obj.value("aqi").toDouble();
        m_Day[i].direction=yesterday_obj.value("fx").toString();
        m_Day[i].speed=yesterday_obj.value("fl").toString();


    }
    //today data
    m_Today.fever = data_obj.value("ganmao").toString();
    m_Today.humidity = data_obj.value("shidu").toString();
    m_Today.temperature = data_obj.value("wendu").toString().toInt();
    m_Today.quality = data_obj.value("quality").toString();
    m_Today.pm2_5 = data_obj.value("pm25").toDouble();
    m_Today.type = m_Day[1].type;
    m_Today.speed= m_Day[1].speed;
    m_Today.direction = m_Day[1].direction;
    m_Today.high = m_Day[1].high;
    m_Today.low = m_Day[1].low;

    m_TypeMap.insert("暴雪",":/res/type/BaoXue.png");
    m_TypeMap.insert("暴雨",":/res/type/BaoYu.png");
    m_TypeMap.insert("暴雨到大暴雨",":/res/type/DaBaoYuDaoTeDaBaoYu.png");
    m_TypeMap.insert("大暴雨",":/res/type/DaBaoYu.png");
    m_TypeMap.insert("大暴雨到特大暴雨",":/res/type/DaBaoYuDaoTeDaBaoYu.png");
    m_TypeMap.insert("大到暴雪",":/res/type/DaDaoBaoXue.png");
    m_TypeMap.insert("大到暴雨",":/res/type/DaDaoBaoYu.png");
    m_TypeMap.insert("大雪",":/res/type/DaXue.png");
    m_TypeMap.insert("大雨",":/res/type/DaYu.png");
    m_TypeMap.insert("冬雨",":/res/type/DongYu.png");
    m_TypeMap.insert("多云",":/res/type/DuoYun.png");
    m_TypeMap.insert("浮尘",":/res/type/FuChen.png");
    m_TypeMap.insert("雷阵雨",":/res/type/LeiZhenYu.png");
    m_TypeMap.insert("雷阵雨伴有冰雹",":/res/type/LeiZhenYuBanYouBingBao.png");
    m_TypeMap.insert("霾",":/res/type/Mai.png");
    m_TypeMap.insert("强沙尘暴",":/res/type/QiangShaChenBao.png");
    m_TypeMap.insert("晴",":/res/type/Qing.png");
    m_TypeMap.insert("沙尘暴",":/res/type/ShaChenBao.png");
    m_TypeMap.insert("特大暴雨",":/res/type/TeDaBaoYu.png");
    m_TypeMap.insert("undefined",":/res/type/undefined.png");
    m_TypeMap.insert("雾",":/res/type/Wu.png");
    m_TypeMap.insert("小到中雪",":/res/type/XiaoDaoZhongXue.png");
    m_TypeMap.insert("小到中雨",":/res/type/XiaoDaoZhongYu.png");
    m_TypeMap.insert("小雪",":/res/type/XiaoXue.png");
    m_TypeMap.insert("小雨",":/res/type/XiaoYu.png");
    m_TypeMap.insert("雪",":/res/type/Xue.png");
    m_TypeMap.insert("扬沙",":/res/type/YangSha.png");
    m_TypeMap.insert("阴",":/res/type/Yin.png");
    m_TypeMap.insert("雨",":/res/type/Yu.png");
    m_TypeMap.insert("雨夹雪",":/res/type/YuJiaXue.png");
    m_TypeMap.insert("阵雪",":/res/type/ZhenXue.png");
    m_TypeMap.insert("阵雨",":/res/type/ZhenYu.png");
    m_TypeMap.insert("中到大雪",":/res/type/ZhongDaoDaXue.png");
    m_TypeMap.insert("中到大雨",":/res/type/ZhongDaoDaYu.png");
    m_TypeMap.insert("中雪",":/res/type/ZhongXue.png");
    m_TypeMap.insert("中雨",":/res/type/ZhongYu.png");



    //update UI
    updateUI();
    //manually draw lines
    ui->lblHighCurve->update();
    ui->lblLowCurve->update();

}

void MainWindow::updateUI()
{
    //update city and date info
    ui->lblDate->setText(QDateTime::fromString(m_Today.date,"yyyyMMdd").toString("yyyy/MM/dd")+" "+m_Day[1].week);
    ui->lblCity->setText(m_Today.city);

    //update today
    ui->lblTypeIcon->setPixmap(m_TypeMap[m_Today.type]);
    ui->lblTemp->setText(QString::number(m_Today.temperature));
    ui->lblType->setText(m_Today.type);
    ui->lblLowHigh->setText(QString::number(m_Today.low)+"-"+QString::number(m_Today.high)+"°");
    ui->lblGanMao->setText("感冒指数"+m_Today.fever);
    ui->lblWindFx->setText(m_Today.direction);
    ui->lblWindFl->setText(m_Today.speed);
    ui->lblPM25->setText(QString::number(m_Today.pm2_5));
    ui->lblShiDu->setText(m_Today.humidity);
    ui->lblQuality->setText(m_Today.quality);

    //update yesterday and next five days

    for (size_t i =0;i<6;i++) {
        //更新日期和时间
        m_WeekList[i]->setText("周"+m_Day[i].week.right(1));


        //problematic
        ui->lblWeek0->setText("昨天");
        ui->lblWeek1->setText("今天");
        ui->lblWeek2->setText("明天");
        QStringList ymdList=m_Day[i].date.split("-");
        m_DateList[i]->setText(ymdList[1]+"/"+ymdList[2]);
        //今天天气类型
        m_TypeList[i]->setText(m_Day[i].type);
        m_TypeIconList[i]->setPixmap(m_TypeMap[m_Day[i].type]);

        //
        if(m_Day[i].aqi>=0&&m_Day[i].aqi<=50)
        {
            m_AqiList[i]->setText("优");
            m_AqiList[i]->setStyleSheet("background-color: rgb(121,184,0);");
        }else if(m_Day[i].aqi>50&&m_Day[i].aqi<=100)
        {
            m_AqiList[i]->setText("良");
            m_AqiList[i]->setStyleSheet("background-color: rgb(255,187,23);");
        }else if(m_Day[i].aqi>100&&m_Day[i].aqi<=150)
        {
            m_AqiList[i]->setText("轻度");
            m_AqiList[i]->setStyleSheet("background-color: rgb(255,87,97);");

        }else if(m_Day[i].aqi>150&&m_Day[i].aqi<=200)
        {
            m_AqiList[i]->setText("中度");
            m_AqiList[i]->setStyleSheet("background-color: rgb(235,17,27);");

        }else if(m_Day[i].aqi>200&&m_Day[i].aqi<=250)
        {
            m_AqiList[i]->setText("重度");
            m_AqiList[i]->setStyleSheet("background-color: rgb(170,0,0);");

        }else
        {
            m_AqiList[i]->setText("严度");
            m_AqiList[i]->setStyleSheet("background-color: rgb(110,0,0);");

        }
        //
        m_spList[i]->setText(m_Day[i].speed);
        m_diList[i]->setText(m_Day[i].direction);


    }

}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched==ui->lblHighCurve&&event->type()==QEvent::Paint)
    {
        paintHighCurve();
    }
    if(watched==ui->lblLowCurve&&event->type()==QEvent::Paint)
    {
        paintLowCurve();
    }
    return QWidget::eventFilter(watched,event);
}

void MainWindow::paintHighCurve()
{
    QPainter painter(ui->lblHighCurve);

    painter.setRenderHint(QPainter::Antialiasing,true);

    int pointX[6] ={0};
    int pointY[6] = {0};
    for (short int i =0;i<6;i++) {
        pointX[i] = m_WeekList[i]->pos().x()+m_WeekList[i]->width()/2;
    }
    int tempSum=0;
    int tempAvg=0;
    for (short int i=0;i<6;i++) {
        tempSum +=m_Day[i].high;
    }
    tempAvg=tempSum/6;

    //
    int yCenter = ui->lblHighCurve->height()/2;
    for (short int i=0;i<6;i++) {
        pointY[i] = yCenter - ((m_Day[i].high- tempAvg)*SCALE);

    }
    //painting
    QPen pen = painter.pen();
    pen.setWidth(1);
    pen.setColor(QColor(255,0,0));

    painter.setPen(pen);
    painter.setBrush(QColor(255,0,0));

    for (short int i=0;i<6;i++) {
        painter.drawEllipse(QPoint(pointX[i],pointY[i]),POINT_RADIUS,POINT_RADIUS);
        //show text
        painter.drawText(pointX[i]-TEXT_OFFSET_X,pointY[i]-TEXT_OFFSET_Y,QString::number(m_Day[i].high)+"°");

    }
    //draw line
    for(short int i=0;i<5;i++)
    {
        if(i==0){
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
        }
        else{
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
        }
        painter.drawLine(pointX[i],pointY[i],pointX[i+1],pointY[i+1]);

    }

}

void MainWindow::paintLowCurve()
{
    QPainter painter(ui->lblLowCurve);

    painter.setRenderHint(QPainter::Antialiasing,true);

    int pointX[6] ={0};
    int pointY[6] = {0};
    for (short int i =0;i<6;i++) {
        pointX[i] = m_WeekList[i]->pos().x()+m_WeekList[i]->width()/2;
    }
    int tempSum=0;
    int tempAvg=0;
    for (short int i=0;i<6;i++) {
        tempSum +=m_Day[i].low;
    }
    tempAvg=tempSum/6;

    //
    int yCenter = ui->lblLowCurve->height()/2;
    for (short int i=0;i<6;i++) {
        pointY[i] = yCenter - ((m_Day[i].low- tempAvg)*SCALE);

    }
    //painting
    QPen pen = painter.pen();
    pen.setWidth(1);
    pen.setColor(QColor(20,40,50));

    painter.setPen(pen);
    painter.setBrush(QColor(20,40,50));

    for (short int i=0;i<6;i++) {
        painter.drawEllipse(QPoint(pointX[i],pointY[i]),POINT_RADIUS,POINT_RADIUS);
        //show text
        painter.drawText(pointX[i]-TEXT_OFFSET_X,pointY[i]-TEXT_OFFSET_Y,QString::number(m_Day[i].low)+"°");

    }
    //draw line
    for(short int i=0;i<5;i++)
    {
        if(i==0){
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
        }
        else{
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
        }
        painter.drawLine(pointX[i],pointY[i],pointX[i+1],pointY[i+1]);

    }
}


void MainWindow::on_btnSearch_clicked()
{
    QString cityName = ui->leCity->text();
    getWeatherInfo(cityName);

}
