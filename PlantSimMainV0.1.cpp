/*
* Current Issues:  1)Adding ability to draw new tree (with randomized parameters) by pressing a key -- DONE
*                  2)Change Camera mode by key press -- DONE [press R]
*                  3) In rotate camera mode, change radius of rotation with a key -- [DONE Press up or down]
*                  3b) Press key to cycle through shader colorsets (at least 3) -- DONE [press C] --DONE
*                       -pink-red, blue-cyan-green, purple-blue, etc   
*                  3c) Press button to re-draw all triangles --DONE
*                  3d) find a more "neutral" background color than dark teal -- maybe dark gray -- DONE
*                  4) Clean up code and add Comments -- DONE
*                  5)Build to executable, push online to share on social media
*Date:27 Jan, 2023
*Author: Alexander Sanfilippo
* Brief: This program uses openGL and L-Systems to draw procedurally generated tree-like modules by generating 
*        all the neccessary vertex information
*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <headers/shader_s.h>

#include <iostream>

/*Add Matric Algebra Functions*/
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

/*Texture Loading*/
#include "headers/stb_image.h"

/*Camera*/
#include <headers/camera.h>

/*Trees*/
#include <headers/turtleplant.h>
#include <headers/plant.h>
#include <headers/map.h>


//function headers
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

