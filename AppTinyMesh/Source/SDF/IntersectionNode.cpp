#include "SDF/IntersectionNode.h"

IntersectionNode::IntersectionNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right) : BinaryNode(std::move(left), std::move(right)) {}

double IntersectionNode::Value(const Vector& p) const {
	return std::max(left_->Value(p), right_->Value(p));
}