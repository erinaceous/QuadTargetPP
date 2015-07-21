//
// Created by owain on 07/07/15.
//

#include "Marker.h"

using namespace qtpp;

Marker::Marker(const int top_y, const int bottom_y,
               const int left, const int right) {
    this->left_x = left;
    this->right_x = right;
    this->top_y = top_y;
    this->bottom_y = bottom_y;
}

int Marker::getLeftX() {
    return this->left_x;
}

int Marker::getRightX() {
    return this->right_x;
}

int Marker::getTopY() {
    return this->top_y;
}

int Marker::getBottomY() {
    return this->bottom_y;
}

int Marker::getCenterX() {
    return this->left_x + ((this->right_x - this->left_x) / 2);
}

int Marker::getCenterY() {
    return this->top_y + ((this->bottom_y - this->top_y) / 2);
}

int Marker::getWidth() {
    return this->right_x - this->left_x;
}

int Marker::getHeight() {
    return this->bottom_y - this->top_y;
}

float Marker::getRatio() {
    return (float) this->getWidth() / (float) this->getHeight();
}

void Marker::setTopY(int top) {
    this->top_y = top;
}

void Marker::setBottomY(int bottom) {
    this->bottom_y = bottom;
}

void Marker::setLeftX(int left) {
    this->left_x = left;
}

void Marker::setRightX(int right) {
    this->right_x = right;
}

void Marker::expand(Marker other) {
    if(other.top_y < this->top_y) {
        this->top_y = other.top_y;
    }
    if(other.bottom_y > this->bottom_y) {
        this->bottom_y = other.bottom_y;
    }
    if(other.left_x < this->left_x) {
        this->left_x = other.left_x;
    }
    if(other.right_x > this->right_x) {
        this->right_x = other.right_x;
    }
}

Marker Marker::expand(Marker m1, Marker m2) {
    Marker m3 = Marker(
            m1.top_y, m1.bottom_y, m1.left_x, m1.right_x
    );

    if(m2.top_y < m3.top_y) {
        m3.top_y = m2.top_y;
    }
    if(m2.bottom_y > m3.bottom_y) {
        m3.bottom_y = m2.bottom_y;
    }
    if(m2.left_x < m3.left_x) {
        m3.left_x = m2.left_x;
    }
    if(m2.right_x > m3.right_x) {
        m3.right_x = m2.right_x;
    }
    return m3;
}

int Marker::distance(Marker m1, Marker m2) {
    /* return abs(m2.getCenterX() - m1.getCenterX()) +
           abs(m2.getCenterY() - m2.getCenterY()); */
    return (int) sqrt(
            pow((m1.getCenterX() - m2.getCenterX()), 2) +
            pow((m1.getCenterY() - m2.getCenterY()), 2)
    );
}

std::string Marker::toString() {
    std::stringstream ss;
    ss << "Marker[top=" << this->top_y << ", left=" << this->left_x
            << ", bottom=" << this->bottom_y << ", right=" << this->right_x
            << ", center_x=" << this->getCenterX()
            << ", center_y=" << this->getCenterY()
            << ", width=" << this->getWidth()
            << ", height=" << this->getHeight()
            << ", ratio=" << this->getRatio() << "]";
    return ss.str();
}
