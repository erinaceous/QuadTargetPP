/*
 * Target tracking algorithm.
 *
 * Author: Owain Jones <odj@aber.ac.uk> <github.com/erinaceous>
 */

#include "TargetFinder.h"

using namespace qtpp;

#define setPixel3(mat, ptr_y, x, b, g, r) \
    if(mat != NULL && y > 0 && y < mat->rows && x > 0 && x < mat->cols) { \
        uchar* o = mat->ptr(y); \
        ptr_y[3 * x] = b; \
        ptr_y[3 * x + 1] = g; \
        ptr_y[3 * x + 2] = r; \
    }

void TargetFinder::setParam(std::string param, void* value) {
    if(param.compare("elliptical_ratio") == 0) {
        this->elliptical_ratio = *static_cast<double*>(value);
    } else if(param.compare("inner_radius") == 0) {
        this->inner_radius = *static_cast<double*>(value);
    } else if(param.compare("central_radius") == 0) {
        this->central_radius = *static_cast<double*>(value);
    } else if(param.compare("outer_radius") == 0) {
        this->outer_radius = *static_cast<double*>(value);
    }
}

std::string TargetFinder::toString() {
    std::stringstream ss;
    ss << "TargetFinder[elliptical_ratio=" << this->elliptical_ratio <<
          ", inner_radius=" << this->inner_radius <<
          ", central_radius=" << this->central_radius <<
          ", outer_radius=" << this->outer_radius <<
          "]";
    return ss.str();
}


