#ifndef OBJECT_H
#define OBJECT_H

#include <opencv2/opencv.hpp>

/**
* Class that represents objects identified by classifier
*/
class Object
{
  public:
    /**
    * Constructor
    * @param _class_id class name
    * @param _boundingBox rectangle that surrounds the object
    *
    */
    Object(int _class_id,cv::Rect _boundingBox);
    Object();
    /**
    * boundingBox getter
    * @return boundingBox
    */
    cv::Rect getBoundingBox();

    /**
    * boundingBox getter
    * @return centroid
    */
    cv::Point getCentroid();
    /**
    * class_id getter
    * @return class_id
    */
    int getClassId();

  
    /**
    * Compute IOU(Intersection Over Union) between boundingBox and r
    * @param r rectangle
    * @return iou
    */
    double computeIOU(cv::Rect r);
    /**
    * Compute IOU(Intersection Over Union) between boundingBox and r
    * @param d other detection
    * @return iou
    */
    double computeIOU(Object d);

    /**
    * Update center of the bounding rectangle
    * @param newCentroid
    */
    void updateCentroid(cv::Point newCentroid);



  private:
    int class_id;
    cv::Point centroid;
    cv::Rect boundingBox;

};
#endif
