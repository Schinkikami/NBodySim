#pragma once


#include <iostream>
#include "Octtree.h"

void OctNode::add(Nbody p) {

	if (!checkBoundary(p.position)) {
		std::cout << "Tried to add particle that doesnt fit in Node!";
	}

	if (!hasParticle) {
		if (numChildParticles == 0) {
			particle = &p;
			hasParticle = true;
		}
		else {
			int childnum = getCorrespondingChild(p.position);
			if (&children[childnum]) {
				(*children[childnum]).add(p);
			}
			else {
				pair<Vec3f, Vec3f> min_max = computeBoundingBox(childnum);
				children[childnum] = new OctNode(this, min_max.first, min_max.second);
				(*children[childnum]).add(p);
			}
		}

	}
	else {
		// Have to resolve that
		// First just add the particle

		int childnum = getCorrespondingChild(p.position);
		if (&children[childnum]) {
			(*children[childnum]).add(p);
		}
		else {
			pair<Vec3f, Vec3f> min_max = computeBoundingBox(childnum);
			children[childnum] = new OctNode(this, min_max.first, min_max.second);
			(*children[childnum]).add(p);
		}

		//Now add your own particle in the subtree
		int childnum = getCorrespondingChild((*particle).position);
		if (&children[childnum]) {
			(*children[childnum]).add((*particle));

		}
		else {
			pair<Vec3f, Vec3f> min_max = computeBoundingBox(childnum);
			children[childnum] = new OctNode(this, min_max.first, min_max.second);
			(*children[childnum]).add((*particle));
		}
		hasParticle = false;
	}
	numChildParticles++;
}


int OctNode::getCorrespondingChild(Vec3f v) {
	bool tx = (max.x - min.x)/2 > v.x - min.x;
	bool ty = (max.y - min.y)/2 > v.y - min.y;
	bool tz = (max.y - min.y)/2 > v.y - min.y;

	if (tx) {
		if (ty) {
			if (tz) {
				return 0;
			}
			else {
				return 1;
			}
		}
		else {
			if (tz) {
				return 2;
			}
			else {
				return 3;
			}
		}
	}
	else {
		if (ty) {
			if (tz) {
				return 4;
			}
			else {
				return 5;
			}
		}
		else {
			if (tz) {
				return 6;
			}
			else {
				return 7;
			}
		}
	}

}
bool OctNode::checkBoundary(Vec3f p) {

	

	if (p.x < min.x || p.x >= max.x) {
		return false;
	}
	if (p.y < min.y || p.y >= max.y) {
		return false;
	}
	if (p.z < min.z || p.z >= max.z) {
		return false;
	}

	return true;
}

pair<Vec3f, Vec3f> OctNode::computeBoundingBox(int childnum) {
	Vec3f new_max = Vec3f(max);
	Vec3f new_min = Vec3f(min);

	if (childnum / 4 == 0) {
		new_max.x -= (max.x - min.x) / 2;
	}
	else {
		childnum %= 4;
		new_min.x += (max.x - min.x) / 2;
	}

	childnum %= 4;

	if (childnum / 2 == 0) {
		new_max.y -= (max.y - min.y) / 2;
	}
	else {
		new_min.y += (max.y - min.y) / 2;
	}

	childnum %= 2;

	if (childnum / 1 == 0) {
		new_max.z -= (max.z - min.z) / 2;
	}
	else {
		new_min.z += (max.z - min.z) / 2;
	}
	
	return pair<Vec3f, Vec3f>(new_min, new_max);

}

//Update the center of mass and mass of the cell as well as the bounding box.
void OctNode::updateTree() {
	

	if (hasParticle) {
		mass = (*particle).mass;
		centerOfMass = (*particle).position;
		numChildParticles = 0;
		return;
	}

	int tmp_num_children = 0;

	for (int i = 0; i < 7; i++) {
		if (&children[i]) {
			tmp_num_children++;
			(*children[i]).updateTree();
			mass += (*children[i]).mass;
			centerOfMass += children[i].mass * children[i].centerOfMass;
			numChildParticles += children[i].numChildParticles;
		}
	}

	//Leere Raumregion. Blatt des Baums.
	if (tmp_num_children == 0) {
		return;
	}

	centerOfMass /= mass;

}
