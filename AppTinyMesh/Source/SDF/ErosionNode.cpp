#include "SDF/ErosionNode.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

ErosionNode::ErosionNode(std::unique_ptr<Node> child)
    : UnaryNode(std::move(child))
{
}

void ErosionNode::AddSphere(const Vector& center, double radius)
{
    if (!(radius > 0.0) || !std::isfinite(radius))
        throw std::invalid_argument("Invalid erosion sphere radius");

    ErosionSphere s;
    s.center = center;
    s.radius = radius;
    spheres_.push_back(s);

    max_radius_ = std::max(max_radius_, radius);
}

void ErosionNode::Clear()
{
    spheres_.clear();
    max_radius_ = 0.0;
}

double ErosionNode::Value(const Vector& p) const
{
    return Evaluate(p, nullptr);
}

double ErosionNode::ValueCounted(const Vector& p, long long& sphere_tests) const
{
    return Evaluate(p, &sphere_tests);
}

double ErosionNode::Lipschitz() const
{
    return std::max(child_->Lipschitz(), 1.0);
}

double ErosionNode::Evaluate(const Vector& p, long long* sphere_tests) const
{
    const double base = child_->Value(p);

    // Version naïve : formule directe, une racine carrée par sphère.
    if (!optimized_) {
        double cut = -std::numeric_limits<double>::infinity();

        for (const ErosionSphere& s : spheres_)
            cut = std::max(cut, s.radius - Norm(p - s.center));

        if (sphere_tests)
            *sphere_tests += static_cast<long long>(spheres_.size());

        return std::max(base, cut);
    }

    // Version optimisée.
    // 1) Chaque sphère donne r_i - |p - c_i| <= r_i <= max_radius_ : si base >= max_radius_,
    //    aucune sphère ne peut changer le résultat.
    if (base >= max_radius_)
        return base;

    // 2) Une sphère ne compte que si r_i - |p - c_i| > result, c'est-à-dire
    //    |p - c_i| < r_i - result : on compare les carrés, la racine n'est calculée que si elle compte.
    double result = base;
    long long tests = 0;

    for (const ErosionSphere& s : spheres_) {
        const double reach = s.radius - result;
        if (reach <= 0.0)
            continue;

        const double d2 = SquaredNorm(p - s.center);
        ++tests;

        if (d2 < reach * reach)
            result = s.radius - std::sqrt(d2);
    }

    if (sphere_tests)
        *sphere_tests += tests;

    return result;
}
