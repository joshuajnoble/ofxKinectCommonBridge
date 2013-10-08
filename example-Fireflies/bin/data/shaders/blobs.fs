#version 150

uniform vec2      resolution;           // viewport resolution (in pixels)
uniform float     time;           // shader playback time (in seconds)

uniform vec2 pp1;
uniform vec2 pp2;
uniform vec2 pp3;

in float depth;
in vec4 colorVarying;
in vec2 texCoordVarying;
																		 
out vec4 fragColor;

uniform float frequency;
uniform float scalar;
uniform float blobDensity;

uniform vec2 headPoint;
uniform vec2 rHandPoint;
uniform vec2 lHandPoint;

uniform sampler2DRect videoTex;

float makePoint(float x,float y,float fx,float fy,float sx,float sy,float t){

    float fsx = fx * frequency;
    float fsy = fy * frequency;

    float ssx = sx * scalar;
    float ssy = sy * scalar;

   float xx=x+(sin(t*fsx)*ssx);
   float yy=y+(cos(t*fsy)*ssy);
   return 1.0/sqrt(xx*xx+yy*yy);
}

void main( void ) {

   vec2 tp = (texCoordVarying.xy/resolution.x) * 2.0 - vec2(1.0,resolution.y/resolution.x);

   vec2 p=tp+headPoint;
   vec2 p1=tp+lHandPoint;
   vec2 p2=tp+rHandPoint;
   
   float x=p.x;
   float y=p.y;

   float x1=p1.x;
   float y1=p1.y;

   float x2=p2.x;
   float y2=p2.y;

   float r = makePoint(x2,y2,1.3,2.9,0.3,0.3,time);
   r=r+makePoint(x2,y2,1.9,2.0,0.4,0.4,time);
   r=r+makePoint(x2,y2,0.8,0.7,0.4,0.5,time);
   r=r+makePoint(x2,y2,2.3,0.1,0.6,0.3,time);
   r=r+makePoint(x2,y2,0.8,1.7,0.5,0.4,time);
   r=r+makePoint(x2,y2,0.3,1.0,0.4,0.4,time);
   r=r+makePoint(x2,y2,1.4,1.7,0.4,0.5,time);
   r=r+makePoint(x2,y2,1.3,2.1,0.6,0.3,time);
   r=r+makePoint(x2,y2,1.8,1.7,0.5,0.4,time);  
   
   float g = makePoint(x,y,1.2,1.9,0.3,0.3,time);
   g=g+makePoint(x,y,0.7,2.7,0.4,0.4,time);
   g=g+makePoint(x,y,1.4,0.6,0.4,0.5,time);
   g=g+makePoint(x,y,2.6,0.4,0.6,0.3,time);
   g=g+makePoint(x,y,0.7,1.4,0.5,0.4,time);
   g=g+makePoint(x,y,0.7,1.7,0.4,0.4,time);
   g=g+makePoint(x,y,0.8,0.5,0.4,0.5,time);
   g=g+makePoint(x,y,1.4,0.9,0.6,0.3,time);
   g=g+makePoint(x,y,0.7,1.3,0.5,0.4,time);

   float b = makePoint(x1,y1,1.7,0.3,0.3,0.3,time);
   b=b+makePoint(x1,y1,1.9,1.3,0.4,0.4,time);
   b=b+makePoint(x1,y1,0.8,0.9,0.4,0.5,time);
   b=b+makePoint(x1,y1,1.2,1.7,0.6,0.3,time);
   b=b+makePoint(x1,y1,0.3,0.6,0.5,0.4,time);
   b=b+makePoint(x1,y1,0.3,0.3,0.4,0.4,time);
   b=b+makePoint(x1,y1,1.4,0.8,0.4,0.5,time);
   b=b+makePoint(x1,y1,0.2,0.6,0.6,0.3,time);
   b=b+makePoint(x1,y1,1.3,0.5,0.5,0.4,time);

   vec3 d=vec3(r,g,b) / (64.0 * blobDensity);
   
   vec4 blobColor = vec4(d.x,d.y,d.z,1.0);
   float interp = (d.x + d.y + d.z)/3.0;

   fragColor = blobColor;
   vec2 videoScaling = vec2(640/1024, 640/768);

   //fragColor = mix( texture(videoTex, texCoordVarying * videoScaling), blobColor, interp);
   //fragColor = texture(videoTex, texCoordVarying.xy);
}