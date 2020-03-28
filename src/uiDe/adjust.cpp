#include "adjust.h"
#include "ui_adjust.h"
#include "cvvariables.h"
#include "metho.h"
int const cqw::test_perspective_size=251;
adjust::adjust(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::adjust)
{
    ui->setupUi(this);
}
adjust::adjust(QString path,QString path2, QWidget *parent):
    QWidget(parent),
    ui(new Ui::adjust){
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    ui->labelStatus->setText("准备就绪");
    ui->textBrowser->setPlaceholderText("现在空空如也...");
    connect(this,SIGNAL(decodeFin()),this,SLOT(decodeFinish()));
    connect(this,SIGNAL(prvalue(int)),this,SLOT(prchange(int)));
    this->PATH=path;
    this->output=path2;
    idle=false;
    counterror=0;
    this->ui->labelError->setText("");
    vid_in.open(this->PATH.toStdString());
}
adjust::~adjust()
{
    delete ui;
}

void adjust::Decode(std::vector<cv::Mat> page){

    static int ipp=0;
    static bool complete=false;
    static std::vector<std::vector<unsigned char>> pageone(88);
    if(page.size()<1||this->idle==true)
        return;
    for(int p=0;p<page.size();p++){
        vector<Point2f> localization_points(3), transformation_points;

        Mat straight;
        Mat bin,binbar;

        bin=page[p];
        std::vector<cv::Vec3d> list_lines_x = cqw::searchHorizontalLines(bin);
        std::vector<Point2f> list_lines_y = cqw::separateVerticalLines(list_lines_x,bin);
        if(list_lines_y.size()<3)
            continue;
        int xmin=std::numeric_limits<int>::max(), xmax=std::numeric_limits<int>::min(),
            ymin = std::numeric_limits<int>::max(), ymax= std::numeric_limits<int>::min();
        for (int i = 0; i < list_lines_y.size(); i++) {
            if (list_lines_y[i].x < xmin)
                xmin = list_lines_y[i].x;
            if (list_lines_y[i].x > xmax)
                xmax = list_lines_y[i].x;
            if (list_lines_y[i].y < ymin)
                ymin = list_lines_y[i].y;
            if (list_lines_y[i].y > ymax)
                ymax = list_lines_y[i].y;
        }
        localization_points[0].x = xmin;
        localization_points[0].y = ymin;
        localization_points[1].x = xmin;
        localization_points[1].y = ymax;
        localization_points[2].x = xmax;
        localization_points[2].y = ymin;

        cqw::fixationPoints(localization_points,bin);
        cqw::computeTransformationPoints(localization_points, transformation_points, bin);

        vector<Point2f> p1(4);
        p1[0] = Point2f(0, 0);
        p1[1] = Point2f(cqw::QRCODESIZE*30, 0);
        p1[2] = Point2f(cqw::QRCODESIZE * 30, cqw::QRCODESIZE * 30);
        p1[3] = Point2f(0, cqw::QRCODESIZE * 30);
        Mat elementTransf;

        if(transformation_points.size()!=4)
            continue;
        elementTransf = getPerspectiveTransform(transformation_points, p1);
        warpPerspective(bin, binbar, elementTransf, Size(cqw::QRCODESIZE * 30, cqw::QRCODESIZE * 30),INTERSECT_FULL);
        threshold(binbar, binbar, 200, 255, THRESH_OTSU);
        cqw::samplingForVersion(binbar, straight);
        static int lastpage=0;
        int pagecount=0;
        for(int row1=27;row1>=20;row1--){
            pagecount<<=1;
            if(straight.ptr<uint8_t>(row1)[4]==255)
                pagecount|=0x01;
        }
        qDebug()<<"head"<<complete;
        qDebug()<<pagecount<<"pagecount";
        qDebug()<<lastpage<<"lastpage";

        if(lastpage==pagecount&&complete==true)
            return;
        if(pagecount-lastpage>1){
            if(complete==false){
                this->text.insert(text.end(),pageone.begin(),pageone.end());
                ipp++;
                qDebug()<<"ipp>1"<<ipp;
                lastpage++;
            }
            pageone.clear();
            for(int t=0;t<88;t++){
                pageone.push_back(vector<unsigned char>(10));
            }
            for(;lastpage<pagecount-1;lastpage++){
                this->text.insert(text.end(),pageone.begin(),pageone.end());
                ipp++;
                qDebug()<<"ipp>=1"<<ipp;
                complete=true;
            }
        }
//        if(pagecount-lastpage==1&&complete==false){
//            this->text.insert(text.end(),pageone.begin(),pageone.end());
//            ipp++;
//            qDebug()<<"ipp==1"<<ipp;
//            pageone.clear();
//            for(int t=0;t<88;t++){
//                pageone.push_back(vector<unsigned char>(10));
//            }
//            lastpage=pagecount;
//            complete=true;
//        }
        if(cqw::decodeCqw(straight,pageone,this->idle)){
            ipp++;
            qDebug()<<"ippde"<<ipp;
            this->text.insert(text.end(),pageone.begin(),pageone.end());

            pageone.clear();
            for(int t=0;t<88;t++){
                pageone.push_back(vector<unsigned char>(10));
            }
            lastpage=pagecount;
            complete=true;
            return;
        }
        else{
            complete=false;
        }
    }
    qDebug()<<complete;
}
void adjust::prchange(int j){
    ui->progressBar->setValue(j*100/similar.size());
}

