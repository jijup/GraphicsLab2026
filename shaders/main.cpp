#include "Helpers/GlError.h"
#include "Triangle/Triangle.h"
#include "Quad/Quad.h"

///--- Task 1 draws the triangle. Set this to false for Tasks 2 and 3 (the Halloween scene).
bool task1 = true;

Triangle triangle;
Quad pumpkin;
Quad bat;

void init(){
    if (task1) {
        ///--- Sets background color
        glClearColor(/*white*/ 1.0, 1.0, 1.0, /*solid*/1.0);
        triangle.init();
    }
    else {
        ///--- A dark night sky
        glClearColor(0.20f, 0.15f, 0.38f, 1.0f);
        pumpkin.init("Quad/pumpkin.png");
        bat.init("Quad/bat.png");
    }
}

void display(){

    glClear(GL_COLOR_BUFFER_BIT);
    if (task1) {
        triangle.draw();
    }
    else {
        pumpkin.draw(0);  ///< id 0: the pumpkin
        bat.draw(1);      ///< the same bat quad, drawn three times
        bat.draw(2);
        bat.draw(3);
    }

}

void cleanup(){
    if (task1) {
        triangle.cleanup();
    }
    else {
        pumpkin.cleanup();
        bat.cleanup();
    }
}

int main(int, char**){
	GLFWwindow* window;
    
	glewExperimental = true; // for core profile

	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	//versions
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // needed on macOS

	// Open a window and create its OpenGL context

	window = glfwCreateWindow(1024, 768, "Shaders", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. Your GPU or driver may not support OpenGL 3.3.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window); // Initialize GLEW

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}
	init();

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	do {
		// Clear the screen and draw
		display();
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	cleanup();
	glfwTerminate();

    return EXIT_SUCCESS;
}
