#ifndef KERNEL_RUNNER_H
#define KERNEL_RUNNER_H
#include "host/mango.h"
#include "smart_surveillance_types.h"
class KernelRunner
{
  public:
    KernelRunner(int minIOUTracker, int minIOUMotion,std::string videoPath, std::string modelPath,std::string configPath, std::string classesPath);
    void run_kernel();
    ~KernelRunner();
  private:
    mango::BBQContext *mango_rt;
    mango::TaskGraph *tg;
    mango::KernelArguments *argsKMOTION;
    mango::KernelArguments *argsKCLASSIFIER;
    mango::KernelArguments *argsKTRACKER;
    mango::KernelArguments *argsKGUI;

    enum { HOST=0, KMOTION, KCLASSIFIER, KTRACKER, KGUI };
    enum { MOTION_VIDEO_IN=1,MOTION_RECT_OUT,MOTION_MAT_OUT, MOTION_NUM_RECT, CLASSIFIER_MODEL_IN,
    CLASSIFIER_CONFIG_IN,CLASSIFIER_MAT_OUT, CLASSIFIER_DETECTION_OUT, CLASSIFIER_NUM_DETECTION,
    TRACKER_MAT_OUT,TRACKER_INFO_OUT, TRACKER_NUM_TRACKS,GUI_CLASSES_PATH, OUTPUT};
};
#endif
