//
// Created by owain on 24/06/15.
//

#include "TargetFinder.h"


using namespace cv;
using namespace std;


uchar get_pixel(Mat* mat, int y, int x, int c) {
    if(mat == NULL) {
        DEBUGPRINT("uninitialized Mat, returning 0");
        return 0;
    }
    if(y < 0 || y >= mat->rows || x < 0 || x >= mat->cols || c < 0 || c >= mat->channels()) {
        return 0;
    }
    return mat->at<uchar>(y, x);
}


void set_pixel(Mat* mat, int y, int x, int c, uchar v) {
    if(mat == NULL) {
        DEBUGPRINT("uninitialized Mat");
        return;
    }
    if(y < 0 || y >= mat->rows || x < 0 || x >= mat->cols || c < 0 || c >= mat->channels()) {
        return;
    }
    mat->at<Vec3b>(y, x)[c] = v;
}


FoundMarker::FoundMarker(int y, int center_left, int center_right, int left_x,
                         int right_x) {
    this->center_left_x = center_left;
    this->center_right_x = center_right;
    this->center_x = (center_left - center_right) / 2;
    this->left_x = left_x;
    this->right_x = right_x;
    this->first_max_y = y;
    this->last_max_y = y;
    this->first_valid_y = y;
}


int FoundMarker::get_center_x() {
    return (int) (this->center_x + 0.5);
}

int FoundMarker::get_center_y() {
    return (int) ((this->last_max_y + this->first_max_y) / 2);
}


int FoundMarker::get_center_radius() {
    return this->center_right_x - this->center_left_x;
}


int FoundMarker::get_radius() {
    return this->right_x - this->left_x;
}


int FoundMarker::get_max_y_count() {
    return this->last_max_y - this->first_max_y + 1;
}


bool FoundMarker::is_y_close(int y) {
    if(abs(y - this->last_max_y) < 10 ||
       abs(y - this->first_max_y < this->get_radius())) {
        return true;
    }
    return false;
}


bool FoundMarker::is_x_close(int x) {
    if(x > this->left_x && x < this->right_x) {
        return true;
    }
    return false;
}


void FoundMarker::expand(int y, int center_left, int center_right, int left_x,
                         int right_x) {
    int center_radius = this->get_center_radius();
    if(center_radius < (center_right - center_left)) {
        this->center_left_x = center_left;
        this->center_right_x = center_right;
        this->first_max_y = y;
        this->center_x = (int) (((double) this->center_right_x
                                        + this->center_left_x) / 2);
    }

    if(center_radius == (center_right - center_left)) {
        this->last_max_y = y;
        this->center_left_x = center_left;
        this->center_right_x = center_right;
    }

    if(this->get_radius() < (right_x - left_x)) {
        this->left_x = left_x;
        this->right_x = right_x;
    }

    this->last_valid_y = y;
}


int FoundMarker::get_valid_y_count() {
    return this->last_valid_y - this->first_valid_y;
}


void FoundMarker::mark_center(Mat *mat) {
    int y = (int) (this->get_center_y() + 0.5);
    int x = (int) (this->get_center_x() + 0.5);
    uchar* p = mat->ptr(y);
    p[x] = 255;
    p[x + 1] = 0;
    p[x + 2] = 0;
    /* set_pixel(mat, y, x, 0, 255);
    set_pixel(mat, y, x, 1, 0);
    set_pixel(mat, y, x, 2, 255); */
}


double FoundMarker::angle(FoundMarker t1, FoundMarker t2) {
    double dx = t2.get_center_x() - t1.get_center_x();
    double dy = t1.get_center_y() - t2.get_center_y();

    if(dx == 0) return 0;

    return atan2(dy, dx) * RADIANS_TO_DEGREES;
}


FoundTarget::FoundTarget(int x, int y) {
    this->markers = vector<FoundMarker>();
    this->image_size_x = x;
    this->image_size_y = y;
}


double FoundTarget::get_size() {
    return this->size;
}


double FoundTarget::get_rotation() {
    return this->rotation;
}


double FoundTarget::get_center_x() {
    return this->center_x;
}


double FoundTarget::get_center_y() {
    return this->center_y;
}


int FoundTarget::get_image_size_x() {
    return this->image_size_x;
}


int FoundTarget::get_image_size_y() {
    return this->image_size_y;
}


void FoundTarget::add_marker(FoundMarker marker) {
    this->markers.push_back(marker);
}


