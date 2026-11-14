#include "Benchmarks/AccelerationBenchmark.h"

#include "SDF/UnionNode.h"
#include "SDF/AcceleratedUnionNode.h"
#include "SDF/BoundingVolumeNode.h"
#include "SDF/SphereNode.h"
#include "SDF/BoxNode.h"

#include <iostream>
#include <memory>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <vector>

namespace
{
    class CountingNode : public Node
    {
    public:
        CountingNode(std::unique_ptr<Node> child, int& counter) : child_(std::move(child)), counter_(counter) {}

        double Value(const Vector& p) const override
        {
            ++counter_;
            return child_->Value(p);
        }

        double Lipschitz() const override
        {
            return child_->Lipschitz();
        }

    private:
        std::unique_ptr<Node> child_;
        int& counter_;
    };


    std::vector<Vector> GeneratePoints()
    {
        std::vector<Vector> points;
        points.reserve(4096);

        const double step = 3.0 / 15.0;

        for (int i = 0; i < 16; ++i)
        {
            for (int j = 0; j < 16; ++j)
            {
                for (int k = 0; k < 16; ++k)
                {
                    points.emplace_back(-1.5 + i * step, -1.5 + j * step, -1.5 + k * step);
                }
            }
        }

        return points;
    }

    std::unique_ptr<Node> CreateSphereGroup(double centerX)
    {
        std::unique_ptr<Node> group;

        const double offsets[] = { -0.25, 0.25 };

        for (double dx : offsets)
        {
            for (double dy : offsets)
            {
                for (double dz : offsets)
                {
                    auto sphere = std::make_unique<SphereNode>(Vector(centerX + dx, dy, dz), 0.3);

                    if (!group)
                        group = std::move(sphere);
                    else
                        group = std::make_unique<UnionNode>(std::move(group), std::move(sphere));
                }
            }
        }

        return group;
    }

    std::unique_ptr<Node> CreateStandardUnion()
    {
        //return std::make_unique<UnionNode>(std::make_unique<SphereNode>(Vector(-1.0, 0.0, 0.0), 0.5), std::make_unique<SphereNode>(Vector(1.0, 0.0, 0.0), 0.5));

        return std::make_unique<UnionNode>(CreateSphereGroup(-1.0), CreateSphereGroup(1.0));
    }

    std::unique_ptr<Node> CreateAcceleratedUnion(bool sphereBounds = false, int* left_calls = nullptr, int* right_calls = nullptr)
    {
        std::unique_ptr<Node> left = CreateSphereGroup(-1.0);
        std::unique_ptr<Node> right = CreateSphereGroup(1.0);

        if (left_calls != nullptr)
            left = std::make_unique<CountingNode>(std::move(left), *left_calls);

        if (right_calls != nullptr)
            right = std::make_unique<CountingNode>(std::move(right), *right_calls);

        std::unique_ptr<Node> left_volume;
        std::unique_ptr<Node> right_volume;

        if (sphereBounds)
        {
            const double radius = 0.25 * std::sqrt(3.0) + 0.3;

            left_volume = std::make_unique<SphereNode>(Vector(-1.0, 0.0, 0.0), radius);
            right_volume = std::make_unique<SphereNode>(Vector(1.0, 0.0, 0.0), radius);
        }
        else
        {
            left_volume = std::make_unique<BoxNode>(Vector(-1.55, -0.55, -0.55), Vector(-0.45, 0.55, 0.55));
            right_volume = std::make_unique<BoxNode>(Vector(0.45, -0.55, -0.55), Vector(1.55, 0.55, 0.55));
        }

        return std::make_unique<AcceleratedUnionNode>(
            std::make_unique<BoundingVolumeNode>(std::move(left), std::move(left_volume)),
            std::make_unique<BoundingVolumeNode>(std::move(right), std::move(right_volume))
        );
    }

    void TestAcceleratedUnion()
    {
        const auto points = GeneratePoints();

        int box_left_calls = 0;
        int box_right_calls = 0;
        int sphere_left_calls = 0;
        int sphere_right_calls = 0;

        auto standard = CreateStandardUnion();
        auto acceleratedBox = CreateAcceleratedUnion(
            false, &box_left_calls, &box_right_calls
        );
        auto accelerated_sphere = CreateAcceleratedUnion(
            true, &sphere_left_calls, &sphere_right_calls
        );

        int box_errors = 0;
        int sphere_errors = 0;
        double box_max_error = 0.0;
        double sphere_max_error = 0.0;

        for (const Vector& p : points)
        {
            const double d = standard->Value(p);

            const double boxError = std::abs(d - acceleratedBox->Value(p));

            const double sphereError = std::abs(d - accelerated_sphere->Value(p));

            if (boxError > 1e-9)
                ++box_errors;

            if (sphereError > 1e-9)
                ++sphere_errors;

            box_max_error = std::max(box_max_error, boxError);
            sphere_max_error = std::max(sphere_max_error, sphereError);
        }

        std::cout << "Bounding Boxes: " << box_errors
            << " errors, max error = " << box_max_error
            << ", sphere evaluations = "
            << 8 * (box_left_calls + box_right_calls)
            << " / " << 16 * points.size() << std::endl;

        std::cout << "Bounding spheres: " << sphere_errors
            << " errors, max error = " << sphere_max_error
            << ", sphere evaluations = "
            << 8 * (sphere_left_calls + sphere_right_calls)
            << " / " << 16 * points.size() << std::endl;
    }

    void BenchmarkNode(const char* name, const Node& node, const std::vector<Vector>& points)
    {
        constexpr int N = 10000000;
        constexpr int repetitions = 5;

        std::vector<double> times;
        times.reserve(repetitions);

        double checksum = 0.0;

        for (int r = 0; r < repetitions; ++r)
        {
            double sum = 0.0;

            auto start = std::chrono::steady_clock::now();

            for (int i = 0; i < N; ++i)
            {
                sum += node.Value(points[i % points.size()]);
            }

            auto end = std::chrono::steady_clock::now();

            std::chrono::duration<double, std::milli> time = end - start;

            times.push_back(time.count());
            checksum = sum;
        }

        std::sort(times.begin(), times.end());

        std::cout << name
            << ": median = " << times[2] << " ms"
            << ", min = " << times.front() << " ms"
            << ", max = " << times.back() << " ms"
            << ", checksum = " << checksum
            << std::endl;
    }

    void BenchmarkAcceleration()
    {
        const auto points = GeneratePoints();

        auto standard = CreateStandardUnion();
        auto accelerated_box = CreateAcceleratedUnion(false);
        auto accelerated_sphere = CreateAcceleratedUnion(true);

        BenchmarkNode("Union classique", *standard, points);
        BenchmarkNode("Acceleration Box", *accelerated_box, points);
        BenchmarkNode("Acceleration Sphere", *accelerated_sphere, points);
    }
}

void RunAccelerationBenchmarks()
{
	std::cout << "\nAcceleration Benchmarks" << std::endl;

    TestAcceleratedUnion();
    BenchmarkAcceleration();
}
