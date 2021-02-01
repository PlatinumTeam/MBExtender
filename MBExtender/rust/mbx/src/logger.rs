use crate::con;
use crate::Plugin;
use log::{Level, LevelFilter, Log, Metadata, Record};

struct ConsoleLogger {
    plugin_name: &'static str,
}

impl Log for ConsoleLogger {
    fn enabled(&self, _metadata: &Metadata) -> bool {
        true
    }

    fn log(&self, record: &Record) {
        let msg = match record.level() {
            Level::Error | Level::Warn | Level::Info => {
                format!("{}: {}", self.plugin_name, record.args())
            }
            Level::Debug => format!("{}: DEBUG: {}", self.plugin_name, record.args()),
            Level::Trace => format!("{}: TRACE: {}", self.plugin_name, record.args()),
        };
        match record.level() {
            Level::Error => con::error(msg),
            Level::Warn => con::warn(msg),
            Level::Info => con::print(msg),
            Level::Debug => con::print(msg),
            Level::Trace => con::print(msg),
        }
    }

    fn flush(&self) {}
}

/// Sets the TGE console as the active logger.
/// This is automatically called by #[plugin_main].
pub fn init(plugin: &Plugin) {
    let logger = Box::new(ConsoleLogger {
        plugin_name: plugin.name,
    });
    log::set_boxed_logger(logger).unwrap();

    // Enable debug logging if this is a debug build or we're using a local extender build
    if cfg!(debug_assertions) || plugin.build_pipeline == 0 {
        log::set_max_level(LevelFilter::Trace);
    } else {
        log::set_max_level(LevelFilter::Info);
    }
}
