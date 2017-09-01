/*
By downloading, copying, installing or using the software you agree to this
license. If you do not agree to this license, do not download, install,
copy or use the software.
                          License Agreement
               For Open Source Computer Vision Library
                       (3-clause BSD License)
Copyright (C) 2013, OpenCV Foundation, all rights reserved.
Third party copyrights are property of their respective owners.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * Neither the names of the copyright holders nor the names of the contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.
This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are
disclaimed. In no event shall copyright holders or contributors be liable for
any direct, indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.

This file was part of GSoC Project: Facemark API for OpenCV
Final report: https://gist.github.com/kurnianggoro/74de9121e122ad0bd825176751d47ecc
Student: Laksono Kurnianggoro
Mentor: Delia Passalacqua
*/

/*Usage:
 download the opencv_extra from https://github.com/opencv/opencv_extra
 and then execute the following commands:
 export OPENCV_TEST_DATA_PATH=/home/opencv/opencv_extra/testdata
 <build_folder>/bin/opencv_test_face
*/

#include "test_precomp.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/face.hpp"
#include <vector>
#include <string>
using namespace std;
using namespace cv;
using namespace cv::face;

CascadeClassifier cascade_detector;
bool myCustomDetector( InputArray image, OutputArray ROIs, void * config = 0 ){
    Mat gray;
    std::vector<Rect> & faces = *(std::vector<Rect>*) ROIs.getObj();
    faces.clear();

    if(config!=0){
        //do nothing
    }

    if(image.channels()>1){
        cvtColor(image.getMat(),gray,CV_BGR2GRAY);
    }else{
        gray = image.getMat().clone();
    }
    equalizeHist( gray, gray );

    cascade_detector.detectMultiScale( gray, faces, 1.4, 2, CV_HAAR_SCALE_IMAGE, Size(30, 30) );
    return true;
}

TEST(CV_Face_FacemarkLBF, can_create_default) {
    FacemarkLBF::Params params;
    params.n_landmarks = 68;

    Ptr<Facemark> facemark;
    EXPECT_NO_THROW(facemark = FacemarkLBF::create(params));
    EXPECT_FALSE(facemark.empty());
}

TEST(CV_Face_FacemarkLBF, can_set_custom_detector) {
    string cascade_filename =
        cvtest::findDataFile("face/lbpcascade_frontalface.xml", true);

    FacemarkLBF::Params params;
    params.cascade_face = cascade_filename;

    EXPECT_TRUE(cascade_detector.load(cascade_filename));

    Ptr<Facemark> facemark = FacemarkLBF::create(params);
    EXPECT_TRUE(facemark->setFaceDetector(myCustomDetector));
}

TEST(CV_Face_FacemarkLBF, can_perform_training) {

    string i1 = cvtest::findDataFile("face/therock.jpg", true);
    string p1 = cvtest::findDataFile("face/therock.pts", true);
    string i2 = cvtest::findDataFile("face/mark.jpg", true);
    string p2 = cvtest::findDataFile("face/mark.pts", true);

    std::vector<string> images_train;
    images_train.push_back(i1);
    images_train.push_back(i2);

    std::vector<String> points_train;
    points_train.push_back(p1);
    points_train.push_back(p2);

    string cascade_filename =
        cvtest::findDataFile("face/lbpcascade_frontalface.xml", true);
    FacemarkLBF::Params params;
    params.cascade_face = cascade_filename;
    params.verbose = false;
    Ptr<Facemark> facemark = FacemarkLBF::create(params);

    Mat image;
    std::vector<Point2f> landmarks;
    for(size_t i=0;i<images_train.size();i++){
        image = imread(images_train[i].c_str());
        EXPECT_TRUE(loadFacePoints(points_train[i].c_str(),landmarks));
        EXPECT_TRUE(landmarks.size()>0);
        EXPECT_TRUE(facemark->addTrainingSample(image, landmarks));
    }

    EXPECT_NO_THROW(facemark->training());
}

TEST(CV_Face_FacemarkLBF, can_load_model) {
    string model_filename = "LBF.model";

    Ptr<Facemark> facemark = FacemarkLBF::create();
    EXPECT_NO_THROW(facemark->loadModel(model_filename.c_str()));
}

TEST(CV_Face_FacemarkLBF, can_detect_landmarks) {
    string model_filename = "LBF.model";
    string cascade_filename =
        cvtest::findDataFile("face/lbpcascade_frontalface.xml", true);

    FacemarkLBF::Params params;
    params.model_filename = model_filename;
    params.cascade_face = cascade_filename;

    cascade_detector.load(cascade_filename);

    Ptr<Facemark> facemark = FacemarkLBF::create(params);
    facemark->setFaceDetector(myCustomDetector);
    facemark->loadModel(params.model_filename.c_str());

    string image_filename = cvtest::findDataFile("face/therock.jpg", true);
    Mat image = imread(image_filename.c_str());
    EXPECT_TRUE(!image.empty());

    std::vector<Rect> rects;
    std::vector<std::vector<Point2f> > landmarks;

    EXPECT_TRUE(facemark->getFaces(image, rects));
    EXPECT_TRUE(rects.size()>0);
    EXPECT_TRUE(facemark->fit(image, rects, landmarks));
    EXPECT_TRUE(landmarks[0].size()>0);
}

TEST(CV_Face_FacemarkLBF, get_data) {
    Ptr<Facemark> facemark = FacemarkLBF::create();
    EXPECT_TRUE(facemark->getData());
}
