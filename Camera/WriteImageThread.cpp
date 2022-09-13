#include "WriteImageThread.h"
#include <QDebug>
WriteImageThread::WriteImageThread(cv::Mat image, std::string writeName) :
    _image(image),
    _writeName(writeName) {
    connect(this, &QThread::started, this, &WriteImageThread::writeThread);
    connect(this, &QThread::finished, this, &WriteImageThread::deleteLater);
}


void WriteImageThread::writeThread() {
    qDebug()<<QString::fromStdString(_writeName)<<endl;

    //write the image as png format without compression
    std::vector<int> flag;
    flag.push_back(cv::IMWRITE_PNG_COMPRESSION);
    flag.push_back(0);
    cv::imwrite(_writeName, _image, flag);

    this->exit();
}
