# TRRojan
The TRRojan is a benchmarking framework for visual computing applications developed in context of SFB/Transregio 161.

## Command line arguments of trrojan.exe

| Name                               | Description |
|---	                             |--- |
| `--trroll <path>`                  | Specifies the path to the TRRoll script to be executed. |
| `--output <path>`	                 | Specifies the path to the output file, which also determines its type. Outputs will be dumped to the console if this argument is missing. |
| `--log <path>`                     | Specifies the path to the log file. Status updates will be dumped to the console if this argument is missing. |
| `--visible`  	                     | If the output is an Excel sheet, show Excel while writing to it. |
| `--separator <string>`             | If the output is a CSV file, use the specified string as separator. This value defaults to "\t". |
| `--do-not-quote-strings`           | If the output is a CSV file, do not quote strings. |
| `--line-break <string>`            | If the output is a CSV file, use the specified new line string. The default depends on the platform. |
| `--line-break <string>`            | If the output is a CSV file, use the specified new line string. The default depends on the platform. |
| `--cool-down-frequency <minutes>`  | If not zero, instructs the benchmark to suspend execution after the given number of minutes. Not all benchmarks might support this. |
| `--cool-down-duration <seconds>`   | If not zero, instructs the benchmark to suspend execution for the given number of seconds after the `--cool-down-frequency` period has elapsed. Not all benchmarks might support this. |
| `--with-basic-render-driver`       | Specifies that the Microsoft Basic Render driver should be considered a valid device. By default, this software device is excluded from the Direct3D environment. |
| `--unique-devices`                 | If this flag is specified, the Direct3D 11 environment will skip a device if another device with the same PCI ID was already enumerated. |
| `--power <path>`                   | Starts collecting power usage samples in background and stores the data to the specified file. |
