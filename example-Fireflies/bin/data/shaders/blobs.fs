#version 150

uniform vec2      resolution;           // viewport resolution (in pixels)
uniform float     time;           // shader playback time (in seconds)

in float depth;
in vec4 colorVarying;
in vec2 texCoordVarying;
																		 
out vec4 fragColor;

uniform float frequency;
uniform float scalar;
uniform float blobDensity;

uniform vec3 headPoint;
uniform vec3 rHandPoint;
uniform vec3 lHandPoint;

uniform sampler2DRect videoTex;

float makePoint( float x, float y, float fx, float fy, float sx, float sy, float scale, float freq)
{
    float fsx = fx * freq;
    float fsy = fy * freq;

    float ssx = sx * scale;
    float ssy = sy * scale;

   float xx=x+(sin(time*fsx)*ssx);
   float yy=y+(cos(time*fsy)*ssy);
   return 1.0/sqrt(xx*xx+yy*yy);
}

void main( void ) {

   vec2 tp = (texCoordVarying.xy/resolution.x) * 2.0 - vec2(1.0,resolution.y/resolution.x);

   vec2 hp=tp+headPoint.xy;
   vec2 lp=tp+lHandPoint.xy;
   vec2 rp=tp+rHandPoint.xy;

   float r = makePoint(rp.x,rp.y,1.3,2.9,0.3,0.3,rHandPoint.z, 3.0 - (rHandPoint.z * 3.0));
   r=r+makePoint(rp.x,rp.y,1.9,2.0,0.4,0.4, rHandPoint.z , 3.0 - (rHandPoint.z * 3.0));
   r=r+makePoint(rp.x,rp.y,0.8,0.7,0.4,0.5,rHandPoint.z, 3.0 - (rHandPoint.z * 3.0));
   r=r+makePoint(rp.x,rp.y,2.3,0.1,0.6,0.3,rHandPoint.z, 3.0 - (rHandPoint.z * 3.0));
   r=r+makePoint(rp.x,rp.y,0.8,1.7,0.5,0.4,rHandPoint.z, 3.0 - (rHandPoint.z * 3.0));
   r=r+makePoint(rp.x,rp.y,0.3,1.0,0.4,0.4,rHandPoint.z, 3.0 - (rHandPoint.z * 3.0));
   r=r+makePoint(rp.x,rp.y,1.4,1.7,0.4,0.5,rHandPoint.z, 3.0 - (rHandPoint.z * 3.0));
   r=r+makePoint(rp.x,rp.y,1.3,2.1,0.6,0.3,rHandPoint.z, 3.0 - (rHandPoint.z * 3.0));
   r=r+makePoint(rp.x,rp.y,1.8,1.7,0.5,0.4,rHandPoint.z, 3.0 - (rHandPoint.z * 3.0));  

      float g = makePoint(hp.x,hp.y,1.2,1.9,0.3,0.3,headPoint.z, 3.0 - (headPoint.z * 3.0));
   g=g+makePoint(hp.x,hp.y,0.7,2.7,0.4,0.4,headPoint.z, 3.0 - (headPoint.z * 3.0));
   g=g+makePoint(hp.x,hp.y,1.4,0.6,0.4,0.5,headPoint.z, 3.0 - (headPoint.z * 3.0));
   g=g+makePoint(hp.x,hp.y,2.6,0.4,0.6,0.3,headPoint.z, 3.0 - (headPoint.z * 3.0));
   g=g+makePoint(hp.x,hp.y,0.7,1.4,0.5,0.4,headPoint.z, 3.0 - (headPoint.z * 3.0));
   g=g+makePoint(hp.x,hp.y,0.7,1.7,0.4,0.4,headPoint.z, 3.0 - (headPoint.z * 3.0));
   g=g+makePoint(hp.x,hp.y,0.8,0.5,0.4,0.5,headPoint.z, 3.0 - (headPoint.z * 3.0));
   g=g+makePoint(hp.x,hp.y,1.4,0.9,0.6,0.3,headPoint.z, 3.0 - (headPoint.z * 3.0));
   g=g+makePoint(hp.x,hp.y,0.7,1.3,0.5,0.4,headPoint.z, 3.0 - (headPoint.z * 3.0));

     float b = makePoint(lp.x,lp.y,1.7,0.3,0.3,0.3,lHandPoint.z, 3.0 - (lHandPoint.z * 3.0));
   b=b+makePoint(lp.x,lp.y,1.9,1.3,0.4,0.4,lHandPoint.z, 3.0 - (lHandPoint.z * 3.0));
   b=b+makePoint(lp.x,lp.y,0.8,0.9,0.4,0.5,lHandPoint.z, 3.0 - (lHandPoint.z * 3.0));
   b=b+makePoint(lp.x,lp.y,1.2,1.7,0.6,0.3,lHandPoint.z, 3.0 - (lHandPoint.z * 3.0));
   b=b+makePoint(lp.x,lp.y,0.3,0.6,0.5,0.4,lHandPoint.z, 3.0 - (lHandPoint.z * 3.0));
   b=b+makePoint(lp.x,lp.y,0.3,0.3,0.4,0.4,lHandPoint.z, 3.0 - (lHandPoint.z * 3.0));
   b=b+makePoint(lp.x,lp.y,1.4,0.8,0.4,0.5,lHandPoint.z, 3.0 - (lHandPoint.z * 3.0));
   b=b+makePoint(lp.x,lp.y,0.2,0.6,0.6,0.3,lHandPoint.z, 3.0 - (lHandPoint.z * 3.0));
   b=b+makePoint(lp.x,lp.y,1.3,0.5,0.5,0.4,lHandPoint.z, 3.0 - (lHandPoint.z * 3.0));

   vec3 d=vec3(r,g,b) / (64.0 * blobDensity);
   
   vec4 blobColor = vec4(d.x,d.y,d.z,1.0);
   float interp = (d.x + d.y + d.z)/3.0;

   vec2 videoScaling = vec2(640.0/1024.0, 480.0/768.0);

   vec4 vidColor = vec4(texture(videoTex, texCoordVarying.xy * videoScaling).rgb, 1.0);
   fragColor = mix( vidColor, blobColor, interp);
   //fragColor = vec4(texture(videoTex, texCoordVarying.xy * videoScaling).rgb, 1.0);
}