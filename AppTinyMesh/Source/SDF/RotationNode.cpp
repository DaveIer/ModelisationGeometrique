#include "SDF/RotationNode.h"

#include <cmath>
#include <stdexcept>
#include <utility>

RotationNode::RotationNode(std::unique_ptr<Node> child, const Vector& axis, double angle, const Vector& center)
	: UnaryNode(std::move(child)), axis_(axis), center_(center), cos_angle_(std::cos(angle)), sin_angle_(std::sin(angle))
{
	const double length = Norm(axis_);

	if (!(length > 0.0) || !std::isfinite(length) || !std::isfinite(angle))
		throw std::invalid_argument("Invalid axis or angle for rotation");

	axis_ /= length; // normalize the axis
}

double RotationNode::Value(const Vector& p) const
{
	const Vector q = p - center_;

	// Rodrigues' rotation formula
	// * : cross product , / : vectorial product
	const Vector rotated = 
			q * cos_angle_ 
			- (axis_ / q) * sin_angle_ 
		+ axis_ * (axis_ * q) * (1.0 - cos_angle_);

	return child_->Value(rotated + center_);
}

double RotationNode::Lipschitz() const { return child_->Lipschitz(); }
 