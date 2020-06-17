#include "opencv2/mcc.hpp"

namespace cv{
namespace mcc{

/**
  *
  */
DetectorParameters::DetectorParameters()
    : adaptiveThreshWinSizeMin(23),
      adaptiveThreshWinSizeMax(153),
      adaptiveThreshWinSizeStep(10),
      adaptiveThreshConstant(7),
      minContoursAreaRate(0.003),
      confidenceThreshold(0.5),
      minContourSolidity(0.9),
      findCandidatesApproxPolyDPEpsMultiplier(0.05),
      borderWidth(0),
      B0factor(1.25f),
      maxError(1.5f),
      minContourPointsAllowed(4),
      minContourLengthAllowed(100),
      minInterContourDistance(100),
      minInterCheckerDistance(10000),
      minImageSize(2000),
      minGroupSize(4)

{
}


/**
  * @brief Create a new set of DetectorParameters with default values.
  */
Ptr<DetectorParameters> DetectorParameters::create() {
    Ptr<DetectorParameters> params = makePtr<DetectorParameters>();
    return params;
}
} // namespace mcc
} // namespace cv
