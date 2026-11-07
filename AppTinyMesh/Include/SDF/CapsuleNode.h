#pragma once

#include "Node.h"

class CapsuleNode : public Node {
public:
	CapsuleNode(const Vector& a, const Vector& b, double r); // constructor with two endpoints and radius
	CapsuleNode(const CapsuleNode& capsule) = default; // copy constructor
	
	~CapsuleNode() = default;
	
	// override the Value function to compute the signed distance from a point to the capsule
	double Value(const Vector& p) const override;

private:
	Vector a_; // first endpoint of the capsule
	Vector b_; // second endpoint of the capsule
	double r_;  // radius of the capsule
};
