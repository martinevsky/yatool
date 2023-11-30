PY23_LIBRARY()

STYLE_PYTHON()

PY_SRCS(
    NAMESPACE exts
    archive.py
    asyncthread.py
    compress.py
    compatible23.py
    copy2.py
    datetime2.py
    decompress.py
    detect_recursive_dict.py
    filelock.py
    flatten.py
    fs.py
    func.py
    hashing.py
    http_client.py
    http_server.py
    io2.py
    log.py
    os2.py
    path2.py
    plocker.py
    process.py
    retry.py
    shlex2.py
    strings.py
    strtobool.py
    timer.py
    tmp.py
    uniq_id.py
    which.py
    windows.py
    yjdump.py
    yjson.py
)

IF (PYTHON2)
    PEERDIR(
        contrib/deprecated/python/scandir
        contrib/deprecated/python/typing
    )
ENDIF()

PEERDIR(
    contrib/deprecated/python/ujson
    contrib/python/portalocker
    contrib/python/simplejson
    contrib/python/six
    devtools/ya/yalibrary/streaming_json_dumper
    library/python/archive
    library/python/cityhash
    library/python/compress
    library/python/filelock
    library/python/func
    library/python/fs
    library/python/tmp
    library/python/unique_id
    library/python/json
    library/python/retry
    library/python/strings
    library/python/windows
)

END()
