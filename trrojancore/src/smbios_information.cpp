/// <copyright file="smbios_information.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

/// <copyright file="smbios_information.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2015 Visualisierungsinstitut der Universität Stuttgart.
/// Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/smbios_information.h"

#ifdef _WIN32
#include <Windows.h>
#endif /* _WIN32 */

#include "trrojan/io.h"


static const char *BIOS_CHARACTERISTICS[] = {
    "Reserved.",
    "Reserved.",
    "Unknown.",
    "BIOS Characteristics are not supported."
    "ISA is supported.",
    "MCA is supported.",
    "EISA is supported.",
    "PCI is supported.",
    "PC card (PCMCIA) is supported.",
    "Plug and Play is supported.",
    "APM is supported.",
    "BIOS is upgradeable (Flash)."
    "BIOS shadowing is allowed.",
    "VL - VESA is supported.",
    "ESCD support is available.",
    "Boot from CD is supported.",
    "Selectable boot is supported.",
    "BIOS ROM is socketed.",
    "Boot from PC card (PCMCIA) is supported.",
    "EDD specification is supported.",
    "Int 13h - Japanese floppy for NEC 9800 1.2 MB (3.5\", 1K bytes/sector, 360 RPM) is supported.",
    "Int 13h - Japanese floppy for Toshiba 1.2 MB (3.5\", 360 RPM) is supported.",
    "Int 13h - 5.25\"/360 KB floppy services are supported.",
    "Int 13h - 5.25\"/1.2 MB floppy services are supported.",
    "Int 13h - 3.5\"/720 KB floppy services are supported.",
    "Int 13h - 3.5\"/2.88 MB floppy services are supported.",
    "Int 5h - print screen service is supported.",
    "Int 9h - 8042 keyboard services are supported.",
    "Int 14h - serial services are supported.",
    "Int 17h - printer services are supported.",
    "Int 10h - CGA/Mono video Services are supported.",
    "NEC PC - 98.",
    //Bits32:47 Reserved for BIOS vendor.
    //Bits 48 : 63 Reserved for system vendor.
};


static const char *BIOS_EXTENSIONS1[] = {
    /* 0x00 */ "ACPI is supported.",
    /* 0x01 */ "USB Legacy is supported.",
    /* 0x02 */ "AGP is supported.",
    /* 0x03 */ "I2O boot is supported.",
    /* 0x04 */ "LS - 120 SuperDisk boot is supported.",
    /* 0x05 */ "ATAPI ZIP drive boot is supported.",
    /* 0x06 */ "1394 boot is supported.",
    /* 0x07 */ "Smart battery is supported."
};


static const char *BIOS_EXTENSIONS2[] = {
    /* 0x00 */ "BIOS Boot Specification is supported.",
    /* 0x01 */ "Function key-initiated network service boot is supported.",
    /* 0x02 */ "Enable targeted content distribution.",
    /* 0x03 */ "UEFI Specification is supported.",
    /* 0x04 */ "SMBIOS table describes a virtual machine.",
    /* 0x05 */ "Reserved 5",
    /* 0x06 */ "Reserved 6",
    /* 0x07 */ "Reserved 7",
};


static const char *CHASSIS_TYPES[] = {
    /* 0x00 */ "",
    /* 0x01 */ "Other",
    /* 0x02 */ "Unknown",
    /* 0x03 */ "Desktop",
    /* 0x04 */ "Low Profile Desktop",
    /* 0x05 */ "Pizza Box",
    /* 0x06 */ "Mini Tower",
    /* 0x07 */ "Tower",
    /* 0x08 */ "Portable",
    /* 0x09 */ "Laptop",
    /* 0x0A */ "Notebook",
    /* 0x0B */ "Hand Held",
    /* 0x0C */ "Docking Station",
    /* 0x0D */ "All in One",
    /* 0x0E */ "Sub Notebook",
    /* 0x0F */ "Space-saving",
    /* 0x10 */ "Lunch Box",
    /* 0x11 */ "Main Server Chassis",
    /* 0x12 */ "Expansion Chassis",
    /* 0x13 */ "SubChassis",
    /* 0x14 */ "Bus Expansion Chassis",
    /* 0x15 */ "Peripheral Chassis",
    /* 0x16 */ "RAID Chassis",
    /* 0x17 */ "Rack Mount Chassis",
    /* 0x18 */ "Sealed-case PC",
    /* 0x19 */ "Multi-system chassis",
    /* 0x1A */ "Compact PCI",
    /* 0x1B */ "Advanced TCA",
    /* 0x1C */ "Blade,"
    /* 0x1D */ "Blade Enclosure,"
    /* 0x1E */ "Tablet",
    /* 0x1F */ "Convertible",
    /* 0x20 */ "Detachable"
};


