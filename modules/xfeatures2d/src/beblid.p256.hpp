// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
// Author: Iago Suarez <iago.suarez.canosa@alumnos.upm.es>

// Implementation of the article:
//     Iago Suarez, Ghesn Sfeir, Jose M. Buenaposada, and Luis Baumela.
//     BEBLID: Boosted Efficient Binary Local Image Descriptor.
//     Pattern Recognition Letters, 133:366–372, 2020.

#ifndef __OPENCV_XFEATURES2D_BEBLID_P256_HPP__
#define __OPENCV_XFEATURES2D_BEBLID_P256_HPP__

// ABWLParams: x1, y1, x2, y2, boxRadius, th

// Pre-trained parameters of BEBLID-256 trained in Liberty data set with
// a million of patch pairs, 20% positives and 80% negatives
static const ABWLParams wl_params_256[] = {
    {26, 20, 14, 16, 5, 16}, {17, 17, 15, 15, 2, 7}, {18, 16, 8, 13, 3, 18},
    {19, 15, 13, 14, 3, 17}, {16, 16, 5, 15, 4, 10}, {25, 10, 16, 16, 6, 11},
    {16, 15, 12, 15, 1, 12}, {18, 17, 14, 17, 1, 13}, {15, 14, 5, 21, 5, 6}, {14, 14, 11, 7, 4, 2},
    {23, 27, 16, 17, 4, 8}, {12, 17, 10, 24, 5, 0}, {15, 15, 13, 14, 1, 6}, {16, 16, 14, 16, 1, 7},
    {19, 18, 16, 15, 1, 6}, {24, 7, 19, 15, 6, 4}, {15, 16, 6, 8, 5, 6}, {24, 16, 8, 15, 7, 22},
    {15, 6, 13, 16, 4, 6}, {17, 19, 15, 15, 1, 6}, {17, 12, 16, 16, 1, 2}, {11, 15, 7, 25, 6, 0},
    {15, 15, 14, 10, 2, 2}, {26, 15, 18, 17, 4, 6}, {18, 12, 17, 27, 4, 3}, {9, 15, 6, 8, 6, 1},
    {15, 17, 14, 23, 3, 1}, {11, 17, 4, 14, 4, 1}, {22, 18, 19, 5, 5, 5}, {11, 18, 11, 5, 5, 3},
    {22, 5, 19, 19, 5, 2}, {12, 26, 6, 15, 3, 5}, {16, 16, 14, 18, 1, 7}, {22, 26, 22, 13, 5, 2},
    {18, 13, 16, 16, 1, 4}, {14, 26, 13, 10, 5, 3}, {17, 13, 14, 14, 1, 10}, {21, 16, 19, 7, 3, 4},
    {14, 15, 14, 13, 1, 0}, {26, 26, 20, 18, 5, 1}, {12, 10, 8, 21, 4, 3}, {14, 17, 13, 7, 3, 0},
    {13, 12, 10, 19, 2, 4}, {17, 20, 17, 13, 2, 0}, {8, 25, 6, 11, 6, 2}, {27, 11, 20, 24, 4, 3},
    {14, 18, 12, 14, 2, 5}, {22, 19, 18, 20, 2, 5}, {18, 4, 17, 14, 3, 1}, {13, 28, 13, 18, 3, 3},
    {15, 12, 14, 17, 1, 4}, {13, 20, 10, 11, 2, 3}, {10, 5, 4, 17, 4, 2}, {7, 18, 3, 18, 3, 2},
    {21, 11, 15, 2, 2, 11}, {20, 15, 17, 17, 1, 6}, {10, 20, 4, 27, 4, 3}, {24, 25, 23, 7, 6, 0},
    {18, 15, 18, 12, 2, 0}, {17, 16, 16, 13, 1, 3}, {14, 20, 14, 15, 1, 1}, {17, 17, 17, 14, 1, 0},
    {7, 15, 6, 5, 5, 3}, {11, 21, 11, 13, 2, 1}, {18, 16, 15, 9, 1, 7}, {19, 19, 18, 15, 1, 2},
    {28, 19, 20, 16, 3, 1}, {14, 16, 11, 10, 1, 3}, {22, 13, 19, 14, 1, 2}, {9, 10, 4, 4, 4, 3},
    {20, 26, 10, 29, 2, 12}, {14, 17, 12, 19, 1, 3}, {21, 18, 18, 24, 2, 6}, {16, 15, 15, 19, 1, 4},
    {27, 4, 24, 15, 4, 2}, {15, 22, 14, 6, 2, 2}, {13, 16, 9, 12, 1, 2}, {12, 12, 11, 18, 1, 2},
    {22, 17, 20, 11, 2, 2}, {18, 28, 17, 23, 3, 1}, {6, 9, 5, 21, 4, 0}, {12, 3, 8, 11, 3, 5},
    {21, 16, 19, 16, 1, 2}, {18, 16, 17, 19, 1, 2}, {27, 12, 22, 3, 3, 2}, {13, 27, 4, 26, 4, 3},
    {5, 22, 3, 26, 3, 2}, {24, 28, 23, 20, 3, 2}, {11, 17, 8, 19, 2, 0}, {13, 16, 11, 16, 1, 3},
    {18, 15, 18, 8, 2, 1}, {15, 17, 14, 14, 1, 3}, {19, 14, 17, 12, 1, 4}, {25, 10, 22, 20, 2, 0},
    {14, 12, 13, 9, 1, 1}, {9, 10, 3, 9, 3, 2}, {20, 22, 19, 17, 1, 0}, {16, 24, 16, 10, 2, 0},
    {15, 23, 13, 29, 2, 2}, {15, 20, 14, 17, 1, 4}, {27, 27, 22, 27, 4, 1}, {14, 7, 6, 3, 3, 3},
    {21, 3, 20, 7, 3, 0}, {29, 5, 25, 11, 2, 1}, {15, 21, 15, 20, 1, 0}, {8, 17, 8, 11, 2, 1},
    {17, 13, 17, 8, 1, 0}, {7, 25, 3, 21, 3, 0}, {7, 11, 7, 8, 3, 1}, {4, 11, 3, 26, 3, 2},
    {15, 18, 15, 11, 1, 1}, {23, 15, 20, 19, 2, 2}, {5, 9, 3, 4, 3, 2}, {28, 18, 25, 8, 3, 0},
    {20, 22, 17, 30, 1, 5}, {29, 29, 28, 16, 2, 1}, {28, 11, 24, 15, 2, 1}, {20, 7, 18, 9, 1, 2},
    {19, 12, 18, 16, 1, 2}, {11, 20, 11, 17, 2, 1}, {13, 16, 13, 13, 1, 0}, {29, 3, 23, 5, 2, 0},
    {19, 21, 17, 18, 1, 3}, {12, 8, 12, 3, 2, 2}, {14, 13, 13, 20, 1, 2}, {11, 21, 9, 29, 2, 3},
    {7, 30, 6, 22, 1, 2}, {11, 9, 10, 15, 1, 3}, {8, 3, 2, 9, 2, 0}, {19, 7, 18, 3, 3, 2},
    {21, 9, 19, 11, 1, 1}, {18, 10, 17, 13, 1, 2}, {6, 17, 1, 30, 1, 6}, {17, 29, 16, 28, 2, 1},
    {17, 20, 17, 18, 1, 0}, {15, 9, 13, 23, 1, 4}, {12, 14, 11, 16, 1, 1}, {7, 17, 5, 14, 2, 1},
    {30, 30, 23, 12, 1, 2}, {29, 18, 26, 20, 2, 0}, {10, 20, 9, 17, 2, 1}, {4, 15, 2, 8, 2, 2},
    {7, 7, 7, 3, 3, 1}, {9, 19, 8, 24, 1, 2}, {28, 25, 27, 25, 3, 0}, {13, 15, 12, 18, 1, 1},
    {25, 2, 19, 5, 2, 2}, {15, 4, 15, 3, 3, 0}, {25, 19, 24, 29, 2, 2}, {18, 24, 18, 20, 1, 1},
    {4, 10, 1, 2, 1, 3}, {5, 18, 1, 18, 1, 2}, {13, 22, 13, 19, 1, 1}, {10, 26, 8, 28, 2, 0},
    {24, 13, 24, 6, 1, 1}, {15, 19, 14, 15, 1, 4}, {5, 8, 2, 16, 2, 0}, {12, 4, 11, 2, 2, 0},
    {14, 29, 14, 24, 1, 1}, {3, 20, 1, 22, 1, 1}, {17, 5, 12, 1, 1, 5}, {21, 16, 20, 23, 1, 2},
    {25, 17, 22, 13, 1, 0}, {6, 21, 5, 16, 1, 0}, {7, 15, 6, 19, 1, 1}, {20, 17, 19, 15, 1, 1},
    {3, 29, 3, 23, 2, 1}, {16, 25, 16, 22, 1, 0}, {28, 20, 28, 12, 3, 0}, {27, 13, 23, 10, 1, 0},
    {24, 24, 17, 29, 1, 5}, {13, 2, 11, 4, 1, 2}, {22, 23, 21, 21, 1, 0}, {19, 30, 19, 24, 1, 1},
    {30, 30, 26, 27, 1, 0}, {17, 5, 17, 1, 1, 0}, {26, 7, 24, 1, 1, 1}, {28, 6, 28, 3, 3, 0},
    {3, 15, 1, 13, 1, 1}, {7, 8, 5, 6, 1, 1}, {19, 16, 19, 15, 1, 0}, {12, 9, 11, 7, 1, 0},
    {17, 22, 16, 20, 1, 2}, {12, 14, 12, 11, 1, 1}, {25, 29, 23, 26, 1, 0}, {15, 19, 15, 18, 1, 0},
    {13, 22, 12, 25, 1, 0}, {1, 22, 1, 11, 1, 0}, {14, 12, 14, 9, 1, 1}, {10, 27, 9, 23, 1, 2},
    {9, 4, 6, 1, 1, 1}, {22, 12, 21, 16, 1, 0}, {5, 27, 1, 28, 1, 1}, {30, 14, 28, 7, 1, 0},
    {17, 9, 16, 21, 1, 2}, {17, 9, 17, 6, 1, 0}, {4, 4, 1, 1, 1, 1}, {30, 2, 28, 5, 1, 0},
    {18, 4, 17, 7, 1, 1}, {15, 13, 15, 10, 1, 1}, {12, 30, 11, 26, 1, 2}, {16, 28, 15, 29, 1, 1},
    {30, 11, 28, 11, 1, 0}, {9, 12, 8, 10, 1, 1}, {22, 19, 21, 16, 1, 0}, {30, 20, 29, 26, 1, 0},
    {22, 10, 20, 7, 1, 2}, {2, 2, 1, 5, 1, 0}, {9, 9, 7, 9, 1, 0}, {27, 1, 25, 3, 1, 0},
    {21, 23, 20, 25, 1, 1}, {10, 3, 8, 5, 1, 1}, {24, 1, 23, 3, 1, 0}, {5, 29, 4, 28, 1, 0},
    {27, 23, 26, 18, 1, 1}, {22, 2, 22, 1, 1, 0}, {7, 20, 6, 19, 1, 0}, {12, 26, 9, 25, 1, 2},
    {7, 1, 5, 2, 1, 0}, {2, 21, 1, 18, 1, 0}, {2, 24, 1, 21, 1, 0}, {8, 17, 8, 14, 1, 0},
    {30, 1, 28, 2, 1, 0}, {15, 30, 15, 28, 1, 0}, {2, 5, 1, 9, 1, 0}, {18, 28, 17, 26, 1, 1},
    {7, 29, 1, 30, 1, 1}, {17, 2, 17, 1, 1, 0}, {21, 13, 21, 9, 1, 1}, {29, 15, 27, 15, 1, 0},
    {28, 8, 27, 7, 2, 0}, {29, 14, 28, 18, 1, 0}, {2, 26, 1, 30, 1, 1}, {16, 8, 16, 6, 1, 0},
    {30, 26, 26, 24, 1, 0}, {15, 17, 15, 16, 6, 0}, {30, 29, 27, 30, 1, 0}, {3, 30, 1, 28, 1, 0},
    {17, 1, 16, 2, 1, 1}, {14, 30, 12, 30, 1, 1}, {12, 17, 12, 16, 1, 0}, {4, 18, 4, 16, 1, 0},
    {11, 4, 11, 1, 1, 1}, {21, 2, 18, 1, 1, 2}, {16, 17, 16, 15, 5, 0}, {3, 1, 2, 2, 1, 0},
    {23, 17, 23, 16, 1, 0}, {18, 12, 18, 11, 1, 0}, {10, 28, 8, 30, 1, 0}, {12, 10, 12, 8, 1, 1},
    {2, 14, 1, 9, 1, 1}, {6, 25, 6, 21, 1, 1}, {6, 2, 2, 1, 1, 1}, {30, 19, 29, 20, 1, 0},
    {25, 21, 23, 20, 1, 0}, {16, 10, 16, 9, 1, 0}
};

#endif