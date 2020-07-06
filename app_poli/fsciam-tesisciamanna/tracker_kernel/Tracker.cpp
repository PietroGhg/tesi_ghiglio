#include "Tracker.h"



int Tracker::trackCount=0;

Tracker::Tracker(Object _obj):
obj(_obj)
{
  trackCount++;
  id=trackCount;
  framesDropped=0;

  int stateSize=4; //[x,y,x',y']
	int measSize=2; //[mx,my]
	int contrSize=0;

  kf=cv::KalmanFilter(stateSize,measSize,contrSize,CV_32F);
	cv::setIdentity(kf.transitionMatrix);
  /*
  [1,0,dT,0]
  [0,1,0,dT]

  dT will be set during prediction
  */
  kf.measurementMatrix = cv::Mat::zeros(measSize, stateSize, CV_32F);
  kf.measurementMatrix.at<float>(0) = 1.0f;
  kf.measurementMatrix.at<float>(5) = 1.0f;
  //process noise is set low because movement is relatively slow and smooth
  cv::setIdentity(kf.processNoiseCov, cv::Scalar::all(1e-5));
   // set measurement noise
  cv::setIdentity(kf.measurementNoiseCov, cv::Scalar::all(1e-4));
  // error covariance
  cv::setIdentity(kf.errorCovPost, cv::Scalar::all(1));

  //set initial state(center coordinates and velocity)
  cv::Mat state(stateSize, 1, CV_32F);
  cv::Point center=obj.getCentroid();
  state.at<float>(0) = center.x;
  state.at<float>(1) = center.y;
  state.at<float>(2) = 0;
  state.at<float>(3) = 0;
  kf.statePre=state;
  kf.statePost = state;

}
Object Tracker::getObj()
{
  return obj;
}
int Tracker::getId()
{
  return id;
}

double Tracker::computeCost(Object o)
{
  return 1-obj.computeIOU(o);
}

void Tracker::update(Object  o)
{
  obj=o;
  cv::Point newCentroid=obj.getCentroid();
  cv::Mat meas(2,1,CV_32F);
  meas.at<float>(0) = newCentroid.x;
  meas.at<float>(1) = newCentroid.y;
	kf.correct(meas);
  framesDropped=0;
}
bool Tracker::update()
{
  framesDropped++;
  return framesDropped<10;
}

void Tracker::predict(double dT)
{

    kf.transitionMatrix.at<float>(2) = dT;
    kf.transitionMatrix.at<float>(7) = dT;
		cv::Mat state = kf.predict();
    obj.updateCentroid(cv::Point(state.at<float>(0), state.at<float>(1)));

}
