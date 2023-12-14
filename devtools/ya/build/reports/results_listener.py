import copy
import json
import logging
import os
import threading
import traceback

import exts.fs

from collections import defaultdict

import build.build_plan as bp
import build.stat.graph_metrics as st
import test.const as test_const
import test.common as test_common
import test.result as test_result
import test.reports as test_reports


class BuildResultsListener(object):
    _logger = logging.getLogger('BuildResultsListener')

    def __init__(self, graph, tests, mergers, report_generator, build_root, opts):
        self._lock = threading.Lock()
        self._build_metrics = st.make_targets_metrics(graph['graph'], {})
        self._report_generator = report_generator
        self._build_root = build_root
        self._notified = set()
        self._processed = set()
        self._reversed_deps = defaultdict(set)
        self._nodes = {}
        self._tests = {}
        self._mergers = {}
        self._opts = opts

        for node in graph['graph']:
            self._nodes[node['uid']] = node
            for dep in node['deps']:
                self._reversed_deps[dep].add(node['uid'])
        for tst in tests:
            self._tests[tst.uid] = tst
        for merger in mergers:
            self._mergers[merger.uid] = merger

    def __call__(self, res=None, build_stage=None):
        if res is None:
            res = {}
        if 'status' in res or res.get('files'):
            if res.get('status', 0) == 0:
                self._on_completed(res['uid'], res)
            else:
                stderr, error_links = self._extract_stderr(res)
                self._on_failed(res['uid'], stderr, error_links, res.get('exit_code', -99))
        if build_stage:
            self._on_trace_stage(build_stage)

    def _extract_stderr(self, res):
        import app_config

        if app_config.in_house:
            from yalibrary.yandex.distbuild import distbs

            return distbs.extract_stderr(
                res, self._opts.mds_read_account, download_stderr=self._opts.download_failed_nodes_stderr
            )
        else:
            stderrs = copy.copy(res.get('stderrs', []))
            if self._opts.arc_root:
                stderrs = [i.replace(self._opts.arc_root, '$(SOURCE_ROOT)') for i in stderrs]
            build_root = res.get('build_root')
            if build_root:
                stderrs = [i.replace(build_root, '$(BUILD_ROOT)') for i in stderrs]
            return '\n'.join(stderrs), []

    def _on_completed(self, uid, res):
        if 'node-type' not in self._nodes[uid]:
            self._on_build_node_completed(uid)
        else:
            self._logger.debug('Unknown node %s is completed', uid)

    def _on_failed(self, uid, error, error_links, exit_code):
        if 'node-type' not in self._nodes[uid]:
            self._on_build_node_failed(uid, error, error_links, exit_code)
        elif self._nodes[uid]['node-type'] == 'merger':
            self._on_merge_node_failed(uid, error)
        elif self._nodes[uid]['node-type'] == 'test':
            self._on_test_node_failed(uid, error)
        else:
            self._logger.debug('Unknown node %s is failed', uid)

    def _on_trace_stage(self, build_stage):
        self._report_generator.add_stage(build_stage)

    def _is_module(self, uid):
        return 'module_type' in self._nodes[uid].get('target_properties', {})

    def _resolve_target(self, uid):
        node = self._nodes[uid]
        return bp.BuildPlan.node_name(node), bp.BuildPlan.node_platform(node), bp.BuildPlan.get_module_tag(node)

    def _on_test_node_failed(self, uid, _):
        self._logger.debug('Test node %s is failed', uid)

    def _on_merge_node_failed(self, uid, _):
        self._logger.debug('Merge node %s is failed', uid)

    def _on_build_node_failed(self, uid, error, error_links, exit_code):
        def notify(u, e, a):
            if u not in self._notified:
                target_name, target_platform, module_tag = self._resolve_target(u)
                self._report_generator.add_build_result(
                    u, target_name, target_platform, [e], self._build_metrics.get(u, {}), module_tag, a, exit_code
                )
                self._notified.add(u)

        def make_broken_by_message(u):
            target_name, _, _ = self._resolve_target(u)
            return 'Depends on broken targets:\n{}'.format(target_name)

        def mark_failed(u, broken_dep):
            if u in self._processed:
                return

            self._processed.add(u)
            self._logger.debug('Node %s was broken by %s', u, uid)

            if self._is_module(u):
                msg = make_broken_by_message(broken_dep) if broken_dep else error
                links = [] if broken_dep else [error_links]
                notify(u, msg, links)
                broken_dep = broken_dep or u

            for reversed_dep in sorted(self._reversed_deps.get(u, tuple())):
                mark_failed(reversed_dep, broken_dep)

        with self._lock:
            mark_failed(uid, None)

    def _on_build_node_completed(self, uid):
        with self._lock:
            if self._is_module(uid) and uid not in self._notified:
                target_name, target_platform, module_tag = self._resolve_target(uid)
                self._report_generator.add_build_result(
                    uid, target_name, target_platform, [], self._build_metrics.get(uid, {}), module_tag, [], 0
                )
                self._notified.add(uid)


