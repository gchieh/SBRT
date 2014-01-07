#version 330 core

uniform sampler1D faceArray;
uniform sampler1D vertice;

in vec3 worldPos;
out vec4 outputF;

struct Sphere 
{
	vec3 pos;
	float radius;
	vec3 color;
	int m;
};

struct Plane 
{
	vec3 pos;
	vec3 norm;
	vec3 color;
	int m;
};

struct Triangle
{
	vec3 a_pos;
	vec3 b_pos;
	vec3 c_pos;
	vec3 color;
	int m;
};

struct Ray {
	vec3 start;
	vec3 direction;
	int object;
};

vec3 lightColor;
vec3 lightPos;
float EPS = .01;
const int sphere_num = 1;
const int plane_num = 1;
const int triangle_num = 12;
Sphere sphere[sphere_num];
Plane plane[plane_num];
Triangle triangle[triangle_num];

#define reflectcof 0.75
#define Refraction 1.8
#define pi 3.14159

bool sphereIntersect (Ray r, int sphereno, inout float t){
 //Compute A, B and C coefficients
    //a = d.d
	float a = dot(r.direction, r.direction);	
	//b = 2*(o-c).d
	vec3 o_c = r.start - sphere[sphereno].pos;
	float b = 2*dot(o_c, r.direction);
	//c = (o-c).(o-c)-r^2
	float c = dot(o_c, o_c) - pow(sphere[sphereno].radius,2.0);

    //Find discriminant
    float disc = b * b - 4 * a * c;
    
    // if discriminant is negative there are no real roots, so return 
    // false as ray misses sphere
    if (disc < 0)
        return false;

    // compute q as described above
    float distSqrt = sqrt(disc);
    float q;
    if (b < 0)
        q = (-b - distSqrt)/2.0;
    else
        q = (-b + distSqrt)/2.0;

    // compute t0 and t1
    float t0 = q / a;
    float t1 = c / q;

    // make sure t0 is smaller than t1
    if (t0 > t1){
        // if t0 is bigger than t1 swap them around
        float temp = t0;
        t0 = t1;
        t1 = temp;
    }

    // if t1 is less than zero, the object is in the ray's negative direction
    // and consequently the ray misses the sphere
    if (t1 < 0)
        return false;

    // if t0 is less than zero, the intersection point is at t1
    if (t0 < 0)
        t = t1;  
    // else the intersection point is at t0
    else   
        t = t0;
	
	if(t< 0.00001)
		return false;

   return true;
}

bool planeIntersection(Ray r, int planeno, inout float t){

	t=dot((plane[planeno].pos-r.start), plane[planeno].norm)/dot(r.direction, plane[planeno].norm);
	if(t>=0)
		return true;
	else
		return false;
}

bool triangleIntersection(Ray r, int triangleno, inout float t){
	vec3 e1 = triangle[triangleno].b_pos-triangle[triangleno].a_pos;
	vec3 e2 = triangle[triangleno].c_pos-triangle[triangleno].a_pos;
	vec3 n = cross(r.direction , e2);

	float a = dot(e1, n);
	if(a > -0.00001 && a < 0.00001)
		return false;
	
	float f=1/a;
	vec3 s = r.start - triangle[triangleno].a_pos;
	float u = f * dot(s, n);

	if(u < 0 || u > 1)
		return false;
	
	vec3 q = cross(s,e1);
	float v = f * dot(r.direction, q);

	if(v < 0 || (u+v) > 1)
		return false;

	// Compute t
	t = f * dot(e2, q);

	if(t > 0.000001)
		return true;
	else
	return false;
}

int Intersection(Ray r, inout float t){
	float Table1[sphere_num];
	float Table2[plane_num];
	float Table3[128];
	int i_sphere = -1;
	int i_plane = -1;
	int i_triangle = -1;
	float ts, tp, tt;
	//找出最近的intersection點，t為深度

	float tmin =100000;
	for ( int k = 0; k < sphere_num; ++k){ 
		Table1[k] = -1;
		if (k != r.object && sphereIntersect(r,k,Table1[k])){
			if(Table1[k] < tmin){
				tmin = Table1[k];
				i_sphere = k;
			}
		}
	}
	
	tmin =100000;
	for ( int k = 0; k < plane_num; ++k){ 
		Table2[k] = -1;
		if (k != (r.object-sphere_num) && planeIntersection(r,k,Table2[k])){
			if(Table2[k] < tmin){
				tmin = Table2[k];
				i_plane = k;
			}
		}
	}

	tmin =100000;
	for ( int k = 0; k < triangle_num; ++k){ 
		Table3[k] = -1;
		if (k != (r.object-sphere_num-plane_num) && triangleIntersection(r,k,Table3[k])){
			if(Table3[k] < tmin){
				tmin = Table3[k];
				i_triangle = k;
			}
		}
	}

	ts = Table1[i_sphere];
	tp = Table2[i_plane];
	tt = Table3[i_triangle];
	
	t = 100000;
	if(ts>0 && ts<t)								//判斷ray先hit到球或三角片或平面
		t = ts;
	if(tp>0 && tp<t)
		t = tp;
	if(tt>0 && tt<t)
		t = tt;

	if(t == ts)
		return i_sphere;
	else if(t == tp)
		return i_plane+sphere_num;
	else if( t == tt)
		return i_triangle+sphere_num+plane_num;
	else
		return -1;
}
bool inShadow(Ray r){
	float u = -1;
	// Shadow ray intersection 
	int i_sphere = Intersection(r,u);
	if (i_sphere >= 0)
		return true;
	return false;
}

