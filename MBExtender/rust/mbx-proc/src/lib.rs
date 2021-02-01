use proc_macro::TokenStream;
use quote::*;
use std::env;
use syn::fold::Fold;
use syn::parse::{Parse, ParseStream};
use syn::punctuated::Punctuated;
use syn::spanned::Spanned;
use syn::*;

/// Generates a PluginMain export for a plugin entry point.
///
/// Accepted function forms:
///
/// fn NAME(ARG: &Plugin) {
///     ...
/// }
///
/// fn NAME(ARG: &Plugin) -> Result<..., ...> {
///     ...
/// }
#[proc_macro_attribute]
pub fn plugin_main(_attr: TokenStream, item: TokenStream) -> TokenStream {
    let func = parse_macro_input!(item as ItemFn);
    let func_ident = &func.sig.ident;
    let invocation = match func.sig.output {
        ReturnType::Default => {
            quote_spanned! {func.sig.span()=>
                #func_ident(&plugin);
                ::mbx::ffi::MBX_Status_MBX_OK
            }
        }
        _ => {
            quote_spanned! {func.sig.span()=>
                match #func_ident(&plugin) {
                    ::std::result::Result::Ok(_) => ::mbx::ffi::MBX_Status_MBX_OK,
                    ::std::result::Result::Err(err) => {
                        plugin.set_error(err);
                        ::mbx::ffi::MBX_Status_MBX_ERROR
                    }
                }
            }
        }
    };
    let result = quote! {
        #func

        #[no_mangle]
        pub extern "C" fn PluginMain(plugin_ptr: *const ::mbx::ffi::MBX_Plugin) -> ::mbx::ffi::MBX_Status {
            let plugin = ::mbx::Plugin::new(plugin_ptr);
            if !plugin.check_version() {
                return ::mbx::ffi::MBX_Status_MBX_ERROR_VERSION;
            }
            ::mbx::logger::init(&plugin);
            #invocation
        }

        #[cfg(target_os = "windows")]
        #[global_allocator]
        static __MBX_ALLOCATOR: ::mbx::PluginAllocator = ::mbx::PluginAllocator;
    };
    result.into()
}

/// Generates a function override definition and automatically expands calls to
/// the original function.
///
/// Example:
///
/// #[fn_override(original_create_window)]
/// unsafe fn my_create_window(width: i32, height: i32, fullscreen: bool) -> *mut c_void {
///     original_create_window(width, height, fullscreen)
/// }
#[proc_macro_attribute]
pub fn fn_override(attr: TokenStream, item: TokenStream) -> TokenStream {
    let original = parse_macro_input!(attr as Ident);
    let mut func = parse_macro_input!(item as ItemFn);

    // Use extern "C"
    if let Some(ref abi) = func.sig.abi {
        return Error::new(abi.span(), "function overrides must not specify an ABI")
            .to_compile_error()
            .into();
    }
    func.sig.abi = Some(parse_quote!(extern "C"));

    // Force unsafe
    if func.sig.unsafety.is_none() {
        return Error::new(func.sig.span(), "function overrides must be `unsafe`")
            .to_compile_error()
            .into();
    }

    let sig = &func.sig;
    let abi = &sig.abi;
    let inputs = &sig.inputs;
    let output = &sig.output;
    let result = quote_spanned! {sig.span()=>
        #[allow(non_upper_case_globals)]
        static #original: ::mbx::interop::OriginalFn<unsafe #abi fn (#inputs) #output> = ::mbx::interop::OriginalFn::new();
        #func
    };
    result.into()
}

/// Fold implementation which expands calls to the original overrided function.
struct ExpandMethodCalls {
    original: Expr,
}

