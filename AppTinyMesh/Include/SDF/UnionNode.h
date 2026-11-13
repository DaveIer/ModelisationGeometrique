#pragma once

#include "BinaryNode.h"

class UnionNode : public BinaryNode
{
public:
	UnionNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right);

	double Value(const Vector& p) const override;
	double Lipschitz() const override;
};
