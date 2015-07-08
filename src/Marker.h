/*
 * 'Marker' class: Store information / do calculations on individual markers.
 *
 * Author: Owain Jones <odj@aber.ac.uk> <github.com/erinaceous>
 */

#ifndef QUADTARGETPLUSPLUS_MARKER_H
#define QUADTARGETPLUSPLUS_MARKER_H

#include <opencv2/opencv.hpp>

namespace qtpp {
    static constexpr double MARKER_ELLIPTICAL_RATIO = 0.1;
    static constexpr double MARKER_INNER_RADIUS = 0.1;
    static constexpr double MARKER_CENTRAL_RADIUS = 0.2;
    static constexpr double MARKER_OUTER_RADIUS = 0.3;

    class Marker {
    protected:
        int left_x, right_x, top_y, bottom_y;
    public:
        Marker(const int top_y, const int bottom_y,
               const int left, const int right);

        double radius();

        int getLeftX();
        int getRightX();
        int getCenterX();
        int getTopY();
        int getBottomY();
        int getCenterY();
        int getWidth();
        int getHeight();
        float getRatio();

        void setTopY(int top);
        void setBottomY(int bottom);
        void setLeftX(int left);
        void setRightX(int right);

        void expand(Marker other);
        std::string toString();

        bool invalid = false;

        static Marker expand(Marker m1, Marker m2);
        static double angle(Marker m1, Marker m2);
        static int distance(Marker m1, Marker m2);
    };

}

#endif //QUADTARGETPLUSPLUS_MARKER_H
