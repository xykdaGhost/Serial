
#include "AutoExpo.h"
#include "omp.h"
using namespace cv;
using std::cout;
using std::endl;

//2022.1.11 lmd
template<class T>
    inline T max_min(T cur,T minv, T maxv) {
        T tm = cur < maxv ? cur : maxv;
        return tm > minv ? tm : minv;
    }

void AutoExposure::logMapping(Mat& img, Mat& out)
{
    assert(img.channels() == 1 && img.type() == CV_32FC1);
    if (out.empty())
        out.create(img.size(), img.type());
    else
        assert(img.size() == out.size());

    float N = log(mLambda * (1 - mDelta) + 1);

    float* pdata = (float*)img.data;
    float* presult = (float*)out.data;
    assert(img.isContinuous() && out.isContinuous());
    int size = out.rows * out.cols;
    for (int i = 0; i < size; i++) {
        presult[i] = (pdata[i] < mDelta) ? 0 : log(mLambda * (pdata[i] - mDelta) + 1) / N;
    }

    return;
}

float AutoExposure::computeIQ(Mat& img)
{
    assert(img.channels() == 1 && img.type() == CV_32FC1);


    Mat gradX, gradY;
    Mat kernelX = (Mat_<float>(3, 3) << -1, 0, 1, -2, 0, 2, -1, 0, 1);
    Mat kernelY = (Mat_<float>(3, 3) << -1, -2, -1, 0, 0, 0, 1, 2, 1);
    filter2D(img, gradX, CV_32FC1, kernelX.mul(0.25));
    filter2D(img, gradY, CV_32FC1, kernelY.mul(0.25));


    Mat gradM, normalizedGradM;
    magnitude(gradX, gradY, gradM);
    normalizedGradM = gradM.mul(1. / sqrt(2));

    Mat logGradM;
    logMapping(normalizedGradM, logGradM);

    Scalar img_quality = sum(logGradM);

    return img_quality[0];
}

void AutoExposure::gammaCorrection(Mat& img, Mat& out, float gamma)
{
    assert(img.channels() == 1 && (img.type() == CV_8UC1) || (img.type() == CV_16UC1));
    assert(out.empty());
    out.create(img.size(), CV_32FC1);


    int maxPixel = img.type() == CV_8UC1 ? 255 : 65535;

    vector<float> lut(maxPixel + 1, 0);
    for (int i = 0; i <= maxPixel; i++) {
        lut[i] = pow(float(i) / maxPixel, gamma);
    }


    float* presult = (float*)out.data;
    if (img.type() == CV_8UC1) {
        unsigned char* pdata = img.data;
        #pragma omp parallel for
        for (int i = 0; i < img.rows; i++) {
            for (int j = 0; j < img.cols; j++) {
                presult[i * img.cols + j] = lut[pdata[i * img.cols + j]];
            }
        }
    }
    else {
        unsigned short* pdata = (unsigned short*)img.data;
        #pragma omp parallel for
        for (int i = 0; i < img.rows; i++) {
            for (int j = 0; j < img.cols; j++) {
                presult[i * img.cols + j] = lut[pdata[i * img.cols + j]];
            }
        }
    }

    return;
}


bool AutoExposure::polyFit(vector<cv::Point2f>& points, int n, Mat& A)
{
    //Number of key points
    int N = points.size();

    cv::Mat X = cv::Mat::zeros(n + 1, n + 1, CV_64FC1);
    for (int i = 0; i < n + 1; i++) {
        for (int j = 0; j < n + 1; j++) {
            for (int k = 0; k < N; k++) {
                X.at<double>(i, j) = X.at<double>(i, j) +
                    std::pow(points[k].x, i + j);
            }
        }
    }

    cv::Mat Y = cv::Mat::zeros(n + 1, 1, CV_64FC1);
    for (int i = 0; i < n + 1; i++) {
        for (int k = 0; k < N; k++) {
            Y.at<double>(i, 0) = Y.at<double>(i, 0) +
                std::pow(points[k].x, i) * points[k].y;
        }
    }

    A = cv::Mat::zeros(n + 1, 1, CV_64FC1);

    cv::solve(X, Y, A, cv::DECOMP_LU);
    return true;
}
float AutoExposure::getOptimGamma(Mat& img)
{
    assert(img.channels() == 1 && (img.type() == CV_8UC1) || (img.type() == CV_16UC1));

    int num = mvAnchorGammas.size();

    vector<float> iqs(num, 0.0);

#pragma omp parallel for
    for (int i = 0; i < num; i++) {
        Mat correctedImg;
        gammaCorrection(img, correctedImg, mvAnchorGammas[i]);
        float iq = computeIQ(correctedImg);
        iqs[i] = iq;
    }

    vector<Point2f> points;
    for (int i = 0; i < iqs.size(); i++) {
        points.push_back(Point2f(mvAnchorGammas[i], iqs[i]));
    }
    Mat A;
    polyFit(points, 5, A);

    float start = mvAnchorGammas[0], end = mvAnchorGammas[mvAnchorGammas.size() - 1];
    float step = (end - start) / 100;
    float maxGamma = 0.0, maxIQ = 0.0;
    for (int i = 0; i <= 100; i++) {
        float x = start + i * step;
        float y = A.at<double>(0, 0) + A.at<double>(1, 0) * x +
            A.at<double>(2, 0) * std::pow(x, 2) + A.at<double>(3, 0) * std::pow(x, 3) +
            A.at<double>(4, 0) * std::pow(x, 4) + A.at<double>(5, 0) * std::pow(x, 5);

        if (y > maxIQ) {
            maxIQ = y;
            maxGamma = x;
        }
    }

    return maxGamma;
}

