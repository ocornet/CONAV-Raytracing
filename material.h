#pragma once

#include "hitable.h"

class material {
public:
	virtual bool scatter(const ray& r_in, const hit_record& hit, vec3& attenuation, ray& scattered) const = 0;
};

class lambertian : public material {
public :
	lambertian(const vec3& a) : albedo(a) {}
	virtual bool scatter(const ray& r_in, const hit_record& hit, vec3& attenuation, ray& scattered) const {
		vec3 target = hit.p + hit.normal + random_in_unit_sphere();
		scattered = ray(hit.p, target - hit.p);
		attenuation = albedo;
		return true;
	}

	vec3 albedo;
};

vec3 reflect(const vec3 & v, const vec3& n) {
	return v - 2 * dot(v, n)*n;
}

class metal : public material {
public:
	metal(const vec3& a, float f) : albedo(a) { if (f < 1) fuzz = f; else fuzz = 1; }
	virtual bool scatter(const ray& r_in, const hit_record& hit, vec3& attenuation, ray& scattered) const {
		vec3 reflected = reflect(unit_vector(r_in.direction()), hit.normal);
		scattered = ray(hit.p, reflected + fuzz*random_in_unit_sphere());
		attenuation = albedo;
		return (dot(scattered.direction(),hit.normal) > 0);
	}

	vec3 albedo;
	float fuzz;
};

bool refract(const vec3 & v, const vec3& n, float ni_over_nt, vec3& refracted) {
	vec3 uv = unit_vector(v);
	float dt = dot(uv, n);
	float discriminant = 1.0 - ni_over_nt * ni_over_nt*(1 - dt * dt);
	if (discriminant > 0) {
		refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
		return true;
	}
	else {
		return false;
	}
}

float schlick(float cosine, float ref_idx) {
	float r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1 - r0)*pow((1 - cosine), 5);
}

class dielectric : public material {
public:
	dielectric(float ri) : ref_idx(ri) {}
	virtual bool scatter(const ray& r_in, const hit_record& hit, vec3& attenuation, ray& scattered) const {
		vec3 outward_normal;
		vec3 reflected = reflect(r_in.direction(), hit.normal);
		float ni_over_nt;
		attenuation = vec3(1.0, 1.0, 1.0);
		vec3 refracted;
		float reflect_prob;
		float cosine;
		if (dot(r_in.direction(), hit.normal) > 0) { //sortie de la sphère
			outward_normal = -hit.normal;
			ni_over_nt = ref_idx;
			cosine = ref_idx * dot(r_in.direction(), hit.normal) / r_in.direction().length();
		}
		else {
			outward_normal = hit.normal;
			ni_over_nt = 1.0 / ref_idx;
			cosine = -dot(r_in.direction(), hit.normal) / r_in.direction().length();
		}
		if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted)) {
			reflect_prob = schlick(cosine, ref_idx);
		}
		else {
			reflect_prob = 1.0;
		}
		if (random() < reflect_prob) {
			scattered = ray(hit.p, reflected);
		}
		else {
			scattered = ray(hit.p, refracted);
		}
		return true;
	}

	float ref_idx;
};