double FoundTarget::get_average_center_width() {
    double center_width = 0;
    int count = this->markers.size();

    for(int i = 0; i < count; i++) {
        center_width += this->markers.at(i).get_center_radius();
    }

    return center_width / count;
}


double FoundTarget::get_average_radius() {
    double total_width = 0;
    int count = this->markers.size();

    for(int i = 0; i < count; i++) {
        total_width += this->markers.at(i).get_radius();
    }

    return total_width / count;
}


void FoundTarget::calculate_geometry() {
    double a01 = FoundMarker::angle(this->markers.at(0), this->markers.at(1));
    double a02 = FoundMarker::angle(this->markers.at(0), this->markers.at(2));
    double a10 = FoundMarker::angle(this->markers.at(1), this->markers.at(0));
    double a12 = FoundMarker::angle(this->markers.at(1), this->markers.at(2));
    double a20 = FoundMarker::angle(this->markers.at(2), this->markers.at(0));
    double a21 = FoundMarker::angle(this->markers.at(2), this->markers.at(1));
    double a0 = abs(a01 - a02);
    double a1 = abs(a10 - a12);
    double a2 = abs(a20 - a21);

    if(a0 > 180) a0 = 360 - a0;
    if(a1 > 180) a1 = 360 - a1;
    if(a2 > 180) a2 = 360 - a2;

    if(a0 >= a1 && a0 >= a2) {
        this->corner = &this->markers.at(0);
        this->corner_angle = a0;
        this->center_x = (
            this->markers.at(1).get_center_x() + this->markers.at(2).get_center_x()
        ) / 2;
        this->center_y = (
            this->markers.at(1).get_center_y() + this->markers.at(2).get_center_y()
        ) / 2;
    }

    if(a1 >= a0 && a1 >= a2) {
        this->corner = &this->markers.at(1);
        this->corner_angle = a1;
        this->center_x = (
            this->markers.at(0).get_center_x() + this->markers.at(2).get_center_x()
        ) / 2;
        this->center_y = (
            this->markers.at(0).get_center_y() + this->markers.at(2).get_center_y()
        ) / 2;
    }

    if(a2 >= a0 && a2 >= a1) {
        this->corner = &this->markers.at(2);
        this->corner_angle = a2;
        this->center_x = (
            this->markers.at(0).get_center_x() + this->markers.at(1).get_center_x()
        ) / 2;
        this->center_y = (
            this->markers.at(0).get_center_y() + this->markers.at(1).get_center_y()
        ) / 2;
    }

    double dx = this->corner->get_center_x() - this->center_x;
    double dy = this->corner->get_center_y() - this->center_y;
    this->rotation = atan2(dy, dx) * RADIANS_TO_DEGREES;
    this->size = sqrt((dx * dx) + (dy * dy));
}


bool FoundTarget::has_three_markers() {
    return this->markers.size() == 3;
}


bool FoundTarget::is_close(FoundMarker test_marker) {
    for(int i = 0; i < this->markers.size(); i++) {
        FoundMarker m = this->markers[i];
        double center_dist_apart = sqrt(
            ((m.get_center_x() - test_marker.get_center_x()) ^ 2) +
            ((m.get_center_y() - test_marker.get_center_y()) ^ 2)
        );
        if(center_dist_apart > (m.get_radius() * 2) * MIN_DISTANCE_RATIO
           && center_dist_apart < (m.get_radius() * 2 * 1.4) * MAX_DISTANCE_RATIO) {
            return true;
        }
        return false;
    }
}


string FoundTarget::toString() {
    stringstream s;
    s << "x=" << this->center_x << ",y=" << this->center_y << ",size="
      << this->size << ",angle=" << this->get_rotation();
    return s.str();
}


TargetFinder::TargetFinder(bool headless) {
    this->headless = headless;
}


