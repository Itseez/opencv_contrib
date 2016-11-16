/*IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.

 By downloading, copying, installing or using the software you agree to this license.
 If you do not agree to this license, do not download, install,
 copy or use the software.


 License Agreement
 For Open Source Computer Vision Library

 Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
 Copyright (C) 2008-2010, Willow Garage Inc., all rights reserved.
 Third party copyrights are property of their respective owners.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 * Redistribution's of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 * Redistribution's in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 * The name of the copyright holders may not be used to endorse or promote products
 derived from this software without specific prior written permission.

 This software is provided by the copyright holders and contributors "as is" and
 any express or implied warranties, including, but not limited to, the implied
 warranties of merchantability and fitness for a particular purpose are disclaimed.
 In no event shall the Intel Corporation or contributors be liable for any direct,
 indirect, incidental, special, exemplary, or consequential damages
 (including, but not limited to, procurement of substitute goods or services;
 loss of use, data, or profits; or business interruption) however caused
 and on any theory of liability, whether in contract, strict liability,
 or tort (including negligence or otherwise) arising in any way out of
 the use of this software, even if advised of the possibility of such damage.*/

#include "precomp.hpp"

namespace cv
{
namespace xfeatures2d
{

/*
* Functions to perform affine adaptation of circular keypoint
*/
void calcAffineCovariantRegions(const Mat& image, const std::vector<KeyPoint>& keypoints, std::vector<Elliptic_KeyPoint>& affRegions);
void calcAffineCovariantDescriptors( const Ptr<DescriptorExtractor>& dextractor, const Mat& img, std::vector<Elliptic_KeyPoint>& affRegions, Mat& descriptors );

/*
 *  HarrisLaplaceFeatureDetector
 */
HarrisLaplaceFeatureDetector::Params::Params(int _numOctaves, float _corn_thresh, float _DOG_thresh, int _maxCorners, int _num_layers) :
    numOctaves(_numOctaves), corn_thresh(_corn_thresh), DOG_thresh(_DOG_thresh), maxCorners(_maxCorners), num_layers(_num_layers)
{}
HarrisLaplaceFeatureDetector::HarrisLaplaceFeatureDetector( int numOctaves, float corn_thresh, float DOG_thresh, int maxCorners, int num_layers)
  : harris( numOctaves, corn_thresh, DOG_thresh, maxCorners, num_layers)
{}

HarrisLaplaceFeatureDetector::HarrisLaplaceFeatureDetector(  const Params& params  )
 : harris( params.numOctaves, params.corn_thresh, params.DOG_thresh, params.maxCorners, params.num_layers)

{}

void HarrisLaplaceFeatureDetector::read (const FileNode& fn)
{
    int numOctaves = fn["numOctaves"];
    float corn_thresh = fn["corn_thresh"];
    float DOG_thresh = fn["DOG_thresh"];
    int maxCorners = fn["maxCorners"];
    int num_layers = fn["num_layers"];

    harris = HarrisLaplace( numOctaves, corn_thresh, DOG_thresh, maxCorners,num_layers );
}

void HarrisLaplaceFeatureDetector::write (FileStorage& fs) const
{
    fs << "numOctaves" << harris.numOctaves;
    fs << "corn_thresh" << harris.corn_thresh;
    fs << "DOG_thresh" << harris.DOG_thresh;
    fs << "maxCorners" << harris.maxCorners;
    fs << "num_layers" << harris.num_layers;
}


void HarrisLaplaceFeatureDetector::detectImpl( const Mat& image, std::vector<KeyPoint>& keypoints, const Mat& mask ) const
{
    harris.detect(image, keypoints);
}

/*
 *  HarrisAffineFeatureDetector
 */
HarrisAffineFeatureDetector::Params::Params(int _numOctaves, float _corn_thresh, float _DOG_thresh, int _maxCorners, int _num_layers) :
    numOctaves(_numOctaves), corn_thresh(_corn_thresh), DOG_thresh(_DOG_thresh), maxCorners(_maxCorners), num_layers(_num_layers)
{}
HarrisAffineFeatureDetector::HarrisAffineFeatureDetector( int numOctaves, float corn_thresh, float DOG_thresh, int maxCorners, int num_layers)
  : harris( numOctaves, corn_thresh, DOG_thresh, maxCorners, num_layers)
{}

HarrisAffineFeatureDetector::HarrisAffineFeatureDetector(  const Params& params  )
 : harris( params.numOctaves, params.corn_thresh, params.DOG_thresh, params.maxCorners, params.num_layers)

{}

void HarrisAffineFeatureDetector::read (const FileNode& fn)
{
    int numOctaves = fn["numOctaves"];
    float corn_thresh = fn["corn_thresh"];
    float DOG_thresh = fn["DOG_thresh"];
    int maxCorners = fn["maxCorners"];
    int num_layers = fn["num_layers"];

    harris = HarrisLaplace( numOctaves, corn_thresh, DOG_thresh, maxCorners,num_layers );
}

void HarrisAffineFeatureDetector::write (FileStorage& fs) const
{
    fs << "numOctaves" << harris.numOctaves;
    fs << "corn_thresh" << harris.corn_thresh;
    fs << "DOG_thresh" << harris.DOG_thresh;
    fs << "maxCorners" << harris.maxCorners;
    fs << "num_layers" << harris.num_layers;
}
void HarrisAffineFeatureDetector::detect( const Mat& image, std::vector<Elliptic_KeyPoint>& ekeypoints, const Mat& mask ) const
{
    std::vector<KeyPoint> keypoints;
    harris.detect(image, keypoints);
    Mat fimage;
    image.convertTo(fimage, CV_32F, 1.f/255);
    calcAffineCovariantRegions(fimage, keypoints, ekeypoints);
}

void HarrisAffineFeatureDetector::detectImpl( const Mat& image, std::vector<KeyPoint>& ekeypoints, const Mat& mask ) const
{
}

}
}
