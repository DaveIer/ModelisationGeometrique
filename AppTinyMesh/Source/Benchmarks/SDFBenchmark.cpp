#include "Benchmarks/SDFBenchmark.h"

#include "SDF/Node.h"
#include "SDF/SphereNode.h"
#include "SDF/BoxNode.h"
#include "SDF/CapsuleNode.h"
#include "SDF/TorusNode.h"

#include "SDF/UnionNode.h"
#include "SDF/IntersectionNode.h"
#include "SDF/DifferenceNode.h"
#include "SDF/SmoothUnionPolynomialNode.h"
#include "SDF/SmoothUnionExpNode.h"

#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <memory>

namespace
{
	constexpr int N = 10000000;

	// fonction qui génère des points
	std::vector<Vector> GeneratePoints()
	{
		std::vector<Vector> points;
		points.reserve(4096);

		const double step = 3.0 / 15.0;

		for (int i = 0; i < 16; ++i)
			for (int j = 0; j < 16; ++j)
				for (int k = 0; k < 16; ++k)
					points.emplace_back(-1.5 + i * step, -1.5 + j * step, -1.5 + k * step);
		
		return points;
	}

	// fonction qui mesure le temps de N appels pour un node donné
	void BenchmarkValue(const char* name, const Node& node, const std::vector<Vector>& points)
	{
		constexpr int repetitions = 5;
		std::vector<double> times;
		times.reserve(repetitions);

		double checksum = 0.0;

		for (int r = 0; r < repetitions; ++r)
		{
			double sum = 0.0;

			auto start = std::chrono::steady_clock::now();
			for (int i = 0; i < N; ++i)
				sum += node.Value(points[i % points.size()]);
			auto end = std::chrono::steady_clock::now();

			std::chrono::duration<double> time = end - start;

			std::cout << name << ": " << time.count() * 1000.0 << " ms, checksum = " << sum << std::endl;

			times.push_back(time.count() * 1000.0);
			checksum += sum;
		}

		std::sort(times.begin(), times.end());

		std::cout << name
			<< ": median = "	<< times[2]		 << "ms"
			<< " - min = "		<< times.front() << "ms"
			<< " - max = "		<< times.back()  << "ms"
			<< " - checksum = " << checksum
			<< std::endl;
	}
}

void RunSDFBenchmarks()
{
	std::cout << "SDF Benchmarks" << std::endl;

	auto points = GeneratePoints();

	/* --- Primitives --- */
	SphereNode sphere(Vector(0.0, 0.0, 0.0), 1.0);
	BoxNode box(Vector(-1.0, -1.0, -1.0), Vector(1.0, 1.0, 1.0));
	CapsuleNode capsule(Vector(-1.0, 0.0, 0.0), Vector(1.0, 0.0, 0.0), 0.5);
	TorusNode torus(Vector(0.0, 0.0, 0.0), 1.0, 0.35);

	/* --- Opérations Booléennes --- */
	UnionNode unionNode(
		std::make_unique<SphereNode>(Vector(0.0, 0.0, 0.0), 1.0),
		std::make_unique<BoxNode>(Vector(0.0, -0.55, -0.55), Vector(1.6, 0.55, 0.55))
	);

	IntersectionNode interNode(
		std::make_unique<SphereNode>(Vector(0.0, 0.0, 0.0), 1.0),
		std::make_unique<BoxNode>(Vector(0.0, -0.55, -0.55), Vector(1.6, 0.55, 0.55))
	);

	DifferenceNode diffNode(
		std::make_unique<SphereNode>(Vector(0.0, 0.0, 0.0), 1.0),
		std::make_unique<BoxNode>(Vector(0.0, -0.55, -0.55), Vector(1.6, 0.55, 0.55))
	);

	SmoothUnionPolynomialNode smoothPoly(
		std::make_unique<SphereNode>(Vector(0.0, 0.0, 0.0), 1.0),
		std::make_unique<BoxNode>(Vector(0.0, -0.55, -0.55), Vector(1.6, 0.55, 0.55)),
		0.8
	);

	SmoothUnionExpNode smoothExp(
		std::make_unique<SphereNode>(Vector(0.0, 0.0, 0.0), 1.0),
		std::make_unique<BoxNode>(Vector(0.0, -0.55, -0.55), Vector(1.6, 0.55, 0.55)),
		0.8
	);

	/* --- Benchmarks --- */
	BenchmarkValue("Sphere",		  sphere, points);
	BenchmarkValue("Box",				 box, points);
	BenchmarkValue("Capsule",		 capsule, points);
	BenchmarkValue("Torus",			   torus, points);
	BenchmarkValue("Union",		   unionNode, points);
	BenchmarkValue("Intersection", interNode, points);
	BenchmarkValue("Difference",	diffNode, points);

	BenchmarkValue("Smooth Polynomial", smoothPoly, points);
	BenchmarkValue("Smooth Exponential", smoothExp, points);
}
