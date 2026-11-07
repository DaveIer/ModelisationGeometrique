#include "SDF/DifferenceNode.h"

DifferenceNode::DifferenceNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right) : BinaryNode(std::move(left), std::move(right)) {}

double DifferenceNode::Value(const Vector& p) const {
	return std::max(left_->Value(p), -right_->Value(p));
}