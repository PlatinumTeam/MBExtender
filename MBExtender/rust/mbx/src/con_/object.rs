use crate::con::Argv;
use mbx_proc::{inherits, virtual_destructor, vtable};
use std::ffi::{c_void, CStr};
use std::os::raw::c_char;

#[repr(C)]
#[vtable(ConsoleObjectVtable)]
#[virtual_destructor]
pub struct ConsoleObject;

vtable! {
    pub struct ConsoleObjectVtable {
        fn get_class_rep(this: &ConsoleObject) -> *mut c_void;
        fn ~ConsoleObject();
    }
}

#[repr(C)]
#[vtable(SimObjectVtable)]
#[inherits(ConsoleObject)]
pub struct SimObject {
    pub name: *const c_char,
    pub next_name_object: *mut SimObject,
    pub next_manager_name_object: *mut SimObject,
    pub next_id_object: *mut SimObject,
    pub group: *mut c_void,
    pub flags: u32,
    pub notify_list: *mut c_void,
    pub id: u32,
    pub namespace: *mut c_void,
    pub type_mask: u32,
    pub field_dictionary: *mut c_void,
}

impl SimObject {
    /// Get the SimObject's name.
    pub fn name(&self) -> Option<&str> {
        unsafe { self.name.as_ref().map(|s| CStr::from_ptr(s).to_str().unwrap()) }
    }
}

vtable! {
    pub struct SimObjectVtable: ConsoleObjectVtable {
        fn process_arguments(this: &SimObject, argc: i32, argv: Argv);
        fn on_add(this: &SimObject);
        fn on_remove(this: &SimObject);
        fn on_group_add(this: &SimObject);
        fn on_group_remove(this: &SimObject);
        fn on_name_change(this: &SimObject, name: *const c_char);
        fn on_static_modified(this: &SimObject, slot_name: *const c_char);
        fn inspect_pre_apply(this: &SimObject);
        fn inspect_post_apply(this: &SimObject);
        fn on_video_kill(this: &SimObject);
        fn on_video_resurrect(this: &SimObject);
        fn on_delete_notify(this: &SimObject, object: *mut SimObject);
        fn on_editor_enable(this: &SimObject);
        fn on_editor_disable(this: &SimObject);
        fn get_editor_class_name(this: &SimObject, buffer: *mut c_char) -> *mut c_char;
        fn find_object(this: &SimObject, name: *const c_char) -> *mut SimObject;
        fn write(this: &SimObject, stream: *mut c_void, tab_stop: u32, flags: u32);
        fn register_lights(this: &SimObject, light_manager: *mut c_void, lighting_scene: bool);
    }
}
