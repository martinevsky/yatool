PY23_LIBRARY()

STYLE_PYTHON()

PY_SRCS(
    __init__.py
)

# !! Keep this list in sync with the RECURSE list in test_tool/ya.make

IF (NOT YA_OPENSOURCE)
    PEERDIR(
        devtools/ya/test/programs/test_tool/run_diff_test
        devtools/ya/test/programs/test_tool/upload
    )
    IF (NOT ARCH_AARCH64 AND NOT ARCH_PPC64LE)
        PEERDIR(
            devtools/ya/test/programs/test_tool/upload_coverage
        )
    ENDIF()
ENDIF()

PEERDIR(
    devtools/ya/test/programs/test_tool/build_python_coverage_report
    devtools/ya/test/programs/test_tool/resolve_python_coverage
)

IF (PYTHON3)
    IF (NOT YA_OPENSOURCE)
        PEERDIR(
            devtools/ya/test/programs/test_tool/build_sancov_coverage_report
            devtools/ya/test/programs/test_tool/check_external
            devtools/ya/test/programs/test_tool/check_mds
            devtools/ya/test/programs/test_tool/check_resource
            devtools/ya/test/programs/test_tool/checkout
            devtools/ya/test/programs/test_tool/cov_merge_vfs
            devtools/ya/test/programs/test_tool/download
            devtools/ya/test/programs/test_tool/minimize_fuzz_corpus
            devtools/ya/test/programs/test_tool/populate_token_to_sandbox_vault
            devtools/ya/test/programs/test_tool/resolve_sancov_coverage
            devtools/ya/test/programs/test_tool/run_clang_tidy
            devtools/ya/test/programs/test_tool/run_eslint
            devtools/ya/test/programs/test_tool/run_hermione
            devtools/ya/test/programs/test_tool/run_jest
            devtools/ya/test/programs/test_tool/sandbox_run_test
        )
    ENDIF()
    PEERDIR(
        devtools/ya/test/programs/test_tool/build_clang_coverage_report
        devtools/ya/test/programs/test_tool/build_go_coverage_report
        devtools/ya/test/programs/test_tool/build_ts_coverage_report
        devtools/ya/test/programs/test_tool/canonization_result_node
        devtools/ya/test/programs/test_tool/canonize
        devtools/ya/test/programs/test_tool/create_allure_report
        devtools/ya/test/programs/test_tool/lib/coverage
        devtools/ya/test/programs/test_tool/lib/migrations_config
        devtools/ya/test/programs/test_tool/lib/monitor
        devtools/ya/test/programs/test_tool/lib/report
        devtools/ya/test/programs/test_tool/lib/secret
        devtools/ya/test/programs/test_tool/lib/testroot
        devtools/ya/test/programs/test_tool/lib/tmpfs
        devtools/ya/test/programs/test_tool/lib/unshare
        devtools/ya/test/programs/test_tool/list_result_node
        devtools/ya/test/programs/test_tool/list_tests
        devtools/ya/test/programs/test_tool/merge_coverage_inplace
        devtools/ya/test/programs/test_tool/merge_python_coverage
        devtools/ya/test/programs/test_tool/resolve_clang_coverage
        devtools/ya/test/programs/test_tool/resolve_go_coverage
        devtools/ya/test/programs/test_tool/resolve_ts_coverage
        devtools/ya/test/programs/test_tool/result_node
        devtools/ya/test/programs/test_tool/results_accumulator
        devtools/ya/test/programs/test_tool/results_merger
        devtools/ya/test/programs/test_tool/run_boost_test
        devtools/ya/test/programs/test_tool/run_check
        devtools/ya/test/programs/test_tool/run_classpath_clash
        devtools/ya/test/programs/test_tool/run_coverage_extractor
        devtools/ya/test/programs/test_tool/run_custom_lint
        devtools/ya/test/programs/test_tool/run_exectest
        devtools/ya/test/programs/test_tool/run_fuzz
        devtools/ya/test/programs/test_tool/run_fuzz_result_node
        devtools/ya/test/programs/test_tool/run_g_benchmark
        devtools/ya/test/programs/test_tool/run_go_fmt
        devtools/ya/test/programs/test_tool/run_go_test
        devtools/ya/test/programs/test_tool/run_go_vet
        devtools/ya/test/programs/test_tool/run_javastyle
        devtools/ya/test/programs/test_tool/run_ktlint_test
        devtools/ya/test/programs/test_tool/run_pyimports
        devtools/ya/test/programs/test_tool/run_skipped_test
        devtools/ya/test/programs/test_tool/run_test
        devtools/ya/test/programs/test_tool/run_ut
        devtools/ya/test/programs/test_tool/unify_clang_coverage
        devtools/ya/test/programs/test_tool/resolve_java_coverage
        devtools/ya/test/programs/test_tool/run_y_benchmark
    )
    IF (NOT ARCH_AARCH64 AND NOT ARCH_PPC64LE)
        PEERDIR(
            devtools/ya/test/programs/test_tool/ytexec_run_test
        )
    ENDIF()
ELSE()
    PEERDIR(
        contrib/deprecated/python/faulthandler
    )
ENDIF()

END()

RECURSE(
    monitor
    report
    coverage
    secret
    tmpfs
    unshare
)
