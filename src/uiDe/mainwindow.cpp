#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "encode.h"
#include "decode.h"
#include <QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_btnEncode_clicked()
{
    Encode *encodeWidget=new Encode(this);
    encodeWidget->show();
}

void MainWindow::on_btnDecode_clicked()
{
    Decode *decodeWidget=new Decode(this);
    decodeWidget->show();
}