static const char *PROCESSOR_FAMILIES[] = {
    /* 0x0000 */ "",
    /* 0x0001 */ "Other",
    /* 0x0002 */ "Unknown",
    /* 0x0003 */ "8086",
    /* 0x0004 */ "80286",
    /* 0x0005 */ "Intel386™ processor",
    /* 0x0006 */ "Intel486™ processor",
    /* 0x0007 */ "8087",
    /* 0x0008 */ "80287",
    /* 0x0009 */ "80387",
    /* 0x000A */ "80487",
    /* 0x000B */ "Intel® Pentium® processor",
    /* 0x000C */ "Pentium® Pro processor",
    /* 0x000D */ "Pentium® II processor",
    /* 0x000E */ "Pentium® processor with MMX™ technology",
    /* 0x000F */ "Intel® Celeron® processor",
    /* 0x0010 */ "Pentium® II Xeon™ processor",
    /* 0x0011 */ "Pentium® III processor",
    /* 0x0012 */ "M1 Family",
    /* 0x0013 */ "M2 Family",
    /* 0x0014 */ "Intel® Celeron® M processor",
    /* 0x0015 */ "Intel® Pentium® 4 HT processor"
    /* 0x0016 */ "[Available for assignment]",
    /* 0x0017 */ "[Available for assignment]",
    /* 0x0018 */ "AMD Duron™ Processor Family",
    /* 0x0019 */ "K5 Family",
    /* 0x001A */ "K6 Family",
    /* 0x001B */ "K6-2",
    /* 0x001C */ "K6-3",
    /* 0x001D */ "AMD Athlon™ Processor Family",
    /* 0x001E */ "AMD29000 Family",
    /* 0x001F */ "K6-2+",
    /* 0x0020 */ "Power PC Family",
    /* 0x0021 */ "Power PC 601",
    /* 0x0022 */ "Power PC 603",
    /* 0x0023 */ "Power PC 603+",
    /* 0x0024 */ "Power PC 604",
    /* 0x0025 */ "Power PC 620",
    /* 0x0026 */ "Power PC x704",
    /* 0x0027 */ "Power PC 750",
    /* 0x0028 */ "Intel® Core™ Duo processor",
    /* 0x0029 */ "Intel® Core™ Duo mobile processor",
    /* 0x002A */ "Intel® Core™ Solo mobile processor",
    /* 0x002B */ "Intel® Atom™ processor",
    /* 0x002C */ "Intel® Core™ M processor",
    /* 0x002D */ "[Available for assignment]",
    /* 0x002E */ "[Available for assignment]",
    /* 0x002F */ "[Available for assignment]",
    /* 0x0030 */ "Alpha Family",
    /* 0x0031 */ "Alpha 21064",
    /* 0x0032 */ "Alpha 21066",
    /* 0x0033 */ "Alpha 21164",
    /* 0x0034 */ "Alpha 21164PC",
    /* 0x0035 */ "Alpha 21164a",
    /* 0x0036 */ "Alpha 21264",
    /* 0x0037 */ "Alpha 21364",
    /* 0x0038 */ "AMD Turion™ II Ultra Dual-Core Mobile M Processor Family",
    /* 0x0039 */ "AMD Turion™ II Dual-Core Mobile M Processor Family",
    /* 0x003A */ "AMD Athlon™ II Dual-Core M Processor Family",
    /* 0x003B */ "AMD Opteron™ 6100 Series Processor",
    /* 0x003C */ "AMD Opteron™ 4100 Series Processor",
    /* 0x003D */ "AMD Opteron™ 6200 Series Processor",
    /* 0x003E */ "AMD Opteron™ 4200 Series Processor",
    /* 0x003F */ "AMD FX™ Series Processor",
    /* 0x0040 */ "MIPS Family",
    /* 0x0041 */ "MIPS R4000",
    /* 0x0042 */ "MIPS R4200",
    /* 0x0043 */ "MIPS R4400",
    /* 0x0044 */ "MIPS R4600",
    /* 0x0045 */ "MIPS R10000",
    /* 0x0046 */ "AMD C-Series Processor",
    /* 0x0047 */ "AMD E-Series Processor",
    /* 0x0048 */ "AMD A-Series Processor",
    /* 0x0049 */ "AMD G-Series Processor",
    /* 0x004A */ "AMD Z-Series Processor",
    /* 0x004B */ "AMD R-Series Processor",
    /* 0x004C */ "AMD Opteron™ 4300 Series Processor",
    /* 0x004D */ "AMD Opteron™ 6300 Series Processor",
    /* 0x004E */ "AMD Opteron™ 3300 Series Processor,"
    /* 0x004F */ "AMD FirePro™ Series Processor",
    /* 0x0050 */ "SPARC Family",
    /* 0x0051 */ "SuperSPARC",
    /* 0x0052 */ "microSPARC II",
    /* 0x0053 */ "microSPARC IIep",
    /* 0x0054 */ "UltraSPARC",
    /* 0x0055 */ "UltraSPARC II",
    /* 0x0056 */ "UltraSPARC Iii",
    /* 0x0057 */ "UltraSPARC III",
    /* 0x0058 */ "UltraSPARC IIIi",
    /* 0x0059 */ "[Available for assignment]",
    /* 0x005A */ "[Available for assignment]",
    /* 0x005B */ "[Available for assignment]",
    /* 0x005C */ "[Available for assignment]",
    /* 0x005D */ "[Available for assignment]",
    /* 0x005E */ "[Available for assignment]",
    /* 0x005F */ "[Available for assignment]",
    /* 0x0060 */ "68040 Family",
    /* 0x0061 */ "68xxx",
    /* 0x0062 */ "68000",
    /* 0x0063 */ "68010",
    /* 0x0064 */ "68020",
    /* 0x0065 */ "68030",
    /* 0x0066 */ "AMD Athlon(TM) X4 Quad-Core Processor Family",
    /* 0x0067 */ "AMD Opteron(TM) X1000 Series Processor",
    /* 0x0068 */ "AMD Opteron(TM) X2000 Series APU",
    /* 0x0069 */ "[Available for assignment]",
    /* 0x006A */ "[Available for assignment]",
    /* 0x006B */ "[Available for assignment]",
    /* 0x006C */ "[Available for assignment]",
    /* 0x006D */ "[Available for assignment]",
    /* 0x006E */ "[Available for assignment]",
    /* 0x006F */ "[Available for assignment]",
    /* 0x0070 */ "Hobbit Family",
    /* 0x0071 */ "[Available for assignment]",
    /* 0x0072 */ "[Available for assignment]",
    /* 0x0073 */ "[Available for assignment]",
    /* 0x0074 */ "[Available for assignment]",
    /* 0x0075 */ "[Available for assignment]",
    /* 0x0076 */ "[Available for assignment]",
    /* 0x0077 */ "[Available for assignment]",
    /* 0x0078 */ "Crusoe™ TM5000 Family",
    /* 0x0079 */ "Crusoe™ TM3000 Family",
    /* 0x007A */ "Efficeon™ TM8000 Family",
    /* 0x007B */ "[Available for assignment]",
    /* 0x007C */ "[Available for assignment]",
    /* 0x007D */ "[Available for assignment]",
    /* 0x007E */ "[Available for assignment]",
    /* 0x007F */ "[Available for assignment]",
    /* 0x0080 */ "Weitek",
    /* 0x0081 */ "[Available for assignment]",
    /* 0x0082 */ "Itanium™ processor",
    /* 0x0083 */ "AMD Athlon™ 64 Processor Family",
    /* 0x0084 */ "AMD Opteron™ Processor Family",
    /* 0x0085 */ "AMD Sempron™ Processor Family",
    /* 0x0086 */ "AMD Turion™ 64 Mobile Technology",
    /* 0x0087 */ "Dual-Core AMD Opteron™ Processor Family",
    /* 0x0088 */ "AMD Athlon™ 64 X2 Dual-Core Processor Family",
    /* 0x0089 */ "AMD Turion™ 64 X2 Mobile Technology",
    /* 0x008A */ "Quad-Core AMD Opteron™ Processor Family",
    /* 0x008B */ "Third-Generation AMD Opteron™ Processor Family",
    /* 0x008C */ "AMD Phenom™ FX Quad-Core Processor Family",
    /* 0x008D */ "AMD Phenom™ X4 Quad-Core Processor Family",
    /* 0x008E */ "AMD Phenom™ X2 Dual-Core Processor Family",
    /* 0x008F */ "AMD Athlon™ X2 Dual-Core Processor Family",
    /* 0x0090 */ "PA-RISC Family",
    /* 0x0091 */ "PA-RISC 8500",
    /* 0x0092 */ "PA-RISC 8000",
    /* 0x0093 */ "PA-RISC 7300LC",
    /* 0x0094 */ "PA-RISC 7200",
    /* 0x0095 */ "PA-RISC 7100LC",
    /* 0x0096 */ "PA-RISC 7100",
    /* 0x0097 */ "[Available for assignment]",
    /* 0x0098 */ "[Available for assignment]",
    /* 0x0099 */ "[Available for assignment]",
    /* 0x009A */ "[Available for assignment]",
    /* 0x009B */ "[Available for assignment]",
    /* 0x009C */ "[Available for assignment]",
    /* 0x009D */ "[Available for assignment]",
    /* 0x009E */ "[Available for assignment]",
    /* 0x009F */ "[Available for assignment]",
    /* 0x00A0 */ "V30 Family",
    /* 0x00A1 */ "Quad-Core Intel® Xeon® processor 3200 Series",
    /* 0x00A2 */ "Dual-Core Intel® Xeon® processor 3000 Series",
    /* 0x00A3 */ "Quad-Core Intel® Xeon® processor 5300 Series",
    /* 0x00A4 */ "Dual-Core Intel® Xeon® processor 5100 Series",
    /* 0x00A5 */ "Dual-Core Intel® Xeon® processor 5000 Series",
    /* 0x00A6 */ "Dual-Core Intel® Xeon® processor LV",
    /* 0x00A7 */ "Dual-Core Intel® Xeon® processor ULV",
    /* 0x00A8 */ "Dual-Core Intel® Xeon® processor 7100 Series",
    /* 0x00A9 */ "Quad-Core Intel® Xeon® processor 5400 Series",
    /* 0x00AA */ "Quad-Core Intel® Xeon® processor",
    /* 0x00AB */ "Dual-Core Intel® Xeon® processor 5200 Series",
    /* 0x00AC */ "Dual-Core Intel® Xeon® processor 7200 Series",
    /* 0x00AD */ "Quad-Core Intel® Xeon® processor 7300 Series",
    /* 0x00AE */ "Quad-Core Intel® Xeon® processor 7400 Series",
    /* 0x00AF */ "Multi-Core Intel® Xeon® processor 7400 Series",
    /* 0x00B0 */ "Pentium® III Xeon™ processor",
    /* 0x00B1 */ "Pentium® III Processor with Intel® SpeedStep™ Technology",
    /* 0x00B2 */ "Pentium® 4 Processor",
    /* 0x00B3 */ "Intel® Xeon® processor",
    /* 0x00B4 */ "AS400 Family",
    /* 0x00B5 */ "Intel® Xeon™ processor MP",
    /* 0x00B6 */ "AMD Athlon™ XP Processor Family",
    /* 0x00B7 */ "AMD Athlon™ MP Processor Family",
    /* 0x00B8 */ "Intel® Itanium® 2 processor",
    /* 0x00B9 */ "Intel® Pentium® M processor",
    /* 0x00BA */ "Intel® Celeron® D processor",
    /* 0x00BB */ "Intel® Pentium® D processor",
    /* 0x00BC */ "Intel® Pentium® Processor Extreme Edition",
    /* 0x00BD */ "Intel® Core™ Solo Processor",
    /* 0x00BE */ "Reserved",
    /* 0x00BF */ "Intel® Core™ 2 Duo Processor",
    /* 0x00C0 */ "Intel® Core™ 2 Solo processor",
    /* 0x00C1 */ "Intel® Core™ 2 Extreme processor",
    /* 0x00C2 */ "Intel® Core™ 2 Quad processor",
    /* 0x00C3 */ "Intel® Core™ 2 Extreme mobile processor",
    /* 0x00C4 */ "Intel® Core™ 2 Duo mobile processor",
    /* 0x00C5 */ "Intel® Core™ 2 Solo mobile processor",
    /* 0x00C6 */ "Intel® Core™ i7 processor",
    /* 0x00C7 */ "Dual-Core Intel® Celeron® processor",
    /* 0x00C8 */ "IBM390 Family",
    /* 0x00C9 */ "G4",
    /* 0x00CA */ "G5",
    /* 0x00CB */ "ESA/390 G6",
    /* 0x00CC */ "z/Architecture base",
    /* 0x00CD */ "Intel® Core™ i5 processor",
    /* 0x00CE */ "Intel® Core™ i3 processor",
    /* 0x00CF */ "[Available for assignment]",
    /* 0x00D0 */ "[Available for assignment]",
    /* 0x00D1 */ "[Available for assignment]",
    /* 0x00D2 */ "VIA C7™-M Processor Family",
    /* 0x00D3 */ "VIA C7™-D Processor Family",
    /* 0x00D4 */ "VIA C7™ Processor Family",
    /* 0x00D5 */ "VIA Eden™ Processor Family",
    /* 0x00D6 */ "Multi-Core Intel® Xeon® processor",
    /* 0x00D7 */ "Dual-Core Intel® Xeon® processor 3xxx Series",
    /* 0x00D8 */ "Quad-Core Intel® Xeon® processor 3xxx Series",
    /* 0x00D9 */ "VIA Nano™ Processor Family",
    /* 0x00DA */ "Dual-Core Intel® Xeon® processor 5xxx Series",
    /* 0x00DB */ "Quad-Core Intel® Xeon® processor 5xxx Series",
    /* 0x00DC */ "[Available for assignment]",
    /* 0x00DD */ "Dual-Core Intel® Xeon® processor 7xxx Series",
    /* 0x00DE */ "Quad-Core Intel® Xeon® processor 7xxx Series",
    /* 0x00DF */ "Multi-Core Intel® Xeon® processor 7xxx Series",
    /* 0x00E0 */ "Multi-Core Intel® Xeon® processor 3400 Series"
    /* 0x00E1 */ "[Available for assignment]",
    /* 0x00E2 */ "[Available for assignment]",
    /* 0x00E3 */ "[Available for assignment]",
    /* 0x00E4 */ "AMD Opteron™ 3000 Series Processor",
    /* 0x00E5 */ "AMD Sempron™ II Processor",
    /* 0x00E6 */ "Embedded AMD Opteron™ Quad-Core Processor Family",
    /* 0x00E7 */ "AMD Phenom™ Triple-Core Processor Family",
    /* 0x00E8 */ "AMD Turion™ Ultra Dual-Core Mobile Processor Family",
    /* 0x00E9 */ "AMD Turion™ Dual-Core Mobile Processor Family",
    /* 0x00EA */ "AMD Athlon™ Dual-Core Processor Family",
    /* 0x00EB */ "AMD Sempron™ SI Processor Family",
    /* 0x00EC */ "AMD Phenom™ II Processor Family",
    /* 0x00ED */ "AMD Athlon™ II Processor Family",
    /* 0x00EE */ "Six-Core AMD Opteron™ Processor Family",
    /* 0x00EF */ "AMD Sempron™ M Processor Family",
    /* 0x00F0 */ "[Available for assignment]",
    /* 0x00F1 */ "[Available for assignment]",
    /* 0x00F2 */ "[Available for assignment]",
    /* 0x00F3 */ "[Available for assignment]",
    /* 0x00F4 */ "[Available for assignment]",
    /* 0x00F5 */ "[Available for assignment]",
    /* 0x00F6 */ "[Available for assignment]",
    /* 0x00F7 */ "[Available for assignment]",
    /* 0x00F8 */ "[Available for assignment]",
    /* 0x00F9 */ "[Available for assignment]",
    /* 0x00FA */"i860",
    /* 0x00FB */"i960",
    /* 0x00FC */"[Available for assignment]",
    /* 0x00FD */"[Available for assignment]",
    /* 0x00FE */"Indicator to obtain the processor family from the Processor Family 2 field",
    /* 0x00FF */"Reserved",
    /* 0x0100 */ "[Available for assignment]",
    /* 0x0101 */ "[Available for assignment]",
    /* 0x0102 */ "[Available for assignment]",
    /* 0x0103 */ "[Available for assignment]",
    /* 0x0104 */  "SH-3",
    /* 0x0105 */  "SH-4",
    /* 0x0106 */ "[Available for assignment]",
    /* 0x0107 */ "[Available for assignment]",
    /* 0x0108 */ "[Available for assignment]",
    /* 0x0109 */ "[Available for assignment]",
    /* 0x010A */ "[Available for assignment]",
    /* 0x010B */ "[Available for assignment]",
    /* 0x010C */ "[Available for assignment]",
    /* 0x010D */ "[Available for assignment]",
    /* 0x010E */ "[Available for assignment]",
    /* 0x010F */ "[Available for assignment]",
    /* 0x0110 */ "[Available for assignment]",
    /* 0x0111 */ "[Available for assignment]",
    /* 0x0112 */ "[Available for assignment]",
    /* 0x0113 */ "[Available for assignment]",
    /* 0x0114 */ "[Available for assignment]",
    /* 0x0115 */ "[Available for assignment]",
    /* 0x0116 */ "[Available for assignment]",
    /* 0x0117 */ "[Available for assignment]",
    /* 0x0118 */ "ARM",
    /* 0x0110 */ "StrongARM",
    /* 0x011A */ "[Available for assignment]",
    /* 0x011B */ "[Available for assignment]",
    /* 0x011C */ "[Available for assignment]",
    /* 0x011D */ "[Available for assignment]",
    /* 0x011E */ "[Available for assignment]",
    /* 0x011F */ "[Available for assignment]",
};


