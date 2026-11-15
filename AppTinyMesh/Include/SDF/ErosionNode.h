#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "SDF/UnaryNode.h"

struct ErosionSphere {
    Vector center;
    double radius;
};

// Objet érodé : child privé de l'union des sphères d'érosion.
// Un seul noeud contient toutes les sphères (liste plate), pas un nœud par impact.
// K = max(K_child, 1) car chaque sphère est 1-Lipschitz.
class ErosionNode : public UnaryNode {
public:
    explicit ErosionNode(std::unique_ptr<Node> child);

    void AddSphere(const Vector& center, double radius);
    void Clear();
    std::size_t SphereCount() const { return spheres_.size(); }

    // false : version naïve (sqrt pour toutes les sphères à chaque appel)
    // true  : version optimisée (sortie anticipée loin de la surface + test sans sqrt)
    void SetOptimized(bool optimized) { optimized_ = optimized; }
    bool IsOptimized() const { return optimized_; }

    double Value(const Vector& p) const override;
    double Lipschitz() const override;

    // Comme Value, mais ajoute à sphere_tests le nombre de distances sphère-point
    // réellement calculées (sert aux mesures de performance).
    double ValueCounted(const Vector& p, long long& sphere_tests) const;

private:
    double Evaluate(const Vector& p, long long* sphere_tests) const;

    std::vector<ErosionSphere> spheres_;
    double max_radius_ = 0.0;
    bool optimized_ = true;
};
