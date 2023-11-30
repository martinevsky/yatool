# Generated by devtools/yamaker (pypi).

PY3_LIBRARY()

VERSION(20230430)

LICENSE(Apache-2.0)

NO_LINT()

PY_SRCS(
    TOP_LEVEL
    diff_match_patch/__init__.py
    diff_match_patch/__version__.py
    diff_match_patch/diff_match_patch.py
)

RESOURCE_FILES(
    PREFIX contrib/python/diff-match-patch/py3/
    .dist-info/METADATA
    .dist-info/top_level.txt
)

END()

RECURSE_FOR_TESTS(
    tests
)