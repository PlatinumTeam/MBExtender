use super::{ShapeBase, ShapeBaseVtable};
use crate::math::Point3D;
use mbx_proc::{inherits, vtable};

tge_methods! {
    pub fn tge_marble_do_power_up(this: &mut Marble, id: i32) = tge_addr!(0x405f51, 0x2576b0);
}

#[repr(C)]
#[vtable(MarbleVtable)]
#[inherits(ShapeBase)]
pub struct Marble;

impl Marble {
    /// Get the marble's velocity.
    pub fn velocity(&self) -> Point3D {
        unsafe { *field_ptr!(self, Point3D, tge_addr!(0xa00, 0x9ec)) }
    }

    /// Set the marble's velocity.
    pub fn set_velocity(&mut self, velocity: Point3D) {
        unsafe {
            *field_ptr!(mut self, Point3D, tge_addr!(0xa00, 0x9ec)) = velocity;
        }
    }
}

vtable! {
    pub struct MarbleVtable: ShapeBaseVtable;
}
