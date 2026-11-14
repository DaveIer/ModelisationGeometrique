#pragma once

#include "ray.h"
#include "mathematics.h"
#include "SDF/Node.h"

struct SphereTraceParams {
    double t_min = 0.0;       // début de la recherche le long du rayon
    double t_max = 100.0;     // fin de la recherche
    double epsilon = 1e-4;    // seuil de hit : f(p) < epsilon
    int max_steps = 256;      // garde-fou contre les incidences rasantes
};

// Précondition : ray.Direction() est unitaire (c'est le contrat de la classe Ray),
// donc t est une vraie distance en unités de monde.
// Retourne true s'il y a intersection ; t_hit et hit_point sont alors remplis.
bool SphereTrace(const Node& root, const Ray& ray, const SphereTraceParams& params,
    double& t_hit, Vector& hit_point);

// Normale par différences finies centrées sur Value().
Vector EstimateNormal(const Node& root, const Vector& p, double h = 1e-4);