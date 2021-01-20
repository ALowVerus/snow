#pragma once

#include "interdependent_initializer.h"
#include "vec3.h"
#include "hit_record.h"
#include "hittable.h"
#include "hittable_list.h"
#include "texture.h"
#include "materials.h"
#include "rectangles.h"


class box : public hittable  {
    public:
        box() {}
        box(const point3& p0, const point3& p1, std::shared_ptr<material> ptr) : box_min(p0), box_max(p1) {
            box_min = p0;
            box_max = p1;

            sides.add(std::make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), ptr));
            sides.add(std::make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), ptr));

            sides.add(std::make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), ptr));
            sides.add(std::make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), ptr));

            sides.add(std::make_shared<yz_rect>(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), ptr));
            sides.add(std::make_shared<yz_rect>(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), ptr));
        }

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override {
            return sides.hit(r, t_min, t_max, rec);
        }
        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
            output_box = aabb(box_min, box_max);
            return true;
        }

    public:
        point3 box_min;
        point3 box_max;
        hittable_list sides;
};


class constant_medium : public hittable {
    public:
        constant_medium(std::shared_ptr<hittable> b, double d, std::shared_ptr<texture> a)
            : boundary(b),
              neg_inv_density(-1/d),
              phase_function(std::make_shared<isotropic>(a))
            {}

        constant_medium(std::shared_ptr<hittable> b, double d, color c)
            : boundary(b),
              neg_inv_density(-1/d),
              phase_function(std::make_shared<isotropic>(c))
            {}

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override {
            hit_record rec1, rec2;

            if (!boundary->hit(r, -infinity, infinity, rec1))
                return false;

            if (!boundary->hit(r, rec1.t+0.0001, infinity, rec2))
                return false;

            if (rec1.t < t_min) rec1.t = t_min;
            if (rec2.t > t_max) rec2.t = t_max;

            if (rec1.t >= rec2.t)
                return false;

            if (rec1.t < 0)
                rec1.t = 0;

            const auto ray_length = r.direction().length();
            const auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length;
            const auto hit_distance = neg_inv_density * log(random_double());

            if (hit_distance > distance_inside_boundary)
                return false;

            rec.t = rec1.t + hit_distance / ray_length;
            rec.p = r.at(rec.t);

            rec.normal = vec3(1,0,0);  // arbitrary
            rec.front_face = true;     // also arbitrary
            rec.mat_ptr = phase_function;

            return true;
        }

        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
            return boundary->bounding_box(time0, time1, output_box);
        }

    public:
        std::shared_ptr<hittable> boundary;
        std::shared_ptr<material> phase_function;
        double neg_inv_density;
};


class sphere : public hittable {
    public:
        sphere() {}
        sphere(point3 cen, double r, std::shared_ptr<material> m) 
            : center(cen), radius(r), mat_ptr(m) {};

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override {
            vec3 oc = r.origin() - center;
            auto a = r.direction().length_squared();
            auto half_b = dot(oc, r.direction());
            auto c = oc.length_squared() - radius*radius;

            auto discriminant = half_b*half_b - a*c;
            if (discriminant < 0) return false;
            auto sqrtd = sqrt(discriminant);

            // Find the nearest root that lies in the acceptable range.
            auto root = (-half_b - sqrtd) / a;
            if (root < t_min || t_max < root) {
                root = (-half_b + sqrtd) / a;
                if (root < t_min || t_max < root)
                    return false;
            }

            rec.t = root;
            rec.p = r.at(rec.t);
            vec3 outward_normal = (rec.p - center) / radius;
            rec.set_face_normal(r, outward_normal);
            get_sphere_uv(outward_normal, rec.u, rec.v);
            rec.mat_ptr = mat_ptr;

            return true;
        }
    
        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
            output_box = aabb(
                center - vec3(radius, radius, radius),
                center + vec3(radius, radius, radius));
            return true;
        }

    public:
        point3 center;
        double radius;
        std::shared_ptr<material> mat_ptr;
    
    private:
        static void get_sphere_uv(const point3& p, double& u, double& v) {
            // p: a given point on the sphere of radius one, centered at the origin.
            // u: returned value [0,1] of angle around the Y axis from X=-1.
            // v: returned value [0,1] of angle from Y=-1 to Y=+1.
            //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
            //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
            //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

            auto theta = acos(-p.y());
            auto phi = atan2(-p.z(), p.x()) + pi;

            u = phi / (2*pi);
            v = theta / pi;
        }
};


class moving_sphere : public hittable {
    public:
        moving_sphere() {}
        moving_sphere(
            point3 cen0, point3 cen1, double _time0, double _time1, double r, std::shared_ptr<material> m)
            : center0(cen0), center1(cen1), time0(_time0), time1(_time1), radius(r), mat_ptr(m)
        {};

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override {
            vec3 oc = r.origin() - center(r.time());
            auto a = r.direction().length_squared();
            auto half_b = dot(oc, r.direction());
            auto c = oc.length_squared() - radius*radius;

            auto discriminant = half_b*half_b - a*c;
            if (discriminant < 0) return false;
            auto sqrtd = sqrt(discriminant);

            // Find the nearest root that lies in the acceptable range.
            auto root = (-half_b - sqrtd) / a;
            if (root < t_min || t_max < root) {
                root = (-half_b + sqrtd) / a;
                if (root < t_min || t_max < root)
                    return false;
            }

            rec.t = root;
            rec.p = r.at(rec.t);
            auto outward_normal = (rec.p - center(r.time())) / radius;
            rec.set_face_normal(r, outward_normal);
            rec.mat_ptr = mat_ptr;

            return true;
        }
    
        virtual bool bounding_box(double _time0, double _time1, aabb& output_box) const override {
            aabb box0(
                center(_time0) - vec3(radius, radius, radius),
                center(_time0) + vec3(radius, radius, radius));
            aabb box1(
                center(_time1) - vec3(radius, radius, radius),
                center(_time1) + vec3(radius, radius, radius));
            output_box = surrounding_box(box0, box1);
            return true;
        }
    
        point3 center(double time) const {
            return center0 + (center1 - center0)*((time - time0) / (time1 - time0));
        }

    public:
        point3 center0, center1;
        double time0, time1;
        double radius;
        std::shared_ptr<material> mat_ptr;
};