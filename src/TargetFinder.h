/*
 * Target tracking algorithm.
 *
 * Author: Owain Jones <odj@aber.ac.uk> <github.com/erinaceous>
 */

#ifndef QUADTARGETPLUSPLUS_TARGETFINDER_H
#define QUADTARGETPLUSPLUS_TARGETFINDER_H

#include <opencv2/opencv.hpp>
#include "Target.h"

namespace qtpp {

    class TargetFinder {
        private:
            /* Changeable parameters */
            float elliptical_ratio = 1.0;
            float inner_radius = 0.1;
            float central_radius = 0.2;
            float outer_radius = 0.3;
            float threshold = 0.0;
            float max_variance = 10.0;
            float alpha = 0.3;
            float omega = 0.0001;
            float window = 5.0;
            float black_white_ratio_threshold = 0.3;
            int inner_marker_distance = 15;
            float min_marker_ratio = 0.1;
            float max_marker_ratio = 15.0;
            int min_pixel_count = 2;
            int tolerance = 10;

            float confidence = 1.0;
            float error = 0.0;

            std::vector<Marker> markers;

        public:
            void setParam(std::string param, void* value);
            std::string toString();
            std::vector<Target> doTargetRecognition(
                    cv::Mat *inputMat, cv::Mat *outputMat=NULL,
                    bool debug=false);
    };

}


#endif //QUADTARGETPLUSPLUS_TARGETFINDER_H
