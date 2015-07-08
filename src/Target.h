/*
 * Store information / do calculations on groups of markers. A valid target is
 * one which has three markers that are equally spaced apart and of similar
 * sizes.
 *
 * Author: Owain Jones <odj@aber.ac.uk> <github.com/erinaceous>
 */

#ifndef QUADTARGETPLUSPLUS_TARGET_H
#define QUADTARGETPLUSPLUS_TARGET_H

#include <opencv2/opencv.hpp>
#include "Marker.h"

namespace qtpp {

    class Target {
    private:
        std::vector<Marker> markers;
    public:
        Target();

        void addMarker(Marker m);

        std::vector<Marker> getMarkers();

        bool isValid();

    };

}


#endif //QUADTARGETPLUSPLUS_TARGET_H
