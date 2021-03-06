//
// Created by 36574 on 2019-08-26.
//

#ifndef OPENCVTESSERACTOCR_BSM_H
#define OPENCVTESSERACTOCR_BSM_H

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

struct bbox{
    int x0;
    int x1;
    int y0;
    int y1;
    pair<int, int> centroid;
};
class bsm {
public:
    static vector<Mat> bsmMog2(Mat &src);
    static vector<Mat> bsmKNN(Mat &src);
    static vector<Mat> bsmMat(Mat &src);
};


#endif //OPENCVTESSERACTOCR_BSM_H
