#include "Scenes/ArchScene.h"

#include "SDF/BoxNode.h"
#include "SDF/CapsuleNode.h"
#include "SDF/UnionNode.h"
#include "SDF/IntersectionNode.h"
#include "SDF/DifferenceNode.h"
#include "SDF/RotationNode.h"

#include <memory>
#include <utility>

namespace
{
	// pilliers
	std::unique_ptr<Node> CreatePillars()
	{
		auto left_pillar = std::make_unique<BoxNode>(Vector(-1.25, -0.4, 0.0), Vector(-0.75, 0.4, 2.05));
		auto right_pillar = std::make_unique<BoxNode>(Vector(0.75, -0.4, 0.0), Vector(1.25, 0.4, 2.05));

		return std::make_unique<UnionNode>(std::move(left_pillar), std::move(right_pillar));
	}

	// voute
	std::unique_ptr<Node> CtreateVault()
	{
		// voute extérieure - capsule
		auto outer_capsule = std::make_unique<CapsuleNode>(Vector(0.0, -0.5, 2.0), Vector(0.0, 0.5, 2.0), 1.25);

		// on garde que la partie supérieure de la capsule en faisant une intersection avec une box
		auto upper_box = std::make_unique<BoxNode>(Vector(-1.5, -0.4, 2.0), Vector(1.5, 0.4, 3.5));
		auto upper_vault = std::make_unique<IntersectionNode>(std::move(outer_capsule), std::move(upper_box));

		// ouverture intérieure - différence avec une capsule de rayon inférieur
		auto inner_capsule = std::make_unique<CapsuleNode>(Vector(0.0, -0.5, 2.0), Vector(0.0, 0.5, 2.0), 0.75);
		
		return std::make_unique<DifferenceNode>(std::move(upper_vault), std::move(inner_capsule));
	}

	// base
	std::unique_ptr<Node> CreateBase()
	{
		// socle principal - box
		auto base = std::make_unique<BoxNode>(Vector(-1.55, -0.65, -0.25), Vector(1.55, 0.65, 0.05));

		// plateformes sous les piliers - deux box
		auto left_base = std::make_unique<BoxNode>(Vector(-1.40, -0.55, 0.03), Vector(-0.60, 0.55, 0.23));
		auto right_base = std::make_unique<BoxNode>(Vector(0.60, -0.55, 0.03), Vector(1.40, 0.55, 0.23));

		// union des deux plateformes
		auto pillar_bases = std::make_unique<UnionNode>(std::move(left_base), std::move(right_base));

		// union avec le socle principal
		return std::make_unique<UnionNode>(std::move(pillar_bases), std::move(base));
	}

	// joints sur la voute 
	std::unique_ptr<Node> CreateVaultJoints()
	{
		std::unique_ptr<Node> joints;
		
		const Vector center(0.0, 0.0, 2.0);
		const Vector axis(0.0, 1.0, 0.0);

		for (int angle : {-70, -45, -15, 15, 45, 70})
		{
			// le joint de référence (vertical)
			auto joint = std::make_unique<BoxNode>(Vector(-0.025, -0.55, 2.70), Vector(0.025, -0.35, 3.35));

			// rotation autour du centre de la voute
			auto rotated_joint = std::make_unique<RotationNode>(std::move(joint), axis, Math::DegreeToRadian(double(angle)), center);

			// ajouter les joint
			if (!joints)
				joints = std::move(rotated_joint);
			else
				joints = std::make_unique<UnionNode>(std::move(joints), std::move(rotated_joint));
		}

		return joints;
	}

	// joints des piliers
	std::unique_ptr<Node> CreatePillarJoints()
	{
		std::unique_ptr<Node> joints;

		for (double x : {-1.0, 1.0})
		{
			for (double z : {0.85, 1.45})
			{
				auto joint = std::make_unique<BoxNode>(
					Vector(x - 0.30, -0.50, z - 0.025),
					Vector(x + 0.30, -0.35, z + 0.025)
				);

				if (!joints)
					joints = std::move(joint);
				else
					joints = std::make_unique<UnionNode>(std::move(joints), std::move(joint));
			}
		}

		return joints;
	}

	// petits chapitaux au dessus des piliers
	std::unique_ptr<Node> CreateCapitals()
	{
		auto left_capital = std::make_unique<BoxNode>(Vector(-1.38, -0.52, 1.90), Vector(-0.62, 0.52, 2.03));
		auto right_capital = std::make_unique<BoxNode>(Vector(0.62, -0.52, 1.90), Vector(1.38, 0.52, 2.03));

		return std::make_unique<UnionNode>(std::move(left_capital), std::move(right_capital));
	}
}

std::unique_ptr<Node> CreateArchScene()
{
	auto pillars = CreatePillars();
	auto pillar_joints = CreatePillarJoints();
	
	auto vault = CtreateVault();
	auto joints = CreateVaultJoints();

	auto bases = CreateBase();
	
	auto stone_vault = std::make_unique<DifferenceNode>(std::move(vault), std::move(joints));
	auto stone_pillar = std::make_unique<DifferenceNode>(std::move(pillars), std::move(pillar_joints));

	auto capitals = CreateCapitals();
	auto pillar_deco = std::make_unique<UnionNode>(std::move(stone_pillar), std::move(capitals));

	auto arch = std::make_unique<UnionNode>(std::move(pillar_deco), std::move(stone_vault));

	return std::make_unique<UnionNode>(std::move(arch), std::move(bases));
}
