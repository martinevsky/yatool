PY23_LIBRARY()

PY_SRCS(
    resolve_clang_coverage.py
)

PEERDIR(
    contrib/python/ujson
    contrib/python/six
    devtools/common/libmagic
    devtools/ya/exts
    devtools/ya/test/programs/test_tool/lib/coverage
    devtools/ya/test/programs/test_tool/lib/runtime
    devtools/ya/test/util
    library/python/testing/yatest_common
)

END()