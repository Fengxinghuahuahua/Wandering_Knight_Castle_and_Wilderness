#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  

uniform sampler2D TexBgColor;

uniform vec3 viewPos; 
uniform vec3 lightPos; 

uniform mat4 view;
uniform mat4 projection;

uniform float cloudSize;
uniform float cloudHeight;

uniform float cloudThickness;
uniform float time;

float bottom = cloudHeight - cloudThickness;
float top = cloudHeight + cloudThickness;

#define baseBright  vec3(1.26,1.25,1.29)    // ������ɫ -- ����
#define baseDark    vec3(0.31,0.31,0.32)    // ������ɫ -- ����
#define lightBright vec3(1.29, 1.17, 1.05)  // ������ɫ -- ����
#define lightDark   vec3(0.7,0.75,0.8)      // ������ɫ -- ����

//#define bottom 13   // �Ʋ�ײ�
//#define top 20      // �Ʋ㶥��
#define cellSize 24  // worleyNoise �ܶ�
float noiseMapSize = 64;

vec2 hash(vec2 p)
{
    float random = sin(666 + p.x *5678 + p.y *1234)*4321;
    return vec2(p.x + sin(random) /2 + 0.5f ,p.y + cos(random) / 2 +0.5f);
}
float noise(vec2 para)
{
    float x = para.x;
    float y = para.y;
    float dis = 10000;
    for(int Y = -1;Y<=1;Y++)
    {
        for(int X = -1;X<=1;X++)
        {
            vec2 cellPoint = hash(vec2( int(x) + X ,int(y) + Y) );
            dis = min(dis , distance(cellPoint,vec2(x,y)));
        }
    }
    return dis;
}

// ���� worleyNoise
// pos -- ��������
float worleyNoise(vec3 pos)
{ 
    float noiseSum = 0;
    vec2 coord = pos.xz / vec2(cloudSize * 2 ,cloudSize * 2) + vec2(0.5,0.5);
    vec2 xy = coord * noiseMapSize;

    float timeShift = time * 0.02; 

    vec2 para = vec2(xy.x/cellSize,xy.y/cellSize);
    noiseSum = noise(para);
    noiseSum += noise(para * (3.5 + 0.2 * sin(timeShift))) / (3.5 + 0.2 * sin(timeShift));
    if(noiseSum > 0.8) return noiseSum;

    noiseSum += noise(para * (12.25 + 0.5 * sin(timeShift))) / (12.25 + 0.5 * sin(timeShift));
    noiseSum += noise(para * 26.87) / 26.87;

    return noiseSum;
}

float random(float x)
{
    float y = fract(sin(x)*100000.0);
    return y;
}

vec3 whiteNoise(vec3 pos)
{
    return vec3(random(pos.x),random(pos.y),random(pos.z));
}

// ���� pos ������ܶ�
// pos -- ��������
float getDensity(vec3 pos) {
    float timeShift = time * 1.2;
    vec3 p = pos; 
    p.x += timeShift; 
    p.z += timeShift * 2;

    float noise = worleyNoise(p);
    noise = 1 - noise;

    // �߶�˥��
    float mid = (bottom + top) / 2.0;
    float h = top - bottom;
    float weight = 1.0 - 2.0 * abs(mid - pos.y) / h;
    weight = pow(weight, 0.5);

    noise *= weight;

    float decayFactor = exp(-0.8 * (length(pos.xz) / cloudSize) * (length(pos.xz) / cloudSize));
    noise *= decayFactor; 

    if(noise < 0.3)noise = 0;
    return noise;
}

// ��ȡ�������ɫ
vec4 getCloud(vec3 worldPos, vec3 cameraPos) {
            
    vec3 direction = normalize(worldPos - cameraPos);   // �������߷���
    vec3 step = direction * 0.25;   // ����
    vec4 colorSum = vec4(0);        // ���۵���ɫ
    vec3 point = cameraPos;         // �����������ʼ��

    // bottom
    if(point.y<bottom) { 
        point += direction * (abs(bottom - cameraPos.y) / abs(direction.y));
    }

    // ray marching
    for(int i=0; i<50; i++) {
        if(colorSum.a > 0.97) break;

        point += step;
        point += whiteNoise(point) * 0.1;

        if(bottom>point.y || point.y>top || cloudSize<abs(point.x) || cloudSize<abs(point.z)) {
            continue;
        }
        
        //float density = 0.1;
        float density = getDensity(point);
        if(density<0.3) continue;

        vec3 L = normalize(lightPos - point);
        float lightDensity = getDensity(point + L);
        float delta = clamp(density - lightDensity ,0.0,1.0);

        density *= 0.5;

        vec3 base = mix(baseBright,baseDark,density) * density;
        vec3 light = mix(lightDark,lightBright,delta);

        vec4 color = vec4(base * light,density);

        colorSum = colorSum + color * (1.0 - colorSum.a);   // ���ۻ�����ɫ���
    }

    return colorSum;
}

void main()
{
    vec4 cloud = getCloud(FragPos  , viewPos); // ����ɫ
    vec4 clipSpacePos = projection * view * vec4(FragPos, 1.0);
    vec3 ndc = clipSpacePos.xyz / clipSpacePos.w;
    vec4 sceneColor = texture(TexBgColor, (ndc.xy + 1.0)/2.0);
    cloud = (cloud.a <= 0.05) ? sceneColor : mix(sceneColor, cloud, cloud.a);

    FragColor = cloud;
} 