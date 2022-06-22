
#include "bsm.h"
#include <android/log.h>
#include <time.h>
#include <string.h>
#include <math.h>

// from android samples
/* return current time in milliseconds */
static double now_ms(void) {

    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000.0 * res.tv_sec + (double) res.tv_nsec / 1e6;
}

static void projection(Mat& src, vector<bbox>& objects){
    //initialize an array to store wihte pixels
    int *colswidth = new int[src.cols];
    memset(colswidth, 0, src.cols * 4);

    //store white pixels of the array into the array
    int color;
    for (int i = 0; i < src.cols; i ++){
        for (int j = 0; j < src.rows; j ++){
            color = (int)src.at<unsigned char>(j, i);

            if(color != 0){
//                __android_log_print(ANDROID_LOG_DEBUG, "bsm", "white pixels in this col  = %d", colswidth[i]);
                colswidth[i] ++;
            }
        }
    }

    //find the x aixs with projection larger than the threshold
    double threshold = 0.3;
    int x0;
    int y0;
    int x1;
    int y1;
    double centroid;

    for (int i = 0; i < src.cols; i ++){
        bool flag = true;
        __android_log_print(ANDROID_LOG_DEBUG, "bsm", "threshold result = %.2f", threshold * src.rows);

        for (int j = 0; j < colswidth[i] && colswidth[i] >= (threshold * src.rows); j ++){
            if (flag == true){
                __android_log_print(ANDROID_LOG_DEBUG, "bsm", "x  = %d", i);
                flag = false;
            }
        }
    }




}

//定义一个开操作
Mat kernel= getStructuringElement(MORPH_RECT,Size(5,5),Point(-1,-1));

//创建高期混合显示的Mat
Mat bsmMOG2;
Ptr<BackgroundSubtractor> pMOG2=createBackgroundSubtractorMOG2();

//创建KNN显示的Mat
Mat bsmKNNMat;
Ptr<BackgroundSubtractor> pKNN =
        createBackgroundSubtractorKNN();

//创建寻找轮廓
vector<vector<Point>> contours;

//高斯背景消除建模
vector<Mat> bsm::bsmMog2(Mat &src) {

    //运用高斯混合存放到bsmMOG2中
    pMOG2->apply(src, bsmMOG2);

    //运用开操作去掉部分干扰
    morphologyEx(bsmMOG2, bsmMOG2, MORPH_OPEN, kernel);

    //寻找轮廓
    findContours(bsmMOG2, contours, RETR_EXTERNAL,
                 CHAIN_APPROX_SIMPLE);
    for (int i = 0; i < contours.size(); ++i)
    {
        //判断轮廓大小，当大于5000时进入
        if(contourArea(contours[i])>5000)
        {
            //获取轮廓矩形
            Rect rect=cv::boundingRect(contours[i]);
            //在矩形的左上角点输出文字
            putText(bsmMOG2, "MOG2", Point(rect.x,rect.y),
                    FONT_HERSHEY_PLAIN, 2, Scalar(255, 255, 255));
            //画出矩形轮廓来
            rectangle(bsmMOG2, rect, Scalar(255, 0, 255), 2);
        }
    }

    bsmMOG2.copyTo(src);

    return vector<Mat>();
}

//KNN背景消息建模
vector<Mat> bsm::bsmKNN(Mat &src) {

    //运用KNN存放到bsmKNN中
    pKNN->apply(src, bsmKNNMat);

    //运用开操作去掉部分干扰
    morphologyEx(bsmKNNMat, bsmKNNMat, MORPH_OPEN, kernel);

    //寻找轮廓
    findContours(bsmKNNMat, contours, RETR_EXTERNAL,
                 CHAIN_APPROX_SIMPLE);
    for (int i = 0; i < contours.size(); ++i)
    {
        //判断轮廓大小，当大于5000时进入
        if(contourArea(contours[i])>5000)
        {
            //获取轮廓矩形
            Rect rect=boundingRect(contours[i]);
            //在矩形的左上角点输出文字
            putText(bsmKNNMat, "KNN", Point(rect.x,rect.y),
                    FONT_HERSHEY_PLAIN, 2, Scalar(255, 255, 255));
            //画出矩形轮廓来
            rectangle(bsmKNNMat, rect, Scalar(255, 0, 255), 2);
        }
    }

    bsmKNNMat.copyTo(src);

    return vector<Mat>();
}

vector<Mat> bsm::bsmMat(Mat &src) {

    //resize the input frame to promote efficiency
    Size srcSize = Size(192*3*0.7, 108*3*0.7);
    resize (src, src, srcSize, 0, 0, INTER_LINEAR);

    double start = now_ms(); // start time
    vector<Mat> output;
    vector<bbox> persons;

//    __android_log_print(ANDROID_LOG_DEBUG,"bsm", "the size of the input frame is %dx%dx%d", src.rows, src.cols, src.channels());
    //运用高斯混合存放到bsmMOG2中
    pMOG2->apply(src, bsmMOG2);
//    __android_log_print(ANDROID_LOG_DEBUG,"bsm", "the size of the crafted fame is %dx%dx%d", bsmMOG2.rows, bsmMOG2.cols, bsmMOG2.channels());
//        for (int j = 0; j < bsmMOG2.cols; j ++){
//            __android_log_print(ANDROID_LOG_DEBUG,"bsm", "the content of the src matrix is = %d ",(int)bsmMOG2.at<unsigned char>(100,j));
//
//        }

    //运用开操作去掉部分干扰
    morphologyEx(bsmMOG2, bsmMOG2, MORPH_OPEN, kernel);

    //vertically projection
    projection(bsmMOG2, persons);


    //在矩形的左上角点输出文字
    putText(bsmMOG2, "MOG2", Point(10,10),
            FONT_HERSHEY_SCRIPT_SIMPLEX , 4.0, Scalar(255, 255, 255));
    //最终生成的矩形存放进vector<Mat>中
    output.push_back(bsmMOG2);
    double end = now_ms(); // finish time
    double delta = end - start; // time your code took to exec in ms
    __android_log_print(ANDROID_LOG_DEBUG,"bsm", "The time used for bsm is %.2f",delta);

    //运用KNN存放到bsmKNN中
    pKNN->apply(src, bsmKNNMat);
    //运用开操作去掉部分干扰
    morphologyEx(bsmKNNMat, bsmKNNMat, MORPH_OPEN, kernel);
    //在矩形的左上角点输出文字
    putText(bsmKNNMat, "KNN", Point(40,10),
            FONT_HERSHEY_SCRIPT_SIMPLEX , 4.0, Scalar(255, 255, 255));
    //最终生成的矩形存放进vector<Mat>中
    output.push_back(bsmKNNMat);

    return output;
}