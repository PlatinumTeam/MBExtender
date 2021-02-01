/// Trait for looking up an object's vtable.
pub trait Vtable {
    /// The type of the object's table.
    type Vtable;

    /// Get a reference to the object's vtable.
    fn vtable(&self) -> &Self::Vtable;
}