bool TargetFinder::check_sequence(int w, int h) {
    if(this->left_white == 0 || this->right_white == 0
       || this->right_black == 0 || this->left_black == 0) {
        return false;
    }

    if(this->center_black != 0) {
        DEBUGPRINT("check_sequence truth");
        double left_pair = this->left_black + this->left_white;
        double right_pair = this->right_black + this->right_white;
        double lr_pair_ratio = (double) left_pair / right_pair;
        this->left_erosion = (int) ((((left_pair / 2) - this->left_black) / 2));
        this->right_erosion = (int) ((((right_pair / 2) - this->right_black) / 2));
        double erosion = (this->left_erosion + this->right_erosion) / 2;

        double left_black_erosion_sq = this->left_black + (this->left_erosion * 2);
        double left_white_erosion_sq = this->left_white + (this->left_erosion * 2);
        double right_black_erosion_sq = this->right_black + (this->right_erosion * 2);
        double right_white_erosion_sq = this->right_white + (this->right_erosion * 2);
        double center_black_erosion_sq = this->center_black + this->left_erosion + this->right_erosion;
        double center_white_erosion_sq = this->center_white + this->left_erosion + this->right_erosion;

        double left_black_ratio_adjusted = (double)
            left_black_erosion_sq / right_black_erosion_sq;
        double left_white_ratio_adjusted = (double)
            left_white_erosion_sq / right_white_erosion_sq;
        double left_black_ratio = (double) (this->left_black / this->right_black);
        double left_white_ratio = (double) (this->left_white / this->right_white);

        double pair_ratio_frac = 1 / PAIR_CENTER_RATIO_THRESHOLD;
        double pair_frac = 1 / LR_RATIO_THRESHOLD;
        if(lr_pair_ratio > LR_PAIR_RATIO_THRESHOLD
            && lr_pair_ratio < pair_ratio_frac
            && left_black_ratio_adjusted > LR_PAIR_RATIO_THRESHOLD
            && left_black_ratio_adjusted < pair_ratio_frac
            && left_white_ratio_adjusted > LR_PAIR_RATIO_THRESHOLD
            && left_white_ratio_adjusted < pair_ratio_frac
            && left_black_ratio > LR_RATIO_THRESHOLD
            && left_black_ratio < pair_frac
            && left_white_ratio > LR_RATIO_THRESHOLD
            && left_white_ratio < pair_frac) {
            DEBUGPRINT("check_sequence inner truth");
            double ratio_left_black_center = (
                (double) (left_black_erosion_sq * 3) / (center_black_erosion_sq)
            );
            double ratio_right_black_center = (
                (double) (right_black_erosion_sq * 3) / (center_black_erosion_sq)
            );

            double ratio_left_pair_center = (left_pair * 1.5) / ((double) this->center_black + erosion * 2);
            double ratio_right_pair_center = (right_pair * 1.5) / ((double) this->center_black + erosion * 2);

            double lr_black_center_frac = 1 / LR_BLACK_TO_CENTER_RATIO_THRESHOLD;

            if(
                ratio_left_pair_center > PAIR_CENTER_RATIO_THRESHOLD
                && ratio_left_pair_center < pair_ratio_frac
                && ratio_right_pair_center > PAIR_CENTER_RATIO_THRESHOLD
                && ratio_right_pair_center < pair_ratio_frac
                && ratio_left_black_center > LR_BLACK_TO_CENTER_RATIO_THRESHOLD
                && ratio_left_black_center < lr_black_center_frac
                && ratio_right_black_center > LR_BLACK_TO_CENTER_RATIO_THRESHOLD
                && ratio_right_black_center < lr_black_center_frac
                && this->left_erosion < this->left_black
                && this->right_erosion < this->right_black
            ) {
                DEBUGPRINT("check_sequence inner inner truth");
                return true;
            }
        }
    }
    return false;
}


