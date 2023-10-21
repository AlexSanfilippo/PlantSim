#version 330 core
out vec4 FragColor;

in vec3 ourColor; // the input variable from the vertex shader (same name and same type)  
//in vec2 TexCoord;

//uniform vec4 uniColor;
//uniform float pulse; 
//uniform float systime;

//texture
//uniform sampler2D ourTexture;

//uniform sampler2D texture1;
//uniform sampler2D texture2;

//texture visability
//uniform float mixValue;

void main()
{
    //FragColor = vec4(uniColor, uniColor, uniColor, 1.0);
    //FragColor = vec4(.9f, .2f, .3f, 1.0f);

    //---using uniform
    //FragColor = uniColor;

    //--original output
    //FragColor = vec4(ourColorB, 1.0f);

    //pulsing
    //FragColor = vec4(ourColorB.r + pulse, ourColorB.g + pulse, ourColorB.b + pulse, 1.0f);


    //simple fractals

    //float angle = systime*0.15;
    //vec2 coords = vec2(ourColor.x, ourColor.y);
    //for(float i = 0.0; i < 32.0; i+=1.0){
    //coords = abs(coords);
    //coords -= 0.5;
    //coords *= 1.05;
    //coords *= mat2( cos(angle), -sin(angle), sin(angle), cos(angle) );
    //}
    //FragColor = vec4(length(coords + vec2(0.1, 0.25)), length(coords + vec2(-0.34, 0.77)), length(coords + vec2(-0.2, 0.432)), 1.0);


    //texture
    //FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);;
    //2 textures 

    //FragColor = mix(texture(texture1, vec2(0.1*(1.0 - TexCoord.x), 0.1*TexCoord.y)), texture(texture2, vec2(2*(1.0 - TexCoord.x), 2*TexCoord.y)), 0.2);

    //change between 2 textures with user input 
    //FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), mixValue);


    //--original output
    FragColor = vec4(ourColor, 1.0f);
}