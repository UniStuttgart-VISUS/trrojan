# TRRojan
The TRRojan is a benchmarking framework for visual computing applications developed in context of SFB-TRR 161.

## Command line arguments of trrojan.exe

| Name                     | Description |
|---	                     |--- |
| `--trroll <path>`        | Specifies the path to the TRRoll script to be executed. |
| `--output <path>`	       | Specifies the path to the output file, which also determines its type. Outputs will be dumped to the console if this argument is missing. |
| `--log <path>`           | Specifies the path to the log file. Status updates will be dumped to the console if this argument is missing. |
| `--visible`  	           | If the output is an Excel sheet, show Excel while writing to it. |
| `--separator <string>`   | If the output is a CSV file, use the specified string as separator. This value defaults to "\t". |
| `--do-not-quote-strings` | If the output is a CSV file, do not quote strings. |
| `--line-break <string>`  | If the output is a CSV file, use the specified new line string. The default depends on the platform.|
