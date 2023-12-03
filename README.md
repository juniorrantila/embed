# The embed file generator for meson

**embed** is a meson generator for embedding files to C/C++ projects

## Usage

first, install the wrap file:

    meson wrap install embed

Then add the following line to your project's `meson.build`:

```meson

embed_text_gen = subproject('embed').get_variable('embed_text_gen')

```

Embedding files is done like the following:

```meson

hello_src = embed_text_gen.process('hello.txt')
executable('hello', [
  './hello.c',
  hello_src
])

```

C/C++ file:

```

#include <stdio.h>
#include <hello.h>

int main(void)
{
    printf("%s\n", hello);
    return 0
}

```

## Build instructions

### Setup:

```sh

meson setup build

```

### Build:

```sh

ninja -C build

```
