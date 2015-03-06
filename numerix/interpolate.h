#ifndef NUMERIX_INTERPOLATE_H
#define NUMERIX_INTERPOLATE_H

#include <iostream>

namespace numerix {

/** Finds nearest value lower than \a xi in vector \a x and return its index. Used by findNearest(). */
template <class VecT, typename T>
int findNearestLower(const VecT& x, const int len, const T xi)
{
	int xidx = -1;
	for (int c = 0; c < len-1; c++) {
		if ((xi >= x[c] && xi <= x[c+1]) || (xi <= x[c] && xi >= x[c+1]))
			xidx=c;
	}
	// If xi is out of bounds, clamp to end/start of table
	if ((xidx < 0) && (x[1] > x[0] && xi > x[len-1]) || (x[1] < x[0] && xi < x[len-1]))
		xidx=len-1;
	return xidx;
}

/** Finds nearest value to \a xi in vector \a x and return its index. Used by interpolateNearest(). */
template <class VecT, typename T>
int findNearest(const VecT& x, const int len, const T xi)
{
	int xidx = findNearestLower(x,len,xi);
	// Find upper x index
	int xidx2;
	if (xidx < 0)
		xidx=xidx2=0;
	else if (xidx >= len-1)
		xidx2=len-1;
	else
		xidx2=xidx+1;
	if (fabs(x[xidx2]-xi) > fabs(x[xidx]-xi))
		return xidx;
	else
		return xidx2;
}

/** Finds nearest value to \a xi in vector \a x and return its corresponding value in \a y */
template <class VecT, typename T>
T interpolateNearest(const VecT& x, const VecT& y, const int len, const T xi)
{
  return (T) y[findNearest(x,len,xi)];
}

/** Finds nearest value to \a xi in vector \a x and return its corresponding value in \a y. */
template <class VecT, typename T>
T interpolateNearest(const VecT& x, const VecT& y, const T xi)
{
  return (T) y[findNearest(x, x.rows(), xi)];
}

/** Nearest-neighbour interpolation.
	This version operates on Eigen types, i.e. \c Eigen::Vector4d or \c Eigen::VectorXd
	(either fixed- or dynamic sized) and the corresponding \c Eigen::Matrix type.
	\param x,y column vectors
	\param z matrix of size  (\c y.rows(), \c x.rows())
	\param xi,yi interpolation point
 */
template <class VecT, class MatrixT, typename T>
T interpolateNearest(const VecT& x, const VecT& y, const MatrixT& z, const T xi, const T yi)
{
	int xidx = findNearest(x, x.rows(), xi);
	int yidx = findNearest(y, y.rows(), yi);
	return z(yidx,xidx);
}

/** Nearest-neighbour interpolation.
	This version operates on standard C++ types like \c const \c double[] or STL types
	(accessible with \c operator[]) like \c std::vector<double>.
	\param x,y vectors
	\param z row-major matrix of size (\c rows, \c columns)
	\param rows number of rows in the matrix \c z
	\param columns number of rows in the matrix \c z
	\param xi,yi interpolation point
 */
template <class VecT, class MatrixT, typename T>
T interpolateNearest(const VecT& x, const VecT& y, const MatrixT& z, const int rows, const int columns, const T xi, const T yi)
{
	int xidx = findNearest(x, columns, xi);
	int yidx = findNearest(y, rows, yi);
	return z[xidx+yidx*columns];
}

template <class VecT, typename T> 
T interpolateLinear(const VecT& x, const VecT& y, const int len, const T xi)
{
	if (x[len-1] > x[0]) {
		int idx = -1;
		for (int i = 0; i < len; i++) {
			if (x[i] > xi) {
				idx = i;
				break;
			}
		}
		if (idx < 0)
		  return (T) y[len-1];
		else if (idx == 0)
		  return (T) y[0];
		else
		  return (T) (y[idx-1]+(y[idx]-y[idx-1])/(x[idx]-x[idx-1])*(xi-x[idx-1]));
	} else if (x[len-1] < x[0]) {
		int idx = -1;
		for (int i = 0; i < len; i++) {
			if (x[i] < xi) {
				idx = i;
				break ;
			}
		}
		if (idx < 0)
		  return (T) y[len-1];
		else if (idx == 0)
		  return (T) y[0];
		else
		  return (T) (y[idx]+(y[idx-1]-y[idx])/(x[idx-1]-x[idx])*(xi-x[idx]));
	} else
	  return (T) y[0];
}

template <class VecT, typename T>
T interpolateLinear(const VecT& x, const VecT& y, const T xi)
{
	return interpolateLinear(x, y, y.rows(), xi);
}

/** Bilinear interpolation.
	This version operates on Eigen types, i.e. \c Eigen::Vector4d or \c Eigen::VectorXd
	(either fixed- or dynamic sized) and the corresponding \c Eigen::Matrix type.
	\param x,y column vectors
	\param z matrix of size  (\c y.rows(), \c x.rows())
	\param xi,yi interpolation point
*/
template <class VecT, class MatrixT, typename T>
T interpolateLinear(const VecT& x, const VecT& y, const MatrixT& z, const T xi, const T yi)
{
		int rows = z.rows();
		int columns = z.cols();
        int xidx = -1;
        int yidx = -1;
        // Find lower x index
        for (int c = 0; c < columns-1; c++) {
                if ((xi >= x[c] && xi <= x[c+1]) || (xi <= x[c] && xi >= x[c+1]))
                        xidx=c;
        }
        // If xi is out of bounds, clamp to end/start of table
        if ((xidx < 0) && (x[1] > x[0] && xi > x[columns-1]) || (x[1] < x[0] && xi < x[columns-1]))
                        xidx=columns-1;
        // Find lower y index
        for (int r = 0; r < rows-1; r++) {
                if ((yi >= y[r] && yi <= y[r+1]) || (yi <= y[r] && yi >= y[r+1]))
                        yidx=r;
        }
        // If yi is out of bounds, clamp to end/start of table
        if ((yidx < 0) && (y[1] > y[0] && yi > y[rows-1]) || (y[1] < y[0] && yi < y[rows-1]))
                yidx=rows-1;
        int xidx2, yidx2;
        // Find upper x index
        if (xidx < 0)
                xidx=xidx2=0;
        else if (xidx >= columns-1)
                xidx2=columns-1;
        else
                xidx2=xidx+1;
        // Find upper y index
        if (yidx < 0)
                yidx=yidx2=0;
        else if (yidx >= rows-1)
                yidx2=rows-1;
        else
                yidx2=yidx+1;
        // Extract the four values to interpolate between
        T z1,z2,z3,z4,z5,z6;
        z1=z(yidx,xidx);
        z2=z(yidx,xidx2);
        z3=z(yidx2,xidx);
        z4=z(yidx2,xidx2);
        // Interpolate
        if (xidx==xidx2 && yidx==yidx2)
	  return (T) z1; // both x and y clamped to start/end - no interpolation
        else if (xidx==xidx2) // x clamped to start/end - interpolate in y direction
	  return (T) (z1+(z3-z1)*(yi-y[yidx])/(y[yidx2]-y[yidx]));
        else if (yidx==yidx2) // y clamped to start/end - interpolate in x direction
	  return (T) (z1+(z2-z1)*(xi-x[xidx])/(x[xidx2]-x[xidx]));
        else {
                // first in the x direction
                z5=z1+(z2-z1)*(xi-x[xidx])/(x[xidx2]-x[xidx]);
                z6=z3+(z4-z3)*(xi-x[xidx])/(x[xidx2]-x[xidx]);
                // then in the y direction
                return (T) (z5+(z6-z5)*(yi-y[yidx])/(y[yidx2]-y[yidx]));
        }
}


/** Bilinear interpolation.
	This version operates on standard C++ types like \c const \c double[] or STL types
	(accessible with \c operator[]) like \c std::vector<double>.
	\param x,y vectors
	\param z row-major matrix of size (\c rows, \c columns)
	\param rows number of rows in the matrix \c z
	\param columns number of rows in the matrix \c z
	\param xi,yi interpolation point
*/
template <class VecT, class MatrixT, typename T>
T interpolateLinear(const VecT& x, const VecT& y, const MatrixT& z, const int rows, const int columns, const T xi, const T yi)
{
        int xidx = -1;
        int yidx = -1;
        // Find lower x index
        for (int c = 0; c < columns-1; c++) {
                if ((xi >= x[c] && xi <= x[c+1]) || (xi <= x[c] && xi >= x[c+1]))
                        xidx=c;
        }
        // If xi is out of bounds, clamp to end/start of table
        if ((xidx < 0) && (x[1] > x[0] && xi > x[columns-1]) || (x[1] < x[0] && xi < x[columns-1]))
                        xidx=columns-1;
        // Find lower y index
        for (int r = 0; r < rows-1; r++) {
                if ((yi >= y[r] && yi <= y[r+1]) || (yi <= y[r] && yi >= y[r+1]))
                        yidx=r;
        }
        // If yi is out of bounds, clamp to end/start of table
        if ((yidx < 0) && (y[1] > y[0] && yi > y[rows-1]) || (y[1] < y[0] && yi < y[rows-1]))
                yidx=rows-1;
        int xidx2, yidx2;
        // Find upper x index
        if (xidx < 0)
                xidx=xidx2=0;
        else if (xidx >= columns-1)
                xidx2=columns-1;
        else
                xidx2=xidx+1;
        // Find upper y index
        if (yidx < 0)
                yidx=yidx2=0;
        else if (yidx >= rows-1)
                yidx2=rows-1;
        else
                yidx2=yidx+1;
        // Extract the four values to interpolate between
        T z1,z2,z3,z4,z5,z6;
        z1=z[xidx+yidx*columns];
        z2=z[xidx2+yidx*columns];
        z3=z[xidx+yidx2*columns];
        z4=z[xidx2+yidx2*columns];
        // Interpolate
        if (xidx==xidx2 && yidx==yidx2)
	  return (T) z1; // both x and y clamped to start/end - no interpolation
        else if (xidx==xidx2) // x clamped to start/end - interpolate in y direction
	  return (T) (z1+(z3-z1)*(yi-y[yidx])/(y[yidx2]-y[yidx]));
        else if (yidx==yidx2) // y clamped to start/end - interpolate in x direction
	  return (T) (z1+(z2-z1)*(xi-x[xidx])/(x[xidx2]-x[xidx]));
        else {
                // first in the x direction
                z5=z1+(z2-z1)*(xi-x[xidx])/(x[xidx2]-x[xidx]);
                z6=z3+(z4-z3)*(xi-x[xidx])/(x[xidx2]-x[xidx]);
                // then in the y direction
                return (T) (z5+(z6-z5)*(yi-y[yidx])/(y[yidx2]-y[yidx]));
        }
}

} // namespace sbxNumerics

#endif
