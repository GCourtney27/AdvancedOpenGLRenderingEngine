#version 330 core
out vec4 FragColor;

in VS_OUT 
{
	vec2 TexCoords;
} fs_in;

uniform sampler2D screenTexture;

const float offset = 1.0 / 300.0;

uniform bool postProcessEnabled;

uniform float time;

// Film Grain
uniform float filmgrainEnabled;
uniform float grainStrength;

// Vignette
uniform float vignetteEnabled;
uniform float vignetteInnerRadius;
uniform float vignetteOuterRadius;
uniform float vignetteOpacity;

vec4 GetTextureColor();

vec4 AddVignette(vec4 sourceColor);
vec4 AddFilmGrain(vec4 sourceColor);
float LinearizeDepth(float depth);

uniform float near_plane;
uniform float far_plane;
uniform float exposure;

void main()
{
//	float depthValue = texture(screenTexture, fs_in.TexCoords).r;
//	FragColor = vec4(vec3(depthValue), 1.0);
////	FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // Use for perspective matrix generated depth textures
//	return;

	vec4 texColor = GetTextureColor();

	vec4 result = texColor;
	result = AddFilmGrain(result) * filmgrainEnabled + result * (1 - filmgrainEnabled);
	result = AddVignette(result) * vignetteEnabled + result * (1 - vignetteEnabled);
	
	const float gamma = 2.2;
	
	vec3 mapped = vec3(1.0) - exp(-result.rgb * exposure);
	mapped = pow(mapped, vec3(1.0 / gamma));

	FragColor = vec4(mapped, 1.0);
}

vec4 GetTextureColor()
{
	return texture(screenTexture, fs_in.TexCoords);
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to Normalized Device Coordinates
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

vec4 AddFilmGrain(vec4 sourceColor)
{

	float x = (fs_in.TexCoords.x + 4.0 ) * (fs_in.TexCoords.y + 4.0 ) * (time * 10.0);
	vec4 grain = vec4(mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01)-0.005) * grainStrength;
    
    grain = 1.0 - grain;
	return grain * sourceColor;
}

vec4 AddVignette(vec4 sourceColor)
{
	vec2 centerUV = fs_in.TexCoords - vec2(0.5);
	vec4 color = vec4(1.0);

	// Vignette
	color.rgb *= 1.0 - smoothstep(vignetteInnerRadius, vignetteOuterRadius, length(centerUV));
	color *= sourceColor;
	color = mix(sourceColor, color, vignetteOpacity);

	return color;
}