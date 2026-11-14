#include "SDF/AcceleratedUnionNode.h"

#include <algorithm>
#include <utility>

AcceleratedUnionNode::AcceleratedUnionNode(std::unique_ptr<BoundingVolumeNode> left, std::unique_ptr<BoundingVolumeNode> right)
	: left_(std::move(left)), right_(std::move(right)) {}

double AcceleratedUnionNode::Value(const Vector& p) const {
	const double left_bound = left_->LowerBound(p);
	const double right_bound = right_->LowerBound(p);

	// on évalue d'abord le sous arbre le moins couteux
	if (left_bound <= right_bound)
	{
		double d = left_->Value(p);
		if (right_bound < d)
			d = std::min(d, right_->Value(p));
		return d;
	}
	double d = right_->Value(p);
	if (left_bound < d)
		d = std::min(d, left_->Value(p));
	return d;
}

double AcceleratedUnionNode::Lipschitz() const
{
	return std::max(left_->Lipschitz(), right_->Lipschitz());
}
