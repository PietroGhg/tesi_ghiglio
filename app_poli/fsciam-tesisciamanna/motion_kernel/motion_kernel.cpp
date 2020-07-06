#include "dev/mango_hn.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/bgsegm.hpp>
#include "smart_surveillance_types.h"

extern "C"
#pragma mango_kernel
void motion_kernel(char * video_path ,void * output_mat, void * rect, int * num_rect, mango_event_t vp_event, mango_event_t om_event , mango_event_t r_event,mango_event_t nr_event, mango_event_t next_continue_event )
{
  mango_wait(&vp_event, WRITE);
  std::cout << "KMOTION START\t opening video:\t"<< video_path << '\n';
  cv::VideoCapture video_in(video_path);

  if(!video_in.isOpened())
	{
		std::cout<<"Impossible to open video: "<<video_path<<"\n";
    std::cout << "KMOTION VIDEO ENDED" << '\n';
    mango_write_synchronization(&next_continue_event,STOP_EXECUTION);

    mango_wait(&om_event,READY);
    mango_write_synchronization(&om_event,FRAME_SENT);
  	return;
	}
  cv::Mat frame,frame_resized,frame_smoothed,mask;


  cv::Ptr<cv::BackgroundSubtractor> p_back_sub;

  p_back_sub=cv::createBackgroundSubtractorMOG2(450,250,false);
  bool next_stopped=false;

  while( !next_stopped )
  {

    if(!video_in.read(frame))
    {
      std::cout << "KMOTION VIDEO ENDED" << '\n';
      mango_write_synchronization(&next_continue_event,STOP_EXECUTION);

      mango_wait(&om_event,READY);
      mango_write_synchronization(&om_event,FRAME_SENT);

      return;
    }
    cv::resize(frame, frame_resized, cv::Size(416,416));
    cv::bilateralFilter(frame_resized,frame_smoothed,5,150,150,cv::BORDER_DEFAULT);


		//update the background model with a choosen learning rate
		p_back_sub->apply(frame_smoothed, mask);
		cv::GaussianBlur(mask,mask,cv::Size(9,9),0);
		cv::threshold(mask,mask, 128, 255, cv::THRESH_BINARY);




		//make the lines in the frame bigger
		//erode(mask,mask,Mat());
		cv::dilate(mask,mask,cv::Mat(), cv::Point(-1,-1), 20);
		cv::erode(mask,mask,cv::Mat(), cv::Point(-1,-1), 20);

		//Find the contours of the moving objects
		std::vector<std::vector<cv::Point> > contours;
		std::vector<cv::Vec4i> hierarchy;

		cv::findContours(mask, contours,hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE );
		//build rectangles around moving objects
		std::vector<std::vector<cv::Point> > contours_poly(contours.size());

    std::cout << "KMOTION SEND FRAME" << '\n';
    mango_wait(&om_event,READY);
    uint32_t state= mango_read_synchronization(&next_continue_event);
    std::cout << "KMOTION READY\t"<< state<< '\n';
    if( state != STOP_EXECUTION)
    {
      memcpy(output_mat,frame_resized.data,3*416*416);
      mango_write_synchronization(&om_event,FRAME_SENT);
      mango_wait(&om_event,FRAME_RECEIVED);

      *num_rect=contours.size();
      std::cout << "KMOTION ABOUT TO SEND "<<*num_rect<<" rectangles"<< '\n';
      mango_write_synchronization(&nr_event,ARRAY_LENGHT);

  		for( size_t i = 0; i < contours.size(); i++ )
  		{
          std::cout << "KMOTION rectangles sent\t"<<i+1<<"\tout of\t"<<*num_rect<< '\n';
  				cv::approxPolyDP( contours[i], contours_poly[i], 7, true );
          cv::Rect  bb=boundingRect( contours_poly[i] );
  				memcpy(rect,&bb,sizeof(cv::Rect));
          std::cout << "KMOTION obj pointer\t"<<rect<<"\t"<<i<< '\n';
          mango_write_synchronization(&r_event,OBJ_SENT);
          mango_wait(&r_event,OBJ_RECEIVED);
  		}
    }
    else
    {
      next_stopped=true;
      std::cout << "KMOTION IS TERMINATING" << '\n';
    }

  }


}
