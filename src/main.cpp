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


void set_targetfinder_config(boost::property_tree::ptree pt) {
    #define pg(param) pt.get<double>("params." #param)
    CENTER_ELLIPTICAL_RATIO = pg(center_elliptical_ratio);
    CENTER_DISTANCES_RATIO = pg(center_distances_ratio);
    MIN_DISTANCE_RATIO = pg(min_distance_ratio);
    MAX_DISTANCE_RATIO = pg(max_distance_ratio);
    TARGET_SIZE_RATIO = pg(target_size_ratio);
    MIN_TARGET_SIZE_RATIO = pg(min_target_size_ratio);
    MAX_TARGET_SIZE_RATIO = pg(max_target_size_ratio);
    LR_PAIR_RATIO_THRESHOLD = pg(lr_pair_ratio_threshold);
    PAIR_CENTER_RATIO_THRESHOLD = pg(pair_center_ratio_threshold);
    LR_RATIO_THRESHOLD = pg(lr_ratio_threshold);
    LR_BLACK_TO_CENTER_RATIO_THRESHOLD = pg(lr_black_to_center_ratio_threshold);
    VALID_ROWS_TO_CENTER_RATIO = pg(valid_rows_to_center_ratio);
    CONTRAST = pg(contrast);
    DECAY_RATE = pg(decay_rate);
    SAMPLE_EVERY = pg(sample_every);
    UPDATE_EVERY = pg(update_every);
}


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

    set_targetfinder_config(pt);
    TargetFinder tf = TargetFinder(headless);

    bool running = true;
    bool otsu = pt.get<bool>("params.otsu_threshold");
    uchar binary_threshold = pt.get<uchar>("params.binary_threshold");
    Mat frame;
    Mat processed;
    Mat output;
    int count = 0;
    while(running) {
        if(cap.read(frame)) {
            cvtColor(frame, processed, CV_BGR2GRAY);
            if(otsu) {
                threshold(processed, processed, 128, 255, THRESH_OTSU);
            } else {
                threshold(processed, processed, binary_threshold, 255, THRESH_BINARY);
            }
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
