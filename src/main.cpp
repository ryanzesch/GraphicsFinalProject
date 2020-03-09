/*
 * Program 3 base code - includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "Multishape.h"
#include "Sentry.h"
#include "Card.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"
#include "stb_image.h"
#include <random>
#include <algorithm>
#include "TextRenderer.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

// Bookkeeping of out obj's
const char *obj_files[] = {"/sphere.obj", "/dummy.obj", "/bunnyNoNorm.obj", "/sentry.obj", "/laser.obj", "/floor.obj", "/pillar.obj", "/dog.obj", 
	"/card.obj", "/cube.obj", "/myfurnace.obj", "/furnacelogs.obj", "/tile.obj", "/sph_float.obj", "/semisph_float.obj", "/pyr_float.obj", "/icosa_float.obj"};
enum obj_idx {sphere, dummy, bunnyNoNorm, sentry, laser, floormod, wall, dog, strike, cube, furnace, logs, floortile, sph, semsph, pyr, icosa};
enum sentry_shapes {sen_bot, sen_top, sen_mid};

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Text renderer
	shared_ptr<TextRenderer> textRenderer;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> card_prog;
	std::shared_ptr<Program> cube_prog;
	std::shared_ptr<Program> floor_prog;
	std::shared_ptr<Program> text_prog;
	std::shared_ptr<Program> slash_prog;

	// Shape to be used (from  file) - modify to support multiple
	shared_ptr<Shape> mesh;
	shared_ptr<Multishape> multishape;
	vector<shared_ptr<Multishape>> meshes;

	// Textures
	shared_ptr<Texture> texture0;
	shared_ptr<Texture> texture1;
	shared_ptr<Texture> texture2;
	shared_ptr<Texture> texture3;
	shared_ptr<Texture> slash1;
	shared_ptr<Texture> slash2;
	shared_ptr<Texture> slash3;
	shared_ptr<Texture> slash4;
	shared_ptr<Texture> slash5;
	shared_ptr<Texture> slash6;

	// Skybox
	vector<std::string> faces {
		"vc_rt.tga",
		"vc_lf.tga",
		"vc_up.tga",
		"vc_dn.tga",
		"vc_ft.tga",
		"vc_bk.tga"
	}; 

	// Window size
	int win_w = 1280;
	int win_h = 720;

	// Lights
	vec3 furnacelight = vec3(-7.728,.52,-4.446);
	vec3 furnacecol = vec3(1,.8,.7);

	// View Matrix Stuff + Camera Vars
	vec3 pos = vec3(0,.6,5);
	vec3 lookat;
	vec3 upvec = vec3(0,1,0);
	vec3 viewdir;
	double mouse_x=win_w/2, mouse_y=win_h/2;
	double phi=0, theta=3*3.14/2;

	// Camera movement speed and fov
	float speed = .03f, fov = 45.0f ;

	// A vector to keep track of our bad guys, duh
	vector<shared_ptr<Sentry>> sentries;
	shared_ptr<Sentry> mysentry;

	// Keeping track of cards
	int selected_card = 0;
	shared_ptr<Card> mycard;
	shared_ptr<Card> mycard2;
	vector<shared_ptr<Card>> hand;
	vector<shared_ptr<Card>> drawpile;
	vector<shared_ptr<Card>> discard;
	vector<shared_ptr<Card>> thrown_cards;
	vector<shared_ptr<Card>> stuck_cards;
	vector<shared_ptr<Card>> slashing_cards;
	enum handstate {
		HAND_READY,
		HAND_THROWING
	};
	int hand_state = HAND_READY;
	float throw_start = 0;
	float throw_duration = .5;
	bool has_activated_card = false;

	// Player data
	int block = 0;
	int health = 75;
	int energy = 3;
	int refresh = 5;
	float refreshtime = 5;

	// Sentry shooting data
	float lastshot = 0;
	float shootdelay = 10;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//animation data
	float sTheta = 0;
	float eTheta = 0;
	float hTheta = 0;
	float gTheta = 0;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
		if (key == GLFW_KEY_E && action == GLFW_RELEASE) {
			if (hand.size() > 0 && hand_state == HAND_READY) {
				if (selected_card == 0) {
					selected_card = hand.size() -1;
				}
				else {
					selected_card = (selected_card - 1) % hand.size();
				}
			}
		}
		if (key == GLFW_KEY_Q && action == GLFW_RELEASE) {
			if (hand.size() > 0 && hand_state == HAND_READY) {
				selected_card = (selected_card + 1) % hand.size();
			}
		}
		// Throw cards
		if (key == GLFW_KEY_F && action == GLFW_RELEASE && hand.size() > 0 && throw_start + throw_duration < glfwGetTime() && energy > 0) {
			// Throwing bookkeeping
			throw_start = glfwGetTime();
			hand_state = HAND_THROWING;
			has_activated_card = false;
			energy -= 1;
		}
		// Draw a new hand
		if (key == GLFW_KEY_R && refreshtime < glfwGetTime()) {
			refreshtime = glfwGetTime() + 5;
			energy = 3;
			while(hand.size() > 0) {
				mycard = hand[0];
				hand.erase(hand.begin() );
				discard.push_back(mycard);
			}
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			 glfwGetCursorPos(window, &posX, &posY);
			 cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
			 cout << "World pos: x: " << pos.x <<" y: " << pos.y << " z: " << pos.z << endl;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
		win_w = width;
		win_h = height;
	}

	void SetMaterial(int i) {
		switch (i) {
		case 0: //shiny blue plastic
		glUniform3f(prog->getUniform("MatAmb"), 0.02, 0.04, 0.2);
		glUniform3f(prog->getUniform("MatDif"), 0.0, 0.16, 0.9);
		glUniform3f(prog->getUniform("MatSpec"), 0.14, 0.2, 0.8);
		glUniform1f(prog->getUniform("shine"), 120.0);
		break;
		case 1: // flat grey
		glUniform3f(prog->getUniform("MatAmb"), 0.13, 0.13, 0.14);
		glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.4);
		glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.3, 0.4);
		glUniform1f(prog->getUniform("shine"), 4.0);
		break;
		case 2: //brass
		glUniform3f(prog->getUniform("MatAmb"), 0.3294, 0.2235, 0.02745);
		glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);
		glUniform3f(prog->getUniform("MatSpec"), 0.9922, 0.941176, 0.80784);
		glUniform1f(prog->getUniform("shine"), 27.9);
		break; 
		case 3: //red
		glUniform3f(prog->getUniform("MatAmb"), 0.07294 * 5, 0.002235 * 5, 0.007745 * 5);
		glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.05686, 0.0711373);
		glUniform3f(prog->getUniform("MatSpec"), 0.09922, 0.0941176, 0.080784);
		glUniform1f(prog->getUniform("shine"), 27.9);
		break;
		case 4: //green
		glUniform3f(prog->getUniform("MatAmb"), 0.07294, 0.72235, 0.07745);
		glUniform3f(prog->getUniform("MatDif"), 0.07804, 0.75686, 0.0711373);
		glUniform3f(prog->getUniform("MatSpec"), 0.9922, 0.941176, 0.80784);
		glUniform1f(prog->getUniform("shine"), 27.9);
		break;
		case 5: //sandstone for sentry
		glUniform3f(prog->getUniform("MatAmb"), 0.19125, 0.0735, 0.0225);
		glUniform3f(prog->getUniform("MatDif"), 0.7038, 0.27048, 0.0828);
		glUniform3f(prog->getUniform("MatSpec"), 0.256777, 0.137622, 0.086014);
		glUniform1f(prog->getUniform("shine"), 12.8);
		break;
		case 6: //blue for sentry
		glUniform3f(prog->getUniform("MatAmb"), 0.2f,0.2f,0.7f);
		glUniform3f(prog->getUniform("MatDif"), 0.4f,0.3f,0.5f);
		glUniform3f(prog->getUniform("MatSpec"),0.04f,0.3f,0.7f);
		glUniform1f(prog->getUniform("shine"), 120);
		break;
		case 7: // dark grey
		glUniform3f(prog->getUniform("MatAmb"), 0.07, 0.07, 0.075);
		glUniform3f(prog->getUniform("MatDif"), 0.2, 0.2, 0.2);
		glUniform3f(prog->getUniform("MatSpec"), 0.2, 0.2, 0.2);
		glUniform1f(prog->getUniform("shine"), 1.0);
		break;
		case 8: // furnace grey
		glUniform3f(prog->getUniform("MatAmb"), 0.04, 0.04, 0.05);
		glUniform3f(prog->getUniform("MatDif"), 0.1, 0.1, 0.1);
		glUniform3f(prog->getUniform("MatSpec"), 0.2, 0.2, 0.2);
		glUniform1f(prog->getUniform("shine"), 1.0);
		break;
		case 9: // firewood
		glUniform3f(prog->getUniform("MatAmb"), 0.66*.3, 0.53*.3, 0.36*.3);
		glUniform3f(prog->getUniform("MatDif"), 0.66*.3, 0.53*.3, 0.36*.3);
		glUniform3f(prog->getUniform("MatSpec"), 0.66*.3, 0.53*.3, 0.36*.3);
		glUniform1f(prog->getUniform("shine"), 1.0);
		break;
		case 10: // furnace grey
		glUniform3f(prog->getUniform("MatAmb"), 0.07, 0.075, 0.075);
		glUniform3f(prog->getUniform("MatDif"), 0.2, 0.25, 0.2);
		glUniform3f(prog->getUniform("MatSpec"), 0.2, 0.25, 0.2);
		glUniform1f(prog->getUniform("shine"), 1.0);
		break;
		}
	}

	void SetCardTex(int i) {
		switch (i) {
		case 0: //Strike
		texture0->bind(card_prog->getUniform("Texture0"));
		break;
		case 1: //Strike
		texture1->bind(card_prog->getUniform("Texture0"));
		break;
		case 2: //Strike
		texture2->bind(card_prog->getUniform("Texture0"));
		break;
		}
	}

	void SetSlashTex(int i) {
		switch (i) {
		case 1: //Strike
		slash1->bind(slash_prog->getUniform("Texture0"));
		break;
		case 2: //Strike
		slash2->bind(slash_prog->getUniform("Texture0"));
		break;
		case 3: //Strike
		slash3->bind(slash_prog->getUniform("Texture0"));
		break;
		case 4: //Strike
		slash4->bind(slash_prog->getUniform("Texture0"));
		break;
		case 5: //Strike
		slash5->bind(slash_prog->getUniform("Texture0"));
		break;
		case 6: //Strike
		slash6->bind(slash_prog->getUniform("Texture0"));
		break;
		}
	}


	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the texture GLSL program.
		card_prog = make_shared<Program>();
		card_prog->setVerbose(true);
		card_prog->setShaderNames(resourceDirectory + "/card_tex_vert.glsl", resourceDirectory + "/card_tex_frag.glsl");
		card_prog->init();
		card_prog->addUniform("P");
		card_prog->addUniform("V");
		card_prog->addUniform("M");
		card_prog->addUniform("lightPos");
		card_prog->addAttribute("vertPos");
		card_prog->addAttribute("vertNor");
		card_prog->addAttribute("vertTex");
		card_prog->addUniform("MatAmb");
		card_prog->addUniform("MatDif");
		card_prog->addUniform("MatSpec");
		card_prog->addUniform("shine");
		card_prog->addUniform("cameraLoc");
		card_prog->addUniform("lightIntensity");
		card_prog->addUniform("lightDropoff");
		card_prog->addUniform("Texture0");

		// Initialize the texture GLSL program.
		floor_prog = make_shared<Program>();
		floor_prog->setVerbose(true);
		floor_prog->setShaderNames(resourceDirectory + "/floor_tex_vert.glsl", resourceDirectory + "/floor_tex_frag.glsl");
		floor_prog->init();
		floor_prog->addUniform("P");
		floor_prog->addUniform("V");
		floor_prog->addUniform("M");
		floor_prog->addUniform("lightPos");
		floor_prog->addAttribute("vertPos");
		floor_prog->addAttribute("vertNor");
		floor_prog->addAttribute("vertTex");
		floor_prog->addUniform("MatAmb");
		floor_prog->addUniform("MatDif");
		floor_prog->addUniform("MatSpec");
		floor_prog->addUniform("shine");
		floor_prog->addUniform("cameraLoc");
		floor_prog->addUniform("lightIntensity");
		floor_prog->addUniform("lightDropoff");
		floor_prog->addUniform("Texture0");

		// Initialize the GLSL program.
		cube_prog = make_shared<Program>();
		cube_prog->setVerbose(true);
		cube_prog->setShaderNames(resourceDirectory + "/cube_vert.glsl", resourceDirectory + "/cube_frag.glsl");
		cube_prog->init();
		cube_prog->addUniform("P");
		cube_prog->addUniform("V");
		cube_prog->addUniform("M");
		cube_prog->addUniform("lightPos");
		cube_prog->addAttribute("vertPos");
		cube_prog->addAttribute("vertNor");
		cube_prog->addAttribute("vertTex");
		cube_prog->addUniform("MatAmb");
		cube_prog->addUniform("MatDif");
		cube_prog->addUniform("MatSpec");
		cube_prog->addUniform("shine");
		cube_prog->addUniform("cameraLoc");
		cube_prog->addUniform("lightIntensity");
		cube_prog->addUniform("lightDropoff");

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("lightPos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("shine");
		prog->addUniform("cameraLoc");
		prog->addUniform("lightIntensity");
		prog->addUniform("lightDropoff");

		// Initialize the GLSL program.
		text_prog = make_shared<Program>();
		text_prog->setVerbose(true);
		text_prog->setShaderNames(resourceDirectory + "/text_vert.glsl", resourceDirectory + "/text_frag.glsl");
		text_prog->init();
		text_prog->addUniform("projection");
		text_prog->addUniform("text");
		text_prog->addUniform("textcolor");
		text_prog->addAttribute("vertex");

		// Initialize the texture GLSL program.
		slash_prog = make_shared<Program>();
		slash_prog->setVerbose(true);
		slash_prog->setShaderNames(resourceDirectory + "/slash_tex_vert.glsl", resourceDirectory + "/slash_tex_frag.glsl");
		slash_prog->init();
		slash_prog->addUniform("P");
		slash_prog->addUniform("V");
		slash_prog->addUniform("M");
		slash_prog->addUniform("lightPos");
		slash_prog->addAttribute("vertPos");
		slash_prog->addAttribute("vertNor");
		slash_prog->addAttribute("vertTex");
		slash_prog->addUniform("MatAmb");
		slash_prog->addUniform("MatDif");
		slash_prog->addUniform("MatSpec");
		slash_prog->addUniform("shine");
		slash_prog->addUniform("cameraLoc");
		slash_prog->addUniform("lightIntensity");
		slash_prog->addUniform("lightDropoff");
		slash_prog->addUniform("Texture0");
	}

	void initGeom(const std::string& resourceDirectory)
	{

		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;
		//load in the mesh and make the shape(s)
		for(const string &path : obj_files) {
			bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + path).c_str());
			if (!rc) {
				cerr << errStr << endl;
			} else {

				//cout << "path:" << path << endl;
				multishape = make_shared<Multishape>();

				for (unsigned i = 0; i<TOshapes.size(); i++) {
					mesh = make_shared<Shape>();
					mesh->createShape(TOshapes[i]);
					mesh->measure();
					mesh->init();
					multishape->addShape(mesh);	
				}

				meshes.push_back(multishape);
			}
		}
	}
	
	// Code to load in the three textures
	void initTex(const std::string& resourceDirectory){
		texture0 = make_shared<Texture>();
		texture0->setFilename(resourceDirectory + "/Strike_FB.jpg");
		texture0->init();
		texture0->setUnit(2);
		texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		texture1 = make_shared<Texture>();
		texture1->setFilename(resourceDirectory + "/Pommel_FB.jpg");
		texture1->init();
		texture1->setUnit(2);
		texture1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		texture2 = make_shared<Texture>();
		texture2->setFilename(resourceDirectory + "/Defend_FB.jpg");
		texture2->init();
		texture2->setUnit(2);
		texture2->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		texture3 = make_shared<Texture>();
		texture3->setFilename(resourceDirectory + "/floortex.jpg");
		texture3->init();
		texture3->setUnit(2);
		texture3->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		slash1 = make_shared<Texture>();
		slash1->setFilename(resourceDirectory + "/slash/slash1.jpg");
		slash1->init();
		slash1->setUnit(2);
		slash1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		slash2 = make_shared<Texture>();
		slash2->setFilename(resourceDirectory + "/slash/slash2.jpg");
		slash2->init();
		slash2->setUnit(2);
		slash2->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		slash3 = make_shared<Texture>();
		slash3->setFilename(resourceDirectory + "/slash/slash3.jpg");
		slash3->init();
		slash3->setUnit(2);
		slash3->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		slash4 = make_shared<Texture>();
		slash4->setFilename(resourceDirectory + "/slash/slash4.jpg");
		slash4->init();
		slash4->setUnit(2);
		slash4->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		slash5 = make_shared<Texture>();
		slash5->setFilename(resourceDirectory + "/slash/slash5.jpg");
		slash5->init();
		slash5->setUnit(2);
		slash5->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		slash6 = make_shared<Texture>();
		slash6->setFilename(resourceDirectory + "/slash/slash6.jpg");
		slash6->init();
		slash6->setUnit(2);
		slash6->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}

	unsigned int createSky(string dir, vector<string> faces) {
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(false);
		for(GLuint i = 0; i < faces.size(); i++) {
		unsigned char *data =
		stbi_load((dir+faces[i]).c_str(), &width, &height, &nrChannels, 0);
		if (data) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
		0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		} else {
		cout << "failed to load: " << (dir+faces[i]).c_str() << endl;
		}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		cout << " creating cube map any errors : " << glGetError() << endl;
		return textureID;
	}  

	void initSentries() {

		vector<int> sen_x = {0,-1,1,4,-3,0,-2,-3};
		vector<int> sen_z = {0,0,0,-2,7,-4,2,-2};

		for (int i=0; i< sen_x.size(); i++) {
			mysentry = make_shared<Sentry>();
			mysentry->pos.x = sen_x[i];
			mysentry->pos.z = sen_z[i];
			sentries.push_back(mysentry);
		}
	}

	void initCards() {
		
		// Add 5 strikes, 5 defendes, and 2 pommels
		for (int i=0; i<5; i++) {
			mycard = make_shared<Card>(STRIKE_ID);
			drawpile.push_back(mycard);
		}
		for (int i=0; i<5; i++) {
			mycard = make_shared<Card>(DEFEND_ID);
			drawpile.push_back(mycard);
		}
		for (int i=0; i<2; i++) {
			mycard = make_shared<Card>(POMMEL_ID);
			drawpile.push_back(mycard);
		}
		// shuffle and draw 5 cards
		random_shuffle(drawpile.begin(), drawpile.end());
		for (int i=0; i<5; i++) {
			mycard = drawpile[0];
			drawpile.erase(drawpile.begin());
			hand.push_back(mycard);
		}
	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
    }

	void render() {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// ***** Camera + Player Positioning **************************************************

		// Pitch and yaw of camera
		glfwGetCursorPos(windowManager->getHandle(), &mouse_x, &mouse_y);
		double delta_x = mouse_x - win_w/2;
		double delta_y = win_h/2 - mouse_y;
		glfwSetCursorPos(windowManager->getHandle(), win_w/2, win_h/2);

		phi += delta_y * .01;
		theta += delta_x * .01;
		if (phi > 3.14/3) {
			phi = 3.14/3;
		}
		if (phi < -3.14/3) {
			phi = -3.14/3;
		}
		if (theta > 6.28) {
			theta -= 6.28;
		}
		if (theta < 0) {
			theta += 6.28;
		}

		lookat = vec3(cos(phi)*cos(theta),sin(phi),cos(phi)*cos(3.14159/2-theta)) + pos;
		viewdir = lookat - pos;
		
		// Check if "sprinting"
		if(glfwGetKey(windowManager->getHandle(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			speed = std::min(speed + .005f, .05f);
			fov = std::min(fov + .05f, 45.2f);
		}
		else {
			speed = std::max(speed - .005f, .03f);
			fov = std::max(fov - .05f, 45.0f);
		}

		// Dolly camera
		if(glfwGetKey(windowManager->getHandle(), GLFW_KEY_W) == GLFW_PRESS) {
			vec3 walkdir = normalize(vec3(viewdir.x,0,viewdir.z)) * speed;
			pos += walkdir;
			lookat += walkdir;
		}
		if(glfwGetKey(windowManager->getHandle(), GLFW_KEY_S) == GLFW_PRESS) {
			vec3 walkdir = normalize(vec3(viewdir.x,0,viewdir.z)) * speed;
			pos -= walkdir;
			lookat -= walkdir;
		}

		// Strafe camera
		if(glfwGetKey(windowManager->getHandle(), GLFW_KEY_A) == GLFW_PRESS) {
			pos -= cross(viewdir, upvec) * speed;
			lookat -= cross(viewdir, upvec) * speed;
		}
		if(glfwGetKey(windowManager->getHandle(), GLFW_KEY_D) == GLFW_PRESS) {
			pos += cross(viewdir, upvec) * speed;
			lookat += cross(viewdir, upvec) * speed;
		}

		// View is global translation along negative z for now
		View->pushMatrix();
			View->lookAt(pos, lookat, upvec);

		// Apply perspective projection.
		Projection->pushMatrix();
			Projection->perspective(fov, aspect, 0.01f, 100.0f);

		// ***** Card Logic *******************************************************

		// Put more cards in hand if empty
		if (hand.size() == 0) {
			for (int i=0; i<5; i++) {
				if (drawpile.empty()) {
					drawpile = discard;
					discard = {};
					random_shuffle(drawpile.begin(), drawpile.end());
				}
				mycard2 = drawpile[0];
				drawpile.erase(drawpile.begin());
				hand.push_back(mycard2);
			}
			selected_card = 0;
		}

		// Move hand down if throwing
		float handshift = 0;
		if (hand_state == HAND_THROWING) {
			handshift = 1.9*pow((glfwGetTime() - throw_start - throw_duration/2), 2) - .13;
		}
		if (glfwGetTime() > throw_start + throw_duration) {
			hand_state = HAND_READY;
		}
		// Actually throw the card halfway through the animation
		else if (glfwGetTime() > throw_start + throw_duration/2 && !has_activated_card) {
			
			has_activated_card = true;
			mycard = hand[selected_card];

			// Put the card in our discard pile
			hand.erase(hand.begin() + selected_card);
			discard.push_back(mycard);
			selected_card = std::max(selected_card-1, 0);

			// Throw the card if it can be thrown
			if (mycard->throwable) {
				thrown_cards.push_back(mycard->throwCard(pos, viewdir, theta));
			}
			// Draw cards if needed
			for (int i=0; i<mycard->draw; i++) {
				if (drawpile.empty()) {
					drawpile = discard;
					discard = {};
					random_shuffle(drawpile.begin(), drawpile.end());
				}
				mycard = drawpile[0];
				drawpile.erase(drawpile.begin());
				hand.push_back(mycard);
			}
			// Gain block if needed
			block += mycard->block;
		}

		//Check thrown cards for collision with floor / ceiling
		int i = 0;
		while(i < thrown_cards.size()) {
			if (thrown_cards[i]->pos.y < 0.01 || thrown_cards[i]->pos.y > 4.1) {
				mycard2 = thrown_cards[i];
				thrown_cards.erase(thrown_cards.begin() + i);
				mycard2->makeStuck();
				stuck_cards.push_back(mycard2);
				i--;
			}
			i++;
		}

		//Check thrown cards for collision with Sentries
		for (int j=0; j<sentries.size(); j++) {
			i = 0;
			vec3 senxyz  =sentries[j]->hitboxpos;
			float sentop = sentries[j]->top;
			float senbot = sentries[j]->bottom;

			while (i<thrown_cards.size()) {
				vec3 cardpos = thrown_cards[i]->pos;
				// Check if cards are within a hitbox made up of two cones
				if (0 > - pow(cardpos.y - sentop, 2)/4.0f + pow(cardpos.x - senxyz.x, 2) + pow(cardpos.z - senxyz.z, 2) &&
					0 > - pow(cardpos.y - senbot, 2)/4.0f + pow(cardpos.x - senxyz.x, 2) + pow(cardpos.z - senxyz.z, 2) &&
					cardpos.y < sentop && cardpos.y > senbot ) {
					sentries[j]->health -= thrown_cards[i]->damage;
					mycard = thrown_cards[i];
					mycard->state = CARD_SLASH;
					slashing_cards.push_back(mycard);
					thrown_cards.erase(thrown_cards.begin() + i);
					i--;
				}
				i++;
			}
		}

		refresh = std::max((int)ceil(refreshtime - glfwGetTime()), 0);

		// ***** Sentry Logic ***************************************************************************

		// Perform various operations on sentries
		while (i<sentries.size()) {
			// Update player position
			if (!sentries[i]->charging) {
				sentries[i]->playerPos = pos;
			}
			// Deal damage to the player if needed
			if (sentries[i]->firing && !sentries[i]->hasDealtDamage) {
				vec3 shootdir = sentries[i]->playerPos - sentries[i]->pos;
				shootdir.y = 0;
				shootdir = normalize(shootdir);
				// Check if the player is being hit by the laser using a plane and distance to the shoot dir
				if (dot(shootdir, sentries[i]->pos - pos) < 0 &&
				    length(cross(pos - sentries[i]->pos, pos - sentries[i]->pos + shootdir)) < .3 ) {
					block -= 9;
					if (block < 0) {
						health += block;
						block = 0;
					}
					sentries[i]->hasDealtDamage = true;
				}	
			}
			// Discard dead sentries
			if (sentries[i]->state == SENTRY_DEAD) {
				sentries.erase(sentries.begin() + i);
				i--;
			}
			i++;
		}

		// ***** Card Texture Prog *****************************************************************

		card_prog->bind();
		glUniformMatrix4fv(card_prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(card_prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));

		// Set up light and camera location
		glUniform3f(card_prog->getUniform("lightIntensity"), furnacecol.x, furnacecol.y, furnacecol.z);
		glUniform3f(card_prog->getUniform("lightDropoff"), 1.0, 0.05, 0.0);
		glUniform3f(card_prog->getUniform("cameraLoc"), pos.x, pos.y, pos.z);
		glUniform3f(card_prog->getUniform("lightPos"), furnacelight.x, furnacelight.y, furnacelight.z);

		// Draw cards in left hand
		for (int i=0; i < hand.size(); i++) {
			SetCardTex(hand[i]->card_id % 3);
			hand[i]->drawHandCard(card_prog,meshes,i,hand.size(),phi,theta,pos,viewdir,handshift);
		}

		// Drawn thrown cards
		for (int i=0; i< thrown_cards.size(); i++) {
			SetCardTex(thrown_cards[i]->card_id % 3);
			thrown_cards[i]->drawThrownCard(card_prog,meshes);
		}

		// Drawn stuck cards (stuck in floor/walls)
		for (int i=0; i< stuck_cards.size(); i++) {
			SetCardTex(stuck_cards[i]->card_id % 3);
			stuck_cards[i]->drawStuckCard(card_prog,meshes);
		}

		card_prog->unbind();


		// ***** Slash Texture Prog *****************************************************************

		slash_prog->bind();
		glUniformMatrix4fv(slash_prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(slash_prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));

		// Set up light and camera location
		glUniform3f(slash_prog->getUniform("lightIntensity"), furnacecol.x, furnacecol.y, furnacecol.z);
		glUniform3f(slash_prog->getUniform("lightDropoff"), 1.0, 0.05, 0.0);
		glUniform3f(slash_prog->getUniform("cameraLoc"), pos.x, pos.y, pos.z);
		glUniform3f(slash_prog->getUniform("lightPos"), furnacelight.x, furnacelight.y, furnacelight.z);

		// Drawn cards doing the slash animation
		i = 0;
		while (i< slashing_cards.size()) {
			SetSlashTex(1);
			if (slashing_cards[i]->slashFrame < 18) {
				SetSlashTex(ceil(slashing_cards[i]->slashFrame / 3));
				slashing_cards[i]->drawSlashingCard(slash_prog,meshes,theta);
				slashing_cards[i]->slashFrame += 1;
			}
			else {
				slashing_cards.erase(slashing_cards.begin() + i);
				i--;
			}
			i++;
		}

		slash_prog->unbind();

		// ***** Floor Texture Prog *****************************************************************

		floor_prog->bind();
		glUniformMatrix4fv(floor_prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(floor_prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));

		// Set up light and camera location
		glUniform3f(floor_prog->getUniform("lightIntensity"), furnacecol.x, furnacecol.y, furnacecol.z);
		glUniform3f(floor_prog->getUniform("lightDropoff"), 1.0, 0.05, 0.0);
		glUniform3f(floor_prog->getUniform("cameraLoc"), pos.x, pos.y, pos.z);
		glUniform3f(floor_prog->getUniform("lightPos"), furnacelight.x, furnacelight.y, furnacelight.z);

		texture3->bind(floor_prog->getUniform("Texture0"));

		Model->pushMatrix();
		Model->loadIdentity();

		for (int i=-15; i < 15; i++) {
			for (int j=-15; j < 15; j++) {
			Model->pushMatrix();
				Model->translate(vec3(i,0,j));
				Model->scale(.5);
				setModel(floor_prog, Model);
				meshes[floortile]->shapes[0]->draw(floor_prog);
			Model->popMatrix();
			}
		}

		Model->popMatrix();

		floor_prog->unbind();

		// ***** Untextured Prog **********************************************************************

		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));

		// Set up light and camera location
		glUniform3f(prog->getUniform("lightIntensity"), furnacecol.x, furnacecol.y, furnacecol.z);
		glUniform3f(prog->getUniform("lightDropoff"), 1.0, 0.05, 0.0);
		glUniform3f(prog->getUniform("cameraLoc"), pos.x, pos.y, pos.z);
		glUniform3f(prog->getUniform("lightPos"), furnacelight.x, furnacelight.y, furnacelight.z);

		// Draw
		Model->pushMatrix();
			Model->loadIdentity();

			// Draw a grid to help visualize scale, activated with "P"
			if(glfwGetKey(windowManager->getHandle(), GLFW_KEY_P) == GLFW_PRESS) {
				for (int i=-5; i<5; i++) {
					for (int j=-5; j<5; j++) {
						for (int k=-5; k<5; k++) {
							Model->pushMatrix();
								
								Model->translate(vec3(i,j,k));
								Model->scale(vec3(.05/(meshes[sphere]->max.y - meshes[sphere]->min.y)));
								setModel(prog, Model);
								meshes[sphere]->shapes[0]->draw(prog);
							Model->popMatrix();
						}
					}
				}
			}

			// Draw card selector
			SetMaterial(3);
			Model->pushMatrix();
				// Move down if throwing
				Model->translate(-handshift * normalize(cross(viewdir,cross(viewdir, upvec))));
				// Move down of center
				Model->translate((.02f + 0.005f * float(pow(abs((hand.size() - 1)/2.0f - float(selected_card)),2))) * normalize(cross(viewdir,cross(viewdir, upvec))));
				// Move left of center
				Model->translate((-.1f + 0.05f * ((hand.size() - 1)/2.0f - float(selected_card)) )*normalize(cross(viewdir, upvec)));
				// Move in front of camera
				Model->translate(pos + normalize(viewdir) * (0.2f + float(selected_card)/1000));
				// Rotate and face camera
				Model->rotate(phi - .05f, normalize(cross(viewdir, upvec)));
				Model->rotate(-theta, vec3(0,1,0));
				Model->rotate(glfwGetTime(), vec3(0,1,0));
				// Scale to size
				Model->scale(vec3(.01/(meshes[sentry]->shapes[0]->max.y - meshes[sentry]->shapes[0]->min.y)));
				setModel(card_prog, Model);
				meshes[sentry]->shapes[0]->draw(card_prog);
			Model->popMatrix();

			// Draw sentries
			for (int i=0; i< sentries.size(); i++) {
				sentries[i]->drawSentry(prog,meshes);
			}

			SetMaterial(1);
			// Draw ceiling
			Model->pushMatrix();
				Model->translate(vec3(0,2*.3*(meshes[wall]->max.y-meshes[wall]->min.y) -meshes[floormod]->max.y,0));
				Model->scale(vec3(10.5,-1,10.5));
				setModel(prog, Model);
				meshes[floormod]->shapes[0]->draw(prog);
			Model->popMatrix();

			// Draw Walls around us
			SetMaterial(7);
			Model->pushMatrix();
				for (int i =0; i < 60; i+=2) {
					Model->pushMatrix();
						Model->rotate(float(i)/60.0 * 6.28, vec3(0,1,0));
						Model->translate(vec3(0,0,10));
						Model->scale(vec3(.5,.3,.5));
						Model->rotate(((i/2)%4)*3.14/2, vec3(0,1,0));
						Model->translate(vec3(-(meshes[wall]->max.x+meshes[wall]->min.x)/2, 0, -(meshes[wall]->max.z+meshes[wall]->min.z)/2));
						setModel(prog, Model);
						meshes[wall]->shapes[0]->draw(prog);
					Model->popMatrix();
				}
				for (int i =0; i < 60; i++) {
					Model->pushMatrix();
						Model->rotate((float(i)/60.0 + 1.0/120)* 6.28, vec3(0,1,0));
						Model->translate(vec3(0,.3*(meshes[wall]->max.y-meshes[wall]->min.y),10));
						Model->scale(vec3(.5,.3,.5));
						Model->rotate((i%4)*3.14/2, vec3(0,1,0));
						setModel(prog, Model);
						meshes[wall]->shapes[0]->draw(prog);
					Model->popMatrix();
				}
			Model->popMatrix();

			// Draw furnace
			Model->pushMatrix();
				SetMaterial(8);
				Model->rotate(11/60.0 * 6.28, vec3(0,1,0));
				Model->translate(vec3(0,0,-8.6));
				Model->scale(vec3(1.0f/(meshes[furnace]->max.y - meshes[furnace]->min.y)));
				Model->translate(vec3((meshes[furnace]->max.x-meshes[furnace]->min.x)/2,0,(meshes[furnace]->max.z-meshes[furnace]->min.z)/2));
				setModel(prog, Model);
				meshes[furnace]->shapes[0]->draw(prog);
				SetMaterial(9);
				for (int i =0; i < meshes[logs]->shapes.size(); i++) {
					setModel(prog, Model);
					meshes[logs]->shapes[i]->draw(prog);
				}
			Model->popMatrix();

			// Draw floating atmospheric objects
			SetMaterial(10);

			// Icosahedrons
			Model->pushMatrix();
				Model->translate(vec3(4,3.5,1));
				Model->rotate(glfwGetTime() * .1, vec3(0,1,.1 * sin(glfwGetTime())));
				Model->scale(0.2f/(meshes[icosa]->max.y - meshes[icosa]->min.y));
				setModel(prog, Model);
				meshes[icosa]->shapes[0]->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();
				Model->translate(vec3(-2,3.7,-5));
				Model->rotate(glfwGetTime() * .2, vec3(1, -.1 * sin(glfwGetTime()), 1));
				Model->scale(0.15f/(meshes[icosa]->max.y - meshes[icosa]->min.y));
				setModel(prog, Model);
				meshes[icosa]->shapes[0]->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();
				Model->translate(vec3(-6,3.3,1));
				Model->rotate(glfwGetTime() * .2, vec3(1, -.1 * sin(glfwGetTime()), 1));
				Model->scale(0.15f/(meshes[icosa]->max.y - meshes[icosa]->min.y));
				setModel(prog, Model);
				meshes[icosa]->shapes[0]->draw(prog);
			Model->popMatrix();

			//spheres + semispheres
			Model->pushMatrix();
				Model->translate(vec3(3,3.7 + .1*sin(.08*glfwGetTime()),5));
				Model->rotate(-.4, vec3(1,0,1));
				Model->scale(0.15f/(meshes[sph]->max.y - meshes[sph]->min.y));
				setModel(prog, Model);
				meshes[semsph]->shapes[0]->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();
				Model->translate(vec3(3,3.6+ .1*sin(.11*glfwGetTime()+1),5.2));
				Model->scale(0.2f/(meshes[sph]->max.y - meshes[sph]->min.y));
				setModel(prog, Model);
				meshes[sph]->shapes[0]->draw(prog);
			Model->popMatrix();

			Model->pushMatrix();
				Model->translate(vec3(-2,3.5 + .1*sin(1+.07*glfwGetTime()),-.2));
				Model->scale(0.15f/(meshes[sph]->max.y - meshes[sph]->min.y));
				setModel(prog, Model);
				meshes[sph]->shapes[0]->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();
				Model->translate(vec3(-2.2,3.7+ .1*sin(2+.15*glfwGetTime()+1), 0.7));
				Model->scale(0.2f/(meshes[sph]->max.y - meshes[sph]->min.y));
				setModel(prog, Model);
				meshes[sph]->shapes[0]->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();
				Model->translate(vec3(-2.4,3.2+ .1*sin(3+.09*glfwGetTime()+1), 0.1));
				Model->rotate(.3, vec3(1,0,0));
				Model->scale(0.2f/(meshes[sph]->max.y - meshes[sph]->min.y));
				setModel(prog, Model);
				meshes[semsph]->shapes[0]->draw(prog);
			Model->popMatrix();

			// Pyramids
			Model->pushMatrix();
				Model->translate(vec3(2.4,3.2+ .1*sin(3+.03*glfwGetTime()+1), -5.3));
				Model->rotate(glfwGetTime() * .2 + 1, vec3(1,0,1));
				Model->pushMatrix();
					Model->translate(vec3(0,.1,0));
					Model->scale(0.1f/(meshes[pyr]->max.y - meshes[pyr]->min.y));
					setModel(prog, Model);
					meshes[pyr]->shapes[0]->draw(prog);
				Model->popMatrix();
				Model->pushMatrix();
					Model->rotate(3.14, vec3(1,0,0));
					Model->translate(vec3(0,.1,0));
					Model->scale(0.1f/(meshes[pyr]->max.y - meshes[pyr]->min.y));
					setModel(prog, Model);
					meshes[pyr]->shapes[0]->draw(prog);
				Model->popMatrix();
				

			Model->popMatrix();


			
		Model->popMatrix();

		prog->unbind();

		// ***** Skybox **********************************************************************

		cube_prog->bind();
		glUniformMatrix4fv(cube_prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		//set the depth function to always draw the box!
		glDepthFunc(GL_LEQUAL);
		//set up view matrix to include your view transforms 
		//(your code likely will be different depending
		glUniformMatrix4fv(cube_prog->getUniform("V"), 1, GL_FALSE,value_ptr(View->topMatrix()) );
		//set and send model transforms - likely want a bigger cube
		Model->pushMatrix();
			Model->scale(100);
			glUniformMatrix4fv(cube_prog->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()));
		Model->popMatrix();
		//bind the cube map texture
		glBindTexture(GL_TEXTURE_CUBE_MAP,0);
		//draw the actual cube
		meshes[cube]->shapes[0]->draw(cube_prog);
		glDepthFunc(GL_LESS);
		cube_prog->unbind(); 

		// Pop matrix stacks.
		Projection->popMatrix();
		View->popMatrix();

		textRenderer->RenderText(text_prog, "Block: " + to_string(block), 50.0f, 680.0f,0.5f,vec3(.1,.1,1),win_w,win_h);
		textRenderer->RenderText(text_prog, "Health: " + to_string(health) + "/" + to_string(75), 50.0f, 650.0f,0.5f,vec3(1,.1,.1),win_w,win_h);
		textRenderer->RenderText(text_prog, "Energy: " + to_string(energy) + "/" + to_string(3), 50.0f, 620.0f,0.5f,vec3(.1,1,.1),win_w,win_h);
		textRenderer->RenderText(text_prog, "Refresh: " + to_string(refresh) + "s", 50.0f, 590.0f,0.5f,vec3(.1,1,.1),win_w,win_h);
	}
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(application->win_w, application->win_h);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);	
	glfwSetCursorPos(windowManager->getHandle(), application->win_w/2, application->win_h/2);

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Set up textures
	application->initTex(resourceDir);

	// Set up skybox
	application->createSky(resourceDir + "/sky/", application->faces);

	// Add bad guys in
	application->initSentries();

	// Add in cards
	application->initCards();

	// Load text renderer
	application->textRenderer = make_shared<TextRenderer>(resourceDir + "/Kreon-Regular.ttf");

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
