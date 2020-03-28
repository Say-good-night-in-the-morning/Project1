#include "metho.h"
const int test_perspective_size = 251;
uint8_t version = 20;
uint8_t version_size = 97;
extern int QRCODESIZE=97;

vector<Vec3d>cqw::searchHorizontalLines(Mat& src) {
    vector<Vec3d> result;
    //长宽
    const int height_bin_barcode = src.rows;
    const int width_bin_barcode = src.cols;
    //划过定位点，1：1：3：1：1
    const size_t test_lines_size = 5;
    //暂存数组
    double test_lines[test_lines_size];
    //存储划过的像素点
    vector<size_t> pixels_position;
    //一行一行搜索定位点的位置
    for (int y = 0; y < height_bin_barcode; y++)
    {
        //每行都要清理一次
        pixels_position.clear();
        //用来遍历的行指针
        const uint8_t *bin_barcode_row = src.ptr<uint8_t>(y);
        //列的位置
        int pos = 0;
        //搜索像素，遇到黑点，可能是二维码的黑点，尝试寻找定位点
        for (; pos < width_bin_barcode; pos++) {
            if (bin_barcode_row[pos] == 0)
                break;
        }
        //遍历完了都没有黑点，这行没用，跳过
        if (pos == width_bin_barcode) { continue; }
        //先存入三个点防止下索引溢出
        pixels_position.push_back(pos);
        pixels_position.push_back(pos);
        pixels_position.push_back(pos);
        //碰到像素颜色发生反转，就记录位置
        uint8_t future_pixel = 255;
        for (int x = pos; x < width_bin_barcode; x++)
        {
            if (bin_barcode_row[x] == future_pixel)
            {
                future_pixel = 255 - future_pixel;
                pixels_position.push_back(x);
            }
        }
        //再存入一个点，即图像最外侧的一个点，这样保证每次到这里，数组里至少有五个点
        pixels_position.push_back(width_bin_barcode - 1);
        //计算1：1：3：1：1的比例
        for (size_t i = 2; i < pixels_position.size() - 4; i += 2)
        {
            //每两个突变点之间的距离
            test_lines[0] = static_cast<double>(pixels_position[i - 1] - pixels_position[i - 2]);
            test_lines[1] = static_cast<double>(pixels_position[i] - pixels_position[i - 1]);
            test_lines[2] = static_cast<double>(pixels_position[i + 1] - pixels_position[i]);
            test_lines[3] = static_cast<double>(pixels_position[i + 2] - pixels_position[i + 1]);
            test_lines[4] = static_cast<double>(pixels_position[i + 3] - pixels_position[i + 2]);
            //和
            double length = 0.0, weight = 0.0;
            //求和
            for (size_t j = 0; j < test_lines_size; j++) { length += test_lines[j]; }
            //异常指，退出
            if (length == 0) { continue; }
            //计算比例
            for (size_t j = 0; j < test_lines_size; j++)
            {
                if (j != 2) { weight += fabs((test_lines[j] / length) - 1.0 / 7.0); }
                else { weight += fabs((test_lines[j] / length) - 3.0 / 7.0); }
            }
            //小于阈值0.2，记录这条线。
            if (weight < 0.1)
            {
                Vec3d line;
                //起始位置横坐标
                line[0] = static_cast<double>(pixels_position[i - 2]);
                //纵坐标
                line[1] = y;
                //长度（横向）
                line[2] = length;
                result.push_back(line);
            }
        }
    }
    return result;
}

