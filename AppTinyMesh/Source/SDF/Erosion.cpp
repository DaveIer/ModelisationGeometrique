#include "SDF/Erosion.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace {

    // Vue en lecture seule d'un ErosionNode qui compte les appels à Value.
    // Une instance par rayon : les compteurs ne sont jamais partagés entre threads.
    class CountingView : public Node {
    public:
        explicit CountingView(const ErosionNode& node) : node_(node) {}

        double Value(const Vector& p) const override
        {
            ++value_calls;
            return node_.ValueCounted(p, sphere_tests);
        }

        double Lipschitz() const override { return node_.Lipschitz(); }

        mutable long long value_calls = 0;
        mutable long long sphere_tests = 0;

    private:
        const ErosionNode& node_;
    };

    struct Shot {
        Ray ray;
        double radius;
    };

    int ThreadCount()
    {
#ifdef _OPENMP
        return omp_get_max_threads();
#else
        return 1;
#endif
    }

    Vector RandomUnitVector(std::mt19937& rng)
    {
        std::normal_distribution<double> gauss(0.0, 1.0);
        Vector v;
        do {
            const double x = gauss(rng);
            const double y = gauss(rng);
            const double z = gauss(rng);
            v = Vector(x, y, z);
        } while (SquaredNorm(v) < 1e-12);

        return Normalized(v);
    }

    void Validate(const ErosionParams& p)
    {
        if (p.ray_count <= 0)
            throw std::invalid_argument("ray_count must be > 0");
        if (!(p.radius_min > 0.0) || !(p.radius_max >= p.radius_min))
            throw std::invalid_argument("Invalid erosion radius range");
        if (!(p.depth_ratio > 0.0) || p.depth_ratio > 1.0)
            throw std::invalid_argument("depth_ratio must be in ]0, 1]");
        if (!(p.aim_radius > 0.0) || !(p.scene_radius > p.aim_radius))
            throw std::invalid_argument("scene_radius must be > aim_radius > 0");
    }

    // Tous les rayons et rayons de sphères sont tirés à l'avance, uniquement à partir de la graine :
    // ils sont identiques quelle que soit la taille de paquet, et ne dépendent pas des impacts.
    std::vector<Shot> GenerateShots(const ErosionParams& p)
    {
        std::mt19937 rng(p.seed);
        std::uniform_real_distribution<double> uniform(0.0, 1.0);

        std::vector<Shot> shots;
        shots.reserve(p.ray_count);

        for (int i = 0; i < p.ray_count; ++i) {
            const Vector origin = p.center + p.scene_radius * RandomUnitVector(rng);

            const double aim = p.aim_radius * std::cbrt(uniform(rng));
            const Vector target = p.center + aim * RandomUnitVector(rng);

            const double u = uniform(rng);
            const double radius = p.radius_min + (p.radius_max - p.radius_min) * u * u;   // plus de petites sphères

            Shot shot;
            shot.ray = Ray(origin, Normalized(target - origin));
            shot.radius = radius;
            shots.push_back(shot);
        }

        return shots;
    }

    double Median(std::vector<double> v)
    {
        std::sort(v.begin(), v.end());
        const size_t n = v.size();
        return (n % 2 == 1) ? v[n / 2] : 0.5 * (v[n / 2 - 1] + v[n / 2]);
    }

} // namespace

