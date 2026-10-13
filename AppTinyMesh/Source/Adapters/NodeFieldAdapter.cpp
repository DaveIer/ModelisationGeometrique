#include "Adapters/NodeFieldAdapter.h"

NodeFieldAdapter::NodeFieldAdapter(const Node& n) : node(n) {}

double NodeFieldAdapter::Value(const Vector& p) const {
    return this->node.Value(p);
}