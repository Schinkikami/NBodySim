#include "OctTree.h"
#include "Vec3.h"
#include "NBody.h"
#include <random>
#include <fstream>
#include <chrono>


int main(int argc, char** argv) {

	default_random_engine rndJesus(42);

	uniform_real_distribution<float> klaus(-100, 100);

	OctNode octtree(0, 14 , Vec3f(-100, -100, -100), Vec3f(100, 100, 100));


	int num_bods = 10000;
	vector<Nbody*> bodys;

	for (int i = 0; i < num_bods; i++) {
		bodys.push_back(new Nbody(Vec3f(klaus(rndJesus), klaus(rndJesus), klaus(rndJesus)), Vec3f(), 1));
	}
	cout << "Created particles!" << endl;

	auto start = chrono::high_resolution_clock::now();

	for (int round = 0; round < 60; round++) {
		octtree.clear();
		for (int i = 0; i < num_bods; i++) {
			octtree.add(bodys[i]);
		}

		octtree.updateTree();
		int num_colls = 0;
		vector<int>  collisons(num_bods, -1);
		vector<Vec3f> acc(num_bods, Vec3f(0,0,0));

		for (int i = 0; i < num_bods; i++) {
			acc[i] = Vec3f(0, 0, 0);
			num_colls += octtree.updateParticle(i, &bodys, &acc, &collisons, 1.f);
			acc[i] *= 1;
		}



		for (int i = 0; i < num_bods; i++) {
			bodys[i]->velocity += acc[i];
			bodys[i]->position += bodys[i]->velocity * 1;

			if (abs(bodys[i]->position.x) > 200 || abs(bodys[i]->position.y) > 200 || abs(bodys[i]->position.z) > 200) {
				bodys[i]->velocity -= 0.01f * bodys[i]->position;

			}


		}


		cout << "Done" << endl;

	}
	auto stop = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
	cout << duration.count()/60 << endl;
	//std::ofstream file;
	//file.open("./treewrite.xml");
	//octtree.write(&file, 0);
	//file.close();
	//octtree.print();

}