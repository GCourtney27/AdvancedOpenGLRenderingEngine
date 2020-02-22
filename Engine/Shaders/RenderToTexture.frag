#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset = 1.0 / 300.0;

uniform bool postProcessEnabled;

uniform float Time;

// Film Grain
uniform float filmgrainEnabled;
uniform float grainStrength;

// Vignette
uniform float vignetteEnabled;
uniform float vignetteInnerRadius;
uniform float vignetteOuterRadius;
uniform float vignetteOpacity;

vec4 GetTextureColor();

vec4 AddVignette();
vec4 AddFilmGrain();

void main()
{
	vec4 result;

	result += AddVignette() * vignetteEnabled;
	result += AddFilmGrain() * filmgrainEnabled;
	//result += GetTextureColor();
	
	FragColor = result;
	return;

	// Inverted colors
//	FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);

	// Grey Scale
//	FragColor = texture(screenTexture, TexCoords);
//	float average = (0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.g) / 3.0;
//	FragColor = vec4(average, average, average, 1.0);

	// Kernal effects
	vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );
	// Sharpen
//	float kernal[9] = float[](
//		-1, -1,  -1, 
//		-1,  9,  -1, 
//		-1, -1,  -1
//	);
	// Blur
	float kernal[9] = float[](
		1.0 / 16, 2.0 / 16, 1.0 / 16,
		2.0 / 16, 4.0 / 16, 2.0 / 16,
		1.0 / 16, 2.0 / 16, 1.0 / 16  
	);
	vec3 sampleTex[9];
	for(int i = 0; i < 9; i++)
	{
		sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
	}
	vec3 col = vec3(0.0);
	for(int i = 0; i < 9; i++)
		col += sampleTex[i] * kernal[i];

	FragColor = vec4(col, 1.0);
}

vec4 GetTextureColor()
{
	return texture(screenTexture, TexCoords);
}

vec4 AddFilmGrain()
{
	vec4 color = GetTextureColor();

	float x = (TexCoords.x + 4.0 ) * (TexCoords.y + 4.0 ) * (Time * 10.0);
	vec4 grain = vec4(mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01)-0.005) * grainStrength;
    
    grain = 1.0 - grain;
	return grain * color;
}

vec4 AddVignette()
{
	vec2 centerUV = TexCoords - vec2(0.5);
	vec4 sourceColor = GetTextureColor();
	vec4 color = vec4(1.0);

	// Vignette
	color.rgb *= 1.0 - smoothstep(vignetteInnerRadius, vignetteOuterRadius, length(centerUV));
	color *= sourceColor;
	color = mix(sourceColor, color, vignetteOpacity);
	return color;
}