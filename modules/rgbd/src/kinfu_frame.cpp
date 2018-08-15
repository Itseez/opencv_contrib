// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html

// This code is also subject to the license terms in the LICENSE_KinectFusion.md file found in this module's directory

#include "precomp.hpp"
#include "kinfu_frame.hpp"

namespace cv {
namespace kinfu {

static void computePointsNormals(const cv::kinfu::Intr, float depthFactor, const Depth, Points, Normals );
static Depth pyrDownBilateral(const Depth depth, float sigma);
static void pyrDownPointsNormals(const Points p, const Normals n, Points& pdown, Normals& ndown);

template<int p>
inline float specPow(float x)
{
    if(p % 2 == 0)
    {
        float v = specPow<p/2>(x);
        return v*v;
    }
    else
    {
        float v = specPow<(p-1)/2>(x);
        return v*v*x;
    }
}

template<>
inline float specPow<0>(float /*x*/)
{
    return 1.f;
}

template<>
inline float specPow<1>(float x)
{
    return x;
}

struct RenderInvoker : ParallelLoopBody
{
    RenderInvoker(const Points& _points, const Normals& _normals, Mat_<Vec4b>& _img, Affine3f _lightPose, Size _sz) :
        ParallelLoopBody(),
        points(_points),
        normals(_normals),
        img(_img),
        lightPose(_lightPose),
        sz(_sz)
    { }

    virtual void operator ()(const Range& range) const override
    {
        for(int y = range.start; y < range.end; y++)
        {
            Vec4b* imgRow = img[y];
            const ptype* ptsRow = points[y];
            const ptype* nrmRow = normals[y];

            for(int x = 0; x < sz.width; x++)
            {
                Point3f p = fromPtype(ptsRow[x]);
                Point3f n = fromPtype(nrmRow[x]);

                Vec4b color;

                if(isNaN(p))
                {
                    color = Vec4b(0, 32, 0, 0);
                }
                else
                {
                    const float Ka = 0.3f;  //ambient coeff
                    const float Kd = 0.5f;  //diffuse coeff
                    const float Ks = 0.2f;  //specular coeff
                    const int   sp = 20;  //specular power

                    const float Ax = 1.f;   //ambient color,  can be RGB
                    const float Dx = 1.f;   //diffuse color,  can be RGB
                    const float Sx = 1.f;   //specular color, can be RGB
                    const float Lx = 1.f;   //light color

                    Point3f l = normalize(lightPose.translation() - Vec3f(p));
                    Point3f v = normalize(-Vec3f(p));
                    Point3f r = normalize(Vec3f(2.f*n*n.dot(l) - l));

                    uchar ix = (uchar)((Ax*Ka*Dx + Lx*Kd*Dx*max(0.f, n.dot(l)) +
                                        Lx*Ks*Sx*specPow<sp>(max(0.f, r.dot(v))))*255.f);
                    color = Vec4b(ix, ix, ix, 0);
                }

                imgRow[x] = color;
            }
        }
    }

