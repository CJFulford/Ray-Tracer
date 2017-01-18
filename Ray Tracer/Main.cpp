#include "Header.h"
#include "ImageBuffer.h"

void QueryGLVersion();
bool CheckGLErrors();

int scene = 3;

void KeyCallback(
	GLFWwindow* window, 
	int key, 
	int scancode, 
	int action, 
	int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		scene = 1;
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		scene = 2;
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		scene = 3;
}

int main(
	int argc, 
	char *argv[])
{
	// initialize the GLFW windowing system
	if (!glfwInit()) {
		cout << "ERROR: GLFW failed to initilize, TERMINATING" << endl;
		return -1;
	}
	glfwSetErrorCallback(ErrorCallback);

	// attempt to create a window with an OpenGL 4.1 core profile context
	GLFWwindow *window;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Ray Tracer Window", 0, 0);
	if (!window) {
		cout << "Program failed to create GLFW window, TERMINATING" << endl;
		glfwTerminate();
		return -1;
	}

	// set keyboard callback function and make our context current (active)
	glfwSetKeyCallback(window, KeyCallback);
	glfwMakeContextCurrent(window);



	// load glad
	gladLoadGL();




	// query and print out information about our OpenGL environment
	QueryGLVersion();



	
	ImageBuffer imageBuffer;
	imageBuffer.Initialize();

	int oldScene = scene - 1;

	while (!glfwWindowShouldClose(window))
	{
		if (oldScene != scene) {
			// start recording time for render timer
			cout << "Rendering... ";
			time_t startTime = time(NULL);
			vec3 colourVec;

			vector<MyLight>		lightVec;
			vector<MySphere>	sphereVec;
			vector<MyPlane>		planeVec;
			vector<MyTriangle>	triangleVec;
			readFromFile("scene" + to_string(scene) + ".txt", lightVec, sphereVec, planeVec, triangleVec);

			// += 2.f since the window goes from -1 top 1, which is a distance of 2
			int xcoord_t;
			#pragma omp parallel for schedule(dynamic)
			for (xcoord_t = -WINDOW_WIDTH; xcoord_t < WINDOW_WIDTH; xcoord_t++)
			{
				float xcoord = (float)xcoord_t / WINDOW_WIDTH;
				for (float ycoord = -1.f; ycoord < 1.f; ycoord += 2.f / WINDOW_HEIGHT)
				{
					int recursive = MAX_RECURSIVE_RAYS;
					MyRay ray;
					if (ratio >= 1.f) ray.direction = vec3(xcoord * ratio, ycoord, FOCAL_LENGTH);
					else ray.direction = vec3(xcoord, ycoord / ratio, FOCAL_LENGTH);
					ray.origin = vec3(0.0, 0.0, 0.0);
					myNormalize(ray);
					colourVec = getColour(ray, sphereVec, triangleVec, planeVec, lightVec, recursive);
					imageBuffer.SetPixel((int)((xcoord * HALF_WIDTH) + HALF_WIDTH),
											(int)(((ycoord * HALF_HEIGHT) + HALF_HEIGHT)),
											colourVec);
				}
			}


			imageBuffer.Render();

			// scene is rendered to the back buffer, so swap to front for display
			glfwSwapBuffers(window);
			time_t endTime = time(NULL);
			cout << difftime(endTime, startTime) << " seconds"<< endl;
			oldScene = scene;
			if (scene == 1) imageBuffer.SaveToFile("scene1.png");
			else if (scene == 2) imageBuffer.SaveToFile("scene2.png");
			else if (scene == 3) imageBuffer.SaveToFile("scene3.png");
		}

		// sleep until next event before drawing again
		glfwWaitEvents();
	}

	// clean up allocated resources before exit
	glfwDestroyWindow(window);
	glfwTerminate();

	cout << "Goodbye!" << endl;
	return 0;
}
