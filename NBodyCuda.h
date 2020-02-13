// NBodyCuda.h: Includedatei für Include-Standardsystemdateien
// oder projektspezifische Includedateien.

#pragma once

#include <iostream>
#include <stdlib.h>       // namespace std
#include <GL/glew.h>      // openGL helper
#include <GL/glut.h>      // openGL helper
#include "Vec3.h"         // basic vector arithmetic class (embedded in std::)
#include <vector>

using namespace std;


struct Nbody {
	Vec3f position;
	Vec3f velocity;
	float mass;
	float radius;
	Nbody(Vec3f pos, Vec3f velo, float m) { position = pos; velocity = velo; mass = m; radius = 0.3 *sqrt(mass); };
	Nbody() { position = Vec3f(); velocity = Vec3f(); mass = 1.f;  radius = 0.3 *sqrt(mass);};
	void computeRadius() { radius = 0.3 * sqrt(mass); }
};
