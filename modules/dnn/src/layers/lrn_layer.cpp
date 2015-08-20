#include "../precomp.hpp"
#include "layers_common.hpp"
#include "lrn_layer.hpp"
#include <opencv2/imgproc.hpp>
#include <algorithm>

namespace cv
{
namespace dnn
{
    LRNLayer::LRNLayer(LayerParams &params)
    {
        String nrmType = params.get<String>("norm_region", "ACROSS_CHANNELS");
        if (nrmType == "ACROSS_CHANNELS")
            type = CHANNEL_NRM;
        else if (nrmType == "WITHIN_CHANNEL")
            type = SPATIAL_NRM;
        else
            CV_Error(cv::Error::StsBadArg, "Unknown region type \"" + nrmType + "\"");

        size = params.get<int>("local_size", 5);
        if (size % 2 != 1 || size <= 0)
            CV_Error(cv::Error::StsBadArg, "LRN layer supports only positive odd values for local_size");

        alpha = params.get<double>("alpha", 1);
        beta = params.get<double>("beta", 0.75);
    }

    void LRNLayer::allocate(const std::vector<Blob*> &inputs, std::vector<Blob> &outputs)
    {
        CV_Assert(inputs.size() == 1);
        outputs.resize(1);

        Vec4i shape = inputs[0]->shape4();
        outputs[0].create(shape);

        shape[0] = 1; //maybe make shape[0] = 1 too
        bufBlob.create(shape);
    }

    void LRNLayer::forward(std::vector<Blob*> &inputs, std::vector<Blob> &outputs)
    {
        Blob &src = *inputs[0];
        Blob &dst = outputs[0];

        switch (type)
        {
        case CHANNEL_NRM:
            channelNoramlization(src, dst);
            break;
        case SPATIAL_NRM:
            spatialNormalization(src, dst);
            break;
        default:
            CV_Error(cv::Error::StsNotImplemented, "Unimplemented mode of LRN layer");
            break;
        }
    }

    void LRNLayer::channelNoramlization(Blob &srcBlob, Blob &dstBlob)
    {
        CV_DbgAssert(srcBlob.ptr() != dstBlob.ptr());

        int num = srcBlob.num();
        int channels = srcBlob.channels();
        int ksize = (size - 1) / 2;

        for (int n = 0; n < num; n++)
        {
            Mat accum = dstBlob.getMat(n, channels-1); //trick for memory saving
            accum.setTo(0);

            for (int cn = 0; cn < std::min(ksize, channels); cn++)
                cv::accumulateSquare(srcBlob.getMat(n, cn), accum);

            for (int cn = 0; cn < channels; cn++)
            {
                if (cn + ksize < channels)
                {
                    cv::accumulateSquare(srcBlob.getMat(n, cn + ksize), accum);
                }

                if (cn - ksize - 1 >= 0)
                {
                    Mat left = srcBlob.getMat(n, cn - ksize - 1);
                    cv::subtract(accum, left.mul(left), accum); //subtractSquare
                }

                Mat dst = dstBlob.getMat(n, cn);
                accum.convertTo(dst, dst.type(), alpha/size, 1);
                cv::pow(dst, beta, dst);
                cv::divide(srcBlob.getMat(n, cn), dst, dst);
            }
        }
    }

    void LRNLayer::spatialNormalization(Blob &srcBlob, Blob &dstBlob)
    {
        int num = srcBlob.num();
        int channels = srcBlob.channels();

        for (int n = 0; n < num; n++)
        {
            for (int cn = 0; cn < channels; cn++)
            {
                Mat src = srcBlob.getMat(n, cn);
                Mat dst = dstBlob.getMat(n, cn);
                uchar *dataDst0 = dst.data;

                cv::pow(srcBlob.getMat(n, cn), 2, dst);
                //TODO: check border type
                cv::boxFilter(dst, dst, dst.depth(), cv::Size(size, size), cv::Point(-1, -1), false, cv::BORDER_CONSTANT);
                dst.convertTo(dst, dst.type(), alpha/(size*size), 1);
                cv::pow(dst, beta, dst);
                cv::divide(src, dst, dst);

                CV_Assert(dataDst0 == dst.data); //debug
            }
        }
    }

}
}
