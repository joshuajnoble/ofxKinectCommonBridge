
#version 150

uniform sampler2DRect src_tex_unit0;
uniform float usingTexture = 0.0;
uniform float usingColors = 0.0;
uniform vec4 globalColor = vec4(1.0);

in float depth;
in vec4 colorVarying;
in vec2 texCoordVarying;
																		 
out vec4 fragColor;

void main(){
	vec4 v = texture(src_tex_unit0, texCoordVarying);
	//fragColor = vec4(v.r, texCoordVarying.x/640.0, texCoordVarying.y/480, 1.0);
	fragColor = vec4(v.a, v.g, v.b, 1.0);
}