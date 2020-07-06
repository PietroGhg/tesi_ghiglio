
#include "HungarianAlgorithm.h"
#include "Object.h"
#include "dev/mango_hn.h"
#include "smart_surveillance_types.h"


extern "C"
#pragma mango_kernel
void tracker_kernel(void * input_mat, void * detection, int * num_det, void * output_mat, void * tracker_info, int * track_num, int threshold,mango_event_t im_event,mango_event_t d_event,mango_event_t nd_event,mango_event_t om_event,mango_event_t ti_event,mango_event_t tn_event,mango_event_t prev_continue_event, mango_event_t next_continue_event)
{

  std::vector<Tracker> trackedObjects;
  float minIOU =threshold/100.0;
  std::cout << "KTRACKER START\t"<<minIOU<< '\n';
  double ticks=0;
  bool next_stopped=false;
  while(!next_stopped)
  {
    std::vector<Object> inputObject;
    mango_write_synchronization(&im_event,READY);
    std::cout << "KTRACKER READY" << '\n';
    mango_wait(&im_event,FRAME_SENT);
    if(mango_read_synchronization(&prev_continue_event)==STOP_EXECUTION)
    {
      std::cout << "KTRACKER END CORRETTO" << '\n';
        mango_write_synchronization(&next_continue_event,STOP_EXECUTION);
        std::cout << "KTRACKER WAITING KGUI" << '\n';
          mango_wait(&om_event,READY);
          std::cout << "KTRACKER KGUI READY" << '\n';
      mango_write_synchronization(&om_event,FRAME_SENT);
      return;
    }
    cv::Mat frame=(cv::Mat(416,416,CV_8UC3,(uchar *)input_mat)).clone();
    std::cout << "KTRACKER read image"<< '\n';
    mango_write_synchronization(&im_event,FRAME_RECEIVED);
    mango_wait(&nd_event,ARRAY_LENGHT);
    for(int i=0;i<*num_det;i++)
    {
      std::cout << "KTRACKER wait obj\t"<<i<<"\tout of\t"<<*num_det<<'\n';
      mango_wait(&d_event,OBJ_SENT);
      Object * received=static_cast <Object *>(detection);
      inputObject.push_back(*received);
      std::cout << "KTRACKER received obj\t"<<i<< '\n';
      mango_write_synchronization(&d_event,OBJ_RECEIVED);
    }

    if(trackedObjects.empty())
    {
      for(auto obj : inputObject)
      {
        trackedObjects.push_back(Tracker(obj));
      }

    }
    else
    {
      if(!inputObject.empty())
      {
        double precTick = ticks;
        ticks = (double) cv::getTickCount();
        double dT = (ticks - precTick) / cv::getTickFrequency();

        for(int i=0;i<trackedObjects.size();i++)
        {
          trackedObjects[i].predict(dT);
        }

        Result result=makeBoundingBoxesPair(trackedObjects, inputObject,minIOU);

        for(auto pair: result.getPairs())
        {

          trackedObjects[pair.first].update(inputObject[pair.second]);
        }

        for(auto i: result.getUnPairedObjects())
        {
          trackedObjects.push_back(Tracker(inputObject[i]));
        }
       for(auto i: result.getUnPairedTracks())
        {
          if(!trackedObjects[i].update())
            trackedObjects.erase(trackedObjects.begin()+i);
        }
      }
    }
    mango_wait(&om_event,READY);
    if(mango_read_synchronization(&next_continue_event) != STOP_EXECUTION)
    {
      memcpy(output_mat,frame.data,3*416*416);
      mango_write_synchronization(&om_event,FRAME_SENT);
      std::cout << "KTRACKER FRAME SENT" << '\n';
      mango_wait(&om_event,FRAME_RECEIVED);
      *track_num=trackedObjects.size();
      mango_write_synchronization(&tn_event,ARRAY_LENGHT);
      for(int i=0; i<*track_num; i++)
      {
        gui_result_t p;
        p.id=trackedObjects[i].getId();
        p.obj=trackedObjects[i].getObj();
        memcpy(tracker_info,&p,sizeof(gui_result_t));
        mango_write_synchronization(&ti_event,OBJ_SENT);
        mango_wait(&ti_event,OBJ_RECEIVED);
      }
    }
    else
    {
      std::cout << "KTRACKER ABOUT TO END" << '\n';
      next_stopped=true;
    }

  }


  mango_write_synchronization(&prev_continue_event,STOP_EXECUTION);
  mango_write_synchronization(&im_event,READY);
  std::cout << "KTRACKER TERMINATED" << '\n';
}