vec3 scaleColor(vec3 color){
		if(color.x > 1) color.x = 1;	
		if(color.y > 1) color.y = 1;
		if(color.z > 1) color.z = 1;
		return color;
}

vec3 triangle_Color(Ray r, Ray shadowRay, int i_obj, vec3 norm){
	vec3 color = vec3(0.0, 0.0, 0.0);
	float power = dot(shadowRay.direction, norm) /  (sqrt(dot(shadowRay.direction, shadowRay.direction)) * sqrt(dot(norm, norm))) ;	//L dot N
	if(power <0) power = 0;
	vec3 H = (r.start+lightPos)/2-shadowRay.start;
	float power2= dot(H , norm) /  (sqrt(dot(H, H)) * sqrt(dot(norm, norm))) ;	//H dot N
	float specular = pow( power2 , 70);
	color +=  power * triangle[i_obj].color * lightColor;					//diffuse
	color += 1.0 * specular * lightColor;							//specular
	return scaleColor(color);
}

vec3 sphere_Color(Ray r, Ray shadowRay, int i_obj, vec3 norm){
	vec3 color = vec3(0.0, 0.0, 0.0);
	float power = dot(shadowRay.direction, norm) /  (sqrt(dot(shadowRay.direction, shadowRay.direction)) * sqrt(dot(norm, norm))) ;	//L dot N
	if(power <0) power = 0;
	vec3 H = (r.start+lightPos)/2-shadowRay.start;
	float power2= dot(H , norm) /  (sqrt(dot(H, H)) * sqrt(dot(norm, norm))) ;	//H dot N
	float specular = pow( power2 , 70);
	color +=  power * sphere[i_obj].color * lightColor;					//diffuse
	color += 1.0 * specular * lightColor;							//specular
	return scaleColor(color);
}

vec3 plane_Color(Ray r, Ray shadowRay, int i_obj, vec3 norm){
	vec3 color = vec3(0.0, 0.0, 0.0);
	float power = dot(shadowRay.direction, norm) /  (sqrt(dot(shadowRay.direction, shadowRay.direction)) * sqrt(dot(norm, norm))) ;	//L dot N
	if(power <0) power = 0;
	vec3 H = (r.start+lightPos)/2-shadowRay.start;
	float power2= dot(H , norm) /  (sqrt(dot(H, H)) * sqrt(dot(norm, norm))) ;	//H dot N
	float specular = pow( power2 , 70);
	color +=  power * plane[i_obj].color * lightColor;					//diffuse
	color += 1.0 * specular * lightColor;							//specular
	return scaleColor(color);
}

vec3 ScomputeColor(Ray r,int level)
{
	float t = -1;
	++level;
	vec3 color = vec3(0.0, 0.0, 0.0);

	if(level<=2){
		int i_obj = Intersection(r,t);
		r.object = i_obj;
		
		//判斷有無intersection
		if(i_obj >=0){
			Ray shadowRay;
			shadowRay.start = r.start + t*r.direction;
			shadowRay.direction = lightPos - shadowRay.start;
			shadowRay.object = i_obj;
			normalize(shadowRay.direction);
			vec3 norm;
			
			if(i_obj >= sphere_num){
				i_obj -= sphere_num;
				norm = plane[i_obj].norm;
				normalize(norm);
				// Not in Shadow				
				if ( !inShadow(shadowRay) ){
					color += plane_Color(r, shadowRay, i_obj, norm);
				}
			}

			//跟球有intersection
			else{
				norm = shadowRay.start - sphere[i_obj].pos;
				normalize(norm);
				// Not in Shadow				
				if ( !inShadow(shadowRay) ){
					color += sphere_Color(r, shadowRay, i_obj, norm);
					//scaleColor(color);
				}
			}	
		}
	}
	return scaleColor(color);
}

