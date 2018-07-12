#include <iostream>
#include <string>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/ximgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/epf.hpp>


using namespace cv;


int main(int argc, char **argv)
{
    cv::CommandLineParser parser(argc, argv,
        "{help h ? |     | help message}"
        "{@image   |     | Image filename to process }"
    );
    if (parser.has("help") || !parser.has("image"))
    {
        parser.printMessage();
        return 0;
		}

	// Load image from first parameter
	std::string filename = parser.get<string>("@image");
	Mat image = imread(filename, 1), res;

	if (!image.data)
	{
		std::cerr << "No image data at " << filename << std::endl;
		throw;
	}

	// Before filtering
	imshow("Original image", image);
	waitKey(0);

	// Initialize filter. Kernel size 5x5, threshold 20
	ximgproc::edgepreservingFilter(&image, &res, 5, 20);

	// After filtering
	imshow("Filtered image", res);
	waitKey(0);

	return 0;
}