std::vector<Target> TargetFinder::doTargetRecognition(cv::Mat *inputMat,
                                                      cv::Mat *outputMat) {
    std::vector<Target> final_targets;
    this->markers.clear();
    cv::Mat processMat;
    inputMat->copyTo(processMat);

    float total_diff = 0.0;
    float total_avg = this->threshold;

    for(int y=0; y < inputMat->rows; y++) {
        unsigned char *p = inputMat->ptr(y);
        unsigned char *m = processMat.ptr(y);
        unsigned char *o;
        if (outputMat != NULL) {
            o = outputMat->ptr(y);
        }
        bool in_black = false;
        bool in_white = false;
        bool has_changed = false;
        bool left_black = false;
        int left_black_count = 0;
        bool left_white = false;
        int left_white_count = 0;
        bool central_black = false;
        int central_black_count = 0;
        bool right_black = false;
        int right_black_count = 0;
        bool right_white = false;
        int right_white_count = 0;
        int left_x = 0;
        int right_x = 0;
        float horiz_avg = 0;
        float last_horiz_avg = 0;
        float horiz_delta = 0;
        float horiz_diff = 0;
        float horiz_incr = 0;
        float horiz_pwr_sum_avg = 0;
        float horiz_std = 0;
        float horiz_var = 0;
        float horiz_delta_abs = 0;
        for(int x=0; x < inputMat->cols; x++) {
            horiz_diff = p[3 * x] - horiz_avg;
            total_diff = p[3 * x] - total_avg;
            total_avg += (this->omega * total_diff);
            horiz_incr = this->alpha * horiz_diff;
            horiz_avg += horiz_incr;
            horiz_var = (1 - this->alpha) * (horiz_var + horiz_diff * horiz_incr);
            // horiz_avg = (1 - this->alpha) * horiz_avg + (this->alpha * p[3 * x]);
            // horiz_avg += ((float) p[3 * x] - horiz_avg) / this->window;
            // horiz_pwr_sum_avg += (p[3 * x] * p[3 * x] - horiz_pwr_sum_avg) / this->window;
            // horiz_std = sqrt()
            // horiz_std = sqrt((horiz_pwr_sum_avg * this->window - this->window * horiz_avg * horiz_avg) / (this->window - 1));
            // horiz_delta = horiz_avg - last_horiz_avg;
            // horiz_delta_abs = abs(horiz_delta);
            // last_horiz_avg = horiz_avg;

            // If we haven't previously had such a big change
            if (abs(horiz_diff) > this->threshold) {
                if (horiz_diff < 0) {
                    in_black = true;
                    in_white = false;
                } else {
                    in_white = true;
                    in_black = false;
                }
            } else {
                //if(horiz_var < this->threshold) {
                    in_white = false;
                    in_black = false;
                //}
            }

            // Start of marker, black area
            if(in_black && !left_black && !left_white && !central_black && !right_white && !right_black) {
                left_black = true;
                left_x = x;
            }

            // Central black area
            if((!in_white) && left_white) {
                left_black = false;
                central_black = true;
                right_black = false;
                left_white = false;
                right_white = false;
            }

            // Right-hand black side
            if((!in_white) && right_white) {
                left_black = false;
                central_black = false;
                right_black = true;
                left_white = false;
                right_white = false;
            }

            // White area to the left of center
            if((!in_black) && left_black) {
                left_black = false;
                central_black = false;
                right_black = false;
                left_white = true;
                right_white = false;
            }

            // White area to the right of center
            if((!in_black) && central_black) {
                left_black = false;
                central_black = false;
                right_black = false;
                left_white = false;
                right_white = true;
            }

            if((!in_white) && left_black) {
                left_black_count++;
            } else if((!in_white) && right_black) {
                right_black_count++;
            } else if((!in_white) && central_black) {
                central_black_count++;
            } else if((!in_black) && left_white) {
                left_white_count++;
            } else if((!in_black) && right_white) {
                right_white_count++;
            }

            // End of marker
            if((!in_black && right_black)
               || (!in_black && left_white && left_black)
               || (!in_black && right_white && right_black)) {
                left_black = false;
                central_black = false;
                right_black = false;
                left_white = false;
                right_white = false;
                right_x = x;

                int black_count = left_black_count + central_black_count + right_black_count;
                int white_count = left_white_count + right_white_count;
                float black_white_ratio = (float) black_count / (float) white_count;
                // std::cout << "black_count = " << black_count << ", white_count = " << white_count << std::endl;
                // std::cout << "black_white_ratio = " << black_white_ratio << std::endl;

                if(abs(1 - black_white_ratio) > this->black_white_ratio_threshold) {
                    // markers.push_back(Marker(y, y, left_x, right_x));
                }

                // Reset counters
                left_black_count = 0;
                central_black_count = 0;
                right_black_count = 0;
                left_white_count = 0;
                right_white_count = 0;
            }

            if(left_black) {
                o[3 * x] = 0;
                o[3 * x + 1] = 0;
                o[3 * x + 2] = 255;
            } else if(left_white) {
                o[3 * x] = 0;
                o[3 * x + 1] = 128;
                o[3 * x + 2] = 255;
            } else if(central_black) {
                o[3 * x] = 0;
                o[3 * x + 1] = 255;
                o[3 * x + 2] = 255;
            } else if(right_white) {
                o[3 * x] = 0;
                o[3 * x + 1] = 255;
                o[3 * x + 2] = 0;
            } else if(right_black) {
                o[3 * x] = 255;
                o[3 * x + 1] = 128;
                o[3 * x + 2] = 0;
            } else if(in_black) {
                o[3 * x] = 0;
                o[3 * x + 1] = 0;
                o[3 * x + 2] = 0;
            } else if(in_white) {
                o[3 * x] = 255;
                o[3 * x + 1] = 255;
                o[3 * x + 2] = 255;
            } else {
                o[3 * x] = 64;
                o[3 * x + 1] = 64;
                o[3 * x + 2] = 64;
            }
            /* if(horiz_var > 255) {
                horiz_var = 255;
            } else if(horiz_var < 0) {
                horiz_var = 0;
            }
            o[3 * x] = (uchar) horiz_var;
            o[3 * x + 1] = (uchar) horiz_var;
            o[3 * x + 2] = (uchar) horiz_var; */
        }
    }

    for(int y=0; y < markers.size(); y++) {
        for(int x=markers.size(); x > 0; x--) {
            if (&markers[x] == &markers[y]) {
                // std::cout << "equal elements" << std::endl;
                continue;
            }
            if (Marker::distance(markers[y], markers[x]) < this->inner_marker_distance) {
                markers[y].expand(markers[x]);
                markers[x].invalid = true;
            }
        }
    }
    for(int y=0; y < markers.size(); y++) {
        if(markers[y].getWidth() == 0 || markers[y].getHeight() == 0) {
            markers[y].invalid = true;
            continue;
        }
        float r = markers[y].getRatio();
        if(r < 0 || r > this->max_marker_ratio || r < this->min_marker_ratio || isinf(r)) {
            markers[y].invalid = true;
        }
    }
    int i=0;
    for(int y=0; y < markers.size(); y++) {
        if(markers[y].invalid) {
            continue;
        }
        int cy = markers[y].getCenterY();
        int cx = markers[y].getCenterX();
        // std::cout << i++ << ": " << markers[y].toString() << std::endl;
        cv::circle(*outputMat, cv::Point(cx, cy), markers[y].getWidth() / 2, 0xFF00FF, -1);
    }

    this->threshold = total_avg;

    return final_targets;
}
