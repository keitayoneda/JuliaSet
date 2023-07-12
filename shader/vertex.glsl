#version 330 core

layout(location=0) in vec3 vertexposition;
layout(location=1) in vec3 vertexColor;
out vec3 fragmentColor;
void main(){
  gl_Position = vec4(vertexposition,1.0);
  fragmentColor = vertexColor;
}
