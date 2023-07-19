#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "weatherdata.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
private:
    void onReply(QNetworkReply* reply);

protected:
    void contextMenuEvent(QContextMenuEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent*event);
    void getWeatherInfo(QString cityCode);
    void parseJson(QByteArray&byteArray);
    void updateUI();
    bool eventFilter(QObject*watched,QEvent*event);

    //draw temprature curve
    void paintHighCurve();
    void paintLowCurve();

private slots:
    void on_btnSearch_clicked();

private:
    Ui::MainWindow* ui;

    QMenu* mExitMenu;   // 右键退出的菜单
    QAction* mExitAct;  // 退出的行为
    QPoint m_Offset;// the offset from mainwindows to the widget
    QNetworkAccessManager* m_NetAccessManager;
    Today m_Today;
    Day m_Day[6];

    QList<QLabel*> m_WeekList;
    QList<QLabel*> m_DateList;

    QList<QLabel*> m_TypeList;
    QList<QLabel*> m_TypeIconList;
    //weather index
    QList<QLabel*> m_AqiList;
    QList<QLabel*> m_diList;
    QList<QLabel*> m_spList;
    QMap<QString,QString> m_TypeMap;

};
#endif  // MAINWINDOW_H