bool TargetFinder::check_vertical_sequence(
    int w, int h, int radius, Mat* input_mat, Mat* output_mat) {
    int height = input_mat->rows;
    this->left_black = 0;
    this->left_white = 0;
    this->center_black = 0;
    this->right_black = 0;
    this->right_white = 0;
    int target_center_edge = h;

    /* set_pixel(output_mat, h, w, 0, 0);
    set_pixel(output_mat, h, w, 1, 0);
    set_pixel(output_mat, h, w, 2, 255); */

    uchar* p = input_mat->ptr<uchar>(h);
    uchar pixel = p[w];
    // uchar contrast = (uchar) (255 * CONTRAST);
    // uchar black = contrast;
    // uchar white = 255 - contrast;
    uchar black = 0;
    uchar white = 255;
    if(pixel != black) {
        DEBUGPRINT("must be a black center pixel");
        return false;
    }
    int h1 = h;
    while(h1 < height && p[w] == black) {
        this->center_black++;
        target_center_edge++;
        p = input_mat->ptr<uchar>(++h1);
    }
    while(h1 < height && p[w] == white) {
        this->right_white++;
        p = input_mat->ptr<uchar>(++h1);
    }
    while(h1 < height && p[w] == black) {
        this->right_black++;
        p = input_mat->ptr<uchar>(++h1);
    }

    h1 = h;
    p = input_mat->ptr<uchar>(h1);
    this->center_black--;
    while(h1 > 0 && p[w] == black) {
        this->center_black++;
        p = input_mat->ptr<uchar>(--h1);
    }
    while(h1 > 0 && p[w] == white) {
        this->left_white++;
        p = input_mat->ptr<uchar>(--h1);
    }
    while(h1 > 0 && p[w] == black) {
        this->left_black++;
        p = input_mat->ptr<uchar>(--h1);
    }

    if(this->check_sequence(w, target_center_edge + this->right_white + this->right_white - 1)) {
        int left_x = w - (this->right_black + this->right_white
                          + this->center_black + this->left_white
                          + this->left_black + this->left_erosion) + 1;
        int right_x = w + this->right_erosion;
        double center_horiz_vert_ratio = (double) (right_x - left_x) / radius;

        if(center_horiz_vert_ratio < CENTER_ELLIPTICAL_RATIO
            || center_horiz_vert_ratio > (1 / CENTER_ELLIPTICAL_RATIO)) {
            uchar* p1 = output_mat->ptr(h);
            p1[w] = 0;
            p1[++w] = 0;
            p1[++w] = 255;

            DEBUGPRINT("failed on elliptical ratio");
            /* set_pixel(output_mat, h, w, 2, 0);
            set_pixel(output_mat, h, w, 1, 0);
            set_pixel(output_mat, h, w, 0, 255);*/

            return false;
        }

        if(!this->headless) {
            this->colour_vertical_target(
                    w, target_center_edge + this->right_white + this->right_black - 1,
                    output_mat
            );
        }
        return true;
    }

    return false;
}


void TargetFinder::update_targets(int y, int center_start, int center_end,
                                  int left_x, int right_x) {
    bool target_found = false;
    for(int i = 0; i < this->markers.size(); i++) {
        FoundMarker m = this->markers.at(i);
        double center_ratio = ((double) center_end - center_start) / m.get_radius();
        if(m.is_y_close(y) && m.is_x_close((center_start - center_end) / 2)) {
            if(center_ratio > 0.5 && center_ratio < 2) {
                m.expand(y, center_start, center_end, left_x, right_x);
                target_found = true;
            }
        }
    }

    if(!target_found) {
        this->markers.push_back(
            FoundMarker(y, center_start, center_end, left_x, right_x)
        );
    }
}


void TargetFinder::calculate_geometry() {
    for(int i = 0; i < this->final_targets.size(); i++) {
        FoundTarget t = this->final_targets.at(i);
        if(t.has_three_markers()) {
            t.calculate_geometry();
        } else {
            DEBUGPRINT("target with != 3 markers");
        }
    }
}


void TargetFinder::validate_targets() {
    for(int i = this->final_targets.size() - 1; i >= 0; i--) {
        FoundTarget t = this->final_targets.at(i);
        if(!t.has_three_markers()) {
            this->final_targets.pop_back();
        }
    }
}


void TargetFinder::group_targets() {
    this->final_targets.clear();

    int i;
    while((i = this->markers.size()) != 0) {
        FoundMarker next_marker = this->markers.at(i);
        this->markers.pop_back();
        bool found = false;

        for(int x = 0; x < this->final_targets.size(); x++) {
            FoundTarget t = this->final_targets.at(i);
            if(next_marker.get_radius() > t.get_average_radius() * MIN_TARGET_SIZE_RATIO
                && next_marker.get_radius() < t.get_average_radius() * MAX_TARGET_SIZE_RATIO) {
                if(t.is_close(next_marker)) {
                    t.add_marker(next_marker);
                    found = true;
                    break;
                }
            }
        }

        if(!found) {
            FoundTarget new_t = FoundTarget(this->input_mat->rows, this->input_mat->cols);
            new_t.add_marker(next_marker);
            this->final_targets.push_back(new_t);
        }
    }
}


