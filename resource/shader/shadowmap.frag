#version 330 core

// Ouput data
//layout(location = 0) out float fragmentdepth;


void main(){
	gl_FragDepth = gl_FragCoord.z;
}