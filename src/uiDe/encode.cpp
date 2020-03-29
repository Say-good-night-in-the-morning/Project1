#include "encode.h"
#include "ui_encode.h"
#include "cvvariables.h"
#include <bitset>

int cqw::QRCODESIZE=97;
cv::Size cqw::FRAME(1920, 1080);//视频尺寸

cv::Scalar BLACK=cv::Scalar(0,0,0);
cv::Scalar WHITE=cv::Scalar(255,255,255);

Encode::Encode(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Encode)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    ui->probarEncode->hide();
    ui->probarEncode->setValue(0);
    ui->labelEncodeWarning->setText("");
    ui->labelEncodeInputPath->setBuddy(ui->lineEditInput);
    ui->labelOutputPath->setBuddy(ui->lineEditOutput);
    connect(this,SIGNAL(prvalue(int)),this,SLOT(prchange(int)),Qt::BlockingQueuedConnection);
    connect(this,SIGNAL(encodeFin()),this,SLOT(encodeFinish()));
}

Encode::~Encode()
{
    delete ui;
}

void Encode::on_btnEncodeCancel_clicked()
{
    this->close();
}

void Encode::Openfile(void){
    QString location = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString path = QFileDialog::getOpenFileName(this,"打开文件",location,"二进制文件(*.bin)");
    if(path.isEmpty())
        return;
    this->openPath=path;
    this->ui->lineEditInput->setText(path);
    binaryInput();
}

void Encode::Outputfile(void){
    QString location = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString path = QFileDialog::getExistingDirectory(this,"选择路径",location);
    if(path.isEmpty())
        return;
    this->outputPath=path;
    this->ui->lineEditOutput->setText(path);
}


void Encode::on_btnEncodeInput_clicked()
{
    Openfile();
}

void Encode::on_btnEncodeOutput_clicked()
{
    Outputfile();
}

void Encode::on_btnEncodeEn_clicked()
{
    if(openPath.isEmpty()||outputPath.isEmpty()){
        ui->labelEncodeWarning->setText("路径不能为空！");
        return;
    }
    else {
        ui->btnEncodeEn->setDisabled(true);
        ui->btnEncodeInput->setDisabled(true);
        ui->btnEncodeOutput->setDisabled(true);
        ui->labelEncodeWarning->setText("转码中...");
    }
    ui->probarEncode->show();
    encodeStart();
}


void Encode::encodeStart(void){
    QtConcurrent::run([this](){
        this->videoInit();
        emit this->encodeFin();//qt信号槽，不用管
        cv::imwrite("D:/22.jpg",videoFrame[0]);
    });

    return;
}

void Encode::videoInit(){
    QString fileName=outputPath+"/vid_output.mp4";//输出路径和文件名，现在已经默认输出到项目文件夹
    //视频格式定义
    int fourcc = cv::VideoWriter::fourcc('a','v','c','1');//文件编码格式h.264
    //24帧，mp4格式
    vid_out=cv::VideoWriter(fileName.toStdString(),fourcc, 8, cqw::FRAME, 0);//视频输出流
    videoFrame.clear();
    cv::Mat a=cv::Mat(cqw::FRAME.height,cqw::FRAME.width,CV_8UC1, WHITE);
    videoFrame.push_back(a);
    QRcodeInit();
    vid_out<<videoFrame[0];
    vid_out.release();
}

void Encode::binaryInput(void)
{
    char* pathC;
    if(fileReadin.size()!=0)
        fileReadin.clear();
    QByteArray pathQ=openPath.toLatin1();
    pathC=pathQ.data();
    FILE* f = fopen(pathC, "rb");//打开文件
    char ch;
    fseek(f, 0, SEEK_END); //定位到文件末
    int nFileLen = ftell(f);//文件长度
    rewind(f);
    for(int i=0;i<nFileLen;i++)
    {
        ch=fgetc(f);
        fileReadin.push_back(ch);//将字符存入c末尾
    }
    fclose(f);//关闭文件
    NUMBER_OF_CHAR=fileReadin.size();//读取字符数量确定视频长度，即关键帧帧数
}

