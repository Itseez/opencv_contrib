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
// Copyright (C) 2015, University of Ostrava, Institute for Research and Applications of Fuzzy Modeling,
// Pavel Vlasanek, all rights reserved. Third party copyrights are property of their respective owners.
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

#include "precomp.hpp"

#include <iostream>
#include <numeric>

namespace cv{

namespace img_hash{

namespace{

inline
float roundingFactor(float val)
{
    return val >= 0 ? 0.5f : -0.5f;
}

inline
int createOffSet(int length)
{
    float const center = static_cast<float>(length/2);
    return static_cast<int>(std::floor(center +
                                       roundingFactor(center)));
}

}

RadialVarianceHash::RadialVarianceHash(double sigma,
                                       float gamma,
                                       int numOfAngleLine) :
    gamma_(gamma),
    numOfAngelLine_(numOfAngleLine),
    sigma_(sigma)
{
}

RadialVarianceHash::~RadialVarianceHash()
{

}

void RadialVarianceHash::compute(cv::Mat const &input, cv::Mat &hash)
{
    CV_Assert(input.type() == CV_8UC3 ||
              input.type() == CV_8U);

    if(input.type() == CV_8UC3)
    {
        cv::cvtColor(input, grayImg_, CV_BGR2GRAY);
    }
    else{
        grayImg_ = input;
    }

    cv::GaussianBlur(grayImg_, blurImg_, cv::Size(0,0), sigma_, sigma_);
    //same as : gammaImg = grayImg / 255.0;
    cv::addWeighted(grayImg_, 1/255.0, cv::Mat(), 0,
                    0, gammaImg_, CV_32F);

    float const invGamma = 1.0f/(gamma_ + 0.0001f);
    cv::pow(gammaImg_, invGamma, gammaImg_);
    gammaImg_ *= 255;
    gammaImg_.convertTo(normalizeImg_, CV_8U);

    radialProjections(normalizeImg_);
    findFeatureVector();
    hashCalculate(hash);
}

double RadialVarianceHash::compare(cv::Mat const &hashOne, cv::Mat const &hashTwo) const
{
    float buffer[1];
    cv::Mat result(1,1,CV_32F,buffer);
    cv::matchTemplate(hashOne, hashTwo, result, CV_TM_CCORR_NORMED);

    return buffer[0];
}

Ptr<RadialVarianceHash> RadialVarianceHash::create()
{
    return makePtr<RadialVarianceHash>();
}

void RadialVarianceHash::
afterHalfProjections(const Mat &input, int D, int xOff, int yOff)
{
    int *pplPtr = pixPerLine_.ptr<int>(0);
    int const init = 3*numOfAngelLine_/4;
    for(int k = init, j = 0; k < numOfAngelLine_; ++k, j += 2)
    {
        float const theta = k*3.14159f/numOfAngelLine_;
        float const alpha = std::tan(theta);
        for(int x = 0; x < D; ++x)
        {
            float const y = alpha*(x-xOff);
            int const yd = static_cast<int>(std::floor(y + roundingFactor(y)));
            if((yd + yOff >= 0)&&(yd + yOff < input.rows) && (x < input.cols))
            {
                projections_.at<uchar>(x, k) = input.at<uchar>(yd+yOff, x);
                pplPtr[k] += 1;
            }
            if ((yOff - yd >= 0)&&(yOff - yd < input.cols)&&
                    (2*yOff - x >= 0)&&(2*yOff- x < input.rows)&&
                    (k != init))
            {
                projections_.at<uchar>(x,k-j) =
                        input.at<uchar>(-(x-yOff)+yOff, -yd+yOff);
                pplPtr[k-j] += 1;
            }
        }
    }
}

void RadialVarianceHash::findFeatureVector()
{
    features_.resize(numOfAngelLine_);
    double sum = 0.0;
    double sumSqd = 0.0;
    int const *pplPtr = pixPerLine_.ptr<int>(0);
    for(int k=0; k < numOfAngelLine_; ++k)
    {
        double lineSum = 0.0;
        double lineSumSqd = 0.0;
        //original implementation of pHash may generate zero pixNum, this
        //will cause NaN value and make the features become less discriminative
        //to avoid this problem, I add a small value--0.00001
        double const pixNum = pplPtr[k] + 0.00001;
        double const pixNumPow2 = pixNum * pixNum;
        for(int i = 0; i < projections_.rows; ++i)
        {
            double const value = projections_.at<uchar>(i,k);
            lineSum += value;
            lineSumSqd += value * value;
        }
        features_[k] = (lineSumSqd/pixNum) -
                (lineSum*lineSum)/(pixNumPow2);
        sum += features_[k];
        sumSqd += features_[k]*features_[k];
    }
    double const numOfALPow2 = numOfAngelLine_ * numOfAngelLine_;
    double const mean = sum/numOfAngelLine_;
    double const var  = std::sqrt((sumSqd/numOfAngelLine_) - (sum*sum)/(numOfALPow2));
    for(int i = 0; i < numOfAngelLine_; ++i)
    {
        features_[i] = (features_[i] - mean)/var;
    }
}

void RadialVarianceHash::
firstHalfProjections(Mat const &input, int D, int xOff, int yOff)
{
    int *pplPtr = pixPerLine_.ptr<int>(0);
    for(int k = 0; k < numOfAngelLine_/4+1; ++k)
    {
        float const theta = k*3.14159f/numOfAngelLine_;
        float const alpha = std::tan(theta);
        for(int x = 0; x < D; ++x)
        {
            float const y = alpha*(x-xOff);
            int const yd = static_cast<int>(std::floor(y + roundingFactor(y)));
            if((yd + yOff >= 0)&&(yd + yOff < input.rows) && (x < input.cols))
            {
                projections_.at<uchar>(x, k) = input.at<uchar>(yd+yOff, x);
                pplPtr[k] += 1;
            }
            if((yd + xOff >= 0) && (yd + xOff < input.cols) &&
                    (k != numOfAngelLine_/4) && (x < input.rows))
            {
                projections_.at<uchar>(x, numOfAngelLine_/2-k) =
                        input.at<uchar>(x, yd+xOff);
                pplPtr[numOfAngelLine_/2-k] += 1;
            }
        }
    }
}

void RadialVarianceHash::hashCalculate(cv::Mat &hash)
{
    hash.create(1, 40, CV_8U);
    double temp[40];
    double max = 0;
    double min = 0;
    size_t const featureSize = features_.size();
    //constexpr is a better choice
    double const sqrtTwo = 1.4142135623730950488016887242097;
    for(int k = 0; k < hash.cols; ++k)
    {
        double sum = 0;
        for(size_t n = 0; n < featureSize; ++n)
        {
            sum += features_[n]*std::cos((3.14159*(2*n+1)*k)/(2*featureSize));
        }
        temp[k] = k == 0 ? sum/std::sqrt(featureSize) :
                           sum*sqrtTwo/std::sqrt(featureSize);
        if(temp[k] > max)
        {
            max = temp[k];
        }
        else if(temp[k] < min)
        {
            min = temp[k];
        }
    }

    double const range = max - min;
    if(range != 0)
    {
        //std::transform is a better choice if lambda supported
        uchar *hashPtr = hash.ptr<uchar>(0);
        for(int i = 0; i < hash.cols; ++i)
        {
            hashPtr[i] = static_cast<uchar>((255*(temp[i] - min)/range));
        }
    }
    else
    {
        hash = 0;
    }
}

void RadialVarianceHash::radialProjections(const Mat &input)
{
    int const D = (input.cols > input.rows) ? input.cols : input.rows;
    projections_.create(D, numOfAngelLine_, CV_8U);
    projections_ = 0;
    pixPerLine_.create(1, numOfAngelLine_, CV_32S);
    pixPerLine_ = 0;
    int const xOff = createOffSet(input.cols);
    int const yOff = createOffSet(input.rows);

    firstHalfProjections(input, D, xOff, yOff);
    afterHalfProjections(input, D, xOff, yOff);
}

void radialVarianceHash(cv::Mat const &input, cv::Mat &hash)
{
    RadialVarianceHash().compute(input, hash);
}

}

}
