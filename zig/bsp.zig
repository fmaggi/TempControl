extern fn BSP_Comms_receive_expect(buf: [*]c_char, size: u16) void;
pub fn commsReceiveExpect(buf: []u8, size: u16) void {
    BSP_Comms_receive_expect(@ptrCast(buf), size);
}

extern fn BSP_Comms_abort() void;
pub const commsAbort = BSP_Comms_abort;

extern fn BSP_Comms_received() u8;
pub const commsReceived = BSP_Comms_received;
