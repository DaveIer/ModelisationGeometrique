#pragma once

#include "Node.h"

class BoxNode : public Node
{
public:
	// constructor with minimum and maximum points of the box
    BoxNode(const Vector& p_min, const Vector& p_max);

    ~BoxNode() = default;

	// override the Value function to compute the signed distance from a point to the box
    double Value(const Vector& p) const override;

private:
    Vector p_min_;
    Vector p_max_;
};