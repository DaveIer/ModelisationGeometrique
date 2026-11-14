#include "SDF/SphereTracing.h"

#include <cmath>
#include <stdexcept>

bool SphereTrace(const Node& root, const Ray& ray, const SphereTraceParams& params, double& t_hit, Vector& hit_point)
{
    const double k = root.Lipschitz();

    if (!(k > 0.0) || !std::isfinite(k))
        throw std::invalid_argument("Invalid Lipschitz constant");

    const double inv_k = 1.0 / k;
    const Vector origin = ray.Origin();
    const Vector direction = ray.Direction();

    double t = params.t_min;

    for (int i = 0; i < params.max_steps && t <= params.t_max; ++i) {
        const Vector p = origin + t * direction;
        const double d = root.Value(p);

        // d < epsilon (et non |d|) : un rayon qui démarre à l'intérieur
        // (d < 0) est un hit immédiat.
        if (d < params.epsilon) {
            t_hit = t;
            hit_point = p;
            return true;
        }

        // Pas sûr : la surface est à au moins d / K.
        t += d * inv_k;
    }

    return false;
}

Vector EstimateNormal(const Node& root, const Vector& p, double h)
{
    const double dx = root.Value(p + Vector(h, 0.0, 0.0)) - root.Value(p - Vector(h, 0.0, 0.0));
    const double dy = root.Value(p + Vector(0.0, h, 0.0)) - root.Value(p - Vector(0.0, h, 0.0));
    const double dz = root.Value(p + Vector(0.0, 0.0, h)) - root.Value(p - Vector(0.0, 0.0, h));

    const Vector n(dx, dy, dz);
    const double length = Norm(n);

    // Gradient nul (point singulier) : on renvoie le vecteur tel quel.
    if (!(length > 0.0))
        return n;

    return n * (1.0 / length);
}