ErosionStats Erode(ErosionNode& node, const ErosionParams& params)
{
    Validate(params);

    node.SetOptimized(params.optimized);

    // Précondition de SphereTrace, vérifiée hors de la région parallèle (pas d'exception dans OpenMP).
    const double k = node.Lipschitz();
    if (!(k > 0.0))
        throw std::invalid_argument("Invalid Lipschitz constant");

    const std::vector<Shot> shots = GenerateShots(params);

    SphereTraceParams trace = params.trace;
    trace.t_min = 0.0;
    trace.t_max = params.scene_radius + params.aim_radius;

    const int batch = std::max(1, params.batch_size);

    ErosionStats stats;
    stats.batch_size = batch;
    stats.parallel = params.parallel;
    stats.optimized = params.optimized;
    stats.rays = params.ray_count;

    std::vector<ErosionSphere> found(batch);
    std::vector<char> valid(batch);

    // Suivi par rayon (une case par rayon : pas de conflit entre threads)
    std::vector<int> ray_steps(params.ray_count, 0);
    std::vector<char> ray_hit(params.ray_count, 0);

    const auto start = std::chrono::steady_clock::now();

    for (int begin = 0; begin < params.ray_count; begin += batch) {
        const int count = std::min(batch, params.ray_count - begin);

        const ErosionNode& view_node = node;   // lecture seule pendant le tracé du paquet

        long long batch_calls = 0;
        long long batch_tests = 0;
        int batch_hits = 0;

        // Tous les rayons du paquet voient la même surface (celle d'avant le paquet).
#pragma omp parallel for schedule(dynamic) reduction(+ : batch_calls, batch_tests, batch_hits) if (params.parallel && count > 1)
        for (int i = 0; i < count; ++i) {
            const Shot& shot = shots[begin + i];
            CountingView view(view_node);

            double t;
            Vector hit;
            valid[i] = 0;

            const bool hit_found = SphereTrace(view, shot.ray, trace, t, hit);

            // Chaque itération du tracé appelle Value une fois : à ce stade, value_calls = nombre de pas.
            ray_steps[begin + i] = static_cast<int>(view.value_calls);
            ray_hit[begin + i] = hit_found ? 1 : 0;

            if (hit_found) {
                const Vector n = EstimateNormal(view, hit);

                // Le centre est décalé le long de la normale : depth_ratio règle la profondeur du cratère.
                found[i].radius = shot.radius;
                found[i].center = hit + n * (shot.radius * (1.0 - params.depth_ratio));
                valid[i] = 1;
                ++batch_hits;
            }

            batch_calls += view.value_calls;
            batch_tests += view.sphere_tests;
        }

        // Les sphères du paquet sont ajoutées ensemble, dans l'ordre des rayons (résultat déterministe).
        for (int i = 0; i < count; ++i) {
            if (valid[i]) {
                node.AddSphere(found[i].center, found[i].radius);
                ++stats.spheres;
            }
        }

        stats.value_calls += batch_calls;
        stats.sphere_tests += batch_tests;
        stats.hits += batch_hits;
        ++stats.batches;
    }

    const auto end = std::chrono::steady_clock::now();
    stats.time_ms = std::chrono::duration<double, std::milli>(end - start).count();

    // Suivi du tracé (calculé après le chronométrage)
    stats.step_limit = trace.max_steps;
    long long total_steps = 0;
    for (int i = 0; i < params.ray_count; ++i) {
        total_steps += ray_steps[i];
        stats.steps_max = std::max(stats.steps_max, ray_steps[i]);
        if (!ray_hit[i] && ray_steps[i] >= trace.max_steps)
            ++stats.exhausted_rays;
    }
    stats.steps_mean = double(total_steps) / double(params.ray_count);

    std::vector<int> sorted_steps = ray_steps;
    const size_t k99 = std::min(sorted_steps.size() - 1, size_t(0.99 * double(sorted_steps.size())));
    std::nth_element(sorted_steps.begin(), sorted_steps.begin() + k99, sorted_steps.end());
    stats.steps_p99 = sorted_steps[k99];

    return stats;
}

