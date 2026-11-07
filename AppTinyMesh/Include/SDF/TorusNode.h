#pragma once

#include "Node.h"
#include <cmath>

class TorusNode : public Node {
public:
	TorusNode(double R, double r); // constructor with major radius R and minor radius r
	TorusNode(const Vector& center, double R, double r); // constructor with center, major radius R and minor radius r
	TorusNode(const TorusNode& Torus) = default; // copy constructor

    ~TorusNode() = default;

	// override the Value function to compute the signed distance from a point to the torus
    double Value(const Vector& p) const;

private:
    Vector center_;
    double R_;
	double r_;
};