vec3 computeColor(Ray r,int level)
{
	float t = -1;
	++level;
	vec3 color = vec3(0.0, 0.0, 0.0);

	if(level<=2){
		int i_obj = Intersection(r,t);
		r.object = i_obj;
		
		//判斷有無intersection
		if(i_obj >=0){
			Ray shadowRay;
			shadowRay.start = r.start + t*r.direction;
			shadowRay.direction = lightPos - shadowRay.start;
			shadowRay.object = i_obj;
			normalize(shadowRay.direction);
			vec3 norm;
			
			//跟三角片有intersection
			if(i_obj >= sphere_num+plane_num){
				i_obj -= sphere_num+plane_num;
				vec3 v1 = triangle[i_obj].b_pos - shadowRay.start;
				vec3 v2 = triangle[i_obj].c_pos - shadowRay.start;
				norm = cross(v1,v2);
				normalize(norm);
				if ( !inShadow(shadowRay) ){
					color += triangle_Color(r, shadowRay, i_obj, norm);
				}
			}
			//跟平面有intersection
			else if(i_obj >= sphere_num){
				i_obj -= sphere_num;
				norm = plane[i_obj].norm;
				normalize(norm);
				// Not in Shadow				
				if ( !inShadow(shadowRay) ){
					color += plane_Color(r, shadowRay, i_obj, norm);
				}
			}
			//跟球有intersection
			else{
				norm = shadowRay.start - sphere[i_obj].pos;
				normalize(norm);
				// Not in Shadow				
				if ( !inShadow(shadowRay) ){
					color += sphere_Color(r, shadowRay, i_obj, norm);
					//scaleColor(color);
				}	
			}
		
			float cosn = dot(r.direction, norm);
			r.start = shadowRay.start;
			r.direction = r.direction -2* cosn*norm;
			normalize(r.direction);

			// 計算反射
			//color += reflectcof*ScomputeColor(r,level); 
			
			/*
			
				// 計算折射
				float Refraction_tmp =0;
				if (level % 2 !=0)
				Refraction_tmp = Refraction;
				else Refraction_tmp = 1/Refraction;
				float refraction_sin2 =pow(Refraction_tmp,2)*(1-cosn*cosn);
				if (refraction_sin2 <= 1.0) {
					r.start=shadowRay.start;
					r.direction = Refraction_tmp*r.direction-(Refraction_tmp+sqrt(1-refraction_sin2))*norm;
					normalize(r.direction);
					color += 0.5*ScomputeColor(r,level); 
				
			}*/
		}
	}
	return scaleColor(color);
}

void main(void)
{

		//face_x[i] = texelFetch(texture, i);


	lightColor = vec3(1, 1, 1);
	vec3 eyePos = vec3(20,30,150);
	lightPos = vec3(250, 600, 150);
	
	sphere[0].color = vec3(0.0, 0.5, 0.0);
	sphere[0].pos = vec3(600, 21,155);
	sphere[0].radius = 3;
	
	//sphere[1].color = vec3(0.0, 0.0 ,0.8);
	//sphere[1].pos = vec3(-5, 15, 55);
	//sphere[1].radius = 6;

	//sphere[2].color = vec3(0.0 ,1.0 ,1.0);
	//sphere[2].pos = vec3(7, 15, 55);
	//sphere[2].radius = 6;

	//sphere[3].color = vec3(0.9 ,0.0 ,0.9);
	//sphere[3].pos = vec3(20, 10, 35);
	//sphere[3].radius = 9;

	//sphere[4].color = vec3(0.9, 0.0, 0.0);
	//sphere[4].pos = vec3(40, 20, 0);
	//sphere[4].radius = 12;

	plane[0].pos = vec3(0.0, -20.0, 0.0);
	plane[0].norm = vec3(0.0, 1.0, 0.0);
	plane[0].color = vec3(0.1, 0.1, 0.1);
	
	/*float ti[12];
	for(int i = 0; i < 12; i++)
		ti[i]= texelFetch(Texture,i, 0).x;*/
	vec4 face[36];
	vec4 vertex[8];

	
	for(int i = 0; i < 12; i++)
		face[i]= texelFetch(faceArray,i, 0);

	for(int i = 0; i < 8; i++)
		vertex[i]= vec4(texelFetch(vertice,i, 0));
	 
	for(int i = 0;i < triangle_num; i++)
	{


		triangle[i].a_pos = vec3(vertex[(int(face[i].x))])*20;
		triangle[i].b_pos = vec3(vertex[(int(face[i].y))])*20;
		triangle[i].c_pos = vec3(vertex[(int(face[i].z))])*20;



		triangle[i].color = vec3(0.9, 0.0 ,0.0);
	}
	
	int level = 0;
	vec3 color = vec3(0.05, 0.05, 0.05);		//background ambient
	Ray r;
	r.start = eyePos;
	r.direction = normalize(worldPos - eyePos);
	//outColor = vec4(computeColor(r,level), 1);
	outputF = vec4(computeColor(r,level), 1);
	//outputF = vec4(ti_x[4].x, 0, 0, 1);
	//outputF = vec4(vertex[0].x, vertex[0].x, vertex[0].x, 1);
}