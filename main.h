
float G = 1;

// camera Information
Vec3<float> cameraPos;
Vec3<float> cameraDir;
float angleX, angleY, movementSpeed;
// light information
Vec3f lightPos;
float lightMotionSpeed;
bool moveLight;
// mouse information
int mouseX, mouseY, mouseButton;
float mouseSensitivy;
// key pressed information
vector<bool> keyDown;
// draw mode
unsigned int drawMode;
float sim_speed;
bool simulate;

// ==============
// === BASICS ===
// ==============



int main(int argc, char** argv);

void initialize();

void setDefaults();

void reshape(GLint width, GLint height);

void processTimedEvent(int x);

void renderScene();

void coutHelp();
void mousePressed(int button, int state, int x, int y);
void mouseMoved(int x, int y);
void keyReleased(unsigned char key, int x, int y);
void keyPressed(unsigned char key, int x, int y);
void drawLight();