vector<Point2f>cqw::separateVerticalLines(const vector<Vec3d> &list_lines, Mat& src)
{
    //初始化
    vector<Vec3d> result;
    int temp_length;
    vector<Point2f> point2f_result;
    uint8_t next_pixel;
    vector<double> test_lines;
    //
    for (int coeff_epsilon = 1; coeff_epsilon < 2; coeff_epsilon++)
    {
        //每次都清理一下
        result.clear();
        temp_length = 0;
        point2f_result.clear();
        //对每一个横向搜索时找到的点
        for (size_t pnt = 0; pnt < list_lines.size(); pnt++)
        {
            //计算中点
            const int x = cvRound(list_lines[pnt][0] + list_lines[pnt][2] * 0.5);
            const int y = cvRound(list_lines[pnt][1]);
            test_lines.clear();
            uint8_t future_pixel_up = 255;
            //对每一列搜索
            for (int j = y; j < src.rows - 1; j++)
            {
                //列指针，从上面计算的中点所在的行开始向下
                next_pixel = src.ptr<uint8_t>(j + 1)[x];
                //计算每一条颜色带的长度
                temp_length++;
                //颜色发生反转时，将此时的宽度存下来
                if (next_pixel == future_pixel_up)
                {
                    future_pixel_up = 255 - future_pixel_up;
                    test_lines.push_back(temp_length);
                    temp_length = 0;
                    //存了三条以后，可以认为已经走出“定位点”
                    if (test_lines.size() == 3) { break; }
                }
            }
            //向下找，同理
            uint8_t future_pixel_down = 255;
            for (int j = y; j >= 1; j--)
            {
                next_pixel = src.ptr<uint8_t>(j - 1)[x];
                temp_length++;
                if (next_pixel == future_pixel_down)
                {
                    future_pixel_down = 255 - future_pixel_down;
                    test_lines.push_back(temp_length);
                    temp_length = 0;
                    if (test_lines.size() == 6) { break; }
                }
            }
            //计算1：1：3：1：1
            if (test_lines.size() == 6)
            {
                double length = 0.0, weight = 0.0;

                for (size_t i = 0; i < test_lines.size(); i++) { length += test_lines[i]; }

                CV_Assert(length > 0);
                for (size_t i = 0; i < test_lines.size(); i++)
                {
                    if (i % 3 != 0) { weight += fabs((test_lines[i] / length) - 1.0 / 7.0); }
                    else { weight += fabs((test_lines[i] / length) - 3.0 / 14.0); }
                }
                //偏差越小越好，但是因为拍摄时纵向容易失真，没办法固定，只能放大
                if (weight < 0.1 * coeff_epsilon)
                {
                    result.push_back(list_lines[pnt]);
                }
            }
        }
        //大于2才说明，可能找到了三个定位点
        if (result.size() > 2)
        {
            //计算y的中点
            for (size_t i = 0; i < result.size(); i++)
            {
                point2f_result.push_back(
                    Point2f(static_cast<float>(result[i][0] + result[i][2] * 0.5),
                        static_cast<float>(result[i][1])));
            }

            vector<Point2f> centers;
            Mat labels;
            double compactness;
            //对y的中点进行聚类
            compactness = kmeans(point2f_result, 3, labels,
                TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 0.1),
                3, KMEANS_PP_CENTERS, centers);
            if (compactness == 0) { continue; }
            if (compactness > 0) { break; }
        }
    }
    return point2f_result;
}

