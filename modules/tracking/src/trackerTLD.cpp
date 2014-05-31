/*///////////////////////////////////////////////////////////////////////////////////////
 //
 //  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 //
 //  By downloading, copying, installing or using the software you agree to this license.
 //  If you do not agree to this license, do not download, install,
 //  copy or use the software.
 //
 //
 //                           License Agreement
 //                For Open Source Computer Vision Library
 //
 // Copyright (C) 2013, OpenCV Foundation, all rights reserved.
 // Third party copyrights are property of their respective owners.
 //
 // Redistribution and use in source and binary forms, with or without modification,
 // are permitted provided that the following conditions are met:
 //
 //   * Redistribution's of source code must retain the above copyright notice,
 //     this list of conditions and the following disclaimer.
 //
 //   * Redistribution's in binary form must reproduce the above copyright notice,
 //     this list of conditions and the following disclaimer in the documentation
 //     and/or other materials provided with the distribution.
 //
 //   * The name of the copyright holders may not be used to endorse or promote products
 //     derived from this software without specific prior written permission.
 //
 // This software is provided by the copyright holders and contributors "as is" and
 // any express or implied warranties, including, but not limited to, the implied
 // warranties of merchantability and fitness for a particular purpose are disclaimed.
 // In no event shall the Intel Corporation or contributors be liable for any direct,
 // indirect, incidental, special, exemplary, or consequential damages
 // (including, but not limited to, procurement of substitute goods or services;
 // loss of use, data, or profits; or business interruption) however caused
 // and on any theory of liability, whether in contract, strict liability,
 // or tort (including negligence or otherwise) arising in any way out of
 // the use of this software, even if advised of the possibility of such damage.
 //
 //M*/

#include "precomp.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc.hpp"
#include "time.h"
#include <algorithm>
#include <limits.h>
#include "TLD.hpp"
#include "opencv2/highgui.hpp"

#define HOW_MANY_CLASSIFIERS 49
#define THETA_NN 0.5
#define CORE_THRESHOLD 0.5
#define NEG_EXAMPLES_IN_INIT_MODEL 300

using namespace cv;

/*
 * FIXME(optimize):
 * TODO
 *   detector --> notify Vadim
 *   TrackerMadianFlow
 *   finish learning
*/
/*ask Kalal: 
 * ./bin/example_tracking_tracker TLD ../TrackerChallenge/test.avi 0 5,110,25,130 > out.txt
 *
 *  init_model:negative_patches  -- all?
 *  posterior: 0/0
 *  sampling: how many base classifiers?
 *  initial model: why 20
 *  scanGrid low overlap
 *  rotated rect in initial model
 */

