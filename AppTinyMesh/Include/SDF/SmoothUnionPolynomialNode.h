#pragma once

#include "BinaryNode.h"

class SmoothUnionPolynomialNode : public BinaryNode
{
public:
	SmoothUnionPolynomialNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right, double radius);

	double Value(const Vector& p) const override;

private:
	double radius_; // Blend radius
};
