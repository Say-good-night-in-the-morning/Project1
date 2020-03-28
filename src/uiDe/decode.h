#ifndef DECODE_H
#define DECODE_H

#include <QWidget>
#include <QFileDialog>
#include <opencv2/opencv.hpp>
#include <QStandardPaths>
#include <QDebug>


namespace Ui {
class Decode;
}

class Decode : public QWidget
{
    Q_OBJECT

public:
    explicit Decode(QWidget *parent = nullptr);
    void Openfile();
    ~Decode();

private slots:
    void on_btnDecodeDe_clicked();
    void on_btnDecodeSelect_clicked();
    void on_btnDecodeCancel_clicked();

    void on_btnDecodeSelect2_pressed();

private:
    Ui::Decode *ui;
    QString Path;
    QString output;
};

#endif // DECODE_H
