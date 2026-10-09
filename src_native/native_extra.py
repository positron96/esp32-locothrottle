Import("env")

if env["PIOENV"] == "native":
    env.Append(
        CPPPATH=[
            env.subst("$PROJECT_LIBDEPS_DIR/$PIOENV/lvgl"),
            env.subst("$PROJECT_LIBDEPS_DIR/$PIOENV/Embedded Template Library/include"),
        ],
        CXXFLAGS=["-Wno-deprecated-enum-enum-conversion"])
    env.BuildSources(
        "$BUILD_DIR/native_app",
        "$PROJECT_DIR/src_native")
