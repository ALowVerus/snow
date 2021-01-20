#pragma once


class bvh_node : public hittable {
    public:
        bvh_node() {};
    
        bvh_node(
            std::vector<std::shared_ptr<hittable>>& src_objects,
            size_t start, size_t end, double time0, double time1
        );
    
        bvh_node(hittable_list& list, double time0, double time1)
            : bvh_node(list.objects, 0, list.objects.size(), time0, time1)
        {}
    
        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override {
            if (!box.hit(r, t_min, t_max))
                return false;

            bool hit_left = left->hit(r, t_min, t_max, rec);
            bool hit_right = right->hit(r, t_min, hit_left ? rec.t : t_max, rec);

            return hit_left || hit_right;
        }
    
        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
            output_box = box;
            return true;
        }

    
    public:
        std::shared_ptr<hittable> left;
        std::shared_ptr<hittable> right;
        aabb box;
};


bvh_node::bvh_node(
    std::vector<std::shared_ptr<hittable>>& src_objects,
    size_t start, size_t end, double time0, double time1
) {

    std::cout << "Organizing a new BVH Node.\n";

    auto objects = src_objects; // Create a modifiable array of the source scene objects

    int axis = random_int(0,2);
    std::cout << "Axis is " << axis << ".\n";
    auto comparator = (axis == 0) ? box_x_compare
                    : (axis == 1) ? box_y_compare
                                  : box_z_compare;

    size_t object_span = end - start;
    std::cout << "Object span is " << object_span << ".\n";

    if (object_span == 1) {
        left = right = objects[start];
    } else if (object_span == 2) {
        if (comparator(objects[start], objects[start+1])) {
            left = objects[start];
            right = objects[start+1];
        } else {
            left = objects[start+1];
            right = objects[start];
        }
    } else {
        std::sort(objects.begin() + start, objects.begin() + end, comparator);

        auto mid = start + object_span/2;
        left = std::make_shared<bvh_node>(objects, start, mid, time0, time1);
        right = std::make_shared<bvh_node>(objects, mid, end, time0, time1);
    }

    aabb box_left, box_right, outer_box;

    // outer_box = box_left;
    // std::cout << "Box_left, prior:\n";
    // std::cout << outer_box.minimum.x() << "," << outer_box.minimum.y() << "," << outer_box.minimum.z() << ";\n";
    // std::cout << outer_box.maximum.x() << "," << outer_box.maximum.y() << "," << outer_box.maximum.z() << ";\n";

    // outer_box = box_right;
    // std::cout << "Box_left, prior:\n";
    // std::cout << outer_box.minimum.x() << "," << outer_box.minimum.y() << "," << outer_box.minimum.z() << ";\n";
    // std::cout << outer_box.maximum.x() << "," << outer_box.maximum.y() << "," << outer_box.maximum.z() << ";\n";

    if (  !left->bounding_box (time0, time1, box_left)
       || !right->bounding_box(time0, time1, box_right)
    )
        std::cerr << "No bounding box in bvh_node constructor.\n";

    // outer_box = box_left;
    // std::cout << "Box_left, post:\n";
    // std::cout << outer_box.minimum.x() << "," << outer_box.minimum.y() << "," << outer_box.minimum.z() << ";\n";
    // std::cout << outer_box.maximum.x() << "," << outer_box.maximum.y() << "," << outer_box.maximum.z() << ";\n";

    // outer_box = box_right;
    // std::cout << "Box_left, post:\n";
    // std::cout << outer_box.minimum.x() << "," << outer_box.minimum.y() << "," << outer_box.minimum.z() << ";\n";
    // std::cout << outer_box.maximum.x() << "," << outer_box.maximum.y() << "," << outer_box.maximum.z() << ";\n";

    box = surrounding_box(box_left, box_right);

    // outer_box = box;
    // std::cout << "Overbox, agglomerated:\n";
    // std::cout << outer_box.minimum.x() << "," << outer_box.minimum.y() << "," << outer_box.minimum.z() << ";\n";
    // std::cout << outer_box.maximum.x() << "," << outer_box.maximum.y() << "," << outer_box.maximum.z() << ";\n";

    std::cout << "\n";
}