#include <glog/logging.h>

#include <iostream>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <vitis/ai/facedetect.hpp>
#include <vitis/ai/facefeature.hpp>
#include <chrono>

using namespace std;
using namespace cv;

bool perf = 0;

std::string class_array [] = { 
  "AngelinaJolie",
  "BradPitt",
  "DenzelWashington",
  "HughJackman",
  "JenniferLawrence",
  "JohnnyDepp",
  "KateWinslet",
  "LeonardoDiCaprio",
  "MeganFox",
  "NataliePortman",
  "NicoleKidman",
  "RobertDowneyJr",
  "SandraBullock",
  "ScarlettJohansson",
  "TomCruise",
  "TomHanks",
  "WillSmith"
}; 

int main(int argc, char* argv[]) {

  float conf_th = atof(argv[2]);
  string input_file = argv[1];
  auto image = imread(input_file);
  auto network_fd = vitis::ai::FaceDetect::create("densebox_640_360", true);
  auto network_fe = vitis::ai::FaceFeature::create("InceptionResnetV1", true);
  auto out_file_name = string{argv[1]};
  auto out_file = "result_" + out_file_name;
  int resize_w = 160;
  int resize_h = 160;
  auto tot_start = std::chrono::system_clock::now();
  auto start = std::chrono::system_clock::now();
  auto result_fd = network_fd->run(image);
  if (perf) {
    auto end = std::chrono::system_clock::now();
    std::cout << "FD (ms)" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
  }


  for (const auto &r : result_fd.rects) {
    auto score = r.score;
    auto x = r.x * image.cols;
    auto y = r.y * image.rows;
    auto width = r.width * image.cols;
    auto height = r.height * image.rows;
    Mat cropped_image = image(Range(y, y + height), Range(x, x + width));
    Mat resized_image;
    resize(cropped_image, resized_image, Size(resize_w, resize_h), INTER_LINEAR);
    if (perf) {
      auto start = std::chrono::system_clock::now();
    }
    auto result_fe = network_fe->run(resized_image);
    if (perf) {
      auto end = std::chrono::system_clock::now();
      std::cout << "FE (ms)" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
    }
    float max_val = -1.0;
    int max_idx = 0;
    int idx = 0;
    for (const float f : *result_fe.feature) {
      if (f > max_val && f > conf_th) {
	max_val = f;	
	max_idx = idx;
      }
      idx++;
    }
    if (!perf) {
      if (max_val == -1.0) {
	cout << "class: " << "UNIDENTIFIED" << ", score: " << max_val << std::endl;  
	rectangle(image, Point(x, y), Point(x + width, y + height), Scalar(0, 0, 128), 3);
	putText(image, "UNIDENTIFIED", Point(x, y), 5, 1, Scalar(0, 0, 255), 1, 8);
      } else {
	cout << "class: " << class_array[max_idx] << ", score: " << max_val << std::endl;  
	rectangle(image, Point(x, y), Point(x + width, y + height), Scalar(0, 255, 0), 3);
	putText(image, class_array[max_idx] + "(" + to_string(int(max_val)) + ")", Point(x - 70, y), 2, 1.0, Scalar(0, 255, 255), 2, 8);
      }
    }
  }
  if (perf) {
    auto tot_end = std::chrono::system_clock::now();
    std::cout << "TOTAL FR (ms)" << std::chrono::duration_cast<std::chrono::milliseconds>(tot_end - tot_start).count() << "ms" << std::endl;
  }
  if (!perf)
    cout << "writing results" << std::endl;
  imwrite(out_file, image);

  return 0;
}
