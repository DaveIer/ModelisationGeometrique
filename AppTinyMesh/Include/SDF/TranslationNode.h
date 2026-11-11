#pragma once

#include "UnaryNode.h"

class TranslationNode : public UnaryNode
{
public:
	TranslationNode(std::unique_ptr<Node> child, const Vector& translation);
	double Value(const Vector& p) const override;

private:
	Vector translation_;
};