static const char *PROCESSOR_UPGRADES[] = {
    "",
    "Other",
    "Unknown",
    "Daughter Board",
    "ZIF Socket",
    "Replaceable Piggy Back",
    "None",
    "LIF Socket",
    "Slot 1",
    "Slot 2",
    "370-pin socket",
    "Slot A",
    "Slot M",
    "Socket 423",
    "Socket A (Socket 462)",
    "Socket 478",
    "Socket 754",
    "Socket 940",
    "Socket 939",
    "Socket mPGA604",
    "Socket LGA771",
    "Socket LGA775",
    "Socket S1",
    "Socket AM2",
    "Socket F (1207)",
    "Socket LGA1366",
    "Socket G34",
    "Socket AM3",
    "Socket C32",
    "Socket LGA1156",
    "Socket LGA1567",
    "Socket PGA988A",
    "Socket BGA1288",
    "Socket rPGA988B",
    "Socket BGA1023",
    "Socket BGA1224",
    "Socket LGA1155",
    "Socket LGA1356",
    "Socket LGA2011",
    "Socket FS1",
    "Socket FS2",
    "Socket FM1",
    "Socket FM2",
    "Socket LGA2011-3",
    "Socket LGA1356-3",
    "Socket LGA1150",
    "Socket BGA1168",
    "Socket BGA1234",
    "Socket BGA1364"
};


