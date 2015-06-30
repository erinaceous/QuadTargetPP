//
// Created by owain on 24/06/15.
//

#ifndef QUADTARGETPLUSPLUS_TARGETFINDER_H
#define QUADTARGETPLUSPLUS_TARGETFINDER_H

#define RADIANS_TO_DEGREES 57.2957795
//#define DEBUG_EXTREME

#ifdef DEBUG_EXTREME
    #define IMDEBUG(label, img) imshow(label, img)
    #define DEBUGPRINT(str) cerr << str << endl
#else
    #define IMDEBUG(label, img)
    #define DEBUGPRINT(str)
#endif

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

// TODO: Give the TargetFinder class its own instance versions of these vars.

static double SAMPLE_EVERY = 1.0 / 30.0;     // Try and keep this FPS
static double UPDATE_EVERY = 1.0 / 10.0;
static double DECAY_RATE = 0.1;  // Decay for confidence values
static double CONTRAST = 0.0;

static double CENTER_ELLIPTICAL_RATIO = 0.5;
static double CENTER_DISTANCES_RATIO = 0.6;
static double MIN_DISTANCE_RATIO = CENTER_DISTANCES_RATIO;
static double MAX_DISTANCE_RATIO = 1.0 / CENTER_DISTANCES_RATIO;
static double TARGET_SIZE_RATIO = 0.5;
static double MIN_TARGET_SIZE_RATIO = TARGET_SIZE_RATIO;
static double MAX_TARGET_SIZE_RATIO = 1.0 / TARGET_SIZE_RATIO;
static double LR_PAIR_RATIO_THRESHOLD = 0.7;
static double PAIR_CENTER_RATIO_THRESHOLD = 0.3;
static double LR_RATIO_THRESHOLD = 0.5;
static double LR_BLACK_TO_CENTER_RATIO_THRESHOLD = 0.3;
static double VALID_ROWS_TO_CENTER_RATIO = 0.1;


uchar get_pixel(Mat *mat, int y, int x, int c);
void set_pixel(Mat *mat, int y, int x, int c, uchar v);


class FoundMarker {
    /**
     * Used to store data about a marker detected within an image.
     * (Plain Old Data class)
     */
    public:
        Point position;
        double scale;
        double confidence;
        Mat* mat;

        FoundMarker(
                int y, int center_left, int center_right, int left_x,
                int right_x
        );
        String csv();
        double get_confidence();
        Mat render();
        static double angle(FoundMarker t1, FoundMarker t2);
        int get_center_radius();
        int get_radius();
        int get_center_x();
        int get_center_y();
        bool is_y_close(int y);
        bool is_x_close(int x);
        void expand(
            int y, int center_left, int center_right, int left_x,
            int right_x
        );
        int get_valid_y_count();
        void mark_center(Mat* mat);

    private:
        int first_max_y = 0;
        int last_max_y = 0;
        int first_valid_y = 0;
        int last_valid_y = 0;
        int center_left_x = 0;
        double center_x;
        int center_right_x = 0;
        int left_x = 0;
        int right_x = 0;

        int get_max_y_count();
};


class FoundTarget {
    /**
    * Store information about a target (3+ markers) detected within an image.
    */
    public:
        vector<FoundMarker> markers;
        double get_size();
        double get_rotation();
        double get_center_x();
        double get_center_y();
        int get_image_size_x();
        int get_image_size_y();
        void add_marker(FoundMarker marker);
        double get_average_center_width();
        double get_average_radius();
        void calculate_geometry();
        bool has_three_markers();
        bool is_close(FoundMarker test_marker);
        void mark_center(Mat* mat);
        FoundTarget(int x, int y);
        string toString();

    private:
        FoundMarker *corner;
        double corner_angle = 0;
        double center_x = 0;
        double center_y = 0;
        double rotation = 0;
        double size = 0;
        int image_size_x = 0;
        int image_size_y = 0;
};


class ConfidenceMatrix {
    /**
     * 2D OpenCV matrix (grayscale image) calculated from the "confidence" of
     * an input image plus specified telemetry data.
     * Given angles and position of previously detected target, we can predict
     * where the target should be in the current frame and use it to generate a
     * "confidence matrix" around where the target would be.
     * The confidence values are doubles from 0.0 to 1.0.
     */

public:
    static Mat generate(
            Point position, double angle, double scale
    );
};


class TargetFinder {
    /**
     * Target recognition and tracking class. Operates on OpenCV Mats. Expects a
     * grayscale image or colour (3-channel) image.
     */
public:
    TargetFinder(bool headless);
    vector<FoundTarget> do_target_recognition(Mat* input_mat, Mat* output_mat);

private:
    Mat* input_mat;
    double confidence = 1.0;
    double error = 0.0;
    int left_black, left_white, center_black, center_white, right_black,
        right_white, left_erosion, right_erosion;
    vector<FoundMarker> markers;
    vector<FoundTarget> final_targets;
    bool found_target = false;
    bool headless = false;

    bool check_sequence(int w, int h);
    bool check_vertical_sequence(
        int w, int h, int radius, Mat* input_mat, Mat* output_mat
    );
    void update_targets(
            int y, int center_start, int center_end, int left_x, int right_x
    );
    void calculate_geometry();
    void validate_targets();
    void group_targets();
    void refine_target_vertically(Mat* input_mat, Mat* output_mat);
    Mat recognise_target(Mat* input_mat);
    void colour_horizontal_target(int w, int h, Mat* mat);
    void colour_vertical_target(int w, int h, Mat* mat);
    void mark_target_centers(Mat* mat);
};


#endif //QUADTARGETPLUSPLUS_TARGETFINDER_H
