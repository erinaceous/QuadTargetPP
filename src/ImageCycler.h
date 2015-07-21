//
// Acts like a VideoCapture class. Cycles over a directory of images. Only the
// constructor and
// Created by owain on 21/07/15.
//

#ifndef QUADTARGETPLUSPLUS_IMAGECYCLER_H
#define QUADTARGETPLUSPLUS_IMAGECYCLER_H

#include <opencv2/core/core.hpp>

namespace qtpp {

    class ImageCycler : public cv::VideoCapture {
        public:
            ImageCycler(const char* directory);
            void open(const char* directory);
            bool read(cv::OutputArray dest);
            void set(void *key, void *val);
            bool isOpened();
        private:
            const char* directory;
            std::vector<std::string> files;
            unsigned long idx = 0;
    };

}
#endif //QUADTARGETPLUSPLUS_IMAGECYCLER_H
