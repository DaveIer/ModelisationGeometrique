#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "mathematics.h"
#include "SDF/Node.h"
#include "SDF/ErosionNode.h"
#include "SDF/SphereTracing.h"

struct ErosionParams {
    int ray_count = 3000;          // nombre de rayons tirés
    int batch_size = 1;            // 1 = incrémental ; ray_count = un seul paquet
    bool parallel = false;         // rayons d'un paquet en parallèle (OpenMP)
    bool optimized = true;         // ErosionNode::Value optimisée ou naïve
    unsigned int seed = 12345;     // même graine => mêmes rayons dans tous les modes

    // Les rayons partent d'une sphère englobante et visent une boule autour du centre.
    Vector center = Vector(0.0, 0.0, 0.0);
    double scene_radius = 1.6;     // rayon de la sphère d'où partent les rayons (> aim_radius)
    double aim_radius = 1.0;       // rayon de la zone visée

    // Sphères d'érosion
    double radius_min = 0.02;
    double radius_max = 0.08;
    double depth_ratio = 0.6;      // profondeur du cratère / rayon, dans ]0, 1]

    SphereTraceParams trace;       // t_min et t_max sont imposés par Erode
};

struct ErosionStats {
    int batch_size = 1;
    bool parallel = false;
    bool optimized = true;

    int rays = 0;                  // rayons tirés
    int hits = 0;                  // impacts
    int batches = 0;               // nombre de paquets
    int spheres = 0;               // sphères ajoutées

    long long value_calls = 0;     // appels à ErosionNode::Value (tracé + normales)
    long long sphere_tests = 0;    // distances sphère-point réellement calculées
    double time_ms = 0.0;          // temps de l'érosion seule (hors génération des rayons)

    // Suivi du sphere tracing des rayons d'érosion (estimation des normales exclue)
    int step_limit = 0;            // max_steps utilisé
    double steps_mean = 0.0;       // nombre moyen de pas par rayon
    int steps_p99 = 0;             // 99e percentile du nombre de pas
    int steps_max = 0;             // nombre maximal de pas observé
    int exhausted_rays = 0;        // rayons arrêtés par max_steps sans impact (faux « ratés » possibles)
};

struct ErosionConfig {
    int batch_size;
    bool parallel;
    bool optimized;
};

// Érode l'objet enveloppé par node. Seul le calcul de l'érosion est chronométré.
ErosionStats Erode(ErosionNode& node, const ErosionParams& params);

// Volume estimé (Monte-Carlo) de {Value < 0} dans le cube center +/- half_size.
double EstimateVolume(const Node& node, const Vector& center, double half_size,
    int samples, unsigned int seed);

// Pourcentage du volume de la scène de base retiré par l'érosion (mêmes points d'échantillonnage).
double RemovedVolumePercent(const std::function<std::unique_ptr<Node>()>& make_scene,
    const ErosionNode& eroded, const Vector& center, double half_size,
    int samples, unsigned int seed);

// Résumé lisible d'une exécution.
std::string FormatErosionStats(const ErosionStats& stats, double removed_volume_percent);

// Lance toutes les configurations (repetitions fois chacune, temps médian), retourne le tableau
// de résultats sous forme de texte et l'écrit aussi en CSV (si csv_path n'est pas vide).
std::string RunErosionBenchmark(const std::function<std::unique_ptr<Node>()>& make_scene,
    const ErosionParams& base,
    const std::vector<ErosionConfig>& configs,
    int repetitions, int volume_samples,
    const std::string& csv_path);