#include <UnitTest++/UnitTest++.h>
#include <numerix/interpolate.h>
#include <Eigen/Core>
#include <iostream>
#include <vector>

using namespace numerix;
using namespace Eigen;

const double x[] = {1, 2, 3, 4};
const double y[] = {1, 5, 10, 20};
const double z[] = {10, 20, 30, 40,
					20, 5, 10, 2,
					47, 11, 1, 30,
					40, 7, 30, 40};

struct InterpolateFixture
{
	InterpolateFixture()
	: xvx(4,1), yvx(4,1), zmx(4,4)
	{
		for (int i = 0; i <  4; i++) {
			xv.push_back(x[i]);
			yv.push_back(y[i]);
			xvd[i] = x[i];
			yvd[i] = y[i];
			xvx[i] = x[i];
			yvx[i] = y[i];
			for (int j = 0; j < 4; j++) {
				zmd(i,j) = z[j+i*4];
				zmx(i,j) = z[j+i*4];
			}
		}
	}

	std::vector<double> xv, yv;
	Vector4d xvd, yvd;
	Matrix4d zmd;
	VectorXd xvx, yvx;
	MatrixXd zmx;
};


TEST_FIXTURE(InterpolateFixture, NearestInterpolation) {
	CHECK_EQUAL(1, findNearestLower(y, 4, 8.0));
	CHECK_EQUAL(2, findNearestLower(y, 4, 14.0));
	CHECK_EQUAL(2, findNearest(y, 4, 8.0));
	CHECK_EQUAL(2, findNearest(y, 4, 14.0));
	
	CHECK_CLOSE(10, interpolateNearest(x, y, 4, 2.7), 1e-10);
	CHECK_CLOSE(1, interpolateNearest(x,y,4,0), 1e-10);
	CHECK_CLOSE(20, interpolateNearest(x,y,4,10), 1e-10);
	CHECK_CLOSE(20, interpolateNearest(xv,yv,4,10), 1e-10);
	CHECK_CLOSE(20, interpolateNearest(xvd,yvd,10), 1e-10);
	CHECK_CLOSE(20, interpolateNearest(xvx,yvx,10), 1e-10);
	
	CHECK_CLOSE(5, interpolateNearest(x, y, z, 4, 4, 2.2, 5.7), 1e-10);
	CHECK_CLOSE(5, interpolateNearest(xvd, yvd, zmd, 2.2, 5.7), 1e-10);
	CHECK_CLOSE(5, interpolateNearest(xvx, yvx, zmx, 2.2, 5.7), 1e-10);
}

TEST_FIXTURE(InterpolateFixture, LinearInterpolation) {
	// Since it's a template function, a bunch of different data types should work
	CHECK_CLOSE(7.5, interpolateLinear(x,y,4,2.5), 1e-10);
	CHECK_CLOSE(7.5, interpolateLinear(xv,yv,4,2.5), 1e-10);
	CHECK_CLOSE(7.5, interpolateLinear(xvd,yvd,2.5), 1e-10);
	CHECK_CLOSE(7.5, interpolateLinear(xvx,yvx,2.5), 1e-10);
	// Check that it clamps to boundary values
	CHECK_CLOSE(1, interpolateLinear(x,y,4,0), 1e-10);
	CHECK_CLOSE(20, interpolateLinear(x,y,4,10), 1e-10);
	
	// BiLinear interpolation
	CHECK_CLOSE(6.42, interpolateLinear(x, y, z, 4, 4, 2.2, 5.7), 1e-10);
	CHECK_CLOSE(6.42, interpolateLinear(xvd, yvd, zmd, 2.2, 5.7), 1e-10);
	// Check clamping to boundary
	CHECK_CLOSE(20, interpolateLinear(x, y, z, 4, 4, 0.1, 5.0), 1e-10);
	CHECK_CLOSE(30, interpolateLinear(x, y, z, 4, 4, 9.9, 10.0), 1e-10);
	CHECK_CLOSE(20, interpolateLinear(x, y, z, 4, 4, 2.0, -7.0), 1e-10);
	CHECK_CLOSE(35, interpolateLinear(x, y, z, 4, 4, 3.5, 70.0), 1e-10);
}

