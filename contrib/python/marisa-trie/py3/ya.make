# Generated by devtools/yamaker (pypi).

PY3_LIBRARY()

VERSION(1.1.0)

LICENSE(MIT)

PEERDIR(
    contrib/python/setuptools
)

ADDINCL(
    contrib/python/marisa-trie/include/marisa-trie/include
    contrib/python/marisa-trie/include/marisa-trie/lib
)

NO_COMPILER_WARNINGS()

NO_LINT()

SRCS(
    marisa-trie/lib/marisa/agent.cc
    marisa-trie/lib/marisa/grimoire/io/mapper.cc
    marisa-trie/lib/marisa/grimoire/io/reader.cc
    marisa-trie/lib/marisa/grimoire/io/writer.cc
    marisa-trie/lib/marisa/grimoire/trie/louds-trie.cc
    marisa-trie/lib/marisa/grimoire/trie/tail.cc
    marisa-trie/lib/marisa/grimoire/vector/bit-vector.cc
    marisa-trie/lib/marisa/keyset.cc
    marisa-trie/lib/marisa/trie.cc
)

PY_SRCS(
    TOP_LEVEL
    CYTHON_DIRECTIVE
    language_level=3
    CYTHON_CPP
    src/marisa_trie.pyx=marisa_trie
)

RESOURCE_FILES(
    PREFIX contrib/python/marisa-trie/py3/
    .dist-info/METADATA
    .dist-info/top_level.txt
)

END()
