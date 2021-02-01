use super::{Quat16, ToolVec};
use crate::core::{ResourceInstance, ResourceInstanceVtable, TgeVec};
use crate::math::Point3F;
use mbx_proc::{inherits, vtable};
use std::ffi::c_void;
use std::os::raw::c_char;

tge_methods! {
    pub fn tge_compute_accelerator(this: &mut TsShape, dl: i32) = tge_addr!(0x403553, 0x1cc530);
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct ConvexHullAccelerator {
    pub num_verts: i32,
    pub vertex_list: *mut Point3F,
    pub normal_list: *mut Point3F,
    pub emit_strings: *mut *mut u8,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct Decal {
    pub name_index: i32,
    pub num_meshes: i32,
    pub start_mesh_index: i32,
    pub object_index: i32,
    pub next_sibling: i32,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct DecalState {
    pub frame_index: i32,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct Detail {
    pub name_index: i32,
    pub sub_shape_num: i32,
    pub object_detail_num: i32,
    pub size: f32,
    pub average_error: f32,
    pub max_error: i32,
    pub poly_count: i32,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct IflMaterial {
    pub name_index: i32,
    pub material_slot: i32,
    pub first_frame: i32,
    pub first_frame_off_time_index: i32,
    pub num_frames: i32,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct Node {
    pub name_index: i32,
    pub parent_index: i32,
    pub first_object: i32,
    pub first_child: i32,
    pub next_sibling: i32,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct Object {
    pub name_index: i32,
    pub num_meshes: i32,
    pub start_mesh_index: i32,
    pub node_index: i32,
    pub next_sibling: i32,
    pub first_decal: i32,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct ObjectState {
    pub vis: f32,
    pub frame_index: i32,
    pub mat_frame_index: i32,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct Sequence {
    // TODO
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct Trigger {
    state: u32,
    pos: f32,
}

#[repr(C)]
#[vtable(TsShapeVtable)]
#[inherits(ResourceInstance)]
pub struct TsShape {
    pub nodes: ToolVec<Node>,
    pub objects: ToolVec<Object>,
    pub decals: ToolVec<Decal>,
    pub ifl_materials: ToolVec<IflMaterial>,
    pub object_states: ToolVec<ObjectState>,
    pub decal_states: ToolVec<DecalState>,
    pub sub_shape_first_node: ToolVec<i32>,
    pub sub_shape_first_object: ToolVec<i32>,
    pub sub_shape_first_decal: ToolVec<i32>,
    pub detail_first_skin: ToolVec<i32>,
    pub sub_shape_num_nodes: ToolVec<i32>,
    pub sub_shape_num_objects: ToolVec<i32>,
    pub sub_shape_num_decals: ToolVec<i32>,
    pub details: ToolVec<Detail>,
    pub default_rotations: ToolVec<Quat16>,
    pub default_translations: ToolVec<Point3F>,
    pub sub_shape_first_translucent_object: ToolVec<i32>,
    pub meshes: ToolVec<*mut c_void>,
    pub alpha_in: ToolVec<f32>,
    pub alpha_out: ToolVec<f32>,
    pub sequences: TgeVec<Sequence>,
    pub node_rotations: TgeVec<Quat16>,
    pub node_translations: TgeVec<Point3F>,
    pub node_uniform_scales: TgeVec<f32>,
    pub node_aligned_scales: TgeVec<Point3F>,
    pub node_arbitrary_scale_rots: TgeVec<Quat16>,
    pub node_arbitrary_scale_factors: TgeVec<Point3F>,
    pub ground_rotations: TgeVec<Quat16>,
    pub ground_translations: TgeVec<Point3F>,
    pub triggers: TgeVec<Trigger>,
    pub ifl_frame_off_times: TgeVec<f32>,
    pub billboard_details: TgeVec<*mut c_void>,
    pub detail_collision_accelerators: TgeVec<*mut ConvexHullAccelerator>,
    pub names: TgeVec<*const c_char>,
    // TODO
}

vtable! {
    pub struct TsShapeVtable: ResourceInstanceVtable;
}
