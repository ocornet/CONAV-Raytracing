#include <iostream>
#include <fstream>
#include "ray.h"
#include "hitable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include <cstdlib>
#include <ctime>

using namespace std;


vec3 color(const ray& r, hitable* world,int depth=0) {
	hit_record hit;
	if (world->hit(r, 0.001, 1000.0, hit)) {
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && hit.mat_ptr->scatter(r, hit, attenuation, scattered)) {
			return attenuation * color(scattered, world, depth + 1);
		}
		else {
			return vec3(0, 0, 0);
		}
	}
	else {
		//blue sky
		vec3 dir = unit_vector(r.direction());
		float t = 0.5 * (dir.y() + 1.0);
		return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
	}
}

hitable* random_scene() {
	int n = 500;
	hitable **list = new hitable*[n + 1];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
	int i = 1;
	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			float choose_mat = random();
			vec3 center(a + 0.9*random(), 0.2, b + random());
			if (choose_mat < 0.8) {
				list[i++] = new sphere(center, 0.2, new lambertian(vec3(random(), random(), random())));
			}
			else if (choose_mat < 0.95) {
				list[i++] = new sphere(center, 0.2, new metal(vec3(0.5*(1 + random()), 0.5*(1 + random()), 0.5*(1 + random())), 0.5*random()));
			}
			else {
				list[i++] = new sphere(center, 0.2, new dielectric(1.5));
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0),1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

	return new hitable_list(list, i);
}

int main(int, char*) {
	srand(time(NULL));

	cout << "Generating image..." << endl;
	ofstream file;
	file.open("output.ppm");
	int nx = 800;
	int ny = 600;
	int ns = 100;
	file << "P3\n" << nx << " " << ny << "\n255\n";

	hitable *world = random_scene();

	vec3 lookfrom(3, 1, 2);
	vec3 lookat(0, 0, 0);
	float dist_to_focus = (lookfrom - lookat).length();
	float aperture = 0.5;

	camera cam(lookfrom, lookat, vec3(0, 1, 0), 110, float(nx) / float(ny),aperture, dist_to_focus);

	//(i,j) = (0,0) en  bas à gauche
	for (int j = ny - 1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {
			cout << "Calculating pixel (" << i << ";" << j << ")...";
			vec3 col(0, 0, 0);
			for (int s = 0; s < ns; s++) {
				float u = float(i+ random()) / float(nx);
				float v = float(j+ random()) / float(ny);

				ray r = cam.get_ray(u, v);
				col += color(r, world);
			}
			col /= float(ns);
			col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
			int ir = int(255.99*col[0]);
			int ig = int(255.99*col[1]);
			int ib = int(255.99*col[2]);
			file << ir << " " << ig << " " << ib << endl;
			cout << "Done." << endl;
		}
	}

	cout << "...Done." << endl;

	file.close();
	int end;
	cin >> end;
	return 0;
}