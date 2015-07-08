#include "TargetFinder.h"

/*
 * Ran in place of main.cpp when testing bits of code.
 */

#define videofile "/windows/Dropbox/Aber/MSc_Dissertation/data/synthetic_targets-3.mp4"
// #define videofile "/tmp/sd/flight_1435671703.48.h264"
// #define videofile "/tmp/sd/flight_1435933737.8.h264"

int main() {
    #ifndef videofile
        cv::VideoCapture cap(0);
    #else
        cv::VideoCapture cap(videofile);
    #endif
    cap.set(CV_CAP_PROP_CONVERT_RGB, false);
    qtpp::TargetFinder tf;
    // cv::Mat input = cv::imread("/tmp/target.png", cv::IMREAD_GRAYSCALE);
    cv::Mat output;
    cv::Mat frame;
    // input.copyTo(output);
    while(true) {
        if(cap.read(frame)) {
            frame.copyTo(output);
            tf.doTargetRecognition(&frame, &output);
            cv::imshow("input", frame);
            cv::imshow("output", output);
            cv::waitKey(1);
        } else {
            #ifdef videofile
                cap.open(videofile);
            #endif
        }
    }
    return 0;
}
