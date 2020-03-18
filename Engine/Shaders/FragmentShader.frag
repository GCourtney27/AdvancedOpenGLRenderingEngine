#version 330 core
out vec4 FragColor;

#define NUM_POINT_LIGHTS 1

in VS_OUT
{
	vec3 FragPos;  // Position in world space
	vec2 TexCoords;
	vec4 FragPosLightSpace;
	vec3 TangentViewPos;
	vec3 TangentFragPos;
	mat3 TBN;
} fs_in;

struct Material
{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_normal1;
	float shininess;
};

struct DirectionalLight
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight
{
	vec3 position;
	vec3 direction;
	float innerCutOff;
	float outerCutOff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

struct PointLight
{
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};


uniform Material material;

// Lights
// ------
uniform DirectionalLight dirLight;
uniform PointLight pointLights[NUM_POINT_LIGHTS];
uniform SpotLight spotLight;

uniform samplerCube skybox;
uniform sampler2D shadowMap;
uniform sampler2D depthMap;

uniform float height_scale;

vec2 parallaxTexCoords;

// Method signatures
// -----------------
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);
vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection);
vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPosition, vec3 viewDirection);
vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDirection);

void main()
{
	vec3 normal = texture(material.texture_normal1, fs_in.TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	vec3 viewDirection = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);

	parallaxTexCoords = ParallaxMapping(fs_in.TexCoords, viewDirection);
	if(parallaxTexCoords.x > 1.0 || parallaxTexCoords.y > 1.0 || parallaxTexCoords.x < 0.0 || parallaxTexCoords.y < 0.0)
		discard;

	vec3 result;// = CalculateDirectionalLight(dirLight, normal, viewDirection);

	for(int i = 0; i < NUM_POINT_LIGHTS; i++)
	{
		result += CalculatePointLight(pointLights[i], normal, fs_in.TangentFragPos, viewDirection);
    }    

//	result += CalculateSpotLight(spotLight, normal, fs_in.TangentFragPos, viewDirection);

	float gamma = 2.2;
	FragColor = vec4(pow(result.rgb, vec3(1.0/gamma)), 1.0);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
	const float minLayer = 8.0;
	const float maxLayers = 32.0;
	float numLayers = mix(maxLayers, minLayer, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
	float layerDepth = 1.0 / numLayers;
	float currentLayerDepth = 0.0;
	vec2 P = viewDir.xy * height_scale;
	vec2 deltaTexCoords = P / numLayers;

	vec2 currentTexCoords = texCoords;
	float currentDepthMapValue = texture(depthMap, currentTexCoords).r;

	while(currentLayerDepth < currentDepthMapValue)
	{
		currentTexCoords -= deltaTexCoords;
		currentDepthMapValue = texture(depthMap, currentTexCoords).r;
		currentLayerDepth += layerDepth;
	}

	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	float afterDepth = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;

	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords;
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDirection)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
	float bias = max(0.05 * (1.0 - dot(normal, lightDirection)), 0.005);

    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	
	// Soft shadows (Performance Heavy)
//	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
//	const int halfKernalWidth = 3;
//	for(int x= -halfKernalWidth; x <= halfKernalWidth; ++x)
//	{
//		for(int y = -halfKernalWidth; y <= halfKernalWidth; ++y)
//		{
//			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
//			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
//		}
//	}
//	shadow /= ((halfKernalWidth * 2 + 1) * (halfKernalWidth * 2 + 1));
	
	if(projCoords.z > 1.0)
	{
		shadow = 0.0;
	}

    return shadow;
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection)
{
	vec3 lightDir = normalize(-light.direction);
	vec3 halfwayDir = normalize(lightDir + viewDirection);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, fs_in.TexCoords));

	float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal, light.direction);
	vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * light.diffuse;  

    return result;
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPosition, vec3 viewDirection)
{
	vec3 tangentLightPos = fs_in.TBN * light.position;
	vec3 lightDir = normalize(tangentLightPos - fragPosition);
	vec3 halfwayDir = normalize(lightDir + viewDirection);
    // diffuse shading
    float diff = max(dot(lightDir, normal), 0.0);
    // specular shading
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance = length(tangentLightPos - fragPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, parallaxTexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, parallaxTexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, parallaxTexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 fragPosition, vec3 viewDirection)
{
//	vec3 tangentLightPos = fs_in.TBN * light.position;
    vec3 lightDir = normalize(light.position - fragPosition);
	vec3 halfwayDir = normalize(lightDir + viewDirection);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.innerCutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, fs_in.TexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}