namespace cv
{

class Data : public TrackerTLD::Private{
public:
    Data(Rect2d initBox);
    void setConfident(bool confident_in){confident=confident_in;}
    bool getCofident(){return confident;}
    void setFailedLastTime(bool failedLastTime_in){failedLastTime=failedLastTime_in;}
    bool getFailedLastTime(){return failedLastTime;}
    Size getMinSize(){return minSize;}
private:
    bool confident;
    bool failedLastTime;
    Size minSize;
};

class TrackerTLDModel;

class TLDDetector : public TrackerTLD::Private{
public:
    TLDDetector(const TrackerTLD::Params& params,Ptr<TrackerModel>model_in):model(model_in),params_(params){}
    ~TLDDetector(){}
    static void generateScanGrid(int rows,int cols,Size initBox,std::vector<Rect2d>& res);
    bool detect(const Mat& img,Rect2d& res);
    bool getNextRect(Rect2d& rect_out,bool& isObject_out,bool& shouldBeIntegrated_out,bool reset=false);
    /*bool detect(const Mat& img,Rect2d& res,std::vector<Rect2d>& rect_out,std::vector<bool>& isObject_out,
            std::vector<bool>& shouldBeIntegrated_out,bool reset=false);*/
protected:
    void computeIntegralImages(Mat& img,Mat_<unsigned int>& intImgP,Mat_<unsigned int>& intImgP2);
    bool patchVariance(Mat_<unsigned int>& intImgP,Mat_<unsigned int>& intImgP2,double originalVariance,Point pt,Size size);
    bool ensembleClassifier(uchar* data,int rowstep);
    TrackerTLD::Params params_;
    Ptr<TrackerModel> model;
};

class Pexpert{
public:
    Pexpert(const Mat& img,Rect2d& resultBox,const TLDDetector* detector,TrackerTLD::Params params,Size initSize):img_(img),resultBox_(resultBox),
        detector_(detector),params_(params),initSize_(initSize){}
    bool operator()(Rect2d box){return false;}
    int additionalExamples(std::vector<Mat_<uchar> >& examples);
protected:
    RNG rng;
    Mat img_;
    Size initSize_;
    const TLDDetector* detector_;
    Rect2d resultBox_;
    TrackerTLD::Params params_;
};

class Nexpert{
public:
    Nexpert(const Mat& img,Rect2d& resultBox,const TLDDetector* detector,TrackerTLD::Params params):img_(img),resultBox_(resultBox),
        detector_(detector),params_(params){}
    bool operator()(Rect2d box);
    int additionalExamples(std::vector<Mat_<uchar> >& examples){return 0;}
protected:
    Mat img_;
    const TLDDetector* detector_;
    Rect2d resultBox_;
    TrackerTLD::Params params_;
};

template <class T,class Tparams>
class TrackerProxyImpl : public TrackerProxy{
public:
    TrackerProxyImpl(Tparams params=Tparams()):params_(params){}
    bool init( const Mat& image, const Rect2d& boundingBox ){
        trackerPtr=Ptr<T>(new T(params_));
        return trackerPtr->init(image,boundingBox);
    }
    bool update( const Mat& image,Rect2d& boundingBox){
        return false;//FIXME
        return trackerPtr->update(image,boundingBox);
    }
private:
    Ptr<T> trackerPtr;
    Tparams params_;
    Rect2d boundingBox_;
};

class TrackerTLDModel : public TrackerModel{
 public:
  TrackerTLDModel(TrackerTLD::Params params,const Mat& image, const Rect2d& boundingBox,Size minSize);
  Rect2d getBoundingBox(){return boundingBox_;}
  void setBoudingBox(Rect2d boundingBox){boundingBox_=boundingBox;}
  double getOriginalVariance(){return originalVariance_;}
  std::vector<TLDEnsembleClassifier>* getClassifiers(){return &classifiers;}
  double Sr(const Mat_<uchar> patch);
  double Sc(const Mat_<uchar> patch);
  void integrate(Mat& img,Mat& imgBlurred,Rect2d box,bool isPositive);
  void integrate(Mat_<uchar> e,bool isPositive);
  Size getMinSize(){return minSize_;}
 protected:
  void modelEstimationImpl( const std::vector<Mat>& /*responses*/ ){}
  void modelUpdateImpl(){}
  Rect2d boundingBox_;
  double originalVariance_;
  Size minSize_;
  std::vector<Mat_<uchar> > positiveExamples,negativeExamples;
  RNG rng;
  std::vector<TLDEnsembleClassifier> classifiers;
};


TrackerTLD::Params::Params(){
}

void TrackerTLD::Params::read( const cv::FileNode& fn ){
}

void TrackerTLD::Params::write( cv::FileStorage& fs ) const{
}

TrackerTLD::TrackerTLD( const TrackerTLD::Params &parameters) :
    params( parameters ){
  isInit = false;
  privateInfo.push_back(Ptr<TrackerProxyImpl<TrackerMedianFlow,TrackerMedianFlow::Params> >(
              new TrackerProxyImpl<TrackerMedianFlow,TrackerMedianFlow::Params>()));
}

TrackerTLD::~TrackerTLD(){
}

void TrackerTLD::read( const cv::FileNode& fn )
{
  params.read( fn );
}

void TrackerTLD::write( cv::FileStorage& fs ) const
{
  params.write( fs );
}

bool TrackerTLD::initImpl(const Mat& image, const Rect2d& boundingBox ){
    Mat image_gray;
    cvtColor( image, image_gray, COLOR_BGR2GRAY );
    Data* data=new Data(boundingBox);
    model=Ptr<TrackerTLDModel>(new TrackerTLDModel(params,image_gray,boundingBox,data->getMinSize()));
    printf("datadata->getMinSize());=%dx%d\n",data->getMinSize().width,data->getMinSize().height);
    TLDDetector* detector=new TLDDetector(params,model);
    ((TrackerProxy*)static_cast<Private*>(privateInfo[0]))->init(image,boundingBox);
    data->setConfident(false);
    data->setFailedLastTime(false);

    privateInfo.push_back(Ptr<TLDDetector>(detector));
    privateInfo.push_back(Ptr<Data>(data));
    return true;
}

bool TrackerTLD::updateImpl(const Mat& image, Rect2d& boundingBox){
    Mat image_gray;
    cvtColor( image, image_gray, COLOR_BGR2GRAY );
    TrackerTLDModel* tldModel=((TrackerTLDModel*)static_cast<TrackerModel*>(model));
    TrackerProxy* trackerProxy=(TrackerProxy*)static_cast<Private*>(privateInfo[0]);
    TLDDetector* detector=((TLDDetector*)static_cast<TrackerTLD::Private*>(privateInfo[1]));
    Data* data=((Data*)static_cast<TrackerTLD::Private*>(privateInfo[2]));
    Mat_<uchar> standardPatch(15,15);
    //best overlap around 92%

    Rect2d tmpCandid=boundingBox;
    std::vector<Rect2d> candidates;
    std::vector<double> candidatesRes;
    bool trackerNeedsReInit=false;
    for(int i=0;i<2;i++){
        if(((i==0)&&!(data->getFailedLastTime())&&trackerProxy->update(image,tmpCandid)) || 
                ((i==1)&&(detector->detect(image_gray,tmpCandid)))){
            candidates.push_back(tmpCandid);
            resample(image_gray,tmpCandid,standardPatch);
            candidatesRes.push_back(tldModel->Sc(standardPatch));
        }else{
            if(i==0){
                trackerNeedsReInit=true;
            }
        }
    }
    printf("candidates.size()=%d\n",candidates.size());
    std::vector<double>::iterator it;
    if((it=std::max_element(candidatesRes.begin(),candidatesRes.end()))==candidatesRes.end()){
        data->setConfident(false);
        data->setFailedLastTime(true);
        return false;
    }else{
        boundingBox=candidates[it-candidatesRes.begin()];
        data->setFailedLastTime(false);
        if(trackerNeedsReInit || it!=candidatesRes.begin()){
            trackerProxy->init(image,boundingBox);
        }
    }

    if(*it > CORE_THRESHOLD){
        data->setConfident(true);
    }

    if(false && data->getCofident()){
        Pexpert pExpert(image_gray,boundingBox,detector,params,data->getMinSize());
        Nexpert nExpert(image_gray,boundingBox,detector,params);
        bool isObject,shouldBeIntegrated,expertResult;
        std::vector<Mat_<uchar> > examples;
        examples.reserve(100);
        Rect2d rect;
        detector->getNextRect(rect,isObject,shouldBeIntegrated,true);
        do{
            if(isObject){
                expertResult=nExpert(rect);
            }else{
                expertResult=pExpert(rect);
            }
            if(shouldBeIntegrated || (expertResult!=isObject)){
                tldModel->integrate(image_gray,image_gray,rect,expertResult);
            }
        }while(detector->getNextRect(rect,isObject,shouldBeIntegrated));
        pExpert.additionalExamples(examples);
        for(int i=0;i<examples.size();i++){
            tldModel->integrate(examples[i],true);
        }
        examples.clear();
        nExpert.additionalExamples(examples);
        for(int i=0;i<examples.size();i++){
            tldModel->integrate(examples[i],false);
        }
    }

    return true;
}

TrackerTLDModel::TrackerTLDModel(TrackerTLD::Params params,const Mat& image, const Rect2d& boundingBox,Size minSize):minSize_(minSize){
    boundingBox_=boundingBox;
    originalVariance_=variance(image(boundingBox));
    std::vector<Rect2d> closest(10),scanGrid;

    TLDDetector::generateScanGrid(image.rows,image.cols,minSize,scanGrid);
    getClosestN(scanGrid, boundingBox,10,closest);

    Mat image_blurred;
    Mat_<uchar> blurredPatch(minSize);
    GaussianBlur(image,image_blurred,Size(3,3),0.0);
    for(int i=0;i<HOW_MANY_CLASSIFIERS;i++){
        classifiers.push_back(TLDEnsembleClassifier(i+1,minSize));
    }

    positiveExamples.reserve(200);
    Point2f center;
    Size2f size;
    for(int i=0;i<closest.size();i++){
        for(int j=0;j<20;j++){
            Mat_<uchar> standardPatch(15,15);
            center.x=closest[i].x+closest[i].width*(0.5+rng.uniform(-0.01,0.01));
            center.y=closest[i].y+closest[i].height*(0.5+rng.uniform(-0.01,0.01));
            size.width=closest[i].width*rng.uniform((double)0.99,(double)1.01);
            size.height=closest[i].height*rng.uniform((double)0.99,(double)1.01);
            float angle=rng.uniform((double)-10.0,(double)10.0);

            resample(image,RotatedRect(center,size,angle),standardPatch);
            for(int y=0;y<standardPatch.rows;y++){
                for(int x=0;x<standardPatch.cols;x++){
                    standardPatch(x,y)+=rng.gaussian(5.0);
                }
            }
            positiveExamples.push_back(standardPatch);

            resample(image_blurred,RotatedRect(center,size,angle),blurredPatch);
            for(int k=0;k<classifiers.size();k++){
                classifiers[k].integrate(blurredPatch,true);
            }
        }
    }

    negativeExamples.clear();
    negativeExamples.reserve(NEG_EXAMPLES_IN_INIT_MODEL);
    std::vector<int> indices;
    indices.reserve(NEG_EXAMPLES_IN_INIT_MODEL);
    while(negativeExamples.size()<NEG_EXAMPLES_IN_INIT_MODEL){
        int i=rng.uniform((int)0,(int)scanGrid.size());
        if(std::find(indices.begin(),indices.end(),i)==indices.end() && overlap(boundingBox,scanGrid[i])<0.2){
            Mat_<uchar> standardPatch(15,15);
            resample(image,scanGrid[i],standardPatch);
            negativeExamples.push_back(standardPatch);

            resample(image_blurred,scanGrid[i],blurredPatch);
            for(int k=0;k<classifiers.size();k++){
                classifiers[k].integrate(blurredPatch,false);
            }
        }
    }
    printf("positive patches: %d\nnegative patches: %d\n",positiveExamples.size(),negativeExamples.size());
}

void TLDDetector::generateScanGrid(int rows,int cols,Size initBox,std::vector<Rect2d>& res){
    res.clear();
    //scales step: 1.2; hor step: 10% of width; verstep: 10% of height; minsize: 20pix
    for(double h=initBox.height, w=initBox.width;h<cols && w<rows;){
        for(double x=0;(x+w)<=(cols-1.0);x+=(0.1*w)){
            for(double y=0;(y+h)<=(rows-1.0);y+=(0.1*h)){
                res.push_back(Rect2d(x,y,w,h));
            }
        }
        if(h<=initBox.height){
            h/=1.2; w/=1.2;
            if(h<20 || w<20){
                h=initBox.height*1.2; w=initBox.width*1.2;
                CV_Assert(h>initBox.height || w>initBox.width);
            }
        }else{
            h*=1.2; w*=1.2;
        }
    }
    printf("%d rects in res\n",res.size());
}

bool TLDDetector::detect(const Mat& img,Rect2d& res){
    int remains=0;
    Mat imgBlurred;
    TrackerTLDModel* tldModel=((TrackerTLDModel*)static_cast<TrackerModel*>(model));
    Size initSize=tldModel->getMinSize();

    Mat resized_img,blurred_img;
    Mat_<uchar> standardPatch(15,15);
    img.copyTo(resized_img);
    std::vector<TLDEnsembleClassifier>* classifiers=tldModel->getClassifiers();
    double originalVariance=tldModel->getOriginalVariance();;
    int dx=initSize.width/10,dy=initSize.height/10;
    Size2d size=img.size();
    double scale=1.0;
    int total=0,pass=0;
    double tmp=0,maxSc=-5.0;
    Rect2d maxScRect;
    START_TICK("detector");
    do{
        GaussianBlur(resized_img,blurred_img,Size(3,3),0.0);
        Mat_<unsigned int> intImgP(resized_img.rows,resized_img.cols),intImgP2(resized_img.rows,resized_img.cols);
        computeIntegralImages(resized_img,intImgP,intImgP2);

        for(int i=0;i<cvFloor((0.0+resized_img.cols-initSize.width)/dx);i++){
            for(int j=0;j<cvFloor((0.0+resized_img.rows-initSize.height)/dy);j++){
                total++;
                if(!patchVariance(intImgP,intImgP2,originalVariance,Point(dx*i,dy*j),initSize)){
                    continue;
                }
                if(!ensembleClassifier(&resized_img.at<uchar>(dy*j,dx*i),resized_img.step[0])){
                    continue;
                }
                pass++;

                resample(resized_img,Rect2d(Point(dx*i,dy*j),initSize),standardPatch);
                tmp=tldModel->Sr(standardPatch);
                bool isObject=(tmp>THETA_NN);
                //shouldBeIntegrated[i]=(abs(tmp-THETA_NN)<0.1);
                if(!isObject){
                    continue;
                }
                tmp=tldModel->Sc(standardPatch);
                if(tmp>maxSc){
                    maxSc=tmp;
                    maxScRect=Rect2d(dx*i*scale,dy*j*scale,initSize.width*scale,initSize.height*scale);
                }
            }
        }

        size.width/=1.2;
        size.height/=1.2;
        scale*=1.2;
        resize(img,resized_img,size);
    }while(size.width>=initSize.width && size.height>=initSize.height);
    END_TICK("detector");

    printf("%d after ensemble\n",pass);
    if(maxSc<0){
        return false;
    }
    res=maxScRect;

    return true;
}

bool TLDDetector::getNextRect(Rect2d& rect_out,bool& isObject_out,bool& shouldBeIntegrated_out,bool reset){
    /*if(reset){
        scanGridPos=0;
    }
    if(scanGridPos>=isObject.size()){
        return false;
    }
    rect_out=scanGrid[scanGridPos];
    isObject_out=isObject[scanGridPos];
    shouldBeIntegrated_out=shouldBeIntegrated[scanGridPos];
    scanGridPos++;*/
    return true;
}

void TLDDetector::computeIntegralImages(Mat& img,Mat_<unsigned int>& intImgP,Mat_<unsigned int>& intImgP2){
    intImgP(0,0)=img.at<uchar>(0,0);
    for(int j=1;j<intImgP.cols;j++){intImgP(0,j)=intImgP(0,j-1)+img.at<uchar>(0,j);}
    for(int i=1;i<intImgP.rows;i++){intImgP(i,0)=intImgP(i-1,0)+img.at<uchar>(i,0);}
    for(int i=1;i<intImgP.rows;i++){for(int j=1;j<intImgP.cols;j++){
            intImgP(i,j)=intImgP(i,j-1)-intImgP(i-1,j-1)+intImgP(i-1,j)+img.at<uchar>(i,j);}}

    unsigned int p;
    p=img.at<uchar>(0,0);intImgP2(0,0)=p*p;
    for(int j=1;j<intImgP2.cols;j++){p=img.at<uchar>(0,j);intImgP2(0,j)=intImgP2(0,j-1)+p*p;}
    for(int i=1;i<intImgP2.rows;i++){p=img.at<uchar>(i,0);intImgP2(i,0)=intImgP2(i-1,0)+p*p;}
    for(int i=1;i<intImgP2.rows;i++){for(int j=1;j<intImgP2.cols;j++){p=img.at<uchar>(i,j);
            intImgP2(i,j)=intImgP2(i,j-1)-intImgP2(i-1,j-1)+intImgP2(i-1,j)+p*p;}}
}

bool TLDDetector::patchVariance(Mat_<unsigned int>& intImgP,Mat_<unsigned int>& intImgP2,double originalVariance,Point pt,Size size){
    return variance(intImgP,intImgP2,Rect(pt.x,pt.y,size.width,size.height)) >= 0.5*originalVariance;
}

bool TLDDetector::ensembleClassifier(uchar* data,int rowstep){
    TrackerTLDModel* tldModel=((TrackerTLDModel*)static_cast<TrackerModel*>(model));
    std::vector<TLDEnsembleClassifier>* classifiers=tldModel->getClassifiers();
    double p=0;
    for(int k=0;k<classifiers->size();k++){
        p+=(*classifiers)[k].posteriorProbability(data,rowstep);
    }
    p/=classifiers->size();
    return (p>0.5);
}

double TrackerTLDModel::Sr(const Mat_<uchar> patch){
    double splus=0.0;
    for(int i=0;i<positiveExamples.size();i++){
        splus=MAX(splus,0.5*(NCC(positiveExamples[i],patch)+1.0));
    }
    double sminus=0.0;
    for(int i=0;i<negativeExamples.size();i++){
        sminus=MAX(sminus,0.5*(NCC(negativeExamples[i],patch)+1.0));
    }
    if(splus+sminus==0.0){
        return 0.0;
    }
    return splus/(sminus+splus);
}

double TrackerTLDModel::Sc(const Mat_<uchar> patch){
    double splus=0.0;
    for(int i=0;i<((positiveExamples.size()+1)/2);i++){
        splus=MAX(splus,0.5*(NCC(positiveExamples[i],patch)+1.0));
    }
    double sminus=0.0;
    for(int i=0;i<negativeExamples.size();i++){
        sminus=MAX(sminus,0.5*(NCC(negativeExamples[i],patch)+1.0));
    }
    if(splus+sminus==0.0){
        return 0.0;
    }
    return splus/(sminus+splus);
}

void TrackerTLDModel::integrate(Mat& img,Mat& imgBlurred,Rect2d box,bool isPositive){
    Mat_<uchar> standardPatch(15,15),blurredPatch(15,15);
    resample(img,box,standardPatch);
    if(isPositive){
        positiveExamples.push_back(standardPatch);
    }else{
        negativeExamples.push_back(standardPatch);
    }

    resample(imgBlurred,box,blurredPatch);
    for(int i=0;i<classifiers.size();i++){
        classifiers[i].integrate(blurredPatch,isPositive);
    }
}
void TrackerTLDModel::integrate(Mat_<uchar> e,bool isPositive){
    if(isPositive){
        positiveExamples.push_back(e);
    }else{
        negativeExamples.push_back(e);
    }
    for(int i=0;i<classifiers.size();i++){
        classifiers[i].integrate(e,isPositive);
    }
}

int Pexpert::additionalExamples(std::vector<Mat_<uchar> >& examples){
    examples.clear();
    examples.reserve(100);
    std::vector<Rect2d> closest,scanGrid;
    closest.reserve(10);
    TLDDetector::generateScanGrid(img_.rows,img_.cols,initSize_,scanGrid);
    getClosestN(scanGrid,resultBox_,10,closest);

    Point2f center;
    Size2f size;
    for(int i=0;i<closest.size();i++){
        for(int j=0;j<10;j++){
            Mat_<uchar> standardPatch(15,15);
            center.x=closest[i].x+closest[i].width*(0.5+rng.uniform(-0.01,0.01));
            center.y=closest[i].y+closest[i].height*(0.5+rng.uniform(-0.01,0.01));
            size.width=closest[i].width*rng.uniform((double)0.99,(double)1.01);
            size.height=closest[i].height*rng.uniform((double)0.99,(double)1.01);
            float angle=rng.uniform((double)-5.0,(double)5.0);

            resample(img_,RotatedRect(center,size,angle),standardPatch);
            for(int y=0;y<standardPatch.rows;y++){
                for(int x=0;x<standardPatch.cols;x++){
                    standardPatch(x,y)+=rng.gaussian(5.0);
                }
            }
            examples.push_back(standardPatch);
        }
    }
    return 0;
}

bool Nexpert::operator()(Rect2d box){
    if(overlap(resultBox_,box)<0.2){
        return false;
    }
    return true;
}

Data::Data(Rect2d initBox){
    double minDim=0;
    if((minDim=MIN(initBox.width,initBox.height))<20){
        printf("initial box has size %dx%d, while both dimensions should be no less than %d\n",(int)initBox.width,(int)initBox.height,20);
        exit(EXIT_FAILURE);
    }
    minSize.width=initBox.width*20.0/minDim;
    minSize.height=initBox.height*20.0/minDim;
    printf("minSize= %dx%d\n",minSize.width,minSize.height);
}

} /* namespace cv */
