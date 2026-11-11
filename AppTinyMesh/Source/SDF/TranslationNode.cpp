#include "SDF/TranslationNode.h"

#include <utility>

TranslationNode::TranslationNode(std::unique_ptr<Node> child, const Vector& translation)
    : UnaryNode(std::move(child)), translation_(translation)
{}

double TranslationNode::Value(const Vector& p) const
{
	return child_->Value(p - translation_);
}