void TargetFinder::refine_target_vertically(Mat *input_mat, Mat *output_mat) {
    vector<FoundMarker> final_markers;
    for(int i = 0; i < this->markers.size(); i++) {
        int valid_sequences = 0;
        FoundMarker m = this->markers.at(i);

        for(int x = m.get_center_x() - (int)((m.get_center_radius() + 1) / 2);
            x <= m.get_center_x() + (int)((m.get_center_radius() + 1) / 2); x++) {
            if((double) m.get_valid_y_count() / m.get_center_radius() > VALID_ROWS_TO_CENTER_RATIO) {
                if(this->check_vertical_sequence(x, m.get_center_y(), m.get_radius(),
                                                 input_mat, output_mat)) {
                    valid_sequences++;
                }
            }
        }

        if(valid_sequences >= 1) {
            final_markers.push_back(m);
        }
    }

    this->markers = final_markers;
}


Mat TargetFinder::recognise_target(Mat *input_mat) {
    this->markers.clear();
    int width = input_mat->cols;
    int height = input_mat->rows;

    Mat output_mat = Mat(height, width, CV_8UC3);

    for(int h = 0; h < height; h++) {
        this->left_black = 0;
        this->left_white = 0;
        this->center_black = 0;
        this->right_white = 0;
        this->right_black = 0;

        int w = 0;
        uchar* p = input_mat->ptr(h);
        uchar* o = output_mat.ptr(h);
        uchar current_colour = p[w];
        int current_count = 1;

        while(!this->headless && w < width) {
            uchar pixel = p[w];
            // uchar pixel = get_pixel(input_mat, h, w, 0);
            if (pixel == 255) {
                o[3 * w] = 255;
                o[3 * w + 1] = 255;
                o[3 * w + 2] = 255;
                /* set_pixel(&output_mat, h, w, 0, 255);
                set_pixel(&output_mat, h, w, 1, 255);
                set_pixel(&output_mat, h, w, 2, 255); */
            }
            else if (pixel == 0) {
                o[3 * w] = 128;
                o[3 * w + 1] = 128;
                o[3 * w + 2] = 128;
                /* set_pixel(&output_mat, h, w, 0, 0);
                set_pixel(&output_mat, h, w, 1, 0);
                set_pixel(&output_mat, h, w, 2, 0); */
            } else {
                o[3 * w] = 0;
                o[3 * w + 1] = 0;
                o[3 * w + 2] = 0;
                /* set_pixel(&output_mat, h, w, 0, 128);
                set_pixel(&output_mat, h, w, 1, 128);
                set_pixel(&output_mat, h, w, 2, 128); */
            }
            w++;
        }

        w = 0;
        while(w < width) {
            uchar pixel = p[w];
            // uchar pixel = get_pixel(input_mat, h, w, 0);
            if(pixel != current_colour) {
                if(current_colour == 0) {
                    this->left_black = this->left_white;
                    this->left_white = this->center_black;
                    this->center_black = this->right_white;
                    this->right_black = current_count;

                    if(this->check_sequence(w - 1, h)) {
                        int center_start = w - (this->right_black + this->right_white
                                                + this->center_black + this->left_erosion) + 1;
                        int center_end = w - (this->right_black + this->right_white)
                                         + this->right_erosion;
                        int left_x = w - (this->right_black + this->right_white
                                          + this->center_black + this->left_white
                                          + this->left_black + this->left_erosion) + 1;
                        int right_x = w + this->right_erosion;
                        this->update_targets(h, center_start, center_end, left_x, right_x);
                        if(!this->headless) {
                            this->colour_horizontal_target(w - 1, h, &output_mat);
                        }
                    }
                } else {
                    this->left_black = this->left_white;
                    this->left_white = this->center_black;
                    this->center_black = this->right_white;
                    this->right_white = this->right_black;
                    this->right_black = current_count;
                }

                current_colour = pixel;
                current_count = 0;
            }

            current_count++;
            w++;
        }
    }

    output_mat.ptr(10)[32] = 255;
    // set_pixel(&output_mat, 10, 10, 2, 255);

    return output_mat;
}


