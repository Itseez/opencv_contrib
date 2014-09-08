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
// Copyright (C) 2014, Itseez Inc, all rights reserved.
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
// In no event shall the Itseez Inc or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "opencv2/datasetstools/msm_epfl.hpp"
#include "precomp.hpp"

namespace cv
{
namespace datasetstools
{

using namespace std;

void MSM_epfl::readFileDouble(const string &fileName, vector<double> &out)
{
    ifstream infile(fileName.c_str());
    double val;
    while (infile >> val)
    {
        out.push_back(val);
    }
}

MSM_epfl::MSM_epfl(const string &path)
{
    loadDataset(path);
}

void MSM_epfl::load(const string &path, int number)
{
    if (number!=0)
    {
        return;
    }

    loadDataset(path);
}

void MSM_epfl::loadDataset(const string &path)
{
    string pathBounding(path + "bounding/");
    string pathCamera(path + "camera/");
    string pathP(path + "P/");
    string pathPng(path + "png/");

    vector<string> fileNames;
    getDirList(pathPng, fileNames);
    for (vector<string>::iterator it=fileNames.begin(); it!=fileNames.end(); ++it)
    {
        Ptr<MSM_epflObj> curr(new MSM_epflObj);
        curr->imageName = *it;

        readFileDouble(string(pathBounding + curr->imageName + ".bounding"), curr->bounding);
        readFileDouble(string(pathCamera + curr->imageName + ".camera"), curr->camera);
        readFileDouble(string(pathP + curr->imageName + ".P"), curr->p);

        train.push_back(curr);
    }
}

}
}