static const char *PROCESSOR_CHARACTERISTICS[] = {
    "Reserved",
    "Unknown",
    "64-bit Capable",
    "Multi-Core",
    "Hardware Thread",
    "Execute Protection",
    "Enhanced Virtualization",
    "Power/Performance Control",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};


static const char *MEMORY_CONTROLLER_ERROR_DETECTING_METHODS[] = {
    "",
    "Other",
    "Unknown",
    "None",
    "8-bit Parity",
    "32-bit ECC",
    "64-bit ECC",
    "128-bit ECC",
    "CRC"
};


static const char *MEMORY_CONTROLLER_ERROR_CORRECTION_CAPABILITIES[] = {
    "Other",
    "Unknown",
    "None",
    "Single-Bit Error Correcting",
    "Double-Bit Error Correcting",
    "Error Scrubbing"
};


static const char *MEMORY_CONTROLLER_INTERLEAVES[] = {
    "",
    "Other",
    "Unknown",
    "One-Way Interleave",
    "Two-Way Interleave",
    "Four-Way Interleave",
    "Eight-Way Interleave",
    "Sixteen-Way Interleave",
};


static const char *MEMORY_CONTROLLER_SPEEDS[] = {
    "Other",
    "Unknown",
    "70ns",
    "60ns",
    "50ns",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};


