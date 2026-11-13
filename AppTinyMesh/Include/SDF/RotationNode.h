#pragma once

#include "UnaryNode.h"

class RotationNode : public UnaryNode
{
public:
	RotationNode(std::unique_ptr<Node> child, const Vector& axis, double angle, const Vector& center = Vector(0.0));
    double Value(const Vector& p) const override;
    double Lipschitz() const override;

private:
    Vector axis_;
    Vector center_;

    double cos_angle_;
    double sin_angle_;
};