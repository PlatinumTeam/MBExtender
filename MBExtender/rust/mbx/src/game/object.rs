use crate::sim::{SceneObject, SceneObjectVtable};
use mbx_proc::{inherits, vtable};

#[repr(C)]
#[vtable(GameBaseVtable)]
#[inherits(SceneObject)]
pub struct GameBase;

vtable! {
    pub struct GameBaseVtable: SceneObjectVtable {
        fn on_new_data_block(this: &GameBase);
        fn process_tick(this: &GameBase);
        fn interpolate_tick(this: &GameBase);
        fn advance_time(this: &GameBase);
        fn advance_physics(this: &GameBase);
        fn get_velocity(this: &GameBase);
        fn get_force(this: &GameBase);
        fn write_packet_data(this: &GameBase);
        fn read_packet_data(this: &GameBase);
        fn get_packet_data_checksum(this: &GameBase);
    }
}

#[repr(C)]
#[vtable(ShapeBaseVtable)]
#[inherits(GameBase)]
pub struct ShapeBase;

vtable! {
    pub struct ShapeBaseVtable: GameBaseVtable {
        fn set_image(this: &ShapeBase);
        fn on_image_recoil(this: &ShapeBase);
        fn eject_shell_casing(this: &ShapeBase);
        fn update_damage_level(this: &ShapeBase);
        fn update_damage_state(this: &ShapeBase);
        fn blow_up(this: &ShapeBase);
        fn on_mount(this: &ShapeBase);
        fn on_unmount(this: &ShapeBase);
        fn on_impact_sceneobject_point3f(this: &ShapeBase);
        fn on_impact_point3f(this: &ShapeBase);
        fn control_pre_packet_send(this: &ShapeBase);
        fn set_energy_level(this: &ShapeBase);
        fn mount_object(this: &ShapeBase);
        fn mount_image(this: &ShapeBase);
        fn unmount_image(this: &ShapeBase);
        fn get_muzzle_vector(this: &ShapeBase);
        fn get_camera_parameters(this: &ShapeBase);
        fn get_camera_transform(this: &ShapeBase);
        fn get_eye_transform(this: &ShapeBase);
        fn get_retraction_transform(this: &ShapeBase);
        fn get_mount_transform(this: &ShapeBase);
        fn get_muzzle_transform(this: &ShapeBase);
        fn get_image_transform_uint_matrixf(this: &ShapeBase);
        fn get_image_transform_uint_int_matrixf(this: &ShapeBase);
        fn get_image_transform_uint_constchar_matrixf(this: &ShapeBase);
        fn get_render_retraction_transform(this: &ShapeBase);
        fn get_render_mount_transform(this: &ShapeBase);
        fn get_render_muzzle_transform(this: &ShapeBase);
        fn get_render_image_transform_uint_matrixf(this: &ShapeBase);
        fn get_render_image_transform_uint_int_matrixf(this: &ShapeBase);
        fn get_render_image_transform_uint_constchar_matrixf(this: &ShapeBase);
        fn get_render_muzzle_vector(this: &ShapeBase);
        fn get_render_muzzle_point(this: &ShapeBase);
        fn get_render_eye_transform(this: &ShapeBase);
        fn get_damage_flash(this: &ShapeBase);
        fn set_damage_flash(this: &ShapeBase);
        fn get_white_out(this: &ShapeBase);
        fn set_white_out(this: &ShapeBase);
        fn get_invincible_effect(this: &ShapeBase);
        fn setup_invincible_effect(this: &ShapeBase);
        fn update_invincible_effect(this: &ShapeBase);
        fn set_velocity(this: &ShapeBase);
        fn apply_impulse(this: &ShapeBase);
        fn set_controlling_client(this: &ShapeBase);
        fn set_controlling_object(this: &ShapeBase);
        fn get_control_object(this: &ShapeBase);
        fn set_control_object(this: &ShapeBase);
        fn get_camera_fov(this: &ShapeBase);
        fn get_default_camera_fov(this: &ShapeBase);
        fn set_camera_fov(this: &ShapeBase);
        fn is_valid_camera_fov(this: &ShapeBase);
        fn render_mounted_image(this: &ShapeBase);
        fn render_image(this: &ShapeBase);
        fn calc_class_render_data(this: &ShapeBase);
        fn on_collision(this: &ShapeBase);
        fn get_surface_friction(this: &ShapeBase);
        fn get_bounce_friction(this: &ShapeBase);
        fn set_hidden(this: &mut ShapeBase, hidden: bool);
    }
}
