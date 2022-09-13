#include "GenCamera.h"

using namespace Pylon;
using namespace Basler_UniversalCameraParams;
using namespace GenApi;

static QString current_time = QDateTime::currentDateTime().toString("yyyyMMdd_hh_mm");
static bool new_folder_flag = true;
static int photo_name_expo_time = 0;
static int photo_name_gain = 0;
static int photo_name_id = 1;
static int speed = 5;
static int trynum = 0;

inline cv::Point getTargetPoint(cv::Point pt_origin, cv::Mat warpMatrix) {
    cv::Mat_<double> mat_pt(3, 1);
    mat_pt(0, 0) = pt_origin.x;
    mat_pt(1, 0) = pt_origin.y;
    mat_pt(2, 0) = 1;
    cv::Mat mat_pt_view = warpMatrix * mat_pt;
    double a1 = mat_pt_view.at<double>(0, 0);
    double a2 = mat_pt_view.at<double>(1, 0);
    double a3 = mat_pt_view.at<double>(2, 0);
    return cv::Point(a1 * 1.0 / a3, a2 * 1.0 / a3);
}

inline int minmax(int pixel,int minv,int maxv) {
    pixel = pixel > maxv ? maxv : pixel;
    pixel = pixel < minv ? minv : pixel;
    return pixel;
}

GenCamera::GenCamera()
{
    autoExpo = new AutoExposure();
}

void GenCamera::camera_init()
{
    PylonInitialize();
    openCamera();
}

void GenCamera::openCamera()
{
    try {
        _camera = new CBaslerUniversalInstantCamera(CTlFactory::GetInstance().CreateFirstDevice());
        //_camera->RegisterConfiguration(new CSoftwareTriggerConfiguration, RegistrationMode_ReplaceAll, Cleanup_Delete);
        _camera->Open();

        // Get camera device information.
        qDebug() << "Camera Device Information" << endl
             << "=========================" << endl;
        qDebug() << "Vendor           : "
             << _camera->DeviceVendorName.GetValue() << endl;
        qDebug() << "Model            : "
             << _camera->DeviceModelName.GetValue() << endl;
        qDebug() << "Firmware version : "
             << _camera->DeviceFirmwareVersion.GetValue() << endl;
        qDebug() << "Format : "
             << _camera->PixelFormat.ToString() << endl;
        // Camera settings.
        qDebug() << "Camera Device Settings" << endl
             << "======================" << endl;
        _camera->MaxNumBuffer = 25;



        _camera->TriggerSelector.SetValue(TriggerSelector_FrameStart);
        _camera->TriggerMode.SetValue(TriggerMode_On); //TriggerMode_Off TriggerMode_OnŽ¥·¢Ä£Êœ TriggerModeEnums
        _camera->TriggerSource.SetValue(TriggerSource_Line1); // TriggerSource_Line1 TriggerSourceEnums TriggerSource_Software
        _camera->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
        _camera->BalanceWhiteAuto.SetValue(BalanceWhiteAuto_Continuous);// BalanceWhiteAuto_Continuous Once

        _camera->TriggerActivation.SetValue(TriggerActivation_RisingEdge);
        _camera->StartGrabbing(GrabStrategy_LatestImageOnly);
        _camera->RegisterConfiguration(new CSoftwareTriggerConfiguration, RegistrationMode_ReplaceAll, Cleanup_Delete);
        std::vector<double> Mdata{ 0.46464507552316109,0.31322056430355943,-384.00841183616387,0.0000000000000000,-0.91177886268307751,1117.8408856494530,-2.5774733953409634e-19,0.0017305003552682847,1.0000000000000000 };
        M.create(3, 3, CV_64FC1);
        for (int i = 0; i < 3; ++i) {
            double* data = M.ptr<double>(i);
            for (int j = 0; j < 3; ++j) {
                data[j] = Mdata[i * 3 + j];
            }
        }
        cv::invert(M, M_1);
    } catch (const GenericException& e) {
        // Error handling.
        std::cerr << "An exception occurred. from open" << std::endl
            << e.GetDescription() << std::endl;
    }
}

/**
 * @brief Destructor of GenCamera
 */
GenCamera::~GenCamera() {
    try {
        if(_camera != nullptr)
            _camera->Close();
        PylonTerminate();
    }  catch (const GenericException& e) {
        // Error handling.
        std::cerr << "An exception occurred. from destructor" << std::endl
            << e.GetDescription() << std::endl;
    }

}


void GenCamera::doWork()
{

    camera_init();
    while (true) {
        acquireImage();
    }
}


void GenCamera::acquireImage()
{
    try {
        CGrabResultPtr ptr;
        if (_camera->IsGrabbing())
        {
            _camera->WaitForFrameTriggerReady(4000, TimeoutHandling_ThrowException);
            _camera->RetrieveResult(15000, ptr, TimeoutHandling_ThrowException);
            if (ptr->GrabSucceeded()) {
                qDebug() << "success";
            }
            else {
                std::cout << "Error: " << ptr->GetErrorCode() << " " << _ptrGrabBuffer->GetErrorDescription() << endl; //·Çµ÷ÊÔœ×¶ÎÊ¹ÓÃ×îºÃ×¢ÊÍµô
            }
        }

    }  catch (const GenericException& e) {
        // Error handling.
        std::cerr << "An exception occurred. from grab" << std::endl
            << e.GetDescription() << std::endl;
        UartRequest::getInstance().requestNewMessage(1);
    }
}


