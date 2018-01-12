﻿#include "header/feature.h"
#include "header/core_func.h"



namespace easypr {


Mat getHistogram(Mat in)
{
    const int VERTICAL = 0;
    const int HORIZONTAL = 1;

    // Histogram features
    Mat vhist = ProjectedHistogram(in, VERTICAL);
    Mat hhist = ProjectedHistogram(in, HORIZONTAL);

    // Last 10 is the number of moments components
    int numCols = vhist.cols + hhist.cols;

    Mat out = Mat::zeros(1, numCols, CV_32F);

    int j = 0;
    for (int i = 0; i < vhist.cols; i++)
    {
        out.at<float>(j) = vhist.at<float>(i);
        j++;
    }
    for (int i = 0; i < hhist.cols; i++)
    {
        out.at<float>(j) = hhist.at<float>(i);
        j++;
    }

    return out;
}


void getHistogramFeatures(const Mat& image, Mat& features)
{
    Mat grayImage;
    cvtColor(image, grayImage, CV_RGB2GRAY);

    //grayImage = histeq(grayImage);

    Mat img_threshold;
    threshold(grayImage, img_threshold, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);
    //Mat img_threshold = grayImage.clone();
    //spatial_ostu(img_threshold, 8, 2, getPlateType(image, false));

    features = getHistogram(img_threshold);
}


// compute color histom
void getColorFeatures(const Mat& src, Mat& features)
{
    Mat src_hsv;

    //grayImage = histeq(grayImage);
    cvtColor(src, src_hsv, CV_BGR2HSV);
    int channels = src_hsv.channels();
    int nRows = src_hsv.rows;

    // consider multi channel image
    int nCols = src_hsv.cols * channels;
    if (src_hsv.isContinuous())
    {
        nCols *= nRows;
        nRows = 1;
    }

    const int sz = 180;
    int h[sz] = { 0 };

    uchar* p;
    for (int i = 0; i < nRows; ++i)
    {
        p = src_hsv.ptr<uchar>(i);
        for (int j = 0; j < nCols; j += 3)
        {
            int H = int(p[j]);      // 0-180
            if (H > sz - 1) H = sz - 1;
            if (H < 0) H = 0;
            h[H]++;
        }
    }

    Mat mhist = Mat::zeros(1, sz, CV_32F);
    for (int j = 0; j < sz; j++)
    {
        mhist.at<float>(j) = (float)h[j];
    }

    // Normalize histogram
    double min, max;
    minMaxLoc(mhist, &min, &max);

    if (max > 0)
        mhist.convertTo(mhist, -1, 1.0f / max, 0);

    features = mhist;
}


void getHistomPlusColoFeatures(const Mat& image, Mat& features)
{
    // TODO
    Mat feature1, feature2;
    getHistogramFeatures(image, feature1);
    getColorFeatures(image, feature2);
    hconcat(feature1.reshape(1, 1), feature2.reshape(1, 1), features);
}


Mat charFeatures(Mat in, int sizeData)
{
    const int VERTICAL = 0;
    const int HORIZONTAL = 1;

    //cut the cetner, will afect 5% perices.
    Rect _rect = GetCenterRect(in);
    Mat tmpIn = CutTheRect(in, _rect);
    //Mat tmpIn = in.clone();

    // Low data feature
    Mat lowData;
    resize(tmpIn, lowData, Size(sizeData, sizeData));

    // Histogram features
    Mat vhist = ProjectedHistogram(lowData, VERTICAL);
    Mat hhist = ProjectedHistogram(lowData, HORIZONTAL);

    // Last 10 is the number of moments components
    int numCols = vhist.cols + hhist.cols + lowData.cols * lowData.cols;

    Mat out = Mat::zeros(1, numCols, CV_32F);
    // Asign values to

    int j = 0;
    for (int i = 0; i < vhist.cols; i++)
    {
        out.at<float>(j) = vhist.at<float>(i);
        j++;
    }
    for (int i = 0; i < hhist.cols; i++)
    {
        out.at<float>(j) = hhist.at<float>(i);
        j++;
    }
    for (int x = 0; x < lowData.cols; x++)
    {
        for (int y = 0; y < lowData.rows; y++)
        {
            out.at<float>(j) += (float)lowData.at <unsigned char>(x, y);
            j++;
        }
    }

    //std::cout << out << std::endl;

    return out;
}



Mat charProjectFeatures(const Mat& in, int sizeData)
{
    const int VERTICAL = 0;
    const int HORIZONTAL = 1;

    //SHOW_IMAGE(in, 0);
    // cut the cetner, will afect 5% perices.

    Mat lowData;
    resize(in, lowData, Size(sizeData, sizeData));

    //SHOW_IMAGE(lowData, 0);
    // Histogram features
    Mat vhist = ProjectedHistogram(lowData, VERTICAL);
    Mat hhist = ProjectedHistogram(lowData, HORIZONTAL);

    // Last 10 is the number of moments components
    int numCols = vhist.cols + hhist.cols;

    Mat out = Mat::zeros(1, numCols, CV_32F);

    int j = 0;
    for (int i = 0; i < vhist.cols; i++)
    {
        out.at<float>(j) = vhist.at<float>(i);
        j++;
    }
    for (int i = 0; i < hhist.cols; i++)
    {
        out.at<float>(j) = hhist.at<float>(i);
        j++;
    }
    //std::cout << out << std::endl;

    return out;
}



void getGrayPlusProject(const Mat& grayChar, Mat& features)
{
    // TODO: check channnels == 1
    //SHOW_IMAGE(grayChar, 0);
    //SHOW_IMAGE(255 - grayChar, 0);

    // resize to uniform size, like 20x32
    bool useResize = false;
    bool useConvert = true;
    bool useMean = true;
    //bool useLBP = false;

    Mat char_mat;
    if (useResize)
    {
        char_mat.create(kGrayCharHeight, kGrayCharWidth, CV_8UC1);
        resize(grayChar, char_mat, char_mat.size(), 0, 0, INTER_LINEAR);
    }
    else
    {
        char_mat = grayChar;
    }
    //SHOW_IMAGE(char_mat, 0);

    // convert to float
    Mat float_img;
    if (useConvert)
    {
        float scale = 1.f / 255;
        char_mat.convertTo(float_img, CV_32FC1, scale, 0);
    }
    else
    {
        float_img = char_mat;
    }
    //SHOW_IMAGE(float_img, 0);

    // cut from mean, it can be optional

    Mat mean_img;
    if (useMean)
    {
        float_img -= mean(float_img);
        mean_img = float_img;
    }
    else
    {
        mean_img = float_img;
    }
    //SHOW_IMAGE(mean_img, 0);

    // use lbp to get features, it can be changed to other
    Mat feautreImg;

    feautreImg = mean_img.reshape(1, 1);

    Mat binaryChar;
    threshold(grayChar, binaryChar, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);
    Mat projectFeature = charProjectFeatures(binaryChar, 32);

    hconcat(feautreImg.reshape(1, 1), projectFeature.reshape(1, 1), features);
}


}
