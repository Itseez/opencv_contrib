#include "opencv2/xphoto.hpp"

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc/types_c.h"


const char* keys =
{
    "{i || input image name}"
    "{m || mask image name}"
    "{o || output image name}"
};

int main( int argc, const char** argv )
{
    bool printHelp = ( argc == 1 );
    printHelp = printHelp || ( argc == 2 && std::string(argv[1]) == "--help" );
    printHelp = printHelp || ( argc == 2 && std::string(argv[1]) == "-h" );

    if ( printHelp )
    {
        printf("\nThis sample demonstrates shift-map image inpainting\n"
            "Call:\n"
            "    inpainting -i=<string> -m=<string> [-o=<string>]\n\n");
        return 0;
    }

    cv::CommandLineParser parser(argc, argv, keys);
    if ( !parser.check() )
    {
        parser.printErrors();
        return -1;
    }

    std::string inFilename = parser.get<std::string>("i");
    std::string maskFilename = parser.get<std::string>("m");
    std::string outFilename = parser.get<std::string>("o");

    cv::Mat src = cv::imread(inFilename, -1);
    if ( src.empty() )
    {
        printf( "Cannot read image file: %s\n", inFilename.c_str() );
        return -1;
    }

    cv::cvtColor(src, src, CV_RGB2Lab);

    cv::Mat mask = cv::imread(maskFilename, 0);
    if ( mask.empty() )
    {
        printf( "Cannot read image file: %s\n", maskFilename.c_str() );
        return -1;
    }

    cv::Mat res(src.size(), src.type());
    cv::xphoto::inpaint( src, mask, res, cv::xphoto::INPAINT_SHIFTMAP );
    cv::cvtColor(res, res, CV_Lab2RGB);

    if ( outFilename == "" )
    {
        cv::namedWindow("inpainting result", 1);
        cv::imshow("inpainting result", res);

        cv::waitKey(0);
    }
    else
        cv::imwrite(outFilename, res);

    return 0;
}