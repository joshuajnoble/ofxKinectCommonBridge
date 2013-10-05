//
//     _____    ___
//    /    /   /  /     vboMeshDrawInstancedExample
//   /  __/ * /  /__    (c) ponies & light, 2013. All rights reserved.
//  /__/     /_____/    poniesandlight.co.uk
//
//  Created by tim on 30/07/2013.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

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
	
	
	vec2 vv = vec2(instanceX * cxRatio, 480 - (instanceY * cyRatio));
	float pixelDepth = 1.0 - texture(tex0, vv).x;

	vec2 vv2 = vec2(instanceX * cxRatio, 480 - (instanceY * cyRatio));
	colorVarying = texture(tex1, vv2);
	
	vec4 vPos = position;
	
	// extrude our primitive along the z coordinate, based on current PixelDepth.
	vPos.z = (vPos.z) * (pixelDepth * 20.0);
	vPos.z = vPos.z + (sin( (instanceX/50.0) + timeValue ) * 10.0);

	texCoordVarying = (textureMatrix*vec4(texcoord.x, 240 - texcoord.y,0,1)).xy;

	gl_Position = projectionMatrix * modelViewMatrix * vPos;
}