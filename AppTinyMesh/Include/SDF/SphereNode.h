#pragma once

#include "Node.h"

class SphereNode : public Node {
public:
    SphereNode(double r); // constructeur avec rayon (sphère centrée à l'origine)
    SphereNode(Vector center, double r); // constructeur par paramètres
    SphereNode(const SphereNode& sphere); // constructeur par copie

    ~SphereNode() = default;

    double Value(const Vector& p) const override;

private:
    Vector center_;
    double r_;
};