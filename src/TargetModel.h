//
// Created by owain on 23/06/15.
//

#ifndef QUADTARGETPLUSPLUS_TARGETMODEL_H
#define QUADTARGETPLUSPLUS_TARGETMODEL_H

#include <opencv2/opencv.hpp>

using namespace cv;


class Marker {
    /**
     * Model of an individual marker.
     */
    public:
        // Default marker circle radii at 1:1 scale (measured in pixels)
        static constexpr double INNER_RADIUS = 55.0;
        static constexpr double MIDDLE_RADIUS = 90.0;
        static constexpr double OUTER_RADIUS = 130.0;
        static constexpr double SCALE = 1.0;

        // Distance between dark and light areas of marker
        static constexpr double CONTRAST = 1.0;

        Marker();
        Marker(
                double inner, double middle, double outer,
                double contrast
        );

        Mat render(double scale=SCALE);
        void render_in_place(
                Mat &mat, Point pos, double scale=SCALE
        );

    private:
        double inner_radius, middle_radius, outer_radius, contrast;
        Point center, size;
};


class Target {
    /**
     * Target model. Describes the positions of markers within the target.
     */

    public:
        static constexpr double POSITIONS[6][3] = {
                // Distance from origin, scale
                // X,       Y,      scale
                {0.0,       -185.0, 1.0},   // Top center
                {-366.0,    182.0,  1.0},   // Bottom left
                {366.0,     182.0,  1.0},   // Bottom right
                {0.0,       144.0,  1.0},   // Small, top center
                {-36.6,     180.0,  1.0},   // Small, bottom left
                {-36.6,     180.0,  1.0}    // Small, bottom right
        };
        static constexpr double SCALE = 1.0;
        static constexpr double ANGLE = 0.0;
        static constexpr double CONTRAST = Marker::CONTRAST;

        Target();
        Target(double scale, double angle);
        Target(double scale, double angle, double contrast);

        Mat render();
        void render_in_place(Mat mat, double position[2]);

    private:
        double positions[][3];
        double scale;
        double angle;
        double contrast;
};





#endif //QUADTARGETPLUSPLUS_TARGETMODEL_H
