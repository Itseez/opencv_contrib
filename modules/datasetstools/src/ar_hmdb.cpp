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

#include "opencv2/datasetstools/ar_hmdb.hpp"
#include "precomp.hpp"

namespace cv
{
namespace datasetstools
{

using namespace std;

void AR_hmdb::loadAction(const string &fileName, vector<string> &train_, vector<string> &test_)
{
    ifstream infile(fileName.c_str());
    string video, label;
    while (infile >> video >> label)
    {
        if ("1"==label)
        {
            train_.push_back(video);
        } else
        if ("2"==label)
        {
            test_.push_back(video);
        }
    }
}

AR_hmdb::AR_hmdb(const string &path, int number)
{
    loadDataset(path, number);
}

void AR_hmdb::load(const string &path, int number)
{
    loadDataset(path, number);
}

void AR_hmdb::loadDataset(const string &path, int number)
{
    // valid number [0,1,2]
    if (number<0 || number>2)
    {
        return;
    }

    string pathDataset(path + "hmdb51_org/");
    string pathSplit(path + "testTrainMulti_7030_splits/");

    vector<string> fileNames;
    getDirList(pathDataset, fileNames);
    for (vector<string>::iterator it=fileNames.begin(); it!=fileNames.end(); ++it)
    {
        Ptr<AR_hmdbObj> currTrain(new AR_hmdbObj);
        Ptr<AR_hmdbObj> currTest(new AR_hmdbObj);
        currTrain->name = *it;
        currTest->name = *it;

        train.push_back(currTrain);
        test.push_back(currTest);

        char tmp[2];
        sprintf(tmp, "%u", number+1);
        string fileName(pathSplit + currTrain->name + "_test_split" + tmp + ".txt");
        loadAction(fileName, currTrain->videoNames, currTest->videoNames);
    }
}

}
}