impl ExpandMethodCalls {
    pub fn new(original: &Ident) -> Self {
        Self {
            original: parse_quote!(#original),
        }
    }
}

impl Fold for ExpandMethodCalls {
    fn fold_expr_call(self: &mut Self, node: ExprCall) -> ExprCall {
        let original = &self.original;
        if *node.func == *original {
            // Pass a null edx
            let mut args = node.args;
            args.insert(1, parse_quote!(::std::ptr::null_mut()));
            parse_quote!(#original(#args))
        } else {
            node
        }
    }
}

fn do_method_override(attr: TokenStream, item: TokenStream, windows: bool) -> TokenStream {
    let original = parse_macro_input!(attr as Ident);
    let mut func = parse_macro_input!(item as ItemFn);

    // Force unsafe
    if func.sig.unsafety.is_none() {
        return Error::new(func.sig.span(), "method overrides must be `unsafe`")
            .to_compile_error()
            .into();
    }

    if let Some(ref abi) = func.sig.abi {
        return Error::new(abi.span(), "method overrides must not specify an ABI")
            .to_compile_error()
            .into();
    }

    if windows {
        // Use fastcall to grab ecx (this) and edx and then insert an edx argument
        func.sig.abi = Some(parse_quote!(extern "fastcall"));
        func.sig.inputs.insert(1, parse_quote!(_edx: *const ()));
        let mut expand = ExpandMethodCalls::new(&original);
        func = expand.fold_item_fn(func);
    } else {
        func.sig.abi = Some(parse_quote!(extern "C"));
    }

    let sig = &func.sig;
    let abi = &sig.abi;
    let inputs = &sig.inputs;
    let output = &sig.output;
    let result = quote_spanned! {sig.span()=>
        #[allow(non_upper_case_globals)]
        static #original: ::mbx::interop::OriginalFn<unsafe #abi fn (#inputs) #output> = ::mbx::interop::OriginalFn::new();
        #func
    };
    result.into()
}

#[doc(hidden)]
#[proc_macro_attribute]
pub fn __impl_method_override_windows(attr: TokenStream, item: TokenStream) -> TokenStream {
    do_method_override(attr, item, true)
}

#[doc(hidden)]
#[proc_macro_attribute]
pub fn __impl_method_override_macos(attr: TokenStream, item: TokenStream) -> TokenStream {
    do_method_override(attr, item, false)
}

/// Generates a method override definition and automatically expands calls to
/// the original function.
///
/// Example:
///
/// #[method_override(original_marble_do_power_up)]
/// fn my_marble_do_power_up(this: *mut (), id: i32) {
///     unsafe { original_marble_do_power_up(this, id); }
/// }
#[proc_macro_attribute]
pub fn method_override(attr: TokenStream, item: TokenStream) -> TokenStream {
    let original = parse_macro_input!(attr as Ident);
    let func = parse_macro_input!(item as ItemFn);
    // cargo doesn't tell us the target, so we have to use cfg_attr as a workaround
    let result = quote_spanned! {func.span()=>
        #[cfg_attr(target_os = "windows", ::mbx::__impl_method_override_windows(#original))]
        #[cfg_attr(target_os = "macos", ::mbx::__impl_method_override_macos(#original))]
        #func
    };
    result.into()
}

/// Emulates inheritance by generating a field for a parent type along with a
/// Deref implementation.
///
/// Example:
///
/// #[repr(C)]
/// #[inherits(SimObject)]
/// pub struct NetObject {
///     ...
/// }
#[proc_macro_attribute]
pub fn inherits(attr: TokenStream, item: TokenStream) -> TokenStream {
    let parent_type = parse_macro_input!(attr as TypePath);
    let mut struct_item = parse_macro_input!(item as ItemStruct);

    // Insert the parent field at the beginning of the struct
    let new_fields: FieldsNamed = parse_quote!({ pub parent: #parent_type });
    match struct_item.fields {
        Fields::Named(ref mut fields) => {
            fields.named.insert(0, new_fields.named.first().unwrap().clone());
        }
        Fields::Unnamed(_) => {
            return Error::new(struct_item.fields.span(), "struct must have named fields")
                .to_compile_error()
                .into();
        }
        Fields::Unit => {
            struct_item.fields = Fields::Named(new_fields);
        }
    }

    // Implement Deref and DerefMut for the parent
    let struct_ident = &struct_item.ident;
    let parent_traits = quote_spanned! {parent_type.span()=>
        impl ::std::ops::Deref for #struct_ident {
            type Target = #parent_type;

            #[inline]
            fn deref(&self) -> &Self::Target {
                &self.parent
            }
        }
        impl ::std::ops::DerefMut for #struct_ident {
            #[inline]
            fn deref_mut(&mut self) -> &mut Self::Target {
                &mut self.parent
            }
        }
    };

    let result = quote_spanned! {struct_item.span()=>
        #struct_item
        #parent_traits
    };
    result.into()
}

#[proc_macro_attribute]
pub fn vtable(attr: TokenStream, item: TokenStream) -> TokenStream {
    let vtable_type = parse_macro_input!(attr as TypePath);
    let mut struct_item = parse_macro_input!(item as ItemStruct);

    // We want to know whether this is a subclass. There are two cases here:
    // 1. The #[inherits] attribute has not been processed yet. It will show up in the attrs list.
    // 2. The #[inherits] attribute has already been processed. We have to look for the parent field.
    let mut is_subclass = struct_item.attrs.iter().any(|a| match a.path.segments.last() {
        Some(segment) => segment.ident.to_string() == "inherits",
        None => false,
    });
    if !is_subclass {
        if let Fields::Named(ref fields) = struct_item.fields {
            if let Some(ref first) = fields.named.first() {
                if let Some(ref ident) = first.ident {
                    if ident.to_string() == "parent" {
                        is_subclass = true;
                    }
                }
            }
        }
    }

    // For the base class, insert the vtable field at the beginning of the struct.
    if !is_subclass {
        let new_fields: FieldsNamed = parse_quote!({ pub vtable: *const #vtable_type });
        match struct_item.fields {
            Fields::Named(ref mut fields) => {
                fields.named.insert(0, new_fields.named.first().unwrap().clone());
            }
            Fields::Unnamed(_) => {
                return Error::new(struct_item.fields.span(), "struct must have named fields")
                    .to_compile_error()
                    .into();
            }
            Fields::Unit => {
                struct_item.fields = Fields::Named(new_fields);
            }
        }
    }

    // This macro can be used inside mbx
    let c = if env::var("CARGO_PKG_NAME").unwrap() == "mbx" {
        quote!(crate)
    } else {
        quote!(::mbx)
    };

    // Implement Vtable.
    let struct_ident = &struct_item.ident;
    let vtable_trait = quote_spanned! {vtable_type.span()=>
        impl #c::interop::Vtable for #struct_ident {
            type Vtable = #vtable_type;

            #[inline]
            fn vtable(&self) -> &Self::Vtable {
                unsafe { &*self.vtable.cast() }
            }
        }
    };

    let result = quote_spanned! {struct_item.span()=>
        #struct_item
        #vtable_trait
    };
    result.into()
}

struct ConsoleFnMeta {
    pub class: Option<String>,
    pub usage: String,
    pub min_args: i32,
    pub max_args: i32,
}

impl Parse for ConsoleFnMeta {
    fn parse(input: ParseStream) -> parse::Result<Self> {
        let list: Punctuated<MetaNameValue, Token![,]> = Punctuated::parse_terminated(input)?;
        let mut class: Option<String> = None;
        let mut usage: Option<String> = None;
        let mut min_args: Option<i32> = None;
        let mut max_args: Option<i32> = None;
        for value in &list {
            let ident = value
                .path
                .get_ident()
                .ok_or_else(|| Error::new(value.path.span(), "expected an identifier"))?;
            let ident_name = format!("{}", ident);
            match ident_name.as_ref() {
                "args" => {
                    min_args = match value.lit {
                        Lit::Int(ref int_value) => Some(int_value.base10_parse()?),
                        _ => return Err(Error::new(value.lit.span(), "args must be an int")),
                    };
                    max_args = min_args;
                }
                "class" => {
                    class = match value.lit {
                        Lit::Str(ref str_value) => Some(str_value.value()),
                        _ => return Err(Error::new(value.lit.span(), "class must be a string")),
                    };
                }
                "max_args" => {
                    max_args = match value.lit {
                        Lit::Int(ref int_value) => Some(int_value.base10_parse()?),
                        _ => return Err(Error::new(value.lit.span(), "max_args must be an int")),
                    };
                }
                "min_args" => {
                    min_args = match value.lit {
                        Lit::Int(ref int_value) => Some(int_value.base10_parse()?),
                        _ => return Err(Error::new(value.lit.span(), "min_args must be an int")),
                    };
                }
                "usage" => {
                    usage = match value.lit {
                        Lit::Str(ref str_value) => Some(str_value.value()),
                        _ => return Err(Error::new(value.lit.span(), "usage must be a string")),
                    };
                }
                _ => {
                    return Err(Error::new(
                        ident.span(),
                        format!("unrecognized key `{}`", ident_name),
                    ));
                }
            }
        }
        Ok(Self {
            class: class,
            usage: usage.ok_or_else(|| Error::new(list.span(), "missing usage"))?,
            min_args: min_args.ok_or_else(|| Error::new(list.span(), "missing min_args"))?,
            max_args: max_args.ok_or_else(|| Error::new(list.span(), "missing max_args"))?,
        })
    }
}

#[proc_macro_attribute]
pub fn command(attr: TokenStream, item: TokenStream) -> TokenStream {
    let meta = parse_macro_input!(attr as ConsoleFnMeta);
    let mut func = parse_macro_input!(item as ItemFn);

    // Force ABI to extern "C"
    if let Some(ref abi) = func.sig.abi {
        return Error::new(abi.span(), "console functions must not specify an ABI")
            .to_compile_error()
            .into();
    }
    func.sig.abi = parse_quote!(extern "C");

    // The function will be replaced by a private function and a struct with the visibility of the
    // original function
    let vis = func.vis;
    func.vis = Visibility::Inherited;

    // Rename the function to __<name>()
    let public_ident = func.sig.ident;
    let private_name = format!("__{}", public_ident);
    let private_ident = Ident::new(&private_name, public_ident.span());
    func.sig.ident = private_ident.clone();

    // Try to detect the return type...
    let return_type_err = Error::new(
        func.sig.output.span(),
        "return type must be none, `bool`, `f32`, `i32`, or `*const c_char`",
    )
    .to_compile_error();
    let func_type: Ident = match func.sig.output {
        ReturnType::Default => parse_quote!(Void),
        ReturnType::Type(_, ref ty) => match (*ty).as_ref() {
            Type::Path(ref path) => {
                let type_ident = match path.path.get_ident() {
                    Some(name) => name,
                    None => return return_type_err.into(),
                };
                let type_name = format!("{}", type_ident);
                match type_name.as_ref() {
                    "bool" => parse_quote!(Bool),
                    "f32" => parse_quote!(Float),
                    "i32" => parse_quote!(Int),
                    _ => return return_type_err.into(),
                }
            }
            // Assume pointers are string returns
            Type::Ptr(_) => parse_quote!(String),
            _ => return return_type_err.into(),
        },
    };

    let class = match meta.class {
        Some(name) => quote! {Some(#name)},
        None => quote! {None},
    };
    let name = format!("{}", public_ident);
    let usage = meta.usage;
    let min_args = meta.min_args;
    let max_args = meta.max_args;
    let result = quote_spanned! {func.span()=>
        #[allow(non_snake_case)]
        #func

        #[allow(non_upper_case_globals)]
        #vis static #public_ident: ::mbx::con::Command = ::mbx::con::Command {
            class: #class,
            name: #name,
            usage: #usage,
            min_args: #min_args,
            max_args: #max_args,
            func: ::mbx::con::CommandFn::#func_type(self::#private_ident),
        };
    };
    result.into()
}

/// Generates a Drop implementation for a struct which has a virtual destructor.
/// This should only be used on the base class.
#[proc_macro_attribute]
pub fn virtual_destructor(_attr: TokenStream, item: TokenStream) -> TokenStream {
    let struct_item = parse_macro_input!(item as ItemStruct);

    // This macro can be used inside mbx
    let c = if env::var("CARGO_PKG_NAME").unwrap() == "mbx" {
        quote!(crate)
    } else {
        quote!(::mbx)
    };

    let struct_ident = &struct_item.ident;
    let result = quote! {
        #struct_item

        impl Drop for #struct_ident {
            #[inline]
            fn drop(&mut self) {
                unsafe {
                    let destructor = #c::interop::Vtable::vtable(self).__destructor;
                    destructor.invoke(self);
                }
            }
        }
    };
    result.into()
}