vector<float> AutoExposure::getNextExpTime(Mat& img, float Speed)
{
    auto t1 = std::chrono::steady_clock::now();

    Mat resizedImg;
    int size = img.rows * img.cols;
    if (size > mSize) {
        int h = img.rows / sqrt((float)size / mSize);
        int w = img.cols / sqrt((float)size / mSize);
        resize(img, resizedImg, Size(w, h));
    }
    else {
        resizedImg = img;
    }

    Mat grayImg;
    if (resizedImg.channels() == 3)
        cvtColor(resizedImg, grayImg, COLOR_BGR2GRAY);
    else
        grayImg = resizedImg;

    float optimGamma = getOptimGamma(grayImg);

//    imwrite("/home/mv/expo.png", grayImg);

    float alpha = optimGamma >= 1 ? 0.5 : 1;
    float Gain_k = 1 + alpha * mKp * (1 - optimGamma);
    float nextExpTime = mCurrExpTime * (1 + alpha * mKp * (1 - optimGamma));

    //add speed ,output Gain and control signal
    //first ExpTime, second speed, third led control, fourth Gain
    vector<float> nextExpTime_Gain_Control;
    nextExpTime_Gain_Control = control_ExpTime(nextExpTime, Speed, Gain_k);
    mCurrExpTime = nextExpTime_Gain_Control[0];
    mCurrGain = nextExpTime_Gain_Control[1];

    auto t2 = std::chrono::steady_clock::now();
    auto t = t2 - t1;
    std::cout<<"time autoexposure:"<<std::chrono::duration<double>(t).count()<<"s"<<std::endl;

    return nextExpTime_Gain_Control;
}

void AutoExposure::calhiststatics(cv::Mat hi){
        Cb = 0, Cb1 = 0, Cm = 0, Cw1 = 0, Cw = 0;
        for (int i = 0; i <= Bd1; ++i) {
            Cb += hi.at<float>(i);
        }
        for (int i = Bd1 + 1; i <= Bd2; ++i) {
            Cb1 += hi.at<float>(i);
        }
        for (int i = Bd2 + 1; i < Bu1; ++i) {
            Cm += hi.at<float>(i);
        }
        for (int i = Bu1; i < Bu2; ++i) {
            Cw1 += hi.at<float>(i);
        }
        for (int i = Bu2; i <= 255; ++i) {
            Cw += hi.at<float>(i);
        }
        lastExpTime = 0.0, lastGain = 0.0, lastave = 0.0; //ÖÃ0
    }

vector<float> AutoExposure::getMyNextExpTime(Mat& img, float speed){
    float currExpTime = mCurrExpTime;
    float currGain = mCurrGain;
    calhiststatics(img);
    vector<float> a{ currExpTime,currGain,0 };
    float nextExpTime, nextGain, maxExpTime;
    if (speed > 0.01) maxExpTime = 10 / speed;//±ÜÃâ³ýÊýÎª0
    else maxExpTime = 2;//Ÿ²Ös¹µÄ×îŽóÆØ¹â

    //meanStdDev(img, ave, stddev); //ave.at<double>(0, 0)
    float ave = mean(img)[0];


    float delta_f = ((Cb + Cw) == 0) ? 0 : ((float)kkk * (Cw - Cb) / (Cb + Cw));
    float delta_s = ((Cb1 + Cw1)==0)?0:((float)kkk * (Cw1 - Cb1) / (Cb1 + Cw1));
    float delta = k_f*delta_f + k_s*delta_s;
    float targetmid = 128 - delta;

    if ((ave < (targetmid - kkk)) || (ave > (targetmid + kkk))) {
        nextExpTime = k_p*(targetmid - ave)/(ave - lastave) * (currExpTime - lastExpTime+1)+currExpTime;
        //ÏµÊý²»ÓŠžÃÈ¡ŸöÓÚŸàÀë128µÄÔ¶œü£¬¶øÓŠžÃÈ¡ŸöÓÚ»·Ÿ³³¡Ÿ°µÄÃ÷°µ³Ì¶È

        //ÏÂÒ»Ö¡ÈÔÈ»ÅÐ¶ÏÒªÔöÇ¿
        if (nextExpTime >= maxExpTime) {
            nextGain = k_gain*(targetmid - ave)*(nextExpTime - maxExpTime)/maxExpTime+ currGain;
        }
        //ÏÂÒ»Ö¡²»ÓÃŒÌÐøÔöÇ¿£¬ÐèÒªÏ÷Èõ£¬²»ÅÅ³ýÔöÒæ×îŽó¹ýµ÷
        else {
            //µ±Ç°ÔöÒæ±ŸÉíÎª0
            if (abs(currGain) < 0.001) {
                nextGain = 0.0;
            }
            //µ±Ç°ÔöÒæŽæÔÚ£¬ÐèÒªÏÂµ÷£¬orÖ±œÓÎª0
            else {
                nextGain = k_gain*(targetmid - ave) / (ave - lastave)*(currGain - lastGain+2) + currGain;// ŒÆËã²»³öžºÖµ
                //ŒÆËã³öÔöÒæÎªžºÖµ
                if (nextGain <= 0.0) {
                    float deltaave = nextGain*(ave - lastave) / (currGain - lastGain + 1);
                    nextExpTime = k_p*(deltaave) / (ave - lastave) * (currExpTime - lastExpTime+1) + currExpTime;
                    nextGain = 0.0;
                }
            }

        }
        a[0] = max_min<float>(nextExpTime, 0.2, maxExpTime);
        a[1] = max_min<float>(nextGain, 0, maxgain);
    }
    //whitebalance
    if (lastave < 70 && abs(a[0] - maxExpTime) < 0.001) {
        a[2] = 0; //fixed
    }
    else {
        a[2] = 1; //auto
    }
    lastave = ave;
    lastExpTime = currExpTime;
    lastGain = currGain;
    mCurrExpTime = a[0];
    mCurrGain = a[1];

    return a;

}

