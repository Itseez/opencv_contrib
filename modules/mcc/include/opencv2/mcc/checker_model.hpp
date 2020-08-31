// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

/*
 * MIT License
 *
 * Copyright (c) 2018 Pedro Diamel Marrero Fernández
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __OPENCV_MCC_CHECKER_MODEL_HPP__
#define __OPENCV_MCC_CHECKER_MODEL_HPP__
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
namespace cv
{
namespace mcc
{

//! @addtogroup mcc
//! @{

/** TYPECHART
 *
 * \brief enum to hold the type of the checker
 *
 */
enum TYPECHART
{
    MCC24 = 0, ///< Standard Macbeth Chart with 24 squares
    SG140,       ///< DigitalSG with 140 squares
    VINYL18,   ///< DKK color chart with 12 squares and 6 rectangle

};

/** CChecker
 *
 * \brief checker object
 *
 *     This class contains the information about the detected checkers,i.e, their
 *     type, the corners of the chart, the color profile, the cost, centers chart,
 *     etc.
 *
 */

class CV_EXPORTS_W CChecker
{
public:
    CChecker() {}
    virtual ~CChecker() {}
    /** \brief Create a new CChecker object.
    * \return A pointer to the implementation of the CChecker
    */

    CV_WRAP static Ptr<CChecker> create();

public:

    CV_WRAP virtual void setTarget(TYPECHART _target)  = 0;
    CV_WRAP virtual void setBox(std::vector<Point2f> _box) = 0;
    CV_WRAP virtual void setPerPatchCosts(std::vector<Point2f> _patchCosts) = 0;

    // Detected directly by the detector or found using geometry, useful to determine if a patch is occluded
    CV_WRAP virtual void setActualDetectedContours(std::vector<std::vector<Point2f>> detectionType) = 0;
    CV_WRAP virtual void setChartsRGB(Mat _chartsRGB) = 0;
    CV_WRAP virtual void setChartsYCbCr(Mat _chartsYCbCr) = 0;
    CV_WRAP virtual void setActualChartsColors(Mat _actualChartsColors) = 0; // the acutal color profile
    CV_WRAP virtual void setCost(float _cost) = 0;
    CV_WRAP virtual void setCenter(Point2f _center) = 0;
    CV_WRAP virtual void setPatchCenters(std::vector<Point2f> _patchCenters) = 0;
    CV_WRAP virtual void setPatchBoundingBoxes(std::vector<Point2f> _patchBoundingBoxes) = 0;

    CV_WRAP virtual TYPECHART getTarget() = 0;
    CV_WRAP virtual std::vector<Point2f> getBox() = 0;
    CV_WRAP virtual std::vector<Point2f> getPerPatchCosts() = 0;
    CV_WRAP virtual std::vector<std::vector<Point2f>> getActualDetectedContours() = 0;
    CV_WRAP virtual Mat getChartsRGB() = 0;
    CV_WRAP virtual Mat getChartsYCbCr() = 0;
    CV_WRAP virtual Mat getActualChartsColors() = 0; // the acutal color profile
    CV_WRAP virtual float getCost() = 0;
    CV_WRAP virtual Point2f getCenter() = 0;
    CV_WRAP virtual std::vector<Point2f> getPatchCenters() = 0;
    CV_WRAP virtual std::vector<Point2f> getPatchBoundingBoxes(float sideRatio=0.5) =0;


    CV_WRAP virtual bool calculate() = 0; ///< Return false if some error occured, true otherwise
};

/** \brief checker draw
 *
 *  This class contains the functions for drawing a detected chart.  This class
 *  expects a pointer to the checker which will be drawn by this object in the
 *  constructor and then later on whenever the draw function is called the
 *  checker will be drawn. Remember that it is not possible to change the
 *  checkers which will be draw by a given object, as it is decided in the
 *  constructor itself. If you want to draw some other object you can create a
 *  new CCheckerDraw instance.
 *
 *  The reason for this type of design is that in some videos we can assume that
 *  the checker is always in the same position, even if the image changes, so
 *  the drawing will always take place at the same position.
*/
class CV_EXPORTS_W CCheckerDraw
{

public:
    virtual ~CCheckerDraw() {}
    /** \brief Draws the checker to the given image.
    * \param img image in color space BGR, original image gets modified
    * \param sideRatio ratio between the side length of drawed boxes, and the actual side of squares
    *                  keeping sideRatio = 1.0 is not recommended as the border can be a bit inaccurate
    *                  in detection, for best output keep it at less than 1.0
    * \param drawActualDetection draw the actually detected patch,i.e,, the non occuled ones,
    *                            useful for debugging, default (false)
    * \return void
    */
    CV_WRAP virtual void draw(InputOutputArray img, float sideRatio = 0.5,
                              bool drawActualDetection = false) = 0;
    /** \brief Create a new CCheckerDraw object.
    * \param pChecker The checker which will be drawn by this object.
    * \param color The color by with which the squares of the checker
    *              will be drawn
    * \param thickness The thickness with which the sqaures will be
    *                  drawn
    * \return A pointer to the implementation of the CCheckerDraw
    */
    CV_WRAP static Ptr<CCheckerDraw> create(Ptr<CChecker> pChecker,
                                            cv::Scalar color = CV_RGB(0, 250, 0),
                                            int thickness = 2);
};

/** @brief draws a single mcc::ColorChecker on the given image
 *         This is a functional version of the CCheckerDraw class, provided for convenience
 * \param img image in color space BGR, it gets changed by this call
 * \param pChecker pointer to the CChecker, which contains the detection
 * \param color color used for drawing
 * \param thickness thickness of the boxes drawed
 * \param sideRatio ratio between the side length of drawed boxes, and the actual side of squares
 *                  keeping sideRatio = 1.0 is not recommended as the border can be a bit inaccurate
 *                  in detection, for best output keep it at less than 1.0
 * \param drawActualDetection draw the actually detected patch,i.e,, the non occuled ones,
 *                            useful for debugging, default (false)
 */
CV_EXPORTS_W void drawColorChecker(InputOutputArray img, Ptr<CChecker> pChecker,
                                   cv::Scalar color = CV_RGB(0, 255, 0), int thickness = 2, float sideRatio = 0.5,
                                   bool drawActualDetection = false);
//! @} mcc
} // namespace mcc
} // namespace cv

#endif
