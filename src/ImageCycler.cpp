//
// Created by owain on 21/07/15.
//

#include <dirent.h>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include "ImageCycler.h"


qtpp::ImageCycler::ImageCycler(const char* directory) {
    this->directory = directory;
    this->files.clear();
    DIR* dirp = opendir(directory);
    std::string dir = std::string(this->directory);
    bool addslash = dir.rfind('/') != strlen(directory) - 1;
    dirent* dp;
    while((dp = readdir(dirp)) != NULL) {
        char* extp = strrchr(dp->d_name, '.');
        // std::cout << extp << std::endl;
        if(strcasecmp(extp, ".jpg") == 0 || strcasecmp(extp, ".png") == 0 ||
                strcasecmp(extp, ".jpeg") == 0 || strcasecmp(extp, ".tif") == 0 ||
                strcasecmp(extp, ".gif") == 0 || strcasecmp(extp, ".bmp") == 0) {
            std::stringstream ss;
            ss << directory;
            if(addslash) {
                ss << "/";
            }
            ss << dp->d_name;
            // std::cout << full << std::endl;
            this->files.push_back(ss.str());
        }
    }

}


bool qtpp::ImageCycler::read(cv::OutputArray dest) {
    if(this->files.size() == 0) {
        return false;
    }
    std::string image_file = this->files[this->idx % this->files.size()];
    std::cout << this->idx << ": " << image_file << std::endl;
    cv::Mat image = cv::imread(image_file, cv::IMREAD_COLOR);
    if(image.cols == 0) {
        return false;
    }
    // std::cout << image.rows << "x" << image.cols << "x" << image.channels() << std::endl;
    this->idx++;
    image.copyTo(dest);
    return true;
}


void qtpp::ImageCycler::set(void *key, void *val) {
    // this is meant to not do anything (for now).
}


void qtpp::ImageCycler::open(const char* directory) {
    // same.
}


bool qtpp::ImageCycler::isOpened() {
    return true;
}
