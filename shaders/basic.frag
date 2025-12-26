#version 410 core

in vec3 fNormal;
in vec4 fPosEye;

out vec4 fColor;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;

vec3 ambient = vec3(0.f);
float ambientStrength = 0.2f;
vec3 diffuse = vec3(0.f);
vec3 specular = vec3(0.f);
float specularStrength = 0.5f;
float shininess = 32.0f;

//  Needed for pos lighting, for attenuation
//  Multiple positional lights
#define MAX_POINT_LIGHTS 10
uniform int numPointLights;  // Actual number of lights active
uniform vec3 pointLightPositions[MAX_POINT_LIGHTS];
uniform vec3 pointLightColor;  //  Shared color
float constant = 1.f;
float linear = 2.f;
float quadratic = 4.f;
vec3 ambientPoint = vec3(0.f);
vec3 diffusePoint = vec3(0.f);
vec3 specularPoint = vec3(0.f);
//uniform vec3 lightPos;
//uniform vec3 posColor;

//needed for light maps
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
in vec2 fragTexCoords;

//shadows
uniform sampler2D shadowMap;
in vec4 fragPosLightSpace;

void computeDirectionalLight(){
    vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
    vec3 normalEye = normalize(fNormal);
    vec3 lightDirN = normalize(lightDir);

    //compute view direction
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

    //compute half vector
    vec3 halfVector = normalize(lightDirN + viewDirN);

    //compute ambient light - directional
    ambient += ambientStrength * lightColor;

    //compute diffuse light - directional
    diffuse += max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflection = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
    specular += specularStrength * specCoeff * lightColor;
}

void computePositionalLight(vec3 lightPos){
    vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin

    //transform normal
    vec3 normalEye = normalize(fNormal);

    //compute light direction
    vec3 lightDirN = normalize(lightPos - fPosEye.xyz);

    //compute view direction
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

    //compute half vector
    vec3 halfVector = normalize(lightDirN + viewDirN);


    //compute ambient light
    float dist = length(lightPos - fPosEye.xyz);
    float att = 1.f / (constant + linear * dist + quadratic*(dist*dist));
    ambientPoint += att * ambientStrength * pointLightColor;

    //compute diffuse light
    diffusePoint += att * max(dot(normalEye, lightDirN), 0.0f) * pointLightColor;

    //compute specular light

    float specCoeff = pow(max(dot(normalEye, halfVector), 0.f), shininess);

    specularPoint += att * specularStrength * specCoeff * pointLightColor;
}

void computeLightComponents()
{
    computeDirectionalLight();
    for(int i = 0; i < numPointLights; i++){
        computePositionalLight(pointLightPositions[i]);
    }
}

float computeShadow(){
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    normalizedCoords = normalizedCoords * 0.5 + 0.5;
    if (normalizedCoords.z > 1.0f)
    return 0.0f;
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
    float currentDepth = normalizedCoords.z;
    float bias = 0.002f;
    float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
    return shadow;
}

float computeFog() {
    float fogDensity = 0.025f;
    float fragmentDistance = length(fPosEye);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

    return clamp(fogFactor, 0.0f, 1.0f);
}

void main()
{
    computeLightComponents();

    //	we dont use alpha blending, use just rgb values
    vec3 texDiffuse = texture(diffuseTexture, fragTexCoords).rgb;
    vec3 texSpecular = texture(specularTexture, fragTexCoords).rgb;

    float shadow = computeShadow();
    vec3 lightingDir = (ambient + (1.0f - shadow) * diffuse) * texDiffuse +
    ((1.0f - shadow) * specular) * texSpecular;

    // Point lights
    vec3 lightingPoint = (ambientPoint + diffusePoint) * texDiffuse +
    (specularPoint) * texSpecular;

    vec3 finalColor = min(lightingDir + lightingPoint, 1.0f);

    //  Un-comment this for fog, does not work on sky
//    float fogFactor = computeFog();
//    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
//    fColor = mix(fogColor, vec4(finalColor, 1.f), fogFactor);
    fColor = vec4(finalColor, 1.0f);
}
