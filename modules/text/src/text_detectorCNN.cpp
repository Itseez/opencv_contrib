#include "precomp.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/core.hpp"



#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <algorithm>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>


#ifdef HAVE_CAFFE
#include "caffe/caffe.hpp"
#endif
namespace cv { namespace text {

inline bool fileExists (String filename) {
    std::ifstream f(filename.c_str());
    return f.good();
}

class DeepCNNTextDetectorCaffeImpl: public DeepCNNTextDetector{
protected:


    void process_(Mat inputImage, Mat &outputMat)
    {
        // do forward pass and stores the output in outputMat


#ifdef HAVE_CAFFE
        net_->input_blobs()[0]->Reshape(1, this->inputChannelCount_,this->inputGeometry_.height,this->inputGeometry_.width);
        net_->Reshape();
        float* inputBuffer=net_->input_blobs()[0]->mutable_cpu_data();
        float* inputData=inputBuffer;

        std::vector<Mat> input_channels;
        Mat preprocessed;
        // if the image have multiple color channels the input layer should be populated accordingly
        for (int channel=0;channel < this->inputChannelCount_;channel++){

            cv::Mat netInputWraped(this->inputGeometry_.height, this->inputGeometry_.width, CV_32FC1, inputData);
            input_channels.push_back(netInputWraped);
            //input_data += width * height;
            inputData+=(this->inputGeometry_.height*this->inputGeometry_.width);
        }
        this->preprocess(inputImage,preprocessed);
        split(preprocessed, input_channels);

        //preprocessed.copyTo(netInputWraped);


        this->net_->Forward();
        const float* outputNetData=net_->output_blobs()[0]->cpu_data();
        // const float* outputNetData1=net_->output_blobs()[1]->cpu_data();




        this->outputGeometry_.height = net_->output_blobs()[0]->height();
        this->outputGeometry_.width = net_->output_blobs()[0]->width();
        this->outputChannelCount_ = net_->output_blobs()[0]->channels();
        int outputSz = this->outputChannelCount_ * this->outputGeometry_.height * this->outputGeometry_.width;
        outputMat.create(this->outputGeometry_.height , this->outputGeometry_.width,CV_32FC1);
        float*outputMatData=(float*)(outputMat.data);

        memcpy(outputMatData,outputNetData,sizeof(float)*outputSz);



#endif
    }



#ifdef HAVE_CAFFE
    Ptr<caffe::Net<float> > net_;
#endif
    //Size inputGeometry_;
    int minibatchSz_;//The existence of the assignment operator mandates this to be nonconst
    //int outputSize_;
public:
    DeepCNNTextDetectorCaffeImpl(const DeepCNNTextDetectorCaffeImpl& dn):
        minibatchSz_(dn.minibatchSz_){
        outputGeometry_=dn.outputGeometry_;
        inputGeometry_=dn.inputGeometry_;
        //Implemented to supress Visual Studio warning "assignment operator could not be generated"
#ifdef HAVE_CAFFE
        this->net_=dn.net_;
#endif
    }
    DeepCNNTextDetectorCaffeImpl& operator=(const DeepCNNTextDetectorCaffeImpl &dn)
    {
#ifdef HAVE_CAFFE
        this->net_=dn.net_;
#endif
        this->setPreprocessor(dn.preprocessor_);
        this->inputGeometry_=dn.inputGeometry_;
        this->inputChannelCount_=dn.inputChannelCount_;
        this->outputChannelCount_ = dn.outputChannelCount_;
        // this->minibatchSz_=dn.minibatchSz_;
        //this->outputGeometry_=dn.outputSize_;
        this->preprocessor_=dn.preprocessor_;
        this->outputGeometry_=dn.outputGeometry_;
        return *this;
        //Implemented to supress Visual Studio warning "assignment operator could not be generated"
    }

    DeepCNNTextDetectorCaffeImpl(String modelArchFilename, String modelWeightsFilename,Ptr<ImagePreprocessor> preprocessor, int maxMinibatchSz)
        :minibatchSz_(maxMinibatchSz)
    {

        CV_Assert(this->minibatchSz_>0);
        CV_Assert(fileExists(modelArchFilename));
        CV_Assert(fileExists(modelWeightsFilename));
        CV_Assert(!preprocessor.empty());
        this->setPreprocessor(preprocessor);
#ifdef HAVE_CAFFE
        this->net_.reset(new caffe::Net<float>(modelArchFilename, caffe::TEST));
        CV_Assert(net_->num_inputs()==1);
        CV_Assert(net_->num_outputs()==1);
        CV_Assert(this->net_->input_blobs()[0]->channels()==1
                ||this->net_->input_blobs()[0]->channels()==3);
        //        this->channelCount_=this->net_->input_blobs()[0]->channels();



        this->net_->CopyTrainedLayersFrom(modelWeightsFilename);

        caffe::Blob<float>* inputLayer = this->net_->input_blobs()[0];

        this->inputGeometry_.height = inputLayer->height();
        this->inputGeometry_.width = inputLayer->width();
        this->inputChannelCount_ = inputLayer->channels();
        //this->inputGeometry_.batchSize =1;

        inputLayer->Reshape(this->minibatchSz_,this->inputChannelCount_,this->inputGeometry_.height, this->inputGeometry_.width);
        net_->Reshape();
        this->outputChannelCount_ = net_->output_blobs()[0]->channels();
        //this->outputGeometry_.batchSize =1;
        this->outputGeometry_.height =net_->output_blobs()[0]->height();
        this->outputGeometry_.width = net_->output_blobs()[0]->width();





#else
        CV_Error(Error::StsError,"Caffe not available during compilation!");
#endif
    }


