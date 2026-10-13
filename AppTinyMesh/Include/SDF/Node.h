#pragma once

#include "mathematics.h"

class Node {
public:
    Node() = default;
    virtual ~Node() = default;

    virtual double Value(const Vector& p) const = 0;
};