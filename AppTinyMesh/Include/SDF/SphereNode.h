#pragma once

#include "Node.h"

class SphereNode : public Node {
public:
	SphereNode(double r); // constructor with radius
    SphereNode(const Vector& center, double r); // constructor with center and radius
    SphereNode(const SphereNode& sphere); // constructor by copy

    ~SphereNode() = default;

	// override the Value function to compute the signed distance from a point to the sphere
    double Value(const Vector& p) const override;

private:
    Vector center_;
    double r_;
};