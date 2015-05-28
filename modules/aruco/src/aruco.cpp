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
*/

#ifndef __OPENCV_ARUCO_CPP__
#define __OPENCV_ARUCO_CPP__
#ifdef __cplusplus

#include "precomp.hpp"
#include "opencv2/aruco.hpp"

#include <opencv2/core.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>


namespace cv{ namespace aruco{

using namespace std;


const char quartets_distances[16][16][4] =
{
    { {0,0,0,0},{1,1,1,1},{1,1,1,1},{2,2,2,2},{1,1,1,1},{2,2,2,2},{2,2,2,2},{3,3,3,3},{1,1,1,1},{2,2,2,2},{2,2,2,2},{3,3,3,3},{2,2,2,2},{3,3,3,3},{3,3,3,3},{4,4,4,4}, },
    { {1,1,1,1},{0,2,2,2},{2,0,2,2},{1,1,3,3},{2,2,0,2},{1,3,1,3},{3,1,1,3},{2,2,2,4},{2,2,2,0},{1,3,3,1},{3,1,3,1},{2,2,4,2},{3,3,1,1},{2,4,2,2},{4,2,2,2},{3,3,3,3}, },
    { {1,1,1,1},{2,2,2,0},{0,2,2,2},{1,3,3,1},{2,0,2,2},{3,1,3,1},{1,1,3,3},{2,2,4,2},{2,2,0,2},{3,3,1,1},{1,3,1,3},{2,4,2,2},{3,1,1,3},{4,2,2,2},{2,2,2,4},{3,3,3,3}, },
    { {2,2,2,2},{1,3,3,1},{1,1,3,3},{0,2,4,2},{3,1,1,3},{2,2,2,2},{2,0,2,4},{1,1,3,3},{3,3,1,1},{2,4,2,0},{2,2,2,2},{1,3,3,1},{4,2,0,2},{3,3,1,1},{3,1,1,3},{2,2,2,2}, },
    { {1,1,1,1},{2,2,0,2},{2,2,2,0},{3,3,1,1},{0,2,2,2},{1,3,1,3},{1,3,3,1},{2,4,2,2},{2,0,2,2},{3,1,1,3},{3,1,3,1},{4,2,2,2},{1,1,3,3},{2,2,2,4},{2,2,4,2},{3,3,3,3}, },
    { {2,2,2,2},{1,3,1,3},{3,1,3,1},{2,2,2,2},{1,3,1,3},{0,4,0,4},{2,2,2,2},{1,3,1,3},{3,1,3,1},{2,2,2,2},{4,0,4,0},{3,1,3,1},{2,2,2,2},{1,3,1,3},{3,1,3,1},{2,2,2,2}, },
    { {2,2,2,2},{3,3,1,1},{1,3,3,1},{2,4,2,0},{1,1,3,3},{2,2,2,2},{0,2,4,2},{1,3,3,1},{3,1,1,3},{4,2,0,2},{2,2,2,2},{3,3,1,1},{2,0,2,4},{3,1,1,3},{1,1,3,3},{2,2,2,2}, },
    { {3,3,3,3},{2,4,2,2},{2,2,4,2},{1,3,3,1},{2,2,2,4},{1,3,1,3},{1,1,3,3},{0,2,2,2},{4,2,2,2},{3,3,1,1},{3,1,3,1},{2,2,2,0},{3,1,1,3},{2,2,0,2},{2,0,2,2},{1,1,1,1}, },
    { {1,1,1,1},{2,0,2,2},{2,2,0,2},{3,1,1,3},{2,2,2,0},{3,1,3,1},{3,3,1,1},{4,2,2,2},{0,2,2,2},{1,1,3,3},{1,3,1,3},{2,2,2,4},{1,3,3,1},{2,2,4,2},{2,4,2,2},{3,3,3,3}, },
    { {2,2,2,2},{1,1,3,3},{3,1,1,3},{2,0,2,4},{3,3,1,1},{2,2,2,2},{4,2,0,2},{3,1,1,3},{1,3,3,1},{0,2,4,2},{2,2,2,2},{1,1,3,3},{2,4,2,0},{1,3,3,1},{3,3,1,1},{2,2,2,2}, },
    { {2,2,2,2},{3,1,3,1},{1,3,1,3},{2,2,2,2},{3,1,3,1},{4,0,4,0},{2,2,2,2},{3,1,3,1},{1,3,1,3},{2,2,2,2},{0,4,0,4},{1,3,1,3},{2,2,2,2},{3,1,3,1},{1,3,1,3},{2,2,2,2}, },
    { {3,3,3,3},{2,2,4,2},{2,2,2,4},{1,1,3,3},{4,2,2,2},{3,1,3,1},{3,1,1,3},{2,0,2,2},{2,4,2,2},{1,3,3,1},{1,3,1,3},{0,2,2,2},{3,3,1,1},{2,2,2,0},{2,2,0,2},{1,1,1,1}, },
    { {2,2,2,2},{3,1,1,3},{3,3,1,1},{4,2,0,2},{1,3,3,1},{2,2,2,2},{2,4,2,0},{3,3,1,1},{1,1,3,3},{2,0,2,4},{2,2,2,2},{3,1,1,3},{0,2,4,2},{1,1,3,3},{1,3,3,1},{2,2,2,2}, },
    { {3,3,3,3},{2,2,2,4},{4,2,2,2},{3,1,1,3},{2,4,2,2},{1,3,1,3},{3,3,1,1},{2,2,0,2},{2,2,4,2},{1,1,3,3},{3,1,3,1},{2,0,2,2},{1,3,3,1},{0,2,2,2},{2,2,2,0},{1,1,1,1}, },
    { {3,3,3,3},{4,2,2,2},{2,4,2,2},{3,3,1,1},{2,2,4,2},{3,1,3,1},{1,3,3,1},{2,2,2,0},{2,2,2,4},{3,1,1,3},{1,3,1,3},{2,2,0,2},{1,1,3,3},{2,0,2,2},{0,2,2,2},{1,1,1,1}, },
    { {4,4,4,4},{3,3,3,3},{3,3,3,3},{2,2,2,2},{3,3,3,3},{2,2,2,2},{2,2,2,2},{1,1,1,1},{3,3,3,3},{2,2,2,2},{2,2,2,2},{1,1,1,1},{2,2,2,2},{1,1,1,1},{1,1,1,1},{0,0,0,0}, },
};


/**
 * @brief detect marker candidates
 * 
 * @param _image input image
 * @param _candidates return candidate corners positions
 * @param _threshParam window size for adaptative thresholding
 * @param _minLenght minimum size of candidates contour lenght. It is indicated as a ratio
 *                  respect to the largest image dimension
 * @param _thresholdedImage if set, returns the thresholded image for debugging purposes.
 * @return void
 */
void _detectCandidates(InputArray _image, OutputArrayOfArrays _candidates, int _threshParam,
                           float _minLenght, OutputArray _thresholdedImage=noArray()) {

    cv::Mat image = _image.getMat();


    /// 1. CONVERT TO GRAY
    cv::Mat grey;
    if ( image.type() ==CV_8UC3 )   cv::cvtColor ( image,grey,cv::COLOR_BGR2GRAY );
    else grey=image;


    /// 2. THRESHOLD
    CV_Assert(_threshParam >= 3);
    cv::Mat thresh;
    cv::adaptiveThreshold ( grey,thresh,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY_INV,_threshParam,7 );
    if(_thresholdedImage.needed()) thresh.copyTo(_thresholdedImage);


    /// 3. DETECT RECTANGLES
    int minSize=_minLenght*std::max(thresh.cols,thresh.rows);
    cv::Mat contoursImg;
    thresh.copyTo ( contoursImg );
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours ( contoursImg , contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE );
    std::vector< std::vector<cv::Point2f> > candidates;
    for ( unsigned int i=0;i<contours.size();i++ )
    {
        if(contours[i].size() < minSize) continue;
        vector<Point>  approxCurve;
        cv::approxPolyDP (  contours[i], approxCurve, double ( contours[i].size() ) *0.05 , true );
        if(approxCurve.size() != 4 || !cv::isContourConvex(approxCurve) ) continue;
        float minDistSq = 1e10;
        for ( int j=0;j<4;j++ ) {
            float d= ( float ) ( approxCurve[j].x-approxCurve[ ( j+1 ) %4].x ) * ( approxCurve[j].x-approxCurve[ ( j+1 ) %4].x ) +
                                 ( approxCurve[j].y-approxCurve[ ( j+1 ) %4].y ) * ( approxCurve[j].y-approxCurve[ ( j+1 ) %4].y ) ;
            minDistSq = std::min(minDistSq,d);
        }
        if(minDistSq<100) continue;
        std::vector<cv::Point2f> currentCandidate;
        currentCandidate.resize(4);
        for ( int j=0;j<4;j++ ) {
            currentCandidate[j] = cv::Point2f ( approxCurve[j].x,approxCurve[j].y );
        }
        candidates.push_back(currentCandidate);
    }


    /// 4. SORT CORNERS
    for ( unsigned int i=0;i<candidates.size(); i++ ) {
        double dx1 = candidates[i][1].x - candidates[i][0].x;
        double dy1 = candidates[i][1].y - candidates[i][0].y;
        double dx2 = candidates[i][2].x - candidates[i][0].x;
        double dy2 = candidates[i][2].y - candidates[i][0].y;
        double o = ( dx1*dy2 )- ( dy1*dx2 );

        if ( o  < 0.0 )	swap ( candidates[i][1],candidates[i][3] );
    }


    /// 5. FILTER OUT NEAR CANDIDATE PAIRS
    std::vector< std::pair<int,int>  > nearCandidates;
    for ( unsigned int i=0;i<candidates.size();i++ ) {
        for ( unsigned int j=i+1;j<candidates.size(); j++ ) {
            float distSq=0;
            for ( int c=0;c<4;c++ )
                distSq += ( candidates[i][c].x-candidates[j][c].x ) * ( candidates[i][c].x-candidates[j][c].x )
                        + ( candidates[i][c].y-candidates[j][c].y ) * ( candidates[i][c].y-candidates[j][c].y ) ;
            distSq/=4.;
            if(distSq < 100) nearCandidates.push_back( std::pair<int,int> ( i,j ) );
        }
    }


    /// 6. MARK SMALLER CANDIDATES IN NEAR PAIRS TO REMOVE
    std::vector<bool> toRemove(candidates.size(), false);
    for(unsigned int i=0; i<nearCandidates.size(); i++) {
        float perimeterSq1=0, perimeterSq2=0;
        for(unsigned int c=0; c<4; c++) {
            perimeterSq1 += (candidates[nearCandidates[i].first][c].x-candidates[nearCandidates[i].first][(c+1)%4].x) *
                            (candidates[nearCandidates[i].first][c].x-candidates[nearCandidates[i].first][(c+1)%4].x) +
                            (candidates[nearCandidates[i].first][c].y-candidates[nearCandidates[i].first][(c+1)%4].y) *
                            (candidates[nearCandidates[i].first][c].y-candidates[nearCandidates[i].first][(c+1)%4].y);
            perimeterSq2 += (candidates[nearCandidates[i].second][c].x-candidates[nearCandidates[i].second][(c+1)%4].x) *
                            (candidates[nearCandidates[i].second][c].x-candidates[nearCandidates[i].second][(c+1)%4].x) +
                            (candidates[nearCandidates[i].second][c].y-candidates[nearCandidates[i].second][(c+1)%4].y) *
                            (candidates[nearCandidates[i].second][c].y-candidates[nearCandidates[i].second][(c+1)%4].y);
            if(perimeterSq1 > perimeterSq2) toRemove[nearCandidates[i].second]=true;
            else toRemove[nearCandidates[i].first]=true;
        }
    }



    /// 7. REMOVE EXTRA CANDIDATES
    int totalRemaining=0;
    for(unsigned int i=0; i<toRemove.size(); i++) if(!toRemove[i]) totalRemaining++;
    _candidates.create(totalRemaining, 1, CV_32FC2);
    for(unsigned int i=0, currIdx=0; i<candidates.size(); i++) {
        if(toRemove[i]) continue;
        _candidates.create(4,1,CV_32FC2, currIdx, true);
        Mat m = _candidates.getMat(currIdx);
        for(int j=0; j<4; j++) m.ptr<cv::Vec2f>(0)[j] = candidates[i][j];
        currIdx++;
    }


//    //find all rectangles in the thresholdes image
//    vector<MarkerCandidate > MarkerCanditates;
//    detectRectangles ( thres,MarkerCanditates );
//    //if the image has been downsampled, then calcualte the location of the corners in the original image
//    if ( pyrdown_level!=0 )
//    {
//        float red_den=pow ( 2.0f,pyrdown_level );
//        float offInc= ( ( pyrdown_level/2. )-0.5 );
//        for ( unsigned int i=0;i<MarkerCanditates.size();i++ ) {
//            for ( int c=0;c<4;c++ )
//            {
//                MarkerCanditates[i][c].x=MarkerCanditates[i][c].x*red_den+offInc;
//                MarkerCanditates[i][c].y=MarkerCanditates[i][c].y*red_den+offInc;
//            }
//            //do the same with the the contour points
//            for ( int c=0;c<MarkerCanditates[i].contour.size();c++ )
//            {
//                MarkerCanditates[i].contour[c].x=MarkerCanditates[i].contour[c].x*red_den+offInc;
//                MarkerCanditates[i].contour[c].y=MarkerCanditates[i].contour[c].y*red_den+offInc;
//            }
//        }
//    }




//    ///refine the corner location if desired
//    if ( detectedMarkers.size() >0 && _cornerMethod!=NONE && _cornerMethod!=LINES )
//    {
//        vector<Point2f> Corners;
//        for ( unsigned int i=0;i<detectedMarkers.size();i++ )
//            for ( int c=0;c<4;c++ )
//                Corners.push_back ( detectedMarkers[i][c] );

//        if ( _cornerMethod==HARRIS )
//            findBestCornerInRegion_harris ( grey, Corners,7 );
//        else if ( _cornerMethod==SUBPIX )
//            cornerSubPix ( grey, Corners,cvSize ( 5,5 ), cvSize ( -1,-1 )   ,cvTermCriteria ( CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,3,0.05 ) );

//        //copy back
//        for ( unsigned int i=0;i<detectedMarkers.size();i++ )
//            for ( int c=0;c<4;c++ )     detectedMarkers[i][c]=Corners[i*4+c];
//    }
//    //sort by id
//    std::sort ( detectedMarkers.begin(),detectedMarkers.end() );
//    //there might be still the case that a marker is detected twice because of the double border indicated earlier,
//    //detect and remove these cases
//    int borderDistThresX=_borderDistThres*float(input.cols);
//    int borderDistThresY=_borderDistThres*float(input.rows);
//    vector<bool> toRemove ( detectedMarkers.size(),false );
//    for ( int i=0;i<int ( detectedMarkers.size() )-1;i++ )
//    {
//        if ( detectedMarkers[i].id==detectedMarkers[i+1].id && !toRemove[i+1] )
//        {
//            //deletes the one with smaller perimeter
//            if ( perimeter ( detectedMarkers[i] ) >perimeter ( detectedMarkers[i+1] ) ) toRemove[i+1]=true;
//            else toRemove[i]=true;
//        }
//        //delete if any of the corners is too near image border
//        for(size_t c=0;c<detectedMarkers[i].size();c++){
//        if ( detectedMarkers[i][c].x<borderDistThresX ||
//          detectedMarkers[i][c].y<borderDistThresY ||
//          detectedMarkers[i][c].x>input.cols-borderDistThresX ||
//          detectedMarkers[i][c].y>input.rows-borderDistThresY ) toRemove[i]=true;

//    }


//    }
//    //remove the markers marker
//    removeElements ( detectedMarkers, toRemove );


}




/**
 * @brief identify a vector of marker candidates based on the dictionary codification
 * 
 * @param image input image
 * @param candidates candidate corners positions
 * @param dictionary
 * @param accepted returns vector of accepted marker corners
 * @param ids returns vector of accepted markers ids
 * @param rejected ... if set, return vector of rejected markers
 * @return void
 */
void _identifyCandidates(InputArray image, InputArrayOfArrays _candidates,
                             Dictionary dictionary, OutputArrayOfArrays _accepted, OutputArray _ids,
                             OutputArrayOfArrays _rejected=noArray()) {


    int ncandidates = _candidates.total();
    CV_Assert(ncandidates > 0);

    std::vector< cv::Mat > accepted;
    std::vector< cv::Mat > rejected;
    std::vector< int > ids;

    cv::Mat grey;
    if ( image.getMat().type() ==CV_8UC3 )   cv::cvtColor ( image.getMat(),grey,cv::COLOR_BGR2GRAY );
    else grey=image.getMat();

    for(int i=0; i<ncandidates; i++) {
        int currId;
        cv::Mat currentCandidate = _candidates.getMat(i);
        if( dictionary.identify(grey,currentCandidate,currId) ) {
            accepted.push_back(currentCandidate);
            ids.push_back(currId);
        }
        else rejected.push_back(_candidates.getMat(i));
    }

    _accepted.create((int)accepted.size(), 1, CV_32FC2);
    for(unsigned int i=0; i<accepted.size(); i++) {
        _accepted.create(4,1,CV_32FC2, i, true);
        Mat m = _accepted.getMat(i);
        accepted[i].copyTo(m);
    }

    _ids.create((int)ids.size(), 1, CV_32SC1);
    for(unsigned int i=0; i<ids.size(); i++) _ids.getMat().ptr<int>(0)[i] = ids[i];

    if(_rejected.needed()) {
        _rejected.create((int)rejected.size(), 1, CV_32FC2);
        for(unsigned int i=0; i<rejected.size(); i++) {
            _rejected.create(4,1,CV_32FC2, i, true);
            Mat m = _rejected.getMat(i);
            rejected[i].copyTo(m);
        }
    }

}



/**
 * @brief Given the marker size, it returns the vector of object points for pose estimation
 * 
 * @param markerSize size of marker in meters
 * @param objPnts vector of 4 3d points
 * @return void
 */
void getSingleMarkerObjectPoints(float markerSize, OutputArray _objPnts) {

    CV_Assert(markerSize > 0);

     _objPnts.create(4, 1, CV_32FC3);
    cv::Mat objPnts = _objPnts.getMat();
    objPnts.ptr<cv::Vec3f>(0)[0] = cv::Vec3f(-markerSize/2., markerSize/2., 0);
    objPnts.ptr<cv::Vec3f>(0)[1] = cv::Vec3f(markerSize/2., markerSize/2., 0);
    objPnts.ptr<cv::Vec3f>(0)[2] = cv::Vec3f(markerSize/2., -markerSize/2., 0);
    objPnts.ptr<cv::Vec3f>(0)[3] = cv::Vec3f(-markerSize/2., -markerSize/2., 0);

}


/**
  */
void detectMarkers(InputArray image, Dictionary dictionary, OutputArrayOfArrays imgPoints,
                       OutputArray ids, int threshParam,float minLenght) {

    cv::Mat grey;
    if ( image.getMat().type() ==CV_8UC3 )   cv::cvtColor ( image.getMat(),grey,cv::COLOR_BGR2GRAY );
    else grey=image.getMat();

    /// STEP 1: Detect marker candidates
    std::vector<std::vector<cv::Point2f> > candidates;
    _detectCandidates(grey,candidates,threshParam,minLenght);

    /// STEP 2: Check candidate codification (identify markers)
    _identifyCandidates(grey, candidates, dictionary, imgPoints, ids);

    /// STEP 3: Clean candidates

    for(int i=0; i<imgPoints.total(); i++) {
        /// STEP 4: Corner refinement
        cv::cornerSubPix ( grey, imgPoints.getMat(i), cvSize ( 5,5 ), cvSize ( -1,-1 ),cvTermCriteria ( CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,30,0.1 ) );
    }

}





/**
  */
void estimatePoseSingleMarkers(InputArrayOfArrays imgPoints, float markersize, InputArray cameraMatrix,
                                          InputArray distCoeffs, OutputArrayOfArrays rvecs, OutputArrayOfArrays tvecs) {

    cv::Mat markerObjPoints;
    getSingleMarkerObjectPoints(markersize, markerObjPoints);
    rvecs.create( (int)imgPoints.total(), 1, CV_32FC1);
    tvecs.create( (int)imgPoints.total(), 1, CV_32FC1);

    for(int i=0; i<imgPoints.total(); i++) {
        rvecs.create(3,1,CV_64FC1, i, true);
        tvecs.create(3,1,CV_64FC1, i, true);
        cv::solvePnP(markerObjPoints, imgPoints.getMat(i), cameraMatrix, distCoeffs, rvecs.getMat(i), tvecs.getMat(i));
    }

}


/**
  */
void estimatePoseBoard(InputArrayOfArrays imgPoints, InputArray ids, Board board, InputArray cameraMatrix,
                                          InputArray distCoeffs, OutputArray rvec, OutputArray tvec) {

    cv::Mat imagePointsConcatenation(imgPoints.total()*4, 1, CV_32FC2);
    for(int i=0; i<imgPoints.total(); i++) {
        for(int j=0; j<4; j++) {
            imagePointsConcatenation.ptr<cv::Point2f>(0)[i*4+j] = imgPoints.getMat(i).ptr<cv::Point2f>(0)[j];
        }
    }

    cv::Mat objectPointsConcatenation;
    board.getObjectPointsDetectedMarkers(ids, objectPointsConcatenation);

    rvec.create(3,1,CV_64FC1);
    tvec.create(3,1,CV_64FC1);
    cv::solvePnP(objectPointsConcatenation, imagePointsConcatenation, cameraMatrix, distCoeffs, rvec, tvec);
}




/**
 */
void drawDetectedMarkers(InputOutputArray image, InputArrayOfArrays markers, InputArray ids, bool drawId) {

    for(int i=0; i<markers.total(); i++) {
        cv::Mat currentMarker = markers.getMat(i);
        for(int j=0; j<4; j++) {
            cv::Point2f p0, p1;
            p0 = currentMarker.ptr<cv::Point2f>(0)[j];
            p1 = currentMarker.ptr<cv::Point2f>(0)[(j+1)%4];
            cv::line(image, p0, p1, cv::Scalar(0,255,0),2);
        }
        cv::rectangle( image, currentMarker.ptr<cv::Point2f>(0)[0]-Point2f(3,3),currentMarker.ptr<cv::Point2f>(0)[0]+Point2f(3,3),Scalar(255,0,0),2,cv::LINE_AA);
        if(drawId) {
            Point2f cent(0,0);
            for(int p=0; p<4; p++) cent += currentMarker.ptr<cv::Point2f>(0)[p];
            cent = cent/4.;
            stringstream s;
            s << "id=" << ids.getMat().ptr<int>(0)[i] ;
            putText(image, s.str(), cent, cv::FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,255), 2);
        }
    }



}


/**
 */
void drawAxis(InputOutputArray image, InputArray cameraMatrix, InputArray distCoeffs, InputArray rvec, InputArray tvec, float lenght) {
    std::vector<cv::Point3f> axisPoints;
    axisPoints.push_back(cv::Point3f(0,0,0));
    axisPoints.push_back(cv::Point3f(lenght,0,0));
    axisPoints.push_back(cv::Point3f(0,lenght,0));
    axisPoints.push_back(cv::Point3f(0,0,lenght));
    std::vector<cv::Point2f> imagePoints;
    cv::projectPoints(axisPoints, rvec, tvec, cameraMatrix, distCoeffs, imagePoints);

    cv::line(image, imagePoints[0], imagePoints[1], cv::Scalar(0,0,255), 3);
    cv::line(image, imagePoints[0], imagePoints[2], cv::Scalar(0,255,0), 3);
    cv::line(image, imagePoints[0], imagePoints[3], cv::Scalar(255,0,0), 3);

}


/**
  */
bool Dictionary::_isBorderValid(cv::Mat bits) {
    int sizeWithBorders = markerSize+2;
    int totalErrors = 0;
    for(int y=0; y<sizeWithBorders; y++) {
        if(bits.ptr<unsigned char>(y)[0]!=0) totalErrors++;
        if(bits.ptr<unsigned char>(y)[sizeWithBorders-1]!=0) totalErrors++;
    }
    for(int x=1; x<sizeWithBorders-1; x++) {
        if(bits.ptr<unsigned char>(0)[x]!=0) totalErrors++;
        if(bits.ptr<unsigned char>(sizeWithBorders-1)[x]!=0) totalErrors++;
    }
    if(totalErrors > 1) return false; // markersize is a good value for check border errors
    else return true;
}



/**
 */
bool Dictionary::identify(InputArray image, InputOutputArray imgPoints, int &idx) {
    // get bits
    cv::Mat candidateBits = _extractBits(image, imgPoints);
    if(!_isBorderValid(candidateBits)) return false; // not really necessary
    cv::Mat onlyBits = candidateBits.rowRange(1,candidateBits.rows-1).colRange(1,candidateBits.rows-1);
    // get quartets
    cv::Mat candidateQuartets = _getQuartet(onlyBits);


    // search closest marker in dict
    int closestId=-1;
    unsigned int rotation=0;
    unsigned int closestDistance=markerSize*markerSize+1;
    cv::Mat candidateDistances = _getDistances(candidateQuartets);
    for(int i=0; i<codes.rows; i++) {
        if(candidateDistances.ptr<int>(i)[0] < closestDistance) {
            closestDistance = candidateDistances.ptr<int>(i)[0];
            closestId = i;
            rotation = candidateDistances.ptr<int>(i)[1];
        }
    }

    // return closest id
    if(closestId!=-1 && closestDistance<=maxCorrectionBits) {
        idx = closestId;
        // correct imgPoints positions
        if(rotation!=0) {
            cv::Mat copyPoints = imgPoints.getMat().clone();
            for(int j=0; j<4; j++) imgPoints.getMat().ptr<cv::Point2f>(0)[j] = copyPoints.ptr<cv::Point2f>(0)[(j+4-rotation)%4];
        }
        return true;
    }
    else {
        idx = -1;
        return false;
    }
}


/**
  */
cv::Mat Dictionary::_extractBits(InputArray image, InputOutputArray imgPoints) {

    CV_Assert(image.getMat().channels()==1);

    cv::Mat resultImg; // marker image after removing perspective
    int squareSizePixels = 8;
    int resultImgSize = (markerSize+2)*squareSizePixels;
    cv::Mat resultImgCorners(4,1,CV_32FC2);
    resultImgCorners.ptr<cv::Point2f>(0)[0]= Point2f ( 0,0 );
    resultImgCorners.ptr<cv::Point2f>(0)[1]= Point2f ( resultImgSize-1,0 );
    resultImgCorners.ptr<cv::Point2f>(0)[2]= Point2f ( resultImgSize-1,resultImgSize-1 );
    resultImgCorners.ptr<cv::Point2f>(0)[3]= Point2f ( 0,resultImgSize-1 );

    // remove perspective
    cv::Mat transformation = cv::getPerspectiveTransform(imgPoints, resultImgCorners);
    cv::warpPerspective(image, resultImg, transformation, cv::Size(resultImgSize, resultImgSize), cv::INTER_NEAREST);

    // now extract code
    cv::Mat bits(markerSize+2, markerSize+2, CV_8UC1, cv::Scalar::all(0));
    cv::threshold(resultImg, resultImg,125, 255, cv::THRESH_BINARY|cv::THRESH_OTSU);
    for (unsigned int y=0; y<markerSize+2; y++)  {
        for (unsigned int x=0; x<markerSize+2;x++) {
            int Xstart=x*(squareSizePixels)+1;
            int Ystart=y*(squareSizePixels)+1;
            cv::Mat square=resultImg(cv::Rect(Xstart,Ystart,squareSizePixels-2,squareSizePixels-2));
            int nZ=countNonZero(square);
            if (nZ> square.total()/2)  bits.at<unsigned char>(y,x)=1;
        }
     }

    return bits;
}



/**
 */
void Dictionary::drawMarker(InputOutputArray img, int id) {
    /// TODO
}


/**
  */
cv::Mat Dictionary::_getQuartet(cv::Mat bits) {

    int nquartets = (markerSize*markerSize)/4 + (markerSize*markerSize)%4;
    cv::Mat candidateQuartets(1, nquartets, CV_8UC1);
    int currentQuartet=0;
    for(int row=0; row<markerSize/2; row++)
    {
        for(int col=row; col<markerSize-row-1; col++) {
            unsigned char bit3 = bits.at<unsigned char>(row,col);
            unsigned char bit2 = bits.at<unsigned char>(col,markerSize-1-row);
            unsigned char bit1 = bits.at<unsigned char>(markerSize-1-row,markerSize-1-col);
            unsigned char bit0 = bits.at<unsigned char>(markerSize-1-col,row);
            unsigned char quartet = 8*bit3 + 4*bit2 + 2*bit1 + bit0;
            candidateQuartets.ptr<unsigned char>()[currentQuartet] = quartet;
            currentQuartet++;
        }
    }
    if((markerSize*markerSize)%4 == 1) { // middle bit
        unsigned char middleBit = 15*bits.at<unsigned char>(markerSize/2,markerSize/2);
        candidateQuartets.ptr<unsigned char>()[currentQuartet] = middleBit;
    }
    return candidateQuartets;

}


/**
  */
cv::Mat Dictionary::_getDistances(cv::Mat quartets) {

    cv::Mat res(codes.rows, 2, CV_32SC1);
    for(unsigned int m=0; m<codes.rows; m++) {
        res.ptr<int>(m)[0]=10e8;
        for(unsigned int r=0; r<4; r++) {
            int currentHamming=0;
            for(unsigned int q=0; q<quartets.total(); q++) {
                currentHamming += (int)quartets_distances[ (codes.ptr<unsigned char>(m)[q]) ][ (quartets.ptr<unsigned char>(0)[q]) ][r];
            }
            if(currentHamming < res.ptr<int>(m)[0]) {
                res.ptr<int>(m)[0]=currentHamming;
                res.ptr<int>(m)[1]=r;
            }
        }
    }
    return res;
}



/**
 */
void Board::drawBoard(InputOutputArray img) {
    /// TODO
}

/**
  */
void Board::getObjectPointsDetectedMarkers(InputArray detectedIds, OutputArray objectPoints) {
    std::vector<cv::Point3f> objPnts;
    objPnts.reserve(detectedIds.total());

    for(int i=0; i<detectedIds.getMat().rows; i++) {
        int currentId = detectedIds.getMat().ptr<int>(0)[i];
        for(int j=0; j<ids.size(); j++) {
            if(currentId == ids[j]) {
                for(int p=0; p<4; p++) objPnts.push_back( objPoints[j][p] );
            }
        }
    }

    objectPoints.create(objPnts.size(), 1, CV_32FC3);
    for(int i=0; objPnts.size(); i++) objectPoints.getMat().ptr<cv::Point3f>(0)[i] = objPnts[i];

}


/**
 */
Board Board::createPlanarBoard(int width, int height, float markerSize, float markerSeparation) {
    /// TODO
    return Board();
}


}}

#endif // cplusplus
#endif // __OPENCV_ARUCO_CPP__

