#version 150

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 textureMatrix;
uniform vec4 globalColor = vec4(1.0);
uniform	sampler2DRect tex0;			// we use this to sample depth data for our boxes.
uniform	sampler2DRect tex1;			// we use this to sample color data for our boxes.
uniform float timeValue=0.0;	// we get this from our testApp, it's a time value moving from 0 to 1 every 30 seconds


in vec4  position;
in vec2  texcoord;
in vec4  color_coord;
in vec3  normal;

out vec4 colorVarying;		// we use flat colors, and, more importantly, flat normals, since we want crisp, flat shaded surfaces.
out vec2 texCoordVarying;

const float dxRatio = 320.0/1024.0;
const float dyRatio = 240.0/768.0;

const float cxRatio = 640.0/1024.0;
const float cyRatio = 480.0/768.0;

// ---------------------------------------------------------------------- 

void main()
{

	float instanceX = position.x;
	float instanceY = position.y;
	
	
	vec2 vv = vec2(instanceX * dxRatio, 240 - (instanceY * dyRatio));
	float pixelDepth = (texture(tex0, vv).x - 0.3) * 1.5;

	vec2 vv2 = vec2(instanceX * cxRatio, 480 - (instanceY * cyRatio));
	colorVarying = texture(tex1, vv2);
	
	vec4 vPos = position;
	
	// extrude our primitive along the z coordinate, based on current PixelDepth.
	vPos.z = (vPos.z) * (pixelDepth * 20.0) * 1.2;
	vPos.z = vPos.z + (sin( (instanceX/50.0) + timeValue ) * 10.0);

	texCoordVarying = (textureMatrix*vec4(texcoord.x, 240 - texcoord.y,0,1)).xy;

	gl_Position = projectionMatrix * modelViewMatrix * vPos;
}