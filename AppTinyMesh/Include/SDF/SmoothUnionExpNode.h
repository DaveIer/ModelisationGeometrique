#pragma once

#include "BinaryNode.h"

class SmoothUnionExpNode : public BinaryNode
{
public:
	SmoothUnionExpNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right, double radius);

	double Value(const Vector& p) const override;

private:
	double radius_; // blend radius
};
