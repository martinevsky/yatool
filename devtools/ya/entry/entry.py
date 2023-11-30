from __future__ import print_function

import os
import six
import sys
import time
import signal
import core.config
import logging
import argparse

try:
    import cProfile as profile
except ImportError:
    import profile

import core.error
import core.respawn
import core.sig_handler
import core.yarg

from core.yarg import LazyCommand, try_load_handler
from core.logger import init_logger
from core.plugin_loader import explore_plugins
from library.python import mlockall
from library.python import svn_version

import app


def _mlockall():
    e = mlockall.mlockall_current()
    app.MLOCK_STATUS_MESSAGE = "mlockall return code: {!s}".format(e)


def do_main(args, extra_help):
    import app_config

    plugin_map = explore_plugins(
        loader_hook=try_load_handler,
        suffix='_handler',
    )

    handler = core.yarg.CompositeHandler(description=app_config.description, examples_count=5, extra_help=extra_help)
    for plugin_name in sorted(plugin_map.names()):
        handler[plugin_name] = LazyCommand(plugin_name, plugin_map.get(plugin_name))
    handler['-'] = core.yarg.FeedbackHandler(handler)
    res = handler.handle(handler, args, prefix=['ya'])
    if isinstance(res, six.integer_types):
        sys.exit(res)


def do_intercepted(func, interceptors):
    def wrap(f, wrapper):
        return lambda: wrapper(f)

    wrapped_func = func
    for interceptor in reversed(interceptors):
        wrapped_func = wrap(wrapped_func, interceptor)

    return wrapped_func()


class BadArg(Exception):
    pass


class ArgParse(argparse.ArgumentParser):
    def parse_max(self, args):
        cur = []
        gen = iter(args)
        res = self.parse_args([])

        for arg in gen:
            cur.append(arg)

            try:
                res, bad = self.parse_known_args(cur)

                if bad:
                    return res, [arg] + list(gen)
            except BadArg:
                pass

        return res, []

    def error(self, s):
        raise BadArg(s)


def setup_faulthandler():
    try:
        import faulthandler
    except ImportError:
        return

    # Dump python backtrace in case of any errors
    faulthandler.enable()
    if hasattr(signal, "SIGQUIT"):
        faulthandler.register(signal.SIGQUIT, chain=True)


def format_msg(args):
    ts = args['_timestamp']
    tn = args['_typename']

    if 'Message' in args:
        try:
            msg = '(' + str(args['PID']) + ') ' + args['Message']
        except KeyError:
            msg = args['Message']
    elif 'Started' in tn:
        msg = 'start ' + args['StageName']
    elif 'Finished' in tn:
        msg = 'end ' + args['StageName']
    else:
        msg = str(args)

    return ts / 1000000.0, msg


def main(args):
    if os.environ.get('Y_FAST_CANCEL', 'no') == 'yes':
        signal.signal(signal.SIGINT, core.sig_handler.instant_sigint_exit_handler)

    _mlockall()

    setup_faulthandler()

    try:
        import app_config

        use_opensource_config = not app_config.in_house
    except ImportError:
        use_opensource_config = False
    no_report = use_opensource_config
    p = ArgParse(prog='ya', add_help=False)

    # Do not forget add arguments with value to `skippable_flags` into /ya script
    p.add_argument('--precise', action='store_const', const=True, default=False, help='show precise timings in log')
    p.add_argument(
        '--profile', action='store_const', const=True, default=False, help='run python profiler for ya binary'
    )
    p.add_argument('--error-file')
    p.add_argument('--keep-tmp', action='store_const', const=True, default=False)
    p.add_argument(
        '--no-logs', action='store_const', const=True, default=True if os.environ.get('YA_NO_LOGS') else False
    )
    p.add_argument(
        '--no-report',
        action='store_const',
        const=True,
        default=True if os.environ.get('YA_NO_REPORT') in ("1", "yes") else no_report,
    )
    p.add_argument(
        '--no-tmp-dir',
        action='store_const',
        const=True,
        default=True if os.environ.get('YA_NO_TMP_DIR') in ("1", "yes") else False,
    )
    p.add_argument('--no-respawn', action='store_const', const=True, default=False, help=argparse.SUPPRESS)
    p.add_argument('--print-path', action='store_const', const=True, default=False)
    p.add_argument('--version', action='store_const', const=True, default=False)
    p.add_argument(
        '-v',
        '--verbose-level',
        action='store_const',
        const=logging.DEBUG,
        default=logging.DEBUG if os.environ.get('YA_VERBOSE') else logging.INFO,
    )
    if use_opensource_config:
        p.add_argument('--diag', action='store_const', const=True, default=False)

    a, args = p.parse_max(args[1:])

    if a.version:
        print("\n".join(svn_version.svn_version().split("\n")[:-2]))
        INDENT = "    "
        print(INDENT + "Python version: {}".format(sys.version).replace("\n", ""))
        print()
        print()

        sys.exit(0)

    if a.print_path:
        print(sys.executable)
        sys.exit(0)

    if a.profile:
        a.precise = True

    init_logger(a.verbose_level)

    if a.precise:
        start = time.time()

        class Handler(logging.StreamHandler):
            def emit(self, record):
                ts, msg = 0, None

                # Special format for events
                if isinstance(record.args, dict) and '_timestamp' in record.args:
                    try:
                        ts, msg = format_msg(record.args)
                    except KeyError:
                        pass

                if msg is None:
                    ts, msg = time.time(), record.getMessage()

                sys.stderr.write("{}: {}\n".format(str(ts - start)[:10], six.ensure_str(msg).strip()))

        logging.root.addHandler(Handler())

    def format_help():
        s = p.format_help().replace('[--diag]', '[--diag] [--help] <SUBCOMMAND> [OPTION]...').strip()

        for line in s.split('\n'):
            if line:
                yield line[0].upper() + line[1:]
            else:
                yield line

    def do_app():
        app.execute_early(do_main)(
            args,
            keep_tmp_dir=a.keep_tmp,
            diag=getattr(a, "diag", False),
            error_file=a.error_file,
            no_report=a.no_report,
            no_logs=a.no_logs,
            no_tmp_dir=a.no_tmp_dir,
            precise=a.precise,
            extra_help='\n'.join(format_help()),
        )

    if a.profile:
        profile.runctx('do_app()', globals(), locals())
    else:
        do_app()