#include "GLInclude.h"
#include "GLUtils.h"
#include "../VectorField.h"
#include "../Material.h"
#include "../Mesh.h"
#include "Camera.h"

#include <sstream>
#include <iostream>
#include <vector>
#include <math.h>
using std::stringstream;
using std::string;
using std::cout;
using std::endl;

using namespace vectorfield;

GLFWwindow* window;
string title;

//timing related variables
float last_time=0, current_time =0;
//delta time
float dt = 0;

#define WIDTH 1024
#define HEIGHT 768

VectorField* vectorField;
vector<Mesh*> objects;

Camera* camera = NULL;

//camera / mouse
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstmouse = true;
bool usemouse = false;

void doMovement();

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //cout << key << endl;
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if(action == GLFW_PRESS)
            keys[key] = true;
        else if(action == GLFW_RELEASE)
            keys[key] = false;
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    //cout << "button: " << button << " action: " << action << endl;
    if (button == 0 && action == 1)
        usemouse = true;
    
    else if (button == 0 && action == 0)
        usemouse = false;
}


static void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    //camera->processMouseScroll(yoffset);
}

static void mouse_position_callback(GLFWwindow* window, double xpos, double ypos) {
    
    if(firstmouse) {
        camera->move_camera = false;
        camera->Move2D((int)xpos, (int)ypos);
        firstmouse = false;
        return;
    }
    
    camera->move_camera = usemouse;
    camera->Move2D((int)xpos, (int)ypos);
}

static void window_size_callback(GLFWwindow* window, int width, int height) {
	
}



void init_resources() {
    
    camera = new Camera();
    camera->SetPosition(glm::vec3(77008, 54684.8, 111898));
    camera->SetLookAt(glm::vec3(77007.8, 54684.2, 111897));
    camera->SetViewport(0, 0, WIDTH, HEIGHT);
    camera->SetClipping(1, 1000000);
    //speed
    camera->camera_scale = 30;
    camera->Update();
    
    // vectorfield
    vectorField = new VectorField();
    vectorField->init(0, 0, 109379.614343, 72919.7428954, 4000, 100);
    
    vectorField->addControlPoint( 72195.6097426 , 10213.8282815 , 4.9193495505 , 9.838699101 );
    vectorField->addControlPoint( 80216.7814611 , 8390.83470912 , 6.36396103068 , 6.36396103068 );
    vectorField->addControlPoint( 19693.3948579 , 52142.6804464 , 10.6066017178 , 10.6066017178 );
    vectorField->addControlPoint( 91519.3416099 , 67820.4251689 , -6.36396103068 , 6.36396103068 );
    vectorField->addControlPoint( 12036.8218539 , 21151.7897158 , 9.19238815543 , 9.19238815543 );
    vectorField->addControlPoint( 9484.63085255 , 33548.146008 , 8.049844719 , 4.0249223595 );
    vectorField->addControlPoint( 38652.5280107 , 23703.9807172 , 7.6026311235 , 15.205262247 );
    vectorField->addControlPoint( 64539.0367386 , 39017.1267252 , 4.9193495505 , 9.838699101 );
    vectorField->addControlPoint( 101728.105615 , 44850.7061568 , 8.4970583145 , 16.994116629 );
    vectorField->addControlPoint( 58340.8585925 , 10943.0257105 , 14.1421356237 , 14.1421356237 );
    vectorField->addControlPoint( 44850.7061568 , 50684.2855885 , 5.8137767415 , 11.627553483 );
    vectorField->addControlPoint( 17141.2038566 , 47767.4958726 , 5.8137767415 , 11.627553483 );
    vectorField->addControlPoint( 22245.5858592 , 48861.2920161 , 14.1421356237 , 14.1421356237 );
    vectorField->addControlPoint( 52871.8778753 , 47038.2984437 , 7.6026311235 , 15.205262247 );
    vectorField->addControlPoint( 70737.2148847 , 25891.573004 , 8.4970583145 , 16.994116629 );
    
    vectorField->printInfo();
    vectorField->update();
    
    // axes
    Mesh* xaxis = MeshUtils::cylinder(200, 10000, 3, 6);
    xaxis->rotate(degreesToRadians(-90), glm::vec3(0, 0, 1));
    xaxis->setColor(glm::vec4(1, 0, 0, 1));
    objects.push_back(xaxis);
    
    Mesh* yaxis = MeshUtils::cylinder(200, 10000, 3, 6);
    yaxis->setColor(glm::vec4(0, 1, 0, 1));
    objects.push_back(yaxis);
    
    Mesh* zaxis = MeshUtils::cylinder(200, 10000, 3, 6);
    zaxis->rotate(degreesToRadians(90), glm::vec3(1, 0, 0));
    zaxis->setColor(glm::vec4(0, 0, 1, 1));
    objects.push_back(zaxis);
    
}

