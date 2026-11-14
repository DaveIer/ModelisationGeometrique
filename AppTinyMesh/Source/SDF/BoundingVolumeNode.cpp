#include "SDF/BoundingVolumeNode.h"

#include <utility>

BoundingVolumeNode::BoundingVolumeNode(std::unique_ptr<Node> child, const Vector& p_min, const Vector& p_max)
	: UnaryNode(std::move(child)), volume_(std::make_unique<BoxNode>(p_min, p_max)) {}

BoundingVolumeNode::BoundingVolumeNode(std::unique_ptr<Node> child, std::unique_ptr<Node> volume)
	: UnaryNode(std::move(child)), volume_(std::move(volume)) {}

double BoundingVolumeNode::Value(const Vector& p) const { return child_->Value(p); }

double BoundingVolumeNode::Lipschitz() const { return child_->Lipschitz(); }

double BoundingVolumeNode::LowerBound(const Vector& p) const { return volume_->Value(p); }
