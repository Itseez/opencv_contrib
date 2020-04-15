// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

#include "test_precomp.hpp"
#include "opencv2/imgcodecs.hpp"

using namespace cv;
using namespace std;
namespace opencv_test { namespace {

TEST (TextDetectionSWT, accuracy_light_on_dark) {
    const string dataPath = cvtest::findDataFile("cv/mser/mser_test.png");
    Mat image = imread(dataPath, IMREAD_COLOR);
    vector<Rect> components;
    detectTextSWT(image, components, false);
    /* all 5 letter candidates should be identified (R9888) */
    EXPECT_EQ((unsigned) 5, components.size());
}

TEST (TextDetectionSWT, accuracy_dark_on_light) {
    const string dataPath = cvtest::findDataFile("cv/mser/mser_test2.png");
    Mat image = imread(dataPath, IMREAD_COLOR);
    vector<Rect> components;
    detectTextSWT(image, components, true);
    /* all 3 letter candidates should be identified 2, 5, 8 */
    EXPECT_EQ((unsigned) 3, components.size());
}

TEST (TextDetectionSWT, accuracy_handwriting) {
    const string dataPath = cvtest::findDataFile("cv/cloning/Mixed_Cloning/source1.png");
    Mat image = imread(dataPath, IMREAD_COLOR);
    vector<Rect> components;
    detectTextSWT(image, components, true);
    /* Handwritten Text is generally more difficult to detect using SWT algorithm due to high variation in stroke width. */
    EXPECT_LT((unsigned) 11, components.size());
    /* Although the text contains 15 characters, the current implementation of algorithm outputs 14, including three wrong guesses. So, we check at least 11 (14 - 3) letters are detected.*/
}

TEST (TextDetectionSWT, regression_natural_scene) {
    const string dataPath = cvtest::findDataFile("cv/shared/box_in_scene.png");

    Mat image = imread(dataPath, IMREAD_COLOR);
    vector<Rect> light_components;
    detectTextSWT(image, light_components, false);
    EXPECT_EQ((unsigned) 81, light_components.size());

    vector<Rect> dark_components;
    detectTextSWT(image, dark_components, true);
    EXPECT_EQ((unsigned) 17, dark_components.size());
    /* Verifies that both modes of algorithm run on natural scenes */
}

TEST (TextDetectionSWT, accuracy_chaining) {
    const string dataPath = cvtest::findDataFile("cv/mser/mser_test.png");
    Mat image = imread(dataPath, IMREAD_COLOR);
    vector<Rect> components;
    Mat out( image.size(), CV_8UC3 );
    vector<Rect> chains;
    detectTextSWT(image, components, false, out, chains);
    Rect chain = chains[0];
    /* Since the word is already segmented and cropped, most of the area is covered by text. It confirms that chaining works. */
    EXPECT_LT(0.95 * image.rows * image.cols, chain.area());
}

}} // namespace