void free_resources()
{
    if(vectorField)
        delete vectorField;
    if(camera)
        delete camera;
    for(int i = 0; i < objects.size(); i++)
        if(objects[i])
            delete objects[i];
}

void doMovement() {
    // Camera controls
    if(keys[GLFW_KEY_W])
        camera->Move(FORWARD);
    if(keys[GLFW_KEY_S])
        camera->Move(BACK);
    if(keys[GLFW_KEY_A])
        camera->Move(LEFT);
    if(keys[GLFW_KEY_D])
        camera->Move(RIGHT);
    if(keys[GLFW_KEY_Q])
        camera->Move(DOWN);
    if(keys[GLFW_KEY_E])
        camera->Move(UP);
    
    if(keys[GLFW_KEY_I]) {
        cout << "pos: " << camera->camera_position[0] << ", " << camera->camera_position[1] << ", " << camera->camera_position[2] << endl;
        cout << "direction: " << camera->camera_direction[0] << ", " << camera->camera_direction[1] << ", " << camera->camera_direction[2] << endl;
        cout << "lookat: " << camera->camera_look_at[0] << ", " << camera->camera_look_at[1] << ", " << camera->camera_look_at[2] << endl;
        cout << "up: " << camera->camera_up[0] << ", " << camera->camera_up[1] << ", " << camera->camera_up[2] << endl;
        keys[GLFW_KEY_I] = false;
    }
    
    if(keys[GLFW_KEY_T]) {
        keys[GLFW_KEY_T] = false;
    }
    if(keys[GLFW_KEY_N]) {
        keys[GLFW_KEY_N] = false;
    }
    camera->Update();
}

void mainLoop()
{
    const int samples = 50;
    float time[samples];
    int index = 0;

    do{
        //timing related calcualtion
        last_time = current_time;
        current_time = glfwGetTime();
        dt = current_time-last_time;
        
        glfwPollEvents();
        doMovement();
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
        // Dark blue background
        //glClearColor(1.0f,0.5f,0.5f,1.0f);
    
        // render vectorfield
        float* MV, *P;
        MV = (float*)glm::value_ptr(camera->MV);
        P = (float*)glm::value_ptr(camera->projection);
        
        if(vectorField)
            vectorField->render(MV, P);
        
        // render objects
        for(int i = 0; i < objects.size(); i++)
            if(objects[i])
                objects[i]->render(MV, P);

        glfwSwapBuffers(window);

        // Update FPS
        time[index] = glfwGetTime();
        index = (index + 1) % samples;

        if( index == 0 ) {
          float sum = 0.0f;
          for( int i = 0; i < samples-1 ; i++ )
            sum += time[i + 1] - time[i];
          float fps = samples / sum;

          stringstream strm;
          strm << title;
          strm.precision(4);
          strm << " (fps: " << fps << ")";
          glfwSetWindowTitle(window, strm.str().c_str());
        }

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );
}

int main(int argc, char* argv[]) {
    
    
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

    glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    
	// Open a window and create its OpenGL context
	window = glfwCreateWindow( WIDTH, HEIGHT, "OpenGL window with GLFW", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_position_callback);
    glfwSetScrollCallback(window, mouse_scroll_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// print GL info
	GLUtils::dumpGLInfo();

	// init resources
	init_resources();

	// Enter the main loop
	mainLoop();

	free_resources();

	// Close window and terminate GLFW
	glfwTerminate();

	// Exit program
	return EXIT_SUCCESS;
}
