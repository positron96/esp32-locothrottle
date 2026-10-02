Import("env")

if env["PIOENV"] == "native":
    env.Append(
        CPPPATH=[env.subst("$PROJECT_LIBDEPS_DIR/$PIOENV/lvgl")])
    env.BuildSources(
        "$BUILD_DIR/native_app",
        "$PROJECT_DIR/src_native")
