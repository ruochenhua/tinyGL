#version 450 compatibility

in vec3 uv;
out vec4 FragColor;

uniform samplerCube skybox;

void main(){

	FragColor = textureLod(skybox, uv, 0); //vec4(1,0.5,0.5,1);
}