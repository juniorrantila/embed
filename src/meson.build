embed_bin = executable('embed', [
  'main.c'
])

embed_text_gen = generator(embed_bin,
    output: ['@BASENAME@.c', '@BASENAME@.h'],
    arguments: ['@BUILD_DIR@', '@BASENAME@', '@INPUT@', 'text']
  )

embed_bin_gen = generator(embed_bin,
    output: ['@BASENAME@.c', '@BASENAME@.h'],
    arguments: ['@BUILD_DIR@', '@BASENAME@', '@INPUT@', 'bin']
  )
