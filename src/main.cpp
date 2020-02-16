#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

constexpr int FRAME_HEIGHT = 950;//定义帧高度
constexpr int FRAME_WIDTH = 1600;//定义帧宽度
constexpr int RECTSIZE = 150;//定义编码点尺寸
constexpr int LOCATERECTSIZE = 200;//定义定位点尺寸

Size RSFRAME(1400, 750);

vector<Point>codePointLocation =
{
	Point(400, 250),Point(400, 500),Point(600, 250) ,Point(600, 500),
	Point(800, 250),Point(800, 500),Point(1000, 250),Point(1000, 500)
};//定义编码点坐标

Point locatPointTopLeft = Point(100, 100);//定义定位点坐标-左上
Size NEEDLESS(100, 100);//修剪尺寸

Point locatPointBottomRight = Point(1300, 650);//定义定位点坐标-右下

Scalar BLACK = Scalar(0, 0, 0);//黑色
Scalar WHITE = Scalar(255, 255, 255);//白色

void codeInit(char);//编码帧生成

int main()
{
	//读入测试文件
	Mat temp = imread("rr1.jpg");
	Mat src = imread("rr0.jpg");

	//-----------------------输入文件预处理-------------------------//
	//彩色转灰度
	cvtColor(src, src, COLOR_RGB2GRAY);

	//平滑图像-高斯模糊
	Size GUASSKERNELSRC(23, 23);
	GaussianBlur(src, src, GUASSKERNELSRC, 0, 0);

	//阈值-二值化
	int SRC_THRESH_LOW = 200;
	threshold(src, src, SRC_THRESH_LOW, 255, THRESH_BINARY);

	//形态学开
	Mat kernelsrc = getStructuringElement(MORPH_RECT, Size(23, 23), Point(-1, -1));
	morphologyEx(src, src, MORPH_OPEN, kernelsrc);

	//-----------------------降噪模板-------------------------//
	//彩色转灰度
	cvtColor(temp, temp, COLOR_RGB2GRAY);

	//平滑图像-高斯模糊
	Size GAUSSKERNELTEMP(13, 13);
	GaussianBlur(temp, temp, GAUSSKERNELTEMP, 0, 0);

	//阈值-二值化
	int TEMP_THRES_LOW = 100;
	threshold(temp, temp, TEMP_THRES_LOW, 255, THRESH_BINARY);

	//-----------------------降噪-------------------------//
	//差值降噪
	src -= temp;

	//形态学开
	Mat kernelbin = getStructuringElement(MORPH_RECT, Size(100, 100), Point(-1, -1));
	morphologyEx(src, src, MORPH_OPEN, kernelbin);

	//-----------------------ROI提取---------------------------//

	//Canny边缘查找
	Mat src_gray = src;
	Mat canny_output;
	Canny(src_gray, canny_output, 200, 255);

	//轮廓查找
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(canny_output, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	//从轮廓生成最小矩形
	vector<Rect>poly_rect(contours.size());
	for (int i = 0; i < contours.size(); i++) {
		poly_rect[i] = boundingRect(contours[i]);
	}

	//矩形轮廓x坐标整理
	for (int i = poly_rect.size(); i > 0; i--) {
		for (int j = 0; j < i-1; j++) {
			if (poly_rect[j].tl().x > poly_rect[j + 1].tl().x)
				swap(poly_rect[j], poly_rect[j + 1]);
		}
	}
	//矩形轮廓y坐标整理
	for (int i = 0; i+1 < poly_rect.size(); i += 2) {
		if (poly_rect[i].tl().y > poly_rect[i + 1].tl().y)
			swap(poly_rect[i], poly_rect[i + 1]);
	}

//坐标整理检查
//	for (int i = 0; i < poly_rect.size(); i++) {
//		cout << poly_rect[i].x << "  " << poly_rect[i].y << endl;
//	}

	//ROI坐标生成 tl=topleft,br=bottomright
	int tl_x = poly_rect[0].tl().x;
	int br_x = poly_rect[poly_rect.size() - 1].br().x;
	int tl_y= poly_rect[0].tl().y;
	int br_y = poly_rect[poly_rect.size() - 1].br().y;

	//将提取出的矩形轮廓重新绘制
	Mat rscrc = Mat::zeros(canny_output.size(), CV_8UC1);
	for (int i = 0; i < poly_rect.size(); i++) {
		rectangle(rscrc, poly_rect[i], WHITE, FILLED);
	}

	//ROI区域生成
	Mat srcRoi = rscrc(Rect(tl_x, tl_y, br_x - tl_x, br_y - br_y));
	resize(srcRoi, srcRoi, RSFRAME);

	//----------------------解码过程--------------------------//
	//读取解码遮罩
	vector<Mat> mask(8);
	for (int i = 0; i < 8; i++) {
		string name = "img" + to_string(i) + ".jpg";
		mask[i] = imread(name);
		cvtColor(mask[i], mask[i], COLOR_BGR2GRAY);
	}

	//解码
	char c;
	for (int i = 7; i >=0 ; i--) {
		//缓存帧
		Mat p = Mat::zeros(RSFRAME, CV_8UC1);

		//添加遮罩
		p = 255 - (mask[i] + srcRoi);

		//去噪
		Mat kernelroi = getStructuringElement(MORPH_RECT, Size(10, 10), Point(-1, -1));
		morphologyEx(p, p, MORPH_OPEN, kernelroi);

		//译码
		if (countNonZero(p) ==0)
			c |= 0x01;
		if(i!=0)
			c <<= 1;
	}

	return 0;
}

void codeInit(char c) {
	//定义帧的高度，宽度，图片类型，底色
	Mat frame = Mat(FRAME_HEIGHT, FRAME_WIDTH, CV_8UC1, BLACK);

	//生成char c的编码帧
	for (int i = 0; i < 8; i++) {
		if (c & 0x01 ) {
			rectangle(frame, codePointLocation[i], Point(codePointLocation[i].x + RECTSIZE, codePointLocation[i].y + RECTSIZE), WHITE, FILLED);
		}
		c >>= 1;
	}
	
	//辅助生成定位符
	rectangle(frame, locatPointTopLeft, Point(locatPointTopLeft.x + LOCATERECTSIZE, locatPointTopLeft.y + LOCATERECTSIZE), WHITE, FILLED);
	rectangle(frame, locatPointBottomRight, Point(locatPointBottomRight.x + LOCATERECTSIZE, locatPointBottomRight.y + LOCATERECTSIZE), WHITE, FILLED);

//	根据帧尺寸和定位点位置输出遮罩
//	vector<Mat> mask(8);
//	for (int i = 0; i < 8; i++) {
//		mask[i] = Mat(frame.size()-NEEDLESS-NEEDLESS,CV_8UC1,WHITE);
//	rectangle(mask[i], codePointLocation[i] - locatPointTopLeft, Point(locatPointTopLeft.x + RECTSIZE - locatPointTopLeft.x, codePointLocation[i].y + RECTSIZE - locatPointTopLeft.y), BLACK, FILLED);
//		string str = "img" + to_string(i) + ".jpg";
//		imwrite(str, mask[i]);
//	}
}

