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
#include "TargetFinder.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace std;
using namespace cv;

int main() {
    boost::property_tree::ptree pt;
    char* config_path = getenv("QUADTARGET_CONFIG");
    if(config_path == NULL) {
        boost::property_tree::ini_parser::read_ini("config.ini", pt);
    } else {
        boost::property_tree::ini_parser::read_ini(config_path, pt);
    }

    bool headless = pt.get<bool>("gui.headless");
    bool saveTmp = pt.get<bool>("gui.saveTmp");
    bool saveToFiles = pt.get<bool>("gui.saveToFiles");

    VideoCapture cap(pt.get<int>("camera.index"));
    if(!cap.isOpened()) {
        cerr << "Couldn't open webcam" << endl;
        return -1;
    }
    cap.set(CV_CAP_PROP_FRAME_WIDTH, pt.get<int>("camera.width"));
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, pt.get<int>("camera.height"));
    cap.set(CV_CAP_PROP_FPS, pt.get<int>("camera.fps"));

    TargetFinder tf = TargetFinder(headless);

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
            if(!headless) {
                imshow("input", frame);
                imshow("processed", processed);
                imshow("output", output);
            }
            if(saveTmp) {
                imwrite("/tmp/target.png", frame);
            }
            if(saveToFiles) {
                imwrite("input.jpg", frame);
                imwrite("processed.jpg", processed);
                imwrite("output.jpg", output);
            }
            count++;
        } else {
            DEBUGPRINT("couldn't get webcam frame?");
        }
        running = !(waitKey(1) == 'q');
    }

    return 0;
}
