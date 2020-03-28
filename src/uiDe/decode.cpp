#include "decode.h"
#include "ui_decode.h"
#include "adjust.h"




Decode::Decode(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Decode)
{
    ui->setupUi(this);
    ui->btnDecodeDe->setDisabled(true);

}

Decode::~Decode()
{
    delete ui;
}
void Decode::on_btnDecodeDe_clicked()
{
    adjust* ad=new adjust(this->Path,this->output,this);
    ad->show();

}

void Decode::Openfile(){
    QString location = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString path = QFileDialog::getOpenFileName(this,"打开文件",location,"视频文件(*.mp4)");
    if(path.isEmpty())
        return;
    this->Path=path;
    this->ui->lineEditDecodePath->setText(path);
    this->ui->btnDecodeDe->setEnabled(true);
}

void Decode::on_btnDecodeSelect_clicked()
{
    Openfile();
}

void Decode::on_btnDecodeCancel_clicked()
{
    this->close();
}


void Decode::on_btnDecodeSelect2_pressed()
{
    QString location = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString path = QFileDialog::getExistingDirectory(this,"选择路径",location);
    if(path.isEmpty())
        return;
    this->output=path;
    this->ui->lineEditOutput->setText(path);
}
