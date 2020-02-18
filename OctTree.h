#pragma once

#include <iostream>
#include "NBody.h"
#include <fstream>

class OctNode {
public:

	OctNode(int d, int md , Vec3f mn, Vec3f mx) {
		depth = d; max_depth = md; max = mx; min = mn; particle = NULL;
	//hasParticle = false;
	numChildParticles = 0;
	mass = 0.f; centerOfMass = Vec3f(0.f, 0.f, 0.f);
	for (int i = 0; i < 8; i++) {
		children[i] = NULL;
	}
	};
	~OctNode() {
		for (int i = 0; i < 8; i++) {
			if (children[i] != 0) {
				delete children[i];
				children[i] = NULL;
			}
		}

		particle = NULL;
	};
	bool checkBoundary(Vec3f par);
	void add(Nbody* par);
	int getCorrespondingChild(Vec3f);
	void updateTree();
	void print();
	void write(std::ofstream* file, int child_id);
	void clear();
	int max_depth;
	int updateParticle(int index, vector<Nbody*>* particles ,vector<Vec3f>* acc, vector<int>* coll, float threshold);
	pair<Vec3f, Vec3f> computeBoundingBox(int childnum);
	int depth;
	Vec3f min;
	Vec3f max;
	Nbody* particle;
	//bool hasParticle;
	OctNode* children[8]; //Order is xyz. So lll, rll, lrl, rrl, llr, rlr, lrr, rrr  
	//OctNode* parent;
	int numChildParticles;
	float mass;
	Vec3f centerOfMass;
};

class Octtree{

public:
	OctNode root;
	void add(Nbody* p) { root.add(p); };
	void updateTree() { root.updateTree(); };

};


