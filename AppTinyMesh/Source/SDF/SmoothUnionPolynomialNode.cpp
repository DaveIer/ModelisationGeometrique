#include "SDF/SmoothUnionPolynomialNode.h"

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <utility>

SmoothUnionPolynomialNode::SmoothUnionPolynomialNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right, double radius)
	: BinaryNode(std::move(left), std::move(right)), radius_(radius)
{
	if (radius <= 0.0)
		throw std::invalid_argument("SmoothUnionPolynomialNode: radius must be positive");
}

double SmoothUnionPolynomialNode::Value(const Vector& p) const
{
	const double a = left_->Value(p);
	const double b = right_->Value(p);

	const double h = std::max(1.0 - std::abs(a - b) / radius_, 0.0);
	
	return std::min(a, b) - (radius_ / 6.0) * h * h * h;
}

double SmoothUnionPolynomialNode::Lipschitz() const { return std::max(left_->Lipschitz(), right_->Lipschitz()); }
