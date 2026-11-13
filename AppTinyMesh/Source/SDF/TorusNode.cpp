#include "SDF/TorusNode.h"

TorusNode::TorusNode(double R, double r) : center_(0.0, 0.0, 0.0), R_(R), r_(r) {}

TorusNode::TorusNode(const Vector& center, double R, double r) : center_(center), R_(R), r_(r) {}

double TorusNode::Value(const Vector& p) const {
	const Vector& q = p - center_;
	const double dist_to_center = sqrt(q[0] * q[0] + q[1] * q[1]) - R_;

	return std::sqrt(dist_to_center * dist_to_center + q[2] * q[2]) - r_;
}

double TorusNode::Lipschitz() const { return 1.0; }
