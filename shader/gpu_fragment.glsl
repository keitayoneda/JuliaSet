#version 330 core

out vec3 color;

uniform vec2 c;
uniform float window_width;
uniform float window_height;
uniform vec2 boundMin;
uniform vec2 boundMax;
int maxIterations = 1000;

float map(float value, float min1, float max1, float min2, float max2){
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

float calcGradient(float x, float y, int maxIterations){
 float gradient = 1.0;
 for (int i = 0; i < maxIterations; i++){
   if (x * x + y * y > 4){
     gradient = float(i) / float(maxIterations);
     break;
   }
   float xtemp = x * x - y * y + c.x;
   float ytemp = 2 * x * y + c.y;
   x = xtemp;
   y = ytemp;
 }
 return gradient;
}

void main(){
  float x = map(gl_FragCoord.x, 0, window_width, boundMin.x, boundMax.x);
  float y = map(gl_FragCoord.y, 0, window_height, boundMax.y, boundMin.y);

  float gradient = calcGradient(x, y, maxIterations);
  color = vec3(-gradient*(gradient-1.0)*2, gradient*(2.0-gradient), gradient);
}
