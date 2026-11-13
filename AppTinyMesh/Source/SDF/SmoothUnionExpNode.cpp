#include "SDF/SmoothUnionExpNode.h"

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <utility>

SmoothUnionExpNode::SmoothUnionExpNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right, double radius)
	: BinaryNode(std::move(left), std::move(right)), radius_(radius)
{
	if (radius <= 0.0)
		throw std::invalid_argument("SmoothUnionExpNode: radius must be positive");
}

/*
double SmoothUnionExpNode::Value(const Vector& p) const
{
	const double a = left_->Value(p);
	const double b = right_->Value(p);

	const double min_distance = std::min(a, b);

	// std::log1p computes log(1+x)
	const double correction = std::log1p( std::exp(-std::abs(a - b) / radius_) );

	return (min_distance / radius_ - correction) / (radius_ * std::log(2.0));
}
*/

double SmoothUnionExpNode::Value(const Vector& p) const
{
	const double a = left_->Value(p);
	const double b = right_->Value(p);

	// smin(a, b) = min(a, b) - r * ln(1 + exp(-|a - b| / r))
	// Numerically stable: the exponential argument is always <= 0.
	const double correction = std::log1p(std::exp(-std::abs(a - b) / radius_));

	return std::min(a, b) - radius_ * correction;
}

double SmoothUnionExpNode::Lipschitz() const { return std::max(left_->Lipschitz(), right_->Lipschitz()); }
