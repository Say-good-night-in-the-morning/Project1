#ifndef ADJUST_H
#define ADJUST_H
#include <opencv2/opencv.hpp>
#include <QWidget>
#include <QString>
#include <QDebug>
#include <QImage>
#include <QValidator>
#include <QPixmapCache>
#include <QtConcurrent>
namespace Ui {
class adjust;
}

class adjust : public QWidget
{
    Q_OBJECT

public:
    explicit adjust(QWidget *parent = nullptr);
    explicit adjust(QString path,QString path2, QWidget *parent=nullptr);
    void Decode(std::vector<cv::Mat> page);
    void Openfile();
    void VideoPreProcess();
    void Write();
    QImage CV2QT(cv::Mat);
    bool idle;
    ~adjust();

private slots:
    void prchange(int);
    void decodeFinish();
    void on_btnDecode_pressed();

    void on_pushButton_released();

signals:
    void prvalue(int);
    void decodeFin();
private:
    Ui::adjust *ui;
    QString PATH;
    QString output;
    cv::VideoCapture vid_in;
    std::vector<std::vector<cv::Mat>>similar;
    std::vector<std::vector<unsigned char>> text;
    int counterror;

};


#endif // ADJUST_H
