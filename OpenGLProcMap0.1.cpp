/*
* Current Issues: 
*               1) get OpelGL running in this new project
*               2) generate and load vertices of flat land mesh
*               3) generate height data and transform map
*               4) create water class
*               5) In-Program controls to raise and lower water level
*               
*Date:29 Jan, 2023
*Author: Alexander Sanfilippo
* Brief: This program uses openGL to create a random map with animated water
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

/*Map*/
#include <headers/map.h>

//function headers
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

//function headers-map part
glm::vec3 calcNormal(glm::vec3 a, glm::vec3 b, glm::vec3 c);
void colorMapNormals(float* vertices, int size);
void changeMapFxn(unsigned int size, unsigned int octaves, unsigned int frequency, float smooth, float scale);
void placeTriangle();


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


//MAP
float mapScale = 40.0;
float mapScaleY = mapScale;
bool changeMap = false;
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Map Generator V0.1 by Alexander Sanfilippo", NULL, NULL);
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
    Shader ourShader("3.3.shader.vs", "3.3.shader.fs");
    
    //Turtle turtle = Turtle(iteration, 1904); //create our initial turtle (and L-System)
    //cout << "calling trutle growthStageDraw\n";
    //turtle.growthStageDraw();
    //cout << "after trutle growthStageDraw\n";
    //turtle.draw3D(); //tell Turtle to generate vertices and indices   
    //cout << "turtle drew " << turtle.numVertices/3 << " triangles. \n";
    
    //get vertex info from the turtle and convert to CPP array
    //vector<float> vertices_vec = turtle.getVertices();

    unsigned int size = 120;
    unsigned int octaves = 5;
    float smooth = 2.5; //higher -> bumpier.  closer to 0 -> flatter
    int seed = 10363;
    unsigned int frequency = 2; //cannot be under 2
    int numMapVertices = size * size * 6;
    float scale = 1.0f;
    Map map = Map(seed, size, octaves, frequency, smooth, scale);
   
    //Wireframe or fill mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //GL_FILL FOR SOLID POLYGONS GL_LINE FOR WIREFRAME
    //chang provoking vertex
    //glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);

    vector <float> vertices_vec = map.getVertices();
    //vector<float> vertices_vec = turtle.getStageVertices(growthStage);
    
    //dynamically created array of vertices
    int verticesSize = vertices_vec.size();
    //cout << "in main(), verticesSize = " << verticesSize << endl;
    float* vertices = new float[verticesSize];
    //populate the vertices
    for (int i = 0; i < verticesSize; i++) {
        vertices[i] = vertices_vec[i];

    }

    /*Normalized Map Colors*/
    //colorMapNormals(vertices, verticesSize);

    /*
    //repeat for indices
    //vector<unsigned int> indices_vec = turtle.getIndicesOrdered();
    vector<unsigned int> indices_vec = turtle.getStageIndices(growthStage);
    int indicesSize = indices_vec.size();
    unsigned int* indices = new unsigned int[indicesSize];
    //populate the indices array
    for (int i = 0; i < indicesSize; i++) {
        indices[i] = indices_vec[i];

    }
    */

    //TP
    bool dispVertexInfo = false;
    if (dispVertexInfo) {
        cout << "from turtle we received these coordinates" << endl;
        for (int i = 0; i < vertices_vec.size(); i++) {
            cout << vertices[i] << ",";
            if (i % 6 == 5)
                cout << "\n";
        }
        /*
        cout << "from turtle we received these indices" << endl;
        for (int i = 0; i < indices_vec.size(); i++) {
            cout << indices[i] << ",";
            if (i % 3 == 2)
                cout << "\n";
        }
        */
    }
      
    
    //unsigned int elmsToRender = turtle.numVertices; //for growth animation and debugging
   // unsigned int elmsToRenderMax = turtle.numVertices; //nonchanging
    //unsigned int elmsToRender = verticesSize; //for growth animation and debugging
    //unsigned int elmsToRenderMax = verticesSize; //nonchanging

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    

    glBindVertexArray(VAO);
  
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0])* verticesSize, &vertices[0], GL_STATIC_DRAW);
    
    
    
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    /**/
    glVertexAttribPointer(1/*location*/, 3/*number of values*/, GL_FLOAT/*d_type*/, GL_FALSE, 6 * sizeof(float)/*size*/, (void*)(3 * sizeof(float))/*offset*/);
    glEnableVertexAttribArray(1);
   


    

    //[COORD SYS]
    //3D projection
    glm::mat4 model = glm::mat4(1.0f);
 
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f)/*FOV*/, 1200.0f / 800.0f/*aspect ratio*/, 0.1f, 100.0f);

    glEnable(GL_DEPTH_TEST); //enable depth testing


    //tree postions 
    /*
    glm::vec3 treePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(15.0f,  0.0f,  -20.0f),
    glm::vec3(30.0f,  0.0f,  -40.0f),
    glm::vec3(-15.0f,  0.0f,  -20.0f),
    glm::vec3(-30.0f,  0.0f,  -40.0f),
    };
    */
    
  

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        
        
        //the background color
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // old teal: 0.2f, 0.3f, 0.3f, 1.0f

        //clear the color and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*Spawn New Tree (Wrap into a function later on, this is messy*/   
        if (changeMap) {
            changeMapFxn(size, octaves, frequency, smooth, scale);
            changeMap = false;

            //placeTriangle(); //TP
           
        }
       
        

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
            elmsToRender = turtle.numStageVertices[growthStage]; //indicesSize; //turtle.numVertices;
            elmsToRenderMax = turtle.numStageVertices[growthStage];//indicesSize; //turtle.numVertices; //nonchanging

            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * verticesSize, &vertices[0], GL_DYNAMIC_DRAW);

            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indicesSize, &indices[0], GL_DYNAMIC_DRAW);

            //numRectangles = turtle.numVertices / 6;
            //numRectanglesMax = turtle.numVertices / 6;
            numRectangles = turtle.numStageVertices[growthStage]/6; //indicesSize/2;
            numRectanglesMax = turtle.numStageVertices[growthStage]/6; //indicesSize/2;
        }
        */

        //KEEP FOR WATER
        //float timeValue = glfwGetTime();
        //for sway animation on plants
        //ourShader.setFloat1("systime", timeValue);

        


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
            
            view = glm::lookAt(glm::vec3(camX, 40.0, camZ)/*Pos*/, glm::vec3(0.0, 19.0, 0.0)/*lookAt*/, glm::vec3(0.0, 1.0, 0.0));
            
        }
        int viewLoc = glGetUniformLocation(ourShader.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        int projectionLoc = glGetUniformLocation(ourShader.ID, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        
        
        glBindVertexArray(VAO);
        float timeVar = 1.0f *  (float)glfwGetTime();
        for (unsigned int i = 0; i < 1; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            //model = glm::translate(model, treePositions[i]);
            model = glm::translate(model, glm::vec3(0.0, -map.hMin*mapScaleY, 0.0)); //move map back to XZ plane
            float angle = 0.0f;
                       
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            model = glm::scale(model, glm::vec3(mapScale, mapScaleY, mapScale));
            int modelLoc = glGetUniformLocation(ourShader.ID, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            ourShader.use();
            glBindVertexArray(VAO);
         
           
            glDrawArrays(GL_TRIANGLES, 0, numMapVertices/*num vertices*/);
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
        changeMap = true;
       
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

/*changes the color of each triangle to its three points normal*/
void colorMapNormals(float *vertices, int size) {
    int i;
    for (int i = 0; i < size; i++) {

        glm::vec3 a, b, c;
        a.x = vertices[i + 3];
        a.y = vertices[i + 4];
        a.z = vertices[i + 5];
        b.x = vertices[i + 9];
        b.y = vertices[i + 10];
        b.z = vertices[i + 11];
        c.x = vertices[i + 15];
        c.y = vertices[i + 16];
        c.z = vertices[i + 17];

        glm::vec3 nColor = calcNormal(a, b, c);
        //cout << "nColor " << nColor.x << "," << nColor.y << "," << nColor.z << endl;
        vertices[i + 3] = nColor.x;
        vertices[i + 4] = nColor.y;
        vertices[i + 5] = nColor.z;
        vertices[i + 9] = nColor.x;
        vertices[i + 10] = nColor.y;
        vertices[i + 11] = nColor.z;
        vertices[i + 15] = nColor.x;
        vertices[i + 16] = nColor.y;
        vertices[i + 17] = nColor.z;

        
        i += 17;
        if (i > size) {
            break;
        }
    }
}
/*returns the normal of 3 vectors*/ //UNUSED
glm::vec3 calcNormal(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
    glm::vec3 u = b - a;
    glm::vec3 v = c - a;

    glm::vec3 normal;
    normal.x = 1*(u.y * v.z - u.z * v.y);
    normal.y = 1*(u.z * v.x - u.x * v.z);
    normal.z = 1*(u.x * v.y - u.y * v.x);

    
    //cout << "normal color is " << normal.x <<"," << normal.y <<"," << normal.z << endl;

    // Set Normal.x to(multiply U.y by V.z) minus(multiply U.z by V.y)
    //    Set Normal.y to(multiply U.z by V.x) minus(multiply U.x by V.z)
     //   Set Normal.z to(multiply U.x by V.y) minus(multiply U.y by V.x)

    return normal;

}
/*Draws a new map by overwriting the map vertices */
void changeMapFxn(unsigned int size, unsigned int octaves, unsigned int frequency, float smooth, float scale) {

    //create a new map
    Map map = Map(time(NULL), size, octaves, frequency, smooth, scale);

    vector <float> vertices_vec = map.getVertices();

    //dynamically created array of vertices
    int verticesSize = vertices_vec.size();
    //cout << "in main(), verticesSize = " << verticesSize << endl;
    float* vertices = new float[verticesSize];
    //populate the vertices
    for (int i = 0; i < verticesSize; i++) {
        vertices[i] = vertices_vec[i];
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * verticesSize, &vertices[0], GL_STATIC_DRAW);
}
/*Places a new object in the world by creating a new VBO*/
//for now, just place a single triangle
void placeTriangle() {

    float vertices[] = {
        -10.0, 0.0, 0.0, 1.0, 0.0 ,0.0,
        -5.0 , 0.0, 0.0, 0.0, 1.0, 0.0,
        -7.5 , 20.0, 0.0, 0.0, 0.0, 1.0
    };
    float verticesSize = 18;

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);


    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * verticesSize, &vertices[0], GL_STATIC_DRAW);



    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    /**/
    glVertexAttribPointer(1/*location*/, 3/*number of values*/, GL_FLOAT/*d_type*/, GL_FALSE, 6 * sizeof(float)/*size*/, (void*)(3 * sizeof(float))/*offset*/);
    glEnableVertexAttribArray(1);
}