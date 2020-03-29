#include "mainwindow.h"
#include <QApplication>
#include <opencv2/opencv.hpp>
#include "log.h"
using namespace cv;

extern Scalar BLACK;
extern Scalar WHITE;

int main(int argc, char *argv[])
{
    qInstallMessageHandler(customMessageHandler);
    QDateTime current_date_time =QDateTime::currentDateTime();
    qDebug()<<current_date_time<<" Debug begin";
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
