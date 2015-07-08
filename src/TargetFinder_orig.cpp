/*
 * TargetFinder_orig: Attempt to get as close to Neal Snooke's original Java
 * implementation of QuadTarget as possible. Didn't work very well, had bugs I
 * couldn't figure out -- probably due to how OpenCV differs in its pixel access
 * etc.
 */

#include "TargetFinder_orig.h"


using namespace cv;
using namespace std;


uchar get_pixel(Mat* mat, int y, int x, int c) {
    return mat->ptr(y)[(mat->channels() * x) + c];
}


void set_pixel(Mat* mat, int y, int x, int c, uchar v) {
    mat->ptr(y)[(mat->channels() * x) + c] = v;
}


#define set_pixel_safe(mat, y, x, c, v) if(in_bounds(mat, y, x, c)) { \
    set_pixel(mat, y, x, c, v); \
}


#define set_pixel3(mat, y, x, b, g, r) if(in_bounds(mat, y, x, 2)) { \
   uchar* o = mat->ptr(y); \
   o[3 * x] = b; \
   o[(3 * x) + 1] = g; \
   o[(3 * x) + 2] = r; \
}


bool in_bounds(Mat* mat, int y, int x, int c) {
    if(mat == NULL) {
        DEBUGPRINT("uninitialized Mat");
        return false;
    }
    if(y < 0 || y >= mat->rows || x < 0 || x >= mat->cols || c < 0 || c >= mat->channels()) {
        return false;
    }
    return true;
}


FoundMarker::FoundMarker(int y, int centerleft, int centerright, int left_x,
                         int right_x) {
    centerleftx = centerleft;
    centerrightx = centerright;
    centerx = (centerleft+centerright)/2;
    leftx = left_x;
    rightx = right_x;
    firstmaxy = y;
    lastmaxy = y;
    firstValidy = y;
    DEBUGPRINT("new FoundMarker, lastValidy: " << getLastValidy());
}


double FoundMarker::getCenterX() {
    return (int)(centerx + 0.5);
}

double FoundMarker::getCenterY() {
    return ((double) (lastmaxy + firstmaxy) / 2);
}


double FoundMarker::centerRadius() {
    return centerrightx - centerleftx;
}


double FoundMarker::radius() {
    return rightx - leftx;
}


bool FoundMarker::yClose(int y) {
    if(abs(y - lastmaxy) < 10 ||
       abs(y - firstmaxy < radius())) {
        return true;
    }
    return false;
}


bool FoundMarker::xClose(int x) {
    if(x > leftx && x < rightx) {
        return true;
    }
    return false;
}


void FoundMarker::expand(const int y, const int centerleft, const int centerright,
                         const int left_x, const int right_x) {
    if (centerRadius() < (centerright-centerleft)){
        // this row is closer to the center
        centerleftx = centerleft;
        centerrightx = centerright;
        firstmaxy = y;
        centerx = (int)(((double)centerrightx+centerleftx)/2);
    }

    if (centerRadius() == (centerright-centerleft)){
        lastmaxy = y;
        centerleftx = centerleft;
        centerrightx = centerright;
        // all centers are used to calculate the final answer to average any anomalies
        // this isn't quite right because the latest ones have more influence
        //centerx = (centerx+(centerrightx+centerleftx)/2)/2;
    }

    if (radius() < (right_x-left_x)){
        // this row is closer to the center
        leftx = left_x;
        rightx = right_x;
        firstmaxy = y;
    }

    lastValidy = y;
    DEBUGPRINT("expand called, lastValidy: " << getLastValidy() << ", firstValidy: " << getFirstValidy());
}


int FoundMarker::getValidyCount() {
    return lastValidy - firstValidy;
}


void FoundMarker::markCenter(Mat *mat) {
    int y = (int) getCenterY();
    int x = (int) getCenterX();
    set_pixel3(mat, y, x, 255, 0, 0);
}


double FoundMarker::angle(FoundMarker t1, FoundMarker t2) {
    double dx = t2.getCenterX() - t1.getCenterX();
    double dy = t1.getCenterY() - t2.getCenterY();

    if(dx == 0) return 0;

    return atan2(dy, dx) * RADIANS_TO_DEGREES;
}