void TargetFinder::colour_horizontal_target(int w, int h, Mat *mat) {
    int left_black_start = w - (this->right_black + this->right_white
                                + this->center_black + this->left_white
                                + this->left_black) + 1;
    int left_black_end = w - (this->right_black + this->right_white
                              + this->center_black + this->left_white);
    int center_start = w - (this->right_black + this->right_white
                            + this->center_black) + 1;
    int center_end = w - (this->right_black + this->right_white);
    int right_black_start = w - (this->right_black) + 1;
    int right_black_end = w;

    // uchar* p = mat->ptr(h);
    for(int i = left_black_start; i <= left_black_end; i++) {
        set_pixel(mat, h, i, 2, 192);
        // p[i + 2] = 192;
    }
    for(int i = center_start; i <= center_end; i++) {
        set_pixel(mat, h, i, 2, 255);
        // p[i + 2] = 255;
    }
    for(int i = right_black_start; i <= right_black_end; i++) {
        set_pixel(mat, h, i, 2, 64);
        // p[i + 2] = 64;
    }

    if(this->left_erosion >= 0) {
        for(int i = 1; i <= this->left_erosion; i++) {
            set_pixel(mat, h, left_black_start - i, 2, 128);
            set_pixel(mat, h, left_black_start - i, 1, 0);
            set_pixel(mat, h, center_start - i, 2, 128);
            set_pixel(mat, h, center_start - i, 1, 0);
            set_pixel(mat, h, left_black_end + i, 2, 128);
            set_pixel(mat, h, left_black_end + i, 1, 0);
            /* p[left_black_start - i + 2] = 128;
            p[left_black_start - i + 1] = 0;
            p[center_start - i + 2] = 128;
            p[center_start - i + 1] = 0;
            p[left_black_end - i + 2] = 128;
            p[left_black_end - i + 1] = 0; */
        }
    } else {
        for(int i = this->left_erosion; i <= -1; i++) {
            set_pixel(mat, h, left_black_start + i, 2, 128);
            set_pixel(mat, h, left_black_start + i, 1, 0);
            set_pixel(mat, h, center_start + i, 2, 128);
            set_pixel(mat, h, center_start + i, 1, 0);
            set_pixel(mat, h, left_black_end - i, 2, 128);
            set_pixel(mat, h, left_black_end - i, 1, 0);
            /* p[left_black_start + i + 2] = 128;
            p[left_black_start + i + 1] = 0;
            p[center_start + i + 2] = 128;
            p[center_start + i + 1] = 0;
            p[left_black_end - i + 2] = 128;
            p[left_black_end - i + 1] = 0; */
        }
    }

    if(this->right_erosion >= 0) {
        for(int i = 1; i <= this->right_erosion; i++) {
            set_pixel(mat, h, right_black_end - i, 2, 128);
            set_pixel(mat, h, right_black_end - i, 1, 0);
            set_pixel(mat, h, center_end + i, 2, 128);
            set_pixel(mat, h, center_end + i, 1, 0);
            set_pixel(mat, h, right_black_start + i, 2, 128);
            set_pixel(mat, h, right_black_start + i, 1, 0);
            /* p[right_black_end - i + 2] = 128;
            p[right_black_end - i + 1] = 0;
            p[center_end + i + 2] = 128;
            p[center_end + i + 1] = 0;
            p[right_black_start + i + 2] = 128;
            p[right_black_start + i + 1] = 0; */
        }
    }
}


