# Smart surveillance

## Requirements
To be able to install and use the program, you have to have on your machine:

* [OpenCv (make sure to build with modules from opencv_contrib)](https://opencv.org/)
* [Barbeque and Libmango](https://bitbucket.org/mango_developers/mangolibs/src/master/)

## How to build the project
1. Clone this repository
2. Open the cloned folder on terminal
3. Make a new folder to keep separated source files and build files and open it
4. Launch "cmake <path-to-repo-root>" command
5. Build with "sudo make install"
## Usage
### Basic usage
Once launched Barbeque daemon navigate to the folder containing build files and launch the program as follows:
`./smart_surveillance -model=absolute-path-to-frozen-dnn-weights -config=absolute-path-to-dnn-config-file -classes=absolute-path-to-classes-list -video=absolute-path-to-video-file`
### Advanced usage
Tracker is tuned to have acceptable performance in standard scenarios. If not satisfied with current performance you can modify some parameters:

         -minIOUMotion:             Int value between 0 and 100 that represents the minimum IOU needed to consider an object identified by the DNNs as moving.

         -minIOUTracker:             Int value between 0 and 100 that represents the minimum IOU needed to consider an object as new (not currently tracked) in the current scene.


## DNN
At the moment the program supports the following DNNs(for best tracking performance use YOLO):

* [YOLO](https://pjreddie.com/darknet/yolo/)
* [MobileNet](https://github.com/opencv/opencv/wiki/TensorFlow-Object-Detection-API#use-existing-config-file-for-your-model)
## Video datasets
Here's some video datasets to test the program:

* Walking people
    * [Oxford town centre](http://www.robots.ox.ac.uk/ActiveVision/Research/Projects/2009bbenfold_headpose/project.html)