FoundTarget::FoundTarget(int x, int y) {
    markers = vector<FoundMarker>();
    imagesizex = x;
    imagesizey = y;
    DEBUGPRINT(toString());
}


void FoundTarget::addMarker(FoundMarker marker) {
    DEBUGPRINT("Added marker");
    markers.push_back(marker);
}


double FoundTarget::getAveCenterWidth() {
    double center_width = 0;
    vector<FoundMarker>::size_type count = markers.size();

    for(vector<FoundMarker>::size_type i = 0; i < count; i++) {
        center_width += markers[i].centerRadius();
    }

    return center_width / (double) count;
}


double FoundTarget::getAveRadius() {
    double total_width = 0;
    vector<FoundMarker>::size_type count = markers.size();

    for(vector<FoundMarker>::size_type i = 0; i < count; i++) {
        total_width += markers[i].radius();
    }

    return total_width / (double) count;
}


void FoundTarget::calculateGeometry() {
    double a01 = FoundMarker::angle(markers[0], markers[1]);
    double a02 = FoundMarker::angle(markers[0], markers[2]);
    double a10 = FoundMarker::angle(markers[1], markers[0]);
    double a12 = FoundMarker::angle(markers[1], markers[2]);
    double a20 = FoundMarker::angle(markers[2], markers[0]);
    double a21 = FoundMarker::angle(markers[2], markers[1]);
    double a0 = abs(a01 - a02);
    double a1 = abs(a10 - a12);
    double a2 = abs(a20 - a21);

    if(a0 > 180) a0 = 360 - a0;
    if(a1 > 180) a1 = 360 - a1;
    if(a2 > 180) a2 = 360 - a2;

    if(a0 >= a1 && a0 >= a2) {
        corner = &markers[0];
        cornerAngle = a0;
        centerx = (
                          markers[1].getCenterX() + markers[2].getCenterX()
        ) / 2;
        centery = (
                          markers[1].getCenterY() + markers[2].getCenterY()
        ) / 2;
    }

    if(a1 >= a0 && a1 >= a2) {
        corner = &markers[1];
        cornerAngle = a1;
        centerx = (
                          markers[0].getCenterX() + markers[2].getCenterX()
        ) / 2;
        centery = (
                          markers[0].getCenterY() + markers[2].getCenterY()
        ) / 2;
    }

    if(a2 >= a0 && a2 >= a1) {
        corner = &markers[2];
        cornerAngle = a2;
        centerx = (
                          markers[0].getCenterX() + markers[1].getCenterX()
        ) / 2;
        centery = (
                          markers[0].getCenterY() + markers[1].getCenterY()
        ) / 2;
    }

    double dx = corner->getCenterX() - centerx;
    double dy = corner->getCenterY() - centery;
    rotation = atan2(dy, dx) * RADIANS_TO_DEGREES;
    size = sqrt((dx * dx) + (dy * dy));
}


bool FoundTarget::hasThreeMarkers() {
    return markers.size() == 3;
}


bool FoundTarget::isClose(FoundMarker testMarker) {
    DEBUGPRINT("isClose, markers: " << markers.size());
    for(vector<FoundMarker>::size_type i = 0; i < markers.size(); i++) {
    // for(vector<FoundMarker>::iterator i = markers.begin(); i < markers.end(); i++) {
        FoundMarker m = markers[i];
        double center_dist_apart = sqrt(
            pow((m.getCenterX() - testMarker.getCenterX()), 2) +
            pow((m.getCenterY() - testMarker.getCenterY()), 2)
        );
        if(center_dist_apart > (m.radius() * 2) * MINDISTANCERATIO
           && center_dist_apart < (m.radius() * 2 * 1.4) * MAXDISTANCERATIO) {
            return true;
        }
        return false;
    }
}


string FoundTarget::toString() {
    stringstream s;
    s << "x=" << centerx << ",y=" << centery << ",size="
      << size << ",angle=" << rotation;
    return s.str();
}


TargetFinder::TargetFinder(bool set_headless) {
    headless = set_headless;
}


