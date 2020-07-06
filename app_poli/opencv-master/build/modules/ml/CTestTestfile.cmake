# CMake generated Testfile for 
# Source directory: /home/pietro/Documents/Polimi/Quinto anno/tesi/app_poli/opencv-master/modules/ml
# Build directory: /home/pietro/Documents/Polimi/Quinto anno/tesi/app_poli/opencv-master/build/modules/ml
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(opencv_test_ml "/home/pietro/Documents/Polimi/Quinto anno/tesi/app_poli/opencv-master/build/bin/opencv_test_ml" "--gtest_output=xml:opencv_test_ml.xml")
set_tests_properties(opencv_test_ml PROPERTIES  LABELS "Main;opencv_ml;Accuracy" WORKING_DIRECTORY "/home/pietro/Documents/Polimi/Quinto anno/tesi/app_poli/opencv-master/build/test-reports/accuracy" _BACKTRACE_TRIPLES "/home/pietro/Documents/Polimi/Quinto anno/tesi/app_poli/opencv-master/cmake/OpenCVUtils.cmake;1640;add_test;/home/pietro/Documents/Polimi/Quinto anno/tesi/app_poli/opencv-master/cmake/OpenCVModule.cmake;1310;ocv_add_test_from_target;/home/pietro/Documents/Polimi/Quinto anno/tesi/app_poli/opencv-master/cmake/OpenCVModule.cmake;1074;ocv_add_accuracy_tests;/home/pietro/Documents/Polimi/Quinto anno/tesi/app_poli/opencv-master/modules/ml/CMakeLists.txt;2;ocv_define_module;/home/pietro/Documents/Polimi/Quinto anno/tesi/app_poli/opencv-master/modules/ml/CMakeLists.txt;0;")
