#include "SDF/BoxNode.h"

// ancien constructeur
//BoxNode::BoxNode(const Vector& p_min, const Vector& p_max) : p_min_(p_min), p_max_(p_max) {}

// nouvelle version du constructeur - calculer le centre et les demi dimensions pour optimiser le temps de calcul
BoxNode::BoxNode(const Vector& p_min, const Vector& p_max) 
    : center_((p_min + p_max) * 0.5), half_((p_max - p_min) * 0.5) {}

double BoxNode::Value(const Vector& p) const {
    // ancienne version
    //const Vector& center = (p_min_ + p_max_) * 0.5;
    //const Vector& half = (p_max_ - p_min_) * 0.5;
    //const Vector& q = Abs(p - center) - half;

    // nouvelle version - on connait déjà le centre et les demi dimensions, pas besoin de les calculer
    const Vector& q = Abs(p - center_) - half_;

    const double out = Norm(Vector::Max(q, Vector(0.0)));
    const double in = std::min((std::max(q[0], std::max(q[1], q[2]))), 0.0);

    return out + in;
}

double BoxNode::Lipschitz() const { return 1.0; } // Exact SDF
