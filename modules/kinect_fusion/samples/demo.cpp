// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html

// This code is also subject to the license terms in the LICENSE file found in this module's directory

#include <iostream>
#include <fstream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/kinect_fusion.hpp>

#ifdef HAVE_OPENCV_VIZ

#include <opencv2/viz.hpp>

using namespace cv;
using namespace std;

static vector<string> readDepth(std::string fileList);

static vector<string> readDepth(std::string fileList)
{
    vector<string> v;

    fstream file(fileList);
    if(!file.is_open())
        throw std::runtime_error("Failed to open file");

    std::string dir;
    size_t slashIdx = fileList.rfind('/');
    slashIdx = slashIdx != std::string::npos ? slashIdx : fileList.rfind('\\');
    dir = fileList.substr(0, slashIdx);

    while(!file.eof())
    {
        std::string s, imgPath;
        std::getline(file, s);
        if(s.empty() || s[0] == '#') continue;
        std::stringstream ss;
        ss << s;
        double thumb;
        ss >> thumb >> imgPath;
        v.push_back(dir+'/'+imgPath);
    }

    return v;
}


static const char* keys =
{
    "{help h usage ? | | print this message   }"
    "{@depth |<none>| Path to depth.txt file listing a set of depth images }"
    "{coarse | | Run on coarse settings (fast but ugly) or on default (slow but looks better),"
        " in coarse mode points and normals are displayed }"
};

static const std::string message =
 "\nThis demo uses RGB-D dataset taken from"
 "\nhttps://vision.in.tum.de/data/datasets/rgbd-dataset"
 "\nto demonstrate KinectFusion implementation \n";


int main(int argc, char **argv)
{
    bool coarse = false;

    CommandLineParser parser(argc, argv, keys);
    parser.about(message);
    if(parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }
    if(parser.has("coarse"))
    {
        coarse = true;
    }

    String depthPath = parser.get<String>(0);

    if(!parser.check())
    {
        parser.printMessage();
        parser.printErrors();
        return -1;
    }

    vector<string> depthFileList = readDepth(depthPath);

    kinfu::KinFu::KinFuParams params;
    if(coarse)
        params = kinfu::KinFu::KinFuParams::coarseParams();
    else
        params = kinfu::KinFu::KinFuParams::defaultParams();

    kinfu::KinFu kf(params);

    cv::viz::Viz3d window("cloud");
    window.setViewerPose(Affine3f::Identity());

    // TODO: can we use UMats for that?
    Mat rendered;
    Mat points;
    Mat normals;

    double prevTime = getTickCount();

    for(size_t nFrame = 0; nFrame < depthFileList.size(); nFrame++)
    {
        Mat frame = cv::imread(depthFileList[nFrame], IMREAD_ANYDEPTH);
        if(frame.empty())
            throw std::runtime_error("Matrix is empty");

        Mat cvt8;
        convertScaleAbs(frame, cvt8, 0.25f/5000.f*256.f);
        imshow("depth", cvt8);

        if(!kf(frame))
            std::cout << "reset" << std::endl;
        else
        {
            if(coarse)
            {
                kf.fetchCloud(points, normals);
                viz::WCloud cloudWidget(points, viz::Color::white());
                viz::WCloudNormals cloudNormals(points, normals, /*level*/1, /*scale*/0.05, viz::Color::gray());
                window.showWidget("cloud", cloudWidget);
                window.showWidget("normals", cloudNormals);
            }

            //window.showWidget("worldAxes", viz::WCoordinateSystem());
            window.showWidget("cube", viz::WCube(Vec3d::all(0),
                                                 Vec3d::all(kf.getParams().volumeSize)),
                              kf.getParams().volumePose);
            window.setViewerPose(kf.getPose());
            window.spinOnce(1, true);
        }

        kf.render(rendered);

        double newTime = getTickCount();
        putText(rendered, cv::format("FPS: %2d press R to reset, Q to quit", (int)(getTickFrequency()/(newTime - prevTime))),
                Point(0, rendered.rows-1), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 255, 255));
        prevTime = newTime;

        imshow("render", rendered);

        int c = waitKey(1);
        if(c == 'r')
            kf.reset();
        if(c == 'q')
            break;
    }

    return 0;
}

#else

int main(int /* argc */, char ** /* argv */)
{
    std::cout << "This demo requires viz module" << std::endl;
    return 0;
}
#endif