//定位图像
void LocationPoint(cv::Mat preMat)
{
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            if (i == 0 || j == 0 || i == 6 || j == 6) {
                preMat.ptr<uint8_t>(i)[j] = 0;
                preMat.ptr<uint8_t>(cqw::QRCODESIZE - i-1)[j] = 0;
                preMat.ptr<uint8_t>(i)[cqw::QRCODESIZE - j-1] = 0;
            }
        }
    }
    for (int i = 2; i < 5; i++) {
        for (int j = 2; j < 5; j++) {
            preMat.ptr<uint8_t>(i)[j] = 0;
            preMat.ptr<uint8_t>(cqw::QRCODESIZE - i-1)[j] = 0;
            preMat.ptr<uint8_t>(i)[cqw::QRCODESIZE - j-1] = 0;
        }
    }
    for (int j = 7; j < cqw::QRCODESIZE; j++) {
        if (j % 2 == 0) {
            preMat.ptr<uint8_t>(6)[j] = 0;
            preMat.ptr<uint8_t>(j)[6] = 0;
        }
    }
}
void Encode::QRcodeInit(){
    int iter=0;
    cv::Mat preMat = cv::Mat(cqw::QRCODESIZE, cqw::QRCODESIZE, CV_8UC1, WHITE);
    LocationPoint(preMat);
    int idle = 0;
    qDebug()<<fileReadin.size();
    for (int page = 0; page < cvCeil(fileReadin.size()*1.0 /(880)); page++) {
        char c = 0;
        cv::Mat Page = preMat.clone();
        int lastiter=iter;
        for (int col = 9; col < cqw::QRCODESIZE; col++) {
            std::bitset<80> data;
            std::bitset<88> code;
            if (idle == 1) {
                iter = lastiter;
            }
            for (int count = 0; count < 10; count++, iter++) {
                if (iter >= fileReadin.size()) {
                    c = 0;
                    code[87] = 1;
                    idle = 1;
                }
                else {
                    c = fileReadin[iter];
                }

                for (int i = 0; i < 8; i++) {
                    data[count * 8 + i] = c & 0x01;
                    c >>= 1;
                }
            }
            int i, j;
            for (i = 0,j = 3; i < 80; i++,j++) {
                if (j == 0x01 || j == 0x02 || j == 0x04 || j == 0x08 || j == 0x10 || j == 0x20 || j == 0x40)
                    j++;
                code[j - 1] = data[i];
                if (j & 0x01 && data[i])
                    code[0x01-1].flip();
                if (j & 0x02 && data[i])
                    code[0x02-1].flip();
                if (j & 0x04 && data[i])
                    code[0x04-1].flip();
                if (j & 0x08 && data[i])
                    code[0x08-1].flip();
                if (j & 0x10 && data[i])
                    code[0x10-1].flip();
                if (j & 0x20 && data[i])
                    code[0x20-1].flip();
                if (j & 0x40 && data[i])
                    code[0x40 - 1].flip();
            }
            for (int row = 9; row <cqw::QRCODESIZE; row++) {
                if (code[row-9] == 0)
                    Page.ptr<uint8_t>(row)[col] = 0;
            }
        }
        int sc=page;
        for(int heal=0;heal<24;heal++){
            int temp=sc&0x01;
            if(temp==0){
                for(int ccc=1;ccc<5;ccc++)
                    Page.ptr<uint8_t>(heal+20)[ccc] = 0;
            }
            sc>>=1;

        }
        emit prvalue(page);
        cv::Mat t;
        cv::resize(Page, t, Page.size()*7,0,0,cv::INTER_AREA);
        cv::Mat back = cv::Mat(1080, 1920, CV_8UC1, WHITE);
        cv::Mat ROI = back(cv::Rect(620, 200, t.cols, t.rows));
        t.copyTo(ROI);
        vid_out<<back;
    }
}
void Encode::prchange(int j){
    ui->probarEncode->setValue(j*100/(NUMBER_OF_CHAR/880));
}

void Encode::encodeFinish(){
    ui->labelEncodeWarning->setText("转码成功！！！");
    ui->probarEncode->setValue(100);
    ui->btnEncodeCancel->setText("返回");
}
