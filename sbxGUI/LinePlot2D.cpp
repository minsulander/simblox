#include "LinePlot2D.h"
#include "uglyfont.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <math.h>

// TODO better names for these 2 functions?
double tenpow(const double val) {
	double p = 1e10;
	int n = 10;
	while (floor(fabs(val)/p) < 1) {
		p /= 10;
		n--;
	}
	return p;
}

int tenpow2(const double val) {
	double p = 1e10;
	int n = 10;
	while (floor(fabs(val)/p) < 1) {
		p /= 10;
		n--;
	}
	return n;
}

#ifdef WIN32
double round(double number)
{
    return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}
#endif

DataSet::DataSet() : 
	linewidth(2),
	marker(MARKER_NONE),
	line(LINE_NORMAL),
	show_value_index(-1),
	color(1,0,0,1)
{
}

void DataSet::append(const double ax, const double ay)
{
	x.push_back(ax);
	y.push_back(ay);
}

void DataSet::clear()
{
	x.clear();
	y.clear();
}

LinePlot2D::LinePlot2D() :
	axiscolor(1,1,1,1),
	gridcolor(1,1,1,0.5),
	textcolor(1,1,1,1)
{
}

void LinePlot2D::draw()
{
	// TODO parameters
	double x_tickstep = 1;
	double y_tickstep = 0.5;
	int x_numticks = 10;
	int y_numticks = 10;
	bool grid = true;
	bool ticktext = true;
	bool axesonside = false;
	double ticktextsize = 0.015;
	int textwidth=1;
	double labelstextsize = 0.02;
	double epsilon=1e-5;
	bool showlegend = false;
	const double x_legend = 0.5;
	const double y_legend = 0.8;
	if (datasets.size() > 1)
		showlegend = true;
		
	glPushMatrix();

	// TODO check this on the fly when setting values
	double minx, maxx, miny, maxy;
	minx = miny = 1e100;
	maxx = maxy = -1e100;
	for (int j = 0; j < datasets.size(); j++) {
		for (int i = 0; i < datasets[j]->x.size(); i++) {
			if (datasets[j]->x[i] < minx) minx = datasets[j]->x[i];
			if (datasets[j]->x[i] > maxx) maxx = datasets[j]->x[i];
			if (datasets[j]->y[i] < miny) miny = datasets[j]->y[i];
			if (datasets[j]->y[i] > maxy) maxy = datasets[j]->y[i];
		}
	}

	double xlen = maxx-minx;
	double ylen = maxy-miny;
	double x_yaxis = -minx/xlen;
	double y_xaxis = -miny/ylen;
	double xtenpow = tenpow(xlen);
	double ytenpow = tenpow(ylen);
	if (axesonside || x_yaxis < 0)
		x_yaxis=0;
	if (axesonside || y_xaxis < 0)
		y_xaxis=0;
		
	if (x_numticks > 0)
		x_tickstep = round(xlen/xtenpow)/x_numticks*xtenpow;
	if (y_numticks > 0)
		y_tickstep = round(ylen/ytenpow)/y_numticks*ytenpow;

	// Local coordinate system in the plot is 0..1, scale/translate to fit in -1..1 box
	double xscale = 2;
	double yscale = 2;
	double xtrans = -xscale/2;
	double ytrans = -yscale/2;
	if (ticktext && y_xaxis < ticktextsize*5) {
		yscale -= ticktextsize*10;
		ytrans += ticktextsize*8;
	}
	if (ticktext && x_yaxis < ticktextsize*5) {
		xscale -= ticktextsize*15;
		xtrans += ticktextsize*12;
	}
	if (title.length() > 0) 
		yscale -= labelstextsize*2.5;
	if (xlabel.length() > 0) {
		yscale -= labelstextsize*6;
		ytrans += labelstextsize*3;
		
	}
	if (ylabel.length() > 0) {
		xscale -= labelstextsize*6;
		xtrans += labelstextsize*3;
	}
	glTranslatef(xtrans,ytrans,0);
	glScalef(xscale,yscale,0);
	
	// Draw axes
	glLineWidth(1);

	glColor4fv(&axiscolor[0]);
	glBegin(GL_LINES);
	glVertex2d(0,y_xaxis);
	glVertex2d(1,y_xaxis);
	glVertex2d(x_yaxis,0);
	glVertex2d(x_yaxis,1);
	glEnd();
	
	// TODO better way to find tickstep and mintick
	double minxtick=0;
	double minytick=0;
	if (!axesonside && minx < 0) {
		for (double x = 0; x >= minx-epsilon; x -= x_tickstep)
			minxtick = x;
	} else if (fabs(minx) > epsilon)
		minxtick=ceil(minx/tenpow(minx)*10.0)*tenpow(minx)/10.0;
	if (!axesonside && miny < 0) {
		for (double y = 0; y >= miny-epsilon; y -= y_tickstep)
			minytick = y;
	} else if (fabs(miny) > epsilon)
		 	minytick=ceil(miny/tenpow(miny)*10.0)*tenpow(miny)/10.0;

	for (double x = minxtick; x <= maxx+epsilon; x += x_tickstep) {
		glBegin(GL_LINES);
		glColor4fv(&axiscolor[0]);
		glVertex2d((x-minx)/xlen,y_xaxis);
		glVertex2d((x-minx)/xlen,y_xaxis-0.01);
		if (grid) {
			glColor4fv(&gridcolor[0]);
			glVertex2d((x-minx)/xlen,0);
			glVertex2d((x-minx)/xlen,1);
		}
		glEnd();
		if (ticktext) {
			std::stringstream ss;
			if (fabs(x) < epsilon)
				ss << 0;
			else if (fabs(xlen) < 1 || fabs(xlen) >= 1000) {
				ss << std::fixed << std::setprecision(1) << (x/xtenpow);
				if (x + x_tickstep > maxx)
					ss << "e" << tenpow2(xlen);
			} else if (x >= 10)
				ss << std::fixed << std::setprecision(0) << x;
			else
				ss << std::fixed << std::setprecision(1) << x;
			glPushMatrix();
			glColor4fv(&textcolor[0]);
			if (!axesonside && fabs(x) < epsilon) {
				// Prevent two zeros from crossing x- and y-axis
				glTranslated(x_yaxis-0.02,y_xaxis-0.02,0);
				uglyfont::drawFont("0",ticktextsize,uglyfont::RIGHT | uglyfont::BELOW);
			} else {
				glTranslated((x-minx)/xlen,y_xaxis-0.02,0);
				uglyfont::drawFont(ss.str().c_str(),ticktextsize,uglyfont::CENTER | uglyfont::BELOW);
			}
			glPopMatrix();
		}
	}
	for (double y = minytick; y <= maxy+epsilon; y += y_tickstep) {
		glBegin(GL_LINES);
		glColor4fv(&axiscolor[0]);
		glVertex2d(x_yaxis,(y-miny)/ylen);
		glVertex2d(x_yaxis-0.01,(y-miny)/ylen);
		if (grid) {
			glColor4fv(&gridcolor[0]);
			glVertex2d(0,(y-miny)/ylen);
			glVertex2d(1,(y-miny)/ylen);
		}
		glEnd();
		if (ticktext) {
			std::stringstream ss;
			if (fabs(y) < epsilon)
				ss << 0;
			else if (fabs(ylen) < 1 || fabs(ylen) >= 1000) {
				ss << std::fixed << std::setprecision(1) << (y/ytenpow);
				if (y+y_tickstep > maxy)
					ss << "e" << tenpow2(ylen);
			} else if (y >= 10)
				ss << std::fixed << std::setprecision(0) << y;
			else
				ss << std::fixed << std::setprecision(1) << y;
			glPushMatrix();
			glColor4fv(&textcolor[0]);
			if (axesonside || fabs(y) > epsilon) {
				glTranslated(x_yaxis-0.02,(y-miny)/ylen,0);
				uglyfont::drawFont(ss.str().c_str(),ticktextsize,uglyfont::RIGHT | uglyfont::MIDDLE);
			}
			glPopMatrix();
		}
	}

	// Draw data
	for (int j = 0; j < datasets.size(); j++) {
		if (datasets[j]->line != LINE_NONE) {
			glLineWidth(datasets[j]->linewidth);
			glColor4fv(&datasets[j]->color[0]);
			if (datasets[j]->line == LINE_POINT)
				glBegin(GL_POINTS);
			else if (datasets[j]->line == LINE_ODD)
				glBegin(GL_LINES);
			else 
				glBegin(GL_LINE_STRIP);
			for (int i = 0; i < datasets[j]->x.size(); i++) {
				glVertex2d((datasets[j]->x[i]-minx)/xlen,(datasets[j]->y[i]-miny)/ylen);
			}
			glEnd();
			
		}
		if (datasets[j]->marker != MARKER_NONE && xlen > 0 && ylen > 0) {
			glLineWidth(datasets[j]->linewidth);
			glColor4fv(&datasets[j]->color[0]);
			for (int i = 0; i < datasets[j]->x.size(); i++) {
				drawMarker(datasets[j]->marker,(datasets[j]->x[i]-minx)/xlen,(datasets[j]->y[i]-miny)/ylen);
			}
		}
		// Show value (text printout) at specific data index
		if (datasets[j]->show_value_index >= 0 && datasets[j]->show_value_index < datasets[j]->x.size() && xlen > 0 && ylen > 0) {
			int i = datasets[j]->show_value_index;
			int justify = uglyfont::CENTER;
			if (i == datasets[j]->x.size()-1)
				justify = uglyfont::RIGHT;
			if (datasets[j]->marker == MARKER_NONE)
				drawMarker(MARKER_CROSS,(datasets[j]->x[i]-minx)/xlen,(datasets[j]->y[i]-miny)/ylen);
			glLineWidth(textwidth);
			glColor4fv(&textcolor[0]);
			std::stringstream sy,sx;
			sy << "y = " << datasets[j]->y[i];
			sx << "x = " << datasets[j]->x[i];
			glPushMatrix();
			glTranslated((datasets[j]->x[i]-minx)/xlen,(datasets[j]->y[i]-miny)/ylen+ticktextsize,0);
			uglyfont::drawFont(sy.str().c_str(),ticktextsize, justify | uglyfont::ABOVE);
			glTranslated(0,-ticktextsize*2,0);
			uglyfont::drawFont(sx.str().c_str(),ticktextsize, justify | uglyfont::BELOW);
			glPopMatrix();
		}
	}
	
	glPopMatrix();
	
	// Draw title and labels
	glLineWidth(textwidth);
	glColor4fv(&textcolor[0]);
	glPushMatrix();
	glTranslated(0,1-labelstextsize*2,0);
	uglyfont::drawFont(title.c_str(),labelstextsize*2,uglyfont::CENTER | uglyfont::MIDDLE);
	glPopMatrix();
	
	glPushMatrix();
	glTranslated(0,-1+labelstextsize*2,0);
	uglyfont::drawFont(xlabel.c_str(),labelstextsize*2,uglyfont::CENTER | uglyfont::MIDDLE);
	glPopMatrix();
	
	glPushMatrix();
	glTranslated(-1+labelstextsize*2,0,0);
	glRotated(90.0,0,0,1);
	uglyfont::drawFont(ylabel.c_str(),labelstextsize*2,uglyfont::CENTER | uglyfont::MIDDLE);
	glPopMatrix();
	
	// Draw legend
	if (showlegend) {
		for (int j = 0; j < datasets.size(); j++) {
			glPushMatrix();
			
			glTranslated(x_legend,y_legend-labelstextsize*4*j,0);
			glLineWidth(datasets[j]->linewidth);
			glColor4fv(&datasets[j]->color[0]);
			glBegin(GL_LINES);
			glVertex2d(0,0);
			glVertex2d(labelstextsize*4.0,0);
			glEnd();
			if (datasets[j]->marker != MARKER_NONE) {
				drawMarker(datasets[j]->marker,labelstextsize*2,0);
			}
			
			glLineWidth(textwidth);
			glColor4fv(&textcolor[0]);
			glTranslated(labelstextsize*6.0,0,0);
			if (datasets[j]->label.length() > 0)
				uglyfont::drawFont(datasets[j]->label.c_str(),labelstextsize*2,uglyfont::MIDDLE);
			else {
				std::stringstream ss;
				ss << j;
				uglyfont::drawFont(ss.str().c_str(),labelstextsize*2,uglyfont::MIDDLE);
			}
			glPopMatrix();
		}
	}
}

