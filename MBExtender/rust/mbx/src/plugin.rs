use crate::ffi;
use crate::interop::{FnPtr, OriginalFn};
use std::ffi::{c_void, CStr, CString};
use std::fmt::Display;
use std::mem;

/// A callback which runs immediately before main.cs.
pub type GameStartFn = extern "C" fn();

/// A callback which runs whenever clientProcess(U32) is called, before anything
/// else in the engine is updated.
pub type ClientProcessFn = extern "C" fn(delta_ms: u32);

/// A callback which runs after the GL context has been created and made
/// current.
pub type GlContextReadyFn = extern "C" fn();

/// A callback which runs before the GL context is deactivated and destroyed.
pub type GlContextDestroyFn = extern "C" fn();

/// A callback which runs after the onExit() script callback.
pub type GameExitFn = extern "C" fn();

/// A callback which runs when the plugin is unloaded after the game engine has
/// shut down.
pub type UnloadFn = extern "C" fn();

/// Wrapper struct for the MBX_Plugin C interface.
pub struct Plugin {
    plugin: &'static ffi::MBX_Plugin,
    pub name: &'static str,
    pub path: &'static str,
    pub build_pipeline: i32,
    pub build_hash: &'static str,
}

impl Plugin {
    /// Construct a new Plugin from a raw MBX_Plugin pointer.
    pub fn new(plugin_ptr: *const ffi::MBX_Plugin) -> Self {
        unsafe {
            let plugin = &*plugin_ptr;
            Plugin {
                plugin,
                name: CStr::from_ptr(plugin.name).to_str().unwrap(),
                path: CStr::from_ptr(plugin.path).to_str().unwrap(),
                build_pipeline: plugin.buildPipeline,
                build_hash: CStr::from_ptr(plugin.buildHash).to_str().unwrap(),
            }
        }
    }

    /// Check whether the plugin interface is compatible with this library.
    pub fn check_version(&self) -> bool {
        let my_version = ffi::MBX_PLUGIN_INTERFACE_VERSION;
        my_version <= self.plugin.version && my_version >= self.plugin.minVersion
    }

    /// Intercept a function so that all calls to it will redirect to another.
    pub fn intercept<T>(
        &self,
        func: FnPtr<T>,
        replacement: T,
        original: &OriginalFn<T>,
    ) -> Result<(), &'static str> {
        unsafe {
            let replacement_ptr: *mut c_void = mem::transmute_copy(&replacement);
            original.set({
                match self.intercept_raw(func.as_ptr(), replacement_ptr).as_ref() {
                    Some(f) => mem::transmute_copy(&f),
                    None => return Err("Interception failed"),
                }
            });
        }
        Ok(())
    }

    fn intercept_raw(&self, address: *mut c_void, replacement: *mut c_void) -> *mut c_void {
        unsafe { (*self.plugin.op).intercept.unwrap()(self.plugin, address, replacement) }
    }

    /// Register a callback to be fired when the engine has finished
    /// initializing. This is fired immediately before main.cs is run.
    pub fn on_game_start(&self, func: GameStartFn) {
        unsafe {
            (*self.plugin.op).onGameStart.unwrap()(self.plugin, Some(func));
        }
    }

    /// Register a callback to be fired whenever clientProcess(U32) is called.
    /// Callbacks will be fired before anything else in the engine is updated.
    pub fn on_client_process(&self, func: ClientProcessFn) {
        unsafe {
            (*self.plugin.op).onClientProcess.unwrap()(self.plugin, Some(func));
        }
    }

    /// Register a callback to be fired after the GL context has been created
    /// and made current.
    pub fn on_gl_context_ready(&self, func: GlContextReadyFn) {
        unsafe {
            (*self.plugin.op).onGlContextReady.unwrap()(self.plugin, Some(func));
        }
    }

    /// Register a callback to be fired before the GL context is deactivated and
    /// destroyed.
    pub fn on_gl_context_destroy(&self, func: GlContextDestroyFn) {
        unsafe {
            (*self.plugin.op).onGlContextDestroy.unwrap()(self.plugin, Some(func));
        }
    }

    /// Register a callback to be fired before the engine shuts down.
    /// This is fired immediately after the TorqueScript onExit() function.
    pub fn on_game_exit(&self, func: GameExitFn) {
        unsafe {
            (*self.plugin.op).onGameExit.unwrap()(self.plugin, Some(func));
        }
    }

    /// Register a callback to be fired when the plugin is unloaded.
    /// The engine will have already been shut down.
    pub fn on_unload(&self, func: UnloadFn) {
        unsafe {
            (*self.plugin.op).onUnload.unwrap()(self.plugin, Some(func));
        }
    }

    /// Set the error message to display if the plugin fails to load.
    /// #[plugin_main] will call this for you using the error you return.
    pub fn set_error<D: Display>(&self, error: D) {
        let message = format!("{}", error);
        let c_message = CString::new(message).unwrap();
        unsafe {
            (*self.plugin.op).setError.unwrap()(self.plugin, c_message.as_ptr());
        }
    }
}
