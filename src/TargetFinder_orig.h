//
// Created by owain on 24/06/15.
//

#ifndef QUADTARGETPLUSPLUS_TARGETFINDER_H
#define QUADTARGETPLUSPLUS_TARGETFINDER_H

#define RADIANS_TO_DEGREES 57.29577951308232
// #define DEBUG_EXTREME

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

/* static double CENTER_ELLIPTICAL_RATIO = 0.5;
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
static double VALID_ROWS_TO_CENTER_RATIO = 0.1; */

// allowable 'eliptical' ratio of center pixels
static double CenterElipticalRatio = 0.5;
// allowable error in the ratio between center distances in terms of the size of the centers
static double CenterDistancesRatio = 0.6;
// distance apart of two target parts in terms of whole target part size. Perfect would be 2.
static double MINDISTANCERATIO = (CenterDistancesRatio);  // plus/minus 1 center width allowable
static double MAXDISTANCERATIO = (1/CenterDistancesRatio);
//allowable error in the ratio between sizes of targets forming a group
static double TargetSizeRatio = 0.5;
// allowable size variation for targets within one group
static double MINTARGETSIZERATIO = 1*TargetSizeRatio;
static double MAXTARGETSIZERATIO = 1/TargetSizeRatio;
// CheckSequence parameters
// variation between left and right pairs
static double LRPairRatioThreshold = 0.7;
// variation between center and outer band pairs
static double pairCenterRatioThreshold = 0.3;
// variation between left and right black or white bands
static double LRRatioThreshold = 0.5;
// variation between left/right black and center
static double LRBlacktoCenterRatioThreshold = 0.3;
// the minimum ratio of valid horizontal pattern rows compared to the height of a target center.
static double VALID_ROWS_TO_CENTER_RATIO = 0.1;


uchar get_pixel(Mat *mat, int y, int x, int c);
void set_pixel(Mat *mat, int y, int x, int c, uchar v);
bool in_bounds(Mat *mat, int y, int x, int c);


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
                int y, int centerleft, int centerright, int left_x,
                int right_x
        );
        String csv();
        double get_confidence();
        Mat render();
        static double angle(FoundMarker t1, FoundMarker t2);
        double centerRadius();
        double radius();
        double getCenterX();
        double getCenterY();
        bool yClose(int y);
        bool xClose(int x);
        void expand(
            int y, int centerleft, int centerright,
            int left_x, int right_x
        );
        int getValidyCount();
        void markCenter(Mat *mat);
        int getFirstValidy() { return firstValidy; }
        int getLastValidy() { return lastValidy; }
    private:
        int firstmaxy = 0;
        int lastmaxy = 0;
        int firstValidy = 0;
        int lastValidy = 0;
        int centerleftx = 0;
        float centerx = 0.9;
        int centerrightx = 0;
        int leftx = 0;
        int rightx = 0;
};


class FoundTarget {
    /**
    * Store information about a target (3+ markers) detected within an image.
    */
    public:
        vector<FoundMarker> markers;
        void addMarker(FoundMarker marker);
        double getAveCenterWidth();
        double getAveRadius();
        void calculateGeometry();
        bool hasThreeMarkers();
        bool isClose(FoundMarker testMarker);
        void markCenter(Mat *mat);
        FoundTarget(int x, int y);
        string toString();
        FoundMarker *corner;
        bool invalid = false;
    private:
        double cornerAngle = 0;
        double centerx = 0;
        double centery = 0;
        double rotation = 0;
        double size = 0;
        int imagesizex = 0;
        int imagesizey = 0;
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
    TargetFinder(bool headless=false);
    vector<FoundTarget> doTargetRecognition(Mat *input_mat, Mat *output_mat);

protected:
    Mat* input_mat;
    double confidence = 1.0;
    double error = 0.0;
    int leftBlack, leftWhite, centerBlack, rightBlack, rightWhite;
    vector<FoundMarker> markers;
    vector<FoundTarget> final_targets;
    bool found_target = false;
    bool headless = false;

    bool checkSequence(int w, int h);
    bool checkVerticalSequence(
            int w, int h, int radius, Mat *input_mat, Mat *output_mat
    );
    void updateTargets(
            int y, int centerstart, int centerend, int leftx, int rightx
    );
    void calculateGeometry();
    void groupTargets();
    void refineTargetsVertically(Mat *input_mat, Mat *output_mat);
    Mat recogniseTarget(Mat *input_mat);
    void colourHorizTarget(int w, int h, Mat *mat);
    void colourVerticalTarget(int w, int h, Mat *mat);
    void markTargetCenters(Mat *mat);
};


class CascadeFinder : public TargetFinder {
    public:
        CascadeFinder(string cascade_xml, bool set_headless=false);
        vector<FoundTarget> doTargetRecognition(Mat *input_mat, Mat *output_mat);
    private:
        CascadeClassifier classifier;
};


#endif //QUADTARGETPLUSPLUS_TARGETFINDER_H