void cqw::fixationPoints(vector<Point2f> &local_point, Mat& src)
{
    //三个点的三个角，三条边
    double cos_angles[3], norm_triangl[3];
    //三条边的边长
    norm_triangl[0] = norm(local_point[1] - local_point[2]);
    norm_triangl[1] = norm(local_point[0] - local_point[2]);
    norm_triangl[2] = norm(local_point[1] - local_point[0]);
    //三个角的余弦
    cos_angles[0] = (norm_triangl[1] * norm_triangl[1] + norm_triangl[2] * norm_triangl[2]
        - norm_triangl[0] * norm_triangl[0]) / (2 * norm_triangl[1] * norm_triangl[2]);
    cos_angles[1] = (norm_triangl[0] * norm_triangl[0] + norm_triangl[2] * norm_triangl[2]
        - norm_triangl[1] * norm_triangl[1]) / (2 * norm_triangl[0] * norm_triangl[2]);
    cos_angles[2] = (norm_triangl[0] * norm_triangl[0] + norm_triangl[1] * norm_triangl[1]
        - norm_triangl[2] * norm_triangl[2]) / (2 * norm_triangl[0] * norm_triangl[1]);
    //如果角度异常的小，说明识别错误
    const double angle_barrier = 0.85;
    if (fabs(cos_angles[0]) > angle_barrier || fabs(cos_angles[1]) > angle_barrier || fabs(cos_angles[2]) > angle_barrier)
    {
        local_point.clear();
        return;
    }
    //找到最小的角
    size_t i_min_cos =
        (cos_angles[0] < cos_angles[1] && cos_angles[0] < cos_angles[2]) ? 0 :
        (cos_angles[1] < cos_angles[0] && cos_angles[1] < cos_angles[2]) ? 1 : 2;

    size_t index_max = 0;
    //面积
    double max_area = std::numeric_limits<double>::min();
    for (size_t i = 0; i < local_point.size(); i++)
    {
        //三个点的相对关系
        const size_t current_index = i % 3;
        const size_t left_index = (i + 1) % 3;
        const size_t right_index = (i + 2) % 3;

        const Point2f current_point(local_point[current_index]),//循环现在所在的点
            left_point(local_point[left_index]), right_point(local_point[right_index]),//左边的点和右边的点
            //求现在点到图像最下沿的交点
            central_point(intersectionLines(current_point,
                Point2f(static_cast<float>((local_point[left_index].x + local_point[right_index].x) * 0.5),
                    static_cast<float>((local_point[left_index].y + local_point[right_index].y) * 0.5)),
                Point2f(0, static_cast<float>(src.rows - 1)),
                Point2f(static_cast<float>(src.cols - 1),
                    static_cast<float>(src.rows - 1))));


        vector<Point2f> list_area_pnt;
        list_area_pnt.push_back(current_point);
        //三条线的迭代器
        //这三条线分别为到另外两个定位点的和到中心点的。
        vector<LineIterator> list_line_iter;
        list_line_iter.push_back(LineIterator(src, current_point, left_point));
        list_line_iter.push_back(LineIterator(src, current_point, central_point));
        list_line_iter.push_back(LineIterator(src, current_point, right_point));

        for (size_t k = 0; k < list_line_iter.size(); k++)
        {
            uint8_t future_pixel = 255, count_index = 0;
            for (int j = 0; j < list_line_iter[k].count; j++, ++list_line_iter[k])
            {
                if (list_line_iter[k].pos().x >= src.cols ||
                    list_line_iter[k].pos().y >= src.rows) {
                    break;
                }
                const uint8_t value = src.at<uint8_t>(list_line_iter[k].pos());
                //反转三次，认为出了定位点，记录一下该点的位置
                if (value == future_pixel)
                {
                    future_pixel = 255 - future_pixel;
                    count_index++;
                    if (count_index == 3)
                    {
                        list_area_pnt.push_back(list_line_iter[k].pos());
                        break;
                    }
                }
            }
        }
        //记录这三个点构成的面积取最大时的点的索引
        //如果是左上角那个点，因为此时到另外两个定位点的夹角最大，理论上此时的面积最大
        const double temp_check_area = contourArea(list_area_pnt);
        if (temp_check_area > max_area)
        {
            index_max = current_index;
            max_area = temp_check_area;
        }

    }
    //如果此时的最大面积的点的索引和余弦值最小的角的索引相同，那么这个点一定是定位点，这个时候让第一个点成为左上角的点
    if (index_max == i_min_cos) { std::swap(local_point[0], local_point[index_max]); }
    //不然出错，滚粗
    else { local_point.clear(); return; }
    //二维码的定位点一定是逆时针连续的，计算三个点，首尾相接的向量坐标的行列式，顺时针为负，逆时针为正，以此确定最终的定位点的顺序
    const Point2f rpt = local_point[0], bpt = local_point[1], gpt = local_point[2];
    Matx22f m(rpt.x - bpt.x, rpt.y - bpt.y, gpt.x - rpt.x, gpt.y - rpt.y);
    //最终的顺序为顺时针
    if (determinant(m) > 0)
    {
        std::swap(local_point[1], local_point[2]);
    }
}
bool cqw::computeTransformationPoints(vector<Point2f> &localization_points, vector<Point2f> &transformation_points,Mat& src)
{
    if (localization_points.size() != 3) { return false; }

    vector<Point> locations, non_zero_elem[3], newHull;
    vector<Point2f> new_non_zero_elem[3];
    for (size_t i = 0; i < 3; i++)
    {
        Mat mask = Mat::zeros(src.rows + 2, src.cols + 2, CV_8UC1);
        uint8_t next_pixel, future_pixel = 255;
        int count_test_lines = 0, index = cvRound(localization_points[i].x);
        //对定位点最外圈的黑边做填充
        for (; index < src.cols - 1; index++)
        {
            next_pixel = src.ptr<uint8_t>(cvRound(localization_points[i].y))[index + 1];
            if (next_pixel == future_pixel)
            {
                future_pixel = 255 - future_pixel;
                count_test_lines++;
                if (count_test_lines == 2)
                {
                    floodFill(src, mask,
                        Point(index + 1, cvRound(localization_points[i].y)), 255,
                        0, Scalar(), Scalar(), FLOODFILL_MASK_ONLY);
                    break;
                }
            }
        }
        Mat mask_roi = mask(Range(1, src.rows - 1), Range(1, src.cols - 1));
        findNonZero(mask_roi, non_zero_elem[i]);
        newHull.insert(newHull.end(), non_zero_elem[i].begin(), non_zero_elem[i].end());
    }
    //凸包
    convexHull(newHull, locations);
    for (size_t i = 0; i < locations.size(); i++)
    {
        for (size_t j = 0; j < 3; j++)
        {
            for (size_t k = 0; k < non_zero_elem[j].size(); k++)
            {
                if (locations[i] == non_zero_elem[j][k])
                {
                    new_non_zero_elem[j].push_back(locations[i]);
                }
            }
        }
    }

    double pentagon_diag_norm = -1;
    Point2f down_left_edge_point, up_right_edge_point, up_left_edge_point;
    //计算最大对角线
    for (size_t i = 0; i < new_non_zero_elem[1].size(); i++)
    {
        for (size_t j = 0; j < new_non_zero_elem[2].size(); j++)
        {
            double temp_norm = norm(new_non_zero_elem[1][i] - new_non_zero_elem[2][j]);
            if (temp_norm > pentagon_diag_norm)
            {
                down_left_edge_point = new_non_zero_elem[1][i];
                up_right_edge_point = new_non_zero_elem[2][j];
                pentagon_diag_norm = temp_norm;
            }
        }
    }

    if (down_left_edge_point == Point2f(0, 0) ||
        up_right_edge_point == Point2f(0, 0) ||
        new_non_zero_elem[0].size() == 0) {
        return false;
    }

    double max_area = -1;
    up_left_edge_point = new_non_zero_elem[0][0];

    for (size_t i = 0; i < new_non_zero_elem[0].size(); i++)
    {
        vector<Point2f> list_edge_points;
        list_edge_points.push_back(new_non_zero_elem[0][i]);
        list_edge_points.push_back(down_left_edge_point);
        list_edge_points.push_back(up_right_edge_point);
        //计算三个点的最大面积确定最左上角的点
        double temp_area = fabs(contourArea(list_edge_points));
        if (max_area < temp_area)
        {
            up_left_edge_point = new_non_zero_elem[0][i];
            max_area = temp_area;
        }
    }
    //分别找到，左下和右上两个定位点区域中，离左上角和已找到了另外连个定位点距离之和最远的点
    Point2f down_max_delta_point, up_max_delta_point;
    double norm_down_max_delta = -1, norm_up_max_delta = -1;
    for (size_t i = 0; i < new_non_zero_elem[1].size(); i++)
    {
        double temp_norm_delta = norm(up_left_edge_point - new_non_zero_elem[1][i])
            + norm(down_left_edge_point - new_non_zero_elem[1][i]);
        if (norm_down_max_delta < temp_norm_delta)
        {
            down_max_delta_point = new_non_zero_elem[1][i];
            norm_down_max_delta = temp_norm_delta;
        }
    }


    for (size_t i = 0; i < new_non_zero_elem[2].size(); i++)
    {
        double temp_norm_delta = norm(up_left_edge_point - new_non_zero_elem[2][i])
            + norm(up_right_edge_point - new_non_zero_elem[2][i]);
        if (norm_up_max_delta < temp_norm_delta)
        {
            up_max_delta_point = new_non_zero_elem[2][i];
            norm_up_max_delta = temp_norm_delta;
        }
    }

    transformation_points.push_back(down_left_edge_point);
    transformation_points.push_back(up_left_edge_point);
    transformation_points.push_back(up_right_edge_point);
    transformation_points.push_back(
        intersectionLines(down_left_edge_point, down_max_delta_point,
            up_right_edge_point, up_max_delta_point));

    vector<Point2f> quadrilateral = getQuadrilateral(transformation_points,src);
    transformation_points = quadrilateral;

    int width = src.size().width;
    int height = src.size().height;
    for (size_t i = 0; i < transformation_points.size(); i++)
    {
        if ((cvRound(transformation_points[i].x) > width) ||
            (cvRound(transformation_points[i].y) > height)) {
            return false;
        }
    }
    return true;
}
vector<Point2f> cqw::getQuadrilateral(vector<Point2f> angle_list,Mat& src)
{
    size_t angle_size = angle_list.size();
    uint8_t value, mask_value;
    Mat mask = Mat::zeros(src.rows + 2, src.cols + 2, CV_8UC1);
    Mat fill_bin_barcode = src.clone();
    for (size_t i = 0; i < angle_size; i++)
    {
        //和下一个点的直线迭代器
        LineIterator line_iter(src, angle_list[i% angle_size],
            angle_list[(i + 1) % angle_size]);
        //有黑色就填充，确定边界
        for (int j = 0; j < line_iter.count; j++, ++line_iter)
        {
            value = src.at<uint8_t>(line_iter.pos());
            mask_value = mask.at<uint8_t>(line_iter.pos() + Point(1, 1));
            if (value == 0 && mask_value == 0)
            {
                floodFill(fill_bin_barcode, mask, line_iter.pos(), 255,
                    0, Scalar(), Scalar(), FLOODFILL_MASK_ONLY);
            }
        }
    }
    vector<Point> locations;
    Mat mask_roi = mask(Range(1, src.rows - 1), Range(1, src.cols - 1));

    findNonZero(mask_roi, locations);

    for (size_t i = 0; i < angle_list.size(); i++)
    {
        int x = cvRound(angle_list[i].x);
        int y = cvRound(angle_list[i].y);
        locations.push_back(Point(x, y));
    }

    vector<Point> integer_hull;
    //计算 后来染出来的凸包
    convexHull(locations, integer_hull);
    int hull_size = (int)integer_hull.size();
    vector<Point2f> hull(hull_size);
    for (int i = 0; i < hull_size; i++)
    {
        float x = saturate_cast<float>(integer_hull[i].x);
        float y = saturate_cast<float>(integer_hull[i].y);
        hull[i] = Point2f(x, y);
    }

    const double experimental_area = fabs(contourArea(hull));

    vector<Point2f> result_hull_point(angle_size);
    double min_norm;
    //计算凸包与定位点的最近点
    for (size_t i = 0; i < angle_size; i++)
    {
        min_norm = std::numeric_limits<double>::max();
        Point closest_pnt;
        for (int j = 0; j < hull_size; j++)
        {
            double temp_norm = norm(hull[j] - angle_list[i]);
            if (min_norm > temp_norm)
            {
                min_norm = temp_norm;
                closest_pnt = hull[j];
            }
        }
        result_hull_point[i] = closest_pnt;
    }

    int start_line[2] = { 0, 0 }, finish_line[2] = { 0, 0 }, unstable_pnt = 0;
    //将四条线排好序
    for (int i = 0; i < hull_size; i++)
    {
        if (result_hull_point[2] == hull[i]) { start_line[0] = i; }
        if (result_hull_point[1] == hull[i]) { finish_line[0] = start_line[1] = i; }
        if (result_hull_point[0] == hull[i]) { finish_line[1] = i; }
        if (result_hull_point[3] == hull[i]) { unstable_pnt = i; }
    }

    int index_hull, extra_index_hull, next_index_hull, extra_next_index_hull;
    Point result_side_begin[4], result_side_end[4];
    //检查这条线是顺时针还是逆时针
    bool bypass_orientation = testBypassRoute(hull, start_line[0], finish_line[0]);

    min_norm = std::numeric_limits<double>::max();
    index_hull = start_line[0];
    do
    {
        if (bypass_orientation) { next_index_hull = index_hull + 1; }
        else { next_index_hull = index_hull - 1; }

        if (next_index_hull == hull_size) { next_index_hull = 0; }
        if (next_index_hull == -1) { next_index_hull = hull_size - 1; }
        //离哪个定位点近就选哪个
        Point angle_closest_pnt = norm(hull[index_hull] - angle_list[1]) >
            norm(hull[index_hull] - angle_list[2]) ? angle_list[2] : angle_list[1];
        //凸包路径线与最上沿的交点
        Point intrsc_line_hull =
            intersectionLines(hull[index_hull], hull[next_index_hull],
                angle_list[1], angle_list[2]);
        double temp_norm = getCosVectors(hull[index_hull], intrsc_line_hull, angle_closest_pnt);
        if (min_norm > temp_norm &&
            norm(hull[index_hull] - hull[next_index_hull]) >
            norm(angle_list[1] - angle_list[2]) * 0.1)
        {
            min_norm = temp_norm;
            result_side_begin[0] = hull[index_hull];
            result_side_end[0] = hull[next_index_hull];
        }


        index_hull = next_index_hull;
    } while (index_hull != finish_line[0]);

    if (min_norm == std::numeric_limits<double>::max())
    {
        result_side_begin[0] = angle_list[1];
        result_side_end[0] = angle_list[2];
    }

    min_norm = std::numeric_limits<double>::max();
    index_hull = start_line[1];
    bypass_orientation = testBypassRoute(hull, start_line[1], finish_line[1]);
    do
    {
        if (bypass_orientation) { next_index_hull = index_hull + 1; }
        else { next_index_hull = index_hull - 1; }

        if (next_index_hull == hull_size) { next_index_hull = 0; }
        if (next_index_hull == -1) { next_index_hull = hull_size - 1; }

        Point angle_closest_pnt = norm(hull[index_hull] - angle_list[0]) >
            norm(hull[index_hull] - angle_list[1]) ? angle_list[1] : angle_list[0];

        Point intrsc_line_hull =
            intersectionLines(hull[index_hull], hull[next_index_hull],
                angle_list[0], angle_list[1]);
        double temp_norm = getCosVectors(hull[index_hull], intrsc_line_hull, angle_closest_pnt);
        if (min_norm > temp_norm &&
            norm(hull[index_hull] - hull[next_index_hull]) >
            norm(angle_list[0] - angle_list[1]) * 0.05)
        {
            min_norm = temp_norm;
            result_side_begin[1] = hull[index_hull];
            result_side_end[1] = hull[next_index_hull];
        }

        index_hull = next_index_hull;
    } while (index_hull != finish_line[1]);

    if (min_norm == std::numeric_limits<double>::max())
    {
        result_side_begin[1] = angle_list[0];
        result_side_end[1] = angle_list[1];
    }

    bypass_orientation = testBypassRoute(hull, start_line[0], unstable_pnt);
    const bool extra_bypass_orientation = testBypassRoute(hull, finish_line[1], unstable_pnt);

    vector<Point2f> result_angle_list(4), test_result_angle_list(4);
    double min_diff_area = std::numeric_limits<double>::max();
    index_hull = start_line[0];
    const double standart_norm = std::max(
        norm(result_side_begin[0] - result_side_end[0]),
        norm(result_side_begin[1] - result_side_end[1]));
    do
    {
        if (bypass_orientation) { next_index_hull = index_hull + 1; }
        else { next_index_hull = index_hull - 1; }

        if (next_index_hull == hull_size) { next_index_hull = 0; }
        if (next_index_hull == -1) { next_index_hull = hull_size - 1; }

        if (norm(hull[index_hull] - hull[next_index_hull]) < standart_norm * 0.1)
        {
            index_hull = next_index_hull; continue;
        }

        extra_index_hull = finish_line[1];
        do
        {
            if (extra_bypass_orientation) { extra_next_index_hull = extra_index_hull + 1; }
            else { extra_next_index_hull = extra_index_hull - 1; }

            if (extra_next_index_hull == hull_size) { extra_next_index_hull = 0; }
            if (extra_next_index_hull == -1) { extra_next_index_hull = hull_size - 1; }

            if (norm(hull[extra_index_hull] - hull[extra_next_index_hull]) < standart_norm * 0.1)
            {
                extra_index_hull = extra_next_index_hull; continue;
            }

            test_result_angle_list[0]
                = intersectionLines(result_side_begin[0], result_side_end[0],
                    result_side_begin[1], result_side_end[1]);
            test_result_angle_list[1]
                = intersectionLines(result_side_begin[1], result_side_end[1],
                    hull[extra_index_hull], hull[extra_next_index_hull]);
            test_result_angle_list[2]
                = intersectionLines(hull[extra_index_hull], hull[extra_next_index_hull],
                    hull[index_hull], hull[next_index_hull]);
            test_result_angle_list[3]
                = intersectionLines(hull[index_hull], hull[next_index_hull],
                    result_side_begin[0], result_side_end[0]);

            const double test_diff_area
                = fabs(fabs(contourArea(test_result_angle_list)) - experimental_area);
            if (min_diff_area > test_diff_area)
            {
                min_diff_area = test_diff_area;
                for (size_t i = 0; i < test_result_angle_list.size(); i++)
                {
                    result_angle_list[i] = test_result_angle_list[i];
                }
            }

            extra_index_hull = extra_next_index_hull;
        } while (extra_index_hull != unstable_pnt);

        index_hull = next_index_hull;
    } while (index_hull != unstable_pnt);

    // check label points
    if (norm(result_angle_list[0] - angle_list[1]) > 2) { result_angle_list[0] = angle_list[1]; }
    if (norm(result_angle_list[1] - angle_list[0]) > 2) { result_angle_list[1] = angle_list[0]; }
    if (norm(result_angle_list[3] - angle_list[2]) > 2) { result_angle_list[3] = angle_list[2]; }

    // check calculation point
    if (norm(result_angle_list[2] - angle_list[3]) >
        (norm(result_angle_list[0] - result_angle_list[1]) +
            norm(result_angle_list[0] - result_angle_list[3])) * 0.5)
    {
        result_angle_list[2] = angle_list[3];
    }

    return result_angle_list;
}
inline double cqw::getCosVectors(Point2f a, Point2f b, Point2f c)
{
    return ((a - b).x * (c - b).x + (a - b).y * (c - b).y) / (norm(a - b) * norm(c - b));
}
//沿着一条线，向前推进，计算长度
bool cqw::testBypassRoute(vector<Point2f> hull, int start, int finish)
{
    int index_hull = start, next_index_hull, hull_size = (int)hull.size();
    double test_length[2] = { 0.0, 0.0 };
    //正着求
    do
    {
        next_index_hull = index_hull + 1;
        if (next_index_hull == hull_size) { next_index_hull = 0; }
        test_length[0] += norm(hull[index_hull] - hull[next_index_hull]);
        index_hull = next_index_hull;
    } while (index_hull != finish);

    index_hull = start;
    //倒着求
    do
    {
        next_index_hull = index_hull - 1;
        if (next_index_hull == -1) { next_index_hull = hull_size - 1; }
        test_length[1] += norm(hull[index_hull] - hull[next_index_hull]);
        index_hull = next_index_hull;
    } while (index_hull != finish);
    //正着大于倒着，对了
    if (test_length[0] < test_length[1]) { return true; }
    else { return false; }
}
bool cqw::updatePerspective(vector<Point2f>& original_points,Mat& src,Mat& dist,Mat& without)
{

    const Point2f centerPt = intersectionLines(original_points[0], original_points[2],
        original_points[1], original_points[3]);
    if (cvIsNaN(centerPt.x) || cvIsNaN(centerPt.y))
        return false;

    const Size temporary_size(cvRound(test_perspective_size), cvRound(test_perspective_size));

    vector<Point2f> perspective_points;
    perspective_points.push_back(Point2f(0.f, 0.f));
    perspective_points.push_back(Point2f(test_perspective_size, 0.f));

    perspective_points.push_back(Point2f(test_perspective_size, test_perspective_size));
    perspective_points.push_back(Point2f(0.f, test_perspective_size));

    perspective_points.push_back(Point2f(test_perspective_size * 0.5f, test_perspective_size * 0.5f));

    vector<Point2f> pts = original_points;
    pts.push_back(centerPt);

    Mat H = findHomography(pts, perspective_points);
    Mat bin_original;
    Mat temp_intermediate;
    warpPerspective(src, temp_intermediate, H, temporary_size, INTER_NEAREST);
    without = temp_intermediate(Range(1, temp_intermediate.rows), Range(1, temp_intermediate.cols));

    const int border = cvRound(0.1 * test_perspective_size);
    const int borderType = BORDER_CONSTANT;
    copyMakeBorder(without, dist , border, border, border, border, borderType, Scalar(255));
    return true;
}
//求交点
Point2f cqw::intersectionLines(Point2f a1, Point2f a2, Point2f b1, Point2f b2)
{
    Point2f result_square_angle(
        ((a1.x * a2.y - a1.y * a2.x) * (b1.x - b2.x) -
        (b1.x * b2.y - b1.y * b2.x) * (a1.x - a2.x)) /
            ((a1.x - a2.x) * (b1.y - b2.y) -
        (a1.y - a2.y) * (b1.x - b2.x)),
                ((a1.x * a2.y - a1.y * a2.x) * (b1.y - b2.y) -
        (b1.x * b2.y - b1.y * b2.x) * (a1.y - a2.y)) /
                    ((a1.x - a2.x) * (b1.y - b2.y) -
        (a1.y - a2.y) * (b1.x - b2.x))
    );
    return result_square_angle;
}
inline Point cqw::computeOffset(const vector<Point>& v)
{
    Rect areaBox = boundingRect(v);
    const int cStep = 7 * 2;
    Point offset = Point(areaBox.width, areaBox.height);
    offset /= cStep;
    return offset;
}
bool cqw::samplingForVersion(Mat& no_border_intermediate,Mat& straight)
{
    //版本，缩放
    const double multiplyingFactor =
        (version < 3) ? 1 :
        (version == 3) ? 1.5 :version*(version+1.0)/100;
    const Size newFactorSize(
        cvRound(no_border_intermediate.size().width  * multiplyingFactor),
        cvRound(no_border_intermediate.size().height * multiplyingFactor));
    Mat postIntermediate(newFactorSize, CV_8UC1);
    resize(no_border_intermediate, postIntermediate, newFactorSize, 0, 0, INTER_AREA);

    const int delta_rows = cvRound((postIntermediate.rows * 1.0) / version_size);
    const int delta_cols = cvRound((postIntermediate.cols * 1.0) / version_size);

    vector<double> listFrequencyElem;
    for (int r = 0; r < postIntermediate.rows; r += delta_rows)
    {
        for (int c = 0; c < postIntermediate.cols; c += delta_cols)
        {
            Mat tile = postIntermediate(
                Range(cvRound(r + delta_rows*0.2), min(cvRound(r + delta_rows*0.8), postIntermediate.rows)),
                Range(cvRound(c + delta_cols*0.2), min(cvRound(c + delta_cols*0.8), postIntermediate.cols)));
            const double frequencyElem = (countNonZero(tile) * 1.0) / tile.total();
            listFrequencyElem.push_back(frequencyElem);
        }
    }

    double dispersionEFE = std::numeric_limits<double>::max();
    double experimentalFrequencyElem = 0;
    for (double expVal = 0; expVal < 1; expVal += 0.001)
    {
        double testDispersionEFE = 0.0;
        for (size_t i = 0; i < listFrequencyElem.size(); i++)
        {
            testDispersionEFE += (listFrequencyElem[i] - expVal) *
                (listFrequencyElem[i] - expVal);
        }
        testDispersionEFE /= (listFrequencyElem.size() - 1);
        if (dispersionEFE > testDispersionEFE)
        {
            dispersionEFE = testDispersionEFE;
            experimentalFrequencyElem = expVal;
        }
    }

    straight = Mat(Size(version_size, version_size), CV_8UC1, Scalar(0));
    for (int r = 0; r < version_size * version_size; r++)
    {
        int i = r / straight.cols;
        int j = r % straight.cols;
        straight.ptr<uint8_t>(i)[j] = (listFrequencyElem[r] < experimentalFrequencyElem) ? 0 : 255;
    }
    return true;
}
bool cqw::decodeCqw(Mat mat,vector<vector<unsigned char>>& pageone,bool& idle) {
    bool flag=true;
    for (int col = 9; col < QRCODESIZE; col++) {
        vector<unsigned char> onepage;
        onepage.clear();
        bitset<88>code;
        bitset<80>data;
        bitset<8>check;
        bitset<8>countcheck;
        for (int row = 9; row < QRCODESIZE; row++) {
            if (mat.ptr<uint8_t>(row)[col] == 255)
                code[row - 9] = 1;
        }
        int i, j, k;
        for (k = 6, j = 1; j <= 88;j++) {
            if (j == 0x01 || j == 0x02 || j == 0x04 || j == 0x08 || j == 0x10 || j == 0x20 || j == 0x40) {
                check[k] = code[j-1];
                k--;
            }
        }
        for (i = 0,j=1; i < 80; i++,j++) {
            if (j == 0x01 || j == 0x02 || j == 0x04 || j == 0x08 || j == 0x10 || j == 0x20 || j == 0x40) {
                i--;
                continue;
            }
            data[i] = code[j - 1];
            if (j & 0x01 && data[i])
                countcheck[6].flip();
            if (j & 0x02 && data[i])
                countcheck[5].flip();
            if (j & 0x04 && data[i])
                countcheck[4].flip();
            if (j & 0x08 && data[i])
                countcheck[3].flip();
            if (j & 0x10 && data[i])
                countcheck[2].flip();
            if (j & 0x20 && data[i])
                countcheck[1].flip();
            if (j & 0x40 && data[i])
                countcheck[0].flip();
        }
        if (check != countcheck) {
            flag=false;
            continue;
//            bitset<8> buff;
//            buff=check ^ countcheck;
//            long x = buff.to_ullong();
//            code[x].flip();
//            for (int i = 0,j=3; i < 80; i++,j++) {
//                if (j == 0x01 || j == 0x02 || j == 0x04 || j == 0x08 || j == 0x10 || j == 0x20 || j == 0x40) {
//                    j++;
//                }
//                data[i] = code[j - 1];
//            }
        }
        for (int count = 0; count < 10; count++) {
            unsigned char c=0;
            for (int bit = 0; bit < 8; bit++) {
                c <<= 1;
                c |= data[count * 8 + (7-bit)];
            }
            onepage.push_back(c);
        }
//        if(pageone.size()!=88)
//            pageone.push_back(onepage);
//        else {
            pageone[col-9]=onepage;
//        }
    }
    for(int i=0;i<pageone.size();i++){
//        if(pageone[i]==NULL)
//            flag=false;
    }

    return flag;
}

