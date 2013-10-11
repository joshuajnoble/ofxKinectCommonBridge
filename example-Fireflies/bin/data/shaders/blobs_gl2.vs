	
/*#version 150

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;

in vec4  position;
in vec2  texcoord;
in vec4  color;
in vec3  normal;
 
out vec4 colorVarying;
out vec2 texCoordVarying;
out vec4 normalVarying;

uniform sampler2DRect videoTex;

void main()
{
	colorVarying = color;
	texCoordVarying = (textureMatrix*vec4(texcoord.x,texcoord.y,0,1)).xy;
	gl_Position = modelViewProjectionMatrix * position;
}*/

#extension GL_ARB_texture_rectangle : enable

//uniform mat4 projectionMatrix;
//uniform mat4 modelViewMatrix;
//uniform mat4 textureMatrix;
//uniform mat4 modelViewProjectionMatrix;

//in vec4  position;
//in vec2  texcoord;
//in vec4  color;
//in vec3  normal;
 
varying vec4 colorVarying;
varying vec2 texCoordVarying;

uniform sampler2DRect videoTex;

void main()
{
	//colorVarying = color;
	//texCoordVarying = (textureMatrix*vec4(texcoord.x,texcoord.y,0,1)).xy;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
