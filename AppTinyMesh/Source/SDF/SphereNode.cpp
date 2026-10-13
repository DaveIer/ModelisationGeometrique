#include "SDF/SphereNode.h"

SphereNode::SphereNode(double r) : center_(0.0, 0.0, 0.0), r_(r) {}

SphereNode::SphereNode(Vector center, double r) : center_(center), r_(r) {}

SphereNode::SphereNode(const SphereNode& sphere) {
    this->r_ = sphere.r_;
    this->center_ = sphere.center_;
}

double SphereNode::Value(const Vector& p) const {
    return Norm(p - this->center_) - this->r_;
}