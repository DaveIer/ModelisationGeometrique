#include "SDF/UnaryNode.h"

#include <stdexcept>
#include <utility>

UnaryNode::UnaryNode(std::unique_ptr<Node> child) : child_(std::move(child))
{
	if (!child_) {
		throw std::invalid_argument("Child node cannot be null");
	}
}
