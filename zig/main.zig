const std = @import("std");

const HAL = @import("HAL");

export fn main() void {
    _ = HAL.nformat_u32;
    _ = HAL.BSP.commsAbort();
}
