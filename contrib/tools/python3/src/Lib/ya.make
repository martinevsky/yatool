# Generated by devtools/yamaker.

ENABLE(PYBUILD_NO_PY)

PY3_LIBRARY()

VERSION(3.12.2)

ORIGINAL_SOURCE(https://github.com/python/cpython/archive/v3.12.2.tar.gz)

LICENSE(Python-2.0)

PEERDIR(
    certs
    contrib/tools/python3/lib/py
)

NO_LINT()

NO_PYTHON_INCLUDES()

PY_SRCS(
    TOP_LEVEL
    __future__.py
    __hello__.py
    _aix_support.py
    _collections_abc.py
    _compat_pickle.py
    _compression.py
    _markupbase.py
    _osx_support.py
    _py_abc.py
    _pydatetime.py
    _pydecimal.py
    _pyio.py
    _pylong.py
    _sitebuiltins.py
    _strptime.py
    _sysconfigdata_arcadia.py
    _threading_local.py
    _weakrefset.py
    abc.py
    aifc.py
    antigravity.py
    argparse.py
    ast.py
    asyncio/__init__.py
    asyncio/__main__.py
    asyncio/base_events.py
    asyncio/base_futures.py
    asyncio/base_subprocess.py
    asyncio/base_tasks.py
    asyncio/constants.py
    asyncio/coroutines.py
    asyncio/events.py
    asyncio/exceptions.py
    asyncio/format_helpers.py
    asyncio/futures.py
    asyncio/locks.py
    asyncio/log.py
    asyncio/mixins.py
    asyncio/proactor_events.py
    asyncio/protocols.py
    asyncio/queues.py
    asyncio/runners.py
    asyncio/selector_events.py
    asyncio/sslproto.py
    asyncio/staggered.py
    asyncio/streams.py
    asyncio/subprocess.py
    asyncio/taskgroups.py
    asyncio/tasks.py
    asyncio/threads.py
    asyncio/timeouts.py
    asyncio/transports.py
    asyncio/trsock.py
    asyncio/unix_events.py
    asyncio/windows_events.py
    asyncio/windows_utils.py
    base64.py
    bdb.py
    bisect.py
    bz2.py
    cProfile.py
    calendar.py
    cgi.py
    cgitb.py
    chunk.py
    cmd.py
    code.py
    codecs.py
    codeop.py
    collections/__init__.py
    collections/abc.py
    colorsys.py
    compileall.py
    concurrent/__init__.py
    concurrent/futures/__init__.py
    concurrent/futures/_base.py
    concurrent/futures/process.py
    concurrent/futures/thread.py
    configparser.py
    contextlib.py
    contextvars.py
    copy.py
    copyreg.py
    crypt.py
    csv.py
    ctypes/__init__.py
    ctypes/_aix.py
    ctypes/_endian.py
    ctypes/macholib/__init__.py
    ctypes/macholib/dyld.py
    ctypes/macholib/dylib.py
    ctypes/macholib/framework.py
    ctypes/util.py
    ctypes/wintypes.py
    curses/__init__.py
    curses/ascii.py
    curses/has_key.py
    curses/panel.py
    curses/textpad.py
    dataclasses.py
    datetime.py
    dbm/__init__.py
    dbm/dumb.py
    dbm/gnu.py
    dbm/ndbm.py
    decimal.py
    difflib.py
    dis.py
    doctest.py
    email/__init__.py
    email/_encoded_words.py
    email/_header_value_parser.py
    email/_parseaddr.py
    email/_policybase.py
    email/base64mime.py
    email/charset.py
    email/contentmanager.py
    email/encoders.py
    email/errors.py
    email/feedparser.py
    email/generator.py
    email/header.py
    email/headerregistry.py
    email/iterators.py
    email/message.py
    email/mime/__init__.py
    email/mime/application.py
    email/mime/audio.py
    email/mime/base.py
    email/mime/image.py
    email/mime/message.py
    email/mime/multipart.py
    email/mime/nonmultipart.py
    email/mime/text.py
    email/parser.py
    email/policy.py
    email/quoprimime.py
    email/utils.py
    encodings/__init__.py
    encodings/aliases.py
    encodings/ascii.py
    encodings/base64_codec.py
    encodings/big5.py
    encodings/big5hkscs.py
    encodings/bz2_codec.py
    encodings/charmap.py
    encodings/cp037.py
    encodings/cp1006.py
    encodings/cp1026.py
    encodings/cp1125.py
    encodings/cp1140.py
    encodings/cp1250.py
    encodings/cp1251.py
    encodings/cp1252.py
    encodings/cp1253.py
    encodings/cp1254.py
    encodings/cp1255.py
    encodings/cp1256.py
    encodings/cp1257.py
    encodings/cp1258.py
    encodings/cp273.py
    encodings/cp424.py
    encodings/cp437.py
    encodings/cp500.py
    encodings/cp720.py
    encodings/cp737.py
    encodings/cp775.py
    encodings/cp850.py
    encodings/cp852.py
    encodings/cp855.py
    encodings/cp856.py
    encodings/cp857.py
    encodings/cp858.py
    encodings/cp860.py
    encodings/cp861.py
    encodings/cp862.py
    encodings/cp863.py
    encodings/cp864.py
    encodings/cp865.py
    encodings/cp866.py
    encodings/cp869.py
    encodings/cp874.py
    encodings/cp875.py
    encodings/cp932.py
    encodings/cp949.py
    encodings/cp950.py
    encodings/euc_jis_2004.py
    encodings/euc_jisx0213.py
    encodings/euc_jp.py
    encodings/euc_kr.py
    encodings/gb18030.py
    encodings/gb2312.py
    encodings/gbk.py
    encodings/hex_codec.py
    encodings/hp_roman8.py
    encodings/hz.py
    encodings/idna.py
    encodings/iso2022_jp.py
    encodings/iso2022_jp_1.py
    encodings/iso2022_jp_2.py
    encodings/iso2022_jp_2004.py
    encodings/iso2022_jp_3.py
    encodings/iso2022_jp_ext.py
    encodings/iso2022_kr.py
    encodings/iso8859_1.py
    encodings/iso8859_10.py
    encodings/iso8859_11.py
    encodings/iso8859_13.py
    encodings/iso8859_14.py
    encodings/iso8859_15.py
    encodings/iso8859_16.py
    encodings/iso8859_2.py
    encodings/iso8859_3.py
    encodings/iso8859_4.py
    encodings/iso8859_5.py
    encodings/iso8859_6.py
    encodings/iso8859_7.py
    encodings/iso8859_8.py
    encodings/iso8859_9.py
    encodings/johab.py
    encodings/koi8_r.py
    encodings/koi8_t.py
    encodings/koi8_u.py
    encodings/kz1048.py
    encodings/latin_1.py
    encodings/mac_arabic.py
    encodings/mac_croatian.py
    encodings/mac_cyrillic.py
    encodings/mac_farsi.py
    encodings/mac_greek.py
    encodings/mac_iceland.py
    encodings/mac_latin2.py
    encodings/mac_roman.py
    encodings/mac_romanian.py
    encodings/mac_turkish.py
    encodings/mbcs.py
    encodings/oem.py
    encodings/palmos.py
    encodings/ptcp154.py
    encodings/punycode.py
    encodings/quopri_codec.py
    encodings/raw_unicode_escape.py
    encodings/rot_13.py
    encodings/shift_jis.py
    encodings/shift_jis_2004.py
    encodings/shift_jisx0213.py
    encodings/tis_620.py
    encodings/undefined.py
    encodings/unicode_escape.py
    encodings/utf_16.py
    encodings/utf_16_be.py
    encodings/utf_16_le.py
    encodings/utf_32.py
    encodings/utf_32_be.py
    encodings/utf_32_le.py
    encodings/utf_7.py
    encodings/utf_8.py
    encodings/utf_8_sig.py
    encodings/uu_codec.py
    encodings/zlib_codec.py
    ensurepip/__init__.py
    ensurepip/__main__.py
    ensurepip/_uninstall.py
    enum.py
    filecmp.py
    fileinput.py
    fnmatch.py
    fractions.py
    ftplib.py
    functools.py
    genericpath.py
    getopt.py
    getpass.py
    gettext.py
    glob.py
    graphlib.py
    gzip.py
    hashlib.py
    heapq.py
    hmac.py
    html/__init__.py
    html/entities.py
    html/parser.py
    http/__init__.py
    http/client.py
    http/cookiejar.py
    http/cookies.py
    http/server.py
    imaplib.py
    imghdr.py
    importlib/__init__.py
    importlib/_abc.py
    importlib/_bootstrap.py
    importlib/_bootstrap_external.py
    importlib/abc.py
    importlib/machinery.py
    importlib/metadata/__init__.py
    importlib/metadata/_adapters.py
    importlib/metadata/_collections.py
    importlib/metadata/_functools.py
    importlib/metadata/_itertools.py
    importlib/metadata/_meta.py
    importlib/metadata/_text.py
    importlib/readers.py
    importlib/resources/__init__.py
    importlib/resources/_adapters.py
    importlib/resources/_common.py
    importlib/resources/_itertools.py
    importlib/resources/_legacy.py
    importlib/resources/abc.py
    importlib/resources/readers.py
    importlib/resources/simple.py
    importlib/simple.py
    importlib/util.py
    inspect.py
    io.py
    ipaddress.py
    json/__init__.py
    json/decoder.py
    json/encoder.py
    json/scanner.py
    json/tool.py
    keyword.py
    linecache.py
    locale.py
    logging/__init__.py
    logging/config.py
    logging/handlers.py
    lzma.py
    mailbox.py
    mailcap.py
    mimetypes.py
    modulefinder.py
    msilib/__init__.py
    msilib/schema.py
    msilib/sequence.py
    msilib/text.py
    multiprocessing/__init__.py
    multiprocessing/connection.py
    multiprocessing/context.py
    multiprocessing/dummy/__init__.py
    multiprocessing/dummy/connection.py
    multiprocessing/forkserver.py
    multiprocessing/heap.py
    multiprocessing/managers.py
    multiprocessing/pool.py
    multiprocessing/popen_fork.py
    multiprocessing/popen_forkserver.py
    multiprocessing/popen_spawn_posix.py
    multiprocessing/popen_spawn_win32.py
    multiprocessing/process.py
    multiprocessing/queues.py
    multiprocessing/reduction.py
    multiprocessing/resource_sharer.py
    multiprocessing/resource_tracker.py
    multiprocessing/shared_memory.py
    multiprocessing/sharedctypes.py
    multiprocessing/spawn.py
    multiprocessing/synchronize.py
    multiprocessing/util.py
    netrc.py
    nntplib.py
    ntpath.py
    nturl2path.py
    numbers.py
    opcode.py
    operator.py
    optparse.py
    os.py
    pathlib.py
    pdb.py
    pickle.py
    pickletools.py
    pipes.py
    pkgutil.py
    platform.py
    plistlib.py
    poplib.py
    posixpath.py
    pprint.py
    profile.py
    pstats.py
    pty.py
    py_compile.py
    pyclbr.py
    pydoc.py
    pydoc_data/__init__.py
    pydoc_data/topics.py
    queue.py
    quopri.py
    random.py
    re/__init__.py
    re/_casefix.py
    re/_compiler.py
    re/_constants.py
    re/_parser.py
    reprlib.py
    rlcompleter.py
    runpy.py
    sched.py
    secrets.py
    selectors.py
    shelve.py
    shlex.py
    shutil.py
    signal.py
    site.py
    smtplib.py
    sndhdr.py
    socket.py
    socketserver.py
    sqlite3/__init__.py
    sqlite3/__main__.py
    sqlite3/dbapi2.py
    sqlite3/dump.py
    sre_compile.py
    sre_constants.py
    sre_parse.py
    ssl.py
    stat.py
    statistics.py
    string.py
    stringprep.py
    struct.py
    subprocess.py
    sunau.py
    symtable.py
    sysconfig.py
    tabnanny.py
    tarfile.py
    telnetlib.py
    tempfile.py
    textwrap.py
    this.py
    threading.py
    timeit.py
    token.py
    tokenize.py
    tomllib/__init__.py
    tomllib/_parser.py
    tomllib/_re.py
    tomllib/_types.py
    trace.py
    traceback.py
    tracemalloc.py
    tty.py
    turtle.py
    types.py
    typing.py
    unittest/__init__.py
    unittest/__main__.py
    unittest/_log.py
    unittest/async_case.py
    unittest/case.py
    unittest/loader.py
    unittest/main.py
    unittest/mock.py
    unittest/result.py
    unittest/runner.py
    unittest/signals.py
    unittest/suite.py
    unittest/util.py
    urllib/__init__.py
    urllib/error.py
    urllib/parse.py
    urllib/request.py
    urllib/response.py
    urllib/robotparser.py
    uu.py
    uuid.py
    venv/__init__.py
    venv/__main__.py
    warnings.py
    wave.py
    weakref.py
    webbrowser.py
    wsgiref/__init__.py
    wsgiref/handlers.py
    wsgiref/headers.py
    wsgiref/simple_server.py
    wsgiref/types.py
    wsgiref/util.py
    wsgiref/validate.py
    xdrlib.py
    xml/__init__.py
    xml/dom/NodeFilter.py
    xml/dom/__init__.py
    xml/dom/domreg.py
    xml/dom/expatbuilder.py
    xml/dom/minicompat.py
    xml/dom/minidom.py
    xml/dom/pulldom.py
    xml/dom/xmlbuilder.py
    xml/etree/ElementInclude.py
    xml/etree/ElementPath.py
    xml/etree/ElementTree.py
    xml/etree/__init__.py
    xml/etree/cElementTree.py
    xml/parsers/__init__.py
    xml/parsers/expat.py
    xml/sax/__init__.py
    xml/sax/_exceptions.py
    xml/sax/expatreader.py
    xml/sax/handler.py
    xml/sax/saxutils.py
    xml/sax/xmlreader.py
    xmlrpc/__init__.py
    xmlrpc/client.py
    xmlrpc/server.py
    zipapp.py
    zipfile/__init__.py
    zipfile/__main__.py
    zipfile/_path/__init__.py
    zipfile/_path/glob.py
    zipimport.py
    zoneinfo/__init__.py
    zoneinfo/_common.py
    zoneinfo/_tzpath.py
    zoneinfo/_zoneinfo.py
)

END()
