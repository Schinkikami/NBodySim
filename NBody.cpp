// NBodyCuda.cpp: Definiert den Einstiegspunkt für die Anwendung.
//

#define num_particles (int)2000
#define eps (float)0.1
#define CUBE_SIZE 200
#define MAX_DEPTH 100
#define SIM_ACC (float)1.

#include <GL/glew.h>      // openGL helper
#include <GL/glut.h>      // openGL helper

#include <random>
#include "NBody.h"
#include <time.h>
#include <algorithm>
#include "GLStuff.h"
#include "OctTree.h"
#include <mutex>

using namespace std;

int num_bodys = num_particles;
float max_velo;
vector<Nbody*> particles;
default_random_engine rndEngine(42);
OctNode* octtree;
OctNode* renderTree;

bool drawOctTree;
Vec3f max_field;
Vec3f min_field;

mutex renderTree_mutex;


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1024, 800);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("OpenGL NBodyView");


	initialize();
	setDefaults();
	glutMouseFunc(mousePressed);
	glutMotionFunc(mouseMoved);
	glutKeyboardFunc(keyPressed);
	glutKeyboardUpFunc(keyReleased);
	glutDisplayFunc(renderScene);
	glutReshapeFunc(reshape);
	glutTimerFunc(20, processTimedEvent, clock());
	glutMainLoop();
}