double EstimateVolume(const Node& node, const Vector& center, double half_size,
    int samples, unsigned int seed)
{
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> uniform(-half_size, half_size);

    // Points tirés une fois pour toutes : deux appels avec la même graine évaluent les mêmes points.
    std::vector<Vector> points(samples);
    for (int i = 0; i < samples; ++i) {
        const double x = uniform(rng);
        const double y = uniform(rng);
        const double z = uniform(rng);
        points[i] = center + Vector(x, y, z);
    }

    long long inside = 0;

#pragma omp parallel for schedule(static) reduction(+ : inside)
    for (int i = 0; i < samples; ++i) {
        if (node.Value(points[i]) < 0.0)
            ++inside;
    }

    const double side = 2.0 * half_size;
    return side * side * side * double(inside) / double(samples);
}

double RemovedVolumePercent(const std::function<std::unique_ptr<Node>()>& make_scene,
    const ErosionNode& eroded, const Vector& center, double half_size,
    int samples, unsigned int seed)
{
    const std::unique_ptr<Node> base = make_scene();

    const double v0 = EstimateVolume(*base, center, half_size, samples, seed);
    const double v1 = EstimateVolume(eroded, center, half_size, samples, seed);

    return (v0 > 0.0) ? 100.0 * (v0 - v1) / v0 : 0.0;
}

std::string FormatErosionStats(const ErosionStats& s, double removed_volume_percent)
{
    std::ostringstream o;
    o << std::fixed << std::setprecision(2);

    o << "Taille de paquet   : " << s.batch_size << (s.batch_size == 1 ? "  (incremental)" : "") << "\n";
    o << "Parallele          : " << (s.parallel ? "oui" : "non") << "\n";
    o << "Value optimisee    : " << (s.optimized ? "oui" : "non") << "\n";
    o << "Rayons / impacts   : " << s.rays << " / " << s.hits << "\n";
    o << "Paquets            : " << s.batches << "\n";
    o << "Spheres ajoutees   : " << s.spheres << "\n";
    o << "Appels a Value     : " << s.value_calls << "\n";
    o << "Tests sphere-point : " << s.sphere_tests << "\n";
    o << "Pas par rayon      : moy " << s.steps_mean << ", p99 " << s.steps_p99 << ", max " << s.steps_max
        << " (limite " << s.step_limit << ")\n";
    o << "Rayons epuises     : " << s.exhausted_rays << "\n";
    o << "Temps d'erosion    : " << s.time_ms << " ms\n";
    o << "Volume retire      : " << removed_volume_percent << " %\n";

    return o.str();
}