    const Points& points;
    const Normals& normals;
    Mat_<Vec4b>& img;
    Affine3f lightPose;
    Size sz;
};


void pyrDownPointsNormals(const Points p, const Normals n, Points &pdown, Normals &ndown)
{
    for(int y = 0; y < pdown.rows; y++)
    {
        ptype* ptsRow = pdown[y];
        ptype* nrmRow = ndown[y];
        const ptype* pUpRow0 = p[2*y];
        const ptype* pUpRow1 = p[2*y+1];
        const ptype* nUpRow0 = n[2*y];
        const ptype* nUpRow1 = n[2*y+1];
        for(int x = 0; x < pdown.cols; x++)
        {
            Point3f point = nan3, normal = nan3;

            Point3f d00 = fromPtype(pUpRow0[2*x]);
            Point3f d01 = fromPtype(pUpRow0[2*x+1]);
            Point3f d10 = fromPtype(pUpRow1[2*x]);
            Point3f d11 = fromPtype(pUpRow1[2*x+1]);

            if(!(isNaN(d00) || isNaN(d01) || isNaN(d10) || isNaN(d11)))
            {
                point = (d00 + d01 + d10 + d11)*0.25f;

                Point3f n00 = fromPtype(nUpRow0[2*x]);
                Point3f n01 = fromPtype(nUpRow0[2*x+1]);
                Point3f n10 = fromPtype(nUpRow1[2*x]);
                Point3f n11 = fromPtype(nUpRow1[2*x+1]);

                normal = (n00 + n01 + n10 + n11)*0.25f;
            }

            ptsRow[x] = toPtype(point);
            nrmRow[x] = toPtype(normal);
        }
    }
}

struct PyrDownBilateralInvoker : ParallelLoopBody
{
    PyrDownBilateralInvoker(const Depth& _depth, Depth& _depthDown, float _sigma) :
        ParallelLoopBody(),
        depth(_depth),
        depthDown(_depthDown),
        sigma(_sigma)
    { }

    virtual void operator ()(const Range& range) const override
    {
        float sigma3 = sigma*3;
        const int D = 5;

        for(int y = range.start; y < range.end; y++)
        {
            depthType* downRow = depthDown[y];
            const depthType* srcCenterRow = depth[2*y];

            for(int x = 0; x < depthDown.cols; x++)
            {
                depthType center = srcCenterRow[2*x];

                int sx = max(0, 2*x - D/2), ex = min(2*x - D/2 + D, depth.cols-1);
                int sy = max(0, 2*y - D/2), ey = min(2*y - D/2 + D, depth.rows-1);

                depthType sum = 0;
                int count = 0;

                for(int iy = sy; iy < ey; iy++)
                {
                    const depthType* srcRow = depth[iy];
                    for(int ix = sx; ix < ex; ix++)
                    {
                        depthType val = srcRow[ix];
                        if(abs(val - center) < sigma3)
                        {
                            sum += val; count ++;
                        }
                    }
                }

                downRow[x] = (count == 0) ? 0 : sum / count;
            }
        }
    }

    const Depth& depth;
    Depth& depthDown;
    float sigma;
};


Depth pyrDownBilateral(const Depth depth, float sigma)
{
    Depth depthDown(depth.rows/2, depth.cols/2);

    PyrDownBilateralInvoker pdi(depth, depthDown, sigma);
    Range range(0, depthDown.rows);
    const int nstripes = -1;
    parallel_for_(range, pdi, nstripes);

    return depthDown;
}

struct ComputePointsNormalsInvoker : ParallelLoopBody
{
    ComputePointsNormalsInvoker(const Depth& _depth, Points& _points, Normals& _normals,
                                const Intr::Reprojector& _reproj, float _dfac) :
        ParallelLoopBody(),
        depth(_depth),
        points(_points),
        normals(_normals),
        reproj(_reproj),
        dfac(_dfac)
    { }

    virtual void operator ()(const Range& range) const override
    {
        for(int y = range.start; y < range.end; y++)
        {
            const depthType* depthRow0 = depth[y];
            const depthType* depthRow1 = (y < depth.rows - 1) ? depth[y + 1] : 0;
            ptype    *ptsRow = points[y];
            ptype   *normRow = normals[y];

            for(int x = 0; x < depth.cols; x++)
            {
                depthType d00 = depthRow0[x];
                depthType z00 = d00*dfac;
                Point3f v00 = reproj(Point3f((float)x, (float)y, z00));

                Point3f p = nan3, n = nan3;

                if(x < depth.cols - 1 && y < depth.rows - 1)
                {
                    depthType d01 = depthRow0[x+1];
                    depthType d10 = depthRow1[x];

                    depthType z01 = d01*dfac;
                    depthType z10 = d10*dfac;

                    // before it was
                    //if(z00*z01*z10 != 0)
                    if(z00 != 0 && z01 != 0 && z10 != 0)
                    {
                        Point3f v01 = reproj(Point3f((float)(x+1), (float)(y+0), z01));
                        Point3f v10 = reproj(Point3f((float)(x+0), (float)(y+1), z10));

                        cv::Vec3f vec = (v01-v00).cross(v10-v00);
                        n = -normalize(vec);
                        p = v00;
                    }
                }

                ptsRow[x] = toPtype(p);
                normRow[x] = toPtype(n);
            }
        }
    }