static const char *MEMORY_MODULE_TYPES[] = {
    "Other",
    "Unknown",
    "Standard",
    "Fast Page Mode",
    "EDO",
    "Parity",
    "ECC",
    "SIMM",
    "DIMM",
    "Burst EDO",
    "SDRAM",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};


static const char *CACHE_SRAM_TYPES[] = {
    "Other",
    "Unknown",
    "Non-Burst",
    "Burst",
    "Pipeline Burst",
    "Synchronous",
    "Asynchronous",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};


static const char *CACHE_ERROR_CORRECTION_TYPES[] = {
    "",
    "Other",
    "Unknown",
    "None",
    "Parity",
    "Single-bit ECC",
    "Multi-bit ECC"
};


static const char *CACHE_TYPES[] = {
    "",
    "Other",
    "Unknown",
    "Instruction",
    "Data",
    "Unified"
};


static const char *CACHE_ASSOCIATIVITIES[] = {
    "",
    "Other",
    "Unknown",
    "Direct Mapped",
    "2-way Set-Associative",
    "4-way Set-Associative",
    "Fully Associative",
    "8-way Set-Associative",
    "16-way Set-Associative",
    "12-way Set-Associative",
    "24-way Set-Associative",
    "32-way Set-Associative",
    "48-way Set-Associative",
    "64-way Set-Associative",
    "20-way Set-Associative"
};


