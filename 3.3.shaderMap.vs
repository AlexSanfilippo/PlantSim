#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;


flat out vec3 ourColor;


//Coord. Sys. chapter
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;



void main()
{



    //Land Map
    gl_Position = projection * view * model * vec4(aPos, 1.0); //camera --NO SWAY
    //gl_Position = vec4(aPos, 1.0f);
    ourColor = aColor;
    //ourColor = vec3(aPos.y/2, aPos.y, aPos.y/2);


}
