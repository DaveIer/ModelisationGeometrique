#pragma once

#include "Node.h"
#include <memory>

class BinaryNode : public Node
{
public:
	// constructor
	BinaryNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right);

protected:
	std::unique_ptr<Node> left_;
	std::unique_ptr<Node> right_;
};
