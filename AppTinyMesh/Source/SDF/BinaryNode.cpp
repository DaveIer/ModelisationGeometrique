#include "SDF/BinaryNode.h"
#include <utility>

BinaryNode::BinaryNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right) : left_(std::move(left)), right_(std::move(right)) {}