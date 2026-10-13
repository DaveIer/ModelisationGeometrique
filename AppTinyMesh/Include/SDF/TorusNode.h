#pragma once

#include "mathematics.h"
#include "Node.h"

class TorusNode : public Node {
public:
    TorusNode();
    TorusNode(const TorusNode& Torus);
    ~TorusNode() = default;

    double Value(const Vector& p) const;
};