std::string RunErosionBenchmark(const std::function<std::unique_ptr<Node>()>& make_scene,
    const ErosionParams& base,
    const std::vector<ErosionConfig>& configs,
    int repetitions, int volume_samples,
    const std::string& csv_path)
{
    struct Row {
        ErosionConfig config;
        ErosionStats stats;
        double median_ms;
        double min_ms;
        double removed_pct;
    };

    const unsigned int volume_seed = 777;
    std::vector<Row> rows;

    for (const ErosionConfig& config : configs) {
        ErosionParams params = base;
        params.batch_size = config.batch_size;
        params.parallel = config.parallel;
        params.optimized = config.optimized;

        std::vector<double> times;
        Row row;
        row.config = config;
        row.removed_pct = 0.0;

        for (int r = 0; r < std::max(1, repetitions); ++r) {
            ErosionNode eroded(make_scene());
            const ErosionStats s = Erode(eroded, params);
            times.push_back(s.time_ms);

            if (r == 0) {
                // Les compteurs et le résultat sont déterministes : une seule mesure de qualité suffit.
                row.stats = s;
                eroded.SetOptimized(true);   // mesure de volume rapide, hors chronométrage
                row.removed_pct = RemovedVolumePercent(make_scene, eroded, base.center, base.scene_radius,
                    volume_samples, volume_seed);
            }
        }

        row.median_ms = Median(times);
        row.min_ms = *std::min_element(times.begin(), times.end());
        rows.push_back(row);
    }

    // ---- Rapport texte ----
    std::ostringstream out;
    out << "Benchmark d'erosion par spheres\n";
    out << "Rayons: " << base.ray_count << "   graine: " << base.seed
        << "   rayons des spheres: [" << base.radius_min << ", " << base.radius_max << "]"
        << "   profondeur: " << base.depth_ratio << "\n";
    out << "Repetitions: " << repetitions << " (temps median)   echantillons de volume: " << volume_samples << "\n";
    out << "max_steps: " << base.trace.max_steps << "   epsilon: " << base.trace.epsilon << "\n";
#ifdef _OPENMP
    out << "OpenMP: actif, " << ThreadCount() << " threads\n\n";
#else
    out << "OpenMP: INACTIF (les lignes 'parallele' sont executees en sequentiel)\n\n";
#endif

    out << std::setw(7) << "Paquet" << " | " << std::setw(3) << "Par" << " | " << std::setw(3) << "Opt" << " | "
        << std::setw(7) << "Paquets" << " | "
        << std::setw(13) << "Appels Value" << " | " << std::setw(15) << "Tests sph-pt" << " | "
        << std::setw(12) << "Median (ms)" << " | " << std::setw(9) << "Min (ms)" << " | "
        << std::setw(8) << "Accel." << " | " << std::setw(9) << "Vol. (%)" << " | "
        << std::setw(8) << "Pas moy" << " | " << std::setw(7) << "Pas p99" << " | "
        << std::setw(7) << "Pas max" << "\n";
    out << std::string(144, '-') << "\n";

    const double reference_ms = rows.empty() ? 1.0 : rows.front().median_ms;

    for (const Row& row : rows) {
        out << std::setw(7) << row.config.batch_size << " | "
            << std::setw(3) << (row.config.parallel ? "oui" : "non") << " | "
            << std::setw(3) << (row.config.optimized ? "oui" : "non") << " | "
            << std::setw(7) << row.stats.batches << " | "
            << std::setw(13) << row.stats.value_calls << " | "
            << std::setw(15) << row.stats.sphere_tests << " | "
            << std::fixed << std::setprecision(2)
            << std::setw(12) << row.median_ms << " | "
            << std::setw(9) << row.min_ms << " | "
            << std::setw(7) << (reference_ms / row.median_ms) << "x | "
            << std::setw(9) << row.removed_pct << " | "
            << std::setw(8) << row.stats.steps_mean << " | "
            << std::setw(7) << row.stats.steps_p99 << " | "
            << std::setw(7) << row.stats.steps_max << "\n";
    }

    out << "\nAccel. : rapport au temps median de la premiere ligne.\n";
    out << "Vol. : pourcentage du volume de la scene de base retire par l'erosion.\n";
    out << "Pas : iterations du sphere tracing par rayon (moyenne, 99e percentile, maximum), normales exclues.\n";

    // ---- CSV ----
    if (!csv_path.empty()) {
        std::ofstream csv(csv_path);
        if (csv.good()) {
            csv << "batch_size,parallel,optimized,rays,hits,batches,spheres,value_calls,sphere_tests,"
                "median_ms,min_ms,speedup,removed_volume_percent,steps_mean,steps_p99,steps_max,exhausted_rays\n";
            for (const Row& row : rows) {
                csv << row.config.batch_size << ',' << (row.config.parallel ? 1 : 0) << ','
                    << (row.config.optimized ? 1 : 0) << ',' << row.stats.rays << ',' << row.stats.hits << ','
                    << row.stats.batches << ',' << row.stats.spheres << ',' << row.stats.value_calls << ','
                    << row.stats.sphere_tests << ',' << row.median_ms << ',' << row.min_ms << ','
                    << (reference_ms / row.median_ms) << ',' << row.removed_pct << ','
                    << row.stats.steps_mean << ',' << row.stats.steps_p99 << ',' << row.stats.steps_max << ','
                    << row.stats.exhausted_rays << '\n';
            }
            out << "CSV ecrit dans : " << csv_path << "\n";
        }
        else {
            out << "Impossible d'ecrire le CSV : " << csv_path << "\n";
        }
    }

    return out.str();
}