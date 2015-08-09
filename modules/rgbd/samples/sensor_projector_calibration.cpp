/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2015, Intel Corporation, all rights reserved.
// Copyright (C) 2009-2011, Willow Garage Inc., all rights reserved.
// Copyright (C) 2015, OpenCV Foundation, all rights reserved.
// Copyright (C) 2015, Itseez Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include <opencv2/rgbd.hpp>

#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
//#include <opencv2/videoio.hpp>
#include <opencv2/structured_light.hpp>
#include <opencv2/core/utility.hpp>

#include <iostream>
#include <fstream>
#include <cmath>

using namespace cv;
using namespace cv::rgbd;
using namespace cv::structured_light;
using namespace std;

int main(int argc, char** argv)
{
    VideoCapture capture(CAP_OPENNI2);
    // set registeration on
    capture.set(CAP_PROP_OPENNI_REGISTRATION, 0.0);

    if (!capture.isOpened())
    {
        cout << "Camera unavailable" << endl;
        return -1;
    }

    Mat image;

    // initialize gray coding
    GrayCodePattern::Params params;
    params.width = 1024 / 2;
    params.height = 768 / 2;
    Ptr<GrayCodePattern> pattern = GrayCodePattern::create(params);

    vector<Mat> patternImages;
    pattern->generate(patternImages, Scalar(0, 0, 0), Scalar(70, 70, 70));

    string window = "pattern";
    namedWindow(window, WINDOW_NORMAL);
    moveWindow(window, 0, 0);
    imshow(window, Mat::zeros(params.height, params.width, CV_8U));

    // window placement; wait for user
    while (true)
    {
        capture.grab();

        capture.retrieve(image, CAP_OPENNI_DEPTH_MAP);
        flip(image, image, 1);
        imshow("camera", image * 10);

        int key = waitKey(30);
        if (key == 'f')
        {
            // TODO: 1px border when fullscreen on Windows (Surface?)
            setWindowProperty(window, WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
        }
        else if (key == 'w')
        {
            setWindowProperty(window, WND_PROP_FULLSCREEN, WINDOW_NORMAL);
        }
        else if (key == ' ')
        {
            break;
        }
    }

    vector<Mat> cameraImages;
    // start structured lighting
    for (size_t i = 0; i < patternImages.size(); i++)
    {
        waitKey(50);

        imshow(window, patternImages.at(i));

        waitKey(50);

        for (int t = 0; t < 5; t++)
        {
            waitKey(50);
            capture.grab();

            capture.retrieve(image, CAP_OPENNI_BGR_IMAGE);
            flip(image, image, 1);
        }

        Mat gray;
        cvtColor(image, gray, COLOR_BGR2GRAY);
        imshow("camera", gray);
        cameraImages.push_back(gray);

        waitKey(50);
    }

    imshow(window, Mat::zeros(params.height, params.width, CV_8U));
    waitKey(30);

    // decode
    pattern->setLightThreshold(20);
    Mat correspondenceMapX = Mat(image.size(), CV_8UC3, Scalar(255, 255, 255));
    Mat correspondenceMapY = Mat(image.size(), CV_8UC3, Scalar(255, 255, 255));
    Size projectorSize(params.width * 2, params.height * 2);
    Mat correspondenceMapProX = Mat::zeros(projectorSize, CV_8UC3);
    Mat correspondenceMapProY = Mat::zeros(projectorSize, CV_8UC3);
    for (int y = 0; y < capture.get(CAP_PROP_FRAME_HEIGHT); y++) {
        for (int x = 0; x < capture.get(CAP_PROP_FRAME_WIDTH); x++) {
            Point point;

            const bool error = true;
            if (pattern->getProjPixel(cameraImages, x, y, point) == error)
            {
                continue;
            }

            if (point.x >= params.width-5 || point.y >= params.height-5 || point.x <= 5 || point.y <= 5)
            {
                continue;
            }

            point *= 2;

            Range xr(x, x + 1);
            Range yr(y, y + 1);
            // weird encoding to reliably recover the point...
//            correspondenceMapX(yr, xr) = Scalar((point.x & 0x0F00) >> 4, (point.x & 0xF0), (point.x & 0x0F) << 4);
//            correspondenceMapY(yr, xr) = Scalar((point.y & 0x0F00) >> 4, (point.y & 0xF0), (point.y & 0x0F) << 4);
            correspondenceMapX(yr, xr) = Scalar(0, (point.x & 0xFF00) / 256, (point.x & 0xFF));
            correspondenceMapY(yr, xr) = Scalar(0, (point.y & 0xFF00) / 256, (point.y & 0xFF));

            Range xp(point.x - 1, point.x + 2);
            Range yp(point.y - 1, point.y + 2);
            //correspondenceMapProX(yp, xp) = Scalar((x & 0x0F00) >> 4, (x & 0xF0), (x & 0x0F) << 4);
            //correspondenceMapProY(yp, xp) = Scalar((y & 0x0F00) >> 4, (y & 0xF0), (y & 0x0F) << 4);
        }
    }

    imshow("correspondence X", correspondenceMapX);
    imshow("correspondence Y", correspondenceMapY);

    imshow("correspondence Pro X", correspondenceMapProX);
    imshow("correspondence Pro Y", correspondenceMapProY);

    imwrite("correspondenceX.png", correspondenceMapX);
    imwrite("correspondenceY.png", correspondenceMapY);

    waitKey(0);
    return 0;
}
