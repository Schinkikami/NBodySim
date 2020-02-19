#include <iostream>
#include "OctTree.h"

void OctNode::add(Nbody* par) {

	//string pref = "";
	//for (int i = 0; i < depth; i++) {
	//	pref += "\t";
	//}

	if (depth > max_depth) { 
		cout << "DepthProblem!" << endl;
		return; }

	//cout << pref << "Inserting Particle" << endl;

	//cout << pref << min << " " << max << endl;

	//if (!checkBoundary(par->position)) {
	//	std::cout << pref <<"Tried to add particle that doesnt fit in Node!" << endl;
	//}
	int childnum;
	if (!particle) {
		//cout << pref<<"Node was empty....";

		if (numChildParticles == 0) {
			//cout << "Just inserting!" << endl;
			particle = par;
		}
		else {
			childnum = getCorrespondingChild(par->position);
			//cout << "Inserting in child " << childnum << endl;
			if (children[childnum]) {
				(*children[childnum]).add(par);
			}
			else {
				pair<Vec3f, Vec3f> min_max = computeBoundingBox(childnum);
				if (depth + 1 > max_depth) {
					//cout << "Could not create child. Depth Prob" << endl;
					return;
				}
				children[childnum] = new OctNode(depth+1, max_depth, min_max.first, min_max.second);
				(*children[childnum]).add(par);
			}
		}

	}
	else {

		//cout << pref <<  "Leaf already has particle. ";
		// Have to resolve that
		// First just add the particle

		childnum = getCorrespondingChild(particle->position);
		//cout << particle->position << endl;
		//cout << "Adding into child " << childnum << " ";
		if (children[childnum]) {
			//cout << "that is already there" << endl;
			(*children[childnum]).add(particle);
		}
		else {
			//cout << "that we have to create!" << endl;
			if (depth + 1 > max_depth) {
				//cout << "Could not create child. Depth Prob" << endl;
				return;
			}
			pair<Vec3f, Vec3f> min_max = computeBoundingBox(childnum);
			//cout << min_max.first << "  " << min_max.second;
			children[childnum] = new OctNode(depth+1 ,max_depth ,min_max.first, min_max.second);
			children[childnum]->add(particle);
		}

		//cout << pref << "Now inserting new value ";
		//Now add your own particle in the subtree
		childnum = getCorrespondingChild(par->position);
		//cout << par->position << endl;
		//cout << "into " << childnum << " ";
		if (children[childnum]) {
			//cout << "that is already there!" << endl;
			children[childnum]->add(par);

		}
		else {
			//cout << "that we have to create!" << endl;
			pair<Vec3f, Vec3f> min_max = computeBoundingBox(childnum);
			children[childnum] = new OctNode(depth + 1, max_depth, min_max.first, min_max.second);
			children[childnum]->add(par);
		}
		//hasParticle = false;
		particle = NULL;
	}
	numChildParticles++;
}


int OctNode::getCorrespondingChild(Vec3f v) {
	int childNum = 0;
	
	Vec3f center = Vec3f((max + min) / 2);

	bool tx = center.x > v.x;
	bool ty = center.y > v.y;
	bool tz = center.z > v.z;

	childNum |= 1 * (int)tz;
	childNum |= 2 * (int)ty;
	childNum |= 4 * (int)tx;

	return childNum;

	
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
	Vec3f new_max = Vec3f();
	Vec3f new_min = Vec3f();

	Vec3f center = Vec3f((max+min)/2);

	bool tz = childnum & 1;
	bool ty = childnum & 2;
	bool tx = childnum & 4;

	if (tz) {
		new_min.z = min.z;
		new_max.z = center.z;
	}
	else {
		new_min.z = center.z;
		new_max.z = max.z;
	}

	if (ty) {
		new_min.y = min.y;
		new_max.y = center.y;
	}
	else {
		new_min.y = center.y;
		new_max.y = max.y;
	}

	if (tx) {
		new_min.x = min.x;
		new_max.x = center.x;
	}
	else {
		new_min.x = center.x;
		new_max.x = max.x;
	}


	return pair<Vec3f, Vec3f>(new_min, new_max);



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
	
	numChildParticles = 0;

	if (particle) {
		mass = particle->mass;
		centerOfMass = particle->position;
		numChildParticles = 1;
		return;
	}

	int tmp_num_children = 0;

	for (int i = 0; i < 8; i++) {
		if (children[i]) {
			tmp_num_children++;
			(*children[i]).updateTree();
			mass += (*children[i]).mass;
			centerOfMass += children[i]->mass * children[i]->centerOfMass;
			numChildParticles += children[i]->numChildParticles;
		}
	}

	//Leere Raumregion. Blatt des Baums.
	if (tmp_num_children == 0) {
		return;
	}

	centerOfMass /= mass;

}

int OctNode::updateParticle(int index, vector<Nbody*>* particles, vector<Vec3f>* acc, vector<int>* coll, float threshold){

	float cell_length = (max - min).length();
	Vec3f delta_dir = (*particles)[index]->position - centerOfMass;
	float dist = delta_dir.length();

	if (particle) {
		if (particle == (*particles)[index] || dist <= (particle->radius+(*particles)[index]->radius) ) {
			return 0;
		}

		(*acc)[index] -= (float)(mass / pow(dist, 3)) * delta_dir;
		return 0;
	}

	if (cell_length / dist < threshold) {
		(*acc)[index] -= (float)(mass / pow(dist, 3)) * delta_dir;
		return 0;
	}

	for (int i = 0; i < 8; i++) {
		if (children[i]) {
			children[i]->updateParticle(index, particles, acc, coll, threshold);
		}
	}
}

void OctNode::clear() {
	particle = NULL;
	numChildParticles = 0;
	mass = 0.f; centerOfMass = Vec3f(0.f, 0.f, 0.f);
	for (int i = 0; i < 8; i++) {
		delete children[i];
		children[i] = NULL;
	}

}


void OctNode::write(ofstream* file, int child_id) {
	if (depth == 0) {
		*file << "<!--XML Document-->\n";
		*file << "<?xml version='1.0' encoding='us-ascii'>\n";
	}

	string pref = "";
	for (int i = 0; i < depth; i++) {
		pref += "\t";
	}

	if (numChildParticles == 1) {
		*file << pref << "< Node " << "depth=\"" << depth << "\" id=\"" << child_id << "\" numPart=\"" << numChildParticles << "\" mass=\"" << mass << "\" center=\"(" << centerOfMass << ")\"/>\n";
	}
	else {
		*file << pref <<"< Node " << "depth=\"" << depth << "\" id=\"" << child_id << "\" numPart=\"" << numChildParticles << "\" mass=\"" << mass << "\" center=\"(" << centerOfMass << ")\">\n";

		for (int i = 0; i < 8; i++) {
			if (children[i]) {
				children[i]->write(file, i);
			}
		}

		*file << pref << "</Node>\n";
	}

}
void OctNode::print() {
	string pref = "";
	for (int i = 0; i < depth; i++) {
		pref += "\t";
	}

	if (particle) {
		cout << pref << (*particle).position << endl;
	}
	else {
		cout << " ---- " << endl;
	}

	for (int i = 0; i < 8; i++) {
		if (children[i]) {
			children[i]->print();
		} {
			cout << pref << "----" << endl;
		}
	}

}