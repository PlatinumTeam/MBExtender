use crate::con::{SimObject, SimObjectVtable};
use mbx_proc::{inherits, vtable};

#[repr(C)]
#[vtable(NetObjectVtable)]
#[inherits(SimObject)]
pub struct NetObject;

vtable! {
    pub struct NetObjectVtable: SimObjectVtable {
        fn get_update_priority(this: &NetObject);
        fn pack_update(this: &NetObject);
        fn unpack_update(this: &NetObject);
        fn on_camera_scope_query(this: &NetObject);
    }
}

#[repr(C)]
#[vtable(SceneObjectVtable)]
#[inherits(NetObject)]
pub struct SceneObject;

vtable! {
    pub struct SceneObjectVtable: NetObjectVtable {
        fn disable_collision(this: &SceneObject);
        fn enable_collision(this: &SceneObject);
        fn is_displacable(this: &SceneObject);
        fn get_momentum(this: &SceneObject);
        fn set_momentum(this: &SceneObject);
        fn get_mass(this: &SceneObject);
        fn displace_object(this: &SceneObject);
        fn set_transform(this: &SceneObject);
        fn set_scale(this: &SceneObject);
        fn set_render_transform(this: &SceneObject);
        fn build_convex(this: &SceneObject);
        fn build_poly_list(this: &SceneObject);
        fn build_collision_bsp(this: &SceneObject);
        fn cast_ray(this: &SceneObject);
        fn collide_box(this: &SceneObject);
        fn get_overlapping_zones(this: &SceneObject);
        fn get_point_zone(this: &SceneObject);
        fn render_shadow_volumes(this: &SceneObject);
        fn render_object(this: &SceneObject);
        fn prep_render_image(this: &SceneObject);
        fn scope_object(this: &SceneObject);
        fn get_material_property(this: &SceneObject);
        fn on_scene_add(this: &SceneObject);
        fn on_scene_remove(this: &SceneObject);
        fn transform_modelview(this: &SceneObject);
        fn transform_position(this: &SceneObject);
        fn compute_new_frustum(this: &SceneObject);
        fn open_portal(this: &SceneObject);
        fn close_portal(this: &SceneObject);
        fn get_ws_portal_plane(this: &SceneObject);
        fn install_lights(this: &SceneObject);
        fn uninstall_lights(this: &SceneObject);
        fn get_lighting_ambient_color(this: &SceneObject);
    }
}
