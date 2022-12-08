#include <glog/logging.h>

#include <iostream>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <vitis/ai/facedetect.hpp>
#include <vitis/ai/facefeature.hpp>

using namespace std;
using namespace cv;

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
  string input_file = argv[1];
  float conf_th = atof(argv[2]);
  VideoCapture vin(input_file);
  int frame_width  = vin.get(CAP_PROP_FRAME_WIDTH);
  int frame_height = vin.get(CAP_PROP_FRAME_HEIGHT);
  auto network_fd = vitis::ai::FaceDetect::create("densebox_640_360", true);
  auto network_fe = vitis::ai::FaceFeature::create("InceptionResnetV1", true);
  auto out_file_name = string{argv[1]};
  auto out_file = "result_" + out_file_name;
  int resize_w = 160;
  int resize_h = 160;
  VideoWriter vout(out_file, VideoWriter::fourcc('a','v','c','1'), 25, Size(frame_width,frame_height));

  if (!vin.isOpened()) {
    printf("No movie file");
    return -1;
  }

  Mat image;
  while (1)
  {
    vin >> image;
    if (image.empty()) {
      printf("empty image");
      return 0;
    }

    auto result_fd = network_fd->run(image);
    for (const auto &r : result_fd.rects) {
      float score = r.score;
      int x = r.x * image.cols;
      int y = r.y * image.rows;
      int width = r.width * image.cols;
      int height = r.height * image.rows;
      if (x + width >= frame_width or x <0)
	continue;
      if (y + height >= frame_height or y <0)
	continue;
      Mat cropped_image = image(Range(y, y + height), Range(x, x + width));
      Mat resized_image;
      resize(cropped_image, resized_image, Size(resize_w, resize_h), INTER_LINEAR);

      auto result_fe = network_fe->run(resized_image);
      int max_val = -1;
      int max_idx = 0;
      int idx = 0;
      for (const auto f : *result_fe.feature) {
	if (f > max_val && f > conf_th) {
	  max_val = f;	
	  max_idx = idx;
	}
	idx++;
      }
      if (max_val == -1) {
	rectangle(image, Point(x, y), Point(x + width, y + height), Scalar(0, 0, 128), 3);
	putText(image, "UNIDENTIFIED", Point(x, y), 5, 1, Scalar(0, 0, 255), 1, 8);
      } else {
	cout << "class: " << class_array[max_idx] << ", score: " << max_val << std::endl;  
	rectangle(image, Point(x, y), Point(x + width, y + height), Scalar(0, 255, 0), 3);
	putText(image, class_array[max_idx] + "(" + to_string(int(max_val)) + ")", Point(x - 70, y), 2, 1.0, Scalar(0, 255, 255), 2, 8);
      }
    }

    putText(image, "Vitis-AI2.5, VCK5000", Point(frame_width - 400, 50), 4, 0.8, Scalar(255, 255, 255), 2, 8);
    vout.write(image);

    char c = (char)waitKey(25);
    if(c == 27)
      break;
  }

  vin.release();
  vout.release();
  return 0;
}
