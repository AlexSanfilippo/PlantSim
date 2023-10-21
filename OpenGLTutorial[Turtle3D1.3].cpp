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

bool rotateCamera = true;

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

//TEMP
int tempgs = 5;

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tree Generator V1.0 by Alexander Sanfilippo", NULL, NULL);
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
    
    TurtlePlant turtle = TurtlePlant(iteration, seed); //create our initial turtle (and L-System)
    //cout << "calling trutle growthStageDraw\n";
    turtle.growthStageDraw();
    turtle.createMeshes();//consider adding to TurtlePlant constructor
    //cout << "after trutle growthStageDraw\n";
    //turtle.draw3D(); //tell Turtle to generate vertices and indices   
    cout << "turtle drew " << turtle.numVertices/3 << " triangles. \n";
    
    //get vertex info from the turtle and convert to CPP array
    //vector<float> vertices_vec = turtle.getVertices();
    vector<float> vertices_vec = turtle.getStageVertices(growthStage);
    
    //dynamically created array of vertices
    int verticesSize = vertices_vec.size();
    float* vertices = new float[verticesSize];
    //populate the vertices
    for (int i = 0; i < verticesSize; i++) {
        vertices[i] = vertices_vec[i];

    }
    //repeat for indices
    //vector<unsigned int> indices_vec = turtle.getIndicesOrdered();
    vector<unsigned int> indices_vec = turtle.getStageIndices(growthStage);
    int indicesSize = indices_vec.size();
    unsigned int* indices = new unsigned int[indicesSize];
    //populate the indices array
    for (int i = 0; i < indicesSize; i++) {
        indices[i] = indices_vec[i];

    }


    //TP
    bool dispVertexInfo = false;
    if (dispVertexInfo) {
        cout << "from turtle we received these coordinates" << endl;
        for (int i = 0; i < vertices_vec.size(); i++) {
            cout << vertices[i] << ",";
            if (i % 3 == 2)
                cout << "\n";
        }
        cout << "from turtle we received these indices" << endl;
        for (int i = 0; i < indices_vec.size(); i++) {
            cout << indices[i] << ",";
            if (i % 3 == 2)
                cout << "\n";
        }

    }
      
    
    unsigned int elmsToRender = indicesSize / 3.0;
    unsigned int elmsToRenderMax = indicesSize / 3.0;

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
  
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0])* verticesSize, &vertices[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) * indicesSize, &indices[0], GL_STATIC_DRAW); //works with small vertices
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0])* indicesSize * 1, &indices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT/*type*/, GL_FALSE, 3 * sizeof(float)/*stride was 3*/, (void*)0)/*offset*/;
    glEnableVertexAttribArray(0);
     
   


    

    //[COORD SYS]
    //3D projection
    glm::mat4 model = glm::mat4(1.0f);
 
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f)/*FOV*/, 1200.0f / 800.0f/*aspect ratio*/, 0.1f, 100.0f);

    glEnable(GL_DEPTH_TEST); //enable depth testing


    //tree postions 
    glm::vec3 treePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(15.0f,  0.0f,  -20.0f),
    glm::vec3(30.0f,  0.0f,  -40.0f),
    glm::vec3(-15.0f,  0.0f,  -20.0f),
    glm::vec3(-30.0f,  0.0f,  -40.0f),
    };

    
    unsigned int placeSpeed = 1;
    unsigned int placeRectangles = 0;
    //numRectangles = verticesSize;//turtle.numVertices / 6;
    //numRectanglesMax = verticesSize; //turtle.numVertices / 6;
    numRectangles = indicesSize / 6.0;
    numRectanglesMax = indicesSize / 6.0;


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        
        
        //the background color
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // old teal: 0.2f, 0.3f, 0.3f, 1.0f

        //clear the color and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

       
        /*Animation logic*/
        if (doAnimate) {
           
            if (scale < 0.999) {
                scale += 0.0001;
                
            }

            /*
            if (numRectangles < numRectanglesMax) {
                numRectangles += (max((int)numRectanglesMax/10000, 1));
            }
            */
            if (placeRectangles == 5) {
                /*
                if (numRectanglesMax - numRectangles >= 1 * placeSpeed) {
                    numRectangles += 1 * placeSpeed;
                }
                else {
                    numRectangles = numRectanglesMax;
                }
                */
                //placeSpeed += 1;
                numRectangles = pow(scale, 2) * numRectanglesMax;
                placeRectangles = 0;
            }
            else {
                placeRectangles += 1;
            }
            
            if (scale > 0.999 && numRectangles >= numRectanglesMax) {
                numRectangles = numRectanglesMax; //correct for over draw
                
                doAnimate = false;
            }

            //SLOW no-scaling animation
        }

        /*======NEW draw via plant mesh draw=====*/
        turtle.draw(tempgs, ourShader);
        if (changeTree) {
            seed += 1;
            turtle = TurtlePlant(iteration, seed); //create our initial turtle (and L-System)
               
            turtle.growthStageDraw();
            turtle.createMeshes();//consider adding to TurtlePlant constructor
            changeTree = false;
        }
        if (changeGrowthStage) {
            changeGrowthStage = false;
            tempgs = growthStage; 
        }

        /*Spawn New Tree (Wrap into a function later on, this is messy*/
        /*
        if (changeTree) {
            cout << "CHANGING TREE\n";
            growthStage = 6;
            changeTree = false;
            
            seed += 1; //replaces time(NULL)
            turtle = TurtlePlant(iteration, seed); //create our turtle
            //tell turtle to generate vertices and index information
            //turtle.draw3D();
            turtle.growthStageDraw(); //consider adding to TurtlePlant constructor
            turtle.createMeshes();//consider adding to TurtlePlant constructor
            
            cout << "turtle drew " << turtle.numVertices / 3 << " triangles. \n";

            //get vertex info from the turtle and convert to CPP array
            vector<float> vertices_vec = turtle.getStageVertices(6);

            float verticesSize = vertices_vec.size();
            float* vertices = new float[verticesSize];
            //populate the vertices
            for (int i = 0; i < verticesSize; i++) {
                vertices[i] = vertices_vec[i];

            }
            //repeat for indices
            //NEW Indices dynamic size
            vector<unsigned int> indices_vec = turtle.getStageIndices(6);
            float indicesSize = indices_vec.size();
            unsigned int* indices = new unsigned int[indicesSize];
            //populate the vertices
            for (int i = 0; i < indicesSize; i++) {
                indices[i] = indices_vec[i];

            }

            //elmsToRender = turtle.numStageVertices[growthStage]; //indicesSize; //turtle.numVertices;
            //elmsToRenderMax = turtle.numStageVertices[growthStage];//indicesSize; //turtle.numVertices; //nonchanging
            elmsToRender = indicesSize/3.0; 
            elmsToRenderMax = indicesSize / 3.0;

            //try to clear out the buffers-I don't beleive this is required
            //glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
            //glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
            //glBindVertexArray(VAO);
            

            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * verticesSize, &vertices[0], GL_DYNAMIC_DRAW);

            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indicesSize, &indices[0], GL_DYNAMIC_DRAW);

            //numRectangles = verticesSize;//turtle.numVertices / 6;
            //numRectanglesMax = verticesSize; //turtle.numVertices / 6;
            //numRectangles = turtle.numStageVertices[growthStage] / 6; //indicesSize/2;
            //numRectanglesMax = turtle.numStageVertices[growthStage] / 6; //indicesSize/2
            numRectangles = indicesSize / 6.0;
            numRectanglesMax = indicesSize / 6.0;
        }
        */
        /*
        if (changeGrowthStage) {
            changeGrowthStage = false;
            cout << "change growth stage to " << growthStage << endl;

            //Turtle turtle = Turtle(iteration); //create our turtle
            //tell turtle to generate vertices and index information
            //turtle.draw3D();
            //turtle.growthStageDraw();

            //cout << "turtle drew " << turtle.numVertices / 3 << " triangles. \n";

            //get vertex info from the turtle and convert to CPP array
            vector<float> vertices_vec = turtle.getStageVertices(growthStage);

            float verticesSize = vertices_vec.size();
            float* vertices = new float[verticesSize];
            //populate the vertices
            for (int i = 0; i < verticesSize; i++) {
                vertices[i] = vertices_vec[i];

            }
            //repeat for indices
            //NEW Indices dynamic size
            vector<unsigned int> indices_vec = turtle.getStageIndices(growthStage);
            float indicesSize = indices_vec.size();
            unsigned int* indices = new unsigned int[indicesSize];
            //populate the vertices
            for (int i = 0; i < indicesSize; i++) {
                indices[i] = indices_vec[i];

            }

            //elmsToRender = turtle.numVertices;
            //elmsToRenderMax = turtle.numVertices; //nonchanging
            elmsToRender = indicesSize / 3.0;
            elmsToRenderMax = indicesSize / 3.0;

            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * verticesSize, &vertices[0], GL_DYNAMIC_DRAW);

            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indicesSize, &indices[0], GL_DYNAMIC_DRAW);

            //numRectangles = turtle.numVertices / 6;
            //numRectanglesMax = turtle.numVertices / 6;
            numRectangles = indicesSize / 6.0;
            numRectanglesMax = indicesSize / 6.0;
        }
        */
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

        glm::mat4 projection = glm::perspective(glm::radians(ourCam.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.001f, 100.0f);
        
        view = ourCam.GetViewMatrix();
        

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
        
        
        //glBindVertexArray(VAO);
        float timeVar = 1.0f *  (float)glfwGetTime();
        for (unsigned int i = 0; i < 1; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, treePositions[i]);
            float angle = 0.0f;
                       
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            model = glm::scale(model, glm::vec3(scale, scale, scale));
            int modelLoc = glGetUniformLocation(ourShader.ID, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            ourShader.use();
            //glBindVertexArray(VAO);
         
            //glDrawElements(GL_TRIANGLES, 6*numRectangles /*num vertices*/, GL_UNSIGNED_INT, 0);
            
        }
        
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

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    //clear dynamically allocated resources
    delete[] vertices; 
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
        ourCam.MovementSpeed = 8.0f;
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
    //animate tree growth by increase scale over time
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        scale = 0.000;  //
        
        doAnimate = true;
        numRectangles = 0;
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