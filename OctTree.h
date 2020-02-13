#pragma once

#include <iostream>
#include "NBody.h"


class OctNode {
public:

	OctNode(OctNode* par, Vec3f mx, Vec3f mn) { parent = par; max = mx; min = mn; particle = nullptr;
	hasParticle = false; numChildParticles = 0;
	mass = 0.f; centerOfMass = Vec3f(0.f, 0.f, 0.f);
	for (int i = 0; i < 8; i++) {
		children[i] = nullptr;
	}
	
	};

	bool checkBoundary(Vec3f p);
	void add(Nbody p);
	int getCorrespondingChild(Vec3f);
	void updateTree();
	pair<Vec3f, Vec3f> computeBoundingBox(int childnum);
	Vec3f min;
	Vec3f max;
	Nbody* particle;
	bool hasParticle;
	OctNode* children[8]; //Order is xyz. So lll, rll, lrl, rrl, llr, rlr, lrr, rrr  
	OctNode* parent;
	int numChildParticles;
	float mass;
	Vec3f centerOfMass;
};

class Octtree{

public:
	OctNode root;
	void add(Nbody p) { root.add(p); };
	void updateTree() { root.updateTree(); };

};


