#version 150

uniform float exposure = 0.79;
uniform float decay = 0.01;
uniform float density = 0.26;
uniform float weight = 1.22;

uniform mat4 camModelViewProjectionMatrix;

//const vec4 lightPosition = vec4(1024, 768, 100, 1);
const vec4 lightPosition = vec4(0, 0, 0, 1);

uniform sampler2DRect rtex;
uniform sampler2DRect otex;
uniform sampler2DRect dtex;

const int NUM_SAMPLES = 30;

in vec4 colorVarying;
in vec2 texCoordVarying;
in vec4 normalVarying;

out vec4 fragColor;

void main()
{   

	vec4 ov = texture(otex, texCoordVarying);

	// if we're trying to do real camera move
	//vec4 tvec = vec4(texCoordVarying, 0, 1) + lightPosition;
	//vec2 camTextCoordVarying = ( camModelViewProjectionMatrix * tvec).xy * .5;
	//vec4 lightPositionOnScreen = camModelViewProjectionMatrix * lightPosition;
	//vec2 textCoord = camTextCoordVarying;

	vec2 lightPositionOnScreen = lightPosition.xy;
	vec2 textCoord = texCoordVarying;

	vec2 deltaTextCoord = vec2( texCoordVarying - lightPositionOnScreen.xy );
	deltaTextCoord *= 1.0 /  float(NUM_SAMPLES) * density;
	float illuminationDecay = 1.0;

	for(int i=0; i < NUM_SAMPLES ; i++)
	{
			textCoord -= deltaTextCoord;
			vec4 sample = texture(rtex, textCoord);
			sample *= illuminationDecay * weight;
			fragColor += sample;
			illuminationDecay *= decay;
	}

	vec4 dt = texture(dtex, texCoordVarying);
	vec3 min = vec3(0.94,0.94,0.94);
	vec3 max = (vec3(dt.x, dt.x, dt.x) - min) * 12.0;
	fragColor = (fragColor * exposure) * vec4(max, 1.0) + (normalVarying * 10.0) + ov;
}
