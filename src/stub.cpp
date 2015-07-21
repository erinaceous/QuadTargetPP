#include "TargetFinder.h"
#include "ImageCycler.h"

/*
 * Ran in place of main.cpp when testing bits of code.
 */

// #define videofile "/windows/Dropbox/Aber/MSc_Dissertation/data/synthetic_targets-3.mp4"
// #define videofile "/tmp/sd/flight_1435671703.48.h264"
// #define videofile "/tmp/sd/flight_1435933737.8.h264"
// #define videofile "/everything/quad_data/test/Target_2.png"
#define videofile 1
// #define imagedir "/everything/quad_data/test"
#define DEBUG_EXTREME true

int main() {
    #ifdef imagedir
        qtpp::ImageCycler cap(imagedir);
    #else
        cv::VideoCapture cap(videofile);
        cap.set(CV_CAP_PROP_CONVERT_RGB, false);
    #endif
    qtpp::TargetFinder tf;
    cv::Mat output;
    cv::Mat frame;
    // input.copyTo(output);
    while(true) {
        if(cap.read(frame)) {
            frame.copyTo(output);
            tf.doTargetRecognition(&frame, &output, DEBUG_EXTREME);
            cv::imshow("input", frame);
            cv::imshow("output", output);
            #ifdef imagedir
                cv::waitKey(0);
            #else
                cv::waitKey(1);
            #endif
        } else {
            cap.open(videofile);
        }
    }
    return 0;
}
