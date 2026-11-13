#include "SDF/CapsuleNode.h"

CapsuleNode::CapsuleNode(const Vector& a, const Vector& b, double r) : a_(a), b_(b), r_(r) {}

double CapsuleNode::Value(const Vector& p) const {
	const Vector& ab = b_ - a_;
	const Vector& ap = p - a_;

	const double length_squared = SquaredNorm(ab);

	// a and b are the same point
	if (length_squared == 0.0) {
		return Norm(ap) - r_;
	}

	// projection on the segment [a, b]
	const double t = Math::Clamp((ap * ab) / length_squared);
	const Vector closest_point = a_ + t * ab;

	return Norm(p - closest_point) - r_;
}

double CapsuleNode::Lipschitz() const { return 1.0; } // Exact SDF
