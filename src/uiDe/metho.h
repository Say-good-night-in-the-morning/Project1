#ifndef METHO_H
#define METHO_H
#include "adjust.h"
#include <bitset>
#include <cmath>
using namespace std;
using namespace cv;
namespace cqw {
vector<Vec3d>searchHorizontalLines(Mat& src);
vector<Point2f>separateVerticalLines(const vector<Vec3d> &list_lines, Mat& src);
void fixationPoints(vector<Point2f> &local_point, Mat& src);
bool computeTransformationPoints(vector<Point2f> &localization_points, vector<Point2f> &transformation_points, Mat& src);
Point2f intersectionLines(Point2f a1, Point2f a2, Point2f b1, Point2f b2);
bool testBypassRoute(vector<Point2f> hull, int start, int finish);
vector<Point2f> getQuadrilateral(vector<Point2f> angle_list, Mat& src);
inline double getCosVectors(Point2f a, Point2f b, Point2f c);
bool updatePerspective(vector<Point2f>& original_points, Mat& src, Mat& dist, Mat& without);
bool samplingForVersion(Mat& no_border_intermediate, Mat& straight);
inline Point computeOffset(const vector<Point>& v);
bool decodeCqw(Mat mat,std::vector<std::vector<unsigned char>>& pageone,bool& idle);
}

#endif // METHO_H