bool TargetFinder::checkSequence(int w, int h) {
    if (leftWhite == 0 ||
        rightWhite == 0 ||
        rightBlack == 0 ||
        leftBlack == 0) {
        return false;
    }

    if (centerBlack != 0){
        double leftPair = leftBlack+leftWhite;
        double rightPair = rightBlack+rightWhite;

        double LRPairRatio = (double)leftPair/rightPair;
        double LBlackRatioAdjusted = (double)(leftBlack)/(rightBlack);
        double LWhiteRatioAdjusted = (double)(leftWhite)/(rightWhite);
        double LBlackRatio = (double)(leftBlack)/(rightBlack);
        double LWhiteRatio = (double)(leftWhite)/(rightWhite);

        // are the left and right pairs similar dimensions?
        if
                ( LRPairRatio > LRPairRatioThreshold && LRPairRatio < (1/LRPairRatioThreshold)
                  && LBlackRatioAdjusted > LRPairRatioThreshold && LBlackRatioAdjusted < (1/LRPairRatioThreshold)
                  && LWhiteRatioAdjusted > LRPairRatioThreshold && LWhiteRatioAdjusted < (1/LRPairRatioThreshold)
                  && LBlackRatio > LRRatioThreshold && LBlackRatio < (1/LRRatioThreshold)
                  && LWhiteRatio > LRRatioThreshold && LWhiteRatio < (1/LRRatioThreshold)
                ){
            // is the center the correct ratio 3:1 center to black ring
            double ratioLeftBlackToCenter = ((double)(leftBlack)*3)/(centerBlack);
            double ratioRightBlackToCenter = ((double)(rightBlack))*3/(centerBlack);

            // pair is 2 bands wide and center is 3 so expected size of center is pair*1.5
            double ratioLeftPairToCenter = ((double)leftPair*1.5)/((double)centerBlack*2);
            double ratioRightPairToCenter = ((double)rightPair*1.5)/((double)centerBlack*2);

            if (       ratioLeftPairToCenter > pairCenterRatioThreshold
                       && ratioLeftPairToCenter < (1/pairCenterRatioThreshold)
                       && ratioRightPairToCenter > pairCenterRatioThreshold
                       && ratioRightPairToCenter < (1/pairCenterRatioThreshold)

                       && ratioLeftBlackToCenter > LRBlacktoCenterRatioThreshold
                       && ratioLeftBlackToCenter < (1/LRBlacktoCenterRatioThreshold)
                       && ratioRightBlackToCenter > LRBlacktoCenterRatioThreshold
                       && ratioRightBlackToCenter < (1/LRBlacktoCenterRatioThreshold)
                    ){
                //System.out.println("Found pattern at "+w+" "+h);
                //System.out.println("Left E:"+leftErosion+"   Right E:"+rightErosion);

                return true;
            }
        }
    }

    return false;
}


