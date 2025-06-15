add_test([=[MyPluginTest.PlaceholderTest]=]  /home/runner/work/clap-jules/clap-jules/build_no_gui/my_plugin_tests [==[--gtest_filter=MyPluginTest.PlaceholderTest]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[MyPluginTest.PlaceholderTest]=]  PROPERTIES WORKING_DIRECTORY /home/runner/work/clap-jules/clap-jules/build_no_gui SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
set(  my_plugin_tests_TESTS MyPluginTest.PlaceholderTest)
