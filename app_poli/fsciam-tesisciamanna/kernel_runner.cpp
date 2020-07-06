#include "kernel_runner.h"
#include "Object.h"
#include <opencv2/core.hpp>
#include <memory>


KernelRunner::KernelRunner(int minIOUTracker, int minIOUMotion,std::string videoPath, std::string modelPath,std::string configPath, std::string classesPath)
{
  //Init logger
  mango::mango_init_logger();
  //Initialize runtime library, recipe generic
  mango_rt = new mango::BBQContext("smart_surveillance", "surveillance");

  //create kernel functions
  auto kf_motion = new mango::KernelFunction();
  auto kf_classifier = new mango::KernelFunction();
  auto kf_tracker= new mango::KernelFunction();
  auto kf_gui= new mango::KernelFunction();

  //load kernel
  kf_motion->load("/opt/mango/usr/local/share/smart_surveillance/motion_kernel/motion_kernel", mango::UnitType::GN, mango::FileType::BINARY);
  kf_classifier->load("/opt/mango/usr/local/share/smart_surveillance/classifier_kernel/classifier_kernel", mango::UnitType::GN, mango::FileType::BINARY);
  kf_tracker->load("/opt/mango/usr/local/share/smart_surveillance/tracker_kernel/tracker_kernel", mango::UnitType::GN, mango::FileType::BINARY);
  kf_gui->load("/opt/mango/usr/local/share/smart_surveillance/gui_kernel/gui_kernel", mango::UnitType::GN, mango::FileType::BINARY);

  //registration of kernel functions into task graph
  auto k_motion  = mango_rt->register_kernel(KMOTION, kf_motion, {MOTION_VIDEO_IN}, {MOTION_RECT_OUT,MOTION_MAT_OUT,MOTION_NUM_RECT});
  auto k_classifier  = mango_rt->register_kernel(KCLASSIFIER, kf_classifier, {MOTION_RECT_OUT,MOTION_NUM_RECT,MOTION_MAT_OUT,CLASSIFIER_MODEL_IN,CLASSIFIER_CONFIG_IN},
    {CLASSIFIER_MAT_OUT, CLASSIFIER_DETECTION_OUT,CLASSIFIER_NUM_DETECTION});
  auto k_tracker  = mango_rt->register_kernel(KTRACKER, kf_tracker, {CLASSIFIER_MAT_OUT, CLASSIFIER_DETECTION_OUT,CLASSIFIER_NUM_DETECTION}, {TRACKER_MAT_OUT,TRACKER_INFO_OUT,TRACKER_NUM_TRACKS});

  auto k_gui = mango_rt->register_kernel(KGUI, kf_gui,{TRACKER_MAT_OUT,TRACKER_INFO_OUT,TRACKER_NUM_TRACKS,GUI_CLASSES_PATH},{OUTPUT});
  //registration of Buffers


  //KMOTION IN
  auto b_motion_video_in=mango_rt->register_buffer(MOTION_VIDEO_IN,(videoPath.size()+1)*sizeof(char),{HOST},{KMOTION});
  //KMOTION OUT
  auto b_motion_mat_out=mango_rt->register_buffer(MOTION_MAT_OUT,416*416*3*sizeof(uchar),{KMOTION},{KCLASSIFIER});
  auto b_motion_rect_out=mango_rt->register_buffer(MOTION_RECT_OUT,sizeof(cv::Rect),{KMOTION},{KCLASSIFIER});
  auto b_motion_num_rect=mango_rt->register_buffer(MOTION_NUM_RECT,sizeof(int),{KMOTION},{KCLASSIFIER});

  //KCLASSIFIER IN
  auto b_classifier_model_in=mango_rt->register_buffer(CLASSIFIER_MODEL_IN,(modelPath.size()+1)*sizeof(char),{HOST},{KCLASSIFIER});
  auto b_classifier_config_in=mango_rt->register_buffer(CLASSIFIER_CONFIG_IN,(configPath.size()+1)*sizeof(char),{HOST},{KCLASSIFIER});
  //KCLASSIFIER OUT
  auto b_classifier_mat_out=mango_rt->register_buffer(CLASSIFIER_MAT_OUT,416*416*3*sizeof(uchar),{KCLASSIFIER},{KTRACKER});
  auto b_classifier_detection_out=mango_rt->register_buffer(CLASSIFIER_DETECTION_OUT,sizeof(Object),{KCLASSIFIER},{KTRACKER});
  auto b_classifier_num_det=mango_rt->register_buffer(CLASSIFIER_NUM_DETECTION,sizeof(int),{KCLASSIFIER},{KTRACKER});

  //KTRACKER
  auto b_tracker_mat_out=mango_rt->register_buffer(TRACKER_MAT_OUT,416*416*3*sizeof(uchar),{KTRACKER},{KGUI});
  auto b_tracker_info_out=mango_rt->register_buffer(TRACKER_INFO_OUT,sizeof(gui_result_t),{KTRACKER},{KGUI});
  auto b_tracker_num_tracks=mango_rt->register_buffer(TRACKER_NUM_TRACKS,sizeof(int),{KTRACKER},{KGUI});

  //KGUI
  auto b_gui_classes=mango_rt->register_buffer(GUI_CLASSES_PATH,(classesPath.size()+1)*sizeof(char),{HOST},{KGUI});
  //KGUI
  auto b_gui_out=mango_rt->register_buffer(OUTPUT,sizeof(sys_performance_t),{KGUI},{HOST});

	//register events
	auto mot_cla_evnt=mango_rt->register_event({KMOTION,KCLASSIFIER},{KCLASSIFIER,KMOTION});
	auto cla_tra_evnt=mango_rt->register_event({KCLASSIFIER,KTRACKER},{KTRACKER,KCLASSIFIER});
	auto tra_gui_evnt=mango_rt->register_event({KTRACKER,KGUI},{KGUI,KTRACKER});



  //create task graph and allocate resources
  auto taskGraph= new mango::TaskGraph({k_motion,k_classifier,k_tracker,k_gui},{b_motion_video_in,b_motion_mat_out,b_motion_rect_out,b_motion_num_rect,
  b_classifier_model_in,b_classifier_config_in,b_classifier_mat_out,b_classifier_detection_out, b_classifier_num_det,b_tracker_mat_out,b_tracker_info_out, b_tracker_num_tracks,b_gui_classes, b_gui_out},
	{mot_cla_evnt,cla_tra_evnt,tra_gui_evnt});



  mango_rt->resource_allocation(*taskGraph);






  auto arg_m_video_in=new mango::BufferArg(b_motion_video_in);
  auto arg_m_mat_out=new mango::BufferArg(b_motion_mat_out);
  auto arg_m_rect_out=new mango::BufferArg(b_motion_rect_out);
  auto arg_m_num_rect=new mango::BufferArg(b_motion_num_rect);

  auto arg_c_model_in=new mango::BufferArg(b_classifier_model_in);
  auto arg_c_config_in=new mango::BufferArg(b_classifier_config_in);
  auto arg_c_mat_out=new mango::BufferArg(b_classifier_mat_out);
  auto arg_c_detection_out=new mango::BufferArg(b_classifier_detection_out);
  auto arg_c_num_det=new mango::BufferArg(b_classifier_num_det);

  auto arg_t_mat_out=new mango::BufferArg(b_tracker_mat_out);
  auto arg_t_info_out=new mango::BufferArg(b_tracker_info_out);
  auto arg_t_num_tracks=new mango::BufferArg(b_tracker_num_tracks);

  auto arg_g_classes=new mango::BufferArg(b_gui_classes);
  auto arg_g_out=new mango::BufferArg(b_gui_out);

  auto arg_m_video_in_evnt=new mango::EventArg(b_motion_video_in->get_event());
  auto arg_m_mat_out_evnt=new mango::EventArg(b_motion_mat_out->get_event());
  auto arg_m_rect_out_evnt=new mango::EventArg(b_motion_rect_out->get_event());
  auto arg_m_num_rect_evnt=new mango::EventArg(b_motion_num_rect->get_event());

  auto arg_c_model_in_evnt=new mango::EventArg(b_classifier_model_in->get_event());
  auto arg_c_config_in_evnt=new mango::EventArg(b_classifier_config_in->get_event());
  auto arg_c_mat_out_evnt=new mango::EventArg(b_classifier_mat_out->get_event());
  auto arg_c_detection_out_evnt=new mango::EventArg(b_classifier_detection_out->get_event());
  auto arg_c_num_det_evnt=new mango::EventArg(b_classifier_num_det->get_event());

  auto arg_t_mat_out_evnt=new mango::EventArg(b_tracker_mat_out->get_event());
  auto arg_t_info_out_evnt=new mango::EventArg(b_tracker_info_out->get_event());
  auto arg_t_num_tracks_evnt=new mango::EventArg(b_tracker_num_tracks->get_event());
  auto arg_g_classes_evnt=new mango::EventArg(b_gui_classes->get_event());

  auto arg_mot_cla_evnt=new mango::EventArg(mot_cla_evnt);
  auto arg_cla_tra_evnt=new mango::EventArg(cla_tra_evnt);
  auto arg_tra_gui_evnt=new mango::EventArg(tra_gui_evnt);



  auto arg_minIOU=new mango::ScalarArg<int>(minIOUTracker);
  auto arg_movThr=new mango::ScalarArg<int>(minIOUMotion);

  b_motion_video_in->write(videoPath.c_str());
  b_classifier_model_in->write(modelPath.c_str());
  b_classifier_config_in->write(configPath.c_str());
  b_gui_classes->write(classesPath.c_str());
  argsKMOTION = new mango::KernelArguments({arg_m_video_in,arg_m_mat_out,arg_m_rect_out,arg_m_num_rect,arg_m_video_in_evnt,arg_m_mat_out_evnt,arg_m_rect_out_evnt, arg_m_num_rect_evnt,arg_mot_cla_evnt},k_motion);
  argsKCLASSIFIER = new mango::KernelArguments({arg_m_mat_out,arg_m_rect_out,arg_m_num_rect,arg_c_model_in,arg_c_config_in,arg_movThr,arg_c_detection_out,arg_c_num_det,arg_c_mat_out,arg_m_mat_out_evnt,arg_m_rect_out_evnt,arg_c_model_in_evnt,arg_c_config_in_evnt,arg_c_detection_out_evnt,arg_c_mat_out_evnt,arg_m_num_rect_evnt,arg_c_num_det_evnt,arg_mot_cla_evnt,arg_cla_tra_evnt},k_classifier);
  argsKTRACKER = new mango::KernelArguments({arg_c_mat_out,arg_c_detection_out,arg_c_num_det,arg_t_mat_out,arg_t_info_out,arg_t_num_tracks,arg_minIOU,arg_c_mat_out_evnt,arg_c_detection_out_evnt,arg_c_num_det_evnt,arg_t_mat_out_evnt,arg_t_info_out_evnt,arg_t_num_tracks_evnt,arg_cla_tra_evnt,arg_tra_gui_evnt},k_tracker);
  argsKGUI = new mango::KernelArguments({arg_g_classes,arg_t_mat_out,arg_t_num_tracks,arg_t_info_out,arg_g_classes_evnt,arg_t_mat_out_evnt,arg_t_num_tracks_evnt,arg_t_info_out_evnt,arg_tra_gui_evnt,arg_g_out},k_gui);

}

void KernelRunner::run_kernel()
{
  auto k_motion  = mango_rt->get_kernel(KMOTION);
  auto k_classifier   = mango_rt->get_kernel(KCLASSIFIER);
  auto k_tracker = mango_rt->get_kernel(KTRACKER);
  auto k_gui = mango_rt->get_kernel(KGUI);

  auto frame_buffer = mango_rt->get_buffer(OUTPUT);
  sys_performance_t  result;
  auto e1=mango_rt->start_kernel(k_motion, *argsKMOTION);
  auto e2=mango_rt->start_kernel(k_classifier, *argsKCLASSIFIER);
  auto e3=mango_rt->start_kernel(k_tracker, *argsKTRACKER);
  auto e4=mango_rt->start_kernel(k_gui,*argsKGUI);

   e1->wait();
   e2->wait();
   e3->wait();
   e4->wait();


   frame_buffer->read(&result);

   	mango::mango_log->Debug("Frame processed:\t%d, average FPS:\t%f", result.processed_frames,result.avg_fps);
}

KernelRunner::~KernelRunner()
{
    // Deallocation and teardown
    mango_rt->resource_deallocation(*tg);
}