class TestResultsListener(object):
    _logger = logging.getLogger('TestResultsListener')

    def __init__(self, graph, display):
        self._nodes = {}
        self._display = display
        for node in graph['graph']:
            self._nodes[node['uid']] = node

    def __call__(self, res=None, build_stage=None):
        if not res:
            return
        kv = self._nodes[res["uid"]].get('kv')
        if res and kv and "test_results_node" in kv:
            self._on_test_results_node(res)

    def _on_test_results_node(self, res):
        if 'build_root' not in res:
            return
        build_root = res['build_root']
        console_report = os.path.join(build_root, "test_results.out")
        if os.path.exists(console_report):
            with open(console_report) as f:
                data = f.read().strip()
            if data:
                self._display.emit_message(data)
        else:
            self._logger.error("Expected test console report file was not found by {}".format(console_report))


class TestNodeListener(object):
    _logger = logging.getLogger('TestNodeListener')

    def __init__(self, tests, output_root, report_generator):
        self._lock = threading.Lock()
        self._output_root = output_root
        self._tests = {}
        self._seen = set()
        self._report_generator = report_generator

        for tst in tests:
            self._tests[tst.uid] = tst

    def __call__(self, res=None, build_stage=None):
        if res is None:
            res = {}
        if 'status' in res or res.get('files'):
            uid = res['uid']
            if uid in self._tests:
                status = res.get('status', 0)
                if status == 0:
                    self._on_test_node_completed(uid, res)
                else:
                    self._on_test_node_failed(uid, status)

    def set_report_generator(self, report_generator):
        self._report_generator = report_generator

    def _on_test_node_failed(self, uid, status):
        self._logger.debug('Test node %s is failed. Status: %s', uid, status)

    def _on_test_node_completed(self, uid, res):
        if 'build_root' not in res or uid in self._seen:
            return
        suite = self._tests[uid]
        build_root = res['build_root']
        work_dir = test_common.get_test_suite_work_dir(
            build_root,
            suite.project_path,
            suite.name,
            target_platform_descriptor=suite.target_platform_descriptor,
            multi_target_platform_run=suite.multi_target_platform_run,
        )
        suite.set_work_dir(work_dir)
        resolver = test_reports.TextTransformer(
            [("$(BUILD_ROOT)", self._output_root or build_root), ("$(SOURCE_ROOT)/", "")]
        )
        result = test_result.TestPackedResultView(work_dir)
        try:
            suite.load_run_results(result.trace_report_path, resolver)
        except Exception:
            msg = "Infrastructure error - contact devtools@.\nFailed to load suite results:{}\n".format(
                traceback.format_exc()
            )
            suite.add_suite_error(msg, test_const.Status.INTERNAL)
            logging.debug(msg)

        if self._report_generator is not None:
            with self._lock:
                if uid not in self._seen:
                    self._report_generator.add_tests_results([suite], None, {}, defaultdict(list))
        self._seen.add(uid)


class SlotListener(object):
    _logger = logging.getLogger('SlotListener')

    def __init__(self, statistics_out_dir):
        self._output_file = None
        if statistics_out_dir:
            if not os.path.exists(statistics_out_dir):
                exts.fs.create_dirs(statistics_out_dir)
            self._output_file = os.path.join(statistics_out_dir, 'slot_time.json')

        self._slot_time = 0  # milliseconds

    def finish(self):
        if not self._output_file:
            return

        try:
            with open(self._output_file, 'w') as f:
                f.write(json.dumps({'slot_time': self._slot_time}))
        except Exception as e:
            self._logger.error('Fail to save slot time: %s', e)

    def __call__(self, res=None, build_stage=None):
        if not res:
            return

        for process_result in res.get('process_results', []):
            self._slot_time += process_result.get('slot_time', 0)


class CompositeResultsListener(object):
    def __init__(self, listeners=None):
        self._listeners = []
        self._listeners.extend(listeners)

    def add(self, listener):
        self._listeners.append(listener)

    def __call__(self, *args, **kwargs):
        for listener in self._listeners:
            listener(*args, **kwargs)


class FailedNodeListener(object):
    NAMESPACE = 'build.reports.failed_node_info'
    EVENT = 'node-failed'

    def __init__(self, evlog):
        self._writer = evlog.get_writer(self.NAMESPACE)

    def __call__(self, res=None, build_stage=None):
        if res and res.get('status', 0) != 0 and 'exit_code' in res:
            self._writer(self.EVENT, uid=res['uid'], exit_code=res['exit_code'])
