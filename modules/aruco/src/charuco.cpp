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

#include "precomp.hpp"
#include "opencv2/aruco/charuco.hpp"

#include <opencv2/core.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

#include <iostream>


namespace cv {
namespace aruco {

using namespace std;



/**
 */
void CharucoBoard::draw(Size outSize, OutputArray _img, int marginSize, int borderBits) {

    CV_Assert(outSize.area() > 0);
    CV_Assert(marginSize >= 0);

    _img.create(outSize, CV_8UC1);
    _img.setTo(255);
    Mat out = _img.getMat();
    Mat noMarginsImg = out.colRange(marginSize, out.cols - marginSize)
                          .rowRange(marginSize, out.rows - marginSize);

    double totalLengthX, totalLengthY;
    totalLengthX = _squareLength * _squaresX;
    totalLengthY = _squareLength * _squaresY;

    double xReduction = totalLengthX / double(noMarginsImg.cols);
    double yReduction = totalLengthY / double(noMarginsImg.rows);

    // determine the zone where the chessboard is placed
    Mat chessboardZoneImg;
    if (xReduction > yReduction) {
        int nRows = int(totalLengthY / xReduction);
        int rowsMargins = (noMarginsImg.rows - nRows) / 2;
        chessboardZoneImg = noMarginsImg.rowRange(rowsMargins, noMarginsImg.rows - rowsMargins);
    } else {
        int nCols = int( totalLengthX / yReduction );
        int colsMargins = (noMarginsImg.cols - nCols) / 2;
        chessboardZoneImg = noMarginsImg.colRange(colsMargins, noMarginsImg.cols - colsMargins);
    }

    // determine the margins to draw only the markers
    // take the minimum just to be sure
    double squareSizePixels = min( double(chessboardZoneImg.cols) / double(_squaresX),
                                   double(chessboardZoneImg.rows) / double(_squaresY) );

    double diffSquareMarkerLength = (_squareLength - _markerLength) / 2;
    int diffSquareMarkerLengthPixels = int(diffSquareMarkerLength *
                                           squareSizePixels / _squareLength);

    // draw markers
    Mat markersImg;
    aruco::drawPlanarBoard((*this), chessboardZoneImg.size(), markersImg,
                           diffSquareMarkerLengthPixels, borderBits);

    markersImg.copyTo(chessboardZoneImg);

    // now draw black boards
    for (int y = 0; y < _squaresY; y++) {
        for (int x = 0; x < _squaresX; x++) {

            if (y % 2 != x % 2)
                continue; // white corner, dont do anything

            double startX, startY;
            startX = squareSizePixels * double(x);
            startY = double(chessboardZoneImg.rows) - squareSizePixels * double(y+1);

            Mat squareZone = chessboardZoneImg.rowRange(int(startY),
                                                        int(startY + squareSizePixels))
                                              .colRange(int(startX),
                                                        int(startX + squareSizePixels));

            squareZone.setTo(0);

        }
    }

}



/**
 */
CharucoBoard CharucoBoard::create(int squaresX, int squaresY, float squareLength,
                                  float markerLength, Dictionary dictionary) {

    CV_Assert(squaresX > 1 && squaresY > 1 && markerLength > 0 && squareLength > markerLength);
    CharucoBoard res;

    res._squaresX = squaresX;
    res._squaresY = squaresY;
    res._squareLength = squareLength;
    res._markerLength = markerLength;
    res.dictionary = dictionary;

    float diffSquareMarkerLength = (squareLength - markerLength) / 2;

    // calculate Board objPoints
    for (int y = squaresY-1; y >= 0; y--) {
        for (int x = 0; x < squaresX; x++) {

            if (y % 2 == x % 2)
                continue; // black corner, no marker here

            vector<Point3f> corners;
            corners.resize(4);
            corners[0] = Point3f(x * squareLength + diffSquareMarkerLength,
                                 y * squareLength + diffSquareMarkerLength + markerLength, 0);
            corners[1] = corners[0] + Point3f(markerLength, 0, 0);
            corners[2] = corners[0] + Point3f(markerLength, -markerLength, 0);
            corners[3] = corners[0] + Point3f(0, -markerLength, 0);
            res.objPoints.push_back(corners);
            // first ids in dictionary
            int nextId = (int)res.ids.size();
            res.ids.push_back(nextId);
        }
    }

    // now fill chessboardCorners
    for (int y = 0; y < squaresY-1; y++) {
        for (int x = 0; x < squaresX-1; x++) {
            Point3f corner;
            corner.x = (x+1)*squareLength;
            corner.y = (y+1)*squareLength;
            corner.z = 0;
            res.chessboardCorners.push_back(corner);
        }
    }

    res._getNearestMarkerCorners();

    return res;
}



/**
  */
void CharucoBoard::_getNearestMarkerCorners() {

    nearestMarkerIdx.resize(chessboardCorners.size());
    nearestMarkerCorners.resize(chessboardCorners.size());

    unsigned int nMarkers = (unsigned int)ids.size();
    unsigned int nCharucoCorners = (unsigned int)chessboardCorners.size();
    for (unsigned int i=0; i<nCharucoCorners; i++) {
        double minDist=-1;
        Point3f charucoCorner = chessboardCorners[i];
        for(unsigned int j=0; j<nMarkers; j++) {
            Point3f center = Point3f(0,0,0);
            for(unsigned int k=0; k<4; k++)
                center += objPoints[j][k];
            center /= 4.;
            double sqDistance;
            Point3f distVector = charucoCorner - center;
            sqDistance = distVector.x*distVector.x + distVector.y*distVector.y;
            if(j==0 || fabs(sqDistance - minDist)<0.0001) {
                nearestMarkerIdx[i].push_back(j);
                minDist = sqDistance;
            }
            else if(sqDistance < minDist) {
                nearestMarkerIdx[i].clear();
                nearestMarkerIdx[i].push_back(j);
                minDist = sqDistance;
            }
        }

        for(unsigned int j=0; j<nearestMarkerIdx[i].size(); j++) {
            nearestMarkerCorners[i].resize(nearestMarkerIdx[i].size());
            double minDistCorner=-1;
            for(unsigned int k=0; k<4; k++) {
                double sqDistance;
                Point3f distVector = charucoCorner - objPoints[nearestMarkerIdx[i][j]][k];
                sqDistance = distVector.x*distVector.x + distVector.y*distVector.y;
                if (k==0 || sqDistance < minDistCorner) {
                    minDistCorner = sqDistance;
                    nearestMarkerCorners[i][j] = k;
                }
            }
        }

    }

}


/**
  */
static unsigned int
_filterCornersWithoutMinMarkers(const CharucoBoard &board,
                                InputArray _allCharucoCorners,
                                InputArray _allCharucoIds, InputArray _allArucoIds,
                                int minMarkers, OutputArray _filteredCharucoCorners,
                                OutputArray _filteredCharucoIds) {

    vector<Point2f> filteredCharucoCorners;
    vector<int> filteredCharucoIds;

    for (unsigned int i=0; i<_allCharucoIds.getMat().total(); i++) {
        int currentCharucoId = _allCharucoIds.getMat().ptr<int>(0)[i];
        int totalMarkers = 0;
        for (unsigned int m=0; m<board.nearestMarkerIdx[currentCharucoId].size(); m++) {
            int markerId = board.ids[ board.nearestMarkerIdx[currentCharucoId][m] ];
            bool found = false;
            for(unsigned int k=0; k<_allArucoIds.getMat().total(); k++) {
                if(_allArucoIds.getMat().ptr<int>(0)[k] == markerId) {
                    found = true;
                    break;
                }
            }
            if(found)
                totalMarkers++;
        }
        if(totalMarkers >= minMarkers) {
            filteredCharucoIds.push_back(currentCharucoId);
            filteredCharucoCorners.push_back(_allCharucoCorners.getMat().ptr<Point2f>(0)[i]);
        }
    }

    // parse output
    _filteredCharucoCorners.create((int)filteredCharucoCorners.size(), 1, CV_32FC2);
    for (unsigned int i = 0; i < filteredCharucoCorners.size(); i++) {
        _filteredCharucoCorners.getMat().ptr<Point2f>(0)[i] = filteredCharucoCorners[i];
    }

    _filteredCharucoIds.create((int)filteredCharucoIds.size(), 1, CV_32SC1);
    for (unsigned int i = 0; i < filteredCharucoIds.size(); i++) {
        _filteredCharucoIds.getMat().ptr<int>(0)[i] = filteredCharucoIds[i];
    }

    return (unsigned int)filteredCharucoCorners.size();

}


/**
  * ParallelLoopBody class for the parallelization of the charuco corners subpixel refinement
  * Called from function _selectAndRefineChessboardCorners()
  */
class CharucoSubpixelParallel : public ParallelLoopBody
{
public:
    CharucoSubpixelParallel( const Mat *_grey,
                             vector<Point2f> *_filteredChessboardImgPoints,
                             vector<Size> *_filteredWinSizes,
                             DetectorParameters *_params )
        : grey(_grey),
          filteredChessboardImgPoints(_filteredChessboardImgPoints),
          filteredWinSizes(_filteredWinSizes),
          params(_params) { }

