#pragma once

#include "BinaryNode.h"

class IntersectionNode : public BinaryNode
{
public:
	// constructor
	IntersectionNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right);

	double Value(const Vector& p) const override;
};
