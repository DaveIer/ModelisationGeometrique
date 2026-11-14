#pragma once

#include "BoundingVolumeNode.h"

#include <memory>

class AcceleratedUnionNode : public Node
{
public:
	AcceleratedUnionNode(std::unique_ptr<BoundingVolumeNode> left, std::unique_ptr<BoundingVolumeNode> right);

	double Value(const Vector& p) const override;
	double Lipschitz() const override;

private:
	std::unique_ptr<BoundingVolumeNode> left_;
	std::unique_ptr<BoundingVolumeNode> right_;
};
