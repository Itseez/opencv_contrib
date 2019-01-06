// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

#ifndef OPENCV_QUALITY_QUALITYMSE_HPP
#define OPENCV_QUALITY_QUALITYMSE_HPP

#include <vector>
#include "QualityBase.hpp"

namespace cv {
	namespace quality {

		/**
        @brief Full reference mean square error algorithm  https://en.wikipedia.org/wiki/Mean_squared_error
        */
		class CV_EXPORTS QualityMSE : public QualityBase {
		public:

            /** @brief */
            CV_WRAP cv::Scalar compute( InputArrayOfArrays cmpImgs ) CV_OVERRIDE;

            /** @brief Implements Algorithm::empty()  */
            CV_WRAP bool empty() const CV_OVERRIDE { return _qualityMaps.empty() && _refImgs.empty(); }

            /** @brief Implements Algorithm::clear()  */
            CV_WRAP void clear() CV_OVERRIDE { _qualityMaps.clear(); _refImgs.clear(); Algorithm::clear(); }

            /** @brief Returns pointer to output quality maps images that were generated during computation, if supported by the algorithm.  */
            CV_WRAP const std::vector<quality_map_type>& getQualityMaps() const CV_OVERRIDE { return _qualityMaps; }

            /**
            @brief Create an object which calculates quality via mean square error
            @param refImgs input image(s) to use as the source for comparison
            */
            CV_WRAP static Ptr<QualityMSE> create(InputArrayOfArrays refImgs);

            /**
            @brief static method for computing quality
            @param refImgs reference image(s)
            @param cmpImgs comparison image(s)
            @param output qualityMaps quality map(s)
            @returns cv::Scalar with per-channel mean square error.  Values range from 0 (best) to potentially max float (worst)
            */
            CV_WRAP static cv::Scalar compute( InputArrayOfArrays refImgs, InputArrayOfArrays cmpImgs, OutputArrayOfArrays qualityMaps );

        protected:

            /** @brief Reference images, converted to internal mat type */
            std::vector<UMat> _refImgs;

            /** @brief Quality maps generated by algorithm */
            std::vector<quality_map_type> _qualityMaps;

            /**
            @brief Base class constructor
            @param refImgs vector of reference images, converted to internal type
            */
            QualityMSE(std::vector<UMat> refImgs)
                : _refImgs(std::move(refImgs))
            {}

		};	// QualityMSE
    }
}
#endif