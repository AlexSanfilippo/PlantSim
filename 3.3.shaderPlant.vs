#version 330 core
layout(location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aColor;
//layout (location = 1) in vec2 aTexCoord;

out vec3 ourColor;
//out vec2 TexCoord;

//my playing around
//uniform float offsetX;
//uniform float offsetY;

//uniform mat4 transform;//Transform. chapter


//Coord. Sys. chapter
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//cycle colors
uniform int colorSet;

//plant sway animation
uniform float systime;

//genetic plant colors
uniform vec3 plantColor;

void main()
{

   

    //Tree
    gl_Position = projection * view * model * vec4(aPos, 1.0); //camera --NO SWAY

    //float timeFactor = 0.6;
    ///float swaySeverity = 0.05;
    //float swayX = +swaySeverity * aPos.y * ((sin(systime * timeFactor) + 0) / 2);
    //float swayZ = +swaySeverity * aPos.y * ((sin(systime * timeFactor + 1) + 0) / 2);
    //gl_Position = projection * view * model * vec4(aPos.x + swayX, aPos.y, aPos.z + swayZ, 1.0); //camera + sway v2
    
    


    ourColor = plantColor;
}