void adjust::Write(){
    FILE* fr,*fv;
    char* pathC;
    QString s=this->output+"/out.bin";
    QByteArray pathQ=s.toLatin1();
    pathC=pathQ.data();

    QString val=this->output+"/val.bin";
    QByteArray pq=val.toLatin1();
    char* pc=pq.data();

    fopen_s(&fr,pathC,"wb");
    fopen_s(&fv,pc,"wb");
    int cerror=0;
    for(int i=0;i<this->text.size();i++){
        cerror=0;
        unsigned char Buff;
        for(int j=0;j<10;j++){

            if(text[i][j]==0)
                cerror++;
            if(j>=text[i].size()){
                Buff=0;
                fwrite(&Buff,sizeof(unsigned char),1,fr);
            }
            else{
                Buff=text[i][j];
                fwrite(&Buff,sizeof(unsigned char),1,fr);
            }
        }

        if(cerror==10){
            Buff=0;
        }
        else{
            Buff=0xFF;
        }
        for(int t=0;t<10;t++){
            fwrite(&Buff,sizeof(unsigned char),1,fv);
        }
    }
    fclose(fr);
    fclose(fv);
}
void adjust::decodeFinish(void){
    ui->progressBar->setValue(100);
    ui->labelStatus->setText("解码完成！");
//    string s(text.begin(),text.end());
//    QString qs=QString::fromStdString(s);
//    ui->textBrowser->insertPlainText(qs);
    Write();
    qDebug()<<"receive: "<<text.size()*10;
    int cerror=0;
    for(int i=0;i<text.size();i++){
        cerror=0;
        string si=std::to_string(i);
        QString qsi=QString::fromStdString(si);
        ui->textBrowser->insertPlainText(qsi);
        for(int j=0;j<text[i].size();j++){
            if(text[i][j]==0)
                cerror++;
            ui->textBrowser->insertPlainText(" ");
            char buff[2];
            sprintf(buff,"%02x",text[i][j]);
            QString qs(buff);
            ui->textBrowser->insertPlainText(qs);
        }
        if(cerror==10){
            this->counterror+=10;
        }
        ui->textBrowser->insertPlainText("\n");
    }
    qDebug()<<counterror;
    char error[20];
    sprintf(error,"error:%d",counterror);
    this->ui->labelError->setText(QString(error));
    ui->pushButton->setText("返回");
    ui->pushButton->setEnabled(true);
}

void adjust::on_btnDecode_pressed()
{
    ui->labelStatus->setText("正在解码请稍等....");
    ui->btnDecode->setEnabled(false);
    ui->pushButton->setEnabled(false);
    QtConcurrent::run([this](){
        VideoPreProcess();
        qDebug()<<"here";
        qDebug()<<similar.size();
        for(int i=0;i<similar.size()&&idle==false;i++){
            Decode(similar[i]);
            emit prvalue(i);
        }
        emit decodeFin();
    });
}
void adjust::VideoPreProcess(){
    std::vector<cv::Mat> framet;
    for (int i = 0; i < vid_in.get(cv::CAP_PROP_FRAME_COUNT); i++) {
        cv::Mat temp;
        vid_in.read(temp);
        cv::cvtColor(temp, temp, cv::COLOR_BGR2GRAY);
        cv::threshold(temp, temp, 0, 255, cv::THRESH_OTSU);
        framet.push_back(temp);
    }

    std::vector<double> diff;
    double sumdiff=.0,one=.0;
    for (int i = 0; i < framet.size()-1; i++) {
        cv::Mat difftemp;
        cv::absdiff(framet[i + 1] , framet[i],difftemp);
        one=cv::countNonZero(difftemp)*1.0/(vid_in.get(cv::CAP_PROP_FRAME_HEIGHT)*vid_in.get(cv::CAP_PROP_FRAME_WIDTH));
        diff.push_back(one);
        sumdiff += one;
    }
    double averdiff=sumdiff/diff.size();

    std::vector<double> var;
    double varsum = 0;
    for (int i = 0; i < diff.size(); i++) {
        one=(averdiff - diff[i])*(averdiff - diff[i]);
        varsum += one;
        var.push_back(one);
    }
    std::vector<cv::Mat> mattemp;
    double segema = sqrt(varsum/diff.size());
    int lasti = 0;
    for (int i = 0; i < diff.size(); i++) {
        double z = (diff[i] - averdiff) / segema;
        if (fabs(z) > 1.0) {
            mattemp.clear();
            int good = i - lasti;
            if (good > 4)
                lasti += 1;
            for (int j = lasti; j < i; j++) {
                mattemp.push_back(framet[j]);
            }
            lasti = i + 1;

            similar.push_back(mattemp);
        }
    }
}


void adjust::on_pushButton_released()
{
    this->close();
}