//static const char *MEMORY_ARRAY_LOCATIONS[] = {
//    "",
//1    "Other",
//2    "Unknown",
//3    "System board or motherboard",
//4    "ISA add - on card",
//5    "EISA add - on card",
//6    "PCI add - on card",
//7    "MCA add - on card",
//8    "PCMCIA add - on card",
//9    "Proprietary add - on card",
//A    "NuBus",
//a0    "PC - 98 / C20 add - on card",
//a1   "PC - 98 / C24 add - on card",
//a2    "PC - 98 / E add - on card",
// a3   "PC - 98 / Local bus add - on card"
//};


static const char *MEMORY_ARRAY_USES[] = {
    "",
    "Other",
    "Unknown",
    "System memory",
    "Video memory",
    "Flash memory",
    "Non-volatile RAM",
    "Cache memory"
};


static const char *MEMORY_ARRAY_ERROR_CORRECTION_TYPES[] = {
    "",
    "Other",
    "Unknown",
    "None",
    "Parity",
    "Single-bit ECC",
    "Multi-bit ECC",
    "CRC"
};


static const char *MEMORY_DEVICE_FORM_FACTORS[] = {
    "",
    "Other",
    "Unknown",
    "SIMM",
    "SIP",
    "Chip",
    "DIP",
    "ZIP",
    "Proprietary Card",
    "DIMM",
    "TSOP",
    "Row of chips",
    "RIMM",
    "SODIMM",
    "SRIMM",
    "FB-DIMM"
};


