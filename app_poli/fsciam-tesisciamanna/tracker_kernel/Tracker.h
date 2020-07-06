#ifndef TRACK_H
#define TRACK_H

#include <opencv2/opencv.hpp>
#include <opencv2/video/tracking.hpp>

#include "../Object.h"


/**
* Class that represents object tracker
*/
class Tracker
{

  public:
    /**
    * Constructor
    */
    Tracker(Object _obj);
    /**
    * Obj getter
    * @return obj tracked
    */
    Object getObj();
    /**
    * Obj getter
    * @return id of the object
    */
    int getId();
    /**
    * Return the cost of "pair" obj and o
    * @param o object
    * @return cost of the assignment
    */
    double computeCost(Object o);
    /**
    * Update tracker based on the new detection
    * @param o new detection
    */
    void update(Object  o);
    /**
    * Update tracker after no new detection
    * @return true if the track is still valid
    */
    bool update();
    /**
    * Predict position using a Kalman filter
    * @param dT delta time
    */
    void predict(double dT);


  private:

    static int trackCount;
    int id;
    Object obj;
    int framesDropped;
    cv::KalmanFilter kf;



};
#endif
