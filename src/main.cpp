/*
 * Runs the target recognition algorithm on webcam feed or an image stream fed
 * from stdin.
 * Reports information on target position, orientation and scale as
 * comma-separated values (ASCII) in a specified UNIX socket.
 * This socket acts like a UDP one -- no buffering; if you don't read data when
 * it's available it won't keep it. Multiple programs can consume the data
 * (broadcast/multicast socket).
 */

#include <iostream>
#include <opencv2/opencv.hpp>
#include "TargetModel.h"
#include "TargetFinder.h"

using namespace std;
using namespace cv;

int main() {
    VideoCapture cap(0);
    if(!cap.isOpened()) {
        cerr << "Couldn't open webcam" << endl;
        return -1;
    }

    TargetFinder tf = TargetFinder();

    bool running = true;
    Mat frame;
    Mat processed;
    Mat output;
    int count = 0;
    while(running) {
        if(cap.read(frame)) {
            cvtColor(frame, processed, CV_BGR2GRAY);
            threshold(processed, processed, 128, 255, THRESH_OTSU);
            frame.copyTo(output);
            vector<FoundTarget> ft = tf.do_target_recognition(&processed, &output);
            for(int i = 0; i < ft.size(); i++) {
                cout << "frame=" << count << ",t=" << i << ",";
                cout << ft.at(i).toString() << endl;
            }
            #ifndef DEBUG_EXTREME
                imshow("input", frame);
                imshow("processed", processed);
                imshow("output", output);
            #endif
            count++;
        } else {
            DEBUGPRINT("couldn't get webcam frame?");
        }
        running = !(waitKey(1) == 'q');
    }

    return 0;
}
