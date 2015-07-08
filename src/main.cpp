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
#include "TargetFinder_orig.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace std;
using namespace cv;


void set_targetfinder_config(boost::property_tree::ptree pt) {
    #define pg(param) pt.get<double>("params." #param)
    CenterElipticalRatio = pg(center_elliptical_ratio);
    CenterDistancesRatio = pg(center_distances_ratio);
    MINDISTANCERATIO = pg(min_distance_ratio);
    MAXDISTANCERATIO = pg(max_distance_ratio);
    TargetSizeRatio = pg(target_size_ratio);
    MINTARGETSIZERATIO = pg(min_target_size_ratio);
    MAXTARGETSIZERATIO = pg(max_target_size_ratio);
    LRPairRatioThreshold = pg(lr_pair_ratio_threshold);
    pairCenterRatioThreshold = pg(pair_center_ratio_threshold);
    LRRatioThreshold = pg(lr_ratio_threshold);
    LRBlacktoCenterRatioThreshold = pg(lr_black_to_center_ratio_threshold);
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
    bool showProcessed = pt.get<bool>("gui.showProcessed");
    bool loop_video = false;
    int wait_key = pt.get<int>("gui.waitKey");
    string video_file = pt.get<string>("camera.file");
    VideoCapture cap;

    if(video_file.compare("none") == 0) {
        cap = VideoCapture(pt.get<int>("camera.index"));
    } else {
        cap = VideoCapture(video_file);
        loop_video = true;
    }
    if(!cap.isOpened()) {
        cerr << "Couldn't open webcam or video" << endl;
        return -1;
    }
    cap.set(CV_CAP_PROP_FRAME_WIDTH, pt.get<int>("camera.width"));
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, pt.get<int>("camera.height"));
    cap.set(CV_CAP_PROP_FPS, pt.get<int>("camera.fps"));
    cap.set(CV_CAP_PROP_CONVERT_RGB, false);

    set_targetfinder_config(pt);
    TargetFinder tf = TargetFinder(headless);

    int threshold_mode = 0;
    int threshold_value = pt.get<int>("threshold.value");
    int threshold_kernel = pt.get<int>("threshold.kernel_size");
    int threshold_c = pt.get<int>("threshold.c");
    if(pt.get<string>("threshold.mode").compare("otsu") == 0) {
        threshold_mode = 1;
    } else if(pt.get<string>("threshold.mode").compare("fixed") == 0) {
        threshold_mode = 2;
    } else if(pt.get<string>("threshold.mode").compare("mean") == 0) {
        threshold_mode = 3;
    } else if(pt.get<string>("threshold.mode").compare("gaussian") == 0) {
        threshold_mode = 4;
    }

    bool running = true;
    Mat frame;
    Mat yuv[3];
    Mat processed;
    Mat output;
    int count = 0;
    while(running) {
        DEBUGPRINT("count: " << count);
        if(cap.read(frame)) {
            // cvtColor(frame, processed, CV_BGR2GRAY);
            split(frame, yuv);
            processed = yuv[0];
            switch(threshold_mode) {
                case 1:
                    threshold(processed, processed, 128, 255, THRESH_OTSU);
                    break;
                case 2:
                    threshold(processed, processed, threshold_value, 255, THRESH_BINARY);
                    break;
                case 3:
                    adaptiveThreshold(processed, processed, 255,
                                      CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY,
                                      threshold_kernel, threshold_c);
                    break;
                case 4:
                    adaptiveThreshold(processed, processed, 255,
                                      CV_ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY,
                                      threshold_kernel, threshold_c);
                    break;
                case 0:
                default:
                    break;
            }
            frame.copyTo(output);
            vector<FoundTarget> ft = tf.doTargetRecognition(&processed, &output);
            for(int i = 0; i < ft.size(); i++) {
                cout << "frame=" << count << ",t=" << i << ",";
                cout << ft.at(i).toString() << endl;
            }
            if(!headless) {
                imshow("input", frame);
                if(showProcessed) {
                    imshow("processed", processed);
                }
                imshow("output", output);
            }
            if(saveTmp) {
                imwrite("/tmp/target.png", frame);
            }
            if(saveToFiles) {
                imwrite("input.jpg", frame);
                if(showProcessed) {
                    imwrite("processed.jpg", processed);
                }
                imwrite("output.jpg", output);
            }
            count++;
        } else {
            DEBUGPRINT("couldn't get webcam frame? Or we've reached end of video.");
            if(loop_video) {
                cap.open(video_file);
            }
        }
        if(!headless) {
            running = !(waitKey(wait_key) == 'q');
        }
    }

    return 0;
}
