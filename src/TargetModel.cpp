//
// Created by owain on 23/06/15.
//

#include "TargetModel.h"
#include <opencv2/opencv.hpp>
#include <math.h>

using namespace cv;


Marker::Marker(double inner, double middle, double outer,
               double contrast) {
    this->inner_radius = inner;
    this->middle_radius = middle;
    this->outer_radius = outer;
    this->contrast = contrast;
    this->center = Point(outer, outer);
    this->size = Point(outer * 2, outer * 2);
}


Marker::Marker() {
    this->inner_radius = Marker::INNER_RADIUS;
    this->middle_radius = Marker::MIDDLE_RADIUS;
    this->outer_radius = Marker::OUTER_RADIUS;
    this->contrast = Marker::CONTRAST;
    this->center = Point(this->outer_radius, this->outer_radius);
    this->size = Point(this->outer_radius * 2, this->outer_radius * 2);
}


void Marker::render_in_place(Mat &mat, Point pos, double scale) {
    circle(mat, pos, this->outer_radius * scale, 1 - this->contrast, -1);
    circle(mat, pos, this->middle_radius * scale, this->contrast, -1);
    circle(mat, pos, this->inner_radius * scale, 1 - this->contrast, -1);
}


Mat Marker::render(double scale) {
    int width = this->size.x * scale + 1;
    int height = this->size.y * scale + 1;
    Point center = Point(
            this->center.x * scale, this->center.y * scale
    );
    Mat mat = Mat::ones(height, width, DataType<double>::type);
    this->render_in_place(mat, this->center, scale);
    return mat;
}


Target::Target(double scale, double angle, double contrast) {
    this->scale = scale;
    this->angle = angle;
    this->contrast = contrast;
    /* this->size = Point(
            max()
    )*/
}


Target::Target(double scale, double angle) {
    Target(scale, angle, Target::CONTRAST);
}


Target::Target() {
    Target(Target::SCALE, Target::ANGLE);
}
