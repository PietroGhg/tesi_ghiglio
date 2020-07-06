
#include <opencv2/dnn.hpp>
#include "Object.h"
#include "dev/mango_hn.h"
#include <iostream>
#include "smart_surveillance_types.h"


static std::vector<Object> checkForMovement(std::vector<cv::Rect> movingRects,std::vector<Object> detectedObj,float threshold)
{

  std::vector<Object> movingObj;
  for(int i=0;i<detectedObj.size();i++)
  {
    for(int j=0;j<movingRects.size();j++)
    {
      if(detectedObj[i].computeIOU(movingRects[j])>threshold)
      {
        movingObj.push_back(detectedObj[i]);
        break;
      }
    }
  }
  return movingObj;

}
void classifierKernelTensorFlow(void * input_mat,void * rect_in, int * num_rect,std::string model,std::string config,float threshold,void * detection,int * num_det,void * output_mat,mango_event_t im_event,mango_event_t ri_event,mango_event_t nr_event,mango_event_t d_event,mango_event_t om_event,mango_event_t nd_event,mango_event_t prev_continue_event, mango_event_t next_continue_event)
{
  cv::dnn::Net net=cv::dnn::readNetFromTensorflow(model,config);
  std::cout << "KCLASSIFIER load Model"<< '\n';
  bool next_stopped=false;
  while(!next_stopped)
  {
    std::vector<cv::Rect> rectangles;
    mango_wait(&im_event,FRAME_SENT);
    if(mango_read_synchronization(&prev_continue_event)==STOP_EXECUTION)
    {
      std::cout << "KCLASSIFIER END" << '\n';
      mango_write_synchronization(&next_continue_event,STOP_EXECUTION);
      std::cout << "KCLASSIFIER WAITING KTRACKER" << '\n';
        mango_wait(&om_event,READY);
        std::cout << "KCLASSIFIER KTRACKER READY" << '\n';
      mango_write_synchronization(&om_event,FRAME_SENT);


      return;

    }
    cv::Mat frame=(cv::Mat(416,416,CV_8UC3,(uchar *)input_mat)).clone();
    mango_write_synchronization(&im_event,FRAME_RECEIVED);
    mango_wait(&nr_event,ARRAY_LENGHT);

    for(int i=0;i<*num_rect;i++)
    {
      mango_wait(&ri_event,OBJ_SENT);
      rectangles.push_back(*(static_cast<cv::Rect *> (rect_in)));
      mango_write_synchronization(&ri_event,OBJ_RECEIVED);
    }

    std::vector<Object> detectedObjs;
    cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 1.0,cv::Size(300,300),cv::Scalar(127.5,127.5,127.5),true,false);
    net.setInput(blob);
    cv::Mat prob = net.forward();
    cv::Mat detectionMat(prob.size[2], prob.size[3], CV_32F, prob.ptr<float>());
    int width=frame.size().width, height=frame.size().height;
    for(int i=0;i<100;i++)
    {
      const  float* Mi = detectionMat.ptr<float>(i);

      if(Mi[2]>0.6)
      {
        int x=Mi[3]*width,y=Mi[4]*height;

        detectedObjs.push_back(Object(Mi[1],cv::Rect(x,y,Mi[5]*300,Mi[6]*300)));

      }
    }
    std::vector<Object> movingObjs=checkForMovement(rectangles,detectedObjs,threshold);
    mango_wait(&om_event,READY);
    if(mango_read_synchronization(&next_continue_event) != STOP_EXECUTION)
    {
      memcpy(output_mat,frame.data,3*416*416);
      std::cout << "KCLASSIFIER SEND IMAGE"<< '\n';
      mango_write_synchronization(&om_event,FRAME_SENT);
      mango_wait(&om_event,FRAME_RECEIVED);
      *num_det=movingObjs.size();
      std::cout << "KCLASSIFIER ABOUT TO SEND\t"<<*num_det<<"\tdetection"<< '\n';
      mango_write_synchronization(&nd_event,ARRAY_LENGHT);
      for(int i=0;i<*num_det;i++)
      {
        memcpy(detection,&movingObjs[i],sizeof(Object));
        std::cout << "KCLASSIFIER object sent\t"<<i<<"\tout of\t"<<*num_det<< '\n';
        mango_write_synchronization(&d_event,OBJ_SENT);
        mango_wait(&d_event,OBJ_RECEIVED);
      }
    }
    else
    {
      next_stopped=true;
    }


  }
  std::cout << "KCLASSIFIER ABOUT TO END\t"<<'\n';
  mango_write_synchronization(&prev_continue_event,STOP_EXECUTION);
  std::cout << "KCLASSIFIER TEST\t"<<mango_read_synchronization(&prev_continue_event)<< '\n';
  mango_write_synchronization(&prev_continue_event,STOP_EXECUTION);
  std::cout << "KCLASSIFIER TEST\t"<<mango_read_synchronization(&prev_continue_event)<< '\n';
  mango_write_synchronization(&prev_continue_event,STOP_EXECUTION);
  mango_write_synchronization(&im_event,READY);
}
void classifierKernelDarkNet(void * input_mat,void * rect_in, int * num_rect,std::string model,std::string config,float threshold,void * detection,int * num_det,void * output_mat,mango_event_t im_event,mango_event_t ri_event,mango_event_t nr_event,mango_event_t d_event,mango_event_t om_event,mango_event_t nd_event,mango_event_t prev_continue_event, mango_event_t next_continue_event)
{

  cv::dnn::Net net=cv::dnn::readNetFromDarknet(config,model);
  std::cout << "KCLASSIFIER load Model"<< '\n';
  std::vector<std::string> layers_names=net.getLayerNames();
	std::vector<std::string> layer;
  bool next_stopped=false;
	for(auto s:net.getUnconnectedOutLayers())
	{
		layer.push_back(layers_names[s-1]);
	}

  while(!next_stopped)
  {
    std::vector<cv::Rect> rectangles;
    mango_write_synchronization(&im_event,READY);
    std::cout << "KCLASSIFIER READY" << '\n';
    mango_wait(&im_event,FRAME_SENT);
    std::cout << "KCLASSIFIER IMAGE RECEIVED" << '\n';
    if(mango_read_synchronization(&prev_continue_event)==STOP_EXECUTION)
    {
      std::cout << "KCLASSIFIER END" << '\n';
      mango_write_synchronization(&next_continue_event,STOP_EXECUTION);
      std::cout << "KCLASSIFIER WAITING KTRACKER" << '\n';
        mango_wait(&om_event,READY);
        std::cout << "KCLASSIFIER KTRACKER READY" << '\n';
      mango_write_synchronization(&om_event,FRAME_SENT);


      return;

    }
    std::cout << "KCLASSIFIER SAVING IMAGE" << '\n';
    cv::Mat frame=(cv::Mat(416,416,CV_8UC3,(uchar *)input_mat)).clone();
    mango_write_synchronization(&im_event,FRAME_RECEIVED);
    mango_wait(&nr_event,ARRAY_LENGHT);
    for(int i=0;i<*num_rect;i++)
    {
      mango_wait(&ri_event,OBJ_SENT);
      std::cout << "KCLASSIFIER obj pointer\t"<<rect_in<<"\t"<<i<< '\n';
      rectangles.push_back(*(static_cast<cv::Rect *> (rect_in)));
      mango_write_synchronization(&ri_event,OBJ_RECEIVED);
    }

    std::vector<Object> detectedObjs;
    cv::Mat blob;

    cv::dnn::blobFromImage(frame, blob, 1.0/255, cv::Size(416,416),cv::Scalar(0,0,0),true,false);
    net.setInput(blob);
    std::vector<cv::Mat> prob;
    net.forward(prob,layer);

    std::vector<cv::Rect> boxes;
    std::vector<float> confidences;
    std::vector<int> ids;
    for(auto output:prob)
    {

      float* data = (float*)output.data;
      for (int j = 0; j < output.rows; ++j, data += output.cols)
      {
          cv::Mat scores = output.row(j).colRange(5, output.cols);
          cv::Point classIdPoint;
          double confidence;
          cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
          if (confidence > 0.6)
          {
              int centerX = (int)(data[0] * frame.cols);
              int centerY = (int)(data[1] * frame.rows);
              int width = (int)(data[2] * frame.cols);
              int height = (int)(data[3] * frame.rows);
              int left = centerX - width / 2;
              int top = centerY - height / 2;

              ids.push_back(classIdPoint.x);
              confidences.push_back((float)confidence);
              boxes.push_back(cv::Rect(left, top, width, height));
          }
      }
    }


    std::vector<int> boxToDraw;


    cv::dnn::NMSBoxes(boxes,confidences,0.5,0.4,boxToDraw);

    for(auto i:boxToDraw)
    {
      detectedObjs.push_back(Object(ids[i],boxes[i]));
    }

    std::vector<Object> movingObjs=checkForMovement(rectangles,detectedObjs,threshold);
    mango_wait(&om_event,READY);
    std::cout << "KCLASSIFIER KTRACKER READY"<< '\n';
    if(mango_read_synchronization(&next_continue_event) != STOP_EXECUTION)
    {
      memcpy(output_mat,frame.data,3*416*416);
      std::cout << "KCLASSIFIER SEND IMAGE"<< '\n';
      mango_write_synchronization(&om_event,FRAME_SENT);
      mango_wait(&om_event,FRAME_RECEIVED);
      *num_det=movingObjs.size();
      std::cout << "KCLASSIFIER ABOUT TO SEND\t"<<*num_det<<"\tdetection"<< '\n';
      mango_write_synchronization(&nd_event,ARRAY_LENGHT);
      for(int i=0;i<*num_det;i++)
      {
        memcpy(detection,&movingObjs[i],sizeof(Object));
        std::cout << "KCLASSIFIER object sent\t"<<i<<"\tout of\t"<<*num_det<< '\n';
        mango_write_synchronization(&d_event,OBJ_SENT);
        mango_wait(&d_event,OBJ_RECEIVED);
      }
    }
    else
    {

      next_stopped=true;

    }
  }
  std::cout << "KCLASSIFIER ABOUT TO END\t"<<'\n';
  mango_write_synchronization(&prev_continue_event,STOP_EXECUTION);
  std::cout << "KCLASSIFIER TEST\t"<<mango_read_synchronization(&prev_continue_event)<< '\n';
  mango_write_synchronization(&prev_continue_event,STOP_EXECUTION);
  std::cout << "KCLASSIFIER TEST\t"<<mango_read_synchronization(&prev_continue_event)<< '\n';
  mango_write_synchronization(&prev_continue_event,STOP_EXECUTION);
  mango_write_synchronization(&im_event,READY);

}
extern "C"
#pragma mango_kernel
void classifierKernel(void * input_mat,  void * rect_in,int * num_rect,char * model_path, char * config_path, int threshold, void * detection, int * num_det,void * output_mat,mango_event_t im_event,mango_event_t ri_event,mango_event_t mp_event,mango_event_t cfp_event,mango_event_t d_event,mango_event_t om_event,mango_event_t nr_event,mango_event_t nd_event,mango_event_t prev_continue_event, mango_event_t next_continue_event)
{


  float minMotion=threshold/100.0;
    std::cout << "KCLASSIFIER START\t"<<minMotion<< '\n';
  mango_wait(&mp_event,WRITE);
  mango_wait(&cfp_event,WRITE);




  std::string model(model_path);
  std::string config(config_path);
  std::string model_ext = model.substr(model.rfind('.') + 1);
  if(model_ext == "pb")
  {
    classifierKernelTensorFlow(input_mat,rect_in,num_rect,model,config,minMotion,detection,num_det,output_mat,im_event,ri_event,nr_event,d_event,om_event,nd_event, prev_continue_event, next_continue_event);
  }
  else if(model_ext == "weights")
  {
    classifierKernelDarkNet(input_mat,rect_in,num_rect,model,config,minMotion,detection,num_det,output_mat,im_event,ri_event,nr_event,d_event,om_event,nd_event, prev_continue_event, next_continue_event);
  }



}