bool TargetFinder::checkVerticalSequence(
        int w, int h, int radius, Mat *input_mat, Mat *output_mat) {
    int height = input_mat->rows;

    leftBlack = 0; // below
    leftWhite = 0; // below
    centerBlack = 0;
    rightWhite = 0; // above
    rightBlack = 0; // above

    int targetCenterEdge = h;

    uchar black = 0;
    uchar white = 255;

    // colour test column start point RED
    set_pixel3(output_mat, h, w, 0, 0, 255);

    uchar* p = input_mat->ptr<uchar>(h);
    uchar pixel = p[w];
    if (pixel != black){
        DEBUGPRINT("checkVerticalSequence: must be a black center pixel!");
        return false;
    }

    int h1 = h;
    p = input_mat->ptr<uchar>(h1);
    while (h1 < height && p[w] == black){
        centerBlack++;
        targetCenterEdge++;
        h1++;
    }

    p = input_mat->ptr<uchar>(h1);
    while (h1 < height && p[w] == white){
        rightWhite++;
        h1++;
    }

    p = input_mat->ptr<uchar>(h1);
    while (h1 < height && p[w] == black){
        rightBlack++;
        h1++;
    }

    h1 = h;
    centerBlack--;
    p = input_mat->ptr<uchar>(h1);
    while (h1 > 0 && p[w] == black){
        centerBlack++;
        h1--;
    }

    p = input_mat->ptr<uchar>(h1);
    while (h1 > 0 && p[w] == white){
        leftWhite++;
        h1--;
    }

    p = input_mat->ptr<uchar>(h1);
    while (h1 > 0 && p[w] == black){
        leftBlack++;
        h1--;
    }

    if (checkSequence(w, targetCenterEdge + rightWhite + rightBlack - 1)){ //coordinate, not used

        int leftx = w - (rightBlack + rightWhite + centerBlack + leftWhite + leftBlack) + 1;
        int rightx = w;
        double centerHorizVertRatio = (double)(rightx-leftx)/radius;

        if (centerHorizVertRatio < CenterElipticalRatio
            || centerHorizVertRatio > (1 / CenterElipticalRatio)){

            DEBUGPRINT("checkVerticalSequence - Failed on elliptical RATIO: " << centerHorizVertRatio);

            // colour cyan if failed on elliptical ratio
            set_pixel3(output_mat, h, w, 255, 255, 0);

            return false;
        }

        colourVerticalTarget(w, targetCenterEdge + rightWhite + rightBlack - 1,
                             output_mat);
        return true;
    }

    return false;
}


void TargetFinder::updateTargets(int y, int centerstart, int centerend,
                                 int leftx, int rightx) {
    bool targetFound = false;
    DEBUGPRINT("updateTargets Markers: " << markers.size());
    // for(vector<FoundMarker>::iterator i = markers.begin(); i < markers.end(); i++) {
    for(vector<FoundMarker>::size_type i = 0; i < markers.size(); i++) {
        FoundMarker m = markers[i];
        double centerRatio = ((double)centerend-centerstart)/m.centerRadius();
        DEBUGPRINT("updateTargets marker " << i << ": lastValidy: " << m.getLastValidy());
        //if (target.yClose(y) && target.xClose(centerstart+(centerend-centerstart)/2)){
        if (m.yClose(y) && m.xClose((centerstart+centerend)/2)){
            //only expand if the center ratio with existing target is reasonable
            if (centerRatio > 0.5 && centerRatio < 2){
                m.expand(y, centerstart, centerend, leftx, rightx);
                targetFound = true;
            }
        }
    }

    if(!targetFound) {
        markers.push_back(
            FoundMarker(y, centerstart, centerend, leftx, rightx)
        );
    }
}


void TargetFinder::calculateGeometry() {
    DEBUGPRINT("calculateGeometry, final_targets: " << final_targets.size());
    for(vector<FoundTarget>::size_type i = 0; i < final_targets.size(); i++) {
    // for(vector<FoundTarget>::iterator i = final_targets.begin(); i < final_targets.end(); i++) {
        FoundTarget t = final_targets[i];
        if(t.hasThreeMarkers()) {
            t.calculateGeometry();
        } else {
            DEBUGPRINT("target with != 3 markers");
        }
    }
}


void TargetFinder::groupTargets() {
    final_targets.clear();
    DEBUGPRINT("groupTargets, markers: " << markers.size());

    vector<FoundMarker>::size_type i;
    while((i = markers.size()) > 0) {
        FoundMarker next_marker = markers[i];
        markers.pop_back();
        bool found = false;

        for(vector<FoundTarget>::size_type x = 0; x < final_targets.size(); x++) {
        // for(vector<FoundTarget>::iterator x = final_targets.begin(); x < final_targets.end(); x++) {
            FoundTarget t = final_targets[x];
            if(next_marker.radius() > t.getAveRadius() * MINTARGETSIZERATIO
                && next_marker.radius() < t.getAveRadius() * MAXTARGETSIZERATIO) {
                if(t.isClose(next_marker)) {
                    t.addMarker(next_marker);
                    found = true;
                    break;
                }
            }
        }

        if(!found) {
            FoundTarget new_t = FoundTarget(input_mat->rows, input_mat->cols);
            new_t.addMarker(next_marker);
            final_targets.push_back(new_t);
        }
    }
}


