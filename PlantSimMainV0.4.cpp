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

#include <algorithm> //for min/max
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
#include <headers/world.h>
#include <headers/meter.h>


#ifdef __cplusplus
extern "C" {
#endif

    __declspec(dllexport) int NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

#ifdef __cplusplus
}
#endif

//function headers
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

//MAP
float mapScale = 1200.0;

// stores how much we're seeing of either texture
float mixValue = 0.2f;
// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
/*[CAMERA] Create Camera Object*/
// camera
Camera ourCam = Camera(glm::vec3(mapScale/2, mapScale*0.7, mapScale/2));
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

//For spawning new tree - change name later
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
int seed = 1912; //seed rng 

//adding new plants
bool addPlant = false;

//TEMP
int tempgs = 6;


//SIMULATION TIME
bool pause = false; //stop lifecycle

//Plant Life 
const int MAX_PLANTS = 300;

//control rendering of meters
bool drawWaterMeter = false;

//identifying memory leak
bool noRain = false;


int main()
{
    /*SEED RNG*/
    srand(seed);
    //srand(time(NULL));


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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Plant Sim V0.3 by Alexander Sanfilippo", NULL, NULL);
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
    Shader mapShader("3.3.shaderMap.vs", "3.3.shaderMap.fs");
    Shader meterShader("3.3.shaderMeter.vs", "3.3.shaderMeter.fs");

    //WORLD SETUP
    World world = World(10/*size*/);

    //PLANT SETUP
    //TEMP: First Map starting location
    float x = mapScale / 2.0f;
    float z = mapScale / 2.0f;

    vector <Plant> plantVector;//holds all plants in world
    cout << "[main] create and emplace_back the first plant\n";
    plantVector.emplace_back(Plant(seed));
    cout << "[main]after\n";
    //Plant plant = Plant(seed); //creates turtle, lsystem, creates position vectors, basically everything
    plantVector[0].worldCell = world.getCellIndex(z, x, mapScale); //gives plant knowledge of its cell. Z BEFORE X
    cout << "PLACED FIRST PLANT AT CELL " << plantVector[0].worldCell << endl;
    //vector <Plant*> plantPtrVec = { &plant };
    
    
    //MAP SETUP
    unsigned int size = 100;
    unsigned int octaves = 8;
    float smooth = 2.5; //higher -> bumpier.  closer to 0 -> flatter
    int mapSeed = 117;   //10363 mountains //1904 nice variation //1997 mtn valley
    unsigned int frequency = 2; //cannot be under 2
    int numMapVertices = size * size * 6;
    float scale = 1.0f; //initial size of map model-should be 1 at all times!
    Map map = Map(mapSeed, size, octaves, frequency, smooth, scale);
    
    //PROVOKING VERTEX
    glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);

    //create meter(WIP)
    //Meter meter = Meter(world.cellVec[36].water, world.MAX_WATER);
    //update world
    world.calcMidpoints(mapScale, &map); //give cell meters their locations
    
    //give initial plant its height on map
    vector <glm::vec3> plantLocations = { glm::vec3(x, map.getHeight(z, x, mapScale), z) }; //for map height testing
    plantVector[plantVector.size() - 1].coordinates = glm::vec3(x, map.getHeight(z, x, mapScale), z);
    
   


    

    //[COORD SYS]
    //3D projection
    glm::mat4 model = glm::mat4(1.0f);
 
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f)/*FOV*/, 1200.0f / 800.0f/*aspect ratio*/, 0.1f, 1500.0f);

    glEnable(GL_DEPTH_TEST); //enable depth testing


    //Stores all plant locations in the world
    //vector <glm::vec3> plantLocations = { glm::vec3(0.0f, 0.0f, 0.0f) }; //original
    

    /*STATISTICS*/
    vector <int> plantCount; //how many plants are recorded 
    
    /*GAME LOOP COUNTERS*/
    //control the speed of things by only calling certain fxns every so many frames
    float plantEatCounter = 0;
    float plantReproduceCounter = 0;
    float worldUpdateCounter = 0;
    float statCounter = 0; //record statistics for graphing

    /*STATISTICS*/
    int plantPop = 0; //current number of plants in the world;
    int numSpecies = 0; //later

    float simSpeed = 0.2; //how fast our simulation runs.  closer to 0 is faster

    /*GAME LOOP UPDATE INTERVALS*/ //curently unused
    int plantIntervali = 0;    
    int plantInterval = 20;
    int plantIntervalf = min(plantInterval, (int)plantVector.size());

    int sexIndex = 0;
    // render loop
    // -----------
    
    while (!glfwWindowShouldClose(window))
    {
        
        
        //the background color
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // old teal: 0.2f, 0.3f, 0.3f, 1.0f

        //clear the color and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
       
       
       

        /*PLANT LIFECYCLE*/
        
        if (plantEatCounter > 100*simSpeed) {
            plantEatCounter = 0;
            //cout << "EAT" << endl;
            //for (int i = 0; i < plantVector.size(); i++) {
            int end;
            
            //cout << "looping plants from " << plantIntervali << " to " << plantIntervalf << endl;
            //for (int i = plantIntervali; i < plantIntervalf; i++) {
            for(int i = 0; i < plantVector.size(); i++){
                //world.lightLevel = 200; //fetch from WORLD CLASS later on
                plantVector[i].eat(world.lightLevel); //photosynthesize
                //plantVector[i].energy += lightLevel;
                //cout << "plant "<<i<< " energy: " << plantVector[i].energy << endl;
                

                //world interaction
                //drinking -add water to plant and remove it from world
                world.removeWater(plantVector[i].drink(world.cellVec[plantVector[i].worldCell].water), plantVector[i].worldCell); //update cell water for plant

                plantVector[i].metabolize(); //subtract resource cost for existing

                if (plantVector[i].checkDeath() == true) {
                    
                    Plant* plantPtr = &plantVector[i]; //get address of this dead plant

                    
                    plantPtr->turtle.cleanUp(); //delete VAO, VBO, EBO
                    //cout << "[main] checkDeath==true an plant Deconstr called\n";
                    plantPtr->~Plant(); //delete the plant from memory!
                    //delete &plantPtr;
                    plantVector.erase(plantVector.begin() + i);
                    if (i >= plantVector.size()) {
                        i -= 1;
                    }
                    
                    //i -= 1;
                    //cout << "after death i = " << i <<" numPlants="<<plantVector.size() << endl;
                    if (plantVector.size() == 0) {
                        cout << "ALL DEAD PAUSING SIM\n";
                        
                        pause = true;
                        break;
                    }
                }
                plantVector[i].checkGrowth();
                plantVector[i].ageUp();
                plantIntervali += plantInterval;
                plantIntervalf += plantInterval;
                if (plantIntervalf > plantVector.size()) {
                    plantIntervalf = (int) plantVector.size();
                    if (plantIntervali > plantVector.size()){
                        plantIntervalf = plantInterval;
                        plantIntervali = 0;
                    }
                    //plantIntervali = 0;
                }
                
            }
            
            plantEatCounter = 0;
        }
        
        if (plantReproduceCounter > 3000*simSpeed && pause == false) {
            //cout << "SEX & DEATH" << endl;
            for (int i = 0; i < plantVector.size(); i++) {
            //for (int i = 0; i < plantVector.size(); i++) {
               
                if (plantVector[i].checkReproduce() == 1 && plantVector.size() < MAX_PLANTS) {
                    
                    //Plant child(true); //create an empty plant
                    //cout << "before add child to plantVec\n";
                    plantVector.emplace_back(Plant(true));
                    //cout << "after add child to plantVec\n";
                    //plantVector[i].asexualReproduction(&child, mapScale); //populate child with parents genetics
                    plantVector[i].asexualReproduction(&plantVector[plantVector.size()-1], mapScale); //populate child with parents genetics
                    
                    //try this: create new simple plant and mutate rather than copy construct
                    //see if lack of copy constructor is issue, of if issue in mutate

                    //Plant child;
                    //child.coordinates.x = plantVector[i].coordinates.x + 20;
                    //child.coordinates.z = plantVector[i].coordinates.z;
                    //child.mutate(seed);

                    //check if child is outside of map 
                    //cout << "child placed at coordinates " << child.coordinates.x << "," << child.coordinates.z << endl;

                    /*
                    if (child.coordinates.x < 0 || child.coordinates.z < 0 || child.coordinates.x > mapScale*0.99
                        || child.coordinates.z > mapScale*0.99) {
                        
                        cout << "WARNING: CHILD z="<< child.coordinates.z <<", x=" << child.coordinates.x << "OUTSIDE OF BOUNDS\n";
                        
                        child.~Plant(); //destroy this child
                    }
                    */

                    //else {
                        //calculate the map height of the child
                        
                        //float childY = map.getHeight(child.coordinates.z, child.coordinates.x, mapScale);
                        //child.coordinates.y = childY;
                        float childY = map.getHeight(plantVector[plantVector.size() - 1].coordinates.z, plantVector[plantVector.size() - 1].coordinates.x, mapScale);
                        plantVector[plantVector.size() - 1].coordinates.y = childY;

                        //give child its world cell
                       
                        //child.worldCell = world.getCellIndex(child.coordinates.z, child.coordinates.x, mapScale);
                        plantVector[plantVector.size() - 1].worldCell = world.getCellIndex(plantVector[plantVector.size() - 1].coordinates.z, plantVector[plantVector.size() - 1].coordinates.x, mapScale);

                        //child's one chance to drink before death
                        //world.removeWater(child.drink(world.cellVec[child.worldCell].water), child.worldCell);
                        world.removeWater(plantVector[plantVector.size() - 1].drink(world.cellVec[plantVector[plantVector.size() - 1].worldCell].water), plantVector[plantVector.size() - 1].worldCell);

                        
                        //plantVector.push_back(child);
                        
                    //}
                    
                    
                }
                
            }
            plantReproduceCounter = 0;
            
        }

        /*WORLD UPDATE*/
        if (worldUpdateCounter > 1001*simSpeed) {
            world.update();
            worldUpdateCounter = 0;
        }
        if (pause == false) {
            plantEatCounter += 1;
            plantReproduceCounter += 1;
            worldUpdateCounter += 1;
            statCounter += 1;
        }
        if (statCounter > 2502) {
            plantPop = plantVector.size();
            cout << "plantPop: " << plantPop << endl;
            statCounter = 0;
        }
        //
        if (noRain) {
            cout << "no rain :(\n";
            world.rainChance = 0.0f;
            noRain = false;
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

        glm::mat4 projection = glm::perspective(glm::radians(ourCam.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.001f, 1500.0f);
        
        //view = ourCam.GetViewMatrix();
        

        /*Rotating Camera*/
        if (rotateCamera) {
            radius;
            float spinSpeed = 0.33;
            float camX = sin(glfwGetTime()*spinSpeed) * radius;
            float camZ = cos(glfwGetTime()*spinSpeed) * radius;
            
            view = glm::lookAt(glm::vec3(camX, 240.0, camZ), glm::vec3(600.0, 19.0, 600.0), glm::vec3(0.0, 1.0, 0.0));
            
        }
        
        int viewLoc = glGetUniformLocation(ourShader.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        int projectionLoc = glGetUniformLocation(ourShader.ID, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        
        
        /*DRAW ALL PLANTS IN WORLD*/
        float timeVar = 1.0f *  (float)glfwGetTime();
        
        for (unsigned int i = 0; i < plantVector.size(); i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            //model = glm::translate(model, plantLocations[i]);
            model = glm::translate(model, plantVector[i].coordinates);
            float angle = 0.0f;
                       
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            model = glm::scale(model, glm::vec3(scale, scale, scale));
            int modelLoc = glGetUniformLocation(ourShader.ID, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glm::vec3 plantColor = plantVector[i].color;
            int plantColorLoc = glGetUniformLocation(ourShader.ID, "plantColor");
            glUniform3fv(plantColorLoc, 1, &plantColor[0]);
            
            plantVector[i].draw(plantVector[i].growthStage, ourShader);
            //glBindVertexArray(VAO);
            
            //glDrawElements(GL_TRIANGLES, 6*numRectangles /*num vertices*/, GL_UNSIGNED_INT, 0);
            
        }
        //ourShader.use();
        //
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
        

        /*++++DRAWING METER++++*/
        if (drawWaterMeter) {
            meterShader.use();
            world.drawMeters(meterShader, &ourCam, (float)SCR_WIDTH, (float)SCR_HEIGHT);
        }
        


        

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
        noRain = true;
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
    //pause simulation time with key P
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        if (pause == false) {
            pause = true;
        }
        else {
            pause = false;
        }
        
    }
    //toggle display cell water meter
    if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        if (drawWaterMeter == false) {
            drawWaterMeter = true;
        }
        else {
            drawWaterMeter = false;
        }

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