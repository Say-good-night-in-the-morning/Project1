#ifndef ENCODE_H
#define ENCODE_H

#include <QWidget>
#include <QFileDialog>
#include <vector>
#include <QString>
#include <QByteArray>
#include <cstdio>
#include <QDebug>
#include <string>
#include <QtConcurrent>
#include <opencv2/opencv.hpp>
#include <QStandardPaths>

namespace Ui {
class Encode;
}

class Encode : public QWidget
{
    Q_OBJECT

public:
    explicit Encode(QWidget *parent = nullptr);
    void Openfile(void);
    void Outputfile(void);

    void binaryInput(void);

    void encodeStart(void);
    void videoInit(void);
    void videoOutput(void);
    void QRcodeInit();
    ~Encode();

private slots:
    void on_btnEncodeCancel_clicked();
    void on_btnEncodeInput_clicked();
    void on_btnEncodeOutput_clicked();
    void on_btnEncodeEn_clicked();

    void prchange(int);
    void encodeFinish();
    void on_lineEditTime_editingFinished();

signals:
    void prvalue(int);
    void encodeFin();

private:

    Ui::Encode *ui;

    size_t NUMBER_OF_CHAR;

    QString openPath;
    QString outputPath;

    std::vector<char> fileReadin;
    std::vector<cv::Mat> videoFrame;

    cv::VideoWriter vid_out;
    int time;

};

#endif // ENCODE_H
