#include <opencv2/opencv.hpp>
#include "smart_surveillance_types.h"
#include "Object.h"

#include "dev/mango_hn.h"

extern "C"
#pragma mango_kernel
void gui_kernel(char * file_path,void * input_mat,int * num_info, void * object_info, mango_event_t fp_event,mango_event_t im_event, mango_event_t ni_event, mango_event_t oi_event,mango_event_t prev_continue_event, void * sys_performance)
{
  std::cout << "KGUI STARTING" << '\n';

  mango_wait(&fp_event,WRITE);
  std::ifstream ifs(file_path);
	std::string line;
	std::vector<std::string> classes;
	while (std::getline(ifs, line))
	{
		classes.push_back(line);
	}

  cv::Scalar colors[20];

  cv::RNG rng;
  for(int i=0; i<20 ;i++)
  {
      colors[i]=cv::Scalar((int)rng.uniform(0,256),(int)rng.uniform(0,256),(int)rng.uniform(0,256));
  }

  cv::namedWindow("Out", cv::WINDOW_NORMAL);



  sys_performance_t p;
  p.processed_frames=0;
  p.avg_fps=0;
  double start_time=(double) cv::getTickCount();
  double ticks=start_time;


  bool user_continue=true;

  while(user_continue )
  {
    std::vector<gui_result_t> tracks;
    mango_write_synchronization(&im_event,READY);
    std::cout << "KGUI READY WAITING FRAME" << '\n';
    mango_wait(&im_event,FRAME_SENT);

    if(mango_read_synchronization(&prev_continue_event)==STOP_EXECUTION)
    {
      std::cout << "KGUI END" << '\n';
      p.avg_fps=p.processed_frames/(((double) cv::getTickCount() - start_time) / cv::getTickFrequency());
      memcpy(sys_performance,&p,sizeof(sys_performance_t));
      return;
    }
    cv::Mat frame=(cv::Mat(416,416,CV_8UC3,(uchar *)input_mat)).clone();
    double prec_ticks=ticks;
    ticks = (double) cv::getTickCount();
    std::cout << "KGUI FRAME RECEIVED" << '\n';
    mango_write_synchronization(&im_event,FRAME_RECEIVED);
    mango_wait(&ni_event,ARRAY_LENGHT);
    std::cout << "KGUI WAITING\t"<<*num_info<<"\tresults" << '\n';
    for(int i=0;i<*num_info;i++)
    {
      mango_wait(&oi_event,OBJ_SENT);
      tracks.push_back(*(static_cast<gui_result_t *> (object_info)));
      mango_write_synchronization(&oi_event,OBJ_RECEIVED);
    }


    for(auto t : tracks)
    {
      std::string label=classes[t.obj.getClassId()]+" ID: "+std::to_string(t.id);
      cv::Rect boundingBox=t.obj.getBoundingBox();
      cv::Point center=t.obj.getCentroid();


      cv::Scalar color=colors[t.id%20];

      cv::circle(frame,center,10,color,cv::FILLED,cv::LINE_8 );
      cv::rectangle(frame,boundingBox,color,5);
      cv::putText(frame,label,cv::Point(boundingBox.x,boundingBox.y+15),cv::FONT_HERSHEY_SIMPLEX,1,cv::Scalar(0,0,255));

    }


    p.processed_frames++;


    int fps=1000*((ticks-prec_ticks)/cv::getTickFrequency());
    cv::putText(frame,"Latency: "+std::to_string(fps)+"ms",cv::Point(0,30),cv::FONT_HERSHEY_SIMPLEX,1,cv::Scalar(0,0,255));

    cv::imshow("Out",frame);
    if(cv::waitKey(1) == 27)
    {
      user_continue=false;
    }

  }

  cv::destroyAllWindows();

  p.avg_fps=p.processed_frames/(((double) cv::getTickCount() - start_time) / cv::getTickFrequency());
  memcpy(sys_performance,&p,sizeof(sys_performance_t));
  mango_write_synchronization(&prev_continue_event,STOP_EXECUTION);
  mango_write_synchronization(&im_event,READY);

  std::cout << "KGUI TERMINATED BY USER REQUEST" << '\n';

}