// stores how much we're seeing of either texture
float mixValue = 0.2f;
// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
/*[CAMERA] Create Camera Object*/
// camera
Camera ourCam = Camera(glm::vec3(0.0f, 13.0f, 35.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//animate drawing of tree
int numRectangles = 1;
unsigned int numRectanglesMax;
bool gotKey = false;

bool rotateCamera = false;

//change tree color on press C
int colorSetVal = 0;
int colorSetChange = 0;

//change camera rotation radius in-program UP and DOWN keys
float radius = 60.0f;

//Forr spawning new tree - change name later
float stretch = 1.0f;
bool changeTree = false;
//For "growing" tree by manipulating scale over time
float scale = 1.0;
bool doAnimate = false;
//countrol iterative growth - unneeded
unsigned int iteration = 6;
//iterate thru growth stages
unsigned int growthStage = 6;
bool changeGrowthStage = false;
int seed = 1904;

//adding new plants
bool addPlant = false;

//TEMP
int tempgs = 6;
//MAP
float mapScale = 1200.0;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Plant Sim V.1 by Alexander Sanfilippo", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //for camera-mouse input
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //discrete key press functionality 
    glfwSetKeyCallback(window, key_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("3.3.shaderPlant.vs", "3.3.shaderPlant.fs");
    Shader mapShader("3.3.shader.vs", "3.3.shader.fs");

    //PLANT SETUP
    Plant plant = Plant(seed); //creates turtle, lsystem, creates position vectors, basically everything
    vector <Plant> plantVector = { plant }; //holds all plants in world

    //MAP SETUP
    unsigned int size = 120;
    unsigned int octaves = 4;
    float smooth = 2.5; //higher -> bumpier.  closer to 0 -> flatter
    int seed = 10363;
    unsigned int frequency = 2; //cannot be under 2
    int numMapVertices = size * size * 6;
    float scale = 1.0f;
    Map map = Map(seed, size, octaves, frequency, smooth, scale);

    //TESTING HEIGHT RETURN ON MAP
    float x = 5.0f;
    float z = 5.0f;
    //cout << "height on map at coords " << x << ", " << z << " is " << map.getHeight(x, z, mapScale) << endl;

    vector <glm::vec3> plantLocations = { glm::vec3(x, map.getHeight(z, x, mapScale), z) }; //for map height testing

    
   


    

    //[COORD SYS]
    //3D projection
    glm::mat4 model = glm::mat4(1.0f);
 
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f)/*FOV*/, 1200.0f / 800.0f/*aspect ratio*/, 0.1f, 1000.0f);

    glEnable(GL_DEPTH_TEST); //enable depth testing


    //Stores all plant locations in the world
    //vector <glm::vec3> plantLocations = { glm::vec3(0.0f, 0.0f, 0.0f) }; //original
    
    

    // render loop
    // -----------
    
    while (!glfwWindowShouldClose(window))
    {
        
        
        //the background color
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // old teal: 0.2f, 0.3f, 0.3f, 1.0f

        //clear the color and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
       
       
        /*++++NEWER draw with plant class AND plant mesh!++++++*/
        
        if (changeTree) {
            changeTree = false;
            seed += 1;
            plant = Plant(seed);
            plantVector[0] = plant;
            
        }
        if (changeGrowthStage) {
            changeGrowthStage = false;
            tempgs = growthStage;
        }
        if (addPlant) {
            addPlant = false;
            seed += 1;
            Plant newPlant = Plant(seed);
            plantVector.push_back(newPlant);
            int plindex =  plantLocations.size() - 1;
            float curX = plantLocations[plindex].x + 2.5;
            float curZ = plantLocations[plindex].z ; //can place along Z-axis! (
            float curY = map.getHeight(curZ, curX/*this is messing up*/, mapScale);
            plantLocations.push_back(glm::vec3(curX, curY, curZ ));
            
        }
        
        
        float timeValue = glfwGetTime();
       

        //for sway animation on plants
        ourShader.setFloat1("systime", timeValue);


        // camera/view transformation
        glm::mat4 view = ourCam.GetViewMatrix();
      
        

        /*Controlable Camera*/
        
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // input
        // -----
        processInput(window);

        glm::mat4 projection = glm::perspective(glm::radians(ourCam.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.001f, 1000.0f);
        
        //view = ourCam.GetViewMatrix();
        

        /*Rotating Camera*/
        if (rotateCamera) {
            radius;
            float spinSpeed = 0.33;
            float camX = sin(glfwGetTime()*spinSpeed) * radius;
            float camZ = cos(glfwGetTime()*spinSpeed) * radius;
            
            view = glm::lookAt(glm::vec3(camX, 40.0, camZ), glm::vec3(0.0, 19.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
            
        }
        
        int viewLoc = glGetUniformLocation(ourShader.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        int projectionLoc = glGetUniformLocation(ourShader.ID, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        
        
        /*DRAW ALL PLANTS IN WORLD*/
        float timeVar = 1.0f *  (float)glfwGetTime();
        
        for (unsigned int i = 0; i < plantLocations.size(); i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, plantLocations[i]);
            float angle = 0.0f;
                       
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            model = glm::scale(model, glm::vec3(scale, scale, scale));
            int modelLoc = glGetUniformLocation(ourShader.ID, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            
            plantVector[i].draw(tempgs, ourShader);
            //glBindVertexArray(VAO);
         
            //glDrawElements(GL_TRIANGLES, 6*numRectangles /*num vertices*/, GL_UNSIGNED_INT, 0);
            
        }
        /*Change Tree Color*/ //replace with genetics later on, this is just leftover fun stuff
        if (colorSetChange == 1) {
            if (colorSetVal == 13) {
                colorSetVal = 0;
            }
            else {
                colorSetVal += 1;
            }
            ourShader.setInt1("colorSet", colorSetVal);
            //cout << "colorSetVal set to " << colorSetVal << endl;
            colorSetChange = 0;
        }

        /*DRAW MAP*/
        //glBindVertexArray(0);
        
        mapShader.use();
        viewLoc = glGetUniformLocation(mapShader.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        projectionLoc = glGetUniformLocation(mapShader.ID, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 model = glm::mat4(1.0f);
        //model = glm::translate(model, treePositions[i]);
        model = glm::translate(model, glm::vec3(0.0, -map.hMin * mapScale, 0.0)); //move map back to XZ plane
        float angle = 0.0f;

        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        model = glm::scale(model, glm::vec3(mapScale, mapScale, mapScale));
        int modelLoc = glGetUniformLocation(mapShader.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //mapShader.use();
        map.draw(mapShader);
        



        

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    //glDeleteVertexArrays(1, &VAO);
    //glDeleteBuffers(1, &VBO);

    //clear dynamically allocated resources
    //delete[] vertices; 
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        mixValue += 0.001f; // change this value accordingly (might be too slow or too fast based on system hardware)
        if (mixValue >= 1.0f)
            mixValue = 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        mixValue -= 0.001f; // change this value accordingly (might be too slow or too fast based on system hardware)
        if (mixValue <= 0.0f)
            mixValue = 0.0f;
    }
    //CAMERA INPUT PROCESSING WITH CAMERA CLASS

    //"sprint" input logic
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        ourCam.MovementSpeed = 200.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
        ourCam.MovementSpeed = SPEED;
    }
   

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        ourCam.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        ourCam.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        ourCam.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        ourCam.ProcessKeyboard(RIGHT, deltaTime);
    
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS && gotKey == false) {
        numRectangles += 1;
        gotKey = true;
    }
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_RELEASE) {
        gotKey = false;
    }
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        numRectangles += 1;

    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        numRectangles = 0;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        numRectangles = numRectanglesMax;
    }
    
        


    //mouse input not handled in this function!!!
    
    
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        colorSetChange = 1;
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        if (rotateCamera == true) {
            rotateCamera = false;
        }
        else {
            rotateCamera = true;
        }
        
    }
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        radius += 1;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        radius -= 1;
    }
    //spawn new tree
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        changeTree = true;
        growthStage = 6;
        
        //iteration = 0; //reset growth
    }
    //spawn new tree
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        addPlant = true;
    }
    //increase growth 
    if (key == GLFW_KEY_N && action == GLFW_PRESS)
    {
        changeGrowthStage = true;
        if(growthStage > 0)
            growthStage -= 1;       
    }
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        changeGrowthStage = true;
        if(growthStage < 6)
            growthStage += 1;
    }
    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    ourCam.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ourCam.ProcessMouseScroll(static_cast<float>(yoffset));
}