    void operator()( const cv::Range& range ) const
    {
        const int begin = range.start;
        const int end = range.end;

        for ( int i = begin; i<end; i++ )
        {
            vector<Point2f> in;
            in.push_back((*filteredChessboardImgPoints)[i]);
            cv::Size winSize = (*filteredWinSizes)[i];
            if(winSize.height == -1 || winSize.width == -1)
                winSize = cv::Size(params->cornerRefinementWinSize,
                                   params->cornerRefinementWinSize);

            cornerSubPix(*grey, in, winSize, cv::Size(),
                         cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS,
                                        params->cornerRefinementMaxIterations,
                                        params->cornerRefinementMinAccuracy));

            (*filteredChessboardImgPoints)[i] = in[0];
        }
    }

private:
    CharucoSubpixelParallel& operator=(const CharucoSubpixelParallel&); // to quiet MSVC

    const Mat *grey;
    vector<Point2f> *filteredChessboardImgPoints;
    vector<Size> *filteredWinSizes;
    DetectorParameters *params;
};




/**
  * @brief From all projected chessboard corners, select those inside the image and apply subpixel
  * refinement. Returns number of valid corners.
  */
static unsigned int
_selectAndRefineChessboardCorners(InputArray _allCorners, InputArray _image,
                                  OutputArray _selectedCorners,
                                  OutputArray _selectedIds,
                                  const vector<Size> &winSizes) {

    // filter points outside image
    int minDistToBorder = 2;
    vector<Point2f> filteredChessboardImgPoints;
    vector<Size> filteredWinSizes;
    vector<int> filteredIds;
    Rect innerRect (minDistToBorder, minDistToBorder,
                    _image.getMat().cols - 2 * minDistToBorder,
                    _image.getMat().rows - 2 * minDistToBorder);
    for (unsigned int i=0; i<_allCorners.getMat().total(); i++) {
        if (innerRect.contains(_allCorners.getMat().ptr<Point2f>(0)[i])) {
            filteredChessboardImgPoints.push_back(_allCorners.getMat().ptr<Point2f>(0)[i]);
            filteredIds.push_back(i);
            filteredWinSizes.push_back(winSizes[i]);
        }
    }

    // if none valid, return 0
    if (filteredChessboardImgPoints.size() == 0) return 0;

    // corner refinement
    Mat grey;
    if (_image.getMat().type() == CV_8UC3)
        cvtColor(_image.getMat(), grey, COLOR_BGR2GRAY);
    else
       _image.getMat().copyTo(grey);

    DetectorParameters params; // use default params for corner refinement

    //// For each of the charuco corners, apply subpixel refinement
    //for(unsigned int i=0; i<filteredChessboardImgPoints.size(); i++) {
    //    vector<Point2f> in;
    //    in.push_back(filteredChessboardImgPoints[i]);
    //    cv::Size winSize = filteredWinSizes[i];
    //    if(winSize.height == -1 || winSize.width == -1)
    //        winSize = cv::Size(params.cornerRefinementWinSize, params.cornerRefinementWinSize);
    //    cornerSubPix(grey, in, winSize, cv::Size(),
    //                 cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS,
    //                                params.cornerRefinementMaxIterations,
    //                                params.cornerRefinementMinAccuracy));
    //    filteredChessboardImgPoints[i] = in[0];
    //}

    // this is the parallel call for the previous commented loop (result is equivalent)
    parallel_for_(Range(0, filteredChessboardImgPoints.size()),
                  CharucoSubpixelParallel(&grey, &filteredChessboardImgPoints, &filteredWinSizes,
                                          &params));

    // parse output
    _selectedCorners.create((int)filteredChessboardImgPoints.size(), 1, CV_32FC2);
    for (unsigned int i = 0; i < filteredChessboardImgPoints.size(); i++) {
        _selectedCorners.getMat().ptr<Point2f>(0)[i] = filteredChessboardImgPoints[i];
    }

    _selectedIds.create((int)filteredIds.size(), 1, CV_32SC1);
    for (unsigned int i = 0; i < filteredIds.size(); i++) {
        _selectedIds.getMat().ptr<int>(0)[i] = filteredIds[i];
    }

    return (unsigned int)filteredChessboardImgPoints.size();

}


/**
  */
static void
_getMaximumSubPixWindowSizes(InputArrayOfArrays markerCorners,
                             InputArray markerIds,
                             InputArray charucoCorners, const CharucoBoard &board,
                             vector< Size> &sizes) {

    unsigned int nCharucoCorners = (unsigned int)charucoCorners.getMat().total();
    sizes.resize(nCharucoCorners, Size(-1,-1));

    for (unsigned int i=0; i<nCharucoCorners; i++) {
        if(charucoCorners.getMat().ptr<Point2f>(0)[i] == Point2f(-1, -1))
            continue;
        if(board.nearestMarkerIdx[i].size() == 0)
            continue;

        double minDist = 999999.;
        int counter = 0;

        for(unsigned int j=0; j<board.nearestMarkerIdx[i].size(); j++) {
            int markerId = board.ids[ board.nearestMarkerIdx[i][j] ];
            int markerIdx = -1;
            for(unsigned int k=0; k<markerIds.getMat().total(); k++) {
                if ( markerIds.getMat().ptr<int>(0)[k] == markerId ) {
                    markerIdx = k;
                    break;
                }
            }
            if(markerIdx == -1) continue;
            Point2f markerCorner = markerCorners.getMat(markerIdx)
                                   .ptr<Point2f>(0)[ board.nearestMarkerCorners[i][j]];
            Point2f charucoCorner = charucoCorners.getMat().ptr<Point2f>(0)[i];
            double dist = norm( markerCorner - charucoCorner);
            minDist = min(dist, minDist);
            counter ++;
        }

        if(counter == 0)
            continue;
        else {
            int winSizeInt = int(minDist-2);
            if(winSizeInt == 0) winSizeInt = 1;
            if(winSizeInt > 10) winSizeInt = 10;
            sizes[i] = Size(winSizeInt, winSizeInt);
        }

    }

}



/**
  */
static int
_interpolateCornersCharucoApproxCalib(InputArrayOfArrays _markerCorners, InputArray _markerIds,
                                     InputArray _image, const CharucoBoard &board,
                                     InputArray _cameraMatrix, InputArray _distCoeffs,
                                     OutputArray _charucoCorners, OutputArray _charucoIds ) {

    CV_Assert(_image.getMat().channels() == 1 || _image.getMat().channels() == 3);
    CV_Assert(_markerCorners.total() == _markerIds.getMat().total() &&
              _markerIds.getMat().total() > 0);

    // approximated pose estimation
    Mat approximatedRvec, approximatedTvec;
    int detectedBoardMarkers;
    detectedBoardMarkers = aruco::estimatePoseBoard(_markerCorners, _markerIds, board,
                                                    _cameraMatrix, _distCoeffs,
                                                    approximatedRvec, approximatedTvec);

    if(detectedBoardMarkers == 0)
        return 0;


    // project chessboard corners
    vector<Point2f> allChessboardImgPoints;
    projectPoints(board.chessboardCorners, approximatedRvec, approximatedTvec, _cameraMatrix,
                  _distCoeffs, allChessboardImgPoints);


    vector<Size> subPixWinSizes;
    _getMaximumSubPixWindowSizes(_markerCorners, _markerIds, allChessboardImgPoints, board,
                                 subPixWinSizes);

    unsigned int nRefinedCorners;
    nRefinedCorners = _selectAndRefineChessboardCorners(allChessboardImgPoints, _image,
                                                        _charucoCorners,
                                                        _charucoIds, subPixWinSizes);

    nRefinedCorners = _filterCornersWithoutMinMarkers(board, _charucoCorners, _charucoIds,
                                                      _markerIds, 2, _charucoCorners, _charucoIds);

    return nRefinedCorners;


}



/**
  */
static int
_interpolateCornersCharucoLocalHom(InputArrayOfArrays _markerCorners, InputArray _markerIds,
                                   InputArray _image, const CharucoBoard &board,
                                   OutputArray _charucoCorners, OutputArray _charucoIds ) {

    CV_Assert(_image.getMat().channels() == 1 || _image.getMat().channels() == 3);
    CV_Assert(_markerCorners.total() == _markerIds.getMat().total() &&
              _markerIds.getMat().total() > 0);

    unsigned int nMarkers = (unsigned int)_markerIds.getMat().total();
    vector<Mat> transformations;
    transformations.resize(nMarkers);
    for (unsigned int i=0; i<nMarkers; i++) {
        vector<Point2f> markerObjPoints2D;
        int markerId = _markerIds.getMat().ptr<int>(0)[i];
        vector<int>::const_iterator it = find(board.ids.begin(), board.ids.end (), markerId);
        if(it == board.ids.end()) continue;
        int boardIdx = (int)std::distance(board.ids.begin(), it);
        markerObjPoints2D.resize(4);
        for(unsigned int j=0; j<4; j++)
            markerObjPoints2D[j] = Point2f(board.objPoints[boardIdx][j].x,
                                           board.objPoints[boardIdx][j].y );

        transformations[i] = getPerspectiveTransform(markerObjPoints2D, _markerCorners.getMat(i));
    }

    unsigned int nCharucoCorners = (unsigned int)board.chessboardCorners.size();
    vector<Point2f> allChessboardImgPoints(nCharucoCorners, Point2f(-1, -1));

    for(unsigned int i = 0; i< nCharucoCorners; i++) {
        Point2f objPoint2D = Point2f(board.chessboardCorners[i].x, board.chessboardCorners[i].y);

        vector<Point2f> interpolatedPositions;

        for (unsigned int j=0; j<board.nearestMarkerIdx[i].size(); j++) {
            int markerId = board.ids[ board.nearestMarkerIdx[i][j] ];
            int markerIdx = -1;
            for(unsigned int k=0; k<_markerIds.getMat().total(); k++) {
                if(_markerIds.getMat().ptr<int>(0)[k] == markerId) {
                    markerIdx = k;
                    break;
                }
            }
            if (markerIdx != -1) {
                vector<Point2f> in, out;
                in.push_back(objPoint2D);
                perspectiveTransform(in, out, transformations[markerIdx]);
                interpolatedPositions.push_back(out[0]);
            }
        }

        if (interpolatedPositions.size() == 0) continue;

        if(interpolatedPositions.size() > 1) {
            allChessboardImgPoints[i] = (interpolatedPositions[0] + interpolatedPositions[1] )/2.;
        }
        else
            allChessboardImgPoints[i] = interpolatedPositions[0];
    }

    vector<Size> subPixWinSizes;
    _getMaximumSubPixWindowSizes(_markerCorners, _markerIds, allChessboardImgPoints, board,
                                 subPixWinSizes);


    // refine corners
    unsigned int nRefinedCorners;
    nRefinedCorners = _selectAndRefineChessboardCorners(allChessboardImgPoints, _image,
                                                        _charucoCorners,
                                                        _charucoIds, subPixWinSizes);

    nRefinedCorners = _filterCornersWithoutMinMarkers(board, _charucoCorners, _charucoIds,
                                                      _markerIds, 2, _charucoCorners, _charucoIds);

    return nRefinedCorners;

}



/**
  */
int interpolateCornersCharuco(InputArrayOfArrays _markerCorners, InputArray _markerIds,
                              InputArray _image, const CharucoBoard &board,
                              OutputArray _charucoCorners, OutputArray _charucoIds,
                              InputArray _cameraMatrix, InputArray _distCoeffs) {

    // if camera parameters are avaible, use approximated calibration
    if(_cameraMatrix.total() != 0) {
        return _interpolateCornersCharucoApproxCalib(_markerCorners, _markerIds, _image, board,
                                                     _cameraMatrix, _distCoeffs, _charucoCorners,
                                                     _charucoIds);
    }
    // else use local homography
    else {
        return _interpolateCornersCharucoLocalHom(_markerCorners, _markerIds, _image, board,
                                                  _charucoCorners, _charucoIds);
    }


}



/**
  */
void drawDetectedCornersCharuco(InputArray _in, OutputArray _out, InputArray _charucoCorners,
                                InputArray _charucoIds, Scalar cornerColor) {

    CV_Assert(_in.getMat().cols != 0 && _in.getMat().rows != 0 &&
              (_in.getMat().channels() == 1 || _in.getMat().channels() == 3));
    CV_Assert((_charucoCorners.getMat().total() == _charucoIds.getMat().total()) ||
              _charucoIds.getMat().total()==0 );

    _out.create(_in.size(), CV_8UC3);
    Mat outImg = _out.getMat();
    if (_in.getMat().channels()==3)
        _in.getMat().copyTo(outImg);
    else
        cvtColor(_in.getMat(), outImg, COLOR_GRAY2BGR);

    unsigned int nCorners = (unsigned int)_charucoCorners.getMat().total();
    for (unsigned int i = 0; i < nCorners; i++) {
        Point2f corner = _charucoCorners.getMat().ptr<Point2f>(0)[i];

        // draw first corner mark
        rectangle(outImg, corner - Point2f(3, 3), corner + Point2f(3, 3), cornerColor, 1, LINE_AA);

        // draw ID
        if (_charucoIds.total() != 0) {
            int id = _charucoIds.getMat().ptr<int>(0)[i];
            stringstream s;
            s << "id=" << id;
            putText(outImg, s.str(), corner + Point2f(5,-5), FONT_HERSHEY_SIMPLEX, 0.5,
                    cornerColor, 2);
        }
    }

}


/**
  * Check if a set of 3d points are enough for calibration. Z coordinate is ignored.
  * Only axis paralel lines are considered
  */
static bool _arePointsEnoughForPoseEstimation(const vector<Point3f> &points) {

    if (points.size() < 4) return false;

    vector<double> sameXValue; // different x values in points
    vector<int> sameXCounter; // number of points with the x value in sameXValue
    for (unsigned int i=0; i<points.size(); i++) {
        bool found = false;
        for (unsigned int j=0; j<sameXValue.size(); j++) {
            if (sameXValue[j] == points[i].x) {
                found = true;
                sameXCounter[j] ++;
            }
        }
        if (!found) {
            sameXValue.push_back(points[i].x);
            sameXCounter.push_back(1);
        }
    }

    // count how many x values has more than 2 points
    int moreThan2 = 0;
    for (unsigned int i=0; i<sameXCounter.size(); i++) {
        if(sameXCounter[i] >= 2)
            moreThan2 ++;
    }

    // if we have more than 1 two xvalues with more than 2 points, calibration is ok
    if (moreThan2 > 1)
        return true;
    else
        return false;

}


/**
  */
bool estimatePoseCharucoBoard(InputArray _charucoCorners, InputArray _charucoIds,
                              CharucoBoard &board, InputArray _cameraMatrix, InputArray _distCoeffs,
                              OutputArray _rvec, OutputArray _tvec) {

    CV_Assert((_charucoCorners.getMat().total() == _charucoIds.getMat().total()));

    // need, at least, 4 corners
    if (_charucoIds.getMat().total() < 4) return false;

    vector<Point3f> objPoints;
    objPoints.reserve(_charucoIds.getMat().total());
    for (unsigned int i=0; i < _charucoIds.getMat().total(); i++) {
        int currId = _charucoIds.getMat().ptr<int>(0)[i];
        CV_Assert(currId >= 0 && currId < (int)board.chessboardCorners.size());
        objPoints.push_back(board.chessboardCorners[currId]);
    }

    // points need to be in different lines
    if (!_arePointsEnoughForPoseEstimation(objPoints))
        return false;

    solvePnP(objPoints, _charucoCorners, _cameraMatrix, _distCoeffs, _rvec, _tvec);

    return true;

}





/**
  */
double calibrateCameraCharuco(InputArrayOfArrays _charucoCorners, InputArrayOfArrays _charucoIds,
                              const CharucoBoard &board, Size imageSize,
                              InputOutputArray _cameraMatrix, InputOutputArray _distCoeffs,
                              OutputArrayOfArrays _rvecs, OutputArrayOfArrays _tvecs, int flags,
                              TermCriteria criteria) {


    CV_Assert(_charucoIds.total() > 0 && (_charucoIds.total() == _charucoCorners.total()));

    vector< vector<Point3f> > allObjPoints;
    allObjPoints.resize(_charucoIds.total());
    for (unsigned int i=0; i<_charucoIds.total(); i++) {
        unsigned int nCorners = (unsigned int)_charucoIds.getMat(i).total();
        CV_Assert(nCorners > 0 && nCorners == _charucoCorners.getMat(i).total());
        allObjPoints[i].reserve(nCorners);

        for (unsigned int j=0; j<nCorners; j++) {
            int pointId = _charucoIds.getMat(i).ptr<int>(0)[j];
            CV_Assert(pointId >= 0 && pointId < (int)board.chessboardCorners.size());
            allObjPoints[i].push_back(board.chessboardCorners[pointId]);
        }

    }

    return calibrateCamera(allObjPoints, _charucoCorners, imageSize, _cameraMatrix, _distCoeffs,
                           _rvecs, _tvecs, flags, criteria);

}



/**
 */
void detectCharucoDiamond(InputArray _image, InputArrayOfArrays _markerCorners,
                          InputArray _markerIds, float squareMarkerLengthRate,
                          OutputArrayOfArrays _diamondCorners, OutputArray _diamondIds,
                          InputArray _cameraMatrix, InputArray _distCoeffs) {

    CV_Assert(_markerIds.total() > 0 && _markerIds.total() == _markerCorners.total());

    const float minRepDistanceRate = 0.12f;

    CharucoBoard charucoDiamondLayout;
    Dictionary dict = getPredefinedDictionary(PREDEFINED_DICTIONARY_NAME(0));
    charucoDiamondLayout = CharucoBoard::create(3, 3, squareMarkerLengthRate, 1., dict);

    vector< vector<Point2f> > diamondCorners;
    vector< Vec4i > diamondIds;

    vector<bool> assigned(_markerIds.total(), false);
    if(_markerIds.total() < 4)
        return;

    Mat grey;
    if (_image.getMat().type() == CV_8UC3)
        cvtColor(_image.getMat(), grey, COLOR_BGR2GRAY);
    else
       _image.getMat().copyTo(grey);

    for (unsigned int i=0; i<_markerIds.total(); i++) {
        if(assigned[i])
            continue;

        float perimeterSq = 0;
        Mat corners = _markerCorners.getMat(i);
        for (int c = 0; c < 4; c++) {
            perimeterSq += (corners.ptr<Point2f>()[c].x - corners.ptr<Point2f>()[(c + 1) % 4].x) *
                           (corners.ptr<Point2f>()[c].x - corners.ptr<Point2f>()[(c + 1) % 4].x) +
                           (corners.ptr<Point2f>()[c].y - corners.ptr<Point2f>()[(c + 1) % 4].y) *
                           (corners.ptr<Point2f>()[c].y - corners.ptr<Point2f>()[(c + 1) % 4].y);
        }
        float minRepDistance = perimeterSq * minRepDistanceRate * minRepDistanceRate;

        int currentId = _markerIds.getMat().ptr<int>()[i];

        vector< Mat > currentMarker;
        vector< int > currentMarkerId;
        currentMarker.push_back(_markerCorners.getMat(i));
        currentMarkerId.push_back(currentId);

        vector< Mat > candidates;
        vector< int > candidatesIdxs;
        for(unsigned int k=0; k<assigned.size(); k++) {
            if(k==i) continue;
            if(!assigned[k]) {
                candidates.push_back(_markerCorners.getMat(k));
                candidatesIdxs.push_back(k);
            }
        }
        if(candidates.size()<3)
            break;

        for(int k=1; k<4; k++)
            charucoDiamondLayout.ids[k] = currentId+1+k;
        charucoDiamondLayout.ids[0] = currentId;

        vector<int> acceptedIdxs;
        aruco::refineDetectedMarkers(grey, charucoDiamondLayout, currentMarker, currentMarkerId,
                                     candidates, noArray(), noArray(), minRepDistance, -1,
                                     false, acceptedIdxs);

        if(currentMarker.size() == 4) {

            assigned[i] = true;

            Vec4i markerId;
            markerId[0] = currentId;
            for(int k=1; k<4; k++) {
                int currentMarkerIdx = candidatesIdxs[ acceptedIdxs[k-1] ];
                markerId[k] = _markerIds.getMat().ptr<int>()[currentMarkerIdx];
                assigned[ currentMarkerIdx ] = true;
            }

            vector< Point2f > currentMarkerCorners;
            Mat aux;
            interpolateCornersCharuco(currentMarker, currentMarkerId, grey,
                                      charucoDiamondLayout, currentMarkerCorners, aux,
                                      _cameraMatrix, _distCoeffs);

            if(currentMarkerCorners.size() > 0) {
                // reorder corners
                vector< Point2f > currentMarkerCornersReorder;
                currentMarkerCornersReorder.resize(4);
                currentMarkerCornersReorder[0] = currentMarkerCorners[2];
                currentMarkerCornersReorder[1] = currentMarkerCorners[3];
                currentMarkerCornersReorder[2] = currentMarkerCorners[1];
                currentMarkerCornersReorder[3] = currentMarkerCorners[0];

                diamondCorners.push_back(currentMarkerCornersReorder);
                diamondIds.push_back(markerId);
            }

        }

    }


    if(diamondIds.size() > 0) {

    // parse output
    _diamondIds.create((int)diamondIds.size(), 1, CV_32SC4);
    for (unsigned int i = 0; i < diamondIds.size(); i++)
        _diamondIds.getMat().ptr<Vec4i>(0)[i] = diamondIds[i];

        _diamondCorners.create((int)diamondCorners.size(), 1, CV_32FC2);
        for (unsigned int i = 0; i < diamondCorners.size(); i++) {
            _diamondCorners.create(4, 1, CV_32FC2, i, true);
            for(int j=0; j<4; j++) {
                _diamondCorners.getMat(i).ptr<Point2f>()[j] =
                        diamondCorners[i][j];

            }
        }
    }

}





/**
  */
void drawCharucoDiamond(Dictionary dictionary, Vec4i ids, int squareLength, int markerLength,
                        OutputArray _img, int marginSize, int borderBits) {

    CV_Assert(squareLength > 0 && markerLength > 0 && squareLength > markerLength);
    CV_Assert(marginSize >= 0 && borderBits > 0);

    // create a charuco board similar to a charuco marker and print it
    CharucoBoard board = CharucoBoard::create(3, 3, squareLength, markerLength, dictionary);

    // assign the charuco marker ids
    for(int i=0; i<4; i++)
        board.ids[i] = ids[i];

    Size outSize(3*squareLength + 2*marginSize, 3*squareLength + 2*marginSize);
    board.draw(outSize, _img, marginSize, borderBits);


}


/**
 */
void drawDetectedDiamonds(InputArray _in, OutputArray _out, InputArrayOfArrays _corners,
                         InputArray _ids, Scalar borderColor) {


    CV_Assert(_in.getMat().cols != 0 && _in.getMat().rows != 0 &&
              (_in.getMat().channels() == 1 || _in.getMat().channels() == 3));
    CV_Assert((_corners.total() == _ids.total()) || _ids.total()==0 );

    // calculate colors
    Scalar textColor, cornerColor;
    textColor = cornerColor = borderColor;
    swap(textColor.val[0], textColor.val[1]);     // text color just sawp G and R
    swap(cornerColor.val[1], cornerColor.val[2]); // corner color just sawp G and B

    _out.create(_in.size(), CV_8UC3);
    Mat outImg = _out.getMat();
    if (_in.getMat().channels()==3)
        _in.getMat().copyTo(outImg);
    else
        cvtColor(_in.getMat(), outImg, COLOR_GRAY2BGR);

    int nMarkers = (int)_corners.total();
    for (int i = 0; i < nMarkers; i++) {
        Mat currentMarker = _corners.getMat(i);
        CV_Assert(currentMarker.total()==4 && currentMarker.type()==CV_32FC2);

        // draw marker sides
        for (int j = 0; j < 4; j++) {
            Point2f p0, p1;
            p0 = currentMarker.ptr<Point2f>(0)[j];
            p1 = currentMarker.ptr<Point2f>(0)[(j + 1) % 4];
            line(outImg, p0, p1, borderColor, 1);
        }

        // draw first corner mark
        rectangle(outImg, currentMarker.ptr<Point2f>(0)[0] - Point2f(3, 3),
                  currentMarker.ptr<Point2f>(0)[0] + Point2f(3, 3), cornerColor, 1, LINE_AA);

        // draw ID
        if (_ids.total() != 0) {
            Point2f cent(0, 0);
            for (int p = 0; p < 4; p++)
                cent += currentMarker.ptr<Point2f>(0)[p];
            cent = cent / 4.;
            stringstream s;
            s << "id=" << _ids.getMat().ptr<Vec4i>(0)[i];
            putText(outImg, s.str(), cent, FONT_HERSHEY_SIMPLEX, 0.5, textColor, 2);
        }
    }

}



}
}