    const Depth& depth;
    Points& points;
    Normals& normals;
    const Intr::Reprojector& reproj;
    float dfac;
};

void computePointsNormals(const Intr intr, float depthFactor, const Depth depth,
                          Points points, Normals normals)
{
    CV_Assert(!points.empty() && !normals.empty());
    CV_Assert(depth.size() == points.size());
    CV_Assert(depth.size() == normals.size());

    // conversion to meters
    // before it was:
    //float dfac = 0.001f/depthFactor;
    float dfac = 1.f/depthFactor;

    Intr::Reprojector reproj = intr.makeReprojector();

    ComputePointsNormalsInvoker ci(depth, points, normals, reproj, dfac);
    Range range(0, depth.rows);
    const int nstripes = -1;
    parallel_for_(range, ci, nstripes);
}

///////// GPU implementation /////////

static bool ocl_renderPointsNormals(const UMat points, const UMat normals, UMat image, Affine3f lightPose);
static bool ocl_makeFrameFromDepth(const UMat depth, OutputArrayOfArrays points, OutputArrayOfArrays normals,
                                   const Intr intr, int levels, float depthFactor,
                                   float sigmaDepth, float sigmaSpatial, int kernelSize);
static bool ocl_buildPyramidPointsNormals(const UMat points, const UMat normals,
                                          OutputArrayOfArrays pyrPoints, OutputArrayOfArrays pyrNormals,
                                          int levels);


static bool computePointsNormalsGpu(const Intr intr, float depthFactor, const UMat& depth, UMat points, UMat normals);
static bool pyrDownBilateralGpu(const UMat& depth, UMat& depthDown, float sigma);
static bool customBilateralFilterGpu(const UMat src, UMat& dst, int kernelSize, float sigmaDepth, float sigmaSpatial);
static bool pyrDownPointsNormalsGpu(const UMat p, const UMat n, UMat &pdown, UMat &ndown);


bool computePointsNormalsGpu(const Intr intr, float depthFactor, const UMat& depth,
                             UMat points, UMat normals)
{
    CV_Assert(!points.empty() && !normals.empty());
    CV_Assert(depth.size() == points.size());
    CV_Assert(depth.size() == normals.size());
    CV_Assert(depth.type() == DEPTH_TYPE);
    CV_Assert(points.type()  == POINT_TYPE);
    CV_Assert(normals.type() == POINT_TYPE);

    // conversion to meters
    float dfac = 1.f/depthFactor;

    Intr::Reprojector reproj = intr.makeReprojector();

    cv::String errorStr;
    cv::String name = "computePointsNormals";
    ocl::ProgramSource source = ocl::rgbd::kinfu_frame_oclsrc;
    cv::String options = "-cl-fast-relaxed-math -cl-mad-enable";
    ocl::Kernel k;
    k.create(name.c_str(), source, options, &errorStr);

    if(k.empty())
        return false;

    k.args(ocl::KernelArg::WriteOnly(points),
           ocl::KernelArg::WriteOnly(normals),
           ocl::KernelArg::ReadOnly(depth),
           reproj.fxinv, reproj.fyinv,
           reproj.cx, reproj.cy,
           dfac);

    size_t globalSize[2];
    globalSize[0] = (size_t)depth.cols;
    globalSize[1] = (size_t)depth.rows;

    return k.run(2, globalSize, NULL, true);
}


bool pyrDownBilateralGpu(const UMat& depth, UMat& depthDown, float sigma)
{
    depthDown.create(depth.rows/2, depth.cols/2, DEPTH_TYPE);

    cv::String errorStr;
    cv::String name = "pyrDownBilateral";
    ocl::ProgramSource source = ocl::rgbd::kinfu_frame_oclsrc;
    cv::String options = "-cl-fast-relaxed-math -cl-mad-enable";
    ocl::Kernel k;
    k.create(name.c_str(), source, options, &errorStr);

    if(k.empty())
        return false;

    k.args(ocl::KernelArg::ReadOnly(depth),
           ocl::KernelArg::WriteOnly(depthDown),
           sigma);

    size_t globalSize[2];
    globalSize[0] = (size_t)depthDown.cols;
    globalSize[1] = (size_t)depthDown.rows;

    return k.run(2, globalSize, NULL, true);
}

//TODO: remove it when OpenCV's bilateral processes 32f on GPU
bool customBilateralFilterGpu(const UMat src /* udepth */, UMat& dst /* smooth */,
                              int kernelSize, float sigmaDepth, float sigmaSpatial)
{
    CV_Assert(src.size().area() > 0);
    CV_Assert(src.type() == DEPTH_TYPE);

    dst.create(src.size(), DEPTH_TYPE);

    cv::String errorStr;
    cv::String name = "customBilateral";
    ocl::ProgramSource source = ocl::rgbd::kinfu_frame_oclsrc;
    cv::String options = "-cl-fast-relaxed-math -cl-mad-enable";
    ocl::Kernel k;
    k.create(name.c_str(), source, options, &errorStr);

    if(k.empty())
        return false;

    k.args(ocl::KernelArg::ReadOnly(src),
           ocl::KernelArg::WriteOnly(dst),
           kernelSize,
           0.5f / (sigmaSpatial * sigmaSpatial),
           0.5f / (sigmaDepth * sigmaDepth));

    size_t globalSize[2];
    globalSize[0] = (size_t)src.cols;
    globalSize[1] = (size_t)src.rows;

    return k.run(2, globalSize, NULL, true);
}


bool pyrDownPointsNormalsGpu(const UMat p, const UMat n, UMat &pdown, UMat &ndown)
{
    cv::String errorStr;
    cv::String name = "pyrDownPointsNormals";
    ocl::ProgramSource source = ocl::rgbd::kinfu_frame_oclsrc;
    cv::String options = "-cl-fast-relaxed-math -cl-mad-enable";
    ocl::Kernel k;
    k.create(name.c_str(), source, options, &errorStr);

    if(k.empty())
        return false;

    k.args(ocl::KernelArg::ReadOnly(p),
           ocl::KernelArg::ReadOnly(n),
           ocl::KernelArg::WriteOnly(pdown),
           ocl::KernelArg::WriteOnly(ndown));

    size_t globalSize[2];
    globalSize[0] = (size_t)pdown.cols;
    globalSize[1] = (size_t)pdown.rows;

    return k.run(2, globalSize, NULL, true);
}


static bool ocl_renderPointsNormals(const UMat points, const UMat normals,
                                    UMat img, Affine3f lightPose)
{
    CV_TRACE_FUNCTION();

    ScopeTime st("frame ocl render");

    cv::String errorStr;
    cv::String name = "render";
    ocl::ProgramSource source = ocl::rgbd::kinfu_frame_oclsrc;
    cv::String options = "-cl-fast-relaxed-math -cl-mad-enable";
    ocl::Kernel k;
    k.create(name.c_str(), source, options, &errorStr);

    if(k.empty())
        return false;

    Vec4f lightPt(lightPose.translation()[0],
                  lightPose.translation()[1],
                  lightPose.translation()[2]);

    k.args(ocl::KernelArg::ReadOnly(points),
           ocl::KernelArg::ReadOnly(normals),
           ocl::KernelArg::WriteOnly(img),
           lightPt.val);

    size_t globalSize[2];
    globalSize[0] = (size_t)points.cols;
    globalSize[1] = (size_t)points.rows;

    return k.run(2, globalSize, NULL, true);
}


void renderPointsNormals(InputArray _points, InputArray _normals, OutputArray image, Affine3f lightPose)
{
    CV_TRACE_FUNCTION();

    CV_Assert(_points.size().area() > 0);
    CV_Assert(_points.size() == _normals.size());

    Size sz = _points.size();
    image.create(sz, CV_8UC4);

    CV_OCL_RUN(_points.isUMat() && _normals.isUMat() && image.isUMat(),
               ocl_renderPointsNormals(_points.getUMat(),
                                       _normals.getUMat(),
                                       image.getUMat(), lightPose))

    ScopeTime st("frame cpu render");

    Points  points  = _points.getMat();
    Normals normals = _normals.getMat();

    Mat_<Vec4b> img = image.getMat();

    RenderInvoker ri(points, normals, img, lightPose, sz);
    Range range(0, sz.height);
    const int nstripes = -1;
    parallel_for_(range, ri, nstripes);
}


static bool ocl_makeFrameFromDepth(const UMat depth, OutputArrayOfArrays points, OutputArrayOfArrays normals,
                                   const Intr intr, int levels, float depthFactor,
                                   float sigmaDepth, float sigmaSpatial, int kernelSize)
{
    CV_TRACE_FUNCTION();

    ScopeTime st("frameGenerator gpu: from depth");

    // looks like OpenCV's bilateral filter works the same as KinFu's
    UMat smooth;
    //TODO: fix that
    // until 32f isn't implemented in OpenCV, we should use our workarounds
    //bilateralFilter(udepth, smooth, kernelSize, sigmaDepth*depthFactor, sigmaSpatial);
    if(!customBilateralFilterGpu(depth, smooth, kernelSize, sigmaDepth*depthFactor, sigmaSpatial))
        return  false;

    // depth truncation is not used by default
    //if (p.icp_truncate_depth_dist > 0) kfusion::cuda::depthTruncation(curr_.depth_pyr[0], p.icp_truncate_depth_dist);

    UMat scaled = smooth;
    Size sz = smooth.size();
    points.create(levels, 1, POINT_TYPE);
    normals.create(levels, 1, POINT_TYPE);
    for(int i = 0; i < levels; i++)
    {
        UMat& p = points.getUMatRef(i);
        UMat& n = normals.getUMatRef(i);
        p.create(sz, POINT_TYPE);
        n.create(sz, POINT_TYPE);

        if(!computePointsNormalsGpu(intr.scale(i), depthFactor, scaled, p, n))
            return false;

        if(i < levels - 1)
        {
            sz.width /= 2, sz.height /= 2;
            UMat halfDepth(sz, DEPTH_TYPE);
            pyrDownBilateralGpu(scaled, halfDepth, sigmaDepth*depthFactor);
            scaled = halfDepth;
        }
    }

    return true;
}


void makeFrameFromDepth(InputArray _depth,
                        OutputArray pyrPoints, OutputArray pyrNormals,
                        const Intr intr, int levels, float depthFactor,
                        float sigmaDepth, float sigmaSpatial, int kernelSize)
{
    CV_TRACE_FUNCTION();

    CV_Assert(_depth.type() == DEPTH_TYPE);

    CV_OCL_RUN(_depth.isUMat() && pyrPoints.isUMatVector() && pyrNormals.isUMatVector(),
               ocl_makeFrameFromDepth(_depth.getUMat(), pyrPoints, pyrNormals,
                                      intr, levels, depthFactor,
                                      sigmaDepth, sigmaSpatial, kernelSize));

    ScopeTime st("frameGenerator cpu: from depth");

    Depth depth = _depth.getMat();

    // looks like OpenCV's bilateral filter works the same as KinFu's
    Depth smooth;
    bilateralFilter(depth, smooth, kernelSize, sigmaDepth*depthFactor, sigmaSpatial);

    // depth truncation is not used by default
    //if (p.icp_truncate_depth_dist > 0) kfusion::cuda::depthTruncation(curr_.depth_pyr[0], p.icp_truncate_depth_dist);

    // we don't need depth pyramid outside this method
    // if we do, the code is to be refactored

    Depth scaled = smooth;
    Size sz = smooth.size();
    pyrPoints.create(levels, 1, POINT_TYPE);
    pyrNormals.create(levels, 1, POINT_TYPE);
    for(int i = 0; i < levels; i++)
    {
        Mat& mp = pyrPoints. getMatRef(i);
        Mat& mn = pyrNormals.getMatRef(i);
        mp.create(sz, POINT_TYPE);
        mn.create(sz, POINT_TYPE);
        Points  p = mp;
        Normals n = mn;

        computePointsNormals(intr.scale(i), depthFactor, scaled, p, n);

        if(i < levels - 1)
        {
            sz.width /= 2; sz.height /= 2;
            scaled = pyrDownBilateral(scaled, sigmaDepth*depthFactor);
        }
    }
}


static bool ocl_buildPyramidPointsNormals(const UMat points, const UMat normals,
                                          OutputArrayOfArrays pyrPoints, OutputArrayOfArrays pyrNormals,
                                          int levels)
{
    CV_TRACE_FUNCTION();

    ScopeTime st("frameGenerator gpu: pyrDown p, n");

    pyrPoints .create(levels, 1, POINT_TYPE);
    pyrNormals.create(levels, 1, POINT_TYPE);

    pyrPoints .getUMatRef(0) = points;
    pyrNormals.getUMatRef(0) = normals;

    Size sz = points.size();
    for(int i = 1; i < levels; i++)
    {
        UMat p1 = pyrPoints .getUMat(i-1);
        UMat n1 = pyrNormals.getUMat(i-1);

        sz.width /= 2; sz.height /= 2;
        UMat& p0 = pyrPoints .getUMatRef(i);
        UMat& n0 = pyrNormals.getUMatRef(i);
        p0.create(sz, POINT_TYPE);
        n0.create(sz, POINT_TYPE);

        if(!pyrDownPointsNormalsGpu(p1, n1, p0, n0))
            return false;
    }

    return true;
}


void buildPyramidPointsNormals(InputArray _points, InputArray _normals,
                               OutputArrayOfArrays pyrPoints, OutputArrayOfArrays pyrNormals,
                               int levels)
{
    CV_TRACE_FUNCTION();

    CV_Assert(_points.type() == POINT_TYPE);
    CV_Assert(_points.type() == _normals.type());
    CV_Assert(_points.size() == _normals.size());

    CV_OCL_RUN(_points.isUMat() && _normals.isUMat() &&
               pyrPoints.isUMatVector() && pyrNormals.isUMatVector(),
               ocl_buildPyramidPointsNormals(_points.getUMat(), _normals.getUMat(),
                                             pyrPoints, pyrNormals,
                                             levels));

    ScopeTime st("frameGenerator cpu: pyrDown p, n");


    Mat p0 = _points.getMat(), n0 = _normals.getMat();

    pyrPoints .create(levels, 1, POINT_TYPE);
    pyrNormals.create(levels, 1, POINT_TYPE);

    pyrPoints .getMatRef(0) = p0;
    pyrNormals.getMatRef(0) = n0;

    Size sz = _points.size();
    for(int i = 1; i < levels; i++)
    {
        Points  p1 = pyrPoints .getMat(i-1);
        Normals n1 = pyrNormals.getMat(i-1);

        sz.width /= 2; sz.height /= 2;
        Mat& mpd = pyrPoints .getMatRef(i);
        Mat& mnd = pyrNormals.getMatRef(i);
        mpd.create(sz, POINT_TYPE);
        mnd.create(sz, POINT_TYPE);
        Points  pd = mpd;
        Normals nd = mnd;

        pyrDownPointsNormals(p1, n1, pd, nd);
    }
}

} // namespace kinfu
} // namespace cv
