const std = @import("std");

comptime {
    if (@bitSizeOf(c_uint) != 32) {
        @compileError("Wrong c_uint size. Not 32");
    }

    if (@bitSizeOf(usize) != 32) {
        @compileError("Wrong usize size. Not 32");
    }
}

export fn nformat_u32(buf: [*]c_char, len: c_uint, value: c_uint) c_int {
    const w = std.fmt.formatIntBuf(buf[0..len], value, 10, .lower, .{});
    return @intCast(w);
}

// export fn memcpy(dest: *anyopaque, source: *const anyopaque, size: usize) void {
//     const _d: [*]u8 = @ptrCast(dest);
//     const _s: [*]const u8 = @ptrCast(source);
//
//     @memcpy(_d[0..size], _s[0..size]);
// }
