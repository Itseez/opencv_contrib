#ifndef __OPENCV_DNN_DNN_BLOB_INL_HPP__
#define __OPENCV_DNN_DNN_BLOB_INL_HPP__
#include "blob.hpp"

namespace cv
{
namespace dnn
{

inline BlobShape::BlobShape(int ndims, int fill) : sz( (size_t)std::max(ndims, 0) )
{
    CV_Assert(ndims >= 0);
    for (int i = 0; i < ndims; i++)
        sz[i] = fill;
}

inline BlobShape::BlobShape(int ndims, const int *sizes) : sz( (size_t)std::max(ndims, 0) )
{
    CV_Assert(ndims >= 0);
    for (int i = 0; i < ndims; i++)
        sz[i] = sizes[i];
}

inline BlobShape::BlobShape(int num, int cn, int rows, int cols) : sz(4)
{
    sz[0] = num;
    sz[1] = cn;
    sz[2] = rows;
    sz[3] = cols;
}

inline BlobShape::BlobShape(const std::vector<int> &sizes) : sz( sizes.size() )
{
    for (int i = 0; i < (int)sizes.size(); i++)
        sz[i] = sizes[i];
}

template<int n>
inline BlobShape::BlobShape(const Vec<int, n> &shape) : sz(n)
{
    for (int i = 0; i < n; i++)
        sz[i] = shape[i];
}

inline int BlobShape::dims() const
{
    return (int)sz.size();
}

inline int BlobShape::xsize(int axis) const
{
    if (axis < -dims() || axis >= dims())
        return 1;

    return sz[(axis < 0) ? axis + dims() : axis];
}

inline int BlobShape::size(int axis) const
{
    CV_Assert(-dims() <= axis && axis < dims());
    return sz[(axis < 0) ? axis + dims() : axis];
}

inline int &BlobShape::size(int axis)
{
    CV_Assert(-dims() <= axis && axis < dims());
    return sz[(axis < 0) ? axis + dims() : axis];
}

inline int BlobShape::operator[] (int axis) const
{
    CV_Assert(-dims() <= axis && axis < dims());
    return sz[(axis < 0) ? axis + dims() : axis];
}

inline int &BlobShape::operator[] (int axis)
{
    CV_Assert(-dims() <= axis && axis < dims());
    return sz[(axis < 0) ? axis + dims() : axis];
}

inline ptrdiff_t BlobShape::total()
{
    if (dims() == 0)
        return 0;

    ptrdiff_t res = 1;
    for (int i = 0; i < dims(); i++)
        res *= sz[i];
    return res;
}

inline const int *BlobShape::ptr() const
{
    return sz;
}

inline bool BlobShape::equal(const BlobShape &other) const
{
    if (this->dims() != other.dims())
        return false;

    for (int i = 0; i < other.dims(); i++)
    {
        if (sz[i] != other.sz[i])
            return false;
    }

    return true;
}

inline bool operator== (const BlobShape &l, const BlobShape &r)
{
    return l.equal(r);
}



inline int Blob::canonicalAxis(int axis) const
{
    CV_Assert(-dims() <= axis && axis < dims());
    return (axis < 0) ? axis + dims() : axis;
}

inline int Blob::dims() const
{
    return m.dims;
}

inline int Blob::xsize(int axis) const
{
    if (axis < -dims() || axis >= dims())
        return 1;

    return sizes()[(axis < 0) ? axis + dims() : axis];
}

inline int Blob::size(int axis) const
{
    CV_Assert(-dims() <= axis && axis < dims());
    return sizes()[(axis < 0) ? axis + dims() : axis];
}

inline size_t Blob::total(int startAxis, int endAxis) const
{
    if (startAxis < 0)
        startAxis += dims();

    if (endAxis == -1)
        endAxis = dims();

    CV_Assert(0 <= startAxis && startAxis <= endAxis && endAxis <= dims());

    size_t size = 1; //assume that blob isn't empty
    for (int i = startAxis; i < endAxis; i++)
        size *= (size_t)sizes()[i];

    return size;
}


template<int n>
inline size_t Blob::offset(const Vec<int, n> &pos) const
{
    size_t ofs = 0;
    int i;
    for (i = 0; i < std::min(n, dims()); i++)
    {
        CV_DbgAssert(pos[i] >= 0 && pos[i] < size(i));
        ofs = ofs * (size_t)size(i) + pos[i];
    }
    for (; i < dims(); i++)
        ofs *= (size_t)size(i);
    return ofs;
}

inline size_t Blob::offset(int n, int cn, int row, int col) const
{
    return offset(Vec4i(n, cn, row, col));
}

inline float *Blob::ptrf(int n, int cn, int row, int col)
{
    CV_Assert(type() == CV_32F);
    return (float*)m.data + offset(n, cn, row, col);
}

inline uchar *Blob::ptr(int n, int cn, int row, int col)
{
    return m.data + m.elemSize() * offset(n, cn, row, col);
}

template<typename TFloat>
inline TFloat* Blob::ptr(int n, int cn, int row, int col)
{
    CV_Assert(type() == cv::DataDepth<TFloat>::value);
    return (TFloat*) ptr(n, cn, row, col);
}

inline BlobShape Blob::shape() const
{
    return BlobShape(dims(), sizes());
}

inline bool Blob::equalShape(const Blob &other) const
{
    if (this->dims() != other.dims())
        return false;

    for (int i = 0; i < dims(); i++)
    {
        if (this->sizes()[i] != other.sizes()[i])
            return false;
    }
    return true;
}

inline Mat& Blob::getMatRef()
{
    return m;
}

inline const Mat& Blob::getMatRef() const
{
    return m;
}

inline Mat Blob::getMat(int n, int cn)
{
    return Mat(rows(), cols(), m.type(), this->ptr(n, cn));
}

inline int Blob::cols() const
{
    return xsize(3);
}

inline int Blob::rows() const
{
    return xsize(2);
}

inline int Blob::channels() const
{
    return xsize(1);
}

inline int Blob::num() const
{
    return xsize(0);
}

inline Size Blob::size2() const
{
    return Size(cols(), rows());
}

inline int Blob::type() const
{
    return m.depth();
}

inline bool Blob::isFloat() const
{
    return (type() == CV_32F);
}

inline bool Blob::isDouble() const
{
    return (type() == CV_32F);
}

inline const int * Blob::sizes() const
{
    return &m.size[0];
}


inline Blob &Blob::shareFrom(const Blob &blob)
{
    this->m = blob.m;
    return *this;
}

inline Blob &Blob::reshape(const BlobShape &shape)
{
    m = m.reshape(1, shape.dims(), shape.ptr());
    return *this;
}

}
}

#endif
