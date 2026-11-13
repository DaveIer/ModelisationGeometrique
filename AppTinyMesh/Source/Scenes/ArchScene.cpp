#include "Scenes/ArchScene.h"

#include "SDF/BoxNode.h"
#include "SDF/CapsuleNode.h"
#include "SDF/UnionNode.h"
#include "SDF/IntersectionNode.h"
#include "SDF/DifferenceNode.h"

#include <memory>
#include <utility>

std::unique_ptr<Node> CreateArchScene()
{
	// piliers
	auto left_pillar = std::make_unique<BoxNode>  (Vector(-1.25, -0.4, 0.0), Vector(-0.75, 0.4, 2.05));
	auto right_pillar = std::make_unique<BoxNode> (Vector(0.75, -0.4, 0.0), Vector(1.25, 0.4, 2.05));

	auto pillars = std::make_unique<UnionNode>(std::move(left_pillar), std::move(right_pillar));

	// voute extérieure
	auto outer_capsule = std::make_unique<CapsuleNode>(Vector(0.0, -0.5, 2.0), Vector(0.0, 0.5, 2.0), 1.25);

	// on garde que la partie supérieure de la capsule et on limite la profondeur
	auto upper_box = std::make_unique<BoxNode>(Vector(-1.5, -0.4, 2.0), Vector(1.5, 0.4, 3.5));
	auto upper_vault = std::make_unique<IntersectionNode>(std::move(outer_capsule), std::move(upper_box));

	// ouverture intérieure
	auto inner_capsule = std::make_unique<CapsuleNode>(Vector(0.0, -0.5, 2.0), Vector(0.0, 0.5, 2.0), 0.75);
	auto vault = std::make_unique<DifferenceNode>(std::move(upper_vault), std::move(inner_capsule));

	// structure principale
	auto arch = std::make_unique<UnionNode>(std::move(pillars), std::move(vault));

	// socle principal
	auto base = std::make_unique<BoxNode>(Vector(-1.55, -0.65, -0.25), Vector(1.55, 0.65, 0.05));

	// plateformes sous les piliers
	auto left_base = std::make_unique<BoxNode> (Vector(-1.40, -0.55, 0.03), Vector(-0.60, 0.55, 0.23));
	auto right_base = std::make_unique<BoxNode> (Vector( 0.60, -0.55, 0.03), Vector( 1.40, 0.55, 0.23));

	// union des deux plateformes
	auto pillar_bases = std::make_unique<UnionNode>(std::move(left_base), std::move(right_base));

	// union arche + plateformes
	auto arch_with_bases = std::make_unique<UnionNode>(std::move(arch), std::move(pillar_bases));

	// arche + base
	return std::make_unique<UnionNode>(std::move(arch_with_bases), std::move(base));
}
