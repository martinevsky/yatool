# Generated by devtools/yamaker (pypi).

PY3_LIBRARY()

VERSION(15.0.1)

LICENSE(MIT)

PEERDIR(
    contrib/python/humanfriendly
)

NO_LINT()

PY_SRCS(
    TOP_LEVEL
    coloredlogs/__init__.py
    coloredlogs/cli.py
    coloredlogs/converter/__init__.py
    coloredlogs/converter/colors.py
    coloredlogs/demo.py
    coloredlogs/syslog.py
)

RESOURCE_FILES(
    PREFIX contrib/python/coloredlogs/py3/
    .dist-info/METADATA
    .dist-info/entry_points.txt
    .dist-info/top_level.txt
)

END()