    void detect(InputArray image, OutputArray Bbox_prob)
    {
        Size outSize = Size(this->outputGeometry_.height,outputGeometry_.width);
        Bbox_prob.create(outSize,CV_32F); // dummy initialization is it needed
        Mat outputMat = Bbox_prob.getMat();
        process_(image.getMat(),outputMat);
        //copy back to outputArray
        outputMat.copyTo(Bbox_prob);
    }

    Size getOutputGeometry()
    {
        return this->outputGeometry_;
    }
    Size getinputGeometry()
    {
        return this->inputGeometry_;
    }

    int getMinibatchSize()
    {
        return this->minibatchSz_;
    }

    int getBackend()
    {
        return OCR_HOLISTIC_BACKEND_CAFFE;
    }
    void setPreprocessor(Ptr<ImagePreprocessor> ptr)
    {
        CV_Assert(!ptr.empty());
        preprocessor_=ptr;
    }

    Ptr<ImagePreprocessor> getPreprocessor()
    {
        return preprocessor_;
    }
};


Ptr<DeepCNNTextDetector> DeepCNNTextDetector::create(String archFilename,String weightsFilename,Ptr<ImagePreprocessor> preprocessor,int minibatchSz,int backEnd)
{
    if(preprocessor.empty())
    {
        // create a custom preprocessor with rawval
        Ptr<ImagePreprocessor> preprocessor=ImagePreprocessor::createImageCustomPreprocessor(255);
        // set the mean for the preprocessor

        Mat textbox_mean(1,3,CV_8U);
        textbox_mean.at<uchar>(0,0)=104;
        textbox_mean.at<uchar>(0,1)=117;
        textbox_mean.at<uchar>(0,2)=123;
        preprocessor->set_mean(textbox_mean);
    }
    switch(backEnd){
    case OCR_HOLISTIC_BACKEND_CAFFE:

        return Ptr<DeepCNNTextDetector>(new DeepCNNTextDetectorCaffeImpl(archFilename, weightsFilename,preprocessor, minibatchSz));
        break;
    case OCR_HOLISTIC_BACKEND_NONE:
    default:
        CV_Error(Error::StsError,"DeepCNN::create backend not implemented");
        return Ptr<DeepCNNTextDetector>();
        break;
    }
    return Ptr<DeepCNNTextDetector>();

}


Ptr<DeepCNNTextDetector> DeepCNNTextDetector::createTextBoxNet(String archFilename,String weightsFilename,int backEnd)
{

    // create a custom preprocessor with rawval
    Ptr<ImagePreprocessor> preprocessor=ImagePreprocessor::createImageCustomPreprocessor(255);
    // set the mean for the preprocessor

    Mat textbox_mean(1,3,CV_8U);
    textbox_mean.at<uchar>(0,0)=104;
    textbox_mean.at<uchar>(0,1)=117;
    textbox_mean.at<uchar>(0,2)=123;
    preprocessor->set_mean(textbox_mean);
    switch(backEnd){
    case OCR_HOLISTIC_BACKEND_CAFFE:
        return Ptr<DeepCNNTextDetector>(new DeepCNNTextDetectorCaffeImpl(archFilename, weightsFilename,preprocessor, 100));
        break;
    case OCR_HOLISTIC_BACKEND_NONE:
    default:
        CV_Error(Error::StsError,"DeepCNN::create backend not implemented");
        return Ptr<DeepCNNTextDetector>();
        break;
    }
    return Ptr<DeepCNNTextDetector>();

}

void DeepCNNTextDetector::preprocess(const Mat& input,Mat& output)
{
    Size inputHtWd = Size(this->inputGeometry_.height,this->inputGeometry_.width);
    this->preprocessor_->preprocess(input,output,inputHtWd,this->inputChannelCount_);
}



}  } //namespace text namespace cv