void TargetFinder::colour_vertical_target(int w, int h, Mat *mat) {
    int left_black_start = h - (this->right_black + this->right_white
                                + this->center_black + this->left_white
                                + this->left_black) + 1;
    int left_black_end = h - (this->right_black + this->right_white
                              + this->left_white);
    int center_start = h - (this->right_black + this->right_white
                            + this->center_black) + 1;
    int center_end = h - (this->right_black + this->right_white);
    int right_black_start = h - (this->right_black) + 1;
    int right_black_end = h;

    for(int i = right_black_start; i <= right_black_end; i++) {
        // mat->ptr(i)[w + 1] = 64;
        set_pixel(mat, i, w, 1, 64);
    }
    for(int i = center_start; i <= center_end; i++) {
        set_pixel(mat, i, w, 1, 255);
        // mat->ptr(i)[w + 1] = 255;
    }
    for(int i = left_black_start; i <= left_black_end; i++) {
        set_pixel(mat, i, w, 1, 192);
        // mat->ptr(i)[w + 1] = 192;
    }

    if(this->left_erosion >= 0) {
        for(int i = 1; i <= this->left_erosion; i++) {
            set_pixel(mat, left_black_start - i, w, 1, 0);
            set_pixel(mat, left_black_start - i, w, 0, 128);
            set_pixel(mat, center_start - i, w, 1, 0);
            set_pixel(mat, center_start - i, w, 0, 128);
            set_pixel(mat, left_black_end + i, w, 1, 0);
            set_pixel(mat, left_black_end + i, w, 0, 128);
            /* mat->ptr(left_black_start - i)[w + 1] = 0;
            mat->ptr(left_black_start - i)[w] = 128;
            mat->ptr(center_start - i)[w + 1] = 0;
            mat->ptr(center_start - i)[w] = 128;
            mat->ptr(left_black_end + i)[w + 1] = 128;
            mat->ptr(left_black_end + i)[w] = 0; */
        }
    } else {
        for(int i = this->left_erosion; i <= -1; i++) {
            set_pixel(mat, left_black_start + i, w, 1, 0);
            set_pixel(mat, left_black_start + i, w, 0, 128);
            set_pixel(mat, center_start + i, w, 1, 0);
            set_pixel(mat, center_start + i, w, 0, 128);
            set_pixel(mat, left_black_end - i, w, 1, 0);
            set_pixel(mat, left_black_end - i, w, 0, 128);
            /* mat->ptr(left_black_start + i)[w + 1] = 0;
            mat->ptr(left_black_start + i)[w] = 128;
            mat->ptr(center_start + i)[w + 1] = 0;
            mat->ptr(center_start + i)[w + 1] = 128;
            mat->ptr(left_black_end - i)[w + 1] = 128;
            mat->ptr(left_black_end - i)[w] = 0; */
        }
    }

    if(this->right_erosion >= 0) {
        for(int i = 1; i <= this->right_erosion; i++) {
            set_pixel(mat, right_black_end + i, w, 1, 0);
            set_pixel(mat, right_black_end + i, w, 0, 128);
            set_pixel(mat, center_end + i, w, 1, 0);
            set_pixel(mat, center_end + i, w, 0, 128);
            set_pixel(mat, right_black_start - i, w, 1, 0);
            set_pixel(mat, right_black_start - i, w, 0, 128);
            /* mat->ptr(right_black_end + i)[w + 1] = 0;
            mat->ptr(right_black_end + i)[w] = 128;
            mat->ptr(center_end + i)[w + 1] = 0;
            mat->ptr(center_end + i)[w] = 128;
            mat->ptr(right_black_start - i)[w + 1] = 0;
            mat->ptr(right_black_start - i)[w] = 128; */
        }
    } else {
        for(int i = this->right_erosion; i <= -1; i++) {
            set_pixel(mat, right_black_end - i, w, 1, 0);
            set_pixel(mat, right_black_end - i, w, 0, 128);
            set_pixel(mat, center_end - i, w, 1, 0);
            set_pixel(mat, center_end - i, w, 0, 128);
            set_pixel(mat, right_black_start + i, w, 1, 0);
            set_pixel(mat, right_black_start + i, w, 0, 128);
            /* mat->ptr(right_black_end - i)[w + 1] = 0;
            mat->ptr(right_black_end - i)[w] = 128;
            mat->ptr(center_end - i)[w + 1] = 0;
            mat->ptr(center_end - i)[w] = 128;
            mat->ptr(right_black_start + i)[w + 1] = 0;
            mat->ptr(right_black_start + i)[w] = 128; */
        }
    }
}


void TargetFinder::mark_target_centers(Mat *mat) {
    for(int i = 0; i < this->markers.size(); i++) {
        FoundMarker m = this->markers.at(i);
        m.mark_center(mat);
    }
}


vector<FoundTarget> TargetFinder::do_target_recognition(Mat *input_mat, Mat *output_mat) {
    this->final_targets.clear();
    IMDEBUG("stage0", *output_mat);

    *output_mat = this->recognise_target(input_mat);
    IMDEBUG("stage1", *output_mat);

    this->refine_target_vertically(input_mat, output_mat);
    this->mark_target_centers(output_mat);
    this->group_targets();
    this->validate_targets();
    this->calculate_geometry();
    IMDEBUG("stage2", *output_mat);

    #ifdef DEBUG_EXTREME
    // Debug output, look for circles
    vector<Vec3f> circles;
    HoughCircles(*input_mat, circles, CV_HOUGH_GRADIENT, 1, input_mat->rows / 16, 255, 50, 0, 0);
    for(int i = 0; i < circles.size(); i++) {
        Point center(cvRound(circles.at(i)[0]), cvRound(circles.at(i)[1]));
        int radius = cvRound(circles.at(i)[2]);
        circle(*output_mat, center, radius, Scalar(255, 0, 0), 3, 8, 0);
    }
    #endif
    IMDEBUG("stage3", *output_mat);

    return this->final_targets;
}
