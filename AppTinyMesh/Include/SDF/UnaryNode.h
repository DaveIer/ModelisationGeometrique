#pragma once

#include "Node.h"
#include <memory>

class UnaryNode : public Node
{
public:
	UnaryNode(std::unique_ptr<Node> child); // constructor

protected:
	std::unique_ptr<Node> child_;
};
