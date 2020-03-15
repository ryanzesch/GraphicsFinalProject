#version 330 core

in vec2 texCoord;
in float depth;
out vec4 color;
uniform sampler2D texBuf;

int getBlurCoeff(int i) {
	if (abs(i)==3) {
		return 8;
	}
	if (abs(i)==2) {
		return 28;
	}
	if (abs(i)==1) {
		return 56;
	}
	if (abs(i)==0) {
		return 70;
	}
	return 1;
}

void main(){

	vec3 texColor = vec3(0,0,0);
	bool doblur = false;
	for (int i=-4; i<5; i++) {
		for (int j=-4; j<5; j++) {
			int colfactor = getBlurCoeff(i);
			int rowfactor = getBlurCoeff(j);
			vec3 ptrgb = texture( texBuf, texCoord - vec2(i,j)*.0025 ).rgb;

			texColor += ptrgb * rowfactor * colfactor;

			if ( abs(ptrgb.r - .178) < .05 && abs(ptrgb.g - .31) < .05  && abs(ptrgb.b - .289) < .1 &&  ptrgb.b > ptrgb.r && ptrgb.b > ptrgb.g && i*i+j*j < 4) {
				doblur = true;
			}
		}
	}
	texColor /= 65536;
	//texColor = vec3(0,0,0);
	if (!doblur) {
		texColor = texture( texBuf, texCoord ).rgb;
	}
	
	color = vec4(texColor, 1.0);

}
