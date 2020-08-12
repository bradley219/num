# num

Command line number converter for programmers and hackers.

## Usage 

```
Usage: num [OPTIONS] <from>
<from> is an integer that may be specified as one of the following formats: 31. 0x1f 037 b11111
OPTIONS
-o --output [FORMAT]  Set the output format. Default is to output in all formats. Available formats: (dec|hex|oct|bin).
-h --help             Display this help message.
-v --version          Display version number.
```

Example:

```
$ num 1027.
Format detected as decimal
dec      hex       oct       bin
1027     0x403     02003     100 0000 0011
                             10000000011
```

## Install

```
gcc ./num.c -o num
sudo install -o root num /usr/local/bin
```
