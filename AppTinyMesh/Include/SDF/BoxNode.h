#pragma once

#include "Node.h"

class BoxNode : public Node
{
public:
    BoxNode(const Vector& p_min, const Vector& p_max);

    ~BoxNode() = default;

    double Value(const Vector& p) const override;

private:
    Vector p_min_;
    Vector p_max_;
};