#pragma once

#include "implicits.h"
#include "SDF/Node.h"

class NodeFieldAdapter : public AnalyticScalarField {
public:
    NodeFieldAdapter(const Node& n);

    double Value(const Vector& p) const override;

private:
    const Node& node;
};