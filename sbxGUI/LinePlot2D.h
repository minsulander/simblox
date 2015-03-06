#ifndef LINEPLOT2D_H
#define LINEPLOT2D_H

#include <Eigen/Core>
#ifdef WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/GL.h>
#else
#include <GL/gl.h>
#endif
#include <vector>
#include <string>


enum MarkerType {
	MARKER_NONE,
	MARKER_CROSS,
	MARKER_SQUARE,
	MARKER_DIAMOND,
	MARKER_PLUS
};

enum LineType {
	LINE_NONE,
	LINE_NORMAL,
	LINE_POINT,
	LINE_ODD
};

struct DataSet
{
	DataSet();
	void append(const double x, const double y);
	void clear();
	
	std::vector<double> x, y;
	int linewidth;
	Eigen::Vector4f color;
	MarkerType marker;
	LineType line;
	std::string label;
	int show_value_index;
};

class LinePlot2D
{
public:
	LinePlot2D();
	void draw();
	void add(DataSet* data) { datasets.push_back(data); }
	
	void setAxisColor(const Eigen::Vector4f& color) { axiscolor = color; }
	void setGridColor(const Eigen::Vector4f& color) { gridcolor = color; }
	void setTextColor(const Eigen::Vector4f& color) { textcolor = color; }
protected:
	void drawMarker(MarkerType marker, double x, double y);
	std::vector<DataSet*> datasets;
private:
	std::string title, xlabel, ylabel;
	Eigen::Vector4f axiscolor, gridcolor, textcolor;
};

#endif
