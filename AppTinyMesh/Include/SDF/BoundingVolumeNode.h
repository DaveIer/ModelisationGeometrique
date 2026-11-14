#pragma once

#include "UnaryNode.h"
#include "BoxNode.h"

class BoundingVolumeNode : public UnaryNode
{
public:
	BoundingVolumeNode(std::unique_ptr<Node> child, const Vector& p_min, const Vector& p_max);
	BoundingVolumeNode(std::unique_ptr<Node> child, std::unique_ptr<Node> volume);

	double Value(const Vector& p) const override;
	double Lipschitz() const override;

	double LowerBound(const Vector& p) const;

private:
	std::unique_ptr<Node> volume_;
};
