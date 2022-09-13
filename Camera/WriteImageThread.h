#ifndef WRITEIMAGETHREAD_H
#define WRITEIMAGETHREAD_H
#include <QThread>
#include <opencv4/opencv2/opencv.hpp>
class WriteImageThread : public QThread
{
public:
    WriteImageThread(cv::Mat image, std::string writeName);

private:
    cv::Mat _image;
    std::string _writeName;
    void writeThread();
};

#endif // WRITEIMAGETHREAD_H