static const char *MEMORY_DEVICE_TYPES[] = {
    "",
    "Other",
    "Unknown",
    "DRAM",
    "EDRAM",
    "VRAM",
    "SRAM",
    "RAM",
    "ROM",
    "FLASH",
    "EEPROM",
    "FEPROM",
    "EPROM",
    "CDRAM",
    "3DRAM",
    "SDRAM",
    "SGRAM",
    "RDRAM",
    "DDR",
    "DDR2",
    "DDR2 FB-DIMM",
    "Reserved",
    "Reserved",
    "Reserved",
    "DDR3",
    "FBD2",
    "DDR4",
    "LPDDR",
    "LPDDR2",
    "LPDDR3",
    "LPDDR4"
};


static const char *MEMORY_DEVICE_TYPE_DETAILS[] = {
    "Reserved",
    "Other",
    "Unknown",
    "Fast-paged",
    "Static column",
    "Pseudo-static",
    "RAMBUS",
    "Synchronous",
    "CMOS",
    "EDO",
    "Window DRAM",
    "Cache DRAM",
    "Non-volatile",
    "Registered (Buffered)",
    "Unbuffered (Unregistered)",
    "LRDIMM"
};


static const char *MEMORY_ERROR_TYPES[] = {
    "",
    "Other",
    "Unknown",
    "OK",
    "Bad read",
    "Parity error",
    "Single-bit error",
    "Double-bit error",
    "Multi-bit error",
    "Nibble error",
    "Checksum error",
    "CRC error",
    "Corrected single-bit error",
    "Corrected error",
    "Uncorrectable error"
};


static const char *MEMORY_ERROR_GRANULARITIES[] = {
    "",
    "Other",
    "Unknown",
    "Device level",
    "Memory partition level"
};


static const char *MEMORY_ERROR_OPERATIONS[] = {
    "",
    "Other",
    "Read",
    "Write",
    "Partial Write"
};



/*
 * trrojan::smbios_information::decode_memory_device_form_factor
 */
std::string trrojan::smbios_information::decode_memory_device_form_factor(
        const byte_type value) {
    auto c = std::size(MEMORY_DEVICE_FORM_FACTORS);
    return (value < c) ? MEMORY_DEVICE_FORM_FACTORS[value] : "";
}


/*
 * trrojan::smbios_information::decode_memory_device_type
 */
