#ifndef GENCAMERA_H
#define GENCAMERA_H

#include <pylon/PylonIncludes.h>
#include <pylon/BaslerUniversalInstantCamera.h>
#include <opencv4/opencv2/opencv.hpp>
#include <unistd.h>
#include <opencv4/opencv2/core/core.hpp>
#include "WriteImageThread.h"
#include <QDebug>
#include <unistd.h>
#include <chrono>
#include <QDir>
#include <QTime>
#include "../AutoExpo/AutoExpo.h"
#include "../Uart/UartRequest.h"

class GenCamera : public QObject
{
    Q_OBJECT
public:
    static GenCamera& getInstance() {
        static GenCamera camera;
        return camera;
    }
    ~GenCamera();

public slots:
    void doWork();

private:
    explicit GenCamera();
    Pylon::CBaslerUniversalInstantCamera* _camera;
    Pylon::CGrabResultPtr _ptrGrabBuffer;
    Pylon::CGrabResultPtr _ptrGrabResult;
    void openCamera();
    void camera_init();
    void acquireImage();
    cv::Mat M_1,M;
    float lastt12=0;
    AutoExposure* autoExpo;

    void setExposure(int exposure);
    void setGain(int gain);
    void setROI(int x, int y, int width, int height);
    void setFixWhiteBalance(bool flag = true);
};

#endif // GENCAMERA_H
