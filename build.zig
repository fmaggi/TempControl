const std = @import("std");

pub fn build(b: *std.Build) void {
    const target_query = std.zig.CrossTarget{
        .cpu_arch = .thumb,
        .os_tag = .freestanding,
        .abi = .eabi,
        .cpu_model = .{ .explicit = &std.Target.arm.cpu.cortex_m3 },
    };
    const target = b.resolveTargetQuery(target_query);

    // .Debug config doesn't seem to fit in flash
    const optimize = .ReleaseSmall;

    const debug_opt = b.option(bool, "Debug", "Define DEBUG macro");
    const debug = debug_opt orelse true;

    const exe = b.addExecutable(.{
        .name = "horno",
        .target = target,
        .optimize = optimize,
        .strip = false,
        .linkage = .static,
        .link_libc = false,
        .single_threaded = true,
    });

    if (debug) {
        exe.root_module.addCMacro("DEBUG", "1");
    }

    exe.bundle_compiler_rt = true;
    exe.link_function_sections = true;
    exe.link_data_sections = true;
    exe.link_gc_sections = true;

    for (Includes) |i| {
        exe.addIncludePath(.{ .path = i });
    }

    const flags = [_][]const u8{
        "-std=gnu11",
        "-DUSE_HAL_DRIVER",
        "-DSTM32F103xB",
        "-ffunction-sections",
        "-fdata-sections",
        "-nostdlib",
        "-nostdinc",
        "-Wall",
        "-Wextra",
        "-pedantic",
        "-fstack-usage",
        "-mthumb",
    };

    exe.addAssemblyFile(.{ .path = "Core/Startup/startup_stm32f103c8tx.s" });

    exe.addCSourceFiles(.{ .files = &DriverSources, .flags = &flags });
    exe.addCSourceFiles(.{ .files = &CoreSources, .flags = &flags });
    exe.addCSourceFiles(.{ .files = &AppSources, .flags = &flags });

    exe.entry = .{ .symbol_name = "Reset_Handler" };

    exe.setLinkerScript(.{ .path = "STM32F103C8TX_FLASH.ld" });

    const libc = b.createModule(.{
        .root_source_file = .{ .path = "zig/lib.zig" },
        .target = target,
        .optimize = optimize,
        .strip = false,
        .link_libc = false,
        .single_threaded = true,
    });

    exe.root_module.addImport("libc", libc);

    b.installArtifact(exe);
}

const Includes = [_][]const u8{
    "Drivers/STM32F1xx_HAL_Driver/Inc",
    "Drivers/CMSIS/Include",
    "Drivers/CMSIS/Device/ST/STM32F1xx/Include",
    "Core/Inc",
    "App/Inc",
    "zig",
    "libc/include",
};

const DriverSources = [_][]const u8{
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c",
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c",
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi.c",
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_exti.c",
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c",
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c",
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c",
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc_ex.c",
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c",
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c",
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c",
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c",
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c",
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc.c",
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c",
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c",
    "Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c",
};

const CoreSources = [_][]const u8{
    "Core/Src/bsp.c",
    "Core/Src/ILI9341_GFX.c",
    "Core/Src/syscalls.c",
    "Core/Src/stm32f1xx_it.c",
    "Core/Src/ILI9341_STM32_Driver.c",
    "Core/Src/power.c",
    "Core/Src/fonts.c",
    "Core/Src/msp_tim.c",
    "Core/Src/msp_adc.c",
    "Core/Src/msp_spi.c",
    "Core/Src/msp_usart.c",
    "Core/Src/system_stm32f1xx.c",
    "Core/Src/display.c",
    "Core/Src/temperature.c",
    "Core/Src/stm32f1xx_hal_msp.c",
    "Core/Src/sysmem.c",
    "Core/Src/io.c",
};

const AppSources = [_][]const u8{
    "App/Src/main.c",
    "App/Src/control.c",
    "App/Src/storage.c",
    "App/Src/ui.c",
};
