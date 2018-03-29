#version 330

in  vec3  normalView;
in  vec3  tangentView;
in  vec3  eyeView;
in  vec2  uvcoord;

uniform sampler2D colormap;
uniform sampler2D normalmap;
uniform vec3      light;

out vec4 outBuffer;

// used for the last question
vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

// used for the last question
float random(vec3 seed, int i){
  vec4 seed4 = vec4(seed,i);
  float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
  return fract(sin(dot_product) * 43758.5453);
}

// simple normal mapping 
vec3 getModifiedNormal() {
  vec3 n   = normalize(normalView);
  vec3 t   = normalize(tangentView);
  vec3 b   = normalize(cross(n,t));
  mat3 tbn = mat3(t,b,n);
  vec3 tn  = normalize(texture(normalmap,uvcoord).xyz*2.0-vec3(1.0));
  
  return normalize(tbn*tn);
}

void main() {
  vec3 n = getModifiedNormal();
  vec3 e = normalize(eyeView);
  vec3 l = normalize(light);
  vec4 c = texture(colormap,uvcoord);

  float diff = max(dot(l,n),0.0);
  float spec = pow(max(dot(reflect(l,n),e),0.0),20.0);

  vec4 color = c*(diff+spec);

  float v = 1.0;

  // *** TODO: compute visibility by comparing current depth and the depth in the shadow map ***


  outBuffer  = color*v;
}
