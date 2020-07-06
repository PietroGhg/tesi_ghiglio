#include "Object.h"

Object::Object(int _class_id,cv::Rect _boundingBox):
class_id(_class_id), boundingBox(_boundingBox)
 {
   centroid=cv::Point(boundingBox.x+(boundingBox.width/2),boundingBox.y+(boundingBox.height/2));
 }
Object::Object()
{
  class_id=0;
  boundingBox=cv::Rect();
  centroid=cv::Point();
}

cv::Rect Object::getBoundingBox()
{
  return boundingBox;
}
cv::Point Object::getCentroid()
{
  return centroid;
}

int Object::getClassId()
{
  return class_id;
}


double Object::computeIOU(cv::Rect r)
{
  double inter=(boundingBox & r).area();
  double uni=(boundingBox | r).area();
  return inter/uni;
}
double Object::computeIOU(Object d)
{
  return computeIOU(d.getBoundingBox());
}

void Object::updateCentroid(cv::Point newCentroid)
{
  cv::Point delta=centroid-newCentroid;
  centroid=newCentroid;
  boundingBox.x-=delta.x;
  boundingBox.y-=delta.y;
}
