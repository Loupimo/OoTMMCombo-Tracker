//! Build script: on Windows, embed the application icon (and basic metadata) as a
//! PE resource so the .exe file — and the taskbar button — carry the same icon as
//! the Qt build, rather than the default Rust/Windows icon. No effect elsewhere.

fn main() {
    #[cfg(windows)]
    {
        // Logo.ico lives at the repository root, next to the crate (../Resources).
        let icon = "../C++-Tracker/Resources/Logo.ico";
        println!("cargo:rerun-if-changed={icon}");
        let mut res = winresource::WindowsResource::new();
        res.set_icon(icon);
        res.set("ProductName", "OoTMMCombo Auto Tracker");
        res.set("FileDescription", "OoTMMCombo Auto Tracker");
        if let Err(e) = res.compile() {
            // Don't hard-fail the build if the Windows SDK's resource compiler is
            // unavailable; just surface it (the app still runs, minus the file icon).
            println!("cargo:warning=could not embed the window icon: {e}");
        }
    }
}