std::string trrojan::smbios_information::decode_memory_device_type(
        const byte_type value) {
    auto c = std::size(MEMORY_DEVICE_TYPES);
    return (value < c) ? MEMORY_DEVICE_TYPES[value] : "";
}


/*
 * trrojan::smbios_information::decode_memory_device_type_detail
 */
std::string trrojan::smbios_information::decode_memory_device_type_detail(
        const word_type value) {
    std::string retval;

    auto c = std::size(MEMORY_DEVICE_TYPE_DETAILS);
    auto v = value;
    for (size_t i = 0; i < c; ++i) {
        auto m = static_cast<word_type>(1 << i);
        if ((v & m) != 0) {
            retval += MEMORY_DEVICE_TYPE_DETAILS[i];

            // Erase current bit and if there is something left, add a
            // comma. Otherwise, we will not append anything any more.
            v &= ~m;
            if (v != 0) {
                retval += ", ";
            }
        }
    }

    return retval;
}


/*
 * trrojan::smbios_information::read
 */
trrojan::smbios_information trrojan::smbios_information::read(void) {
    smbios_information retval;

#if defined(_WIN32)
#pragma pack(push)
#pragma pack(1)
    // See https://msdn.microsoft.com/en-us/library/ms724379(VS.85).aspx
    struct RawSMBIOSData {
        BYTE Used20CallingMethod;
        BYTE SMBIOSMajorVersion;
        BYTE SMBIOSMinorVersion;
        BYTE DmiRevision;
        DWORD Length;
        //BYTE SMBIOSTableData[];
    };
#pragma pack(pop)

    /* Determine required buffer size. */
    {
        auto status = ::GetSystemFirmwareTable('RSMB', 0, nullptr, 0);
        if (status == 0) {
            throw std::exception("Failed to retrieve SMBIOS table size.");
        }
        retval.rawData.resize(status);
    }

    /* Read the data. */
    {
        auto status = ::GetSystemFirmwareTable('RSMB', 0, retval.rawData.data(),
            static_cast<DWORD>(retval.rawData.size()));
        if (status != retval.rawData.size()) {
            throw std::exception("Failed to retrieve SMBIOS table.");
        }
    }

    /* Determine the bounds of the structures. */
    retval.enumFlags = 0;
    retval.tableBegin = sizeof(RawSMBIOSData);
    retval.tableEnd = retval.rawData.size();


#else /* defined(_WIN32) */
    // TODO: implement this!
    auto ep = read_binary_file("/sys/firmware/dmi/tables/smbios_entry_point");
    // "/sys/firmware/dmi/tables/DMI"
#endif /* defined(_WIN32) */

    return std::move(retval);
}


/*
 * trrojan::smbios_information::smbios_information
 */
trrojan::smbios_information::smbios_information(const smbios_information& rhs)
    : enumFlags(rhs.enumFlags), rawData(rhs.rawData),
    tableBegin(rhs.tableBegin), tableEnd(rhs.tableEnd) { }


/*
 * trrojan::smbios_information::smbios_information
 */
trrojan::smbios_information::smbios_information(smbios_information&& rhs)
    : enumFlags(rhs.enumFlags), rawData(std::move(rhs.rawData)),
    tableBegin(rhs.tableBegin), tableEnd(rhs.tableEnd) { }


/*
 * trrojan::smbios_information::~smbios_information
 */
trrojan::smbios_information::~smbios_information(void) { }

/*
 * trrojan::smbios_information::operator =
 */
trrojan::smbios_information& trrojan::smbios_information::operator =(
        const smbios_information& rhs) {
    if (this != &rhs) {
        this->enumFlags = rhs.enumFlags;
        this->rawData = rhs.rawData;
        this->tableBegin = rhs.tableBegin;
        this->tableEnd = rhs.tableEnd;
    }
    return *this;
}


/*
 * trrojan::smbios_information::operator =
 */
trrojan::smbios_information& trrojan::smbios_information::operator =(
        smbios_information&& rhs) {
    if (this != &rhs) {
        this->enumFlags = rhs.enumFlags;
        this->rawData = std::move(rhs.rawData);
        this->tableBegin = rhs.tableBegin;
        this->tableEnd = rhs.tableEnd;
    }
    return *this;
}


/*
 * trrojan::smbios_information::validate_checksum
 */
bool trrojan::smbios_information::validate_checksum(const uint8_t *buf,
        const size_t len) {
    uint8_t sum = 0;

    for (size_t i = 0; i < len; ++i) {
        sum += buf[i];
    }

    return (sum == 0);
}
