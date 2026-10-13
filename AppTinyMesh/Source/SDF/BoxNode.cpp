#include "SDF/BoxNode.h"

BoxNode::BoxNode(const Vector& p_min, const Vector& p_max) : p_min_(p_min), p_max_(p_max) {}

double BoxNode::Value(const Vector& p) const {
    const Vector& center = (p_min_ + p_max_) * 0.5;
    const Vector& half = (p_max_ - p_min_) * 0.5;

    const Vector& q = Abs(p - center) - half;

    const double out = Norm(Vector::Max(q, Vector(0.0)));
    const double in = std::min((std::max(q[0], std::max(q[1], q[2]))), 0.0);

    return out + in;
}