vector<float> AutoExposure::control_ExpTime(float ExpTime, float Speed,float Gain_k)
{
    //ParamManage& paramManage = ParamManage::getInstance();

    vector<float> nextExpTime_Gain_Control;
    float nextExpTime = ExpTime;
    float nextGain = 0;
    float led_control = 0;
    float warning_signal = 0;

    float mMinExpTimeXSpeed;
    float mMaxExpTimeXSpeed;

    //the MaxExpTime's relationship with Speed
    //the fastest is 15km/h and the MaxExpTime is 0.5ms
    //the slowest is 3km/h and the MaxExpTime is 2ms
//    mMinExpTimeXSpeed = paramManage.model()->paramStruct().aec.minExpTime;
 //   mMaxExpTimeXSpeed = paramManage.model()->paramStruct().aec.expTime_b + Speed * paramManage.model()->paramStruct().aec.expTime_a;
//    mMaxExpTimeXSpeed /= 100;
    cout<<"min exposure time"<<mMinExpTimeXSpeed<<" max exposure time"<<mMaxExpTimeXSpeed<<endl;
    //the mMinExpTime and mMaxExpTime are related to Speed
    if (nextExpTime < mMinExpTimeXSpeed)
        nextExpTime = mMinExpTimeXSpeed;
    if (nextExpTime > mMaxExpTimeXSpeed)
        nextExpTime = mMaxExpTimeXSpeed;

    //how to control led
    //1. ExpTime is not enough all the time
    //2, ExpTime is not enough but enough when led is on
    //3, ExpTime is enough all the time
    //4. ExpTime is enough but not enough when led is off
//    float i = ExpTime / mMaxExpTimeXSpeed;
//    if (ExpTime > mMaxExpTimeXSpeed)
//        led_control = (i >= led_control_a) ? 1 : led_control;
//    else
//        led_control = (i >= led_control_b) ? led_control : 0;

    //how to control Gain
    if (ExpTime > mMaxExpTimeXSpeed && led_control == 1)
    {
        if (mCurrGain == 0)
            nextGain = Gain_k * (mCurrGain + 1);
        else
            nextGain = Gain_k * mCurrGain;
        /*float j = ExpTime / mMaxExpTimeXSpeed;
        if (j > MaxGain)
            nextGain = MaxGain;
        else if (MidGain <= j <= MaxGain)
            nextGain = (mCurrGain+1) * j;
        else
            nextGain = nextGain;*/
    }
    else {
        nextGain = 0;
    }
//    if (nextGain >= paramManage.model()->paramStruct().aec.maxGain)
//        nextGain = paramManage.model()->paramStruct().aec.maxGain;
//    if (nextGain < paramManage.model()->paramStruct().aec.minGain)
//        nextGain = paramManage.model()->paramStruct().aec.minGain;

    //how to control warning_signal
//    if (nextGain == paramManage.model()->paramStruct().aec.maxGain && led_control == 1 && ExpTime > mMaxExpTimeXSpeed)
//        warning_signal = 1;
//    else
//        warning_signal = 0;

    nextExpTime_Gain_Control.push_back(nextExpTime);
    nextExpTime_Gain_Control.push_back(nextGain);
    nextExpTime_Gain_Control.push_back(led_control);
    nextExpTime_Gain_Control.push_back(warning_signal);
    return nextExpTime_Gain_Control;
}

void AutoExposure::setCurrExpTime(float currExpTime)
{
    mCurrExpTime = currExpTime;
}

float AutoExposure::getCurrExpTime()
{
    return mCurrExpTime;
}