void initialize()
{
	// black screen
	glClearColor(0, 0, 0, 0);
	// initialize glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
		cout << "GLEW Error (" << err << "): " << glewGetErrorString(err) << endl;
	cout << "GLEW Version: " << glewGetString(GLEW_VERSION) << endl;
	// check if VBOs are supported
	if (glGenBuffers && glBindBuffer && glBufferData && glBufferSubData &&
		glMapBuffer && glUnmapBuffer && glDeleteBuffers && glGetBufferParameteriv)
		cout << "VBOs are supported!" << endl;
	else
		cout << "VBOs are NOT supported!" << endl;
	// set shading model
	glShadeModel(GL_SMOOTH);
	// set lighting (white light)
	GLfloat global_ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat ambientLight[] = { 1.f, 1.f, 1.f, 1.0f };
	GLfloat diffuseLight[] = { 0.f, 0.f, 0.f, 0.f };
	GLfloat specularLight[] = { 0.f, 0.f, 0.f, 0.f };
	GLfloat shininess = 0.8f * 128.0f;
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
	glLightf(GL_LIGHT0, GL_SHININESS, shininess);
	glEnable(GL_LIGHT0);
	// enable use of glColor instead of glMaterial
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void setDefaults()
{

	// scene Information
	cameraPos.set(0, 100, 0);
	cameraDir.set(0, -1, 0);
	movementSpeed = 0.02f;
	// light information
	lightPos.set(0.0f, 0.0f, -10.0f);
	lightMotionSpeed = 0.02f;
	moveLight = false;
	// mouse informations
	mouseX = 0;
	mouseY = 0;
	mouseButton = 0;
	mouseSensitivy = 1.0f;
	// key information, all false
	keyDown.resize(255, false);
	// draw mode (VBO)
	drawMode = 0;
	drawOctTree = false;
	simulate = false;
	sim_speed = 0.001f;
	num_bodys = num_particles;
	particles = vector<Nbody*>(num_particles);
	octtree = new OctNode(0, MAX_DEPTH, Vec3f(-CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE), Vec3f(CUBE_SIZE, CUBE_SIZE, CUBE_SIZE));
	renderTree = new OctNode(0, MAX_DEPTH, Vec3f(-CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE), Vec3f(CUBE_SIZE, CUBE_SIZE, CUBE_SIZE));

	Vec3f dist_scale(50, 50, 50);
	normal_distribution<float> nx(0.f, dist_scale.x);
	normal_distribution<float> ny(0.f, dist_scale.y);
	normal_distribution<float> nz(0.f, dist_scale.z);
	uniform_real_distribution<float> weigthdist(1., 1.);

	Vec3f g1_pos = Vec3f(75, 0, 0);
	Vec3f g2_pos = Vec3f(-75, 0, 0);

	for (int i = 0; i < num_bodys/2; i++) {
		float rads =uniform_real_distribution<float>(0, 2 * 3.14)(rndEngine);
		float rads2 = uniform_real_distribution<float>(0, 2 * 3.14)(rndEngine);
		float dist = abs(normal_distribution<float>(0, 20)(rndEngine));
		float z_height = normal_distribution<float>(0.f, 5.f)(rndEngine);

		particles[i] = new Nbody(  Vec3f(cos(rads) * dist + g1_pos.x, z_height + g1_pos.y, sin(rads) * dist + g1_pos.z), Vec3f(-sin(rads) * dist * 0.0015 * (num_bodys), 0,   cos(rads) * dist * 0.0015 * (num_bodys) +7), weigthdist(rndEngine));


		//particles[i] = Nbody(Vec3f(cos(rads2) * dist + sin(rads) * dist + g1_pos.x, sin(rads2) * dist + g1_pos.y, cos(rads) * dist + g1_pos.z), Vec3f(0,0,0), weigthdist(rndEngine));
		//particles[i] = new Nbody(Vec3f(cos(rads2) *dist + sin(rads) * dist + g1_pos.x, sin(rads2)*dist + g1_pos.y, cos(rads) * dist + g1_pos.z), Vec3f(-sin(rads2)* cos(rads) * dist *0.0 *(num_bodys), 0, sin(rads2) * -sin(rads) * dist * 0.0 *(num_bodys)), weigthdist(rndEngine));
		//float distance = Vec3f(particles[i].position.x, particles[i].position.z, 0.f).length();
		//float rads = std::asin(particles[i].position.x / distance);
		//particles[i].velocity.set(cos(rads)*distance*10, 0 , -sin(rads) * distance * 10);
	}

	for (int i = num_bodys/2; i < num_bodys; i++) {
		float rads = uniform_real_distribution<float>(0, 2 * 3.14)(rndEngine);
		float rads2 = uniform_real_distribution<float>(0, 2 * 3.14)(rndEngine);
		float dist = abs(normal_distribution<float>(0, 20)(rndEngine));
		float z_height = normal_distribution<float>(0.f, 5.f)(rndEngine);

		particles[i] = new Nbody(Vec3f(cos(rads) * dist + g2_pos.x, z_height + g2_pos.y, sin(rads) * dist + g2_pos.z), Vec3f(-sin(rads) * dist * 0.002 * (num_bodys), 0, cos(rads) * dist * 0.002 * (num_bodys) -7), weigthdist(rndEngine));

		//particles[i] = new Nbody(Vec3f(cos(rads2) * dist + sin(rads) * dist + g2_pos.x, sin(rads2) * dist + g2_pos.y, cos(rads) * dist + g2_pos.z), Vec3f(-sin(rads2) * cos(rads) * dist * 0.01 * (num_bodys), 0, sin(rads2) * -sin(rads) * dist * 0.01 * (num_bodys)), weigthdist(rndEngine));
		//float distance = Vec3f(particles[i].position.x, particles[i].position.z, 0.f).length();
		//float rads = std::asin(particles[i].position.x / distance);
		//particles[i].velocity.set(cos(rads)*distance*10, 0 , -sin(rads) * distance * 10);
	}

	//for (int i = num_bodys/2; i < num_bodys; i++) {
	//	float rads = (float)i / (num_bodys/2) * 2 * 3.14; //uniform_real_distribution<float>(0, 2 * 3.14)(rndEngine);
	//	float dist = uniform_real_distribution<float>(0, 25)(rndEngine);

		//particles[i] = Nbody(Vec3f(sin(rads) * dist + g2_pos.x, g2_pos.y, cos(rads) * dist + g2_pos.z), Vec3f(cos(rads) * dist* 0.1* (num_bodys / 2), 0, -sin(rads) * dist* 0.05 *(num_bodys / 2)), weigthdist(rndEngine));
		//float distance = Vec3f(particles[i].position.x, particles[i].position.z, 0.f).length();
		//float rads = std::asin(particles[i].position.x / distance);
		//particles[i].velocity.set(cos(rads)*distance*10, 0 , -sin(rads) * distance * 10);
	//}
	
	
	
	glutPostRedisplay();
}


void simStep(float secpassed) {

	for (int i = 0; i < num_bodys; i++) {
		max_field.x = max(max_field.x, particles[i]->position.x);
		min_field.x = min(min_field.x, particles[i]->position.x);

		max_field.y = max(max_field.y, particles[i]->position.y);
		min_field.y = min(min_field.y, particles[i]->position.y);

		max_field.z = max(max_field.z, particles[i]->position.z);
		min_field.z = min(min_field.z, particles[i]->position.z);
	}
	vector<Vec3f> acc(num_bodys);

	vector<int>  collisons(num_bodys, -1);

	int num_colls = 0;
	
	octtree->clear();

	octtree->max = Vec3f(max_field.x, max_field.y, max_field.z);
	octtree->min = Vec3f(min_field.x, min_field.y, min_field.z);

	for (int i = 0; i < num_bodys; i++) {
		octtree->add(particles[i]);
	}
	//octtree->print();
	octtree->updateTree();

	#pragma omp parallel for
	for (int i = 0; i < num_bodys; i++) {
		acc[i] = Vec3f(0, 0, 0);
		num_colls += octtree->updateParticle(i , &particles , &acc, &collisons, SIM_ACC);
		acc[i] *= G;
	}
	
	/*
	for (int i = 0; i < num_bodys; i++) {
		acc[i] = Vec3f(0, 0, 0);
		for (int j = 0; j < num_bodys; j++) {
			if (i == j) { continue; }
			
			Vec3f delta_dir = particles[j]->position - particles[i]->position;
			float dist = delta_dir.length();
			if (dist < particles[i]->radius || dist < particles[j]->radius) {
				#pragma omp atomic
				num_colls++;
				collisons[min(i,j)] = max(i,j);
			}
			else {
				//dist = std::max(dist, 1.f);
				acc[i] += (float)(particles[j]->mass / pow(dist, 3)) * delta_dir;
			}
		}
		acc[i] *= G;
	}

	if (num_colls > 0) {
		vector<int> to_rem(num_bodys, -1);
		int idx = 0;

		for (int i = 0; i < num_bodys; i++) {
			if (collisons[i] != -1) {
				to_rem[idx] = collisons[i];
				idx++;
				
				particles[i]->mass = particles[i]->mass + particles[collisons[i]]->mass;
				particles[i]->position = (particles[i]->position * (1 - (particles[collisons[i]]->mass / particles[i]->mass)) + particles[collisons[i]]->position * (particles[collisons[i]]->mass / particles[i]->mass));
				particles[i]->velocity = (particles[i]->velocity * (1 - (particles[collisons[i]]->mass / particles[i]->mass)) + particles[collisons[i]]->velocity * (particles[collisons[i]]->mass / particles[i]->mass));
				particles[i]->computeRadius();
			}
		}

		sort(to_rem.begin(), to_rem.end(), greater<int>());
		for(int rem_indx : to_rem) {
			if (rem_indx < 0) {
				continue;
			}
			delete particles[rem_indx];

			particles.erase(particles.begin() + rem_indx);
			num_bodys--;
		}

	}
	*/


	max_velo = 0.f;
	for (int i = 0; i < num_bodys; i++) {
		particles[i]->velocity += acc[i];
		particles[i]->position += particles[i]->velocity * secpassed;



		if (particles[i]->velocity.length() > max_velo) {
			max_velo = particles[i]->velocity.length();
		}
		if (abs(particles[i]->position.x) > 200 || abs(particles[i]->position.y) > 200 || abs(particles[i]->position.z) > 200) {
			delete particles[i];
			particles.erase(particles.begin() + i);
			num_bodys--;
			//particles[i]->velocity -= 0.01f * particles[i]->position;
			//float rads = uniform_real_distribution<float>(0, 2 * 3.14)(rndEngine);
			//float rads2 = uniform_real_distribution<float>(0, 2 * 3.14)(rndEngine);
			//float dist = uniform_real_distribution<float>(0, 100)(rndEngine);
			//particles[i] = Nbody(Vec3f(cos(rads2) * dist + sin(rads) * dist , sin(rads2) * dist, cos(rads) * dist), Vec3f(-sin(rads2) * cos(rads) * dist * 0.01 * (num_bodys), 0, sin(rads2) * -sin(rads) * dist * 0.01 * (num_bodys)), 1);

		}


	}

	renderTree_mutex.lock();
	delete renderTree;
	renderTree = octtree;
	octtree = new OctNode(0, MAX_DEPTH, Vec3f(), Vec3f());
	renderTree_mutex.unlock();

}

void processTimedEvent(int x)
{
	

	// Time now. int x is time of last run
	clock_t clock_this_run = clock();
	int msPassed = (clock_this_run - x) * 1000 / CLOCKS_PER_SEC;
	
	

	glutPostRedisplay();
	
	// perform light movement and trigger redisplays
	if (moveLight == true)
	{
		lightPos.rotY(lightMotionSpeed * msPassed);
		glutPostRedisplay();
	}
	// first person movement
	if (keyDown[(int)'w'] || keyDown[(int)'W'])
	{
		cameraPos += movementSpeed * cameraDir * (float)msPassed;
		glutPostRedisplay();
	}
	if (keyDown[(int)'s'] || keyDown[(int)'S'])
	{
		cameraPos -= movementSpeed * cameraDir * (float)msPassed;
		glutPostRedisplay();
	}
	if (keyDown[(int)'a'] || keyDown[(int)'A'])
	{
		Vec3f ortho(-cameraDir.z, 0.0f, cameraDir.x);
		ortho.normalize();
		cameraPos -= movementSpeed * ortho * (float)msPassed;
		glutPostRedisplay();
	}
	if (keyDown[(int)'d'] || keyDown[(int)'D'])
	{
		Vec3f ortho(-cameraDir.z, 0.0f, cameraDir.x);
		ortho.normalize();
		cameraPos += movementSpeed * ortho * (float)msPassed;
		glutPostRedisplay();
	}
	if (keyDown[(int)'q'] || keyDown[(int)'Q'])
	{
		cameraPos.y += movementSpeed * (float)msPassed;
		glutPostRedisplay();
	}
	if (keyDown[(int)'e'] || keyDown[(int)'E'])
	{
		cameraPos.y -= movementSpeed * (float)msPassed;
		glutPostRedisplay();
	}
	if (keyDown[(int)'o'] || keyDown[(int)'O'])
	{
		sim_speed *= 1.2;
		glutPostRedisplay();
	}
	if (keyDown[(int)'p'] || keyDown[(int)'P'])
	{
		sim_speed *= 0.8;
		glutPostRedisplay();
	}


	if (simulate) {
		simStep(sim_speed);
	}

	// start event again
	glutTimerFunc(std::max(1, 20 - msPassed), processTimedEvent, clock_this_run);
}


void reshape(GLint width, GLint height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(65.0, (float)width / (float)height, 0.5, 1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void drawCS()
{
	glBegin(GL_LINES);
	// red X
	glColor3f(1, 0, 0);
	glVertex3f(100, 0, 0);
	glVertex3f(-100, 0, 0);
	// green Y
	glColor3f(0, 1, 0);
	glVertex3f(0, 100, 0);
	glVertex3f(0, -100, 0);
	// blue Z
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 100);
	glVertex3f(0, 0, -100);
	glEnd();
}

void drawNode(OctNode* currentNode) {

	glBegin(GL_LINES);
	glColor3f(0, 1, 0);
	glVertex3f(currentNode->min.x, currentNode->min.y, currentNode->min.z);
	glVertex3f(currentNode->max.x, currentNode->min.y, currentNode->min.z);

	glVertex3f(currentNode->min.x, currentNode->min.y, currentNode->min.z);
	glVertex3f(currentNode->min.x, currentNode->max.y, currentNode->min.z);

	glVertex3f(currentNode->min.x, currentNode->min.y, currentNode->min.z);
	glVertex3f(currentNode->min.x, currentNode->min.y, currentNode->max.z);

	glVertex3f(currentNode->max.x, currentNode->min.y, currentNode->min.z);
	glVertex3f(currentNode->max.x, currentNode->max.y, currentNode->min.z);

	glVertex3f(currentNode->max.x, currentNode->min.y, currentNode->min.z);
	glVertex3f(currentNode->max.x, currentNode->min.y, currentNode->max.z);

	glVertex3f(currentNode->min.x, currentNode->max.y, currentNode->min.z);
	glVertex3f(currentNode->max.x, currentNode->max.y, currentNode->min.z);

	glVertex3f(currentNode->min.x, currentNode->max.y, currentNode->min.z);
	glVertex3f(currentNode->min.x, currentNode->max.y, currentNode->max.z);


	glVertex3f(currentNode->min.x, currentNode->min.y, currentNode->max.z);
	glVertex3f(currentNode->max.x, currentNode->min.y, currentNode->max.z);

	glVertex3f(currentNode->min.x, currentNode->min.y, currentNode->max.z);
	glVertex3f(currentNode->min.x, currentNode->max.y, currentNode->max.z);

	glVertex3f(currentNode->min.x, currentNode->max.y, currentNode->max.z);
	glVertex3f(currentNode->max.x, currentNode->max.y, currentNode->max.z);

	glVertex3f(currentNode->max.x, currentNode->min.y, currentNode->max.z);
	glVertex3f(currentNode->max.x, currentNode->max.y, currentNode->max.z);

	glVertex3f(currentNode->max.x, currentNode->max.y, currentNode->min.z);
	glVertex3f(currentNode->max.x, currentNode->max.y, currentNode->max.z);


	glEnd();

	for (int i = 0; i < 8; i++) {
		if (currentNode->children[i]) {
			drawNode(currentNode->children[i]);
		}
	}

}
void renderScene() {



	//drawOctTree = true;
	//simStep(sim_speed);
	// clear and set camera
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// sky box without lighting and without depth test
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	
	
	Vec3f cameraLookAt = cameraPos + cameraDir;
	gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z,          // Position
		cameraLookAt.x, cameraLookAt.y, cameraLookAt.z, // Lookat
		0.0, 1.0, 0.0);                                 // Up-direction
	drawCS();


	for (int i = 0; i < num_bodys; i++) {
		glPushMatrix();

		float cam_dist = (cameraPos - particles[i]->position).length();

		glTranslatef(particles[i]->position.x, particles[i]->position.y, particles[i]->position.z);


		glColor3f(particles[i]->velocity.length()/max_velo, 0, 1 - particles[i]->velocity.length()/max_velo);

		glutSolidSphere(particles[i]->radius, max(int(20.f/(cam_dist/5)) ,3), max(int(20.f / (cam_dist/5)), 3));
		glPopMatrix();
	}
	
	if (drawOctTree) {
		renderTree_mutex.lock();
		drawNode(renderTree);
		renderTree_mutex.unlock();
	}
	  // swap Buffers
	glutSwapBuffers();
}



void drawLight()
{
	GLfloat lp[] = { lightPos.x, lightPos.y, lightPos.z, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lp);
	// draw yellow sphere for light source
	glPushMatrix();
	glTranslatef(lp[0], lp[1], lp[2]);
	glColor3f(1, 1, 0);
	glutSolidSphere(2.0f, 16, 16);
	glPopMatrix();
}



void keyPressed(unsigned char key, int x, int y)
{
	keyDown[key] = true;
	switch (key)
	{
	case 27:
		exit(0);
		break;
	case '1':
		glShadeModel(GL_FLAT);
		glutPostRedisplay();
		break;
	case '2':
		glShadeModel(GL_SMOOTH);
		glutPostRedisplay();
		break;
	case 'h':
	case 'H':
		coutHelp();
		break;
		// reset view
	case 'r':
	case 'R':
		setDefaults();
		glutPostRedisplay();
		break;
		// light movement
	case 'l':
	case 'L':
		moveLight = !moveLight;
		break;
		// movement speed up
	case '+':
		movementSpeed *= 2.0f;
		break;
		// movement speed down
	case '-':
		movementSpeed /= 2.0f;
		break;
	case 'i':
		simulate = !simulate;
		glutPostRedisplay();
		break;
	case 'j':
		if (drawOctTree) {
			drawOctTree = false;
		}
		else {
			simulate = false;
			drawOctTree = true;
		}
		glutPostRedisplay();
		break;
	}
}

void keyReleased(unsigned char key, int x, int y)
{
	keyDown[key] = false;
}

void mousePressed(int button, int state, int x, int y)
{
	mouseButton = button;
	mouseX = x;
	mouseY = y;
}

void mouseMoved(int x, int y)
{
	// update angle with relative movement
	angleX = fmod(angleX + (x - mouseX) * mouseSensitivy, 360.0f);
	angleY -= (y - mouseY) * mouseSensitivy;
	angleY = max(-70.0f, min(angleY, 70.0f));
	// calculate camera direction (vector length 1)
	cameraDir.x = sin(angleX * M_RadToDeg) * cos(angleY * M_RadToDeg);
	cameraDir.z = -cos(angleX * M_RadToDeg) * cos(angleY * M_RadToDeg);
	cameraDir.y = max(0.0f, min(sqrtf(1.0f - cameraDir.x * cameraDir.x - cameraDir.z * cameraDir.z), 1.0f));
	if (angleY < 0)
		cameraDir.y = -cameraDir.y;
	// update mouse for next relative movement
	mouseX = x;
	mouseY = y;
	glutPostRedisplay();
}


void coutHelp()
{
	cout << endl;
	cout << "====== KEY BINDINGS ======" << endl;
	cout << "ESC: exit" << endl;
	cout << "H: show this (H)elp file" << endl;
	cout << "R: (R)eset to default values" << endl;
	cout << "L: toggle (L)ight movement" << endl;
	cout << endl;
	cout << "A,W,S,D: first person movement" << endl;
	cout << "+,-: movement speed up and down" << endl;
	cout << endl;
	cout << "1: GL Shader FLAT" << endl;
	cout << "2: GL Shader SMOOTH" << endl;
	cout << endl;
	cout << "M: Switch Draw (M)ode. 0: Immediate, 1: Array, 2: VBO" << endl;
	cout << "C: toggle use of (C)olor array" << endl;
	cout << "T: toggle use of (T)extures" << endl;
	cout << "B: toggle (B)ounding box" << endl;
	cout << "N: toggle drawing (N)ormals" << endl;
	cout << "==========================" << endl;
	cout << endl;
}