void LinePlot2D::drawMarker(MarkerType marker, double x, double y)
{
	const double markersize = 0.01;
	switch (marker) {
		case MARKER_CROSS:
			glBegin(GL_LINES);
			glVertex2d(x-markersize,y-markersize);
			glVertex2d(x+markersize,y+markersize);
			glVertex2d(x+markersize,y-markersize);
			glVertex2d(x-markersize,y+markersize);
			glEnd();
			break;
		case MARKER_SQUARE:
			glBegin(GL_LINE_STRIP);
			glVertex2d(x-markersize,y-markersize);
			glVertex2d(x-markersize,y+markersize);
			glVertex2d(x+markersize,y+markersize);
			glVertex2d(x+markersize,y-markersize);
			glVertex2d(x-markersize,y-markersize);
			glEnd();
			break;
		case MARKER_DIAMOND:
			glBegin(GL_LINE_STRIP);
			glVertex2d(x,y-markersize);
			glVertex2d(x-markersize,y);
			glVertex2d(x,y+markersize);
			glVertex2d(x+markersize,y);
			glVertex2d(x,y-markersize);
			glEnd();
			break;
		case MARKER_PLUS:
			glBegin(GL_LINES);
			glVertex2d(x,y-markersize);
			glVertex2d(x,y+markersize);
			glVertex2d(x-markersize,y);
			glVertex2d(x+markersize,y);
			glEnd();
			break;
	}
}