void TargetFinder::refineTargetsVertically(Mat *input_mat, Mat *output_mat) {
    DEBUGPRINT("refineTargetsVertically, markers: " << markers.size());
    vector<FoundMarker> final_markers;
    for (vector<FoundMarker>::size_type i = 0; i < markers.size(); i++){
    // for(vector<FoundMarker>::iterator i = markers.begin(); i < markers.end(); i++) {
        FoundMarker m = markers[i];
        int validSequences = 0;
        for (int x = m.getCenterX()-(int)((m.centerRadius()+1)/2);
             x <= m.getCenterX()+(int)((m.centerRadius()+1)/2); x++){
        //for(int x = m.leftx; x < m.rightx; x++) {
            // if there are not at least 1/10 maximum valid center rows compared to the size of the center then not a target
            if (checkVerticalSequence(x, (int) m.getCenterY(),
                                      (int) m.radius(), input_mat,
                                      output_mat)) {
                if (((double) m.getValidyCount())/ m.centerRadius() > VALID_ROWS_TO_CENTER_RATIO){
                    DEBUGPRINT("refineTargetsVertically, valid center rows, i: " << i);
                    validSequences++;
                    }
            }
            DEBUGPRINT("refineTargetsVertically, i: " << i << ", x: " << x << ", centerx: " <<
                       m.getCenterX() << ", center_radius: " <<
                       m.centerRadius() << ", valid_y_count: " <<
                       m.getValidyCount() << ", lastValidy: " << m.getLastValidy());
        }

        if (validSequences >= 1){
            DEBUGPRINT("refineTargetsVertically, i: " << i << ", validSequences: " << validSequences);
            final_markers.push_back(m);
        }
    }

    // markers = final_markers;
    DEBUGPRINT("end of refineTargetsVertically, final_markers: " << final_markers.size());
    markers.clear();
    for(vector<FoundMarker>::size_type i = 0; i < final_markers.size(); i++) {
    // for(vector<FoundMarker>::iterator i = final_markers.begin(); i < final_markers.end(); i++) {
        markers.push_back(final_markers[i]);
    }
    DEBUGPRINT("end of refineTargetsVertically, markers: " << markers.size());
}


Mat TargetFinder::recogniseTarget(Mat *input_mat) {
    markers.clear();
    int width = input_mat->cols;
    int height = input_mat->rows;

    Mat output_mat = Mat(height, width, CV_8UC3);
    //cvtColor(*input_mat, output_mat, COLOR_GRAY2BGR);

    for(int h=0; h<height; h++) {

        leftBlack = 0;
        leftWhite = 0;
        centerBlack = 0;
        rightWhite = 0;
        rightBlack = 0;
        uchar* p = input_mat->ptr(h);
        uchar* o = output_mat.ptr(h);

        // process a row - copy to the output.  Need to be done before the
        // target algorithm because target size may extend beyond the current pixel (edge of target)
        int w=0;
        while(!headless && w < width) {
            uchar pixel = p[w];
            if (pixel == 255) {
                o[3 * w] = 255;
                o[(3 * w) + 1] = 255;
                o[(3 * w) + 2] = 255;
            }
            else if (pixel == 0) {
                o[3 * w] = 128;
                o[(3 * w) + 1] = 128;
                o[(3 * w) + 2] = 128;
            } else {
                o[3 * w] = 0;
                o[(3 * w) + 1] = 0;
                o[(3 * w) + 2] = 0;
            }
            w++;
        }

        // process a row
        w=0;
        uchar currentColor = p[w];
        int currentCount = 1;

        while (w < width){
            uchar pixel = p[w];

            if (pixel != currentColor){

                if (currentColor == 0){ // white pixel found after black sequence

                    leftBlack = leftWhite;
                    leftWhite = centerBlack;
                    centerBlack = rightWhite;
                    rightWhite = rightBlack;
                    rightBlack = currentCount; //actually is a black sequence

                    if (checkSequence(w - 1, h)){
                        int centerStart = w-(rightBlack +rightWhite +centerBlack)+1;
                        int centerEnd = w-(rightBlack +rightWhite);
                        int leftx = w-(rightBlack +rightWhite +centerBlack +leftWhite +leftBlack) + 1;
                        int rightx = w;
                        updateTargets(h, centerStart, centerEnd, leftx, rightx); // row, center-left and center-right
                        colourHorizTarget(w - 1, h, &output_mat);
                    }
                } else { // black pixel found after white sequence 
                    leftBlack = leftWhite;
                    leftWhite = centerBlack;
                    centerBlack = rightWhite;
                    rightWhite = rightBlack;
                    rightBlack = currentCount; //actually a white sequence
                }

                currentColor = pixel;
                currentCount = 0;
            }

            currentCount++;
            w++; //next pixel
        } // w
    } // h	

    return output_mat;
}


