# Transfer List Compiler

The Transfer List Compiler (tlc) is a host tool used by TF-A to generate transfer
lists compliant with v0.9 of the [Firmware Handoff specification](https://github.com/FirmwareHandoff/firmware_handoff/).

It enables developers to statically generate transfer list blobs containing any
number of transfer entries.

## Getting Started

The Transfer List Compiler (`tlc`) is distributed as a Python package and can be
installed using [Poetry](https://python-poetry.org/).

To install `tlc` and its dependencies, clone the repository and run:

```bash
poetry install
```

This will create a virtual environment and install all dependencies declared in
tools/tlc/pyproject.toml

```bash
poetry run tlc --help
```

This will generate distribution files in the dist/ directory (e.g., `.whl` and `.tar.gz`).

To install the generated wheel into your environment, use `pip`:

```bash
pip install dist/tlc-<version>-py3-none-any.whl
```

### Packaging with Poetry

You can also package tlc as a Python wheel or source distribution. This allows
you to install it in any Python environment without relying on Poetry.

To build the package:

```bash
poetry build
```

## Creating a Transfer List

To create an empty TL, you can use the `create` command.

```bash
tlc create tl.bin
```

This command generates a binary blob representing an empty TL, shown in the
hexdump below:

```bash
$ hexdump tl.bin | head
0000000 b10b 4a0f 01a6 0318 0018 0000 1000 0000
0000010 0001 0000 0000 0000
```

A common use-case this tool supports is the addition of TE's via the option
`--entry`. This takes as input the tag ID and path to a binary blob to be
included in the transfer list. For example, including an FDT:

```bash
tlc create --entry 1 fdt.dtb tl.bin
```

Alternatively, addition of a device tree is supported through the option
`--fdt`. This has the same effect as passing the device tree and its tag ID
through the `--entry` option.

```bash
tlc create --fdt fdt.dtb tl.bin
```

> **Note** > `tlc` makes no effort to verify the contents of a binary blob
> against the provided tag ID. It only checks that the tags provided as input
> are within range and that there is sufficient memory to include their TE's.

You can also create a TL from a YAML config file:

```bash
tlc create --from-yaml config.yaml tl.bin
```

## Printing the contents of a TL

Support is provided for dumping the contents of a TL via the `info` command.
This prints the header of the TL and all included TE's.

```bash
$ tlc info tl.bin
signature  0x4a0fb10b
checksum   0xe1
version    0x1
hdr_size   0x18
alignment  0x3
size       0x2a6f
total_size 0x4e20
flags      0x1
----
id         0x1
data_size  0x2a47
hdr_size   0x8
offset     0x18
----
id         0x0
data_size  0x0
hdr_size   0x8
offset     0x2a68
```

The example above shows the dump produced by `tlc` for a 20Kb TL containing a
device tree (`tag_id=1`) and a NULL entry (`tag_id=0`).

## Modifying the contents of an existing TL

`tlc` supports removal of one or more entries from a TL through the `remove`
command. It takes as argument the filename, and one or more tag ID's, passed
through the `--tags` option. It produces a valid TL blob without those entries.

For example, removing the FDT TE:

```bash
$ tlc remove --tags 1 tl.bin
```

Verifying with `info`:

```bash
$ tlc info tl.bin
signature  0x4a0fb10b
checksum   0x38
version    0x1
hdr_size   0x18
alignment  0x3
size       0x20
total_size 0x4e20
flags      0x1
----
id         0x0
data_size  0x0
hdr_size   0x8
offset     0x18
```

> More than one entry can be removed at a time. The `--tags` option accepts multiple tag IDs.

Conversely, TE's can be added to an existing TL using the `add` command:

```bash
$ tlc add --entry 1 fdt.dtb tl.bin
```

Resulting TL:

```bash
$ tlc info tl.bin
signature  0x4a0fb10b
checksum   0xe1
version    0x1
hdr_size   0x18
alignment  0x3
size       0x2a6f
total_size 0x4e20
flags      0x1
----
id         0x0
data_size  0x0
hdr_size   0x8
offset     0x18
----
id         0x1
data_size  0x2a47
hdr_size   0x8
offset     0x20
```

## Unpacking a Transfer List

Given a transfer list, `tlc` also provides a mechanism for extracting TE data.
Running the `unpack` command yields binary files containing data from all the TE's.

```bash
$ tlc create --size 20000 --fdt build/fvp/debug/fdts/fvp-base-gicv3-psci.dtb tl.bin
$ tlc unpack tl.bin
$ file te_1.bin
te_1.bin: Device Tree Blob version 17, size=10823, boot CPU=0, string block size=851, DT structure block size=9900
```

## Validate a Transfer List

`tlc validate` provides a quick and simple mechanism for checking whether the TL
is compliant with the version of the specification supported by the tool. It
performs the following checks:

1. Validates the signature.
2. Ensures that the specified version is greater than or equal to the tool’s current version.
3. Verifies alignment criteria for all TE’s.

## YAML Config File Format

Example YAML config file:

```yaml
execution_state: aarch32
has_checksum: true
max_size: 4096
entries:
  - tag_id: 258 # entry point info
    ep_info:
      args:
        - 67112968
        - 67112960
        - 0
        - 0
        - 0
        - 0
        - 0
        - 0
      h:
        attr: 8
        type: 1
        version: 2
      pc: 67239936
      spsr: 467
  - tag_id: 3 # memory layout
    addr: 8
    size: 8
  - tag_id: 1 # fdt
    blob_file_path: "fdt.bin"
```

Defaults:

- `max_size` → `0x1000`
- `execution_state` → `aarch64`
- `has_checksum` → `true`

The fields of the YAML file should match those in the specification. You don’t
need to give `hdr_size` or `data_size`.

### Examples

**Memory layout entry:**

```yaml
tag_id: 3
addr: 8
size: 8
```

**FDT entry:**

```yaml
tag_id: 1
blob_file_path: "fdt.bin"
```

**Entry point info:**

```yaml
tag_id: 258
ep_info:
  args:
    - 67112968
    - 67112960
    - 0
    - 0
  h:
    attr: 8
    type: 1
    version: 2
  lr_svc: 0
  pc: 67239936
  spsr: 467
```

You can also use tag names instead of IDs. Valid tag names are in the
[`transfer_entry_formats`](https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/heads/master/tools/tlc/tlc/tl.py)
dict. Examples:

- `empty`
- `fdt`
- `hob_block`
- `hob_list`

You can input the `attr` field of `entry_point_info` as a string of flag
names separated by `|`, taken from `ep_info_exp.h` in TF-A:

```yaml
has_checksum: true
max_size: 4096
entries:
  - tag_id: 0x102
    ep_info:
      args:
        - 67112976
        - 67112960
        - 0
        - 0
        - 0
        - 0
        - 0
        - 0
      h:
        attr: EP_NON_SECURE | EP_ST_ENABLE
        type: 1
        version: 2
      pc: 67239936
      spsr: 965
```

