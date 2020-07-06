
#include <iostream>
#include <sys/stat.h>

#include <opencv2/opencv.hpp>


#include "Object.h"
#include "kernel_runner.h"

const char* params
    = "{ help h           |       | Print usage }"
      "{ video            |   0   | Path to input video}"
      "{ model            |       | DNN weights }"
      "{ config           |       | DNN configuration file }"
      "{ classes          |       | Path to file containing classes names list}"
      "{ minIOUMotion     |   10  | Int value between 0 and 100 that represents the minimum IOU needed to consider an object identified by the dnn as moving.}"
      "{ minIOUTracker    |   70  | Int value between 0 and 100 that represents the minimum IOU needed to consider an object as new (not currently tracked) in the current scene.}";

inline bool exists_file (const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

int main(int argc, char ** argv)
{

  cv::CommandLineParser parser(argc, argv, params);

  //Read params from parser
  std::string videoPath=parser.get<std::string>("video");
  std::string modelPath=parser.get<std::string>("model");
  std::string configPath=parser.get<std::string>("config");
  std::string classesPath=parser.get<std::string>("classes");
  int minIOUTracker=parser.get<int>("minIOUTracker");
  int minIOUMotion=parser.get<int>("minIOUMotion");

  if (!parser.check())
  {
      parser.printErrors();
      return 0;
  }

  if(minIOUTracker<0 && minIOUTracker>100)
  {
      std::cout << "ERROR:\tminIOU value should be between 0 and 100" << std::endl;
      return 0;
  }
  if(minIOUMotion<0 && minIOUMotion>100)
  {
    std::cout << "ERROR:\tminMotion value should be between 0 and 100" << std::endl;
    return 0;
  }

  if(!exists_file(videoPath))
  {
    std::cout << "ERROR:\t"<<videoPath<<" not found" << std::endl;
    return 0;
  }
  if(!exists_file(modelPath))
  {
    std::cout << "ERROR:\t"<<modelPath<<" not found" << std::endl;
    return 0;
  }
  if(!exists_file(configPath))
  {
    std::cout << "ERROR:\t"<<configPath<<" not found" << std::endl;
    return 0;
  }
  if(!exists_file(classesPath))
  {
    std::cout << "ERROR:\t"<<classesPath<<" not found" << std::endl;
    return 0;
  }

  KernelRunner kr(minIOUTracker,minIOUMotion,videoPath,modelPath,configPath,classesPath);

  kr.run_kernel();
  return 0;
}