void TargetFinder::colourHorizTarget(int w, int h, Mat *mat) {
    int left_black_start = w - (rightBlack + rightWhite
                                + centerBlack + leftWhite
                                + leftBlack) + 1;
    int left_black_end = w - (rightBlack + rightWhite
                              + centerBlack + leftWhite);
    int center_start = w - (rightBlack + rightWhite
                            + centerBlack) + 1;
    int center_end = w - (rightBlack + rightWhite);
    int right_black_start = w - (rightBlack) + 1;
    int right_black_end = w;

    for(int i = left_black_start; i <= left_black_end; i++) {
        set_pixel_safe(mat, h, i, 2, 192);
    }
    for(int i = center_start; i <= center_end; i++) {
        set_pixel_safe(mat, h, i, 2, 255);
    }
    for(int i = right_black_start; i <= right_black_end; i++) {
        set_pixel_safe(mat, h, i, 2, 64);
    }
}


void TargetFinder::colourVerticalTarget(int w, int h, Mat *mat) {
    int leftBlackStart = h-(rightBlack +rightWhite +centerBlack +leftWhite +
                                                                                   leftBlack)+1;
    int leftBlackEnd = h-(rightBlack +rightWhite +centerBlack +leftWhite);
    int centerStart = h-(rightBlack +rightWhite +centerBlack)+1;
    int centerEnd = h-(rightBlack +rightWhite);
    int rightBlackStart = h-(rightBlack)+1;
    int rightBlackEnd = h;

    for (int i = rightBlackStart;  i<=rightBlackEnd; i++){
        set_pixel3(mat, i, w, 255, 64, 128);
    }

    for (int i = centerStart;  i<=centerEnd; i++){
        set_pixel3(mat, i, w, 128, 255, 0);
    }

    for (int i = leftBlackStart;  i<=leftBlackEnd; i++){
        set_pixel3(mat, i, w, 0, 0, 192);
    }
}


void TargetFinder::markTargetCenters(Mat *mat) {
    DEBUGPRINT("markTargetCenters, markers: " << markers.size());
    for(vector<FoundMarker>::size_type i = 0; i < markers.size(); i++) {
    // for(vector<FoundMarker>::iterator i = markers.begin(); i < markers.end(); i++) {
        FoundMarker m = markers[i];
        m.markCenter(mat);
    }
}


vector<FoundTarget> TargetFinder::doTargetRecognition(Mat *input_mat,
                                                      Mat *output_mat) {
    final_targets.clear();
    IMDEBUG("stage0", *output_mat);

    *output_mat = recogniseTarget(input_mat);
    IMDEBUG("stage1", *output_mat);

    refineTargetsVertically(input_mat, output_mat);
    markTargetCenters(output_mat);
    groupTargets();
    calculateGeometry();
    IMDEBUG("stage2", *output_mat);

    #ifdef DEBUG_EXTREME
    // Debug output, look for circles
    vector<Vec3f> circles;
    HoughCircles(*input_mat, circles, CV_HOUGH_GRADIENT, 1, input_mat->rows / 16, 255, 50, 0, 0);
    for(int i = 0; i < circles.size(); i++) {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        circle(*output_mat, center, radius, Scalar(255, 0, 0), 3, 8, 0);
    }
    #endif
    IMDEBUG("stage3", *output_mat);

    return final_targets;
}
